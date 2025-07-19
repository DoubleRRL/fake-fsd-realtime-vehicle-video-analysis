#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

class VideoProcessorGUI {
private:
    GLFWwindow* window_;
    cv::VideoCapture cap_;
    cv::Mat currentFrame_;
    bool isRunning_;
    bool showPerformance_;
    bool showControls_;
    bool showStats_;
    
    // Performance tracking
    std::vector<float> fpsHistory_;
    std::vector<float> latencyHistory_;
    int frameCount_;
    auto startTime_;
    double currentFPS_;
    double averageLatency_;
    
    // GUI state
    float confidenceThreshold_;
    float nmsThreshold_;
    int maxDetections_;
    bool enableDetection_;
    bool enableTracking_;
    
    // Video properties
    int videoWidth_;
    int videoHeight_;
    double videoFPS_;
    std::string videoSource_;
    
public:
    VideoProcessorGUI() : window_(nullptr), isRunning_(false), showPerformance_(true),
                         showControls_(true), showStats_(true), frameCount_(0),
                         confidenceThreshold_(0.5f), nmsThreshold_(0.4f), maxDetections_(100),
                         enableDetection_(false), enableTracking_(false),
                         videoWidth_(0), videoHeight_(0), videoFPS_(0.0) {
        startTime_ = std::chrono::high_resolution_clock::now();
    }
    
    ~VideoProcessorGUI() {
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
        
        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
        // Setup ImGui style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        
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
            
            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            // Process video frame
            processFrame();
            
            // Render GUI
            renderGUI();
            
            // Render ImGui
            ImGui::Render();
            
            // Clear screen
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // Render ImGui
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
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
        latencyHistory_.push_back(static_cast<float>(averageLatency_));
        
        if (fpsHistory_.size() > 100) {
            fpsHistory_.erase(fpsHistory_.begin());
            latencyHistory_.erase(latencyHistory_.begin());
        }
        
        // Simulate detection processing
        if (enableDetection_) {
            // Add some processing delay to simulate detection
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
    void renderGUI() {
        // Main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(1280, 720));
        
        ImGui::Begin("Real-Time Video Analysis", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
        
        // Video display area
        ImGui::BeginChild("VideoDisplay", ImVec2(800, 600), true);
        
        if (!currentFrame_.empty()) {
            // Convert OpenCV Mat to ImGui texture (simplified)
            // In a real implementation, you'd use proper texture conversion
            ImGui::Text("Video Frame: %dx%d", currentFrame_.cols, currentFrame_.rows);
            ImGui::Text("Frame: %d | FPS: %.1f | Latency: %.2f ms", 
                       frameCount_, currentFPS_, averageLatency_);
            
            // Add some visual elements to simulate video display
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size(600, 400);
            
            // Draw a rectangle to represent the video frame
            drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
                            ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)), 0.0f, 0, 2.0f);
            
            // Simulate detection boxes if enabled
            if (enableDetection_) {
                for (int i = 0; i < 3; ++i) {
                    ImVec2 boxPos(pos.x + 50 + i * 150, pos.y + 50);
                    ImVec2 boxSize(100, 80);
                    drawList->AddRect(boxPos, ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y),
                                    ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), 0.0f, 0, 2.0f);
                    drawList->AddText(ImVec2(boxPos.x, boxPos.y - 20), 
                                    ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 
                                    ("Object " + std::to_string(i + 1)).c_str());
                }
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No video frame available");
        }
        
        ImGui::EndChild();
        
