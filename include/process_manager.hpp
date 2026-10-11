#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>

struct ProcessInfo {
    DWORD pid = 0;
    std::string name;
    double cpuUsagePercent = 0.0;
    uint64_t ramUsageMB = 0;
    bool isSystemProcess = false;
};

enum SortCriteria {
    SORT_BY_RAM,
    SORT_BY_CPU
};

class ProcessManager {
public:
    std::vector<ProcessInfo> getTopProcesses(size_t limit = 10,
                                              SortCriteria sortBy = SORT_BY_RAM,
                                              const std::string& filter = "");
    bool killProcess(DWORD pid);

private:
    struct CpuSample {
        ULONGLONG totalTime;
    };

    // Trạng thái lần đo CPU trước (theo PID), dùng để tính delta giữa các lần gọi getTopProcesses()
    std::unordered_map<DWORD, CpuSample> prevCpuSamples;
    ULONGLONG prevSampleTimestamp = 0;
    bool hasPrevSample = false;

    std::vector<std::pair<DWORD, std::string>> snapshotProcessList() const;
    std::string getProcessNameByPid(DWORD pid) const;
    static bool isSystemProcessName(const std::string& name);
};
