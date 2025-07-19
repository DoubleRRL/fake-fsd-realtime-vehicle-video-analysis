#pragma once

#include "Types.hpp"
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <Metal/Metal.h>
#include <CoreVideo/CoreVideo.h>

namespace RealTimeVideoAnalysis {

/**
 * @brief High-performance memory pool for zero-copy operations
 * 
 * Pre-allocates buffers to eliminate runtime memory allocation overhead.
 * Supports GPU memory pooling for Metal operations.
 */
class BufferPool {
private:
    // CPU buffer pool
    struct CPUBuffer {
        void* data;
        size_t size;
        bool inUse;
        std::chrono::high_resolution_clock::time_point lastUsed;
        
        CPUBuffer() : data(nullptr), size(0), inUse(false) {}
    };
    
    // GPU buffer pool
    struct GPUBuffer {
        id<MTLBuffer> metalBuffer;
        CVPixelBufferRef pixelBuffer;
        bool inUse;
        std::chrono::high_resolution_clock::time_point lastUsed;
        
        GPUBuffer() : metalBuffer(nullptr), pixelBuffer(nullptr), inUse(false) {}
    };
    
    std::vector<CPUBuffer> cpuBuffers;
    std::vector<GPUBuffer> gpuBuffers;
    
    // Thread-safe buffer management
    mutable std::mutex bufferMutex;
    std::atomic<int> activeBuffers{0};
    
    // Pool configuration
    size_t maxBuffers;
    size_t bufferSize;
    id<MTLDevice> metalDevice;
    
    // Performance monitoring
    std::atomic<size_t> totalAllocated{0};
    std::atomic<size_t> peakAllocated{0};
    
public:
    /**
     * @brief Initialize buffer pool
     * @param device Metal device for GPU buffers
     * @param maxSize Maximum number of buffers
     * @param bufferSize Size of each buffer
     */
    bool initialize(id<MTLDevice> device, size_t maxSize, size_t bufferSize);
    
    /**
     * @brief Get CPU buffer from pool
     * @param size Required buffer size
     * @return Buffer pointer or nullptr if unavailable
     */
    void* getCPUBuffer(size_t size);
    
    /**
     * @brief Get GPU buffer from pool
     * @param size Required buffer size
     * @return Metal buffer or nullptr if unavailable
     */
    id<MTLBuffer> getGPUBuffer(size_t size);
    
    /**
     * @brief Get pixel buffer from pool
     * @param width Buffer width
     * @param height Buffer height
     * @param format Pixel format
     * @return CVPixelBufferRef or nullptr if unavailable
     */
    CVPixelBufferRef getPixelBuffer(int width, int height, OSType format);
    
    /**
     * @brief Return buffer to pool
     * @param buffer Buffer to return
     */
    void returnBuffer(void* buffer);
    void returnBuffer(id<MTLBuffer> buffer);
    void returnBuffer(CVPixelBufferRef buffer);
    
    /**
     * @brief Get pool statistics
     * @return Memory usage and allocation statistics
     */
    BufferPoolStats getStats() const;
    
    /**
     * @brief Clean up unused buffers
     */
    void cleanup();
    
    /**
     * @brief Shutdown and release all buffers
     */
    void shutdown();
    
private:
    /**
     * @brief Find available CPU buffer
     * @param size Required size
     * @return Index of available buffer or -1
     */
    int findAvailableCPUBuffer(size_t size) const;
    
    /**
     * @brief Find available GPU buffer
     * @param size Required size
     * @return Index of available buffer or -1
     */
    int findAvailableGPUBuffer(size_t size) const;
    
    /**
     * @brief Create new CPU buffer
     * @param size Buffer size
     * @return Buffer index or -1 on failure
     */
    int createCPUBuffer(size_t size);
    
    /**
     * @brief Create new GPU buffer
     * @param size Buffer size
     * @return Buffer index or -1 on failure
     */
    int createGPUBuffer(size_t size);
    
    /**
     * @brief Create new pixel buffer
     * @param width Buffer width
     * @param height Buffer height
     * @param format Pixel format
     * @return Buffer index or -1 on failure
     */
    int createPixelBuffer(int width, int height, OSType format);
    
    /**
     * @brief Update peak allocation tracking
     */
    void updatePeakAllocation();
};

} // namespace RealTimeVideoAnalysis 