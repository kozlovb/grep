#include "grep_utils.h"
#include "threadmanager.h"

#include <thread>
#include <functional>
#include "result.h"
#include <iostream>

namespace grep
{
    void ThreadManager::search()
    {
        std::vector<std::thread> threadPool;

        unsigned int numCores = std::max(1u, std::thread::hardware_concurrency());

        std::thread output_thread(std::bind(&ThreadManager::output_results, this));

        for (unsigned int i = 0; i < numCores; ++i)
        {
            threadPool.emplace_back(std::bind(&ThreadManager::collect, this));
        }

        std::thread find_files_thread(&find_files,
                                      m_pathStart,
                                      [this](fs::path path)
                                      {
                                          std::unique_lock<std::mutex> lock(m_fileQueueMutex);
                                          m_fileQueue.push(path);
                                          m_cvInput.notify_all();
                                      });

        find_files_thread.join();
        m_stopCollectFlag = true;
        m_cvInput.notify_all();

        for (auto &thread : threadPool)
        {
            thread.join();
        }

        m_stopOutputFlag = true;
        m_cvOutput.notify_one();
        output_thread.join();
    }

    void ThreadManager::collect()
    {
        while (!m_fileQueue.empty() || !m_stopCollectFlag)
        {
            fs::path filepath;
            {
                std::unique_lock<std::mutex> lock(m_fileQueueMutex);

                m_cvInput.wait(lock, [this]
                               { return !m_fileQueue.empty() || m_stopCollectFlag; });

                if (!m_fileQueue.empty())
                {

                    filepath = std::move(m_fileQueue.front());
                    m_fileQueue.pop();
                }
            }

            auto res = search_in_file(m_pattern, filepath);

            if (!res.empty())
            {
                std::lock_guard<std::mutex> lock(m_resultQueueMutex);
                m_resultQueue.push(std::move(res));
                m_cvOutput.notify_one();
            }
        }
    }

    void ThreadManager::output_results()
    {
        while (!m_stopOutputFlag || !m_resultQueue.empty())
        {
            Result res;
            {
                std::unique_lock<std::mutex> lock(m_resultQueueMutex);
                m_cvOutput.wait(lock, [this]
                                { return !m_resultQueue.empty() || m_stopOutputFlag; });

                if (!m_resultQueue.empty())
                {
                    res = std::move(m_resultQueue.front());
                    m_resultQueue.pop();
                }
            }
            if (!res.empty())
            {
                output_colored_result(res);
            }
        }
    }
}