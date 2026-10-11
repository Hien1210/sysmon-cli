#pragma once

#include <cstdint>
#include <windows.h>

struct SystemStats {
    double cpuUsage = 0.0;
    uint64_t totalRamMB = 0;
    uint64_t usedRamMB = 0;
    double ramUsagePercent = 0.0;
};

class SystemCollector {
public:
    void update();
    SystemStats getStats() const;

private:
    SystemStats currentStats;

    // Trạng thái lần đo CPU trước, dùng để tính % theo khoảng thời gian giữa 2 lần update()
    ULARGE_INTEGER prevIdleTime{};
    ULARGE_INTEGER prevKernelTime{};
    ULARGE_INTEGER prevUserTime{};
    bool hasPrevSample = false;

    void updateCpuUsage();
    void updateRamUsage();
};
