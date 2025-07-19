#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

// Forward declarations
class Track;

struct Detection {
    cv::Rect bbox;
    float confidence;
    int class_id;
    std::string class_name;
};

struct DetectionResult {
    cv::Rect box;
    float confidence;
    int class_id;
};

struct TrackedObject {
    int track_id;
    cv::Rect bbox;
    float confidence;
    int class_id;
    std::string class_name;
    int age;
    int total_hits;
    int time_since_update;
};

class DetectionTracker {
public:
    DetectionTracker();
    ~DetectionTracker();

    // Initialize the detection and tracking system
    bool initialize(const std::string& model_path, const std::string& config_path, 
                   const std::string& classes_path, float conf_threshold = 0.5, 
                   float nms_threshold = 0.4);

    // Process a frame and return tracked objects
    std::vector<TrackedObject> processFrame(const cv::Mat& frame);

    // Get performance metrics
    double getFPS() const { return current_fps_; }
    double getDetectionTime() const { return detection_time_ms_; }
    double getTrackingTime() const { return tracking_time_ms_; }
    int getActiveTracks() const { return active_tracks_; }

    // Settings
    void setConfidenceThreshold(float threshold) { conf_threshold_ = threshold; }
    void setNMSThreshold(float threshold) { nms_threshold_ = threshold; }
    void setMaxDisappeared(int frames) { max_disappeared_ = frames; }
    void setMinHits(int hits) { min_hits_ = hits; }
    void setIOUThreshold(float threshold) { iou_threshold_ = threshold; }
    
    // Performance settings
    void enableHighPerformanceMode(bool enable = true);
    void setThreadCount(int threads);
    void setBufferSize(int size);

private:
    // YOLO detection
    cv::dnn::Net yolo_net_;
    std::vector<std::string> class_names_;
    float conf_threshold_;
    float nms_threshold_;
    
    // SORT tracking
    std::vector<std::unique_ptr<Track>> tracks_;
    int next_track_id_;
    int max_disappeared_;
    int min_hits_;
    float iou_threshold_;
    
    // Performance metrics
    double current_fps_;
    double detection_time_ms_;
    double tracking_time_ms_;
    int active_tracks_;
    std::chrono::high_resolution_clock::time_point last_frame_time_;
    
    // Performance optimization buffers
    cv::Mat frame_buffer_;
    cv::Mat processed_buffer_;
    std::vector<cv::Mat> blob_buffer_;
    std::vector<Detection> detection_buffer_;
    std::vector<TrackedObject> tracked_objects_buffer_;
    
    // Threading and optimization
    int num_threads_;
    bool use_optimizations_;
    
    // Detection methods
    std::vector<Detection> detectObjects(const cv::Mat& frame);
    cv::Mat preprocessFrame(const cv::Mat& frame);
    std::vector<cv::Rect> postprocessDetections(const cv::Mat& output, 
                                               const cv::Size& original_size);
    std::vector<DetectionResult> postprocessDetectionsWithInfo(const cv::Mat& output,
                                                              const cv::Size& original_size);
    
    // Tracking methods
    void updateTracks(const std::vector<Detection>& detections);
    void updateTracksFromResults(const std::vector<DetectionResult>& results);
    std::vector<int> associateDetectionsToTracks(const std::vector<Detection>& detections,
                                                std::vector<TrackedObject>& tracked_objects);
    float calculateIOU(const cv::Rect& rect1, const cv::Rect& rect2);
    void updateTrackedObjects(std::vector<TrackedObject>& tracked_objects);
    
    // Utility methods
    void loadClassNames(const std::string& classes_path);
    void drawDetections(cv::Mat& frame, const std::vector<TrackedObject>& objects);
};

// Track class for SORT algorithm
class Track {
public:
    Track(const cv::Rect& bbox, int track_id, int class_id, float confidence, 
          const std::string& class_name);
    
    void predict();
    void update(const cv::Rect& bbox, float confidence);
    cv::Rect getBBox() const;
    cv::Point2f getCenter() const;
    int getTrackId() const { return track_id_; }
    int getClassId() const { return class_id_; }
    float getConfidence() const { return confidence_; }
    std::string getClassName() const { return class_name_; }
    int getAge() const { return age_; }
    int getTotalHits() const { return total_hits_; }
    int getTimeSinceUpdate() const { return time_since_update_; }
    bool isConfirmed() const { return total_hits_ >= 3; }
    
private:
    int track_id_;
    int class_id_;
    float confidence_;
    std::string class_name_;
    cv::Rect bbox_;
    int age_;
    int total_hits_;
    int time_since_update_;
    
    // Kalman filter state (simplified for this implementation)
    cv::Point2f velocity_;
    cv::Point2f position_;
}; 