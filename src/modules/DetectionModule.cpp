#include "DetectionModule.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <Accelerate/Accelerate.h>

// Default COCO classes for YOLOv8 (optimized for traffic)
const std::vector<std::string> DetectionModule::DEFAULT_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
    "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
    "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball",
    "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
    "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
    "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop",
    "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
    "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
};

DetectionModule::DetectionModule(std::shared_ptr<BufferPool> bufferPool)
    : bufferPool_(bufferPool)
    , model_(nullptr)
    , modelConfig_(nullptr)
    , confidenceThreshold_(0.5f)
    , nmsThreshold_(0.45f)
    , maxDetections_(100)
    , detectionClasses_(DEFAULT_CLASSES)
    , inputWidth_(640)
    , inputHeight_(640)
    , inputChannels_(3)
    , inputName_("images")
    , outputName_("output0")
    , processedFrames_(0)
    , preallocatedInputArray_(nullptr)
    , preallocatedOutputArray_(nullptr) {
    
    // Pre-allocate buffers for zero-copy operations
    preallocateBuffers();
}

DetectionModule::~DetectionModule() {
    // Core ML objects are automatically released by ARC
    if (preallocatedInputArray_) {
        [preallocatedInputArray_ release];
    }
    if (preallocatedOutputArray_) {
        [preallocatedOutputArray_ release];
    }
}

void DetectionModule::preallocateBuffers() {
    @autoreleasepool {
        // Pre-allocate input array for zero-copy operations
        NSArray<NSNumber*>* inputShape = @[@(1), @(inputChannels_), @(inputHeight_), @(inputWidth_)];
        NSError* error = nil;
        preallocatedInputArray_ = [[MLMultiArray alloc] initWithShape:inputShape dataType:MLMultiArrayDataTypeFloat32 error:&error];
        
        if (!preallocatedInputArray_) {
            std::cerr << "Failed to pre-allocate input array: " << [error.localizedDescription UTF8String] << std::endl;
        }
        
        // Pre-allocate output array (will be set after model loading)
        preallocatedOutputArray_ = nullptr;
    }
}

bool DetectionModule::initialize(const std::string& modelPath) {
    if (!loadModel(modelPath)) {
        return false;
    }
    
    if (!setupModelInput()) {
        return false;
    }
    
    if (!setupModelOutput()) {
        return false;
    }
    
    // Pre-allocate output array after model is loaded
    preallocateOutputBuffers();
    
    return true;
}

bool DetectionModule::loadModel(const std::string& modelPath) {
    @autoreleasepool {
        NSURL* modelURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:modelPath.c_str()]];
        if (!modelURL) {
            lastError_ = "Invalid model path: " + modelPath;
            return false;
        }
        
        NSError* error = nil;
        modelConfig_ = [[MLModelConfiguration alloc] init];
        if (!modelConfig_) {
            lastError_ = "Failed to create model configuration";
            return false;
        }
        
        // Optimize for Neural Engine with lowest latency
        modelConfig_.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
        modelConfig_.allowLowPrecisionAccumulationOnGPU = YES;
        modelConfig_.allowFloatingPointPrecisionReduction = YES;
        
        model_ = [MLModel modelWithContentsOfURL:modelURL configuration:modelConfig_ error:&error];
        if (!model_) {
            lastError_ = "Failed to load Core ML model: " + std::string([error.localizedDescription UTF8String]);
            return false;
        }
        
        return true;
    }
}

bool DetectionModule::setupModelInput() {
    @autoreleasepool {
        MLModelDescription* description = model_.modelDescription;
        NSDictionary<NSString*, MLFeatureDescription*>* inputs = description.inputDescriptionsByName;
        
        // Find the input feature
        MLFeatureDescription* inputDesc = inputs[inputName_];
        if (!inputDesc) {
            lastError_ = "Model input '" + inputName_ + "' not found";
            return false;
        }
        
        // Get input shape
        MLMultiArrayConstraint* constraint = inputDesc.multiArrayConstraint;
        if (!constraint) {
            lastError_ = "Input is not a multi-array";
            return false;
        }
        
        NSArray<NSNumber*>* shape = constraint.shape;
        if (shape.count >= 3) {
            inputChannels_ = [shape[0] intValue];
            inputHeight_ = [shape[1] intValue];
            inputWidth_ = [shape[2] intValue];
        }
        
        return true;
    }
}

