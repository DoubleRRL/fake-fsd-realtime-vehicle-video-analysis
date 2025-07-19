#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <optional>

// Forward declarations for Core Video
typedef struct __CVBuffer* CVPixelBufferRef;

namespace RealTimeVideoAnalysis {

// Video resolution enum
enum class VideoResolution {
    HD540p,     // 960x540
    HD720p,     // 1280x720
    HD1080p,    // 1920x1080
    Custom
};

// Detection structure
struct Detection {
    cv::Rect boundingBox;
    float confidence;
    int classId;
    std::string className;
    cv::Point2f center;
    float area;
    
    Detection() = default;
    Detection(const cv::Rect& box, float conf, int id, const std::string& name)
        : boundingBox(box), confidence(conf), classId(id), className(name) {
        center = cv::Point2f(box.x + box.width/2.0f, box.y + box.height/2.0f);
        area = box.width * box.height;
    }
};

// Track structure for object tracking
struct Track {
    int id;
    cv::Rect boundingBox;
    cv::Point2f center;
    cv::Point2f velocity;
    float confidence;
    int age;
    int hits;
    int timeSinceUpdate;
    std::vector<cv::Point2f> history;
    
    Track() : id(-1), age(0), hits(0), timeSinceUpdate(0), confidence(0.0f) {}
    Track(int trackId, const Detection& detection)
        : id(trackId), boundingBox(detection.boundingBox), center(detection.center),
          confidence(detection.confidence), age(1), hits(1), timeSinceUpdate(0) {
        history.push_back(center);
    }
};

// Action label structure
struct ActionLabel {
    int trackId;
    std::string action;
    float confidence;
    std::string direction;
    float speed;
    float acceleration;
    
    ActionLabel() : trackId(-1), confidence(0.0f), speed(0.0f), acceleration(0.0f) {}
    ActionLabel(int id, const std::string& act, float conf)
        : trackId(id), action(act), confidence(conf), speed(0.0f), acceleration(0.0f) {}
};

// Track prediction structure
struct TrackPrediction {
    int trackId;
    cv::Point2f predictedPosition;
    cv::Point2f velocity;
    float confidence;
    std::chrono::milliseconds timeHorizon;
    std::vector<cv::Point2f> trajectory;
    
    TrackPrediction() : trackId(-1), confidence(0.0f) {}
    TrackPrediction(int id, const cv::Point2f& pos, float conf)
        : trackId(id), predictedPosition(pos), confidence(conf) {}
};

// Collision risk structure
struct CollisionRisk {
    int trackId1;
    int trackId2;
    float probability;
    std::chrono::milliseconds timeToCollision;
    cv::Point2f collisionPoint;
    
    CollisionRisk() : trackId1(-1), trackId2(-1), probability(0.0f) {}
};

// Frame processing result
struct FrameResult {
    CVPixelBufferRef pixelBuffer;
    std::chrono::high_resolution_clock::time_point timestamp;
    double processingTime;
    bool success;
    
    FrameResult() : pixelBuffer(nullptr), processingTime(0.0), success(false) {}
};

// Detection result
struct DetectionResult {
    std::vector<Detection> detections;
    double processingTime;
    int frameId;
    bool success;
    
    DetectionResult() : processingTime(0.0), frameId(-1), success(false) {}
};

// Tracking result
struct TrackingResult {
    std::vector<Track> tracks;
    std::vector<int> matchedDetections;
    double processingTime;
    int frameId;
    
    TrackingResult() : processingTime(0.0), frameId(-1) {}
};

// Labeling result
struct LabelingResult {
    std::vector<ActionLabel> labels;
    double processingTime;
    int frameId;
    
    LabelingResult() : processingTime(0.0), frameId(-1) {}
};

// Prediction result
struct PredictionResult {
    std::vector<TrackPrediction> predictions;
    std::vector<CollisionRisk> collisionRisks;
    double processingTime;
    int frameId;
    
    PredictionResult() : processingTime(0.0), frameId(-1) {}
};

// Rendering result
struct RenderingResult {
    CVPixelBufferRef renderedFrame;
    double processingTime;
    bool success;
    
    RenderingResult() : renderedFrame(nullptr), processingTime(0.0), success(false) {}
};

// Pipeline configuration
struct PipelineConfig {
    VideoResolution inputResolution = VideoResolution::HD540p;
    int targetFPS = 50;
    bool enableGPUAcceleration = true;
    bool enableNeuralEngine = true;
    double maxLatency = 20.0; // ms
    float confidenceThreshold = 0.5f;
    float nmsThreshold = 0.4f;
    int maxDetections = 100;
    int maxTracks = 50;
    float predictionHorizon = 2.0f; // seconds
};

// Performance metrics
struct PerformanceMetrics {
    double currentFPS;
    double averageLatency;
    double peakLatency;
    double memoryUsage;
    double cpuUsage;
    double gpuUsage;
    int frameCount;
    std::chrono::high_resolution_clock::time_point startTime;
    
    PerformanceMetrics() : currentFPS(0.0), averageLatency(0.0), peakLatency(0.0),
                          memoryUsage(0.0), cpuUsage(0.0), gpuUsage(0.0), frameCount(0) {}
};

// Video input metrics
struct VideoInputMetrics {
    double currentFPS;
    double averageLatency;
    int bufferUsage;
    int droppedFrames;
    std::string codec;
    cv::Size resolution;
    
