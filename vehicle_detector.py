#!/usr/bin/env python3
"""
Real-time Vehicle Detection and Tracking System
Using YOLOv8 + Supervision for optimal performance
"""

import cv2
import numpy as np
import time
import argparse
import sys
from pathlib import Path
from typing import List, Tuple, Optional
import torch

# Import YOLOv8 and Supervision
try:
    from ultralytics import YOLO
    import supervision as sv
except ImportError as e:
    print(f"❌ Missing dependencies: {e}")
    print("Please install: pip install ultralytics supervision")
    sys.exit(1)

class VehicleDetector:
    """Modern vehicle detection and tracking using YOLOv12"""
    
    def __init__(self, model_size: str = "n", conf_threshold: float = 0.3):
        self.model_size = model_size
        self.conf_threshold = conf_threshold
        self.frame_count = 0
        self.model_version = "unknown"
        
        # Vehicle class IDs (COCO format) - expanded for better detection
        self.vehicle_classes = {
            0: "person", 1: "bicycle", 2: "car", 3: "motorcycle", 
            5: "bus", 7: "truck", 8: "boat", 4: "airplane", 6: "train",
            9: "traffic_light", 10: "fire_hydrant", 11: "stop_sign",
            13: "bench", 14: "bird", 15: "cat", 16: "dog", 17: "horse",
            18: "sheep", 19: "cow", 20: "elephant", 21: "bear", 22: "zebra",
            23: "giraffe", 24: "backpack", 25: "umbrella", 27: "handbag",
            28: "suitcase", 31: "sports_ball", 32: "kite", 33: "baseball_bat",
            34: "baseball_glove", 35: "skateboard", 36: "surfboard",
            37: "tennis_racket", 38: "bottle", 39: "wine_glass", 40: "cup",
            41: "fork", 42: "knife", 43: "spoon", 44: "bowl", 46: "banana",
            47: "apple", 48: "sandwich", 49: "orange", 50: "broccoli",
            51: "carrot", 52: "hot_dog", 53: "pizza", 54: "donut", 55: "cake",
            56: "chair", 57: "couch", 58: "potted_plant", 59: "bed",
            60: "dining_table", 61: "toilet", 62: "tv", 63: "laptop",
            64: "mouse", 65: "remote", 66: "keyboard", 67: "cell_phone",
            68: "microwave", 69: "oven", 70: "toaster", 71: "sink",
            72: "refrigerator", 73: "book", 74: "clock", 75: "vase",
            76: "scissors", 77: "teddy_bear", 78: "hair_drier", 79: "toothbrush"
        }
        
        # Initialize YOLOv8 model
        self.model = None
        self.load_model()
        
        # Initialize tracker
        self.tracker = sv.ByteTrack()
        
        # Initialize annotators
        self.box_annotator = sv.BoxAnnotator()
        
        # Performance tracking
        self.fps_history = []
        self.detection_history = []
        
    def load_model(self):
        """Load YOLOv8 model for optimal real-time performance"""
        try:
            print(f"🚀 Loading YOLOv8{self.model_size}...")
            self.model = YOLO(f"yolov8{self.model_size}.pt")
            print(f"✅ YOLOv8{self.model_size} loaded successfully!")
            self.model_version = "v8"
            
            # Test model on a dummy image
            dummy_img = np.zeros((640, 640, 3), dtype=np.uint8)
            self.model(dummy_img, verbose=False)
            print("✅ Model inference test passed!")
            
        except Exception as e:
            print(f"❌ Failed to load YOLOv8{self.model_size}: {e}")
            print("💡 Try updating ultralytics: pip install --upgrade ultralytics")
            sys.exit(1)
    
    def filter_vehicles(self, detections: sv.Detections) -> sv.Detections:
        """Filter detections to only include vehicles"""
        if len(detections) == 0:
            return detections
        
        # Create mask for vehicle classes
        vehicle_mask = np.array([
            class_id in self.vehicle_classes 
            for class_id in detections.class_id
        ])
        
        # Apply mask to filter vehicles
        filtered_detections = detections[vehicle_mask]
        
        return filtered_detections
    
    def process_frame(self, frame: np.ndarray) -> Tuple[np.ndarray, sv.Detections]:
        """Process a single frame and return annotated frame with detections"""
        self.frame_count += 1
        
        # Downsample frame to 480p for maximum performance
        height, width = frame.shape[:2]
        target_height = 480
        target_width = int(width * target_height / height)
        
        # Resize frame for processing
        processed_frame = cv2.resize(frame, (target_width, target_height))
        
        # Run YOLO inference on downsampled frame
        start_time = time.time()
        results = self.model(processed_frame, verbose=False)[0]
        inference_time = time.time() - start_time
        
        # Convert to supervision detections
        detections = sv.Detections.from_ultralytics(results)
        
        # Scale detection coordinates back to original frame size
        if len(detections) > 0:
            # Scale bounding boxes from processed size back to original size
            scale_x = width / target_width
            scale_y = height / target_height
            
            # Scale the bounding box coordinates
            scaled_xyxy = detections.xyxy.copy()
            scaled_xyxy[:, [0, 2]] *= scale_x  # x coordinates
            scaled_xyxy[:, [1, 3]] *= scale_y  # y coordinates
            
            # Create new detections with scaled coordinates
            detections = sv.Detections(
                xyxy=scaled_xyxy,
                confidence=detections.confidence,
                class_id=detections.class_id
            )
        
        # Filter for vehicles only
        vehicle_detections = self.filter_vehicles(detections)
        
        # Track vehicles
        if len(vehicle_detections) > 0:
            vehicle_detections = self.tracker.update_with_detections(vehicle_detections)
        
        # Create labels with class names, confidence, and tracking IDs
        labels = []
        for i, (class_id, confidence) in enumerate(zip(vehicle_detections.class_id, vehicle_detections.confidence)):
            class_name = self.vehicle_classes.get(class_id, f"class_{class_id}")
            # Add tracking ID if available
            if hasattr(vehicle_detections, 'tracker_id') and len(vehicle_detections.tracker_id) > i:
                tracker_id = vehicle_detections.tracker_id[i]
                labels.append(f"ID:{tracker_id} {class_name} {confidence:.2f}")
            else:
                labels.append(f"{class_name} {confidence:.2f}")
        
        # Annotate frame with labels
        annotated_frame = self.box_annotator.annotate(
            scene=frame.copy(),
            detections=vehicle_detections
        )
        
        # Add custom labels manually
        for i, (detection, label) in enumerate(zip(vehicle_detections.xyxy, labels)):
            x1, y1, x2, y2 = detection
            # Add label text
            cv2.putText(annotated_frame, label, (int(x1), int(y1) - 10), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        
        # Add performance info
        fps = 1.0 / inference_time if inference_time > 0 else 0
        self.fps_history.append(fps)
        if len(self.fps_history) > 30:
            self.fps_history.pop(0)
        
        avg_fps = sum(self.fps_history) / len(self.fps_history)
        
        # Add info overlay
        info_text = f"Vehicles: {len(vehicle_detections)} | FPS: {avg_fps:.1f} | Frame: {self.frame_count}"
        cv2.putText(annotated_frame, info_text, (10, 30), 
                   cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        
        # Add inference time and resolution info
        cv2.putText(annotated_frame, f"Inference: {inference_time*1000:.1f}ms | Processed: {target_width}x{target_height}", (10, 70), 
                   cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
        
        return annotated_frame, vehicle_detections
    
    def get_performance_stats(self) -> dict:
        """Get performance statistics"""
        if not self.fps_history:
            return {"avg_fps": 0, "total_frames": 0}
        
        if not self.fps_history:
            return {
                "avg_fps": 0,
                "total_frames": self.frame_count,
                "min_fps": 0,
                "max_fps": 0
            }
        
        return {
            "avg_fps": sum(self.fps_history) / len(self.fps_history),
            "total_frames": self.frame_count,
            "min_fps": min(self.fps_history),
            "max_fps": max(self.fps_history)
        }

def main():
    parser = argparse.ArgumentParser(description="Real-time Vehicle Detection with YOLOv12")
    parser.add_argument("--source", type=str, default="0", 
                       help="Video source (file path or camera index)")
    parser.add_argument("--model", type=str, default="n", 
                       choices=["n", "s", "m", "l", "x"],
                       help="YOLOv12 model size (n=nano, s=small, m=medium, l=large, x=xlarge)")
    parser.add_argument("--conf", type=float, default=0.3,
                       help="Confidence threshold")
    parser.add_argument("--save", action="store_true",
                       help="Save output video")
    args = parser.parse_args()
    
    # Initialize detector
    detector = VehicleDetector(model_size=args.model, conf_threshold=args.conf)
    
    # Open video source
    if args.source.isdigit():
        cap = cv2.VideoCapture(int(args.source))
        source_name = f"Camera {args.source}"
    else:
        cap = cv2.VideoCapture(args.source)
        source_name = Path(args.source).name
    
    if not cap.isOpened():
        print(f"❌ Error: Could not open {source_name}")
        return
    
    # Get video properties
    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    print(f"🎥 Source: {source_name}")
    print(f"📐 Resolution: {width}x{height}")
    print(f"🎬 FPS: {fps}")
    print(f"🤖 Model: YOLO{detector.model_version}{args.model}")
    print(f"🎯 Confidence: {args.conf}")
    print("Press 'q' to quit, 's' to save screenshot")
    
    # Setup video writer if saving
    writer = None
    if args.save:
        output_path = f"output_{int(time.time())}.mp4"
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        writer = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
        print(f"💾 Saving to: {output_path}")
    
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("End of video")
                break
            
            # Process frame
            annotated_frame, detections = detector.process_frame(frame)
            
            # Save frame if requested
            if writer:
                writer.write(annotated_frame)
            
            # Display frame
            cv2.imshow("Vehicle Detection", annotated_frame)
            
            # Handle key presses
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('s'):
                screenshot_path = f"screenshot_{int(time.time())}.jpg"
                cv2.imwrite(screenshot_path, annotated_frame)
                print(f"📸 Screenshot saved: {screenshot_path}")
    
    except KeyboardInterrupt:
        print("\n⏹️ Interrupted by user")
    
    finally:
        # Cleanup
        cap.release()
        if writer:
            writer.release()
        cv2.destroyAllWindows()
        
        # Print performance stats
        stats = detector.get_performance_stats()
        print(f"\n📊 Performance Summary:")
        print(f"   Total frames: {stats['total_frames']}")
        print(f"   Average FPS: {stats['avg_fps']:.1f}")
        print(f"   Min FPS: {stats['min_fps']:.1f}")
        print(f"   Max FPS: {stats['max_fps']:.1f}")
        print("👋 Vehicle detection stopped")

if __name__ == "__main__":
    main() 