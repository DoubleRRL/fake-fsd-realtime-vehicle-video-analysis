#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iomanip>

struct BenchmarkResult {
    std::string testName;
    double averageLatencyMs;
    double minLatencyMs;
    double maxLatencyMs;
    double averageFPS;
    double totalProcessingTime;
    int totalFrames;
    int totalDetections;
    double averageDetectionsPerFrame;
    double memoryUsageMB;
    double cpuUsagePercent;
    double gpuUsagePercent;
};

struct BenchmarkConfig {
    std::string videoPath;
    std::string modelPath;
    int numFrames = 300; // Test with 300 frames (10 seconds at 30fps)
    int warmupFrames = 30;
    bool exportAnnotatedVideo = true;
    std::string outputVideoPath = "benchmark_output.mp4";
    std::string outputReportPath = "benchmark_report.json";
    int targetFPS = 50;
    std::string qualityLevel = "medium";
};

class BenchmarkRunner {
public:
    BenchmarkRunner();
    ~BenchmarkRunner();
    
    /**
     * Run comprehensive benchmark tests
     * @param config Benchmark configuration
     * @return Benchmark results
     */
    BenchmarkResult runBenchmark(const BenchmarkConfig& config);
    
    /**
     * Run quick performance test (for recruiters)
     * @param config Benchmark configuration
     * @return Quick performance summary
     */
    std::string runQuickTest(const BenchmarkConfig& config);
    
    /**
     * Generate annotated video for demonstration
     * @param config Benchmark configuration
     * @return Path to generated video
     */
    std::string generateDemoVideo(const BenchmarkConfig& config);
    
    /**
     * Export benchmark results to JSON
     * @param result Benchmark result
     * @param outputPath Output file path
     */
    void exportResultsToJSON(const BenchmarkResult& result, const std::string& outputPath);
    
    /**
     * Generate performance summary for README
     * @param result Benchmark result
     * @return Markdown formatted summary
     */
    std::string generatePerformanceSummary(const BenchmarkResult& result);

private:
    // Performance monitoring
    double getCurrentMemoryUsage();
    double getCurrentCPUUsage();
    double getCurrentGPUUsage();
    
    // Video processing helpers
    cv::Mat drawDetections(const cv::Mat& frame, const std::vector<Detection>& detections);
    void addPerformanceOverlay(cv::Mat& frame, const BenchmarkResult& result, int currentFrame);
    
    // System info
    std::string getSystemInfo();
    std::string getHardwareInfo();
}; 