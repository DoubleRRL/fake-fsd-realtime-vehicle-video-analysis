#include "Pipeline.hpp"
#include "../modules/VideoInputModule.hpp"
#include "../modules/PreprocessingModule.hpp"
#include "../modules/DetectionModule.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>

Pipeline::Pipeline()
    : bufferPool_(std::make_shared<BufferPool>())
    , performanceMonitor_(std::make_shared<PerformanceMonitor>())
    , running_(false)
    , shouldStop_(false)
    , frameCount_(0) {
    
    // Initialize modules
    videoInput_ = std::make_unique<VideoInputModule>(bufferPool_);
    preprocessing_ = std::make_unique<PreprocessingModule>(bufferPool_);
    detection_ = std::make_unique<DetectionModule>(bufferPool_);
    
    // Initialize circular buffers
    inputBuffer_ = std::make_unique<CircularBuffer<std::shared_ptr<FrameData>>>(10);
    processedBuffer_ = std::make_unique<CircularBuffer<std::shared_ptr<ProcessedFrame>>>(10);
    detectionBuffer_ = std::make_unique<CircularBuffer<std::vector<Detection>>>(10);
    resultBuffer_ = std::make_unique<CircularBuffer<FrameResult>>(10);
}

Pipeline::~Pipeline() {
    stop();
}

bool Pipeline::initialize(const PipelineConfig& config) {
    config_ = config;
    
    // Initialize buffer pool
    if (!bufferPool_->initialize(config.bufferPoolSize, config.maxBufferSize)) {
        std::cerr << "Failed to initialize buffer pool" << std::endl;
        return false;
    }
    
    // Initialize performance monitor
    if (!performanceMonitor_->initialize()) {
        std::cerr << "Failed to initialize performance monitor" << std::endl;
        return false;
    }
    
    // Initialize video input
    if (!videoInput_->initialize(config.videoSource, config.isCamera)) {
        std::cerr << "Failed to initialize video input: " << videoInput_->getLastError() << std::endl;
        return false;
    }
    
    // Set video input configuration
    videoInput_->setResolution(config.targetWidth, config.targetHeight);
    videoInput_->setFPS(config.targetFPS);
    videoInput_->setBufferSize(config.inputBufferSize);
    
    // Initialize preprocessing
    if (!preprocessing_->initialize()) {
        std::cerr << "Failed to initialize preprocessing: " << preprocessing_->getLastError() << std::endl;
        return false;
    }
    
    // Set preprocessing configuration
    preprocessing_->setTargetResolution(config.targetWidth, config.targetHeight);
    preprocessing_->setEnhancementLevel(config.enhancementLevel);
    preprocessing_->setNoiseReduction(config.noiseReduction);
    preprocessing_->setHistogramEqualization(config.histogramEqualization);
    
    // Initialize detection
    if (!detection_->initialize(config.modelPath)) {
        std::cerr << "Failed to initialize detection: " << detection_->getLastError() << std::endl;
        return false;
    }
    
    // Set detection configuration
    detection_->setConfidenceThreshold(config.confidenceThreshold);
    detection_->setNMSThreshold(config.nmsThreshold);
    detection_->setMaxDetections(config.maxDetections);
    
    return true;
}

void Pipeline::start() {
    if (running_.load()) {
        return;
    }
    
    shouldStop_ = false;
    running_ = true;
    
    // Start video input
    videoInput_->start();
    
    // Start processing threads
    inputThread_ = std::thread(&Pipeline::inputThread, this);
    preprocessingThread_ = std::thread(&Pipeline::preprocessingThread, this);
    detectionThread_ = std::thread(&Pipeline::detectionThread, this);
    resultThread_ = std::thread(&Pipeline::resultThread, this);
    
    // Start performance monitoring
    performanceMonitor_->start();
    
    std::cout << "Pipeline started successfully" << std::endl;
}

void Pipeline::stop() {
    if (!running_.load()) {
        return;
    }
    
    shouldStop_ = true;
    
    // Stop video input
    videoInput_->stop();
    
    // Stop performance monitoring
    performanceMonitor_->stop();
    
    // Wait for threads to finish
    if (inputThread_.joinable()) {
        inputThread_.join();
    }
    if (preprocessingThread_.joinable()) {
        preprocessingThread_.join();
    }
    if (detectionThread_.joinable()) {
        detectionThread_.join();
    }
    if (resultThread_.joinable()) {
        resultThread_.join();
    }
    
    running_ = false;
    
    std::cout << "Pipeline stopped" << std::endl;
}

bool Pipeline::isRunning() const {
    return running_.load();
}

PipelineStats Pipeline::getStats() const {
    PipelineStats stats;
    
    // Get module stats
    stats.videoInputStats = videoInput_->getStats();
    stats.preprocessingStats = preprocessing_->getStats();
    stats.detectionStats = detection_->getStats();
    stats.performanceStats = performanceMonitor_->getStats();
    
    // Calculate pipeline stats
    stats.totalFrames = frameCount_;
    stats.averageLatency = calculateAverageLatency();
    stats.currentFPS = calculateCurrentFPS();
    
    return stats;
}

std::shared_ptr<FrameResult> Pipeline::getLatestResult() {
    std::lock_guard<std::mutex> lock(resultMutex_);
    if (latestResult_) {
        return latestResult_;
    }
    return nullptr;
}

