#pragma once

#include "../core/Types.hpp"
#include "../core/BufferPool.hpp"
#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

class VideoInputModule {
public:
    explicit VideoInputModule(std::shared_ptr<BufferPool> bufferPool);
    ~VideoInputModule();

    // Initialize video input from file or camera
    bool initialize(const std::string& source, bool isCamera = false);
    
    // Start/stop video capture
    void start();
    void stop();
    
    // Get next frame (non-blocking)
    std::shared_ptr<FrameData> getNextFrame();
    
    // Configuration
    void setResolution(int width, int height);
    void setFPS(int fps);
    void setBufferSize(size_t size);
    
    // Status
    bool isRunning() const { return running_.load(); }
    VideoInputStats getStats() const;
    
    // Error handling
    std::string getLastError() const { return lastError_; }

private:
    // Video capture setup
    bool setupFileInput(const std::string& filePath);
    bool setupCameraInput();
    void setupVideoOutput();
    
    // Frame processing
    void processFrame(CMSampleBufferRef sampleBuffer);
    std::shared_ptr<FrameData> convertToFrameData(CVPixelBufferRef pixelBuffer);
    
    // Thread management
    void captureThread();
    void processThread();
    
    // AVFoundation objects
    AVCaptureSession* captureSession_;
    AVCaptureDeviceInput* videoInput_;
    AVCaptureVideoDataOutput* videoOutput_;
    AVPlayer* player_;
    AVPlayerItem* playerItem_;
    
    // Buffer management
    std::shared_ptr<BufferPool> bufferPool_;
    std::queue<std::shared_ptr<FrameData>> frameQueue_;
    std::mutex queueMutex_;
    std::condition_variable frameAvailable_;
    
    // Threading
    std::thread captureThread_;
    std::thread processThread_;
    std::atomic<bool> running_;
    std::atomic<bool> shouldStop_;
    
    // Configuration
    int targetWidth_;
    int targetHeight_;
    int targetFPS_;
    size_t maxBufferSize_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    VideoInputStats stats_;
    
    // Error handling
    mutable std::mutex errorMutex_;
    std::string lastError_;
    
    // Source information
    std::string sourcePath_;
    bool isCamera_;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    uint64_t frameCount_;
}; 