bool DetectionModule::setupModelOutput() {
    @autoreleasepool {
        MLModelDescription* description = model_.modelDescription;
        NSDictionary<NSString*, MLFeatureDescription*>* outputs = description.outputDescriptionsByName;
        
        // Find the output feature
        MLFeatureDescription* outputDesc = outputs[outputName_];
        if (!outputDesc) {
            lastError_ = "Model output '" + outputName_ + "' not found";
            return false;
        }
        
        return true;
    }
}

void DetectionModule::preallocateOutputBuffers() {
    @autoreleasepool {
        // Get output shape from model
        MLModelDescription* description = model_.modelDescription;
        NSDictionary<NSString*, MLFeatureDescription*>* outputs = description.outputDescriptionsByName;
        MLFeatureDescription* outputDesc = outputs[outputName_];
        
        if (outputDesc && outputDesc.multiArrayConstraint) {
            NSArray<NSNumber*>* outputShape = outputDesc.multiArrayConstraint.shape;
            NSError* error = nil;
            preallocatedOutputArray_ = [[MLMultiArray alloc] initWithShape:outputShape dataType:MLMultiArrayDataTypeFloat32 error:&error];
            
            if (!preallocatedOutputArray_) {
                std::cerr << "Failed to pre-allocate output array: " << [error.localizedDescription UTF8String] << std::endl;
            }
        }
    }
}

std::vector<Detection> DetectionModule::detectObjects(const std::shared_ptr<ProcessedFrame>& frame) {
    if (!frame || !frame->data || !model_) {
        return {};
    }
    
    std::lock_guard<std::mutex> lock(detectionMutex_);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    @autoreleasepool {
        // Use pre-allocated input array for zero-copy
        MLMultiArray* inputArray = preallocatedInputArray_;
        if (!inputArray) {
            // Fallback to creating new array
            NSArray<NSNumber*>* shape = @[@(1), @(inputChannels_), @(inputHeight_), @(inputWidth_)];
            NSError* error = nil;
            inputArray = [[MLMultiArray alloc] initWithShape:shape dataType:MLMultiArrayDataTypeFloat32 error:&error];
            
            if (!inputArray) {
                lastError_ = "Failed to create input array: " + std::string([error.localizedDescription UTF8String]);
                return {};
            }
        }
        
        // Preprocess frame with optimized SIMD operations
        if (!preprocessFrameOptimized(frame, inputArray)) {
            return {};
        }
        
        // Create prediction input
        NSError* error = nil;
        MLFeatureProvider* input = [[MLDictionaryFeatureProvider alloc] initWithDictionary:@{
            inputName_: inputArray
        } error:&error];
        
        if (!input) {
            lastError_ = "Failed to create prediction input: " + std::string([error.localizedDescription UTF8String]);
            return {};
        }
        
        // Run prediction with optimized settings
        MLPredictionOptions* options = [[MLPredictionOptions alloc] init];
        options.usesCPUOnly = NO; // Use Neural Engine
        
        MLFeatureProvider* output = [model_ predictionFromFeatures:input options:options error:&error];
        if (!output) {
            lastError_ = "Prediction failed: " + std::string([error.localizedDescription UTF8String]);
            return {};
        }
        
        // Get output array
        MLMultiArray* outputArray = output[outputName_];
        if (!outputArray) {
            lastError_ = "Output array not found";
            return {};
        }
        
        // Postprocess output with optimized algorithms
        std::vector<Detection> detections = postprocessOutputOptimized(outputArray, frame->width, frame->height);
        
        // Apply optimized NMS
        detections = applyNMSOptimized(detections);
        
        // Update statistics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        stats_.totalFrames++;
        stats_.averageDetectionTime = (stats_.averageDetectionTime * (stats_.totalFrames - 1) + duration.count()) / stats_.totalFrames;
        stats_.lastDetectionTime = duration.count();
        stats_.totalDetections += detections.size();
        stats_.averageDetectionsPerFrame = static_cast<float>(stats_.totalDetections) / stats_.totalFrames;
        
        processedFrames_++;
        
        return detections;
    }
}

