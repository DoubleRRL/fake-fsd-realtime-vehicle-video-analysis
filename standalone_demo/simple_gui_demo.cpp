#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <thread>

class SimpleVideoGUI {
private:
    GLFWwindow* window_;
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    bool isRunning_;
    
    // Performance tracking
    std::vector<float> fpsHistory_;
    int frameCount_;
    auto startTime_;
    double currentFPS_;
    double averageLatency_;
    
    // GUI state
    bool showPerformance_;
    bool enableDetection_;
    float confidenceThreshold_;
    
    // Video properties
    int videoWidth_;
    int videoHeight_;
    double videoFPS_;
    std::string videoSource_;
    
    // OpenGL texture
    GLuint textureID_;
    
public:
    SimpleVideoGUI() : window_(nullptr), isRunning_(false), showPerformance_(true),
                      enableDetection_(false), confidenceThreshold_(0.5f),
                      frameCount_(0), videoWidth_(0), videoHeight_(0), videoFPS_(0.0),
                      textureID_(0) {
        startTime_ = std::chrono::high_resolution_clock::now();
    }
    
    ~SimpleVideoGUI() {
        cleanup();
    }
    
    bool initialize(const std::string& source) {
        videoSource_ = source;
        
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        // Create window
        window_ = glfwCreateWindow(1280, 720, "Real-Time Video Analysis GUI", nullptr, nullptr);
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
        glViewport(0, 0, 1280, 720);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1280, 720, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Open video source
        if (!openVideoSource(source)) {
            return false;
        }
        
        isRunning_ = true;
        return true;
    }
    
    bool openVideoSource(const std::string& source) {
        if (source == "camera") {
            std::cout << "WARNING: This will request camera access!" << std::endl;
            std::cout << "Press Enter to continue or Ctrl+C to cancel..." << std::endl;
            std::cin.get();
            
            cap_.open(0);
            std::cout << "Opening camera..." << std::endl;
        } else {
            cap_.open(source);
            std::cout << "Opening video file: " << source << std::endl;
        }
        
        if (!cap_.isOpened()) {
            std::cerr << "Error: Could not open video source!" << std::endl;
            return false;
        }
        
        // Get video properties
        videoFPS_ = cap_.get(cv::CAP_PROP_FPS);
        videoWidth_ = cap_.get(cv::CAP_PROP_FRAME_WIDTH);
        videoHeight_ = cap_.get(cv::CAP_PROP_FRAME_HEIGHT);
        
        std::cout << "Video properties:" << std::endl;
        std::cout << "  Resolution: " << videoWidth_ << "x" << videoHeight_ << std::endl;
        std::cout << "  FPS: " << videoFPS_ << std::endl;
        
        return true;
    }
    
    void run() {
        while (!glfwWindowShouldClose(window_) && isRunning_) {
            glfwPollEvents();
            
            // Process video frame
            processFrame();
            
            // Render GUI
            renderGUI();
            
            // Update window
            glfwSwapBuffers(window_);
            
            // Small delay
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }
    
    void processFrame() {
        cap_ >> currentFrame_;
        
        if (currentFrame_.empty()) {
            std::cout << "End of video or camera disconnected" << std::endl;
            isRunning_ = false;
            return;
        }
        
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
        
        // Simulate detection processing
        if (enableDetection_) {
            // Add some processing delay to simulate detection
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
    void renderGUI() {
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render video frame
        renderVideoFrame();
        
        // Render performance overlay
        if (showPerformance_) {
            renderPerformanceOverlay();
        }
        
        // Render controls
        renderControls();
    }
    
    void renderVideoFrame() {
        if (currentFrame_.empty()) {
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
        glTexCoord2f(0.0f, 1.0f); glVertex2f(10, 10);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(810, 10);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(810, 610);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(10, 610);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        
        // Draw detection boxes if enabled
        if (enableDetection_) {
            glColor3f(1.0f, 0.0f, 0.0f);
            glLineWidth(2.0f);
            
            // Simulate detection boxes
            for (int i = 0; i < 3; ++i) {
                float x = 50 + i * 150;
                float y = 50;
                float w = 100;
                float h = 80;
                
                glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + w, y);
                glVertex2f(x + w, y + h);
                glVertex2f(x, y + h);
                glEnd();
            }
            
            glColor3f(1.0f, 1.0f, 1.0f);
        }
    }
    
    void renderPerformanceOverlay() {
        // Draw performance panel background
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(820, 10);
        glVertex2f(1270, 10);
        glVertex2f(1270, 300);
        glVertex2f(820, 300);
        glEnd();
        
        // Draw text (simplified - in a real implementation you'd use a text rendering library)
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw performance metrics as colored rectangles
        // FPS indicator
        float fpsRatio = std::min(currentFPS_ / 60.0, 1.0);
        glColor3f(1.0f - fpsRatio, fpsRatio, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(830, 30);
        glVertex2f(830 + fpsRatio * 200, 30);
        glVertex2f(830 + fpsRatio * 200, 50);
        glVertex2f(830, 50);
        glEnd();
        
        // Latency indicator
        float latencyRatio = std::min(averageLatency_ / 20.0, 1.0);
        glColor3f(latencyRatio, 1.0f - latencyRatio, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(830, 70);
        glVertex2f(830 + latencyRatio * 200, 70);
        glVertex2f(830 + latencyRatio * 200, 90);
        glVertex2f(830, 90);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    void renderControls() {
        // Draw controls panel background
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(820, 320);
        glVertex2f(1270, 320);
        glVertex2f(1270, 710);
        glVertex2f(820, 710);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        // Draw control buttons (simplified)
        // Detection toggle button
        glColor3f(enableDetection_ ? 0.0f : 0.5f, enableDetection_ ? 1.0f : 0.5f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(830, 340);
        glVertex2f(930, 340);
        glVertex2f(930, 370);
        glVertex2f(830, 370);
        glEnd();
        
        // Confidence threshold slider
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(830, 390);
        glVertex2f(1030, 390);
        glVertex2f(1030, 410);
        glVertex2f(830, 410);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(830, 390);
        glVertex2f(830 + confidenceThreshold_ * 200, 390);
        glVertex2f(830 + confidenceThreshold_ * 200, 410);
        glVertex2f(830, 410);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
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
    
    // Handle input
    void handleInput() {
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            isRunning_ = false;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
            enableDetection_ = !enableDetection_;
        }
        
        if (glfwGetKey(window_, GLFW_KEY_P) == GLFW_PRESS) {
            showPerformance_ = !showPerformance_;
        }
        
        // Mouse input for confidence threshold
        double mouseX, mouseY;
        glfwGetCursorPos(window_, &mouseX, &mouseY);
        
        if (mouseX >= 830 && mouseX <= 1030 && mouseY >= 390 && mouseY <= 410) {
            if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                confidenceThreshold_ = (mouseX - 830) / 200.0f;
            }
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <video_file>" << std::endl;
        std::cout << "Or use: " << argv[0] << " camera" << std::endl;
        return -1;
    }
    
    SimpleVideoGUI gui;
    
    if (!gui.initialize(argv[1])) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return -1;
    }
    
    std::cout << "GUI initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - ESC: Quit" << std::endl;
    std::cout << "  - D: Toggle detection" << std::endl;
    std::cout << "  - P: Toggle performance overlay" << std::endl;
    std::cout << "  - Mouse: Adjust confidence threshold" << std::endl;
    
    gui.run();
    
    return 0;
} 