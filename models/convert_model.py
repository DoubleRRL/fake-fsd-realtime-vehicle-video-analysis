#!/usr/bin/env python3
"""
YOLOv8 to Core ML Model Conversion Script

Converts YOLOv8n model to Core ML format optimized for Neural Engine
on Apple Silicon processors.
"""

import torch
import coremltools as ct
from ultralytics import YOLO
import os
import sys
import argparse
from pathlib import Path

def convert_yolo_to_coreml(model_path='yolov8n.pt', 
                          output_path='yolov8n_optimized.mlmodel',
                          input_size=416,
                          quantize=True,
                          include_nms=True):
    """
    Convert YOLOv8n to Core ML format optimized for Neural Engine
    
    Args:
        model_path: Path to YOLOv8n model file
        output_path: Output path for Core ML model
        input_size: Input image size (should be multiple of 32)
        quantize: Whether to quantize to INT8
        include_nms: Whether to include NMS in the model
    """
    
    print(f"Converting {model_path} to Core ML format...")
    
    # Load YOLOv8n model
    try:
        model = YOLO(model_path)
        print(f"Loaded model: {model_path}")
    except Exception as e:
        print(f"Error loading model: {e}")
        return False
    
    # Export to Core ML with optimizations
    try:
        print("Exporting to Core ML...")
        model.export(
            format='coreml',
            imgsz=input_size,
            int8=quantize,
            nms=include_nms,
            half=True,  # Half precision for better performance
            simplify=True,  # Simplify model architecture
            dynamic=False,  # Static shapes for better optimization
            device='cpu'  # Export on CPU for compatibility
        )
        print("Core ML export completed")
    except Exception as e:
        print(f"Error during export: {e}")
        return False
    
    # Load and optimize the Core ML model
    try:
        print("Loading Core ML model for optimization...")
        mlmodel = ct.models.MLModel('yolov8n.mlmodel')
        
        # Set compute units to Neural Engine + GPU
        print("Configuring compute units...")
        mlmodel = ct.models.neural_network.quantization_utils.quantize_weights(
            mlmodel, nbits=8
        )
        
        # Additional optimizations
        print("Applying additional optimizations...")
        
        # Set compute units for optimal performance
        mlmodel.compute_units = ct.ComputeUnit.CPU_AND_NE
        
        # Save optimized model
        mlmodel.save(output_path)
        print(f"Optimized model saved to: {output_path}")
        
        # Print model information
        print("\nModel Information:")
        print(f"Input shape: {mlmodel.get_spec().description.input[0].type.imageType.width}x{mlmodel.get_spec().description.input[0].type.imageType.height}")
        print(f"Output shape: {len(mlmodel.get_spec().description.output)} outputs")
        print(f"Compute units: {mlmodel.compute_units}")
        
        return True
        
    except Exception as e:
        print(f"Error during optimization: {e}")
        return False

