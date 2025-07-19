#pragma once

#include "Types.hpp"
#include <deque>
#include <mutex>
#include <atomic>
#include <chrono>

namespace RealTimeVideoAnalysis {

/**
 * @brief Performance monitoring and metrics collection
 * 
 * Tracks real-time performance metrics including FPS, latency,
 * memory usage, and thermal status for the video processing pipeline.
 */
class PerformanceMonitor {
private:
    // Frame timing history
    std::deque<double> frameTimes;
    std::deque<double> latencyHistory;
    std::deque<double> fpsHistory;
    
    // Performance counters
    std::atomic<int> frameCount{0};
    std::atomic<double> totalProcessingTime{0.0};
    std::atomic<double> peakLatency{0.0};
    std::atomic<double> averageLatency{0.0};
    std::atomic<double> currentFPS{0.0};
    
    // Memory tracking
    std::atomic<double> currentMemoryUsage{0.0};
    std::atomic<double> peakMemoryUsage{0.0};
    
    // CPU and GPU usage
    std::atomic<double> currentCPUUsage{0.0};
    std::atomic<double> currentGPUUsage{0.0};
    
    // Timing
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    
    // Configuration
    static constexpr size_t MAX_HISTORY_SIZE = 1000;
    static constexpr double FPS_UPDATE_INTERVAL = 1.0; // seconds
    
    // Thread safety
    mutable std::mutex metricsMutex;
    
public:
    /**
     * @brief Initialize performance monitor
     */
    PerformanceMonitor();
    
    /**
     * @brief Start performance monitoring
     */
    void start();
    
    /**
     * @brief Record frame processing time
     * @param processingTime Processing time in milliseconds
     */
    void recordFrameTime(double processingTime);
    
    /**
     * @brief Record memory usage
     * @param memoryUsage Memory usage in MB
     */
    void recordMemoryUsage(double memoryUsage);
    
    /**
     * @brief Record CPU usage
     * @param cpuUsage CPU usage percentage
     */
    void recordCPUUsage(double cpuUsage);
    
    /**
     * @brief Record GPU usage
     * @param gpuUsage GPU usage percentage
     */
    void recordGPUUsage(double gpuUsage);
    
    /**
     * @brief Get current performance metrics
     * @return Complete performance metrics
     */
    PerformanceMetrics getMetrics() const;
    
    /**
     * @brief Get average FPS
     * @return Average FPS over the monitoring period
     */
    double getAverageFPS() const;
    
    /**
     * @brief Get average latency
     * @return Average latency in milliseconds
     */
    double getAverageLatency() const;
    
    /**
     * @brief Get peak latency
     * @return Peak latency in milliseconds
     */
    double getPeakLatency() const;
    
    /**
     * @brief Get current FPS
     * @return Current FPS
     */
    double getCurrentFPS() const;
    
    /**
     * @brief Get frame count
     * @return Total number of frames processed
     */
    int getFrameCount() const;
    
    /**
     * @brief Get total processing time
     * @return Total processing time in seconds
     */
    double getTotalProcessingTime() const;
    
    /**
     * @brief Get memory usage
     * @return Current memory usage in MB
     */
    double getMemoryUsage() const;
    
    /**
     * @brief Get CPU usage
     * @return Current CPU usage percentage
     */
    double getCPUUsage() const;
    
    /**
     * @brief Get GPU usage
     * @return Current GPU usage percentage
     */
    double getGPUUsage() const;
    
    /**
     * @brief Get performance history
     * @return Vector of recent performance data
     */
    std::vector<double> getLatencyHistory() const;
    std::vector<double> getFPSHistory() const;
    
    /**
     * @brief Reset performance counters
     */
    void reset();
    
    /**
     * @brief Check if performance targets are met
     * @param targetFPS Target FPS
     * @param maxLatency Maximum allowed latency in ms
     * @return True if targets are met
     */
    bool checkPerformanceTargets(double targetFPS, double maxLatency) const;
    
    /**
     * @brief Get performance summary
     * @return Formatted performance summary string
     */
    std::string getPerformanceSummary() const;
    
private:
    /**
     * @brief Update FPS calculation
     */
    void updateFPS();
    
    /**
     * @brief Update average latency
     */
    void updateAverageLatency();
    
    /**
     * @brief Get system memory usage
     * @return Current system memory usage in MB
     */
    double getSystemMemoryUsage() const;
    
    /**
     * @brief Get system CPU usage
     * @return Current system CPU usage percentage
     */
    double getSystemCPUUsage() const;
    
    /**
     * @brief Get system GPU usage
     * @return Current system GPU usage percentage
     */
    double getSystemGPUUsage() const;
};

} // namespace RealTimeVideoAnalysis 