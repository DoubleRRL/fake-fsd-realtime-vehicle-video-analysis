#include "PerformanceMonitor.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>

// macOS system includes for performance monitoring
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/types.h>

namespace RealTimeVideoAnalysis {

PerformanceMonitor::PerformanceMonitor() {
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
}

void PerformanceMonitor::start() {
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
    frameCount = 0;
    totalProcessingTime = 0.0;
    peakLatency = 0.0;
    averageLatency = 0.0;
    currentFPS = 0.0;
    
    std::lock_guard<std::mutex> lock(metricsMutex);
    frameTimes.clear();
    latencyHistory.clear();
    fpsHistory.clear();
}

void PerformanceMonitor::recordFrameTime(double processingTime) {
    auto now = std::chrono::high_resolution_clock::now();
    
    // Update frame count and timing
    frameCount++;
    totalProcessingTime += processingTime;
    
    // Update peak latency
    double currentPeak = peakLatency.load();
    while (processingTime > currentPeak) {
        if (peakLatency.compare_exchange_weak(currentPeak, processingTime)) {
            break;
        }
    }
    
    // Record timing data
    {
        std::lock_guard<std::mutex> lock(metricsMutex);
        frameTimes.push_back(processingTime);
        latencyHistory.push_back(processingTime);
        
        // Limit history size
        if (frameTimes.size() > MAX_HISTORY_SIZE) {
            frameTimes.pop_front();
        }
        if (latencyHistory.size() > MAX_HISTORY_SIZE) {
            latencyHistory.pop_front();
        }
    }
    
    // Update FPS and average latency periodically
    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastFrameTime).count() / 1000.0;
    
    if (timeSinceLastUpdate >= FPS_UPDATE_INTERVAL) {
        updateFPS();
        updateAverageLatency();
        lastFrameTime = now;
    }
    
    // Update system metrics
    recordMemoryUsage(getSystemMemoryUsage());
    recordCPUUsage(getSystemCPUUsage());
    recordGPUUsage(getSystemGPUUsage());
}

void PerformanceMonitor::recordMemoryUsage(double memoryUsage) {
    currentMemoryUsage = memoryUsage;
    
    double currentPeak = peakMemoryUsage.load();
    while (memoryUsage > currentPeak) {
        if (peakMemoryUsage.compare_exchange_weak(currentPeak, memoryUsage)) {
            break;
        }
    }
}

void PerformanceMonitor::recordCPUUsage(double cpuUsage) {
    currentCPUUsage = cpuUsage;
}

void PerformanceMonitor::recordGPUUsage(double gpuUsage) {
    currentGPUUsage = gpuUsage;
}

PerformanceMetrics PerformanceMonitor::getMetrics() const {
    PerformanceMetrics metrics;
    
    metrics.currentFPS = currentFPS.load();
    metrics.averageLatency = averageLatency.load();
    metrics.peakLatency = peakLatency.load();
    metrics.memoryUsage = currentMemoryUsage.load();
    metrics.cpuUsage = currentCPUUsage.load();
    metrics.gpuUsage = currentGPUUsage.load();
    metrics.frameCount = frameCount.load();
    metrics.startTime = startTime;
    
    return metrics;
}

double PerformanceMonitor::getAverageFPS() const {
    if (frameCount.load() == 0) return 0.0;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - startTime).count() / 1000.0;
    
    return totalTime > 0.0 ? frameCount.load() / totalTime : 0.0;
}

double PerformanceMonitor::getAverageLatency() const {
    return averageLatency.load();
}

double PerformanceMonitor::getPeakLatency() const {
    return peakLatency.load();
}

double PerformanceMonitor::getCurrentFPS() const {
    return currentFPS.load();
}

int PerformanceMonitor::getFrameCount() const {
    return frameCount.load();
}

double PerformanceMonitor::getTotalProcessingTime() const {
    return totalProcessingTime.load();
}

double PerformanceMonitor::getMemoryUsage() const {
    return currentMemoryUsage.load();
}

double PerformanceMonitor::getCPUUsage() const {
    return currentCPUUsage.load();
}

double PerformanceMonitor::getGPUUsage() const {
    return currentGPUUsage.load();
}

std::vector<double> PerformanceMonitor::getLatencyHistory() const {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return std::vector<double>(latencyHistory.begin(), latencyHistory.end());
}

std::vector<double> PerformanceMonitor::getFPSHistory() const {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return std::vector<double>(fpsHistory.begin(), fpsHistory.end());
}

void PerformanceMonitor::reset() {
    start();
}

