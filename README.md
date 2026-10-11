# sysmon-cli

Công cụ giám sát hệ thống thời gian thực (CPU, RAM, tiến trình) chạy trên dòng lệnh Windows, viết bằng C++17 và Windows API nguyên bản.

![Platform](https://img.shields.io/badge/platform-Windows-0078D6)
![Language](https://img.shields.io/badge/C%2B%2B-17-00599C)

## Demo

```
=== SYSMON-CLI DASHBOARD ===

CPU Usage: 58.4%
[█████████████████░░░░░░░░░░░░░]

RAM Usage: 12117 MB / 16316 MB (74.0%)
[██████████████████████░░░░░░░░]

=== TOP PROCESSES ===

PID     PROCESS NAME               CPU %   RAM (MB)  TYPE
17176   chrome.exe                  24.9%        826  USER
4196    claude.exe                   1.8%        569  USER
1444    Gemini.exe                   0.0%        493  USER
```

Dashboard tự làm mới mỗi giây, không chớp nháy màn hình; tiến trình hệ thống (`svchost.exe`, `csrss.exe`, ...) được đánh dấu `[SYS]` và tô màu khác biệt.

## Tính năng

- **Theo dõi CPU/RAM thời gian thực** qua `GetSystemTimes` và `GlobalMemoryStatusEx`.
- **Dashboard terminal** vẽ bằng ANSI Escape Sequences, có thanh tiến trình (progress bar) đổi màu theo mức tải (xanh/vàng/đỏ).
- **Danh sách Top tiến trình** sắp xếp theo RAM hoặc CPU, hỗ trợ lọc theo tên.
- **Kết thúc tiến trình (kill) theo PID**, tự động chặn thao tác với tiến trình hệ thống quan trọng (`System`, `csrss.exe`, `smss.exe`, `svchost.exe`, ...).
- Thoát mượt mà bằng phím `q` hoặc `ESC`, không chiếm dụng CPU khi chờ (polling 100ms).

## Kiến trúc

| Module | File | Vai trò |
|---|---|---|
| `SystemCollector` | [`include/system_collector.hpp`](include/system_collector.hpp), [`src/system_collector.cpp`](src/system_collector.cpp) | Thu thập dữ liệu CPU/RAM toàn hệ thống |
| `ProcessManager` | [`include/process_manager.hpp`](include/process_manager.hpp), [`src/process_manager.cpp`](src/process_manager.cpp) | Liệt kê, sắp xếp, lọc và kết thúc tiến trình qua Toolhelp32/PSAPI |
| `TerminalUI` | [`include/terminal_ui.hpp`](include/terminal_ui.hpp), [`src/terminal_ui.cpp`](src/terminal_ui.cpp) | Vẽ dashboard và bảng tiến trình trên terminal |
| `main` | [`src/main.cpp`](src/main.cpp) | Vòng lặp điều phối chính, xử lý phím thoát |

## Yêu cầu

- Windows 10/11 (x64)
- Trình biên dịch hỗ trợ C++17: MinGW-w64 (`g++`) hoặc MSVC Build Tools

## Build & chạy

### Dùng g++ (MinGW-w64)

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp src/system_collector.cpp src/terminal_ui.cpp src/process_manager.cpp -o sysmon-cli.exe -lpsapi
```

```bash
./sysmon-cli.exe
```

### Dùng VS Code

Mở project trong VS Code, nhấn `Ctrl+Shift+B` để chạy task **Build sysmon-cli (g++)** (đã cấu hình sẵn trong [`.vscode/tasks.json`](.vscode/tasks.json)).

### Thoát ứng dụng

Nhấn `q`, `Q` hoặc `ESC` trong lúc chương trình đang chạy.

## Quy chuẩn & đóng góp

Xem [`CLAUDE.md`](CLAUDE.md) và [`AGENTS.md`](AGENTS.md) để biết quy ước đặt tên, kiến trúc dự kiến và nguyên tắc an toàn khi đóng góp code (đặc biệt là xử lý Windows API và quản lý bộ nhớ).
