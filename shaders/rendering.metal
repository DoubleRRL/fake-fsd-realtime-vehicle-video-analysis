#include <metal_stdlib>
using namespace metal;

/**
 * @brief GPU-accelerated overlay rendering
 * 
 * Renders detection boxes, labels, and prediction trajectories
 * directly on GPU for optimal performance.
 */

struct BoundingBox {
    float4 rect;        // x, y, width, height
    float4 color;       // r, g, b, a
    float thickness;
    int trackId;
};

struct TrajectoryPoint {
    float2 position;
    float4 color;
    float confidence;
};

// Render bounding boxes
kernel void render_bounding_boxes(texture2d<float, access::read> inputTexture [[texture(0)]],
                                 texture2d<float, access::write> outputTexture [[texture(1)]],
                                 constant BoundingBox* boxes [[buffer(0)]],
                                 constant uint& boxCount [[buffer(1)]],
                                 uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Check if pixel is on any bounding box edge
    for (uint i = 0; i < boxCount; ++i) {
        BoundingBox box = boxes[i];
        
        // Check if point is on box boundary
        bool onLeft = abs(pos.x - box.rect.x) < box.thickness && 
                      pos.y >= box.rect.y && pos.y <= box.rect.y + box.rect.w;
        bool onRight = abs(pos.x - (box.rect.x + box.rect.z)) < box.thickness && 
                       pos.y >= box.rect.y && pos.y <= box.rect.y + box.rect.w;
        bool onTop = abs(pos.y - box.rect.y) < box.thickness && 
                     pos.x >= box.rect.x && pos.x <= box.rect.x + box.rect.z;
        bool onBottom = abs(pos.y - (box.rect.y + box.rect.w)) < box.thickness && 
                        pos.x >= box.rect.x && pos.x <= box.rect.x + box.rect.z;
        
        if (onLeft || onRight || onTop || onBottom) {
            // Alpha blend with box color
            pixel.rgb = mix(pixel.rgb, box.color.rgb, box.color.a);
            break;
        }
    }
    
    outputTexture.write(pixel, gid);
}

// Render trajectory predictions
kernel void render_trajectories(texture2d<float, access::read> inputTexture [[texture(0)]],
                               texture2d<float, access::write> outputTexture [[texture(1)]],
                               constant TrajectoryPoint* points [[buffer(0)]],
                               constant uint& pointCount [[buffer(1)]],
                               uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Draw trajectory points
    for (uint i = 0; i < pointCount; ++i) {
        TrajectoryPoint point = points[i];
        float distance = length(pos - point.position);
        
        if (distance < 3.0) {
            // Alpha blend based on confidence
            float alpha = point.confidence * point.color.a;
            pixel.rgb = mix(pixel.rgb, point.color.rgb, alpha);
        }
    }
    
    outputTexture.write(pixel, gid);
}

// Render filled bounding boxes
kernel void render_filled_boxes(texture2d<float, access::read> inputTexture [[texture(0)]],
                               texture2d<float, access::write> outputTexture [[texture(1)]],
                               constant BoundingBox* boxes [[buffer(0)]],
                               constant uint& boxCount [[buffer(1)]],
                               constant float& fillAlpha [[buffer(2)]],
                               uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Check if pixel is inside any bounding box
    for (uint i = 0; i < boxCount; ++i) {
        BoundingBox box = boxes[i];
        
        if (pos.x >= box.rect.x && pos.x <= box.rect.x + box.rect.z &&
            pos.y >= box.rect.y && pos.y <= box.rect.y + box.rect.w) {
            
            // Alpha blend with box color
            float4 fillColor = float4(box.color.rgb, fillAlpha);
            pixel.rgb = mix(pixel.rgb, fillColor.rgb, fillColor.a);
            break;
        }
    }
    
    outputTexture.write(pixel, gid);
}