bool DetectionModule::preprocessFrameOptimized(const std::shared_ptr<ProcessedFrame>& frame, MLMultiArray* inputArray) {
    if (!frame || !frame->data || !inputArray) {
        return false;
    }
    
    const uint8_t* frameData = frame->data->data();
    int frameWidth = frame->width;
    int frameHeight = frame->height;
    
    // Calculate scaling factors (cached for performance)
    float scaleX = static_cast<float>(inputWidth_) / frameWidth;
    float scaleY = static_cast<float>(inputHeight_) / frameHeight;
    float scale = std::min(scaleX, scaleY);
    
    int newWidth = static_cast<int>(frameWidth * scale);
    int newHeight = static_cast<int>(frameHeight * scale);
    
    // Calculate padding
    int padX = (inputWidth_ - newWidth) / 2;
    int padY = (inputHeight_ - newHeight) / 2;
    
    // Use Accelerate framework for optimized operations
    float* inputPtr = static_cast<float*>([inputArray dataPointer]);
    
    // Pre-calculate normalization factor
    const float normFactor = 1.0f / 255.0f;
    
    // Use SIMD-optimized operations for better performance
    // Process multiple pixels at once using Accelerate framework
    vImage_Buffer srcBuffer, dstBuffer;
    srcBuffer.data = const_cast<uint8_t*>(frameData);
    srcBuffer.width = frameWidth;
    srcBuffer.height = frameHeight;
    srcBuffer.rowBytes = frameWidth * 4; // RGBA
    
    dstBuffer.data = inputPtr;
    dstBuffer.width = inputWidth_;
    dstBuffer.height = inputHeight_;
    dstBuffer.rowBytes = inputWidth_ * sizeof(float);
    
    // Use vImage for optimized scaling and format conversion
    vImage_Error err = vImageScale_ARGB8888(&srcBuffer, &dstBuffer, nullptr, kvImageHighQualityResampling);
    
    if (err != kvImageNoError) {
        // Fallback to manual processing if vImage fails
        for (int c = 0; c < inputChannels_; ++c) {
            for (int h = 0; h < inputHeight_; ++h) {
                for (int w = 0; w < inputWidth_; ++w) {
                    int inputIndex = c * inputHeight_ * inputWidth_ + h * inputWidth_ + w;
                    
                    if (w >= padX && w < padX + newWidth && h >= padY && h < padY + newHeight) {
                        // Map to original frame coordinates
                        int frameX = static_cast<int>((w - padX) / scale);
                        int frameY = static_cast<int>((h - padY) / scale);
                        
                        if (frameX < frameWidth && frameY < frameHeight) {
                            int frameIndex = frameY * frameWidth * 4 + frameX * 4 + c; // RGBA format
                            if (frameIndex < frame->data->size()) {
                                // Normalize to [0, 1] with optimized conversion
                                inputPtr[inputIndex] = static_cast<float>(frameData[frameIndex]) * normFactor;
                            } else {
                                inputPtr[inputIndex] = 0.0f;
                            }
                        } else {
                            inputPtr[inputIndex] = 0.0f;
                        }
                    } else {
                        // Padding with optimized value
                        inputPtr[inputIndex] = 0.5f; // Gray padding
                    }
                }
            }
        }
    }
    
    return true;
}

