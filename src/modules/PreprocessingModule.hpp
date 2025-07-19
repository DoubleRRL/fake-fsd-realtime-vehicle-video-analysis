#pragma once

#include "../core/Types.hpp"
#include "../core/BufferPool.hpp"
#include <Metal/Metal.h>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

class PreprocessingModule {
public:
    explicit PreprocessingModule(std::shared_ptr<BufferPool> bufferPool);
    ~PreprocessingModule();

    // Initialize Metal device and shaders
    bool initialize();
    
    // Process frame with GPU acceleration
    std::shared_ptr<ProcessedFrame> processFrame(const std::shared_ptr<FrameData>& inputFrame);
    
    // Configuration
    void setTargetResolution(int width, int height);
    void setEnhancementLevel(float level);
    void setNoiseReduction(bool enable);
    void setHistogramEqualization(bool enable);
    
    // Performance
    PreprocessingStats getStats() const;
    
    // Error handling
    std::string getLastError() const { return lastError_; }

private:
    // Metal setup
    bool setupMetal();
    bool loadShaders();
    bool createCommandQueue();
    
    // GPU processing
    bool resizeFrame(const std::shared_ptr<FrameData>& input, std::shared_ptr<ProcessedFrame>& output);
    bool enhanceImage(std::shared_ptr<ProcessedFrame>& frame);
    bool reduceNoise(std::shared_ptr<ProcessedFrame>& frame);
    bool equalizeHistogram(std::shared_ptr<ProcessedFrame>& frame);
    bool detectEdges(std::shared_ptr<ProcessedFrame>& frame);
    
    // Buffer management
    id<MTLBuffer> createGPUBuffer(const void* data, size_t size);
    void copyToGPU(id<MTLBuffer> buffer, const void* data, size_t size);
    void copyFromGPU(id<MTLBuffer> buffer, void* data, size_t size);
    
    // Metal objects
    id<MTLDevice> device_;
    id<MTLCommandQueue> commandQueue_;
    id<MTLComputePipelineState> resizePipeline_;
    id<MTLComputePipelineState> enhancePipeline_;
    id<MTLComputePipelineState> noiseReductionPipeline_;
    id<MTLComputePipelineState> histogramPipeline_;
    id<MTLComputePipelineState> edgeDetectionPipeline_;
    
    // Buffer pool
    std::shared_ptr<BufferPool> bufferPool_;
    
    // Configuration
    int targetWidth_;
    int targetHeight_;
    float enhancementLevel_;
    bool noiseReductionEnabled_;
    bool histogramEqualizationEnabled_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    PreprocessingStats stats_;
    
    // Error handling
    mutable std::mutex errorMutex_;
    std::string lastError_;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastProcessTime_;
    uint64_t processedFrames_;
    
    // Thread safety
    std::mutex processingMutex_;
}; 