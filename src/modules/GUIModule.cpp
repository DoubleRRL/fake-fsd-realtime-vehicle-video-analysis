#include "GUIModule.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>

GUIModule::GUIModule(std::shared_ptr<Pipeline> pipeline)
    : pipeline_(pipeline)
    , window_(nullptr)
    , device_(nullptr)
    , commandQueue_(nullptr)
    , videoTexture_(nullptr)
    , renderPassDescriptor_(nullptr)
    , imguiContext_(nullptr)
    , windowWidth_(1280)
    , windowHeight_(720)
    , fullscreen_(false)
    , showPerformanceOverlay_(true)
    , showDetectionOverlay_(true)
    , running_(false)
    , shouldStop_(false)
    , showControls_(true)
    , showStats_(true)
    , showConfig_(false)
    , zoomLevel_(1.0f)
    , panOffset_(0.0f, 0.0f) {
}

GUIModule::~GUIModule() {
    stop();
}

bool GUIModule::initialize(int width, int height, const std::string& title) {
    windowWidth_ = width;
    windowHeight_ = height;
    
    if (!setupGLFW()) {
        return false;
    }
    
    if (!setupMetal()) {
        return false;
    }
    
    if (!setupImGui()) {
        return false;
    }
    
    if (!createRenderTargets()) {
        return false;
    }
    
    running_ = true;
    return true;
}

bool GUIModule::setupGLFW() {
    if (!glfwInit()) {
        lastError_ = "Failed to initialize GLFW";
        return false;
    }
    
    // Configure GLFW for Metal
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "Real-time Car Vision", nullptr, nullptr);
    if (!window_) {
        lastError_ = "Failed to create GLFW window";
        return false;
    }
    
    // Set up callbacks
    glfwSetWindowUserPointer(window_, this);
    
    glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* gui = static_cast<GUIModule*>(glfwGetWindowUserPointer(window));
        gui->handleKeyPress(key, action);
    });
    
    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double x, double y) {
        auto* gui = static_cast<GUIModule*>(glfwGetWindowUserPointer(window));
        gui->handleMouseMove(x, y);
    });
    
    glfwSetMouseButtonCallback(window_, [](GLFWwindow* window, int button, int action, int mods) {
        auto* gui = static_cast<GUIModule*>(glfwGetWindowUserPointer(window));
        gui->handleMouseButton(button, action);
    });
    
    return true;
}

bool GUIModule::setupMetal() {
    device_ = MTLCreateSystemDefaultDevice();
    if (!device_) {
        lastError_ = "Failed to create Metal device";
        return false;
    }
    
    commandQueue_ = [device_ newCommandQueue];
    if (!commandQueue_) {
        lastError_ = "Failed to create Metal command queue";
        return false;
    }
    
    return true;
}

bool GUIModule::setupImGui() {
    IMGUI_CHECKVERSION();
    imguiContext_ = ImGui::CreateContext();
    if (!imguiContext_) {
        lastError_ = "Failed to create ImGui context";
        return false;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Set up ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    
    // Initialize ImGui for Metal
    // Note: This is a simplified implementation
    // Full implementation would require Metal-specific ImGui backend
    
    return true;
}

bool GUIModule::createRenderTargets() {
    // Create video texture
    MTLTextureDescriptor* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                           width:windowWidth_
                                                                                          height:windowHeight_
                                                                                       mipmapped:NO];
    videoTexture_ = [device_ newTextureWithDescriptor:textureDesc];
    if (!videoTexture_) {
        lastError_ = "Failed to create video texture";
        return false;
    }
    
    // Create render pass descriptor
    renderPassDescriptor_ = [MTLRenderPassDescriptor renderPassDescriptor];
    if (!renderPassDescriptor_) {
        lastError_ = "Failed to create render pass descriptor";
        return false;
    }
    
    return true;
}