def create_model_config(output_dir='.'):
    """
    Create model configuration file
    """
    config = {
        "model": {
            "name": "yolov8n_optimized",
            "version": "1.0",
            "description": "YOLOv8n model optimized for Apple Silicon Neural Engine",
            "input_size": 416,
            "classes": [
                {"id": 0, "name": "person", "color": [255, 0, 0]},
                {"id": 1, "name": "bicycle", "color": [0, 255, 0]},
                {"id": 2, "name": "car", "color": [0, 0, 255]},
                {"id": 3, "name": "motorcycle", "color": [255, 255, 0]},
                {"id": 4, "name": "airplane", "color": [255, 0, 255]},
                {"id": 5, "name": "bus", "color": [0, 255, 255]},
                {"id": 6, "name": "train", "color": [128, 0, 0]},
                {"id": 7, "name": "truck", "color": [0, 128, 0]},
                {"id": 8, "name": "boat", "color": [0, 0, 128]},
                {"id": 9, "name": "traffic light", "color": [128, 128, 0]},
                {"id": 10, "name": "fire hydrant", "color": [128, 0, 128]},
                {"id": 11, "name": "stop sign", "color": [0, 128, 128]},
                {"id": 12, "name": "parking meter", "color": [64, 0, 0]},
                {"id": 13, "name": "bench", "color": [0, 64, 0]},
                {"id": 14, "name": "bird", "color": [0, 0, 64]},
                {"id": 15, "name": "cat", "color": [64, 64, 0]},
                {"id": 16, "name": "dog", "color": [64, 0, 64]},
                {"id": 17, "name": "horse", "color": [0, 64, 64]},
                {"id": 18, "name": "sheep", "color": [192, 0, 0]},
                {"id": 19, "name": "cow", "color": [0, 192, 0]},
                {"id": 20, "name": "elephant", "color": [0, 0, 192]},
                {"id": 21, "name": "bear", "color": [192, 192, 0]},
                {"id": 22, "name": "zebra", "color": [192, 0, 192]},
                {"id": 23, "name": "giraffe", "color": [0, 192, 192]},
                {"id": 24, "name": "backpack", "color": [96, 0, 0]},
                {"id": 25, "name": "umbrella", "color": [0, 96, 0]},
                {"id": 26, "name": "handbag", "color": [0, 0, 96]},
                {"id": 27, "name": "tie", "color": [96, 96, 0]},
                {"id": 28, "name": "suitcase", "color": [96, 0, 96]},
                {"id": 29, "name": "frisbee", "color": [0, 96, 96]},
                {"id": 30, "name": "skis", "color": [160, 0, 0]},
                {"id": 31, "name": "snowboard", "color": [0, 160, 0]},
                {"id": 32, "name": "sports ball", "color": [0, 0, 160]},
                {"id": 33, "name": "kite", "color": [160, 160, 0]},
                {"id": 34, "name": "baseball bat", "color": [160, 0, 160]},
                {"id": 35, "name": "baseball glove", "color": [0, 160, 160]},
                {"id": 36, "name": "skateboard", "color": [32, 0, 0]},
                {"id": 37, "name": "surfboard", "color": [0, 32, 0]},
                {"id": 38, "name": "tennis racket", "color": [0, 0, 32]},
                {"id": 39, "name": "bottle", "color": [32, 32, 0]},
                {"id": 40, "name": "wine glass", "color": [32, 0, 32]},
                {"id": 41, "name": "cup", "color": [0, 32, 32]},
                {"id": 42, "name": "fork", "color": [224, 0, 0]},
                {"id": 43, "name": "knife", "color": [0, 224, 0]},
                {"id": 44, "name": "spoon", "color": [0, 0, 224]},
                {"id": 45, "name": "bowl", "color": [224, 224, 0]},
                {"id": 46, "name": "banana", "color": [224, 0, 224]},
                {"id": 47, "name": "apple", "color": [0, 224, 224]},
                {"id": 48, "name": "sandwich", "color": [48, 0, 0]},
                {"id": 49, "name": "orange", "color": [0, 48, 0]},
                {"id": 50, "name": "broccoli", "color": [0, 0, 48]},
                {"id": 51, "name": "carrot", "color": [48, 48, 0]},
                {"id": 52, "name": "hot dog", "color": [48, 0, 48]},
                {"id": 53, "name": "pizza", "color": [0, 48, 48]},
                {"id": 54, "name": "donut", "color": [176, 0, 0]},
                {"id": 55, "name": "cake", "color": [0, 176, 0]},
                {"id": 56, "name": "chair", "color": [0, 0, 176]},
                {"id": 57, "name": "couch", "color": [176, 176, 0]},
                {"id": 58, "name": "potted plant", "color": [176, 0, 176]},
                {"id": 59, "name": "bed", "color": [0, 176, 176]},
                {"id": 60, "name": "dining table", "color": [16, 0, 0]},
                {"id": 61, "name": "toilet", "color": [0, 16, 0]},
                {"id": 62, "name": "tv", "color": [0, 0, 16]},
                {"id": 63, "name": "laptop", "color": [16, 16, 0]},
                {"id": 64, "name": "mouse", "color": [16, 0, 16]},
                {"id": 65, "name": "remote", "color": [0, 16, 16]},
                {"id": 66, "name": "keyboard", "color": [240, 0, 0]},
                {"id": 67, "name": "cell phone", "color": [0, 240, 0]},
                {"id": 68, "name": "microwave", "color": [0, 0, 240]},
                {"id": 69, "name": "oven", "color": [240, 240, 0]},
                {"id": 70, "name": "toaster", "color": [240, 0, 240]},
                {"id": 71, "name": "sink", "color": [0, 240, 240]},
                {"id": 72, "name": "refrigerator", "color": [8, 0, 0]},
                {"id": 73, "name": "book", "color": [0, 8, 0]},
                {"id": 74, "name": "clock", "color": [0, 0, 8]},
                {"id": 75, "name": "vase", "color": [8, 8, 0]},
                {"id": 76, "name": "scissors", "color": [8, 0, 8]},
                {"id": 77, "name": "teddy bear", "color": [0, 8, 8]},
                {"id": 78, "name": "hair drier", "color": [248, 0, 0]},
                {"id": 79, "name": "toothbrush", "color": [0, 248, 0]}
            ],
            "detection_config": {
                "confidence_threshold": 0.5,
                "nms_threshold": 0.4,
                "max_detections": 100
            },
            "performance": {
                "target_fps": 50,
                "max_latency_ms": 20,
                "optimization_level": "high"
            }
        }
    }
    
    import json
    config_path = os.path.join(output_dir, 'model_config.json')
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)
    
    print(f"Model configuration saved to: {config_path}")
    return config_path

def main():
    parser = argparse.ArgumentParser(description='Convert YOLOv8n to Core ML format')
    parser.add_argument('--model', default='yolov8n.pt', help='Input YOLOv8n model path')
    parser.add_argument('--output', default='yolov8n_optimized.mlmodel', help='Output Core ML model path')
    parser.add_argument('--input-size', type=int, default=416, help='Input image size')
    parser.add_argument('--no-quantize', action='store_true', help='Disable INT8 quantization')
    parser.add_argument('--no-nms', action='store_true', help='Disable NMS inclusion')
    parser.add_argument('--output-dir', default='.', help='Output directory')
    
    args = parser.parse_args()
    
    # Create output directory if it doesn't exist
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Convert model
    success = convert_yolo_to_coreml(
        model_path=args.model,
        output_path=os.path.join(args.output_dir, args.output),
        input_size=args.input_size,
        quantize=not args.no_quantize,
        include_nms=not args.no_nms
    )
    
    if success:
        # Create configuration file
        create_model_config(args.output_dir)
        print("\nModel conversion completed successfully!")
        print(f"Optimized model: {os.path.join(args.output_dir, args.output)}")
    else:
        print("\nModel conversion failed!")
        sys.exit(1)

if __name__ == "__main__":
    main() 