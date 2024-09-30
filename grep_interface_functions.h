#pragma once

#include <string>

namespace grep
{    
    void grep(int argc, char *argv[]);
    void grep(std::string pattern, std::string path);
}