    VideoInputMetrics() : currentFPS(0.0), averageLatency(0.0), bufferUsage(0), droppedFrames(0) {}
};

// Processing options
struct ProcessingOptions {
    bool enableResize = true;
    bool enableEnhancement = true;
    bool enableFormatConversion = true;
    float resizeScale = 1.0f;
    float contrast = 1.0f;
    float brightness = 0.0f;
    
    ProcessingOptions() = default;
};

// Rendering options
struct RenderingOptions {
    bool showBoundingBoxes = true;
    bool showLabels = true;
    bool showPredictions = true;
    bool showTrajectories = true;
    bool showPerformanceMetrics = true;
    float boxThickness = 2.0f;
    float textScale = 1.0f;
    cv::Scalar backgroundColor = cv::Scalar(0, 0, 0);
    
    RenderingOptions() = default;
};

// Kalman filter parameters
struct KalmanParams {
    float processNoise = 0.1f;
    float measurementNoise = 0.1f;
    float initialError = 1.0f;
    int stateSize = 4; // x, y, vx, vy
    int measurementSize = 2; // x, y
    
    KalmanParams() = default;
};

// Classification rule
struct ClassificationRule {
    std::string name;
    std::function<bool(const Track&, const std::vector<Track>&)> condition;
    std::string action;
    float confidence;
    
    ClassificationRule() : confidence(0.0f) {}
    ClassificationRule(const std::string& n, const std::string& act, float conf)
        : name(n), action(act), confidence(conf) {}
};

// Motion history
struct MotionHistory {
    int trackId;
    std::vector<cv::Point2f> positions;
    std::vector<cv::Point2f> velocities;
    std::vector<std::chrono::high_resolution_clock::time_point> timestamps;
    int maxHistoryLength;
    
    MotionHistory() : trackId(-1), maxHistoryLength(10) {}
    MotionHistory(int id, int maxLen) : trackId(id), maxHistoryLength(maxLen) {}
};

// Motion statistics
struct MotionStats {
    float averageSpeed;
    float maxSpeed;
    float averageAcceleration;
    cv::Point2f averageDirection;
    float directionVariance;
    std::chrono::milliseconds duration;
    
    MotionStats() : averageSpeed(0.0f), maxSpeed(0.0f), averageAcceleration(0.0f),
                   directionVariance(0.0f) {}
};

// Prediction state
struct PredictionState {
    cv::KalmanFilter kalmanFilter;
    cv::Mat state;
    cv::Mat measurement;
    float lastUpdateTime;
    int updateCount;
    
    PredictionState() : lastUpdateTime(0.0f), updateCount(0) {}
};

// Prediction metrics
struct PredictionMetrics {
    float averageError;
    float maxError;
    float accuracy;
    int totalPredictions;
    int accuratePredictions;
    
    PredictionMetrics() : averageError(0.0f), maxError(0.0f), accuracy(0.0f),
                         totalPredictions(0), accuratePredictions(0) {}
};

// Buffer pool statistics
struct BufferPoolStats {
    size_t totalBuffers;
    size_t activeBuffers;
    size_t totalMemory;
    size_t peakMemory;
    double utilizationRate;
    
    BufferPoolStats() : totalBuffers(0), activeBuffers(0), totalMemory(0),
                       peakMemory(0), utilizationRate(0.0) {}
};

// GUI update result
struct GUIUpdateResult {
    bool shouldExit;
    bool showPerformancePanel;
    bool showDetectionPanel;
    bool showControlPanel;
    struct PlaybackControls {
        bool isPlaying;
        float speed;
        int currentFrame;
        bool seekRequested;
        int seekFrame;
    } playbackControls;
    
    GUIUpdateResult() : shouldExit(false), showPerformancePanel(true),
                       showDetectionPanel(true), showControlPanel(true) {
        playbackControls.isPlaying = false;
        playbackControls.speed = 1.0f;
        playbackControls.currentFrame = 0;
        playbackControls.seekRequested = false;
        playbackControls.seekFrame = 0;
    }
};

// Input event
struct InputEvent {
    enum Type {
        KeyPress,
        MouseClick,
        MouseMove,
        Scroll
    } type;
    
    int key;
    cv::Point2f position;
    float scrollDelta;
    
    InputEvent() : type(KeyPress), key(0), scrollDelta(0.0f) {}
};

// Pipeline result
struct PipelineResult {
    CVPixelBufferRef renderedFrame;
    std::vector<Detection> detections;
    std::vector<Track> tracks;
    std::vector<ActionLabel> labels;
    std::vector<TrackPrediction> predictions;
    PerformanceMetrics metrics;
    int frameId;
    
    PipelineResult() : renderedFrame(nullptr), frameId(-1) {}
};

// Batch detection result
struct BatchDetectionResult {
    std::vector<std::vector<Detection>> detections;
    double totalProcessingTime;
    double averageProcessingTime;
    int batchSize;
    bool success;
    
    BatchDetectionResult() : totalProcessingTime(0.0), averageProcessingTime(0.0),
                           batchSize(0), success(false) {}
};

} // namespace RealTimeVideoAnalysis 