void GUIModule::run() {
    while (!shouldStop_.load() && !glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        
        // Start ImGui frame
        ImGui_ImplMetal_NewFrame(renderPassDescriptor_);
        ImGui::NewFrame();
        
        // Render GUI
        renderFrame();
        
        // Render ImGui
        ImGui::Render();
        
        // Present frame
        // Note: This is simplified - full implementation would use Metal rendering
        
        // Update performance history
        updatePerformanceHistory();
        
        // Small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void GUIModule::stop() {
    if (!running_.load()) {
        return;
    }
    
    shouldStop_ = true;
    
    if (renderThread_.joinable()) {
        renderThread_.join();
    }
    
    // Cleanup
    if (imguiContext_) {
        ImGui::DestroyContext(imguiContext_);
    }
    
    if (window_) {
        glfwDestroyWindow(window_);
    }
    
    glfwTerminate();
    
    running_ = false;
}

void GUIModule::renderFrame() {
    // Main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowWidth_, windowHeight_));
    
    ImGui::Begin("Real-time Car Vision", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    // Render video frame
    renderVideoFrame();
    
    // Render detections
    if (showDetectionOverlay_) {
        renderDetections();
    }
    
    // Render performance overlay
    if (showPerformanceOverlay_) {
        renderPerformanceOverlay();
    }
    
    // Render controls
    if (showControls_) {
        renderControls();
    }
    
    ImGui::End();
}

void GUIModule::renderVideoFrame() {
    // Get latest frame from pipeline
    auto latestResult = pipeline_->getLatestResult();
    if (latestResult && !latestResult->detections.empty()) {
        // Update texture with latest frame
        // This is a simplified implementation
        // In a full implementation, you would copy frame data to Metal texture
        
        // Display the texture
        ImGui::Image((void*)videoTexture_, ImVec2(windowWidth_ * 0.8f, windowHeight_ * 0.8f));
    } else {
        // Show placeholder
        ImGui::TextColored(colors_.text, "No video frame available");
    }
}

void GUIModule::renderDetections() {
    auto latestResult = pipeline_->getLatestResult();
    if (!latestResult) {
        return;
    }
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    for (const auto& detection : latestResult->detections) {
        // Convert detection bbox to screen coordinates
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        ImVec2 bboxMin(screenPos.x + detection.bbox.x * zoomLevel_ + panOffset_.x,
                      screenPos.y + detection.bbox.y * zoomLevel_ + panOffset_.y);
        ImVec2 bboxMax(screenPos.x + (detection.bbox.x + detection.bbox.width) * zoomLevel_ + panOffset_.x,
                      screenPos.y + (detection.bbox.y + detection.bbox.height) * zoomLevel_ + panOffset_.y);
        
        // Draw bounding box
        drawList->AddRect(bboxMin, bboxMax, ImGui::ColorConvertFloat4ToU32(colors_.detection), 2.0f);
        
        // Draw label
        std::string label = detection.className + " " + std::to_string(static_cast<int>(detection.confidence * 100)) + "%";
        ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        drawList->AddRectFilled(ImVec2(bboxMin.x, bboxMin.y - textSize.y - 4),
                               ImVec2(bboxMin.x + textSize.x + 8, bboxMin.y),
                               ImGui::ColorConvertFloat4ToU32(colors_.detection));
        drawList->AddText(ImVec2(bboxMin.x + 4, bboxMin.y - textSize.y - 2),
                         ImGui::ColorConvertFloat4ToU32(colors_.text),
                         label.c_str());
    }
}

void GUIModule::renderPerformanceOverlay() {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(300, 200));
    
    ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // Get pipeline stats
    auto stats = pipeline_->getStats();
    
    // Display current FPS
    ImGui::TextColored(colors_.text, "FPS: %.1f", stats.currentFPS);
    
    // Display latency
    ImGui::TextColored(colors_.text, "Latency: %.2f ms", stats.averageLatency / 1000.0f);
    
    // Display CPU/GPU usage
    ImGui::TextColored(colors_.text, "CPU: %.1f%%", stats.performanceStats.cpuUsage);
    ImGui::TextColored(colors_.text, "GPU: %.1f%%", stats.performanceStats.gpuUsage);
    
    // Display memory usage
    ImGui::TextColored(colors_.text, "Memory: %.1f MB", stats.performanceStats.memoryUsage / 1024.0f / 1024.0f);
    
    // Performance graphs
    if (!fpsHistory_.empty()) {
        ImGui::PlotLines("FPS History", fpsHistory_.data(), fpsHistory_.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(280, 60));
    }
    
    if (!latencyHistory_.empty()) {
        ImGui::PlotLines("Latency History", latencyHistory_.data(), latencyHistory_.size(), 0, nullptr, 0.0f, 50.0f, ImVec2(280, 60));
    }
    
    ImGui::End();
}

void GUIModule::renderControls() {
    ImGui::SetNextWindowPos(ImVec2(windowWidth_ - 310, 10));
    ImGui::SetNextWindowSize(ImVec2(300, 400));
    
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // Pipeline controls
    if (ImGui::CollapsingHeader("Pipeline", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (pipeline_->isRunning()) {
            if (ImGui::Button("Stop Pipeline")) {
                pipeline_->stop();
            }
        } else {
            if (ImGui::Button("Start Pipeline")) {
                pipeline_->start();
            }
        }
        
        ImGui::Separator();
        
        // Configuration
        auto config = pipeline_->getConfig();
        
        bool configChanged = false;
        
        float confidence = config.confidenceThreshold;
        if (ImGui::SliderFloat("Confidence Threshold", &confidence, 0.0f, 1.0f)) {
            config.confidenceThreshold = confidence;
            configChanged = true;
        }
        
        float nms = config.nmsThreshold;
        if (ImGui::SliderFloat("NMS Threshold", &nms, 0.0f, 1.0f)) {
            config.nmsThreshold = nms;
            configChanged = true;
        }
        
        int maxDetections = config.maxDetections;
        if (ImGui::SliderInt("Max Detections", &maxDetections, 1, 200)) {
            config.maxDetections = maxDetections;
            configChanged = true;
        }
        
        if (configChanged) {
            pipeline_->updateConfig(config);
        }
    }
    
    // Display controls
    if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Performance Overlay", &showPerformanceOverlay_);
        ImGui::Checkbox("Show Detection Overlay", &showDetectionOverlay_);
        
        ImGui::SliderFloat("Zoom Level", &zoomLevel_, 0.1f, 3.0f);
        
        if (ImGui::Button("Reset View")) {
            zoomLevel_ = 1.0f;
            panOffset_ = cv::Point2f(0.0f, 0.0f);
        }
    }
    
    // Statistics
    if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto stats = pipeline_->getStats();
        
        ImGui::Text("Total Frames: %llu", stats.totalFrames);
        ImGui::Text("Average Detections: %.1f", stats.detectionStats.averageDetectionsPerFrame);
        ImGui::Text("Detection Time: %.2f ms", stats.detectionStats.averageDetectionTime / 1000.0f);
        ImGui::Text("Preprocessing Time: %.2f ms", stats.preprocessingStats.averageProcessingTime / 1000.0f);
    }
    
    ImGui::End();
}

