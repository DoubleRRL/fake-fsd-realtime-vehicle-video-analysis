#include "BufferPool.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace RealTimeVideoAnalysis {

bool BufferPool::initialize(id<MTLDevice> device, size_t maxSize, size_t bufferSize) {
    if (!device) {
        std::cerr << "Error: Invalid Metal device" << std::endl;
        return false;
    }
    
    this->metalDevice = device;
    this->maxBuffers = maxSize;
    this->bufferSize = bufferSize;
    
    // Pre-allocate CPU buffers
    cpuBuffers.reserve(maxSize);
    for (size_t i = 0; i < maxSize / 2; ++i) {
        CPUBuffer buffer;
        buffer.data = malloc(bufferSize);
        if (!buffer.data) {
            std::cerr << "Error: Failed to allocate CPU buffer " << i << std::endl;
            return false;
        }
        buffer.size = bufferSize;
        buffer.inUse = false;
        cpuBuffers.push_back(buffer);
        totalAllocated += bufferSize;
    }
    
    // Pre-allocate GPU buffers
    gpuBuffers.reserve(maxSize / 2);
    for (size_t i = 0; i < maxSize / 2; ++i) {
        GPUBuffer buffer;
        buffer.metalBuffer = [metalDevice newBufferWithLength:bufferSize 
                                                      options:MTLResourceStorageModeShared];
        if (!buffer.metalBuffer) {
            std::cerr << "Error: Failed to allocate GPU buffer " << i << std::endl;
            return false;
        }
        buffer.pixelBuffer = nullptr;
        buffer.inUse = false;
        gpuBuffers.push_back(buffer);
        totalAllocated += bufferSize;
    }
    
    updatePeakAllocation();
    
    std::cout << "BufferPool initialized with " << cpuBuffers.size() 
              << " CPU buffers and " << gpuBuffers.size() << " GPU buffers" << std::endl;
    
    return true;
}

void* BufferPool::getCPUBuffer(size_t size) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Try to find existing buffer
    int index = findAvailableCPUBuffer(size);
    if (index >= 0) {
        cpuBuffers[index].inUse = true;
        cpuBuffers[index].lastUsed = std::chrono::high_resolution_clock::now();
        activeBuffers++;
        return cpuBuffers[index].data;
    }
    
    // Create new buffer if pool not full
    if (cpuBuffers.size() < maxBuffers) {
        index = createCPUBuffer(size);
        if (index >= 0) {
            cpuBuffers[index].inUse = true;
            cpuBuffers[index].lastUsed = std::chrono::high_resolution_clock::now();
            activeBuffers++;
            return cpuBuffers[index].data;
        }
    }
    
    std::cerr << "Warning: No available CPU buffer for size " << size << std::endl;
    return nullptr;
}

id<MTLBuffer> BufferPool::getGPUBuffer(size_t size) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Try to find existing buffer
    int index = findAvailableGPUBuffer(size);
    if (index >= 0) {
        gpuBuffers[index].inUse = true;
        gpuBuffers[index].lastUsed = std::chrono::high_resolution_clock::now();
        activeBuffers++;
        return gpuBuffers[index].metalBuffer;
    }
    
    // Create new buffer if pool not full
    if (gpuBuffers.size() < maxBuffers / 2) {
        index = createGPUBuffer(size);
        if (index >= 0) {
            gpuBuffers[index].inUse = true;
            gpuBuffers[index].lastUsed = std::chrono::high_resolution_clock::now();
            activeBuffers++;
            return gpuBuffers[index].metalBuffer;
        }
    }
    
    std::cerr << "Warning: No available GPU buffer for size " << size << std::endl;
    return nullptr;
}

CVPixelBufferRef BufferPool::getPixelBuffer(int width, int height, OSType format) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Try to find existing pixel buffer
    for (auto& buffer : gpuBuffers) {
        if (!buffer.inUse && buffer.pixelBuffer) {
            size_t bufferWidth = CVPixelBufferGetWidth(buffer.pixelBuffer);
            size_t bufferHeight = CVPixelBufferGetHeight(buffer.pixelBuffer);
            OSType bufferFormat = CVPixelBufferGetPixelFormatType(buffer.pixelBuffer);
            
            if (bufferWidth == width && bufferHeight == height && bufferFormat == format) {
                buffer.inUse = true;
                buffer.lastUsed = std::chrono::high_resolution_clock::now();
                activeBuffers++;
                return buffer.pixelBuffer;
            }
        }
    }
    
    // Create new pixel buffer
    int index = createPixelBuffer(width, height, format);
    if (index >= 0) {
        gpuBuffers[index].inUse = true;
        gpuBuffers[index].lastUsed = std::chrono::high_resolution_clock::now();
        activeBuffers++;
        return gpuBuffers[index].pixelBuffer;
    }
    
    std::cerr << "Warning: Failed to create pixel buffer " << width << "x" << height << std::endl;
    return nullptr;
}

void BufferPool::returnBuffer(void* buffer) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    for (auto& cpuBuffer : cpuBuffers) {
        if (cpuBuffer.data == buffer && cpuBuffer.inUse) {
            cpuBuffer.inUse = false;
            activeBuffers--;
            return;
        }
    }
    
    std::cerr << "Warning: Attempted to return unknown CPU buffer" << std::endl;
}

void BufferPool::returnBuffer(id<MTLBuffer> buffer) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    for (auto& gpuBuffer : gpuBuffers) {
        if (gpuBuffer.metalBuffer == buffer && gpuBuffer.inUse) {
            gpuBuffer.inUse = false;
            activeBuffers--;
            return;
        }
    }
    
    std::cerr << "Warning: Attempted to return unknown GPU buffer" << std::endl;
}

