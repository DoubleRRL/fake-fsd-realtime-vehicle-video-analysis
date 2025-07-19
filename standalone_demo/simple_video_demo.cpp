#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

class SimpleVideoPlayer {
private:
    GLFWwindow* window;
    cv::VideoCapture cap;
    cv::Mat currentFrame;
    bool isPlaying;
    int currentFrameIndex;
    double fps;
    int totalFrames;
    std::string currentVideoPath;
    
    // File browser state
    std::string currentDirectory;
    std::vector<std::string> videoFiles;
    int selectedFileIndex;
    bool showFileBrowser;

public:
    SimpleVideoPlayer() : window(nullptr), isPlaying(false), currentFrameIndex(0), 
                         selectedFileIndex(0), showFileBrowser(true) {
        currentDirectory = fs::current_path().string();
        scanForVideoFiles();
    }

    ~SimpleVideoPlayer() {
        if (cap.isOpened()) cap.release();
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool initialize() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        window = glfwCreateWindow(1200, 800, "Simple Video Player", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        return true;
    }

    void scanForVideoFiles() {
        videoFiles.clear();
        for (const auto& entry : fs::directory_iterator(currentDirectory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".mp4" || ext == ".avi" || ext == ".mov" || ext == ".mkv" || 
                    ext == ".wmv" || ext == ".flv" || ext == ".webm") {
                    videoFiles.push_back(entry.path().filename().string());
                }
            }
        }
    }

    bool loadVideo(const std::string& filename) {
        std::string fullPath = currentDirectory + "/" + filename;
        
        if (cap.isOpened()) cap.release();
        
        cap.open(fullPath);
        if (!cap.isOpened()) {
            std::cerr << "Failed to open video: " << fullPath << std::endl;
            return false;
        }

        currentVideoPath = fullPath;
        fps = cap.get(cv::CAP_PROP_FPS);
        totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        currentFrameIndex = 0;
        isPlaying = false;

        std::cout << "Loaded video: " << filename << std::endl;
        std::cout << "FPS: " << fps << ", Total frames: " << totalFrames << std::endl;
        
        return true;
    }

    void renderFileBrowser() {
        if (!showFileBrowser) return;

        // Simple file browser using OpenGL
        glViewport(0, 0, 300, 800);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 300, 800, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Background
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(300, 0);
        glVertex2f(300, 800);
        glVertex2f(0, 800);
        glEnd();

        // File list background
        int y = 60;
        for (int i = 0; i < videoFiles.size(); i++) {
            if (i == selectedFileIndex) {
                glColor3f(0.0f, 0.5f, 1.0f); // Selected file
            } else {
                glColor3f(0.3f, 0.3f, 0.3f); // Normal file
            }
            
            glBegin(GL_QUADS);
            glVertex2f(5, y - 20);
            glVertex2f(295, y - 20);
            glVertex2f(295, y + 5);
            glVertex2f(5, y + 5);
            glEnd();
            y += 30;
        }
    }

    void renderVideo() {
        if (!cap.isOpened()) return;

        glViewport(300, 0, 900, 800);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 900, 800, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Get current frame
        if (isPlaying) {
            cap >> currentFrame;
            if (currentFrame.empty()) {
                isPlaying = false;
                currentFrameIndex = totalFrames - 1;
                cap.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
                cap >> currentFrame;
            } else {
                currentFrameIndex++;
            }
        } else {
            cap.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
            cap >> currentFrame;
        }

        if (!currentFrame.empty()) {
            // Convert BGR to RGB and flip
            cv::Mat rgbFrame;
            cv::cvtColor(currentFrame, rgbFrame, cv::COLOR_BGR2RGB);
            cv::flip(rgbFrame, rgbFrame, 0);

            // Display frame
            glDrawPixels(rgbFrame.cols, rgbFrame.rows, GL_RGB, GL_UNSIGNED_BYTE, rgbFrame.data);
        }

        // Simple info overlay using colored rectangles
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(200, 0);
        glVertex2f(200, 80);
        glVertex2f(0, 80);
        glEnd();
    }

    void handleInput() {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            if (selectedFileIndex > 0) selectedFileIndex--;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            if (selectedFileIndex < videoFiles.size() - 1) selectedFileIndex++;
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            if (!videoFiles.empty()) {
                loadVideo(videoFiles[selectedFileIndex]);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            isPlaying = !isPlaying;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            if (currentFrameIndex > 0) {
                currentFrameIndex--;
                cap.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            if (currentFrameIndex < totalFrames - 1) {
                currentFrameIndex++;
                cap.set(cv::CAP_PROP_POS_FRAMES, currentFrameIndex);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            showFileBrowser = !showFileBrowser;
        }
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);
            
            handleInput();
            renderFileBrowser();
            renderVideo();
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            
            // Control playback speed
            if (isPlaying && fps > 0) {
                glfwWaitEventsTimeout(1.0 / fps);
            } else {
                glfwWaitEventsTimeout(0.016); // ~60 FPS for UI
            }
        }
    }
};

int main() {
    SimpleVideoPlayer player;
    
    if (!player.initialize()) {
        std::cerr << "Failed to initialize video player" << std::endl;
        return -1;
    }

    std::cout << "Simple Video Player" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  UP/DOWN: Select video file" << std::endl;
    std::cout << "  ENTER: Load selected video" << std::endl;
    std::cout << "  SPACE: Play/Pause" << std::endl;
    std::cout << "  LEFT/RIGHT: Step through frames" << std::endl;
    std::cout << "  F: Toggle file browser" << std::endl;
    std::cout << "  ESC: Quit" << std::endl;

    player.run();
    return 0;
} 