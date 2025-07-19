#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    // Check if video file is provided
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <video_file>" << std::endl;
        std::cout << "Or use: " << argv[0] << " camera" << std::endl;
        return -1;
    }

    cv::VideoCapture cap;
    
    // Open video file or camera
    if (std::string(argv[1]) == "camera") {
        cap.open(0); // Open default camera
        std::cout << "Opening camera..." << std::endl;
    } else {
        cap.open(argv[1]); // Open video file
        std::cout << "Opening video file: " << argv[1] << std::endl;
    }

    if (!cap.isOpened()) {
        std::cout << "Error: Could not open video source!" << std::endl;
        return -1;
    }

    // Get video properties
    double fps = cap.get(cv::CAP_PROP_FPS);
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    std::cout << "Video properties:" << std::endl;
    std::cout << "  Resolution: " << width << "x" << height << std::endl;
    std::cout << "  FPS: " << fps << std::endl;

    cv::Mat frame;
    int frameCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::cout << "\nPress 'q' to quit, 's' to save current frame" << std::endl;

    while (true) {
        cap >> frame;
        
        if (frame.empty()) {
            std::cout << "End of video or camera disconnected" << std::endl;
            break;
        }

        frameCount++;
        
        // Calculate current FPS
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        double currentFPS = (frameCount * 1000.0) / elapsed.count();

        // Add text overlay with frame info
        std::string info = "Frame: " + std::to_string(frameCount) + 
                          " | FPS: " + std::to_string(static_cast<int>(currentFPS));
        
        cv::putText(frame, info, cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        // Show the frame
        cv::imshow("Real-Time Video Processor", frame);

        // Handle key presses
        char key = cv::waitKey(1) & 0xFF;
        if (key == 'q') {
            std::cout << "Quitting..." << std::endl;
            break;
        } else if (key == 's') {
            std::string filename = "frame_" + std::to_string(frameCount) + ".jpg";
            cv::imwrite(filename, frame);
            std::cout << "Saved frame to: " << filename << std::endl;
        }
    }

    // Clean up
    cap.release();
    cv::destroyAllWindows();

    // Print final statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\nProcessing complete!" << std::endl;
    std::cout << "Total frames processed: " << frameCount << std::endl;
    std::cout << "Total time: " << totalTime.count() / 1000.0 << " seconds" << std::endl;
    std::cout << "Average FPS: " << (frameCount * 1000.0) / totalTime.count() << std::endl;

    return 0;
} 