void GUIModule::handleKeyPress(int key, int action) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                shouldStop_ = true;
                break;
            case GLFW_KEY_SPACE:
                if (pipeline_->isRunning()) {
                    pipeline_->stop();
                } else {
                    pipeline_->start();
                }
                break;
            case GLFW_KEY_P:
                showPerformanceOverlay_ = !showPerformanceOverlay_;
                break;
            case GLFW_KEY_D:
                showDetectionOverlay_ = !showDetectionOverlay_;
                break;
            case GLFW_KEY_C:
                showControls_ = !showControls_;
                break;
            case GLFW_KEY_F:
                setFullscreen(!fullscreen_);
                break;
        }
    }
}

void GUIModule::handleMouseMove(double x, double y) {
    // Handle mouse movement for panning
    // Implementation depends on specific requirements
}

void GUIModule::handleMouseButton(int button, int action) {
    // Handle mouse button events
    // Implementation depends on specific requirements
}

void GUIModule::updatePerformanceHistory() {
    auto stats = pipeline_->getStats();
    
    // Update FPS history
    fpsHistory_.push_back(stats.currentFPS);
    if (fpsHistory_.size() > 100) {
        fpsHistory_.erase(fpsHistory_.begin());
    }
    
    // Update latency history
    latencyHistory_.push_back(stats.averageLatency / 1000.0f);
    if (latencyHistory_.size() > 100) {
        latencyHistory_.erase(latencyHistory_.begin());
    }
    
    // Update CPU/GPU history
    cpuHistory_.push_back(stats.performanceStats.cpuUsage);
    if (cpuHistory_.size() > 100) {
        cpuHistory_.erase(cpuHistory_.begin());
    }
    
    gpuHistory_.push_back(stats.performanceStats.gpuUsage);
    if (gpuHistory_.size() > 100) {
        gpuHistory_.erase(gpuHistory_.begin());
    }
}

void GUIModule::setWindowSize(int width, int height) {
    windowWidth_ = width;
    windowHeight_ = height;
    
    if (window_) {
        glfwSetWindowSize(window_, width, height);
    }
}

void GUIModule::setFullscreen(bool fullscreen) {
    fullscreen_ = fullscreen;
    
    if (window_) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        if (fullscreen) {
            glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window_, nullptr, 100, 100, windowWidth_, windowHeight_, 0);
        }
    }
}

void GUIModule::setShowPerformanceOverlay(bool show) {
    showPerformanceOverlay_ = show;
}

void GUIModule::setShowDetectionOverlay(bool show) {
    showDetectionOverlay_ = show;
} 