        // Performance panel
        if (showPerformance_) {
            ImGui::SameLine();
            ImGui::BeginChild("Performance", ImVec2(300, 300), true);
            ImGui::Text("Performance Metrics");
            ImGui::Separator();
            
            ImGui::Text("Current FPS: %.1f", currentFPS_);
            ImGui::Text("Average Latency: %.2f ms", averageLatency_);
            ImGui::Text("Total Frames: %d", frameCount_);
            ImGui::Text("Video Source: %s", videoSource_.c_str());
            
            if (!fpsHistory_.empty()) {
                ImGui::PlotLines("FPS History", fpsHistory_.data(), fpsHistory_.size(), 
                               0, nullptr, 0.0f, 100.0f, ImVec2(280, 60));
            }
            
            if (!latencyHistory_.empty()) {
                ImGui::PlotLines("Latency History", latencyHistory_.data(), latencyHistory_.size(), 
                               0, nullptr, 0.0f, 50.0f, ImVec2(280, 60));
            }
            
            ImGui::EndChild();
        }
        
        // Controls panel
        if (showControls_) {
            ImGui::SetNextWindowPos(ImVec2(820, 10));
            ImGui::SetNextWindowSize(ImVec2(300, 400));
            
            ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
            ImGui::Text("Pipeline Controls");
            ImGui::Separator();
            
            ImGui::Checkbox("Enable Detection", &enableDetection_);
            ImGui::Checkbox("Enable Tracking", &enableTracking_);
            
            ImGui::Separator();
            ImGui::Text("Detection Settings");
            
            ImGui::SliderFloat("Confidence Threshold", &confidenceThreshold_, 0.0f, 1.0f);
            ImGui::SliderFloat("NMS Threshold", &nmsThreshold_, 0.0f, 1.0f);
            ImGui::SliderInt("Max Detections", &maxDetections_, 1, 200);
            
            ImGui::Separator();
            ImGui::Text("Display Settings");
            
            ImGui::Checkbox("Show Performance Panel", &showPerformance_);
            ImGui::Checkbox("Show Statistics Panel", &showStats_);
            
            ImGui::Separator();
            
            if (ImGui::Button("Reset Statistics")) {
                frameCount_ = 0;
                fpsHistory_.clear();
                latencyHistory_.clear();
                startTime_ = std::chrono::high_resolution_clock::now();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Quit")) {
                isRunning_ = false;
            }
            
            ImGui::End();
        }
        
        // Statistics panel
        if (showStats_) {
            ImGui::SetNextWindowPos(ImVec2(820, 420));
            ImGui::SetNextWindowSize(ImVec2(300, 200));
            
            ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
            ImGui::Text("Video Statistics");
            ImGui::Separator();
            
            ImGui::Text("Resolution: %dx%d", videoWidth_, videoHeight_);
            ImGui::Text("Video FPS: %.1f", videoFPS_);
            ImGui::Text("Processing FPS: %.1f", currentFPS_);
            ImGui::Text("Frame Drop: %.1f%%", 
                       videoFPS_ > 0 ? ((videoFPS_ - currentFPS_) / videoFPS_) * 100.0 : 0.0);
            
            ImGui::Separator();
            ImGui::Text("Detection Stats");
            ImGui::Text("Detections Enabled: %s", enableDetection_ ? "Yes" : "No");
            ImGui::Text("Tracking Enabled: %s", enableTracking_ ? "Yes" : "No");
            ImGui::Text("Confidence Threshold: %.2f", confidenceThreshold_);
            
            ImGui::End();
        }
        
        ImGui::End();
    }
    
    void cleanup() {
        if (cap_.isOpened()) {
            cap_.release();
        }
        
        if (window_) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <video_file>" << std::endl;
        std::cout << "Or use: " << argv[0] << " camera" << std::endl;
        return -1;
    }
    
    VideoProcessorGUI gui;
    
    if (!gui.initialize(argv[1])) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return -1;
    }
    
    std::cout << "GUI initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - Use the control panels to adjust settings" << std::endl;
    std::cout << "  - Toggle detection and tracking" << std::endl;
    std::cout << "  - Monitor performance in real-time" << std::endl;
    std::cout << "  - Press 'Quit' button or close window to exit" << std::endl;
    
    gui.run();
    
    return 0;
} 