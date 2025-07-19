#include "ModelConverter.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

void printUsage(const char* programName) {
    std::cout << "C++ Model Converter for YOLOv8 to Core ML\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --input <path>       Input model path (optional, will create minimal model)\n";
    std::cout << "  --output <path>      Output Core ML model path (default: yolov8n_optimized.mlmodel)\n";
    std::cout << "  --size <pixels>      Input size (default: 640)\n";
    std::cout << "  --no-quantize        Disable INT8 quantization\n";
    std::cout << "  --no-nms             Disable NMS layer\n";
    std::cout << "  --confidence <value> Confidence threshold (default: 0.5)\n";
    std::cout << "  --nms <value>        NMS threshold (default: 0.45)\n";
    std::cout << "  --max-detections <n> Maximum detections (default: 100)\n";
    std::cout << "  --neural-engine      Enable Neural Engine (default: true)\n";
    std::cout << "  --gpu                Enable GPU\n";
    std::cout << "  --cpu-only           Use CPU only\n";
    std::cout << "  --help               Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --output models/yolov8n_optimized.mlmodel\n";
    std::cout << "  " << programName << " --input yolov8n.pt --output yolov8n_coreml.mlmodel --size 416\n";
    std::cout << "  " << programName << " --output model.mlmodel --no-quantize --cpu-only\n";
}

int main(int argc, char* argv[]) {
    ModelConverter converter;
    ModelConverter::ConversionConfig config;
    
    // Default values
    config.outputModelPath = "yolov8n_optimized.mlmodel";
    config.inputSize = 640;
    config.quantize = true;
    config.includeNMS = true;
    config.confidenceThreshold = 0.5f;
    config.nmsThreshold = 0.45f;
    config.maxDetections = 100;
    
    bool useNeuralEngine = true;
    bool useGPU = false;
    bool useCPU = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--input" && i + 1 < argc) {
            config.inputModelPath = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            config.outputModelPath = argv[++i];
        } else if (arg == "--size" && i + 1 < argc) {
            config.inputSize = std::stoi(argv[++i]);
        } else if (arg == "--no-quantize") {
            config.quantize = false;
        } else if (arg == "--no-nms") {
            config.includeNMS = false;
        } else if (arg == "--confidence" && i + 1 < argc) {
            config.confidenceThreshold = std::stof(argv[++i]);
        } else if (arg == "--nms" && i + 1 < argc) {
            config.nmsThreshold = std::stof(argv[++i]);
        } else if (arg == "--max-detections" && i + 1 < argc) {
            config.maxDetections = std::stoi(argv[++i]);
        } else if (arg == "--neural-engine") {
            useNeuralEngine = true;
            useGPU = false;
            useCPU = true;
        } else if (arg == "--gpu") {
            useGPU = true;
            useCPU = true;
        } else if (arg == "--cpu-only") {
            useNeuralEngine = false;
            useGPU = false;
            useCPU = true;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Set compute units
    converter.setComputeUnits(useNeuralEngine, useGPU, useCPU);
    
    // Validate configuration
    if (config.inputSize % 32 != 0) {
        std::cerr << "Error: Input size must be a multiple of 32" << std::endl;
        return 1;
    }
    
    if (config.confidenceThreshold < 0.0f || config.confidenceThreshold > 1.0f) {
        std::cerr << "Error: Confidence threshold must be between 0.0 and 1.0" << std::endl;
        return 1;
    }
    
    if (config.nmsThreshold < 0.0f || config.nmsThreshold > 1.0f) {
        std::cerr << "Error: NMS threshold must be between 0.0 and 1.0" << std::endl;
        return 1;
    }
    
    if (config.maxDetections <= 0) {
        std::cerr << "Error: Max detections must be positive" << std::endl;
        return 1;
    }
    
    // Print configuration
    std::cout << "Model Converter Configuration:" << std::endl;
    std::cout << "  Input: " << (config.inputModelPath.empty() ? "Create minimal model" : config.inputModelPath) << std::endl;
    std::cout << "  Output: " << config.outputModelPath << std::endl;
    std::cout << "  Input size: " << config.inputSize << "x" << config.inputSize << std::endl;
    std::cout << "  Quantize: " << (config.quantize ? "Yes" : "No") << std::endl;
    std::cout << "  Include NMS: " << (config.includeNMS ? "Yes" : "No") << std::endl;
    std::cout << "  Confidence threshold: " << config.confidenceThreshold << std::endl;
    std::cout << "  NMS threshold: " << config.nmsThreshold << std::endl;
    std::cout << "  Max detections: " << config.maxDetections << std::endl;
    std::cout << "  Compute units: ";
    if (useNeuralEngine && useGPU) {
        std::cout << "CPU + GPU + Neural Engine";
    } else if (useNeuralEngine) {
        std::cout << "CPU + Neural Engine";
    } else if (useGPU) {
        std::cout << "CPU + GPU";
    } else {
        std::cout << "CPU only";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    
    // Convert model
    std::cout << "Starting model conversion..." << std::endl;
    
    bool success = converter.convertModel(config);
    
    if (success) {
        std::cout << "Model conversion completed successfully!" << std::endl;
        std::cout << "Output model: " << config.outputModelPath << std::endl;
        
        // Validate the converted model
        std::cout << "Validating converted model..." << std::endl;
        auto modelInfo = converter.validateModel(config.outputModelPath);
        if (modelInfo) {
            std::cout << "Model validation successful!" << std::endl;
            std::cout << "  Name: " << modelInfo->name << std::endl;
            std::cout << "  Version: " << modelInfo->version << std::endl;
            std::cout << "  Input: " << modelInfo->inputWidth << "x" << modelInfo->inputHeight << "x" << modelInfo->inputChannels << std::endl;
            std::cout << "  Classes: " << modelInfo->classNames.size() << std::endl;
        } else {
            std::cout << "Warning: Model validation failed: " << converter.getLastError() << std::endl;
        }
        
        return 0;
    } else {
        std::cerr << "Model conversion failed: " << converter.getLastError() << std::endl;
        return 1;
    }
} 