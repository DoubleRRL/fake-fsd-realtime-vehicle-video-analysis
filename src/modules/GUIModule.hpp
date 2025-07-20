#pragma once

#include "../core/Types.hpp"
#include "../core/Pipeline.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <Metal/Metal.h>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

class GUIModule {
public:
    explicit GUIModule(std::shared_ptr<Pipeline> pipeline);
    ~GUIModule();

    // Initialize GUI with Metal rendering
    bool initialize(int width, int height, const std::string& title);
    
    // Main GUI loop
    void run();
    
    // Stop GUI
    void stop();
    
    // Configuration
    void setWindowSize(int width, int height);
    void setFullscreen(bool fullscreen);
    void setShowPerformanceOverlay(bool show);
    void setShowDetectionOverlay(bool show);
    
    // Status
    bool isRunning() const { return running_.load(); }
    
    // Error handling
    std::string getLastError() const { return lastError_; }

private:
    // GUI setup
    bool setupGLFW();
    bool setupImGui();
    bool setupMetal();
    bool createRenderTargets();
    
    // Rendering
    void renderFrame();
    void renderVideoFrame();
    void renderDetections();
    void renderPerformanceOverlay();
    void renderControls();
    
    // Event handling
    void handleInput();
    void handleKeyPress(int key, int action);
    void handleMouseMove(double x, double y);
    void handleMouseButton(int button, int action);
    
    // Utility functions
    void updateTexture(const std::shared_ptr<ProcessedFrame>& frame);
    void drawBoundingBox(const Detection& detection);
    void drawTrajectory(const Track& track);
    void drawPerformanceGraph(const std::vector<float>& data, const std::string& label);
    
    // GLFW objects
    GLFWwindow* window_;
    
    // Metal objects
    id<MTLDevice> device_;
    id<MTLCommandQueue> commandQueue_;
    id<MTLTexture> videoTexture_;
    id<MTLRenderPassDescriptor> renderPassDescriptor_;
    
    // ImGui context
    ImGuiContext* imguiContext_;
    
    // Pipeline reference
    std::shared_ptr<Pipeline> pipeline_;
    
    // Configuration
    int windowWidth_;
    int windowHeight_;
    bool fullscreen_;
    bool showPerformanceOverlay_;
    bool showDetectionOverlay_;
    
    // State
    std::atomic<bool> running_;
    std::atomic<bool> shouldStop_;
    std::mutex renderMutex_;
    
    // Performance tracking
    std::vector<float> fpsHistory_;
    std::vector<float> latencyHistory_;
    std::vector<float> cpuHistory_;
    std::vector<float> gpuHistory_;
    
    // Error handling
    mutable std::mutex errorMutex_;
    std::string lastError_;
    
    // Threading
    std::thread renderThread_;
    
    // UI state
    bool showControls_;
    bool showStats_;
    bool showConfig_;
    float zoomLevel_;
    cv::Point2f panOffset_;
    
    // Colors and styling
    struct Colors {
        ImVec4 background = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
        ImVec4 text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 detection = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 warning = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 error = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    } colors_;
}; 