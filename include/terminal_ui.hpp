#pragma once

#include <string>
#include <vector>
#include "process_manager.hpp"
#include "system_collector.hpp"

class TerminalUI {
public:
    void init();
    void render(const SystemStats& stats, const std::vector<ProcessInfo>& processes);

private:
    std::string drawBar(double percent, int width = 30) const;
};
