#pragma once

#include <string>
#include <vector>
#include <memory>
#include <CoreML/CoreML.h>
#include <Foundation/Foundation.h>

/**
 * Pure C++ Model Converter for YOLOv8 to Core ML
 * Eliminates Python dependencies for lowest latency deployment
 */
class ModelConverter {
public:
    struct ConversionConfig {
        std::string inputModelPath;
        std::string outputModelPath;
        int inputSize = 640;
        bool quantize = true;
        bool includeNMS = true;
        float confidenceThreshold = 0.5f;
        float nmsThreshold = 0.45f;
        int maxDetections = 100;
    };
    
    struct ModelInfo {
        std::string name;
        std::string version;
        std::string description;
        int inputWidth;
        int inputHeight;
        int inputChannels;
        std::vector<std::string> classNames;
        float confidenceThreshold;
        float nmsThreshold;
        int maxDetections;
    };
    
    ModelConverter();
    ~ModelConverter();
    
    /**
     * Convert YOLOv8 model to Core ML format
     * @param config Conversion configuration
     * @return true if successful, false otherwise
     */
    bool convertModel(const ConversionConfig& config);
    
    /**
     * Download YOLOv8n model if not present
     * @param modelPath Path to save the model
     * @return true if successful, false otherwise
     */
    bool downloadYOLOv8nModel(const std::string& modelPath);
    
    /**
     * Validate Core ML model
     * @param modelPath Path to Core ML model
     * @return Model information if valid
     */
    std::optional<ModelInfo> validateModel(const std::string& modelPath);
    
    /**
     * Get last error message
     * @return Error message
     */
    std::string getLastError() const { return lastError_; }
    
    /**
     * Set compute units for optimization
     * @param useNeuralEngine Enable Neural Engine
     * @param useGPU Enable GPU
     * @param useCPU Enable CPU
     */
    void setComputeUnits(bool useNeuralEngine = true, bool useGPU = false, bool useCPU = true);
    
private:
    std::string lastError_;
    bool useNeuralEngine_;
    bool useGPU_;
    bool useCPU_;
    
    /**
     * Create optimized Core ML model specification
     * @param config Conversion configuration
     * @return Core ML model specification
     */
    MLModel* createOptimizedModelSpec(const ConversionConfig& config);
    
    /**
     * Add preprocessing layers to model
     * @param modelSpec Model specification
     * @param config Conversion configuration
     */
    void addPreprocessingLayers(MLModel* modelSpec, const ConversionConfig& config);
    
    /**
     * Add postprocessing layers to model
     * @param modelSpec Model specification
     * @param config Conversion configuration
     */
    void addPostprocessingLayers(MLModel* modelSpec, const ConversionConfig& config);
    
    /**
     * Apply quantization to model
     * @param modelSpec Model specification
     * @param config Conversion configuration
     */
    void applyQuantization(MLModel* modelSpec, const ConversionConfig& config);
    
    /**
     * Set compute units on model
     * @param modelSpec Model specification
     */
    void setComputeUnits(MLModel* modelSpec);
    
    /**
     * Validate model inputs and outputs
     * @param modelSpec Model specification
     * @return true if valid, false otherwise
     */
    bool validateModelSpec(MLModel* modelSpec);
    
    /**
     * Create model configuration file
     * @param config Conversion configuration
     * @param modelInfo Model information
     */
    void createModelConfig(const ConversionConfig& config, const ModelInfo& modelInfo);
}; 