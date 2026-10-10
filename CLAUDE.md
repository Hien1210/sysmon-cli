# CLAUDE.md - Ngữ cảnh & Hướng dẫn cho Claude

Tài liệu này chứa các hướng dẫn, quy tắc làm việc và lệnh biên dịch/chạy để Claude hỗ trợ hiệu quả cho dự án `sysmon-cli`.

---

## 1. Lệnh Biên dịch & Chạy Nhanh
- **Trình biên dịch:** MSVC (Visual Studio Toolchain) hoặc MinGW-w64 (`g++`)
- **Tiêu chuẩn C++:** C++17 / C++20
- **Lệnh biên dịch (MinGW):**
  `g++ -std=c++17 -O2 src/main.cpp -o sysmon-cli.exe -lpsapi`
- **Build qua CMake (nếu áp dụng):**
  - Cấu hình: `cmake -B build -S .`
  - Build: `cmake --build build --config Release`
- **Chạy ứng dụng:** `./sysmon-cli.exe` hoặc `.\build\Release\sysmon-cli.exe`

---

## 2. Quy chuẩn Mã nguồn & Kiến trúc

### Quy tắc đặt tên (Naming Conventions)
- **File & Thư mục:** Chữ thường, phân tách bằng dấu gạch dưới (ví dụ: `system_collector.cpp`, `process_manager.hpp`).
- **Lớp / Cấu trúc (Classes / Structs):** Viết hoa chữ cái đầu mỗi từ - PascalCase (ví dụ: `SystemCollector`, `ProcessInfo`).
- **Hàm / Phương thức (Functions / Methods):** camelCase (ví dụ: `getCpuUsage()`, `killProcessByPid()`).
- **Biến (Variables):** camelCase (ví dụ: `totalMemory`, `processList`).
- **Hằng số (Constants):** Viết hoa toàn bộ, phân tách bằng dấu gạch dưới (ví dụ: `MAX_BUFFER_SIZE`, `REFRESH_INTERVAL_MS`).

### Quản lý Bộ nhớ & An toàn Hệ thống
- **Windows API:** Luôn kiểm tra giá trị trả về và kiểm tra `GetLastError()`. Đóng các handle sạch sẽ bằng `CloseHandle()` hoặc các cơ chế RAII.
- **Smart Pointers:** Tránh dùng con trỏ thô `new`/`delete`. Ưu tiên sử dụng `std::unique_ptr` và `std::shared_ptr`.
- **Kiểu chuỗi (String Types):** Sử dụng `std::string` hoặc `std::string_view`. Xử lý chuỗi rộng của Windows (`std::wstring` / `LPCWSTR`) cẩn thận khi tương tác với Win32 APIs.

---

## 3. Quy trình làm việc & Kỳ vọng Phản hồi cho Claude

1. **Mã nguồn tự chứa (Self-Contained Code):** Khi được yêu cầu viết mã C++, hãy cung cấp các đoạn code hoàn chỉnh, có thể biên dịch được, bao gồm tất cả các thư viện cần thiết (`#include <windows.h>`, `<psapi.h>`, v.v.).
2. **Hạn chế thư viện ngoài:** Bám sát thư viện chuẩn C++ (STL) và Windows API nguyên bản trừ khi được yêu cầu rõ ràng tích hợp thư viện bên thứ ba (ví dụ: `FTXUI`, `ncurses`).
3. **Giải thích cô đọng:** Tập trung vào chi tiết triển khai, cách sử dụng API và lý do xử lý lỗi. Tránh viết văn quá dài dòng trừ khi có yêu cầu.
4. **Lặp lại tương tác:** Chia nhỏ các tính năng phức tạp thành các bước tăng dần (logic lõi -> tích hợp Win32 -> định dạng giao diện).