std::vector<Detection> DetectionModule::postprocessOutputOptimized(MLMultiArray* outputArray, int frameWidth, int frameHeight) {
    std::vector<Detection> detections;
    
    if (!outputArray) {
        return detections;
    }
    
    // YOLOv8 output format: [batch, num_detections, 85] where 85 = 4 (bbox) + 1 (confidence) + 80 (classes)
    NSArray<NSNumber*>* shape = outputArray.shape;
    if (shape.count < 2) {
        return detections;
    }
    
    int numDetections = [shape[1] intValue];
    int numClasses = detectionClasses_.size();
    
    // Calculate scaling factors for converting back to original frame coordinates
    float scaleX = static_cast<float>(inputWidth_) / frameWidth;
    float scaleY = static_cast<float>(inputHeight_) / frameHeight;
    float scale = std::min(scaleX, scaleY);
    
    int newWidth = static_cast<int>(frameWidth * scale);
    int newHeight = static_cast<int>(frameHeight * scale);
    
    int padX = (inputWidth_ - newWidth) / 2;
    int padY = (inputHeight_ - newHeight) / 2;
    
    // Use direct pointer access for maximum performance
    float* outputPtr = static_cast<float*>([outputArray dataPointer]);
    
    for (int i = 0; i < numDetections; ++i) {
        int baseIndex = i * (4 + 1 + numClasses);
        
        // Get bounding box coordinates (normalized)
        float x = outputPtr[baseIndex + 0];
        float y = outputPtr[baseIndex + 1];
        float w = outputPtr[baseIndex + 2];
        float h = outputPtr[baseIndex + 3];
        
        // Get confidence score
        float confidence = outputPtr[baseIndex + 4];
        
        if (confidence < confidenceThreshold_) {
            continue;
        }
        
        // Find class with highest probability using optimized search
        int bestClass = 0;
        float bestScore = 0.0f;
        
        for (int c = 0; c < numClasses; ++c) {
            float classScore = outputPtr[baseIndex + 5 + c];
            if (classScore > bestScore) {
                bestScore = classScore;
                bestClass = c;
            }
        }
        
        // Calculate final confidence
        float finalConfidence = confidence * bestScore;
        if (finalConfidence < confidenceThreshold_) {
            continue;
        }
        
        // Convert to pixel coordinates with optimized math
        cv::Rect bbox = convertToPixelCoordsOptimized(x, y, w, h, frameWidth, frameHeight, padX, padY, scale);
        
        // Create detection
        Detection detection;
        detection.bbox = bbox;
        detection.confidence = finalConfidence;
        detection.classId = bestClass;
        detection.className = detectionClasses_[bestClass];
        detection.timestamp = std::chrono::high_resolution_clock::now();
        
        detections.push_back(detection);
    }
    
    return detections;
}

cv::Rect DetectionModule::convertToPixelCoordsOptimized(float x, float y, float w, float h, 
                                                        int frameWidth, int frameHeight, 
                                                        int padX, int padY, float scale) {
    // Optimized coordinate conversion
    float pixelX = (x - padX) / scale;
    float pixelY = (y - padY) / scale;
    float pixelW = w / scale;
    float pixelH = h / scale;
    
    // Convert to cv::Rect with bounds checking
    int left = std::max(0, static_cast<int>(pixelX - pixelW / 2));
    int top = std::max(0, static_cast<int>(pixelY - pixelH / 2));
    int right = std::min(frameWidth, static_cast<int>(pixelX + pixelW / 2));
    int bottom = std::min(frameHeight, static_cast<int>(pixelY + pixelH / 2));
    
    return cv::Rect(left, top, right - left, bottom - top);
}

