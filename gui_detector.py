#!/usr/bin/env python3
"""
Simple GUI for vehicle detection using tkinter
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import cv2
from PIL import Image, ImageTk
import threading
import time
from pathlib import Path
import sys

# Import our vehicle detector
from vehicle_detector import VehicleDetector

class VehicleDetectionGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Vehicle Detection GUI")
        self.root.geometry("1200x800")
        
        # Initialize detector
        self.detector = None
        self.video_path = None
        self.cap = None
        self.is_playing = False
        self.current_frame = 0
        self.total_frames = 0
        self.fps = 30
        
        self.setup_ui()
        self.initialize_detector()
    
    def setup_ui(self):
        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(1, weight=1)
        
        # Controls frame
        controls_frame = ttk.LabelFrame(main_frame, text="Controls", padding="5")
        controls_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        # File selection
        ttk.Button(controls_frame, text="Open Video", command=self.open_video).grid(row=0, column=0, padx=5)
        ttk.Button(controls_frame, text="Play/Pause", command=self.toggle_playback).grid(row=0, column=1, padx=5)
        ttk.Button(controls_frame, text="Stop", command=self.stop_video).grid(row=0, column=2, padx=5)
        
        # Model selection
        ttk.Label(controls_frame, text="Model:").grid(row=0, column=3, padx=(20, 5))
        self.model_var = tk.StringVar(value="n")
        model_combo = ttk.Combobox(controls_frame, textvariable=self.model_var, 
                                  values=["n", "s", "m", "l", "x"], width=5)
        model_combo.grid(row=0, column=4, padx=5)
        model_combo.bind("<<ComboboxSelected>>", self.on_model_change)
        
        # Confidence threshold
        ttk.Label(controls_frame, text="Confidence:").grid(row=0, column=5, padx=(20, 5))
        self.conf_var = tk.DoubleVar(value=0.5)
        conf_scale = ttk.Scale(controls_frame, from_=0.1, to=1.0, variable=self.conf_var, 
                              orient=tk.HORIZONTAL, length=100)
        conf_scale.grid(row=0, column=6, padx=5)
        conf_scale.bind("<ButtonRelease-1>", self.on_conf_change)
        
        # Frame slider
        self.frame_var = tk.IntVar()
        self.frame_slider = ttk.Scale(controls_frame, from_=0, to=100, variable=self.frame_var,
                                     orient=tk.HORIZONTAL, length=200)
        self.frame_slider.grid(row=1, column=0, columnspan=7, sticky=(tk.W, tk.E), pady=10)
        self.frame_slider.bind("<ButtonRelease-1>", self.on_frame_change)
        
        # Video display
        self.video_label = ttk.Label(main_frame, text="No video loaded")
        self.video_label.grid(row=1, column=1, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(10, 0))
        
        # Info frame
        info_frame = ttk.LabelFrame(main_frame, text="Information", padding="5")
        info_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(0, 10))
        
        self.info_text = tk.Text(info_frame, width=30, height=20)
        self.info_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Scrollbar for info text
        scrollbar = ttk.Scrollbar(info_frame, orient=tk.VERTICAL, command=self.info_text.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.info_text.configure(yscrollcommand=scrollbar.set)
    
    def initialize_detector(self):
        """Initialize the vehicle detector"""
        try:
            self.detector = VehicleDetector(model_size=self.model_var.get(), 
                                          conf_threshold=self.conf_var.get())
            self.log_info("Detector initialized successfully")
        except Exception as e:
            self.log_info(f"Error initializing detector: {e}")
    
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
        """Load the selected video"""
        if self.cap:
            self.cap.release()
        
        self.cap = cv2.VideoCapture(self.video_path)
        if not self.cap.isOpened():
            messagebox.showerror("Error", "Could not open video file")
            return
        
        self.total_frames = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS)
        self.current_frame = 0
        
        self.frame_slider.configure(to=self.total_frames - 1)
        self.frame_var.set(0)
        
        self.log_info(f"Loaded video: {Path(self.video_path).name}")
        self.log_info(f"Frames: {self.total_frames}")
        self.log_info(f"FPS: {self.fps:.2f}")
        
        self.update_frame()
    
    def toggle_playback(self):
        """Toggle video playback"""
        if not self.cap or not self.cap.isOpened():
            return
        
        self.is_playing = not self.is_playing
        if self.is_playing:
            self.play_video()
    
    def play_video(self):
        """Play video in a separate thread"""
        if not self.is_playing:
            return
        
        def play_loop():
            while self.is_playing and self.cap and self.cap.isOpened():
                if self.current_frame < self.total_frames - 1:
                    self.current_frame += 1
                    self.frame_var.set(self.current_frame)
                    self.update_frame()
                    time.sleep(1.0 / self.fps)
                else:
                    self.is_playing = False
                    break
        
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
        """Update the displayed frame"""
        if not self.cap or not self.cap.isOpened():
            return
        
        self.cap.set(cv2.CAP_PROP_POS_FRAMES, self.current_frame)
        ret, frame = self.cap.read()
        
        if not ret:
            return
        
        # Run detection if detector is available
        if self.detector:
            try:
                processed_frame, detections = self.detector.process_frame(frame)
                frame = processed_frame
                
                # Log detection results
                if hasattr(detections, 'xyxy') and len(detections) > 0:
                    self.log_info(f"Frame {self.current_frame}: {len(detections)} objects detected")
                else:
                    self.log_info(f"Frame {self.current_frame}: No objects detected")
                    
            except Exception as e:
                self.log_info(f"Detection error: {e}")
        
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
        self.video_label.image = photo  # Keep a reference
        
        # Update status
        self.status_var.set(f"Frame: {self.current_frame + 1}/{self.total_frames}")
    
    def on_frame_change(self, event):
        """Handle frame slider change"""
        if self.cap and self.cap.isOpened():
            self.current_frame = self.frame_var.get()
            self.update_frame()
    
    def on_model_change(self, event):
        """Handle model selection change"""
        self.initialize_detector()
    
    def on_conf_change(self, event):
        """Handle confidence threshold change"""
        if self.detector:
            self.detector.conf_threshold = self.conf_var.get()
    
    def log_info(self, message):
        """Log information to the info text area"""
        self.info_text.insert(tk.END, f"{message}\n")
        self.info_text.see(tk.END)
        self.root.update_idletasks()

def main():
    root = tk.Tk()
    app = VehicleDetectionGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main() 