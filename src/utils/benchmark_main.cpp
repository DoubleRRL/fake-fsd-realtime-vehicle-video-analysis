#include "BenchmarkRunner.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

void printUsage(const char* programName) {
    std::cout << "Real-time Vehicle Detection Benchmark Tool\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --video <path>        Input video file (required)\n";
    std::cout << "  --model <path>        Core ML model path (default: models/yolov8n_optimized.mlmodel)\n";
    std::cout << "  --frames <n>          Number of frames to test (default: 300)\n";
    std::cout << "  --warmup <n>          Warmup frames (default: 30)\n";
    std::cout << "  --output <path>       Output video path (default: benchmark_output.mp4)\n";
    std::cout << "  --report <path>       JSON report path (default: benchmark_report.json)\n";
    std::cout << "  --quality <level>     Quality: low, medium, high (default: medium)\n";
    std::cout << "  --fps <value>         Target FPS (default: 50)\n";
    std::cout << "  --quick               Run quick test (100 frames)\n";
    std::cout << "  --demo                Generate demo video only\n";
    std::cout << "  --no-video            Don't export annotated video\n";
    std::cout << "  --help                Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --video data/sample.mp4 --quick\n";
    std::cout << "  " << programName << " --video data/sample.mp4 --demo --output demo.mp4\n";
    std::cout << "  " << programName << " --video data/sample.mp4 --frames 1000 --quality high\n";
}

int main(int argc, char* argv[]) {
    BenchmarkRunner runner;
    BenchmarkConfig config;
    
    // Parse command line arguments
    bool quickTest = false;
    bool demoOnly = false;
    bool noVideo = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--video" && i + 1 < argc) {
            config.videoPath = argv[++i];
        } else if (arg == "--model" && i + 1 < argc) {
            config.modelPath = argv[++i];
        } else if (arg == "--frames" && i + 1 < argc) {
            config.numFrames = std::stoi(argv[++i]);
        } else if (arg == "--warmup" && i + 1 < argc) {
            config.warmupFrames = std::stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            config.outputVideoPath = argv[++i];
        } else if (arg == "--report" && i + 1 < argc) {
            config.outputReportPath = argv[++i];
        } else if (arg == "--quality" && i + 1 < argc) {
            config.qualityLevel = argv[++i];
        } else if (arg == "--fps" && i + 1 < argc) {
            config.targetFPS = std::stoi(argv[++i]);
        } else if (arg == "--quick") {
            quickTest = true;
        } else if (arg == "--demo") {
            demoOnly = true;
        } else if (arg == "--no-video") {
            noVideo = true;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Validate required arguments
    if (config.videoPath.empty()) {
        std::cerr << "Error: Video path is required. Use --video <path>" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // Set default model path if not provided
    if (config.modelPath.empty()) {
        config.modelPath = "models/yolov8n_optimized.mlmodel";
    }
    
    // Configure video export
    if (noVideo) {
        config.exportAnnotatedVideo = false;
    }
    
    std::cout << "=== Real-time Vehicle Detection Benchmark ===\n";
    std::cout << "Video: " << config.videoPath << "\n";
    std::cout << "Model: " << config.modelPath << "\n";
    std::cout << "Quality: " << config.qualityLevel << "\n";
    std::cout << "Target FPS: " << config.targetFPS << "\n";
    
    if (quickTest) {
        std::cout << "Mode: Quick Test (100 frames)\n";
    } else if (demoOnly) {
        std::cout << "Mode: Demo Video Generation\n";
    } else {
        std::cout << "Mode: Full Benchmark (" << config.numFrames << " frames)\n";
    }
    std::cout << std::endl;
    
    try {
        if (quickTest) {
            // Run quick performance test
            std::string summary = runner.runQuickTest(config);
            std::cout << summary << std::endl;
            
            // Save quick test results
            BenchmarkConfig quickConfig = config;
            quickConfig.numFrames = 100;
            quickConfig.warmupFrames = 10;
            quickConfig.exportAnnotatedVideo = false;
            
            BenchmarkResult result = runner.runBenchmark(quickConfig);
            runner.exportResultsToJSON(result, "quick_test_results.json");
            
        } else if (demoOnly) {
            // Generate demo video
            std::string videoPath = runner.generateDemoVideo(config);
            std::cout << "Demo video generated: " << videoPath << std::endl;
            
        } else {
            // Run full benchmark
            BenchmarkResult result = runner.runBenchmark(config);
            
            // Export results
            runner.exportResultsToJSON(result, config.outputReportPath);
            
            // Generate performance summary
            std::string summary = runner.generatePerformanceSummary(result);
            std::cout << "\n" << summary << std::endl;
            
            // Save summary to file
            std::ofstream summaryFile("performance_summary.md");
            summaryFile << summary;
            summaryFile.close();
            
            std::cout << "Results saved to:\n";
            std::cout << "  - " << config.outputReportPath << " (JSON)\n";
            std::cout << "  - performance_summary.md (Markdown)\n";
            if (config.exportAnnotatedVideo) {
                std::cout << "  - " << config.outputVideoPath << " (Annotated Video)\n";
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        return 1;
    }
} 