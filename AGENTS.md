# AGENTS.md - Hướng dẫn phát triển dự án sysmon-cli

Tài liệu này định nghĩa mục tiêu, quy chuẩn kỹ thuật và quy tắc làm việc cho AI agents khi tham gia phát triển dự án **sysmon-cli**.

---

## 1. Tổng quan dự án (Project Overview)
* **Tên dự án:** `sysmon-cli` (Windows Real-time System & Process Monitor)
* **Mô tả:** Ứng dụng dòng lệnh (CLI) viết bằng C++ nhằm giám sát hiệu năng hệ thống (CPU, RAM) và quản lý tiến trình (Process Management) theo thời gian thực trên hệ điều hành Windows.
* **Mục tiêu:** 
  * Tối ưu hiệu năng thô và quản lý bộ nhớ an toàn bằng C++.
  * Khai thác trực tiếp Windows API cấp thấp.
  * Cung cấp giao diện Terminal trực quan, mượt mà.

---

## 2. Ngăn xếp công nghệ (Tech Stack)
* **Ngôn ngữ:** C++17 / C++20
* **Nền tảng mục tiêu:** Windows OS (x64)
* **Thư viện chính:** 
  * Windows API (`<windows.h>`, `<psapi.h>`, `<tlhelp32.h>`) để lấy thông tin hệ thống và tiến trình.
  * C++ Standard Library (`<vector>`, `<string>`, `<thread>`, `<chrono>`, `<filesystem>`).
  * ANSI Escape Sequences (hoặc thư viện TUI nhẹ) để vẽ giao diện Terminal.
* **Trình biên dịch/Công cụ:** MSVC hoặc MinGW-w64, VS Code / Visual Studio, CMake (nếu cần quản lý build).

---

## 3. Kiến trúc hệ thống dự kiến (Architecture)
Dự án được thiết kế theo mô hình mô-đun rõ ràng:
1. **`SystemCollector` (Thu thập dữ liệu):** Các hàm/class gọi Windows API để lấy thông tin CPU, RAM và danh sách tiến trình định kỳ.
2. **`ProcessManager` (Quản lý tiến trình):** Xử lý logic tìm kiếm, sắp xếp, lọc và thao tác với tiến trình (ví dụ: Kill process bằng PID).
3. **`TerminalUI` (Giao diện hiển thị):** Xử lý việc làm mới màn hình Terminal, vẽ biểu đồ thanh (Bar Chart) và định dạng bảng dữ liệu gọn gàng.
4. **`Main Loop` (Luồng điều khiển chính):** Điều phối chu kỳ cập nhật (ví dụ: quét phần cứng mỗi 1 giây) và lắng nghe phím bấm từ người dùng.

---

## 4. Quy chuẩn lập trình C++ (Coding Guidelines)
* **Quản lý bộ nhớ an toàn:** Ưu tiên sử dụng Smart Pointers (`std::unique_ptr`, `std::shared_ptr`) thay vì quản lý con trỏ thô (`new`/`delete`) để tránh rò rỉ bộ nhớ (memory leaks).
* **Modern C++:** Tận dụng các tính năng từ C++17 trở lên như `std::string_view`, cấu trúc `auto`, range-based for loops, và thư viện `<chrono>`.
* **Xử lý lỗi (Error Handling):** Kiểm tra mã trả về của các hàm Windows API (`GetLastError()`) và xử lý ngoại lệ cẩn thận, tránh crash ứng dụng khi không đọc được tiến trình hệ thống (Access Denied).
* **Tách biệt logic và giao diện:** Code logic hệ thống phải độc lập với code vẽ giao diện terminal để dễ bảo trì và kiểm thử.

---

## 5. Quy tắc tương tác với AI (AI Vibe Coding Rules)
Khi AI hỗ trợ viết code cho dự án này, cần tuân thủ các nguyên tắc sau:
1. **Đầy đủ và Biên dịch được:** Các đoạn code C++ cung cấp phải hoàn chỉnh, có bao gồm các thư viện cần thiết (`#include`), tránh bỏ sót hàm hoặc biến chưa khai báo.
2. **Giải thích ngắn gọn:** Cung cấp giải thích logic đằng sau các hàm Windows API phức tạp (vì các hàm này thường có cấu trúc rườm rà).
3. **An toàn hệ thống:** Không đề xuất các mã lệnh có nguy cơ can thiệp nguy hiểm vào tiến trình hệ thống cốt lõi (Critical System Processes) mà không có cảnh báo rõ ràng.