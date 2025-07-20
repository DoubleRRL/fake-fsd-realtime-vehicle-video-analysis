#!/usr/bin/env python3
"""
Vehicle Detection GUI - Single, Robust Version
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import cv2
from PIL import Image, ImageTk
import threading
import time
from pathlib import Path
import psutil
import os
import sys

# Import our vehicle detector
from vehicle_detector import VehicleDetector

class VehicleGUI:
    def __init__(self):
        # Create root window
        self.root = tk.Tk()
        self.root.title("Vehicle Detection System")
        self.root.geometry("1200x800")
        
        # Force window to front on macOS
        self.root.lift()
        self.root.attributes('-topmost', True)
        self.root.after_idle(self.root.attributes, '-topmost', False)
        
        # Performance settings
        self.performance_settings = {
            'threads': tk.IntVar(value=1),
            'downsample': tk.BooleanVar(value=True),
            'downsample_factor': tk.DoubleVar(value=0.5),
            'gpu_acceleration': tk.BooleanVar(value=False)
        }
        
        # Detection settings
        self.detection_settings = {
            'model': tk.StringVar(value="n"),
            'confidence': tk.DoubleVar(value=0.5),
            'iou_threshold': tk.DoubleVar(value=0.45)
        }
        
        # Model descriptions
        self.model_descriptions = {
            "n": "Nano (6.7M params) - Fastest, lowest accuracy",
            "s": "Small (22.1M params) - Good speed/accuracy balance", 
            "m": "Medium (52.2M params) - Better accuracy, slower",
            "l": "Large (87.7M params) - High accuracy, slower",
            "x": "XLarge (189.9M params) - Best accuracy, slowest"
        }
        
        # Initialize components
        self.detector = None
        self.video_path = None
        self.cap = None
        self.is_playing = False
        self.current_frame = 0
        self.total_frames = 0
        self.fps = 30
        self.processing_times = []
        
        # Set OpenCV threading to single thread for stability
        cv2.setNumThreads(1)
        
        self.setup_ui()
        self.initialize_detector()
        self.start_performance_monitor()
        
        # Ensure window is visible
        self.root.deiconify()
        self.root.focus_force()
    
    def setup_ui(self):
        # Main container
        main_container = ttk.Frame(self.root, padding="10")
        main_container.pack(fill=tk.BOTH, expand=True)
        
        # Left panel - Controls
        left_panel = ttk.Frame(main_container)
        left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 10))
        
        # Right panel - Video display
        right_panel = ttk.Frame(main_container)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        
        self.setup_controls(left_panel)
        self.setup_video_display(right_panel)
        self.setup_performance_panel(left_panel)
    
    def setup_controls(self, parent):
        # File controls
        file_frame = ttk.LabelFrame(parent, text="Video Controls", padding="5")
        file_frame.pack(fill=tk.X, pady=(0, 10))
        
        ttk.Button(file_frame, text="Open Video", command=self.open_video).pack(fill=tk.X, pady=2)
        ttk.Button(file_frame, text="Play/Pause", command=self.toggle_playback).pack(fill=tk.X, pady=2)
        ttk.Button(file_frame, text="Stop", command=self.stop_video).pack(fill=tk.X, pady=2)
        
        # Frame navigation
        frame_frame = ttk.LabelFrame(parent, text="Frame Navigation", padding="5")
        frame_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.frame_var = tk.IntVar()
        self.frame_slider = ttk.Scale(frame_frame, from_=0, to=100, variable=self.frame_var,
                                     orient=tk.HORIZONTAL, length=200)
        self.frame_slider.pack(fill=tk.X, pady=2)
        self.frame_slider.bind("<ButtonRelease-1>", self.on_frame_change)
        
        self.frame_label = ttk.Label(frame_frame, text="Frame: 0/0")
        self.frame_label.pack()
        
        # Model selection
        model_frame = ttk.LabelFrame(parent, text="Model Selection", padding="5")
        model_frame.pack(fill=tk.X, pady=(0, 10))
        
        model_combo = ttk.Combobox(model_frame, textvariable=self.detection_settings['model'],
                                  values=list(self.model_descriptions.keys()), width=8)
        model_combo.pack(fill=tk.X, pady=2)
        model_combo.bind("<<ComboboxSelected>>", self.on_model_change)
        
        self.model_desc_label = ttk.Label(model_frame, text=self.model_descriptions["n"], 
                                         wraplength=200, justify=tk.LEFT)
        self.model_desc_label.pack(fill=tk.X, pady=2)
        
        # Detection settings
        detection_frame = ttk.LabelFrame(parent, text="Detection Settings", padding="5")
        detection_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Confidence with value display
        conf_frame = ttk.Frame(detection_frame)
        conf_frame.pack(fill=tk.X, pady=2)
        ttk.Label(conf_frame, text="Confidence:").pack(side=tk.LEFT)
        self.conf_label = ttk.Label(conf_frame, text="0.50")
        self.conf_label.pack(side=tk.RIGHT)
        
        conf_scale = ttk.Scale(detection_frame, from_=0.1, to=1.0, 
                              variable=self.detection_settings['confidence'],
                              orient=tk.HORIZONTAL, length=200)
        conf_scale.pack(fill=tk.X, pady=2)
        conf_scale.bind("<ButtonRelease-1>", self.on_conf_change)
        conf_scale.bind("<B1-Motion>", self.on_conf_change)
        
        # IOU threshold
        iou_frame = ttk.Frame(detection_frame)
        iou_frame.pack(fill=tk.X, pady=2)
        ttk.Label(iou_frame, text="IOU Threshold:").pack(side=tk.LEFT)
        self.iou_label = ttk.Label(iou_frame, text="0.45")
        self.iou_label.pack(side=tk.RIGHT)
        
        iou_scale = ttk.Scale(detection_frame, from_=0.1, to=1.0,
                             variable=self.detection_settings['iou_threshold'],
                             orient=tk.HORIZONTAL, length=200)
        iou_scale.pack(fill=tk.X, pady=2)
        iou_scale.bind("<ButtonRelease-1>", self.on_iou_change)
        iou_scale.bind("<B1-Motion>", self.on_iou_change)
    
    def setup_performance_panel(self, parent):
        # Performance settings
        perf_frame = ttk.LabelFrame(parent, text="Performance Settings", padding="5")
        perf_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Thread control
        thread_frame = ttk.Frame(perf_frame)
        thread_frame.pack(fill=tk.X, pady=2)
        ttk.Label(thread_frame, text="Threads:").pack(side=tk.LEFT)
        self.thread_label = ttk.Label(thread_frame, text="1")
        self.thread_label.pack(side=tk.RIGHT)
        
        thread_scale = ttk.Scale(perf_frame, from_=1, to=2,
                                variable=self.performance_settings['threads'],
                                orient=tk.HORIZONTAL, length=200)
        thread_scale.pack(fill=tk.X, pady=2)
        thread_scale.bind("<ButtonRelease-1>", self.on_thread_change)
        thread_scale.bind("<B1-Motion>", self.on_thread_change)
        
        # Downsampling
        ttk.Checkbutton(perf_frame, text="Enable Downsampling", 
                       variable=self.performance_settings['downsample']).pack(anchor=tk.W, pady=2)
        
        # Downsample factor
        ds_frame = ttk.Frame(perf_frame)
        ds_frame.pack(fill=tk.X, pady=2)
        ttk.Label(ds_frame, text="Downsample:").pack(side=tk.LEFT)
        self.ds_label = ttk.Label(ds_frame, text="0.50")
        self.ds_label.pack(side=tk.RIGHT)
        
        ds_scale = ttk.Scale(perf_frame, from_=0.1, to=1.0,
                            variable=self.performance_settings['downsample_factor'],
                            orient=tk.HORIZONTAL, length=200)
        ds_scale.pack(fill=tk.X, pady=2)
        ds_scale.bind("<ButtonRelease-1>", self.on_ds_change)
        ds_scale.bind("<B1-Motion>", self.on_ds_change)
        
        # GPU acceleration
        ttk.Checkbutton(perf_frame, text="GPU Acceleration", 
                       variable=self.performance_settings['gpu_acceleration']).pack(anchor=tk.W, pady=2)
        
        # Performance metrics
        metrics_frame = ttk.LabelFrame(parent, text="Performance Metrics", padding="5")
        metrics_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.fps_label = ttk.Label(metrics_frame, text="FPS: 0.0")
        self.fps_label.pack(anchor=tk.W)
        
        self.processing_label = ttk.Label(metrics_frame, text="Processing: 0ms")
        self.processing_label.pack(anchor=tk.W)
        
        self.detection_label = ttk.Label(metrics_frame, text="Detections: 0")
        self.detection_label.pack(anchor=tk.W)
        
        self.memory_label = ttk.Label(metrics_frame, text="Memory: 0MB")
        self.memory_label.pack(anchor=tk.W)
        
        self.cpu_label = ttk.Label(metrics_frame, text="CPU: 0%")
        self.cpu_label.pack(anchor=tk.W)
    
    def setup_video_display(self, parent):
        # Video display
        self.video_label = ttk.Label(parent, text="No video loaded", 
                                    background="black", foreground="white")
        self.video_label.pack(fill=tk.BOTH, expand=True)
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(parent, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.pack(fill=tk.X, pady=(10, 0))
    
    def initialize_detector(self):
        """Initialize the vehicle detector with current settings"""
        try:
            self.detector = VehicleDetector(
                model_size=self.detection_settings['model'].get(),
                conf_threshold=self.detection_settings['confidence'].get()
            )
            self.log_status("Detector initialized successfully")
        except Exception as e:
            self.log_status(f"Error initializing detector: {e}")
    
    def open_video(self):
        """Open a video file"""
        file_path = filedialog.askopenfilename(
            title="Select Video File",
            filetypes=[("Video files", "*.mp4 *.avi *.mov *.mkv"), ("All files", "*.*")]
        )
        
        if file_path:
            self.video_path = file_path
            self.load_video()
    
    def load_video(self):
        """Load the selected video with error handling"""
        try:
            if self.cap:
                self.cap.release()
            
            self.cap = cv2.VideoCapture(self.video_path)
            if not self.cap.isOpened():
                messagebox.showerror("Error", "Could not open video file")
                return
            
            # Set buffer size to reduce memory usage
            self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
            
            self.total_frames = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
            self.fps = self.cap.get(cv2.CAP_PROP_FPS)
            self.current_frame = 0
            
            self.frame_slider.configure(to=self.total_frames - 1)
            self.frame_var.set(0)
            
            self.log_status(f"Loaded: {Path(self.video_path).name}")
            self.update_frame()
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load video: {e}")
    
    def toggle_playback(self):
        """Toggle video playback"""
        if not self.cap or not self.cap.isOpened():
            return
        
        self.is_playing = not self.is_playing
        if self.is_playing:
            self.play_video()
    
    def play_video(self):
        """Play video in a separate thread with error handling"""
        if not self.is_playing:
            return
        
        def play_loop():
            try:
                while self.is_playing and self.cap and self.cap.isOpened():
                    if self.current_frame < self.total_frames - 1:
                        self.current_frame += 1
                        self.frame_var.set(self.current_frame)
                        self.update_frame()
                        time.sleep(1.0 / self.fps)
                    else:
                        self.is_playing = False
                        break
            except Exception as e:
                self.log_status(f"Playback error: {e}")
                self.is_playing = False
        
        thread = threading.Thread(target=play_loop, daemon=True)
        thread.start()
    
    def stop_video(self):
        """Stop video playback"""
        self.is_playing = False
        if self.cap and self.cap.isOpened():
            self.current_frame = 0
            self.frame_var.set(0)
            self.update_frame()
    
    def update_frame(self):
        """Update the displayed frame with detection and error handling"""
        try:
            if not self.cap or not self.cap.isOpened():
                return
            
            start_time = time.time()
            
            # Set frame position
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, self.current_frame)
            ret, frame = self.cap.read()
            
            if not ret:
                return
            
            # Apply downsampling if enabled
            if self.performance_settings['downsample'].get():
                factor = self.performance_settings['downsample_factor'].get()
                height, width = frame.shape[:2]
                new_width = int(width * factor)
                new_height = int(height * factor)
                frame = cv2.resize(frame, (new_width, new_height))
            
            # Run detection if detector is available
            detections_count = 0
            if self.detector:
                try:
                    processed_frame, detections = self.detector.process_frame(frame)
                    frame = processed_frame
                    
                    if hasattr(detections, 'xyxy') and len(detections) > 0:
                        detections_count = len(detections)
                        
                except Exception as e:
                    self.log_status(f"Detection error: {e}")
            
            # Calculate processing time
            processing_time = (time.time() - start_time) * 1000
            self.processing_times.append(processing_time)
            if len(self.processing_times) > 30:  # Keep last 30 frames
                self.processing_times.pop(0)
            
            # Resize frame for display
            height, width = frame.shape[:2]
            max_size = 600
            if width > max_size or height > max_size:
                scale = min(max_size / width, max_size / height)
                new_width = int(width * scale)
                new_height = int(height * scale)
                frame = cv2.resize(frame, (new_width, new_height))
            
            # Convert to PIL Image
            frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            pil_image = Image.fromarray(frame_rgb)
            photo = ImageTk.PhotoImage(pil_image)
            
            # Update display
            self.video_label.configure(image=photo, text="")
            self.video_label.image = photo
            
            # Update labels
            self.frame_label.config(text=f"Frame: {self.current_frame + 1}/{self.total_frames}")
            self.detection_label.config(text=f"Detections: {detections_count}")
            self.processing_label.config(text=f"Processing: {processing_time:.1f}ms")
            
        except Exception as e:
            self.log_status(f"Frame update error: {e}")
    
    def start_performance_monitor(self):
        """Start performance monitoring"""
        def monitor():
            while True:
                try:
                    # Memory usage
                    memory = psutil.Process().memory_info().rss / 1024 / 1024
                    self.memory_label.config(text=f"Memory: {memory:.1f}MB")
                    
                    # CPU usage
                    cpu_percent = psutil.cpu_percent(interval=1)
                    self.cpu_label.config(text=f"CPU: {cpu_percent:.1f}%")
                    
                    # FPS calculation
                    if len(self.processing_times) > 1:
                        avg_processing = sum(self.processing_times) / len(self.processing_times)
                        fps = 1000 / avg_processing if avg_processing > 0 else 0
                        self.fps_label.config(text=f"FPS: {fps:.1f}")
                    
                    time.sleep(1)
                except:
                    break
        
        thread = threading.Thread(target=monitor, daemon=True)
        thread.start()
    
    def on_frame_change(self, event):
        """Handle frame slider change"""
        if self.cap and self.cap.isOpened():
            self.current_frame = self.frame_var.get()
            self.update_frame()
    
    def on_model_change(self, event):
        """Handle model selection change"""
        model = self.detection_settings['model'].get()
        self.model_desc_label.config(text=self.model_descriptions[model])
        self.initialize_detector()
    
    def on_conf_change(self, event):
        """Handle confidence threshold change"""
        conf = self.detection_settings['confidence'].get()
        self.conf_label.config(text=f"{conf:.2f}")
        if self.detector:
            self.detector.conf_threshold = conf
    
    def on_iou_change(self, event):
        """Handle IOU threshold change"""
        iou = self.detection_settings['iou_threshold'].get()
        self.iou_label.config(text=f"{iou:.2f}")
    
    def on_thread_change(self, event):
        """Handle thread count change"""
        threads = self.performance_settings['threads'].get()
        self.thread_label.config(text=str(threads))
    
    def on_ds_change(self, event):
        """Handle downsampling factor change"""
        ds = self.performance_settings['downsample_factor'].get()
        self.ds_label.config(text=f"{ds:.2f}")
    
    def log_status(self, message):
        """Log status message"""
        self.status_var.set(message)
        self.root.update_idletasks()
    
    def run(self):
        """Run the GUI"""
        try:
            print("Starting Vehicle Detection GUI...")
            print("If the window doesn't appear, check your Dock or use Cmd+Tab")
            self.root.mainloop()
        except Exception as e:
            print(f"GUI Error: {e}")
            sys.exit(1)

def main():
    app = VehicleGUI()
    app.run()

if __name__ == "__main__":
    main() 