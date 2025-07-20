#!/usr/bin/env python3
"""
Python detection script for Qt GUI integration
"""

import sys
import json
import cv2
import numpy as np
from pathlib import Path

# Add the parent directory to the path to import vehicle_detector
sys.path.append(str(Path(__file__).parent.parent))

try:
    from vehicle_detector import VehicleDetector
except ImportError as e:
    print(f"Error importing vehicle_detector: {e}")
    sys.exit(1)

def detect_frame(image_path, model_size='n', conf_threshold=0.5):
    """
    Detect objects in a single frame image
    
    Args:
        image_path: Path to the image file
        model_size: YOLO model size (n/s/m/l/x)
        conf_threshold: Confidence threshold
    
    Returns:
        JSON string with detection results
    """
    try:
        # Initialize detector
        detector = VehicleDetector(model_size=model_size, conf_threshold=conf_threshold)
        
        # Load image
        frame = cv2.imread(image_path)
        if frame is None:
            return json.dumps({"error": "Could not load image"})
        
        # Process frame
        processed_frame, detections = detector.process_frame(frame)
        
        # Convert detections to JSON-serializable format
        results = []
        if hasattr(detections, 'xyxy') and len(detections) > 0:
            for i, bbox in enumerate(detections.xyxy):
                if hasattr(detections, 'tracker_id') and i < len(detections.tracker_id):
                    track_id = int(detections.tracker_id[i])
                else:
                    track_id = i
                
                if hasattr(detections, 'confidence') and i < len(detections.confidence):
                    confidence = float(detections.confidence[i])
                else:
                    confidence = 0.5
                
                if hasattr(detections, 'class_id') and i < len(detections.class_id):
                    class_id = int(detections.class_id[i])
                else:
                    class_id = 2  # Default to car
                
                # Get class name from COCO classes
                coco_classes = ['person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat']
                if class_id < len(coco_classes):
                    class_name = coco_classes[class_id]
                else:
                    class_name = f"class_{class_id}"
                
                result = {
                    "bbox": [float(bbox[0]), float(bbox[1]), float(bbox[2]), float(bbox[3])],
                    "track_id": track_id,
                    "confidence": confidence,
                    "class_id": class_id,
                    "class_name": class_name
                }
                results.append(result)
        
        return json.dumps({"detections": results, "success": True})
        
    except Exception as e:
        return json.dumps({"error": str(e), "success": False})

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python_detector.py <image_path> [model_size] [conf_threshold]")
        sys.exit(1)
    
    image_path = sys.argv[1]
    model_size = sys.argv[2] if len(sys.argv) > 2 else 'n'
    conf_threshold = float(sys.argv[3]) if len(sys.argv) > 3 else 0.5
    
    result = detect_frame(image_path, model_size, conf_threshold)
    print(result) 