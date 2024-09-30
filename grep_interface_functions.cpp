#include "grep_interface_functions.h"

#include <iostream>

#include "grep_utils.h"
#include "threadmanager.h"
#include "result.h"

namespace grep
{
    void grep(int argc, char *argv[])
    {

        auto [pattern, path] = grep::read_grep_arguments(argc, argv);
        if (pattern.empty())
        {
            return;
        }

        grep::grep(std::move(pattern), std::move(path));
    }

    void grep(std::string pattern, std::string path)
    {
        try
        {
            ThreadManager threadManager(std::move(path), std::move(pattern));
            threadManager.search();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception caught in grep: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unknown exception caught in grep." << std::endl;
        }
    }
}
