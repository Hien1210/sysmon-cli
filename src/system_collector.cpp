#include "system_collector.hpp"

#include <iostream>

void SystemCollector::update() {
    updateCpuUsage();
    updateRamUsage();
}

SystemStats SystemCollector::getStats() const {
    return currentStats;
}

void SystemCollector::updateCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        std::cerr << "[SystemCollector] GetSystemTimes failed, error code: " << GetLastError() << '\n';
        return;
    }

    ULARGE_INTEGER idle, kernel, user;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    // Lần gọi đầu tiên chưa có mẫu trước đó để so sánh, chỉ lưu lại và bỏ qua tính toán
    if (!hasPrevSample) {
        prevIdleTime = idle;
        prevKernelTime = kernel;
        prevUserTime = user;
        hasPrevSample = true;
        return;
    }

    // KernelTime của Windows đã bao gồm cả IdleTime, nên tổng thời gian CPU = kernel + user
    const ULONGLONG idleDiff = idle.QuadPart - prevIdleTime.QuadPart;
    const ULONGLONG kernelDiff = kernel.QuadPart - prevKernelTime.QuadPart;
    const ULONGLONG userDiff = user.QuadPart - prevUserTime.QuadPart;
    const ULONGLONG totalDiff = kernelDiff + userDiff;

    if (totalDiff > 0) {
        const ULONGLONG busyDiff = totalDiff - idleDiff;
        currentStats.cpuUsage = (static_cast<double>(busyDiff) / static_cast<double>(totalDiff)) * 100.0;
    }

    prevIdleTime = idle;
    prevKernelTime = kernel;
    prevUserTime = user;
}

void SystemCollector::updateRamUsage() {
    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(memStatus);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        std::cerr << "[SystemCollector] GlobalMemoryStatusEx failed, error code: " << GetLastError() << '\n';
        return;
    }

    constexpr uint64_t BYTES_PER_MB = 1024ULL * 1024ULL;
    currentStats.totalRamMB = memStatus.ullTotalPhys / BYTES_PER_MB;
    const uint64_t availRamMB = memStatus.ullAvailPhys / BYTES_PER_MB;
    currentStats.usedRamMB = currentStats.totalRamMB - availRamMB;
    currentStats.ramUsagePercent = static_cast<double>(memStatus.dwMemoryLoad);
}
