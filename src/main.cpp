#include "core/Pipeline.hpp"
#include "core/PerformanceMonitor.hpp"
#include "modules/GUIModule.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <iomanip>

// Global flag for graceful shutdown
std::atomic<bool> g_shutdownRequested{false};

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived shutdown signal. Cleaning up..." << std::endl;
    g_shutdownRequested = true;
}

void printUsage(const char* programName) {
    std::cout << "Real-time Car Vision Pipeline\n";
    std::cout << "Optimized for Apple Silicon (M2+)\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "  --video <file>      Use video file as input\n";
    std::cout << "  --camera            Use camera as input (default)\n";
    std::cout << "  --model <path>      Path to Core ML model (default: models/yolov8n_coreml.mlmodel)\n";
    std::cout << "  --gui               Enable GUI mode\n";
    std::cout << "  --width <pixels>    GUI window width (default: 1280)\n";
    std::cout << "  --height <pixels>   GUI window height (default: 720)\n";
    std::cout << "  --fps <value>       Target FPS (default: 50)\n";
    std::cout << "  --quality <level>   Quality level: low, medium, high (default: high)\n";
    std::cout << "  --output <file>     Save output video to file\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --camera --gui\n";
    std::cout << "  " << programName << " --video data/sample_video.mp4 --model models/yolov8n_coreml.mlmodel\n";
    std::cout << "  " << programName << " --camera --fps 60 --quality high\n";
}

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Parse command line arguments
        std::string videoSource = "camera"; // Default to camera
        bool isCamera = true;
        std::string modelPath = "models/yolov8n_coreml.mlmodel";
        std::string outputPath;
        int targetFPS = 50;
        std::string qualityLevel = "high";
        bool enableGUI = false;
        int windowWidth = 1280;
        int windowHeight = 720;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else if (arg == "--video" && i + 1 < argc) {
                videoSource = argv[++i];
                isCamera = false;
            } else if (arg == "--camera") {
                videoSource = "camera";
                isCamera = true;
            } else if (arg == "--model" && i + 1 < argc) {
                modelPath = argv[++i];
            } else if (arg == "--gui") {
                enableGUI = true;
            } else if (arg == "--width" && i + 1 < argc) {
                windowWidth = std::stoi(argv[++i]);
            } else if (arg == "--height" && i + 1 < argc) {
                windowHeight = std::stoi(argv[++i]);
            } else if (arg == "--fps" && i + 1 < argc) {
                targetFPS = std::stoi(argv[++i]);
            } else if (arg == "--quality" && i + 1 < argc) {
                qualityLevel = argv[++i];
            } else if (arg == "--output" && i + 1 < argc) {
                outputPath = argv[++i];
            }
        }
        
        // Set up pipeline configuration
        PipelineConfig config;
        config.videoSource = videoSource;
        config.isCamera = isCamera;
        config.modelPath = modelPath;
        config.targetFPS = targetFPS;
        config.confidenceThreshold = 0.5f;
        config.nmsThreshold = 0.45f;
        config.maxDetections = 100;
        config.enhancementLevel = 1.2f;
        config.noiseReduction = true;
        config.histogramEqualization = false;
        config.bufferPoolSize = 1000;
        config.maxBufferSize = 1024 * 1024; // 1MB
        config.inputBufferSize = 10;
        
        // Adjust quality settings
        if (qualityLevel == "low") {
            config.targetWidth = 960;
            config.targetHeight = 540;
            config.confidenceThreshold = 0.7f;
            config.maxDetections = 50;
        } else if (qualityLevel == "medium") {
            config.targetWidth = 1280;
            config.targetHeight = 720;
            config.confidenceThreshold = 0.6f;
            config.maxDetections = 75;
        } else { // high
            config.targetWidth = 1920;
            config.targetHeight = 1080;
            config.confidenceThreshold = 0.5f;
            config.maxDetections = 100;
        }
        
        // Create and initialize pipeline
        auto pipeline = std::make_shared<Pipeline>();
        if (!pipeline->initialize(config)) {
            throw std::runtime_error("Failed to initialize pipeline");
        }
        
        std::cout << "Pipeline initialized successfully" << std::endl;
        std::cout << "Video source: " << (isCamera ? "Camera" : videoSource) << std::endl;
        std::cout << "Model: " << modelPath << std::endl;
        std::cout << "Resolution: " << config.targetWidth << "x" << config.targetHeight << std::endl;
        std::cout << "Target FPS: " << config.targetFPS << std::endl;
        
        // Initialize GUI if requested
        std::shared_ptr<GUIModule> gui;
        if (enableGUI) {
            gui = std::make_shared<GUIModule>(pipeline);
            if (!gui->initialize(windowWidth, windowHeight, "Real-time Car Vision")) {
                throw std::runtime_error("Failed to initialize GUI: " + gui->getLastError());
            }
            std::cout << "GUI initialized successfully" << std::endl;
        }
        
        // Start pipeline
        pipeline->start();
        std::cout << "Pipeline started" << std::endl;
        
        if (enableGUI) {
            // Run GUI (this will block until GUI is closed)
            std::cout << "Starting GUI..." << std::endl;
            gui->run();
        } else {
            // Command-line mode
            std::cout << "Running in command-line mode" << std::endl;
            std::cout << "Press Ctrl+C to stop" << std::endl;
            
            // Main processing loop
            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t frameCount = 0;
            
            while (!g_shutdownRequested) {
                // Get latest result
                auto result = pipeline->getLatestResult();
                if (result) {
                    frameCount++;
                    
                    // Print detection results every 30 frames
                    if (frameCount % 30 == 0) {
                        auto currentTime = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
                        float fps = (frameCount * 1000.0f) / duration.count();
                        
                        std::cout << "Frame " << frameCount 
                                  << " | FPS: " << fps 
                                  << " | Detections: " << result->detections.size() << std::endl;
                        
                        // Print top detections
                        for (const auto& detection : result->detections) {
                            if (detection.confidence > 0.7f) {
                                std::cout << "  - " << detection.className 
                                          << " (confidence: " << (detection.confidence * 100) << "%)"
                                          << " at [" << detection.bbox.x << "," << detection.bbox.y 
                                          << "," << detection.bbox.width << "," << detection.bbox.height << "]"
                                          << std::endl;
                            }
                        }
                    }
                }
                
                // Small delay to prevent excessive CPU usage
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        
            // Print final statistics
            auto endTime = std::chrono::high_resolution_clock::now();
            auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            float avgFPS = (frameCount * 1000.0f) / totalDuration.count();
            
            std::cout << "\nFinal Statistics:" << std::endl;
            std::cout << "Total frames processed: " << frameCount << std::endl;
            std::cout << "Average FPS: " << avgFPS << std::endl;
            std::cout << "Total runtime: " << (totalDuration.count() / 1000.0f) << " seconds" << std::endl;
            
            // Get pipeline statistics
            auto stats = pipeline->getStats();
            std::cout << "Pipeline latency: " << (stats.averageLatency / 1000.0f) << " ms" << std::endl;
            std::cout << "CPU usage: " << stats.performanceStats.cpuUsage << "%" << std::endl;
            std::cout << "GPU usage: " << stats.performanceStats.gpuUsage << "%" << std::endl;
            std::cout << "Memory usage: " << (stats.performanceStats.memoryUsage / 1024.0f / 1024.0f) << " MB" << std::endl;
        }
        
        std::cout << "\nPipeline shutdown complete" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 