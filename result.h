#pragma once

#include <string>
#include <vector>

namespace grep
{
    struct MatchPosition
    {
        MatchPosition(size_t start, size_t end)
            : start(start), end(end){};

        bool operator==(const MatchPosition &other) const
        {
            return start == other.start && end == other.end;
        }

        bool operator!=(const MatchPosition &other) const
        {
            return !(*this == other);
        }

        size_t start;
        size_t end;
    };

    struct Result
    {
        using LineMatchResults = std::vector<std::pair<std::string, std::vector<MatchPosition>>>;
        Result() = default;
        Result(const std::string &file_name, LineMatchResults results)
            : file_name(std::move(file_name)), results(std::move(results))
        {
        }
        Result(Result &&other)
            : file_name(std::move(other.file_name)), results(std::move(other.results))
        {
        }
        Result &operator=(Result &&other)
        {
            if (this != &other)
            {
                file_name = std::move(other.file_name);
                results = std::move(other.results);
            }
            return *this;
        }

        bool operator==(const Result &other) const
        {
            return file_name == other.file_name && results == other.results; // Vectors compare element-wise
        }

        bool operator!=(const Result &other) const
        {
            return !(*this == other);
        }

        std::string file_name;
        LineMatchResults results;
        bool empty() { return results.empty(); }
    };
}