#include "VideoInputModule.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>

VideoInputModule::VideoInputModule(std::shared_ptr<BufferPool> bufferPool)
    : bufferPool_(bufferPool)
    , captureSession_(nullptr)
    , videoInput_(nullptr)
    , videoOutput_(nullptr)
    , player_(nullptr)
    , playerItem_(nullptr)
    , running_(false)
    , shouldStop_(false)
    , targetWidth_(960)
    , targetHeight_(540)
    , targetFPS_(50)
    , maxBufferSize_(10)
    , frameCount_(0) {
}

VideoInputModule::~VideoInputModule() {
    stop();
}

bool VideoInputModule::initialize(const std::string& source, bool isCamera) {
    sourcePath_ = source;
    isCamera_ = isCamera;
    
    if (isCamera_) {
        return setupCameraInput();
    } else {
        return setupFileInput(source);
    }
}

bool VideoInputModule::setupFileInput(const std::string& filePath) {
    @autoreleasepool {
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePath.c_str()]];
        if (!url) {
            lastError_ = "Invalid file path: " + filePath;
            return false;
        }
        
        playerItem_ = [[AVPlayerItem alloc] initWithURL:url];
        if (!playerItem_) {
            lastError_ = "Failed to create player item for: " + filePath;
            return false;
        }
        
        player_ = [[AVPlayer alloc] initWithPlayerItem:playerItem_];
        if (!player_) {
            lastError_ = "Failed to create player for: " + filePath;
            return false;
        }
        
        // Set video output for frame extraction
        setupVideoOutput();
        
        return true;
    }
}

bool VideoInputModule::setupCameraInput() {
    @autoreleasepool {
        captureSession_ = [[AVCaptureSession alloc] init];
        if (!captureSession_) {
            lastError_ = "Failed to create capture session";
            return false;
        }
        
        // Find the best camera (prefer wide angle)
        AVCaptureDevice* camera = [AVCaptureDevice defaultDeviceWithDeviceType:AVCaptureDeviceTypeBuiltInWideAngleCamera
                                                                   mediaType:AVMediaTypeVideo
                                                                   position:AVCaptureDevicePositionBack];
        if (!camera) {
            camera = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
        }
        
        if (!camera) {
            lastError_ = "No camera available";
            return false;
        }
        
        // Create input
        NSError* error = nil;
        videoInput_ = [AVCaptureDeviceInput deviceInputWithDevice:camera error:&error];
        if (!videoInput_) {
            lastError_ = "Failed to create camera input: " + std::string([error.localizedDescription UTF8String]);
            return false;
        }
        
        if (![captureSession_ canAddInput:videoInput_]) {
            lastError_ = "Cannot add camera input to session";
            return false;
        }
        
        [captureSession_ addInput:videoInput_];
        
        // Create output
        videoOutput_ = [[AVCaptureVideoDataOutput alloc] init];
        if (!videoOutput_) {
            lastError_ = "Failed to create video output";
            return false;
        }
        
        // Configure output format
        videoOutput_.videoSettings = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
        };
        
        // Set delegate for frame processing
        dispatch_queue_t queue = dispatch_queue_create("video.capture.queue", DISPATCH_QUEUE_SERIAL);
        [videoOutput_ setSampleBufferDelegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)self queue:queue];
        
        if (![captureSession_ canAddOutput:videoOutput_]) {
            lastError_ = "Cannot add video output to session";
            return false;
        }
        
        [captureSession_ addOutput:videoOutput_];
        
        // Configure session quality
        if ([captureSession_ canSetSessionPreset:AVCaptureSessionPreset1280x720]) {
            [captureSession_ setSessionPreset:AVCaptureSessionPreset1280x720];
        }
        
        return true;
    }
}

void VideoInputModule::setupVideoOutput() {
    // For file input, we'll extract frames using AVAssetReader
    // This will be implemented in the capture thread
}