void BufferPool::returnBuffer(CVPixelBufferRef buffer) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    for (auto& gpuBuffer : gpuBuffers) {
        if (gpuBuffer.pixelBuffer == buffer && gpuBuffer.inUse) {
            gpuBuffer.inUse = false;
            activeBuffers--;
            return;
        }
    }
    
    std::cerr << "Warning: Attempted to return unknown pixel buffer" << std::endl;
}

BufferPoolStats BufferPool::getStats() const {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    BufferPoolStats stats;
    stats.totalBuffers = cpuBuffers.size() + gpuBuffers.size();
    stats.activeBuffers = activeBuffers.load();
    stats.totalMemory = totalAllocated.load();
    stats.peakMemory = peakAllocated.load();
    stats.utilizationRate = stats.totalBuffers > 0 ? 
        static_cast<double>(stats.activeBuffers) / stats.totalBuffers : 0.0;
    
    return stats;
}

void BufferPool::cleanup() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    auto now = std::chrono::high_resolution_clock::now();
    auto timeout = std::chrono::seconds(30); // 30 second timeout
    
    // Clean up unused CPU buffers
    for (auto& buffer : cpuBuffers) {
        if (!buffer.inUse && 
            (now - buffer.lastUsed) > timeout) {
            // Keep buffer but mark as recently used
            buffer.lastUsed = now;
        }
    }
    
    // Clean up unused GPU buffers
    for (auto& buffer : gpuBuffers) {
        if (!buffer.inUse && 
            (now - buffer.lastUsed) > timeout) {
            // Keep buffer but mark as recently used
            buffer.lastUsed = now;
        }
    }
}

void BufferPool::shutdown() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Release CPU buffers
    for (auto& buffer : cpuBuffers) {
        if (buffer.data) {
            free(buffer.data);
            buffer.data = nullptr;
        }
    }
    cpuBuffers.clear();
    
    // Release GPU buffers
    for (auto& buffer : gpuBuffers) {
        if (buffer.metalBuffer) {
            [buffer.metalBuffer release];
            buffer.metalBuffer = nullptr;
        }
        if (buffer.pixelBuffer) {
            CVPixelBufferRelease(buffer.pixelBuffer);
            buffer.pixelBuffer = nullptr;
        }
    }
    gpuBuffers.clear();
    
    activeBuffers = 0;
    totalAllocated = 0;
    peakAllocated = 0;
    
    std::cout << "BufferPool shutdown complete" << std::endl;
}

int BufferPool::findAvailableCPUBuffer(size_t size) const {
    for (size_t i = 0; i < cpuBuffers.size(); ++i) {
        if (!cpuBuffers[i].inUse && cpuBuffers[i].size >= size) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int BufferPool::findAvailableGPUBuffer(size_t size) const {
    for (size_t i = 0; i < gpuBuffers.size(); ++i) {
        if (!gpuBuffers[i].inUse && [gpuBuffers[i].metalBuffer length] >= size) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int BufferPool::createCPUBuffer(size_t size) {
    CPUBuffer buffer;
    buffer.data = malloc(size);
    if (!buffer.data) {
        return -1;
    }
    
    buffer.size = size;
    buffer.inUse = false;
    buffer.lastUsed = std::chrono::high_resolution_clock::now();
    
    cpuBuffers.push_back(buffer);
    totalAllocated += size;
    updatePeakAllocation();
    
    return static_cast<int>(cpuBuffers.size() - 1);
}

int BufferPool::createGPUBuffer(size_t size) {
    GPUBuffer buffer;
    buffer.metalBuffer = [metalDevice newBufferWithLength:size 
                                                  options:MTLResourceStorageModeShared];
    if (!buffer.metalBuffer) {
        return -1;
    }
    
    buffer.pixelBuffer = nullptr;
    buffer.inUse = false;
    buffer.lastUsed = std::chrono::high_resolution_clock::now();
    
    gpuBuffers.push_back(buffer);
    totalAllocated += size;
    updatePeakAllocation();
    
    return static_cast<int>(gpuBuffers.size() - 1);
}

int BufferPool::createPixelBuffer(int width, int height, OSType format) {
    GPUBuffer buffer;
    
    // Create pixel buffer attributes
    NSDictionary* attributes = @{
        (NSString*)kCVPixelBufferMetalCompatibilityKey: @YES,
        (NSString*)kCVPixelBufferIOSurfacePropertiesKey: @{}
    };
    
    // Create pixel buffer
    CVReturn result = CVPixelBufferCreate(kCFAllocatorDefault,
                                         width, height,
                                         format,
                                         (__bridge CFDictionaryRef)attributes,
                                         &buffer.pixelBuffer);
    
    if (result != kCVReturnSuccess || !buffer.pixelBuffer) {
        std::cerr << "Error: Failed to create pixel buffer: " << result << std::endl;
        return -1;
    }
    
    buffer.metalBuffer = nullptr;
    buffer.inUse = false;
    buffer.lastUsed = std::chrono::high_resolution_clock::now();
    
    gpuBuffers.push_back(buffer);
    
    size_t bufferSize = CVPixelBufferGetDataSize(buffer.pixelBuffer);
    totalAllocated += bufferSize;
    updatePeakAllocation();
    
    return static_cast<int>(gpuBuffers.size() - 1);
}

void BufferPool::updatePeakAllocation() {
    size_t current = totalAllocated.load();
    size_t peak = peakAllocated.load();
    
    while (current > peak) {
        if (peakAllocated.compare_exchange_weak(peak, current)) {
            break;
        }
    }
}

} // namespace RealTimeVideoAnalysis 