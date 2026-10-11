#include <iostream>
#include <thread>
#include <chrono>
#include <conio.h>
#include "../include/process_manager.hpp"
#include "../include/system_collector.hpp"
#include "../include/terminal_ui.hpp"

int main() {
    SystemCollector collector;
    TerminalUI ui;
    ProcessManager processManager;

    ui.init();

    // Xóa màn hình một lần duy nhất lúc khởi động; mỗi khung hình sau chỉ vẽ đè lên vị trí cũ
    std::cout << "\033[2J\033[H\033[?25l" << "Nhan 'q' hoac ESC de thoat...\n" << std::flush;

    bool running = true;
    while (running) {
        collector.update();
        const auto processes = processManager.getTopProcesses(10, SortCriteria::SORT_BY_RAM);
        ui.render(collector.getStats(), processes);

        // Chia khoảng nghỉ 1 giây thành các bước 100ms để phản hồi phím thoát nhanh hơn,
        // vẫn đảm bảo không chiếm dụng CPU trong lúc chờ
        for (int elapsed = 0; elapsed < 1000 && running; elapsed += 100) {
            if (_kbhit()) {
                const int key = _getch();
                if (key == 'q' || key == 'Q' || key == 27) {
                    running = false;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Khôi phục con trỏ và màu mặc định của console trước khi thoát
    std::cout << "\033[?25h\033[0m" << std::endl;
    return 0;
}
