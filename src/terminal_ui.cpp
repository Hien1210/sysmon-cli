#include "terminal_ui.hpp"

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace {

// Chọn màu ANSI theo mức tải để cảnh báo trực quan (xanh/vàng/đỏ)
const char* colorForPercent(double percent) {
    if (percent >= 85.0) return "\033[1;31m"; // đỏ: tải cao
    if (percent >= 60.0) return "\033[1;33m"; // vàng: tải trung bình
    return "\033[1;32m";                      // xanh lá: bình thường
}

constexpr const char* ANSI_RESET = "\033[0m";
constexpr const char* ANSI_SYSTEM_PROCESS_COLOR = "\033[90m"; // xám nhạt: tiến trình hệ thống

// Cắt ngắn tên tiến trình nếu quá dài để không phá vỡ bố cục cột
std::string truncateName(const std::string& name, size_t maxLen) {
    return name.size() <= maxLen ? name : name.substr(0, maxLen);
}

void appendProcessTable(std::ostringstream& out, const std::vector<ProcessInfo>& processes) {
    out << "\033[1;36m=== TOP PROCESSES ===\033[0m\033[K\n";
    out << "\033[K\n";

    out << "\033[1m" << std::left
        << std::setw(8) << "PID"
        << std::setw(26) << "PROCESS NAME"
        << std::right << std::setw(8) << "CPU %"
        << std::setw(11) << "RAM (MB)"
        << "  " << std::left << std::setw(6) << "TYPE"
        << ANSI_RESET << "\033[K\n";

    for (const auto& process : processes) {
        const char* rowColor = process.isSystemProcess ? ANSI_SYSTEM_PROCESS_COLOR : "";
        const char* typeLabel = process.isSystemProcess ? "[SYS]" : "USER";

        out << rowColor << std::left
            << std::setw(8) << process.pid
            << std::setw(26) << truncateName(process.name, 25)
            << std::right << std::fixed << std::setprecision(1) << std::setw(7) << process.cpuUsagePercent << "%"
            << std::setw(11) << process.ramUsageMB
            << "  " << std::left << std::setw(6) << typeLabel
            << ANSI_RESET << "\033[K\n";
    }
}

} // namespace

void TerminalUI::init() {
    // Bật UTF-8 để hiển thị đúng ký tự khối █ ░ trong thanh tiến trình
    SetConsoleOutputCP(CP_UTF8);

    // Bật Virtual Terminal Processing để console Windows hiểu mã ANSI Escape Sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        std::cerr << "[TerminalUI] GetStdHandle failed, error code: " << GetLastError() << '\n';
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        std::cerr << "[TerminalUI] GetConsoleMode failed, error code: " << GetLastError() << '\n';
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        std::cerr << "[TerminalUI] SetConsoleMode failed, error code: " << GetLastError() << '\n';
    }
}

std::string TerminalUI::drawBar(double percent, int width) const {
    if (percent < 0.0) percent = 0.0;
    if (percent > 100.0) percent = 100.0;

    const int filled = static_cast<int>((percent / 100.0) * width);

    std::string bar;
    bar.reserve(static_cast<size_t>(width) * 3); // █/░ là ký tự UTF-8 3 byte
    for (int i = 0; i < width; ++i) {
        bar += (i < filled) ? "\xE2\x96\x88" : "\xE2\x96\x91"; // █ : ░
    }
    return bar;
}

void TerminalUI::render(const SystemStats& stats, const std::vector<ProcessInfo>& processes) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    // "\033[H" đưa con trỏ về đầu màn hình thay vì xóa toàn bộ (tránh chớp nháy);
    // "\033[K" ở cuối mỗi dòng xóa phần ký tự cũ còn sót lại khi nội dung mới ngắn hơn
    out << "\033[H";
    out << "\033[1;36m=== SYSMON-CLI DASHBOARD ===\033[0m\033[K\n";
    out << "\033[K\n";

    out << "\033[1mCPU Usage:\033[0m " << stats.cpuUsage << "%\033[K\n";
    out << colorForPercent(stats.cpuUsage) << "[" << drawBar(stats.cpuUsage) << "]" << ANSI_RESET << "\033[K\n";
    out << "\033[K\n";

    out << "\033[1mRAM Usage:\033[0m " << stats.usedRamMB << " MB / " << stats.totalRamMB
        << " MB (" << stats.ramUsagePercent << "%)\033[K\n";
    out << colorForPercent(stats.ramUsagePercent) << "[" << drawBar(stats.ramUsagePercent) << "]" << ANSI_RESET << "\033[K\n";
    out << "\033[K\n";

    appendProcessTable(out, processes);

    // Xóa mọi dòng cũ còn sót bên dưới (ví dụ khi danh sách tiến trình ngắn hơn lần render trước)
    out << "\033[0J";

    std::cout << out.str() << std::flush;
}
