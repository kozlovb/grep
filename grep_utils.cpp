#include "grep_utils.h"

#include <fstream>
#include <string>
#include <iostream>

namespace grep
{
    std::pair<std::string, std::string> read_grep_arguments(int argc, char *argv[])
    {
        if (argc != 3)
        {
            std::cerr << "Error: Invalid input. Please provide exactly two arguments:\n"
                         "1. A search pattern (string).\n"
                         "2. A path to a file or directory.\n"
                         "Usage: grep_test <pattern> <file_or_directory_path>"
                      << std::endl;
            return {"", ""};
        }
        return {std::string(argv[1]), std::string(argv[2])};
    }

    void find_files(fs::path startPath, std::function<void(fs::path)> submit_to_queue)
    {
        if (!fs::exists(startPath))
        {
            std::cerr << "Error: Path does not exist: " << startPath << std::endl;
            return;
        }

        if (fs::is_regular_file(startPath))
        {
            submit_to_queue(startPath);
            return;
        }

        if (fs::is_directory(startPath) && !fs::is_symlink(startPath))
        {
            for (const auto &entry : fs::recursive_directory_iterator(startPath, fs::directory_options::skip_permission_denied))
            {
                if (!fs::is_symlink(entry) && fs::is_regular_file(entry))
                {
                    submit_to_queue(entry.path());
                }
            }
        }
        else
        {
            std::cerr << "Error: Path is not a file or directory: " << startPath << std::endl;
        }
    }

    void output_colored_result(const Result &res)
    {
        for (const auto &r : res.results)
        {
            const std::string &text = r.first;
            const std::vector<MatchPosition> &matchPositions = r.second;

            size_t lastPos = 0;
            std::string_view lineView(text);

            std::cout << res.file_name << ": "; // Print file name

            for (const auto &matchPosition : matchPositions)
            {
                if (matchPosition.start > lastPos)
                {
                    std::cout << lineView.substr(lastPos, matchPosition.start - lastPos);
                }
                std::cout << ColorStart
                          << lineView.substr(matchPosition.start, matchPosition.end - matchPosition.start + 1)
                          << ColorEnd;

                lastPos = matchPosition.end + 1;
            }

            if (lastPos < text.size())
            {
                std::cout << lineView.substr(lastPos);
            }
            std::cout << std::endl;
        }
    }

    Result search_in_file(const std::string &m_pattern, const fs::path &filepath)
    {
        std::ifstream file(filepath);
        Result::LineMatchResults results;

        if (!file.is_open())
        {
            return Result();
        }

        std::string line;
        while (std::getline(file, line))
        {
            size_t pos = 0;
            std::vector<MatchPosition> matchPositions;

            while ((pos = line.find(m_pattern, pos)) != std::string::npos)
            {
                size_t end_pos = pos + m_pattern.length() - 1;
                matchPositions.emplace_back(pos, end_pos);
                pos += m_pattern.length();
            }
            if (!matchPositions.empty())
            {
                results.emplace_back(std::move(line), std::move(matchPositions));
            }
        }

        return Result(filepath.string(), std::move(results));
    }
}
