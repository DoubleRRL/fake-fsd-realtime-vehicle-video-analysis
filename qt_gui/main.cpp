#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileSystemModel>
#include <QTreeView>

#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QProgressBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>

#include "detection_tracker.h"

class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    VideoPlayerWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        setupVideoTimer();
    }

    void loadVideo(const QString& filePath) {
        if (videoCapture.isOpened()) {
            videoCapture.release();
        }

        videoCapture.open(filePath.toStdString());
        if (!videoCapture.isOpened()) {
            QMessageBox::warning(this, "Error", "Could not open video file: " + filePath);
            return;
        }

        // Get video properties
        totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
        fps = videoCapture.get(cv::CAP_PROP_FPS);
        frameWidth = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
        frameHeight = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));

        // Initialize detection and tracking
        initializeDetection();

        // Update UI
        frameSlider->setMaximum(totalFrames - 1);
        frameSlider->setValue(0);
        currentFrame = 0;
        isPlaying = false;

        // Update labels
        updateFrameInfo();
        
        // Load first frame
        loadCurrentFrame();
    }

    void playPause() {
        if (!videoCapture.isOpened()) return;
        
        isPlaying = !isPlaying;
        playButton->setText(isPlaying ? "⏸️ Pause" : "▶️ Play");
        
        if (isPlaying) {
            videoTimer->start(1000.0 / fps);
        } else {
            videoTimer->stop();
        }
    }

    void stepForward() {
        if (!videoCapture.isOpened()) return;
        
        if (currentFrame < totalFrames - 1) {
            currentFrame++;
            frameSlider->setValue(currentFrame);
            loadCurrentFrame();
            updateFrameInfo();
        }
    }

    void stepBackward() {
        if (!videoCapture.isOpened()) return;
        
        if (currentFrame > 0) {
            currentFrame--;
            frameSlider->setValue(currentFrame);
            loadCurrentFrame();
            updateFrameInfo();
        }
    }

    void setFrame(int frame) {
        if (!videoCapture.isOpened()) return;
        
        currentFrame = frame;
        loadCurrentFrame();
        updateFrameInfo();
    }

    void setShowAnnotations(bool show) {
        showAnnotations = show;
        if (videoCapture.isOpened()) {
            loadCurrentFrame();
        }
    }

    void setConfidenceThreshold(double threshold) {
        confidenceThreshold = threshold;
        if (videoCapture.isOpened()) {
            loadCurrentFrame();
        }
    }

signals:
    void frameChanged(int frame);
    void fpsChanged(double fps);

private slots:
    void onVideoTimer() {
        if (currentFrame < totalFrames - 1) {
            currentFrame++;
            frameSlider->setValue(currentFrame);
            loadCurrentFrame();
            updateFrameInfo();
            emit frameChanged(currentFrame);
        } else {
            // End of video
            isPlaying = false;
            playButton->setText("▶️ Play");
            videoTimer->stop();
        }
    }

    void onFrameSliderChanged(int value) {
        if (!videoCapture.isOpened()) return;
        
        currentFrame = value;
        loadCurrentFrame();
        updateFrameInfo();
        emit frameChanged(currentFrame);
    }

