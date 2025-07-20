#!/usr/bin/env python3
"""
Demo script for the Vehicle Detection System
Shows the system capabilities and performance
"""

import cv2
import numpy as np
import time
from vehicle_detector import VehicleDetector

def create_demo_video():
    """Create a demo video with moving vehicles"""
    print("🎬 Creating demo video...")
    
    # Create video writer
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('demo_traffic.mp4', fourcc, 30.0, (1280, 720))
    
    # Create frames with moving vehicles
    for frame_num in range(300):  # 10 seconds at 30fps
        # Create background
        frame = np.zeros((720, 1280, 3), dtype=np.uint8)
        frame[:] = (50, 50, 50)  # Dark gray background
        
        # Add road
        cv2.rectangle(frame, (0, 400), (1280, 720), (100, 100, 100), -1)
        
        # Add lane markings
        for i in range(0, 1280, 100):
            cv2.rectangle(frame, (i, 550), (i + 50, 570), (255, 255, 255), -1)
        
        # Add moving vehicles
        offset = frame_num * 3
        
        # Car 1 (moving right)
        car1_x = 200 + offset
        if car1_x < 1280:
            cv2.rectangle(frame, (car1_x, 300), (car1_x + 120, 380), (0, 255, 0), -1)
            cv2.rectangle(frame, (car1_x, 300), (car1_x + 120, 380), (255, 255, 255), 2)
        
        # Car 2 (moving left)
        car2_x = 800 - offset
        if car2_x > 0:
            cv2.rectangle(frame, (car2_x, 350), (car2_x + 100, 420), (255, 0, 0), -1)
            cv2.rectangle(frame, (car2_x, 350), (car2_x + 100, 420), (255, 255, 255), 2)
        
        # Truck (moving slower)
        truck_x = 400 + offset // 2
        if truck_x < 1280:
            cv2.rectangle(frame, (truck_x, 250), (truck_x + 150, 320), (0, 0, 255), -1)
            cv2.rectangle(frame, (truck_x, 250), (truck_x + 150, 320), (255, 255, 255), 2)
        
        # Add some pedestrians
        if frame_num % 60 < 30:  # Every 2 seconds
            ped_x = 600 + (frame_num % 30) * 2
            cv2.circle(frame, (ped_x, 500), 15, (255, 255, 0), -1)
        
        out.write(frame)
    
    out.release()
    print("✅ Demo video created: demo_traffic.mp4")

def run_demo():
    """Run the vehicle detection demo"""
    print("🚗 Vehicle Detection Demo")
    print("=" * 40)
    
    # Create demo video if it doesn't exist
    import os
    if not os.path.exists('demo_traffic.mp4'):
        create_demo_video()
    
    # Initialize detector
    print("🤖 Initializing YOLOv12 detector...")
    detector = VehicleDetector(model_size="n", conf_threshold=0.3)
    
    # Process demo video
    print("🎬 Processing demo video...")
    cap = cv2.VideoCapture('demo_traffic.mp4')
    
    frame_count = 0
    total_detections = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Process frame
        annotated_frame, detections = detector.process_frame(frame)
        total_detections += len(detections)
        frame_count += 1
        
        # Display every 10th frame to show progress
        if frame_count % 10 == 0:
            print(f"Frame {frame_count}: {len(detections)} vehicles detected")
        
        # Show frame (optional)
        cv2.imshow("Vehicle Detection Demo", annotated_frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cap.release()
    cv2.destroyAllWindows()
    
    # Print results
    stats = detector.get_performance_stats()
    print("\n📊 Demo Results:")
    print(f"   Total frames processed: {frame_count}")
    print(f"   Total vehicle detections: {total_detections}")
    if frame_count > 0:
        print(f"   Average vehicles per frame: {total_detections/frame_count:.1f}")
        print(f"   Average FPS: {stats['avg_fps']:.1f}")
        if stats['avg_fps'] > 0:
            print(f"   Processing time: {frame_count/stats['avg_fps']:.1f} seconds")
    else:
        print("   No frames processed")

if __name__ == "__main__":
    run_demo() 