// Render motion vectors
kernel void render_motion_vectors(texture2d<float, access::read> inputTexture [[texture(0)]],
                                 texture2d<float, access::write> outputTexture [[texture(1)]],
                                 constant float2* positions [[buffer(0)]],
                                 constant float2* velocities [[buffer(1)]],
                                 constant float4* colors [[buffer(2)]],
                                 constant uint& vectorCount [[buffer(3)]],
                                 constant float& scale [[buffer(4)]],
                                 uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Draw motion vectors
    for (uint i = 0; i < vectorCount; ++i) {
        float2 startPos = positions[i];
        float2 velocity = velocities[i] * scale;
        float2 endPos = startPos + velocity;
        float4 color = colors[i];
        
        // Draw line from start to end position
        float2 lineVec = endPos - startPos;
        float lineLength = length(lineVec);
        
        if (lineLength > 0.0) {
            float2 lineDir = lineVec / lineLength;
            float2 perpDir = float2(-lineDir.y, lineDir.x);
            
            // Check if pixel is on the line
            float2 toPixel = pos - startPos;
            float projLength = dot(toPixel, lineDir);
            float projDist = abs(dot(toPixel, perpDir));
            
            if (projLength >= 0.0 && projLength <= lineLength && projDist < 1.0) {
                // Alpha blend with vector color
                pixel.rgb = mix(pixel.rgb, color.rgb, color.a);
            }
        }
    }
    
    outputTexture.write(pixel, gid);
}

// Render confidence heatmap
kernel void render_confidence_heatmap(texture2d<float, access::read> inputTexture [[texture(0)]],
                                     texture2d<float, access::write> outputTexture [[texture(1)]],
                                     constant float2* positions [[buffer(0)]],
                                     constant float* confidences [[buffer(1)]],
                                     constant uint& pointCount [[buffer(2)]],
                                     constant float& radius [[buffer(3)]],
                                     uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    float4 heatmapColor = float4(0.0);
    
    // Accumulate confidence contributions
    for (uint i = 0; i < pointCount; ++i) {
        float2 pointPos = positions[i];
        float confidence = confidences[i];
        float distance = length(pos - pointPos);
        
        if (distance < radius) {
            // Gaussian falloff
            float weight = exp(-distance * distance / (2.0 * radius * radius));
            float4 pointColor = float4(confidence, 0.0, 1.0 - confidence, weight);
            heatmapColor = mix(heatmapColor, pointColor, weight);
        }
    }
    
    // Blend with original pixel
    if (heatmapColor.a > 0.0) {
        pixel.rgb = mix(pixel.rgb, heatmapColor.rgb, heatmapColor.a * 0.7);
    }
    
    outputTexture.write(pixel, gid);
}

// Render text labels (simplified - just colored rectangles for now)
kernel void render_text_labels(texture2d<float, access::read> inputTexture [[texture(0)]],
                              texture2d<float, access::write> outputTexture [[texture(1)]],
                              constant float2* positions [[buffer(0)]],
                              constant float4* colors [[buffer(1)]],
                              constant uint& labelCount [[buffer(2)]],
                              uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Draw label backgrounds
    for (uint i = 0; i < labelCount; ++i) {
        float2 labelPos = positions[i];
        float4 color = colors[i];
        
        // Simple rectangular label background
        if (pos.x >= labelPos.x && pos.x <= labelPos.x + 60.0 &&
            pos.y >= labelPos.y && pos.y <= labelPos.y + 20.0) {
            
            pixel.rgb = mix(pixel.rgb, color.rgb, color.a);
        }
    }
    
    outputTexture.write(pixel, gid);
}

// Render grid overlay
kernel void render_grid(texture2d<float, access::read> inputTexture [[texture(0)]],
                       texture2d<float, access::write> outputTexture [[texture(1)]],
                       constant float& gridSpacing [[buffer(0)]],
                       constant float4& gridColor [[buffer(1)]],
                       uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    float2 pos = float2(gid);
    
    // Check if pixel is on grid lines
    bool onVerticalLine = fmod(pos.x, gridSpacing) < 1.0;
    bool onHorizontalLine = fmod(pos.y, gridSpacing) < 1.0;
    
    if (onVerticalLine || onHorizontalLine) {
        pixel.rgb = mix(pixel.rgb, gridColor.rgb, gridColor.a * 0.3);
    }
    
    outputTexture.write(pixel, gid);
}

// Render performance overlay
kernel void render_performance_overlay(texture2d<float, access::read> inputTexture [[texture(0)]],
                                      texture2d<float, access::write> outputTexture [[texture(1)]],
                                      constant float& fps [[buffer(0)]],
                                      constant float& latency [[buffer(1)]],
                                      constant float& memoryUsage [[buffer(2)]],
                                      uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    
    // Render performance metrics in top-left corner
    if (gid.x < 200 && gid.y < 100) {
        // Simple colored background for metrics
        if (gid.x < 180 && gid.y < 80) {
            float4 bgColor = float4(0.0, 0.0, 0.0, 0.7);
            pixel.rgb = mix(pixel.rgb, bgColor.rgb, bgColor.a);
        }
    }
    
    outputTexture.write(pixel, gid);
} 