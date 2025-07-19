#include <metal_stdlib>
using namespace metal;

/**
 * @brief GPU-accelerated frame preprocessing
 * 
 * Performs resize, format conversion, and enhancement operations
 * using Metal compute shaders for maximum performance.
 */

// Resize kernel with bilinear interpolation
kernel void resize_bilinear(texture2d<float, access::read> inputTexture [[texture(0)]],
                           texture2d<float, access::write> outputTexture [[texture(1)]],
                           uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    // Calculate input coordinates
    float2 inputSize = float2(inputTexture.get_width(), inputTexture.get_height());
    float2 outputSize = float2(outputTexture.get_width(), outputTexture.get_height());
    float2 scale = inputSize / outputSize;
    float2 inputCoord = (float2(gid) + 0.5) * scale - 0.5;
    
    // Bilinear interpolation
    float2 inputFloor = floor(inputCoord);
    float2 inputFrac = inputCoord - inputFloor;
    
    uint2 c00 = uint2(inputFloor);
    uint2 c01 = uint2(inputFloor.x, inputFloor.y + 1);
    uint2 c10 = uint2(inputFloor.x + 1, inputFloor.y);
    uint2 c11 = uint2(inputFloor.x + 1, inputFloor.y + 1);
    
    // Clamp coordinates
    c00 = clamp(c00, uint2(0), uint2(inputTexture.get_width() - 1, inputTexture.get_height() - 1));
    c01 = clamp(c01, uint2(0), uint2(inputTexture.get_width() - 1, inputTexture.get_height() - 1));
    c10 = clamp(c10, uint2(0), uint2(inputTexture.get_width() - 1, inputTexture.get_height() - 1));
    c11 = clamp(c11, uint2(0), uint2(inputTexture.get_width() - 1, inputTexture.get_height() - 1));
    
    // Sample texels
    float4 p00 = inputTexture.read(c00);
    float4 p01 = inputTexture.read(c01);
    float4 p10 = inputTexture.read(c10);
    float4 p11 = inputTexture.read(c11);
    
    // Interpolate
    float4 p0 = mix(p00, p10, inputFrac.x);
    float4 p1 = mix(p01, p11, inputFrac.x);
    float4 result = mix(p0, p1, inputFrac.y);
    
    outputTexture.write(result, gid);
}

// Format conversion kernel (RGB to YUV)
kernel void rgb_to_yuv(texture2d<float, access::read> inputTexture [[texture(0)]],
                      texture2d<float, access::write> outputTexture [[texture(1)]],
                      uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 rgb = inputTexture.read(gid);
    
    // RGB to YUV conversion matrix
    float y = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    float u = -0.14713 * rgb.r - 0.28886 * rgb.g + 0.436 * rgb.b;
    float v = 0.615 * rgb.r - 0.51499 * rgb.g - 0.10001 * rgb.b;
    
    outputTexture.write(float4(y, u, v, rgb.a), gid);
}

// Enhancement kernel (contrast and brightness)
kernel void enhance_image(texture2d<float, access::read> inputTexture [[texture(0)]],
                         texture2d<float, access::write> outputTexture [[texture(1)]],
                         constant float& contrast [[buffer(0)]],
                         constant float& brightness [[buffer(1)]],
                         uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    
    // Apply contrast and brightness
    pixel.rgb = clamp(pixel.rgb * contrast + brightness, 0.0, 1.0);
    
    outputTexture.write(pixel, gid);
}

