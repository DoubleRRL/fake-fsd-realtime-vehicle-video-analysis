#include "PreprocessingModule.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>

PreprocessingModule::PreprocessingModule(std::shared_ptr<BufferPool> bufferPool)
    : bufferPool_(bufferPool)
    , device_(nullptr)
    , commandQueue_(nullptr)
    , resizePipeline_(nullptr)
    , enhancePipeline_(nullptr)
    , noiseReductionPipeline_(nullptr)
    , histogramPipeline_(nullptr)
    , edgeDetectionPipeline_(nullptr)
    , targetWidth_(960)
    , targetHeight_(540)
    , enhancementLevel_(1.0f)
    , noiseReductionEnabled_(true)
    , histogramEqualizationEnabled_(false)
    , processedFrames_(0) {
}

PreprocessingModule::~PreprocessingModule() {
    // Metal objects are automatically released by ARC
}

bool PreprocessingModule::initialize() {
    if (!setupMetal()) {
        return false;
    }
    
    if (!loadShaders()) {
        return false;
    }
    
    if (!createCommandQueue()) {
        return false;
    }
    
    return true;
}

bool PreprocessingModule::setupMetal() {
    device_ = MTLCreateSystemDefaultDevice();
    if (!device_) {
        lastError_ = "Failed to create Metal device";
        return false;
    }
    
    return true;
}

