#pragma once

#include <functional>
#include <filesystem>

#include "result.h"

namespace grep
{
    namespace fs = std::filesystem;
    const std::string ColorStart = "\033[1;31m"; // ANSI escape code for red text
    const std::string ColorEnd = "\033[0m";      // ANSI escape code to reset color
    std::pair<std::string, std::string> read_grep_arguments(int argc, char *argv[]);
    void find_files(fs::path startPath, std::function<void(fs::path)> submit_to_queue);
    void output_colored_result(const Result &res);
    Result search_in_file(const std::string &pattern, const fs::path &filepath);
}