void Pipeline::inputThread() {
    while (!shouldStop_.load()) {
        auto frame = videoInput_->getNextFrame();
        if (frame) {
            // Add to input buffer
            if (inputBuffer_->tryPush(frame)) {
                frameCount_++;
            } else {
                // Buffer full, skip frame
                std::cerr << "Input buffer full, skipping frame" << std::endl;
            }
        } else {
            // No frame available, small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Pipeline::preprocessingThread() {
    while (!shouldStop_.load()) {
        std::shared_ptr<FrameData> inputFrame;
        
        if (inputBuffer_->tryPop(inputFrame)) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Process frame
            auto processedFrame = preprocessing_->processFrame(inputFrame);
            
            if (processedFrame) {
                // Add to processed buffer
                if (!processedBuffer_->tryPush(processedFrame)) {
                    std::cerr << "Processed buffer full, skipping frame" << std::endl;
                }
            }
            
            // Update timing
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            
            std::lock_guard<std::mutex> lock(timingMutex_);
            preprocessingTimes_.push_back(duration.count());
            if (preprocessingTimes_.size() > 100) {
                preprocessingTimes_.pop_front();
            }
        } else {
            // No frame available, small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Pipeline::detectionThread() {
    while (!shouldStop_.load()) {
        std::shared_ptr<ProcessedFrame> processedFrame;
        
        if (processedBuffer_->tryPop(processedFrame)) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Detect objects
            auto detections = detection_->detectObjects(processedFrame);
            
            // Add to detection buffer
            if (!detectionBuffer_->tryPush(detections)) {
                std::cerr << "Detection buffer full, skipping frame" << std::endl;
            }
            
            // Update timing
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            
            std::lock_guard<std::mutex> lock(timingMutex_);
            detectionTimes_.push_back(duration.count());
            if (detectionTimes_.size() > 100) {
                detectionTimes_.pop_front();
            }
        } else {
            // No frame available, small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void Pipeline::resultThread() {
    while (!shouldStop_.load()) {
        std::vector<Detection> detections;
        
        if (detectionBuffer_->tryPop(detections)) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Create frame result
            auto result = std::make_shared<FrameResult>();
            result->detections = detections;
            result->timestamp = std::chrono::high_resolution_clock::now();
            result->frameNumber = frameCount_;
            
            // Add to result buffer
            if (!resultBuffer_->tryPush(*result)) {
                std::cerr << "Result buffer full, skipping frame" << std::endl;
            }
            
            // Update latest result
            {
                std::lock_guard<std::mutex> lock(resultMutex_);
                latestResult_ = result;
            }
            
            // Update timing
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            
            std::lock_guard<std::mutex> lock(timingMutex_);
            resultTimes_.push_back(duration.count());
            if (resultTimes_.size() > 100) {
                resultTimes_.pop_front();
            }
        } else {
            // No detections available, small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

float Pipeline::calculateAverageLatency() const {
    std::lock_guard<std::mutex> lock(timingMutex_);
    
    if (preprocessingTimes_.empty() && detectionTimes_.empty() && resultTimes_.empty()) {
        return 0.0f;
    }
    
    int64_t totalTime = 0;
    int count = 0;
    
    // Sum preprocessing times
    for (auto time : preprocessingTimes_) {
        totalTime += time;
        count++;
    }
    
    // Sum detection times
    for (auto time : detectionTimes_) {
        totalTime += time;
        count++;
    }
    
    // Sum result times
    for (auto time : resultTimes_) {
        totalTime += time;
        count++;
    }
    
    return count > 0 ? static_cast<float>(totalTime) / count : 0.0f;
}

float Pipeline::calculateCurrentFPS() const {
    // Calculate FPS based on recent frame processing
    auto now = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(timingMutex_);
    
    if (preprocessingTimes_.empty()) {
        return 0.0f;
    }
    
    // Calculate average time per frame
    int64_t totalTime = 0;
    for (auto time : preprocessingTimes_) {
        totalTime += time;
    }
    
    float avgTimePerFrame = static_cast<float>(totalTime) / preprocessingTimes_.size();
    
    // Convert to FPS
    return avgTimePerFrame > 0 ? 1000000.0f / avgTimePerFrame : 0.0f;
}

void Pipeline::updateConfig(const PipelineConfig& config) {
    // Update configuration (thread-safe)
    std::lock_guard<std::mutex> lock(configMutex_);
    config_ = config;
    
    // Apply configuration changes
    if (videoInput_) {
        videoInput_->setResolution(config.targetWidth, config.targetHeight);
        videoInput_->setFPS(config.targetFPS);
    }
    
    if (preprocessing_) {
        preprocessing_->setTargetResolution(config.targetWidth, config.targetHeight);
        preprocessing_->setEnhancementLevel(config.enhancementLevel);
        preprocessing_->setNoiseReduction(config.noiseReduction);
        preprocessing_->setHistogramEqualization(config.histogramEqualization);
    }
    
    if (detection_) {
        detection_->setConfidenceThreshold(config.confidenceThreshold);
        detection_->setNMSThreshold(config.nmsThreshold);
        detection_->setMaxDetections(config.maxDetections);
    }
}

PipelineConfig Pipeline::getConfig() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
} 