// Noise reduction kernel (simple bilateral filter)
kernel void noise_reduction(texture2d<float, access::read> inputTexture [[texture(0)]],
                           texture2d<float, access::write> outputTexture [[texture(1)]],
                           constant float& sigma [[buffer(0)]],
                           uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 center = inputTexture.read(gid);
    float4 result = float4(0.0);
    float totalWeight = 0.0;
    
    // 3x3 bilateral filter
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            uint2 samplePos = uint2(gid.x + dx, gid.y + dy);
            
            if (samplePos.x < inputTexture.get_width() && samplePos.y < inputTexture.get_height()) {
                float4 sample = inputTexture.read(samplePos);
                
                // Spatial weight
                float spatialDist = sqrt(float(dx * dx + dy * dy));
                float spatialWeight = exp(-spatialDist * spatialDist / (2.0 * sigma * sigma));
                
                // Intensity weight
                float intensityDiff = length(center.rgb - sample.rgb);
                float intensityWeight = exp(-intensityDiff * intensityDiff / (2.0 * sigma * sigma));
                
                float weight = spatialWeight * intensityWeight;
                result += sample * weight;
                totalWeight += weight;
            }
        }
    }
    
    if (totalWeight > 0.0) {
        result /= totalWeight;
    }
    
    outputTexture.write(result, gid);
}

// Histogram equalization kernel
kernel void histogram_equalization(texture2d<float, access::read> inputTexture [[texture(0)]],
                                  texture2d<float, access::write> outputTexture [[texture(1)]],
                                  constant float* histogram [[buffer(0)]],
                                  uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    
    // Convert to grayscale for histogram calculation
    float gray = 0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b;
    
    // Apply histogram equalization
    int bin = int(gray * 255.0);
    bin = clamp(bin, 0, 255);
    
    float equalized = histogram[bin];
    
    // Apply to all channels
    float scale = equalized / (gray + 0.001); // Avoid division by zero
    pixel.rgb *= scale;
    pixel.rgb = clamp(pixel.rgb, 0.0, 1.0);
    
    outputTexture.write(pixel, gid);
}

// Edge detection kernel (Sobel filter)
kernel void edge_detection(texture2d<float, access::read> inputTexture [[texture(0)]],
                          texture2d<float, access::write> outputTexture [[texture(1)]],
                          uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    // Sobel kernels
    const float3x3 sobelX = float3x3(
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );
    
    const float3x3 sobelY = float3x3(
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    );
    
    float gx = 0.0, gy = 0.0;
    
    // Apply Sobel filters
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            uint2 samplePos = uint2(gid.x + dx, gid.y + dy);
            
            if (samplePos.x < inputTexture.get_width() && samplePos.y < inputTexture.get_height()) {
                float4 sample = inputTexture.read(samplePos);
                float gray = 0.299 * sample.r + 0.587 * sample.g + 0.114 * sample.b;
                
                gx += gray * sobelX[dy + 1][dx + 1];
                gy += gray * sobelY[dy + 1][dx + 1];
            }
        }
    }
    
    // Calculate gradient magnitude
    float magnitude = sqrt(gx * gx + gy * gy);
    magnitude = clamp(magnitude, 0.0, 1.0);
    
    outputTexture.write(float4(magnitude, magnitude, magnitude, 1.0), gid);
}

// Color correction kernel
kernel void color_correction(texture2d<float, access::read> inputTexture [[texture(0)]],
                            texture2d<float, access::write> outputTexture [[texture(1)]],
                            constant float3x3& colorMatrix [[buffer(0)]],
                            uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    
    // Apply color correction matrix
    float3 corrected = colorMatrix * pixel.rgb;
    corrected = clamp(corrected, 0.0, 1.0);
    
    outputTexture.write(float4(corrected, pixel.a), gid);
}

// Gamma correction kernel
kernel void gamma_correction(texture2d<float, access::read> inputTexture [[texture(0)]],
                            texture2d<float, access::write> outputTexture [[texture(1)]],
                            constant float& gamma [[buffer(0)]],
                            uint2 gid [[thread_position_in_grid]]) {
    
    if (gid.x >= outputTexture.get_width() || gid.y >= outputTexture.get_height()) {
        return;
    }
    
    float4 pixel = inputTexture.read(gid);
    
    // Apply gamma correction
    float invGamma = 1.0 / gamma;
    pixel.rgb = pow(pixel.rgb, float3(invGamma));
    
    outputTexture.write(pixel, gid);
} 