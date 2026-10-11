#include "process_manager.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <psapi.h>
#include <tlhelp32.h>
#include <type_traits>

namespace {

using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&::CloseHandle)>;

UniqueHandle makeHandle(HANDLE h) {
    return UniqueHandle(h, &::CloseHandle);
}

std::string toLower(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

// Mốc thời gian hệ thống dạng 100-ns, cùng đơn vị với FILETIME của GetProcessTimes
ULONGLONG currentFileTime100ns() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER t;
    t.LowPart = ft.dwLowDateTime;
    t.HighPart = ft.dwHighDateTime;
    return t.QuadPart;
}

DWORD getNumberOfProcessors() {
    SYSTEM_INFO sysInfo{};
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors > 0 ? sysInfo.dwNumberOfProcessors : 1;
}

// Danh sách tên tiến trình hệ thống quan trọng, không được phép kết thúc qua killProcess()
const std::vector<std::string>& systemProcessNames() {
    static const std::vector<std::string> names = {
        "system idle process", "system", "idle", "registry",
        "csrss.exe", "smss.exe", "wininit.exe", "winlogon.exe",
        "services.exe", "lsass.exe", "svchost.exe"
    };
    return names;
}

} // namespace

bool ProcessManager::isSystemProcessName(const std::string& name) {
    const std::string lowerName = toLower(name);
    const auto& names = systemProcessNames();
    return std::find(names.begin(), names.end(), lowerName) != names.end();
}

std::vector<std::pair<DWORD, std::string>> ProcessManager::snapshotProcessList() const {
    std::vector<std::pair<DWORD, std::string>> list;

    const HANDLE rawSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (rawSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[ProcessManager] CreateToolhelp32Snapshot failed, error code: " << GetLastError() << '\n';
        return list;
    }
    UniqueHandle snapshot = makeHandle(rawSnapshot);

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(entry);

    if (!Process32First(snapshot.get(), &entry)) {
        std::cerr << "[ProcessManager] Process32First failed, error code: " << GetLastError() << '\n';
        return list;
    }

    do {
        list.emplace_back(entry.th32ProcessID, std::string(entry.szExeFile));
    } while (Process32Next(snapshot.get(), &entry));

    return list;
}

std::string ProcessManager::getProcessNameByPid(DWORD pid) const {
    for (const auto& entry : snapshotProcessList()) {
        if (entry.first == pid) {
            return entry.second;
        }
    }
    return "";
}

std::vector<ProcessInfo> ProcessManager::getTopProcesses(size_t limit, SortCriteria sortBy, const std::string& filter) {
    const auto rawList = snapshotProcessList();
    if (rawList.empty()) {
        return {};
    }

    const ULONGLONG now = currentFileTime100ns();
    const DWORD numProcessors = getNumberOfProcessors();
    const std::string lowerFilter = toLower(filter);

    // Danh sách mẫu CPU mới, thay thế prevCpuSamples ở cuối hàm — tự động loại bỏ
    // các PID đã thoát khỏi lần đo trước thay vì để chúng tích tụ mãi trong map
    std::unordered_map<DWORD, CpuSample> newSamples;
    newSamples.reserve(rawList.size());

    std::vector<ProcessInfo> result;
    result.reserve(rawList.size());

    for (const auto& entry : rawList) {
        const DWORD pid = entry.first;
        const std::string& name = entry.second;

        if (!lowerFilter.empty() && toLower(name).find(lowerFilter) == std::string::npos) {
            continue;
        }

        ProcessInfo info;
        info.pid = pid;
        info.name = name;
        info.isSystemProcess = isSystemProcessName(name);

        // Mở tiến trình với quyền tối thiểu; nếu thất bại (System/Idle/tiến trình được bảo vệ)
        // thì bỏ qua và giữ RAM/CPU mặc định 0 thay vì làm sập ứng dụng
        UniqueHandle hProcess = makeHandle(
            OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid));

        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc{};
            if (GetProcessMemoryInfo(hProcess.get(), &pmc, sizeof(pmc))) {
                info.ramUsageMB = static_cast<uint64_t>(pmc.WorkingSetSize) / (1024ULL * 1024ULL);
            }

            FILETIME creationTime{}, exitTime{}, kernelTime{}, userTime{};
            if (GetProcessTimes(hProcess.get(), &creationTime, &exitTime, &kernelTime, &userTime)) {
                ULARGE_INTEGER kernel, user;
                kernel.LowPart = kernelTime.dwLowDateTime;
                kernel.HighPart = kernelTime.dwHighDateTime;
                user.LowPart = userTime.dwLowDateTime;
                user.HighPart = userTime.dwHighDateTime;
                const ULONGLONG totalProcessTime = kernel.QuadPart + user.QuadPart;

                const auto prevIt = prevCpuSamples.find(pid);
                if (hasPrevSample && prevIt != prevCpuSamples.end() && now > prevSampleTimestamp) {
                    const ULONGLONG processTimeDiff = totalProcessTime - prevIt->second.totalTime;
                    const ULONGLONG wallTimeDiff = now - prevSampleTimestamp;
                    info.cpuUsagePercent = (static_cast<double>(processTimeDiff) / static_cast<double>(wallTimeDiff))
                                            * 100.0 / static_cast<double>(numProcessors);
                }

                newSamples[pid] = CpuSample{totalProcessTime};
            }
        }

        result.push_back(std::move(info));
    }

    prevCpuSamples = std::move(newSamples);
    prevSampleTimestamp = now;
    hasPrevSample = true;

    std::sort(result.begin(), result.end(), [sortBy](const ProcessInfo& a, const ProcessInfo& b) {
        return sortBy == SORT_BY_CPU ? a.cpuUsagePercent > b.cpuUsagePercent
                                      : a.ramUsageMB > b.ramUsageMB;
    });

    if (result.size() > limit) {
        result.resize(limit);
    }

    return result;
}

bool ProcessManager::killProcess(DWORD pid) {
    const std::string processName = getProcessNameByPid(pid);
    if (processName.empty()) {
        std::cerr << "[ProcessManager] killProcess: khong tim thay tien trinh PID " << pid << '\n';
        return false;
    }

    if (isSystemProcessName(processName)) {
        std::cerr << "[ProcessManager] killProcess: tu choi ket thuc tien trinh he thong \""
                  << processName << "\" (PID " << pid << ")\n";
        return false;
    }

    UniqueHandle hProcess = makeHandle(OpenProcess(PROCESS_TERMINATE, FALSE, pid));
    if (!hProcess) {
        std::cerr << "[ProcessManager] OpenProcess(PROCESS_TERMINATE) failed for PID " << pid
                  << ", error code: " << GetLastError() << '\n';
        return false;
    }

    if (!TerminateProcess(hProcess.get(), 1)) {
        std::cerr << "[ProcessManager] TerminateProcess failed for PID " << pid
                  << ", error code: " << GetLastError() << '\n';
        return false;
    }

    return true;
}
