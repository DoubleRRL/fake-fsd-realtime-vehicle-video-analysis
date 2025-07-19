#pragma once

#include "Types.hpp"
#include "BufferPool.hpp"
#include "PerformanceMonitor.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <optional>

// Forward declarations
namespace RealTimeVideoAnalysis {
    class VideoInputModule;
    class PreprocessingModule;
    class DetectionModule;
    class TrackingModule;
    class LabelingModule;
    class PredictionModule;
    class RenderingModule;
    class GUIModule;
}

namespace RealTimeVideoAnalysis {

/**
 * @brief High-performance parallel processing pipeline
 * 
 * Implements producer-consumer pattern with multiple stages running
 * concurrently. Achieves <20ms latency through parallel execution.
 */
class Pipeline {
private:
    // Pipeline stages
    std::unique_ptr<VideoInputModule> videoInput;
    std::unique_ptr<PreprocessingModule> preprocessing;
    std::unique_ptr<DetectionModule> detection;
    std::unique_ptr<TrackingModule> tracking;
    std::unique_ptr<LabelingModule> labeling;
    std::unique_ptr<PredictionModule> prediction;
    std::unique_ptr<RenderingModule> rendering;
    
    // Parallel processing threads
    std::thread inputThread;
    std::thread preprocessingThread;
    std::thread detectionThread;
    std::thread trackingThread;
    std::thread renderingThread;
    
    // Lock-free queues for inter-thread communication
    struct alignas(64) FrameData {
        CVPixelBufferRef pixelBuffer;
        int frameId;
        std::chrono::high_resolution_clock::time_point timestamp;
        std::vector<Detection> detections;
        std::vector<Track> tracks;
        std::vector<ActionLabel> labels;
        std::vector<TrackPrediction> predictions;
        double processingTime;
        bool success;
        
        FrameData() : pixelBuffer(nullptr), frameId(-1), processingTime(0.0), success(false) {}
    };
    
    // Circular buffers for zero-copy data transfer
    static constexpr int BUFFER_SIZE = 8;
    std::array<FrameData, BUFFER_SIZE> frameBuffer;
    std::atomic<int> inputIndex{0};
    std::atomic<int> outputIndex{0};
    
    // Performance monitoring
    std::atomic<double> currentFPS{0.0};
    std::atomic<double> averageLatency{0.0};
    PerformanceMonitor monitor;
    
    // Thread synchronization
    std::atomic<bool> isRunning{false};
    std::mutex processingMutex;
    std::condition_variable processingCV;
    
    // Pipeline configuration
    PipelineConfig config;
    
    // Buffer pool
    std::unique_ptr<BufferPool> bufferPool;
    
    // Metal device
    id<MTLDevice> metalDevice;
    
    // Latest result for synchronous access
    std::mutex resultMutex;
    std::optional<PipelineResult> latestResult;
    
public:
    /**
     * @brief Initialize parallel pipeline
     * @param config Pipeline configuration
     * @return Initialization success
     */
    bool initialize(const PipelineConfig& config);
    
    /**
     * @brief Initialize video input
     * @param inputPath Path to video file or camera device
     * @return Initialization success
     */
    bool initializeVideoInput(const std::string& inputPath);
    
    /**
     * @brief Start parallel processing threads
     */
    void startProcessing();
    
    /**
     * @brief Stop parallel processing
     */
    void stopProcessing();
    
    /**
     * @brief Process single frame (for synchronous mode)
     * @return Complete processing result
     */
    PipelineResult processFrame();
    
    /**
     * @brief Get latest processed frame (for asynchronous mode)
     * @return Latest result if available
     */
    std::optional<PipelineResult> getLatestResult();
    
    /**
     * @brief Check if more frames are available
     * @return True if frames available, false if end of stream
     */
    bool hasNextFrame() const;
    
    /**
     * @brief Update playback controls
     * @param controls New playback control settings
     */
    void updatePlaybackControls(const GUIUpdateResult::PlaybackControls& controls);
    
    /**
     * @brief Get current performance metrics
     * @return Performance metrics
     */
    PerformanceMetrics getPerformanceMetrics() const;
    
    /**
     * @brief Get pipeline configuration
     * @return Current configuration
     */
    const PipelineConfig& getConfig() const { return config; }
    
    /**
     * @brief Update pipeline configuration
     * @param newConfig New configuration
     */
    void updateConfig(const PipelineConfig& newConfig);
    
    /**
     * @brief Shutdown pipeline and cleanup resources
     */
    void shutdown();
    
    /**
     * @brief Get buffer pool statistics
     * @return Buffer pool stats
     */
    BufferPoolStats getBufferPoolStats() const;
    
private:
    /**
     * @brief Input processing thread function
     */
    void inputThreadFunction();
    
    /**
     * @brief Preprocessing thread function
     */
    void preprocessingThreadFunction();
    
    /**
     * @brief Detection thread function
     */
    void detectionThreadFunction();
    
    /**
     * @brief Tracking and labeling thread function
     */
    void trackingThreadFunction();
    
    /**
     * @brief Rendering thread function
     */
    void renderingThreadFunction();
    
    /**
     * @brief Initialize Metal device
     * @return Success status
     */
    bool initializeMetalDevice();
    
    /**
     * @brief Initialize buffer pool
     * @return Success status
     */
    bool initializeBufferPool();
    
    /**
     * @brief Process frame through all stages
     * @param frameData Frame data to process
     * @return Processing result
     */
    PipelineResult processFrameStages(FrameData& frameData);
    
    /**
     * @brief Update latest result
     * @param result New result
     */
    void updateLatestResult(const PipelineResult& result);
    
    /**
     * @brief Get next available buffer slot
     * @return Buffer index or -1 if full
     */
    int getNextBufferSlot();
    
    /**
     * @brief Get next available result slot
     * @return Result index or -1 if none available
     */
    int getNextResultSlot();
    
    /**
     * @brief Wait for available buffer slot
     * @return Buffer index
     */
    int waitForBufferSlot();
    
    /**
     * @brief Wait for available result
     * @return Result index or -1 if timeout
     */
    int waitForResult(int timeoutMs = 100);
};

} // namespace RealTimeVideoAnalysis 