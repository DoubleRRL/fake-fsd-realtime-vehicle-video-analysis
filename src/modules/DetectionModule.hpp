#pragma once

#include "../core/Types.hpp"
#include "../core/BufferPool.hpp"
#include <CoreML/CoreML.h>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

class DetectionModule {
public:
    explicit DetectionModule(std::shared_ptr<BufferPool> bufferPool);
    ~DetectionModule();

    // Initialize Core ML model
    bool initialize(const std::string& modelPath);
    
    // Process frame for object detection
    std::vector<Detection> detectObjects(const std::shared_ptr<ProcessedFrame>& frame);
    
    // Configuration
    void setConfidenceThreshold(float threshold);
    void setNMSThreshold(float threshold);
    void setMaxDetections(int maxDetections);
    void setDetectionClasses(const std::vector<std::string>& classes);
    
    // Performance
    DetectionStats getStats() const;
    
    // Error handling
    std::string getLastError() const { return lastError_; }

private:
    // Core ML setup
    bool loadModel(const std::string& modelPath);
    bool setupModelInput();
    bool setupModelOutput();
    
    // Detection processing (optimized versions)
    bool preprocessFrameOptimized(const std::shared_ptr<ProcessedFrame>& frame, MLMultiArray* inputArray);
    std::vector<Detection> postprocessOutputOptimized(MLMultiArray* outputArray, int frameWidth, int frameHeight);
    std::vector<Detection> applyNMSOptimized(const std::vector<Detection>& detections);
    
    // Utility functions (optimized versions)
    cv::Rect convertToPixelCoordsOptimized(float x, float y, float w, float h, int frameWidth, int frameHeight, int padX, int padY, float scale);
    float calculateIoUOptimized(const cv::Rect& rect1, const cv::Rect& rect2);
    
    // Buffer pre-allocation for zero-copy operations
    void preallocateBuffers();
    void preallocateOutputBuffers();
    
    // Legacy methods for compatibility
    bool preprocessFrame(const std::shared_ptr<ProcessedFrame>& frame, MLMultiArray* inputArray);
    std::vector<Detection> postprocessOutput(MLMultiArray* outputArray, int frameWidth, int frameHeight);
    std::vector<Detection> applyNMS(const std::vector<Detection>& detections);
    cv::Rect convertToPixelCoords(float x, float y, float w, float h, int frameWidth, int frameHeight);
    float calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2);
    
    // Core ML objects
    MLModel* model_;
    MLModelConfiguration* modelConfig_;
    
    // Pre-allocated buffers for zero-copy operations
    MLMultiArray* preallocatedInputArray_;
    MLMultiArray* preallocatedOutputArray_;
    
    // Buffer pool
    std::shared_ptr<BufferPool> bufferPool_;
    
    // Configuration
    float confidenceThreshold_;
    float nmsThreshold_;
    int maxDetections_;
    std::vector<std::string> detectionClasses_;
    
    // Model parameters
    int inputWidth_;
    int inputHeight_;
    int inputChannels_;
    std::string inputName_;
    std::string outputName_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    DetectionStats stats_;
    
    // Error handling
    mutable std::mutex errorMutex_;
    std::string lastError_;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastDetectionTime_;
    uint64_t processedFrames_;
    
    // Thread safety
    std::mutex detectionMutex_;
    
    // Default COCO classes (can be overridden)
    static const std::vector<std::string> DEFAULT_CLASSES;
}; 