void VideoInputModule::start() {
    if (running_.load()) {
        return;
    }
    
    shouldStop_ = false;
    running_ = true;
    
    if (isCamera_) {
        @autoreleasepool {
            [captureSession_ startRunning];
        }
    }
    
    // Start processing thread
    processThread_ = std::thread(&VideoInputModule::processThread, this);
}

void VideoInputModule::stop() {
    if (!running_.load()) {
        return;
    }
    
    shouldStop_ = true;
    
    if (isCamera_) {
        @autoreleasepool {
            [captureSession_ stopRunning];
        }
    }
    
    if (processThread_.joinable()) {
        processThread_.join();
    }
    
    running_ = false;
    
    // Clear frame queue
    std::lock_guard<std::mutex> lock(queueMutex_);
    while (!frameQueue_.empty()) {
        frameQueue_.pop();
    }
}

std::shared_ptr<FrameData> VideoInputModule::getNextFrame() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    
    if (frameQueue_.empty()) {
        return nullptr;
    }
    
    auto frame = frameQueue_.front();
    frameQueue_.pop();
    
    return frame;
}

void VideoInputModule::setResolution(int width, int height) {
    targetWidth_ = width;
    targetHeight_ = height;
}

void VideoInputModule::setFPS(int fps) {
    targetFPS_ = fps;
}

void VideoInputModule::setBufferSize(size_t size) {
    maxBufferSize_ = size;
}

VideoInputStats VideoInputModule::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

void VideoInputModule::processFrame(CMSampleBufferRef sampleBuffer) {
    if (!sampleBuffer) return;
    
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imageBuffer) return;
    
    auto frameData = convertToFrameData(imageBuffer);
    if (!frameData) return;
    
    // Add to queue with size limit
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (frameQueue_.size() >= maxBufferSize_) {
        frameQueue_.pop(); // Remove oldest frame
    }
    frameQueue_.push(frameData);
    frameAvailable_.notify_one();
    
    // Update statistics
    auto now = std::chrono::high_resolution_clock::now();
    if (frameCount_ > 0) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime_);
        stats_.averageFrameTime = (stats_.averageFrameTime * frameCount_ + duration.count()) / (frameCount_ + 1);
    }
    lastFrameTime_ = now;
    frameCount_++;
    stats_.totalFrames = frameCount_;
}

std::shared_ptr<FrameData> VideoInputModule::convertToFrameData(CVPixelBufferRef pixelBuffer) {
    if (!pixelBuffer) return nullptr;
    
    CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    
    void* baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
    if (!baseAddress) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return nullptr;
    }
    
    // Allocate frame data from buffer pool
    auto frameData = std::make_shared<FrameData>();
    frameData->width = static_cast<int>(width);
    frameData->height = static_cast<int>(height);
    frameData->timestamp = std::chrono::high_resolution_clock::now();
    
    // Allocate buffer for frame data
    size_t dataSize = height * bytesPerRow;
    frameData->data = bufferPool_->allocateBuffer(dataSize);
    
    if (frameData->data) {
        // Copy pixel data
        memcpy(frameData->data->data(), baseAddress, dataSize);
        frameData->stride = static_cast<int>(bytesPerRow);
    }
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    
    return frameData;
}

void VideoInputModule::processThread() {
    while (!shouldStop_.load()) {
        // For camera input, frames are processed via delegate callbacks
        // For file input, we need to extract frames from the player
        
        if (!isCamera_ && player_) {
            @autoreleasepool {
                // Extract frame from player (simplified implementation)
                // In a full implementation, we'd use AVAssetReader for better control
                CMTime currentTime = player_.currentTime;
                if (CMTIME_IS_VALID(currentTime)) {
                    // Request frame at current time
                    // This is a simplified approach - full implementation would use AVAssetReader
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// AVFoundation delegate methods (implemented as Objective-C++ extension)
@implementation VideoInputModule (AVCaptureVideoDataOutputSampleBufferDelegate)

- (void)captureOutput:(AVCaptureOutput*)output 
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
       fromConnection:(AVCaptureConnection*)connection {
    if (self->running_.load()) {
        self->processFrame(sampleBuffer);
    }
}

@end 