bool PerformanceMonitor::checkPerformanceTargets(double targetFPS, double maxLatency) const {
    return getCurrentFPS() >= targetFPS && getAverageLatency() <= maxLatency;
}

std::string PerformanceMonitor::getPerformanceSummary() const {
    std::stringstream ss;
    
    ss << std::fixed << std::setprecision(2);
    ss << "Performance Summary:\n";
    ss << "  FPS: " << getCurrentFPS() << " (avg: " << getAverageFPS() << ")\n";
    ss << "  Latency: " << getAverageLatency() << "ms (peak: " << getPeakLatency() << "ms)\n";
    ss << "  Frames: " << getFrameCount() << "\n";
    ss << "  Memory: " << getMemoryUsage() << "MB (peak: " << peakMemoryUsage.load() << "MB)\n";
    ss << "  CPU: " << getCPUUsage() << "%\n";
    ss << "  GPU: " << getGPUUsage() << "%\n";
    ss << "  Total Time: " << getTotalProcessingTime() << "s\n";
    
    return ss.str();
}

void PerformanceMonitor::updateFPS() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    if (frameTimes.size() < 2) {
        currentFPS = 0.0;
        return;
    }
    
    // Calculate FPS based on recent frame times
    size_t recentFrames = std::min(frameTimes.size(), size_t(30)); // Last 30 frames
    auto recentTimes = std::vector<double>(frameTimes.end() - recentFrames, frameTimes.end());
    
    double averageFrameTime = std::accumulate(recentTimes.begin(), recentTimes.end(), 0.0) / recentFrames;
    double fps = averageFrameTime > 0.0 ? 1000.0 / averageFrameTime : 0.0;
    
    currentFPS = fps;
    fpsHistory.push_back(fps);
    
    if (fpsHistory.size() > MAX_HISTORY_SIZE) {
        fpsHistory.pop_front();
    }
}

void PerformanceMonitor::updateAverageLatency() {
    std::lock_guard<std::mutex> lock(metricsMutex);
    
    if (latencyHistory.empty()) {
        averageLatency = 0.0;
        return;
    }
    
    // Calculate average latency over recent history
    size_t recentLatencies = std::min(latencyHistory.size(), size_t(100)); // Last 100 frames
    auto recentLatencyData = std::vector<double>(latencyHistory.end() - recentLatencies, latencyHistory.end());
    
    double avg = std::accumulate(recentLatencyData.begin(), recentLatencyData.end(), 0.0) / recentLatencies;
    averageLatency = avg;
}

double PerformanceMonitor::getSystemMemoryUsage() const {
    // Get system memory info using mach APIs
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t page_size;
    vm_statistics_data_t vm_stats;
    
    host_page_size(host_port, &page_size);
    
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stats, &host_size) != KERN_SUCCESS) {
        return 0.0;
    }
    
    // Calculate memory usage
    natural_t mem_used = (vm_stats.active_count + vm_stats.wire_count) * page_size;
    natural_t mem_total = (vm_stats.active_count + vm_stats.inactive_count + vm_stats.wire_count + vm_stats.free_count) * page_size;
    
    return static_cast<double>(mem_used) / (1024.0 * 1024.0); // Convert to MB
}

double PerformanceMonitor::getSystemCPUUsage() const {
    // Get CPU usage using host_statistics
    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(host_cpu_load_info_data_t) / sizeof(integer_t);
    host_cpu_load_info_data_t cpu_load;
    
    if (host_statistics(host_port, HOST_CPU_LOAD_INFO, (host_info_t)&cpu_load, &host_size) != KERN_SUCCESS) {
        return 0.0;
    }
    
    // Calculate CPU usage percentage
    natural_t total = cpu_load.cpu_ticks[CPU_STATE_USER] + 
                     cpu_load.cpu_ticks[CPU_STATE_SYSTEM] + 
                     cpu_load.cpu_ticks[CPU_STATE_IDLE] + 
                     cpu_load.cpu_ticks[CPU_STATE_NICE];
    
    natural_t used = total - cpu_load.cpu_ticks[CPU_STATE_IDLE];
    
    return total > 0 ? (static_cast<double>(used) / total) * 100.0 : 0.0;
}

double PerformanceMonitor::getSystemGPUUsage() const {
    // Note: Getting accurate GPU usage on macOS requires additional frameworks
    // For now, return a placeholder value
    // In a real implementation, you would use IOKit or Metal Performance Shaders
    return 0.0;
}

} // namespace RealTimeVideoAnalysis 