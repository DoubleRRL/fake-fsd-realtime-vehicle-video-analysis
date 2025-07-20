#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// Simple annotation structure for UA-DETRAC format
struct Detection {
    int frame_id;
    int track_id;
    float x, y, width, height;
    std::string label;
    float confidence;
    
    Detection() : frame_id(0), track_id(0), x(0), y(0), width(0), height(0), confidence(0.0f) {}
};

struct VideoAnnotation {
    std::string video_path;
    std::vector<Detection> detections;
    int total_frames;
    double fps;
    int width, height;
    
    VideoAnnotation() : total_frames(0), fps(0.0), width(0), height(0) {}
};

class ProfessionalVideoGUI {
private:
    GLFWwindow* window_;
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    bool isRunning_;
    bool isPlaying_;
    bool showAnnotations_;
    bool showPerformance_;
    bool showFileBrowser_;
    
    // Video and annotation data
    VideoAnnotation currentVideo_;
    std::vector<Detection> currentFrameDetections_;
    int currentFrameIndex_;
    double playbackSpeed_;
    
    // Performance tracking
    std::vector<float> fpsHistory_;
    int frameCount_;
    std::chrono::high_resolution_clock::time_point startTime_;
    double currentFPS_;
    double averageLatency_;
    
    // GUI state
    float confidenceThreshold_;
    bool showTracks_;
    bool showLabels_;
    bool showConfidence_;
    
    // File browser
    std::string currentDirectory_;
    std::vector<fs::path> videoFiles_;
    int selectedFileIndex_;
    
    // OpenGL texture
    GLuint textureID_;
    
    // Colors for different object types
    struct Colors {
        float car[3] = {0.0f, 1.0f, 0.0f};      // Green
        float bus[3] = {1.0f, 0.0f, 0.0f};      // Red
        float van[3] = {0.0f, 0.0f, 1.0f};      // Blue
        float truck[3] = {1.0f, 1.0f, 0.0f};    // Yellow
        float others[3] = {1.0f, 0.0f, 1.0f};   // Magenta
    } colors_;
    
public:
    ProfessionalVideoGUI() : window_(nullptr), isRunning_(false), isPlaying_(false),
                           showAnnotations_(true), showPerformance_(true), showFileBrowser_(true),
                           currentFrameIndex_(0), playbackSpeed_(1.0), frameCount_(0),
                           confidenceThreshold_(0.5f), showTracks_(true), showLabels_(true),
                           showConfidence_(true), selectedFileIndex_(-1), textureID_(0) {
        startTime_ = std::chrono::high_resolution_clock::now();
        currentDirectory_ = fs::current_path().string();
        scanForVideoFiles();
    }
    
    ~ProfessionalVideoGUI() {
        cleanup();
    }
    
    bool initialize() {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        // Create window
        window_ = glfwCreateWindow(1600, 900, "Professional Video Analysis GUI", nullptr, nullptr);
        if (!window_) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync
        
        // Initialize OpenGL
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &textureID_);
        glBindTexture(GL_TEXTURE_2D, textureID_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Set up OpenGL viewport
        glViewport(0, 0, 1600, 900);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1600, 900, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        isRunning_ = true;
        return true;
    }
    
