#pragma once

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <queue>

namespace grep
{
    struct Result;

    namespace fs = std::filesystem;

    class ThreadManager
    {
    public:
        ThreadManager(fs::path path, std::string pattern)
            : m_pathStart(std::move(path)), m_pattern(std::move(pattern))
        {}

    public:
        void search();

    private:
        void collect();
        void output_results();

    private:
        std::string m_pattern;
        std::queue<fs::path> m_fileQueue;
        fs::path m_pathStart;
        std::queue<Result> m_resultQueue;
        std::mutex m_resultQueueMutex;
        std::mutex m_fileQueueMutex;
        std::condition_variable m_cvInput;
        std::condition_variable m_cvOutput;
        std::atomic<bool> m_stopOutputFlag{false};
        std::atomic<bool> m_stopCollectFlag{false};
    };
}