private:
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Video display area
        videoLabel = new QLabel("No video loaded");
        videoLabel->setAlignment(Qt::AlignCenter);
        videoLabel->setMinimumSize(640, 480);
        videoLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #cccccc; border: 1px solid #555555; }");
        mainLayout->addWidget(videoLabel);
        
        // Controls
        QHBoxLayout* controlsLayout = new QHBoxLayout();
        
        // Playback controls
        playButton = new QPushButton("▶️ Play");
        QPushButton* stepBackButton = new QPushButton("⏮️ Step Back");
        QPushButton* stepForwardButton = new QPushButton("⏭️ Step Forward");
        
        controlsLayout->addWidget(playButton);
        controlsLayout->addWidget(stepBackButton);
        controlsLayout->addWidget(stepForwardButton);
        
        // Frame slider
        frameSlider = new QSlider(Qt::Horizontal);
        frameSlider->setEnabled(false);
        controlsLayout->addWidget(frameSlider);
        
        // Frame info
        frameInfoLabel = new QLabel("Frame: 0 / 0 | FPS: 0.0");
        controlsLayout->addWidget(frameInfoLabel);
        
        mainLayout->addLayout(controlsLayout);
        
        // Connect signals
        connect(playButton, &QPushButton::clicked, this, &VideoPlayerWidget::playPause);
        connect(stepBackButton, &QPushButton::clicked, this, &VideoPlayerWidget::stepBackward);
        connect(stepForwardButton, &QPushButton::clicked, this, &VideoPlayerWidget::stepForward);
        connect(frameSlider, &QSlider::valueChanged, this, &VideoPlayerWidget::onFrameSliderChanged);
    }

    void setupVideoTimer() {
        videoTimer = new QTimer(this);
        connect(videoTimer, &QTimer::timeout, this, &VideoPlayerWidget::onVideoTimer);
    }

    void initializeDetection() {
        if (detection_initialized_) return;
        
        detector_ = std::make_unique<DetectionTracker>();
        
        // Try to initialize with a default model (will use dummy detections if not found)
        std::string model_path = "models/yolov8n.onnx";
        std::string config_path = "";
        std::string classes_path = "models/coco.names";
        
        if (!detector_->initialize(model_path, config_path, classes_path, confidenceThreshold, 0.4)) {
            std::cout << "Warning: Could not load YOLO model, using dummy detections" << std::endl;
            // Create a dummy detector that will work without model files
            detection_initialized_ = true;
        } else {
            std::cout << "Detection and tracking initialized successfully" << std::endl;
            detection_initialized_ = true;
        }
    }

    void loadCurrentFrame() {
        if (!videoCapture.isOpened()) return;
        
        cv::Mat frame;
        videoCapture.set(cv::CAP_PROP_POS_FRAMES, currentFrame);
        videoCapture >> frame;
        
        if (frame.empty()) return;
        
        // Run detection and tracking if enabled
        if (showAnnotations && detection_initialized_ && detector_) {
            current_tracked_objects_ = detector_->processFrame(frame);
            drawDetections(frame, current_tracked_objects_);
        }
        
        // Convert to Qt format
        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
        
        QImage qimg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(qimg);
        
        // Scale to fit the label while maintaining aspect ratio
        pixmap = pixmap.scaled(videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        videoLabel->setPixmap(pixmap);
    }

    void drawDetections(cv::Mat& frame, const std::vector<TrackedObject>& objects) {
        for (const auto& obj : objects) {
            // Draw bounding box
            cv::Scalar color = cv::Scalar(0, 255, 0); // Green for vehicles
            if (obj.class_name == "car" || obj.class_name == "truck" || obj.class_name == "bus") {
                color = cv::Scalar(0, 255, 0); // Green
            } else if (obj.class_name == "person") {
                color = cv::Scalar(255, 0, 0); // Blue
            } else {
                color = cv::Scalar(0, 0, 255); // Red
            }
            
            cv::rectangle(frame, obj.bbox, color, 2);
            
            // Draw label with track ID and class
            std::string label = obj.class_name + " #" + std::to_string(obj.track_id);
            if (obj.confidence > 0) {
                label += " (" + std::to_string(static_cast<int>(obj.confidence * 100)) + "%)";
            }
            
            int baseline = 0;
            cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
            cv::rectangle(frame, cv::Point(obj.bbox.x, obj.bbox.y - text_size.height - 10),
                         cv::Point(obj.bbox.x + text_size.width, obj.bbox.y), color, -1);
            cv::putText(frame, label, cv::Point(obj.bbox.x, obj.bbox.y - 5),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
    }

    void updateFrameInfo() {
        QString info = QString("Frame: %1 / %2 | FPS: %3")
                      .arg(currentFrame + 1)
                      .arg(totalFrames)
                      .arg(fps, 0, 'f', 1);
        frameInfoLabel->setText(info);
    }

    // Video capture and properties
    cv::VideoCapture videoCapture;
    int totalFrames = 0;
    double fps = 30.0;
    int frameWidth = 0;
    int frameHeight = 0;
    int currentFrame = 0;
    bool isPlaying = false;
    bool showAnnotations = false;
    double confidenceThreshold = 0.5;
    
public:
    // Detection and tracking
    std::unique_ptr<DetectionTracker> detector_;
    std::vector<TrackedObject> current_tracked_objects_;
    bool detection_initialized_ = false;

private:

    // UI elements
    QLabel* videoLabel;
    QPushButton* playButton;
    QSlider* frameSlider;
    QLabel* frameInfoLabel;
    QTimer* videoTimer;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupMenuBar();
        setupStatusBar();
        loadSettings();
    }

    ~MainWindow() {
        saveSettings();
    }

private slots:
    void openFile() {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            "Open Video File",
            lastDirectory,
            "Video Files (*.mp4 *.avi *.mov *.mkv *.wmv *.flv *.webm);;All Files (*)"
        );
        
        if (!filePath.isEmpty()) {
            lastDirectory = QFileInfo(filePath).absolutePath();
            videoPlayer->loadVideo(filePath);
            setWindowTitle("Professional Video Analysis - " + QFileInfo(filePath).fileName());
        }
    }

    void openDirectory() {
        QString dirPath = QFileDialog::getExistingDirectory(
            this,
            "Open Directory",
            lastDirectory,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        
        if (!dirPath.isEmpty()) {
            lastDirectory = dirPath;
            fileSystemModel->setRootPath(dirPath);
            fileTreeView->setRootIndex(fileSystemModel->index(dirPath));
        }
    }

    void onFileSelected(const QModelIndex& index) {
        QString filePath = fileSystemModel->filePath(index);
        if (QFileInfo(filePath).isFile()) {
            QStringList videoExtensions = {"mp4", "avi", "mov", "mkv", "wmv", "flv", "webm"};
            QString extension = QFileInfo(filePath).suffix().toLower();
            
            if (videoExtensions.contains(extension)) {
                videoPlayer->loadVideo(filePath);
                setWindowTitle("Professional Video Analysis - " + QFileInfo(filePath).fileName());
            }
        }
    }

    void onShowAnnotationsChanged(bool checked) {
        videoPlayer->setShowAnnotations(checked);
    }

    void onConfidenceThresholdChanged(double value) {
        videoPlayer->setConfidenceThreshold(value);
    }

    void updatePerformanceMetrics() {
        DetectionTracker* detector = getDetector();
        if (detector) {
            fpsLabel->setText(QString("FPS: %1").arg(detector->getFPS(), 0, 'f', 1));
            latencyLabel->setText(QString("Detection: %1ms | Tracking: %2ms")
                                .arg(detector->getDetectionTime(), 0, 'f', 1)
                                .arg(detector->getTrackingTime(), 0, 'f', 1));
            frameCountLabel->setText(QString("Active Tracks: %1")
                                   .arg(detector->getActiveTracks()));
        }
    }

private:
    void setupUI() {
        setWindowTitle("Professional Video Analysis");
        setMinimumSize(1200, 800);
        
        // Create central widget
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // Create main layout
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        
        // Create splitter for resizable panels
        QSplitter* splitter = new QSplitter(Qt::Horizontal);
        mainLayout->addWidget(splitter);
        
        // Left panel: File browser
        QWidget* leftPanel = new QWidget;
        QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
        
        QLabel* fileBrowserLabel = new QLabel("File Browser");
        fileBrowserLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; padding: 5px; }");
        leftLayout->addWidget(fileBrowserLabel);
        
        fileSystemModel = new QFileSystemModel(this);
        fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        
        fileTreeView = new QTreeView;
        fileTreeView->setModel(fileSystemModel);
        fileTreeView->setRootIndex(fileSystemModel->index(QDir::homePath()));
        fileTreeView->setMaximumWidth(300);
        leftLayout->addWidget(fileTreeView);
        
        splitter->addWidget(leftPanel);
        
        // Center panel: Video player
        videoPlayer = new VideoPlayerWidget;
        splitter->addWidget(videoPlayer);
        
        // Right panel: Controls and stats
        QWidget* rightPanel = new QWidget;
        QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
        rightPanel->setMaximumWidth(250);
        
        // Controls group
        QGroupBox* controlsGroup = new QGroupBox("Controls");
        QVBoxLayout* controlsLayout = new QVBoxLayout(controlsGroup);
        
        showAnnotationsCheckBox = new QCheckBox("Show Annotations");
        controlsLayout->addWidget(showAnnotationsCheckBox);
        
        QLabel* confidenceLabel = new QLabel("Confidence Threshold:");
        confidenceSpinBox = new QDoubleSpinBox;
        confidenceSpinBox->setRange(0.0, 1.0);
        confidenceSpinBox->setSingleStep(0.1);
        confidenceSpinBox->setValue(0.5);
        confidenceSpinBox->setDecimals(2);
        controlsLayout->addWidget(confidenceLabel);
        controlsLayout->addWidget(confidenceSpinBox);
        
        rightLayout->addWidget(controlsGroup);
        
        // Performance group
        QGroupBox* performanceGroup = new QGroupBox("Performance");
        QVBoxLayout* performanceLayout = new QVBoxLayout(performanceGroup);
        
        fpsLabel = new QLabel("FPS: 0.0");
        latencyLabel = new QLabel("Latency: 0ms");
        frameCountLabel = new QLabel("Frame Count: 0");
        
        performanceLayout->addWidget(fpsLabel);
        performanceLayout->addWidget(latencyLabel);
        performanceLayout->addWidget(frameCountLabel);
        
        rightLayout->addWidget(performanceGroup);
        
        // Add stretch to push everything to the top
        rightLayout->addStretch();
        
        splitter->addWidget(rightPanel);
        
        // Set splitter proportions
        splitter->setSizes({300, 800, 250});
        
        // Connect signals
        connect(fileTreeView, &QTreeView::doubleClicked, this, &MainWindow::onFileSelected);
        connect(showAnnotationsCheckBox, &QCheckBox::toggled, this, &MainWindow::onShowAnnotationsChanged);
        connect(confidenceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                this, &MainWindow::onConfidenceThresholdChanged);
        
        // Setup performance monitoring timer
        performanceTimer = new QTimer(this);
        connect(performanceTimer, &QTimer::timeout, this, &MainWindow::updatePerformanceMetrics);
        performanceTimer->start(100); // Update every 100ms
    }

    void setupMenuBar() {
        QMenuBar* menuBar = this->menuBar();
        
        // File menu
        QMenu* fileMenu = menuBar->addMenu("&File");
        
        QAction* openFileAction = new QAction("&Open Video File", this);
        openFileAction->setShortcut(QKeySequence::Open);
        connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
        fileMenu->addAction(openFileAction);
        
        QAction* openDirectoryAction = new QAction("Open &Directory", this);
        connect(openDirectoryAction, &QAction::triggered, this, &MainWindow::openDirectory);
        fileMenu->addAction(openDirectoryAction);
        
        fileMenu->addSeparator();
        
        QAction* exitAction = new QAction("E&xit", this);
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QApplication::quit);
        fileMenu->addAction(exitAction);
        
        // Help menu
        QMenu* helpMenu = menuBar->addMenu("&Help");
        
        QAction* aboutAction = new QAction("&About", this);
        connect(aboutAction, &QAction::triggered, [this]() {
            QMessageBox::about(this, "About", 
                "Professional Video Analysis GUI\n\n"
                "A Qt-based application for real-time video analysis with annotation support.\n\n"
                "Features:\n"
                "- File browser for easy video selection\n"
                "- Real-time video playback\n"
                "- Annotation overlay support\n"
                "- Performance monitoring\n"
                "- UA-DETRAC dataset support");
        });
        helpMenu->addAction(aboutAction);
    }

    void setupStatusBar() {
        statusBar()->showMessage("Ready");
    }

    void loadSettings() {
        QSettings settings;
        lastDirectory = settings.value("lastDirectory", QDir::homePath()).toString();
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

    void saveSettings() {
        QSettings settings;
        settings.setValue("lastDirectory", lastDirectory);
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }

public:
    VideoPlayerWidget* videoPlayer;
    DetectionTracker* getDetector() const { return videoPlayer ? videoPlayer->detector_.get() : nullptr; }

private:
    // UI elements
    QFileSystemModel* fileSystemModel;
    QTreeView* fileTreeView;
    QCheckBox* showAnnotationsCheckBox;
    QDoubleSpinBox* confidenceSpinBox;
    QLabel* fpsLabel;
    QLabel* latencyLabel;
    QLabel* frameCountLabel;
    QTimer* performanceTimer;
    
    // Settings
    QString lastDirectory;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Professional Video Analysis");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Video Analysis Team");
    
    // Set dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    // Load video file if provided as command line argument
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        if (QFileInfo(filePath).exists()) {
            window.videoPlayer->loadVideo(filePath);
            window.setWindowTitle("Professional Video Analysis - " + QFileInfo(filePath).fileName());
        }
    }
    
    return app.exec();
}

#include "main.moc" 