    void scanForVideoFiles() {
        videoFiles_.clear();
        for (const auto& entry : fs::directory_iterator(currentDirectory_)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".mp4" || ext == ".avi" || ext == ".mov" || ext == ".mkv") {
                    videoFiles_.push_back(entry.path());
                }
            }
        }
    }
    
    bool loadVideo(const std::string& videoPath) {
        cap_.release();
        cap_.open(videoPath);
        
        if (!cap_.isOpened()) {
            std::cerr << "Error: Could not open video: " << videoPath << std::endl;
            return false;
        }
        
        currentVideo_.video_path = videoPath;
        currentVideo_.total_frames = cap_.get(cv::CAP_PROP_FRAME_COUNT);
        currentVideo_.fps = cap_.get(cv::CAP_PROP_FPS);
        currentVideo_.width = cap_.get(cv::CAP_PROP_FRAME_WIDTH);
        currentVideo_.height = cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
        
        // Try to load annotations
        loadAnnotations(videoPath);
        
        currentFrameIndex_ = 0;
        isPlaying_ = false;
        
        std::cout << "Loaded video: " << videoPath << std::endl;
        std::cout << "  Frames: " << currentVideo_.total_frames << std::endl;
        std::cout << "  FPS: " << currentVideo_.fps << std::endl;
        std::cout << "  Resolution: " << currentVideo_.width << "x" << currentVideo_.height << std::endl;
        std::cout << "  Annotations: " << currentVideo_.detections.size() << " detections" << std::endl;
        
        return true;
    }
    
    void loadAnnotations(const std::string& videoPath) {
        // Look for annotation file (UA-DETRAC format)
        fs::path videoFile(videoPath);
        fs::path annotationFile = videoFile.parent_path() / (videoFile.stem().string() + ".xml");
        
        if (fs::exists(annotationFile)) {
            std::cout << "Loading annotations from: " << annotationFile << std::endl;
            // Simplified XML parsing - in a real implementation you'd use a proper XML parser
            parseAnnotations(annotationFile.string());
        } else {
            std::cout << "No annotation file found, generating dummy annotations" << std::endl;
            generateDummyAnnotations();
        }
    }
    
    void parseAnnotations(const std::string& annotationPath) {
        // Simplified XML parsing for UA-DETRAC format
        std::ifstream file(annotationPath);
        std::string line;
        currentVideo_.detections.clear();
        
        while (std::getline(file, line)) {
            if (line.find("<frame>") != std::string::npos) {
                // Parse frame
                int frameId = 0;
                if (line.find("num=\"") != std::string::npos) {
                    size_t pos = line.find("num=\"") + 5;
                    size_t end = line.find("\"", pos);
                    frameId = std::stoi(line.substr(pos, end - pos));
                }
                
                // Parse target (detection)
                while (std::getline(file, line) && line.find("</frame>") == std::string::npos) {
                    if (line.find("<target>") != std::string::npos) {
                        Detection det;
                        det.frame_id = frameId;
                        
                        while (std::getline(file, line) && line.find("</target>") == std::string::npos) {
                            if (line.find("id=\"") != std::string::npos) {
                                size_t pos = line.find("id=\"") + 4;
                                size_t end = line.find("\"", pos);
                                det.track_id = std::stoi(line.substr(pos, end - pos));
                            } else if (line.find("<box>") != std::string::npos) {
                                // Parse bounding box
                                std::getline(file, line); // x
                                det.x = std::stof(line.substr(line.find(">") + 1, line.find("</") - line.find(">") - 1));
                                std::getline(file, line); // y
                                det.y = std::stof(line.substr(line.find(">") + 1, line.find("</") - line.find(">") - 1));
                                std::getline(file, line); // width
                                det.width = std::stof(line.substr(line.find(">") + 1, line.find("</") - line.find(">") - 1));
                                std::getline(file, line); // height
                                det.height = std::stof(line.substr(line.find(">") + 1, line.find("</") - line.find(">") - 1));
                            } else if (line.find("<attribute>") != std::string::npos) {
                                // Parse vehicle type
                                std::getline(file, line);
                                if (line.find("vehicle_type>") != std::string::npos) {
                                    det.label = line.substr(line.find(">") + 1, line.find("</") - line.find(">") - 1);
                                }
                            }
                        }
                        det.confidence = 0.8f; // Default confidence
                        currentVideo_.detections.push_back(det);
                    }
                }
            }
        }
    }
    
    void generateDummyAnnotations() {
        currentVideo_.detections.clear();
        
        // Generate some dummy detections for demonstration
        for (int frame = 0; frame < std::min(100, currentVideo_.total_frames); frame += 5) {
            for (int i = 0; i < 3; ++i) {
                Detection det;
                det.frame_id = frame;
                det.track_id = i + 1;
                det.x = 100 + i * 200 + (frame % 50);
                det.y = 100 + (frame % 30);
                det.width = 80 + (frame % 20);
                det.height = 60 + (frame % 15);
                det.confidence = 0.7f + (frame % 30) / 100.0f;
                
                std::string labels[] = {"car", "bus", "van", "truck", "others"};
                det.label = labels[i % 5];
                
                currentVideo_.detections.push_back(det);
            }
        }
    }
    
    void run() {
        while (!glfwWindowShouldClose(window_) && isRunning_) {
            glfwPollEvents();
            
            // Handle input
            handleInput();
            
            // Process video frame
            if (isPlaying_) {
                processFrame();
            }
            
            // Render GUI
            renderGUI();
            
            // Update window
            glfwSwapBuffers(window_);
            
            // Control playback speed
            if (isPlaying_) {
                int delay = static_cast<int>(1000.0 / (currentVideo_.fps * playbackSpeed_));
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }
    }
    
    void processFrame() {
        if (currentFrameIndex_ >= currentVideo_.total_frames) {
            isPlaying_ = false;
            currentFrameIndex_ = 0;
            return;
        }
        
        cap_.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex_);
        cap_ >> currentFrame_;
        
        if (currentFrame_.empty()) {
            isPlaying_ = false;
            return;
        }
        
        // Get detections for current frame
        currentFrameDetections_.clear();
        for (const auto& det : currentVideo_.detections) {
            if (det.frame_id == currentFrameIndex_ && det.confidence >= confidenceThreshold_) {
                currentFrameDetections_.push_back(det);
            }
        }
        
        currentFrameIndex_++;
        frameCount_++;
        
        // Calculate performance metrics
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime_);
        currentFPS_ = (frameCount_ * 1000.0) / elapsed.count();
        averageLatency_ = elapsed.count() / (double)frameCount_;
        
        // Update performance history
        fpsHistory_.push_back(static_cast<float>(currentFPS_));
        if (fpsHistory_.size() > 100) {
            fpsHistory_.erase(fpsHistory_.begin());
        }
    }
    
    void renderGUI() {
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render file browser
        if (showFileBrowser_) {
            renderFileBrowser();
        }
        
        // Render video frame
        renderVideoFrame();
        
        // Render performance overlay
        if (showPerformance_) {
            renderPerformanceOverlay();
        }
        
        // Render controls
        renderControls();
        
        // Render playback controls
        renderPlaybackControls();
    }
    
    void renderFileBrowser() {
        // Draw file browser panel
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(10, 10);
        glVertex2f(400, 10);
        glVertex2f(400, 300);
        glVertex2f(10, 300);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw file list (simplified - in real implementation you'd use text rendering)
        for (size_t i = 0; i < videoFiles_.size(); ++i) {
            if (i == selectedFileIndex_) {
                glColor3f(0.3f, 0.6f, 1.0f);
            } else {
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            
            // Draw file entry background
            glBegin(GL_QUADS);
            glVertex2f(20, 30 + i * 25);
            glVertex2f(390, 30 + i * 25);
            glVertex2f(390, 50 + i * 25);
            glVertex2f(20, 50 + i * 25);
            glEnd();
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void renderVideoFrame() {
        if (currentFrame_.empty()) {
            // Draw placeholder
            glColor3f(0.3f, 0.3f, 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(420, 10);
            glVertex2f(1200, 10);
            glVertex2f(1200, 600);
            glVertex2f(420, 600);
            glEnd();
            
            glColor3f(1.0f, 1.0f, 1.0f);
            return;
        }
        
        // Convert BGR to RGB
        cv::Mat rgbFrame;
        cv::cvtColor(currentFrame_, rgbFrame, cv::COLOR_BGR2RGB);
        
        // Update OpenGL texture
        glBindTexture(GL_TEXTURE_2D, textureID_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbFrame.cols, rgbFrame.rows, 
                    0, GL_RGB, GL_UNSIGNED_BYTE, rgbFrame.data);
        
        // Draw video frame
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID_);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(420, 10);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1200, 10);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1200, 600);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(420, 600);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        
        // Draw annotations
        if (showAnnotations_) {
            renderAnnotations();
        }
    }
    
    void renderAnnotations() {
        for (const auto& det : currentFrameDetections_) {
            // Get color based on vehicle type
            float* color = colors_.car; // default
            if (det.label == "bus") color = colors_.bus;
            else if (det.label == "van") color = colors_.van;
            else if (det.label == "truck") color = colors_.truck;
            else if (det.label == "others") color = colors_.others;
            
            // Scale coordinates to screen space
            float x = 420 + (det.x / currentVideo_.width) * 780;
            float y = 10 + (det.y / currentVideo_.height) * 590;
            float w = (det.width / currentVideo_.width) * 780;
            float h = (det.height / currentVideo_.height) * 590;
            
            // Draw bounding box
            glColor3f(color[0], color[1], color[2]);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
            glEnd();
            
            // Draw track ID
            if (showTracks_) {
                glColor3f(1.0f, 1.0f, 1.0f);
                // In real implementation, you'd render text here
            }
            
            // Draw confidence
            if (showConfidence_) {
                glColor3f(1.0f, 1.0f, 0.0f);
                // In real implementation, you'd render text here
            }
        }
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void renderPerformanceOverlay() {
        // Draw performance panel
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(1210, 10);
        glVertex2f(1590, 10);
        glVertex2f(1590, 300);
        glVertex2f(1210, 300);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw performance metrics as colored bars
        // FPS indicator
        float fpsRatio = std::min(currentFPS_ / 60.0, 1.0);
        glColor3f(1.0f - fpsRatio, fpsRatio, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(1220, 30);
        glVertex2f(1220 + fpsRatio * 200, 30);
        glVertex2f(1220 + fpsRatio * 200, 50);
        glVertex2f(1220, 50);
        glEnd();
        
        // Latency indicator
        float latencyRatio = std::min(averageLatency_ / 20.0, 1.0);
        glColor3f(latencyRatio, 1.0f - latencyRatio, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(1220, 70);
        glVertex2f(1220 + latencyRatio * 200, 70);
        glVertex2f(1220 + latencyRatio * 200, 90);
        glVertex2f(1220, 90);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void renderControls() {
        // Draw controls panel
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(1210, 320);
        glVertex2f(1590, 320);
        glVertex2f(1590, 600);
        glVertex2f(1210, 600);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw control buttons
        // Confidence threshold slider
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(1220, 340);
        glVertex2f(1420, 340);
        glVertex2f(1420, 360);
        glVertex2f(1220, 360);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(1220, 340);
        glVertex2f(1220 + confidenceThreshold_ * 200, 340);
        glVertex2f(1220 + confidenceThreshold_ * 200, 360);
        glVertex2f(1220, 360);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void renderPlaybackControls() {
        // Draw playback controls
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(420, 620);
        glVertex2f(1200, 620);
        glVertex2f(1200, 700);
        glVertex2f(420, 700);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw progress bar
        float progress = currentVideo_.total_frames > 0 ? 
                        (float)currentFrameIndex_ / currentVideo_.total_frames : 0.0f;
        
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(430, 650);
        glVertex2f(1190, 650);
        glVertex2f(1190, 670);
        glVertex2f(430, 670);
        glEnd();
        
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(430, 650);
        glVertex2f(430 + progress * 760, 650);
        glVertex2f(430 + progress * 760, 670);
        glVertex2f(430, 670);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void handleInput() {
        // Keyboard controls
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            isRunning_ = false;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
            isPlaying_ = !isPlaying_;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS) {
            currentFrameIndex_ = std::max(0, currentFrameIndex_ - 1);
            cap_.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex_);
        }
        
        if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            currentFrameIndex_ = std::min(currentVideo_.total_frames - 1, currentFrameIndex_ + 1);
            cap_.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex_);
        }
        
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            showAnnotations_ = !showAnnotations_;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_P) == GLFW_PRESS) {
            showPerformance_ = !showPerformance_;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS) {
            showFileBrowser_ = !showFileBrowser_;
        }
        
        // Mouse input for file selection
        double mouseX, mouseY;
        glfwGetCursorPos(window_, &mouseX, &mouseY);
        
        if (mouseX >= 20 && mouseX <= 390 && mouseY >= 30 && mouseY <= 30 + videoFiles_.size() * 25) {
            if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                int fileIndex = (int)((mouseY - 30) / 25);
                if (fileIndex >= 0 && fileIndex < videoFiles_.size()) {
                    selectedFileIndex_ = fileIndex;
                    loadVideo(videoFiles_[fileIndex].string());
                }
            }
        }
        
        // Mouse input for confidence threshold
        if (mouseX >= 1220 && mouseX <= 1420 && mouseY >= 340 && mouseY <= 360) {
            if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                confidenceThreshold_ = (mouseX - 1220) / 200.0f;
            }
        }
    }
    
    void cleanup() {
        if (cap_.isOpened()) {
            cap_.release();
        }
        
        if (textureID_ != 0) {
            glDeleteTextures(1, &textureID_);
        }
        
        if (window_) {
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }
};

int main(int argc, char** argv) {
    ProfessionalVideoGUI gui;
    
    if (!gui.initialize()) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return -1;
    }
    
    std::cout << "Professional Video Analysis GUI initialized!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - ESC: Quit" << std::endl;
    std::cout << "  - SPACE: Play/Pause" << std::endl;
    std::cout << "  - LEFT/RIGHT: Step frames" << std::endl;
    std::cout << "  - A: Toggle annotations" << std::endl;
    std::cout << "  - P: Toggle performance overlay" << std::endl;
    std::cout << "  - F: Toggle file browser" << std::endl;
    std::cout << "  - Mouse: Select files, adjust settings" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "  - File browser for video selection" << std::endl;
    std::cout << "  - UA-DETRAC annotation support" << std::endl;
    std::cout << "  - Real-time annotation display" << std::endl;
    std::cout << "  - Performance monitoring" << std::endl;
    std::cout << "  - Interactive controls" << std::endl;
    
    gui.run();
    
    return 0;
} 