bool PreprocessingModule::loadShaders() {
    NSError* error = nil;
    
    // Load shader library
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* libraryPath = [bundle pathForResource:@"preprocessing" ofType:@"metallib"];
    if (!libraryPath) {
        // Try to load from shaders directory
        libraryPath = @"shaders/preprocessing.metallib";
    }
    
    id<MTLLibrary> library = [device_ newLibraryWithFile:libraryPath error:&error];
    if (!library) {
        lastError_ = "Failed to load Metal shader library: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    // Create compute pipeline states
    resizePipeline_ = [device_ newComputePipelineStateWithFunction:[library newFunctionWithName:@"resizeFrame"] error:&error];
    if (!resizePipeline_) {
        lastError_ = "Failed to create resize pipeline: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    enhancePipeline_ = [device_ newComputePipelineStateWithFunction:[library newFunctionWithName:@"enhanceImage"] error:&error];
    if (!enhancePipeline_) {
        lastError_ = "Failed to create enhance pipeline: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    noiseReductionPipeline_ = [device_ newComputePipelineStateWithFunction:[library newFunctionWithName:@"reduceNoise"] error:&error];
    if (!noiseReductionPipeline_) {
        lastError_ = "Failed to create noise reduction pipeline: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    histogramPipeline_ = [device_ newComputePipelineStateWithFunction:[library newFunctionWithName:@"equalizeHistogram"] error:&error];
    if (!histogramPipeline_) {
        lastError_ = "Failed to create histogram pipeline: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    edgeDetectionPipeline_ = [device_ newComputePipelineStateWithFunction:[library newFunctionWithName:@"detectEdges"] error:&error];
    if (!edgeDetectionPipeline_) {
        lastError_ = "Failed to create edge detection pipeline: " + std::string([error.localizedDescription UTF8String]);
        return false;
    }
    
    return true;
}

bool PreprocessingModule::createCommandQueue() {
    commandQueue_ = [device_ newCommandQueue];
    if (!commandQueue_) {
        lastError_ = "Failed to create Metal command queue";
        return false;
    }
    
    return true;
}

std::shared_ptr<ProcessedFrame> PreprocessingModule::processFrame(const std::shared_ptr<FrameData>& inputFrame) {
    if (!inputFrame || !inputFrame->data) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(processingMutex_);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Create output frame
    auto outputFrame = std::make_shared<ProcessedFrame>();
    outputFrame->width = targetWidth_;
    outputFrame->height = targetHeight_;
    outputFrame->timestamp = inputFrame->timestamp;
    
    // Allocate output buffer
    size_t outputSize = targetWidth_ * targetHeight_ * 4; // RGBA
    outputFrame->data = bufferPool_->allocateBuffer(outputSize);
    
    if (!outputFrame->data) {
        lastError_ = "Failed to allocate output buffer";
        return nullptr;
    }
    
    // Resize frame
    if (!resizeFrame(inputFrame, outputFrame)) {
        return nullptr;
    }
    
    // Apply enhancements
    if (enhancementLevel_ > 1.0f) {
        if (!enhanceImage(outputFrame)) {
            return nullptr;
        }
    }
    
    // Apply noise reduction
    if (noiseReductionEnabled_) {
        if (!reduceNoise(outputFrame)) {
            return nullptr;
        }
    }
    
    // Apply histogram equalization
    if (histogramEqualizationEnabled_) {
        if (!equalizeHistogram(outputFrame)) {
            return nullptr;
        }
    }
    
    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    std::lock_guard<std::mutex> statsLock(statsMutex_);
    stats_.totalFrames++;
    stats_.averageProcessingTime = (stats_.averageProcessingTime * (stats_.totalFrames - 1) + duration.count()) / stats_.totalFrames;
    stats_.lastProcessingTime = duration.count();
    
    processedFrames_++;
    
    return outputFrame;
}

bool PreprocessingModule::resizeFrame(const std::shared_ptr<FrameData>& input, std::shared_ptr<ProcessedFrame>& output) {
    if (!resizePipeline_ || !commandQueue_) {
        return false;
    }
    
    // Create GPU buffers
    id<MTLBuffer> inputBuffer = createGPUBuffer(input->data->data(), input->data->size());
    id<MTLBuffer> outputBuffer = createGPUBuffer(nullptr, output->data->size());
    
    if (!inputBuffer || !outputBuffer) {
        return false;
    }
    
    // Create command buffer
    id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    
    [encoder setComputePipelineState:resizePipeline_];
    [encoder setBuffer:inputBuffer offset:0 atIndex:0];
    [encoder setBuffer:outputBuffer offset:0 atIndex:1];
    
    // Set parameters
    struct ResizeParams {
        uint32_t inputWidth;
        uint32_t inputHeight;
        uint32_t outputWidth;
        uint32_t outputHeight;
        uint32_t inputStride;
    } params;
    
    params.inputWidth = static_cast<uint32_t>(input->width);
    params.inputHeight = static_cast<uint32_t>(input->height);
    params.outputWidth = static_cast<uint32_t>(output->width);
    params.outputHeight = static_cast<uint32_t>(output->height);
    params.inputStride = static_cast<uint32_t>(input->stride);
    
    id<MTLBuffer> paramsBuffer = createGPUBuffer(&params, sizeof(params));
    [encoder setBuffer:paramsBuffer offset:0 atIndex:2];
    
    // Calculate threadgroup size
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
    MTLSize threadgroups = MTLSizeMake(
        (output->width + threadgroupSize.width - 1) / threadgroupSize.width,
        (output->height + threadgroupSize.height - 1) / threadgroupSize.height,
        1
    );
    
    [encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadgroupSize];
    [encoder endEncoding];
    
    // Commit and wait
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
    // Copy result back to CPU
    copyFromGPU(outputBuffer, output->data->data(), output->data->size());
    
    return true;
}

bool PreprocessingModule::enhanceImage(std::shared_ptr<ProcessedFrame>& frame) {
    if (!enhancePipeline_ || !commandQueue_) {
        return false;
    }
    
    // Create GPU buffer for enhanced image
    id<MTLBuffer> enhancedBuffer = createGPUBuffer(nullptr, frame->data->size());
    
    if (!enhancedBuffer) {
        return false;
    }
    
    // Create command buffer
    id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    
    [encoder setComputePipelineState:enhancePipeline_];
    [encoder setBuffer:enhancedBuffer offset:0 atIndex:0];
    
    // Set enhancement parameters
    struct EnhanceParams {
        float enhancementLevel;
        uint32_t width;
        uint32_t height;
    } params;
    
    params.enhancementLevel = enhancementLevel_;
    params.width = static_cast<uint32_t>(frame->width);
    params.height = static_cast<uint32_t>(frame->height);
    
    id<MTLBuffer> paramsBuffer = createGPUBuffer(&params, sizeof(params));
    [encoder setBuffer:paramsBuffer offset:0 atIndex:1];
    
    // Copy current frame to GPU
    id<MTLBuffer> currentBuffer = createGPUBuffer(frame->data->data(), frame->data->size());
    [encoder setBuffer:currentBuffer offset:0 atIndex:2];
    
    // Dispatch
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
    MTLSize threadgroups = MTLSizeMake(
        (frame->width + threadgroupSize.width - 1) / threadgroupSize.width,
        (frame->height + threadgroupSize.height - 1) / threadgroupSize.height,
        1
    );
    
    [encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadgroupSize];
    [encoder endEncoding];
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
    // Copy enhanced result back
    copyFromGPU(enhancedBuffer, frame->data->data(), frame->data->size());
    
    return true;
}

bool PreprocessingModule::reduceNoise(std::shared_ptr<ProcessedFrame>& frame) {
    if (!noiseReductionPipeline_ || !commandQueue_) {
        return false;
    }
    
    // Similar implementation to enhanceImage but with noise reduction parameters
    // This is a simplified implementation - full version would include bilateral filtering
    
    id<MTLBuffer> denoisedBuffer = createGPUBuffer(nullptr, frame->data->size());
    
    if (!denoisedBuffer) {
        return false;
    }
    
    id<MTLCommandBuffer> commandBuffer = [commandQueue_ commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
    
    [encoder setComputePipelineState:noiseReductionPipeline_];
    [encoder setBuffer:denoisedBuffer offset:0 atIndex:0];
    
    // Set noise reduction parameters
    struct NoiseParams {
        float sigma;
        uint32_t width;
        uint32_t height;
    } params;
    
    params.sigma = 1.0f; // Noise reduction strength
    params.width = static_cast<uint32_t>(frame->width);
    params.height = static_cast<uint32_t>(frame->height);
    
    id<MTLBuffer> paramsBuffer = createGPUBuffer(&params, sizeof(params));
    [encoder setBuffer:paramsBuffer offset:0 atIndex:1];
    
    id<MTLBuffer> currentBuffer = createGPUBuffer(frame->data->data(), frame->data->size());
    [encoder setBuffer:currentBuffer offset:0 atIndex:2];
    
    MTLSize threadgroupSize = MTLSizeMake(16, 16, 1);
    MTLSize threadgroups = MTLSizeMake(
        (frame->width + threadgroupSize.width - 1) / threadgroupSize.width,
        (frame->height + threadgroupSize.height - 1) / threadgroupSize.height,
        1
    );
    
    [encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threadgroupSize];
    [encoder endEncoding];
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    
    copyFromGPU(denoisedBuffer, frame->data->data(), frame->data->size());
    
    return true;
}

bool PreprocessingModule::equalizeHistogram(std::shared_ptr<ProcessedFrame>& frame) {
    if (!histogramPipeline_ || !commandQueue_) {
        return false;
    }
    
    // Histogram equalization implementation
    // This would compute histogram and apply equalization in two passes
    
    return true; // Simplified for now
}

bool PreprocessingModule::detectEdges(std::shared_ptr<ProcessedFrame>& frame) {
    if (!edgeDetectionPipeline_ || !commandQueue_) {
        return false;
    }
    
    // Edge detection implementation using Sobel or Canny
    // This would create an edge map overlay
    
    return true; // Simplified for now
}

id<MTLBuffer> PreprocessingModule::createGPUBuffer(const void* data, size_t size) {
    MTLResourceOptions options = MTLResourceStorageModeShared;
    return [device_ newBufferWithBytes:data length:size options:options];
}

void PreprocessingModule::copyToGPU(id<MTLBuffer> buffer, const void* data, size_t size) {
    if (buffer && data) {
        memcpy([buffer contents], data, size);
    }
}

void PreprocessingModule::copyFromGPU(id<MTLBuffer> buffer, void* data, size_t size) {
    if (buffer && data) {
        memcpy(data, [buffer contents], size);
    }
}

void PreprocessingModule::setTargetResolution(int width, int height) {
    targetWidth_ = width;
    targetHeight_ = height;
}

void PreprocessingModule::setEnhancementLevel(float level) {
    enhancementLevel_ = std::max(0.1f, std::min(3.0f, level));
}

void PreprocessingModule::setNoiseReduction(bool enable) {
    noiseReductionEnabled_ = enable;
}

void PreprocessingModule::setHistogramEqualization(bool enable) {
    histogramEqualizationEnabled_ = enable;
}

PreprocessingStats PreprocessingModule::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
} 