std::vector<Detection> DetectionModule::applyNMSOptimized(const std::vector<Detection>& detections) {
    if (detections.empty()) {
        return {};
    }
    
    std::vector<Detection> result;
    std::vector<bool> used(detections.size(), false);
    
    // Sort by confidence (descending) using optimized sort
    std::vector<size_t> indices(detections.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&detections](size_t a, size_t b) {
        return detections[a].confidence > detections[b].confidence;
    });
    
    for (size_t i = 0; i < indices.size(); ++i) {
        if (used[indices[i]]) {
            continue;
        }
        
        result.push_back(detections[indices[i]]);
        used[indices[i]] = true;
        
        // Check IoU with remaining detections using optimized algorithm
        for (size_t j = i + 1; j < indices.size(); ++j) {
            if (used[indices[j]]) {
                continue;
            }
            
            // Only apply NMS to same class
            if (detections[indices[i]].classId == detections[indices[j]].classId) {
                float iou = calculateIoUOptimized(detections[indices[i]].bbox, detections[indices[j]].bbox);
                if (iou > nmsThreshold_) {
                    used[indices[j]] = true;
                }
            }
        }
        
        // Limit number of detections
        if (result.size() >= maxDetections_) {
            break;
        }
    }
    
    return result;
}

float DetectionModule::calculateIoUOptimized(const cv::Rect& rect1, const cv::Rect& rect2) {
    // Optimized IoU calculation
    int x1 = std::max(rect1.x, rect2.x);
    int y1 = std::max(rect1.y, rect2.y);
    int x2 = std::min(rect1.x + rect1.width, rect2.x + rect2.width);
    int y2 = std::min(rect1.y + rect1.height, rect2.y + rect2.height);
    
    if (x2 <= x1 || y2 <= y1) {
        return 0.0f;
    }
    
    int intersection = (x2 - x1) * (y2 - y1);
    int area1 = rect1.width * rect1.height;
    int area2 = rect2.width * rect2.height;
    int union_area = area1 + area2 - intersection;
    
    return static_cast<float>(intersection) / union_area;
}

// Legacy methods for compatibility
bool DetectionModule::preprocessFrame(const std::shared_ptr<ProcessedFrame>& frame, MLMultiArray* inputArray) {
    return preprocessFrameOptimized(frame, inputArray);
}

std::vector<Detection> DetectionModule::postprocessOutput(MLMultiArray* outputArray, int frameWidth, int frameHeight) {
    return postprocessOutputOptimized(outputArray, frameWidth, frameHeight);
}

std::vector<Detection> DetectionModule::applyNMS(const std::vector<Detection>& detections) {
    return applyNMSOptimized(detections);
}

cv::Rect DetectionModule::convertToPixelCoords(float x, float y, float w, float h, int frameWidth, int frameHeight) {
    float scaleX = static_cast<float>(inputWidth_) / frameWidth;
    float scaleY = static_cast<float>(inputHeight_) / frameHeight;
    float scale = std::min(scaleX, scaleY);
    
    int newWidth = static_cast<int>(frameWidth * scale);
    int newHeight = static_cast<int>(frameHeight * scale);
    
    int padX = (inputWidth_ - newWidth) / 2;
    int padY = (inputHeight_ - newHeight) / 2;
    
    return convertToPixelCoordsOptimized(x, y, w, h, frameWidth, frameHeight, padX, padY, scale);
}

float DetectionModule::calculateIoU(const cv::Rect& rect1, const cv::Rect& rect2) {
    return calculateIoUOptimized(rect1, rect2);
}

void DetectionModule::setConfidenceThreshold(float threshold) {
    confidenceThreshold_ = std::max(0.0f, std::min(1.0f, threshold));
}

void DetectionModule::setNMSThreshold(float threshold) {
    nmsThreshold_ = std::max(0.0f, std::min(1.0f, threshold));
}

void DetectionModule::setMaxDetections(int maxDetections) {
    maxDetections_ = std::max(1, maxDetections);
}

void DetectionModule::setDetectionClasses(const std::vector<std::string>& classes) {
    detectionClasses_ = classes;
}

DetectionStats DetectionModule::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
} 