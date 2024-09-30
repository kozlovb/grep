# Grep

This is a simplified version of grep, more precisely mimics "grep -r" for patterns.
Regex exprssions are not supported.

## Requirements

C++17 compiler
Developed and tested on Ubuntu 18.04, not tested on other OSs.
No gurantee for other OSs.

### Build

To build the executable, run:
```sh
g++-11 -std=c++17 main.cpp threadmanager.cpp grep_interface_functions.cpp grep_utils.cpp -o simple_grep -lpthread
```
To build tests run:
```sh
g++-11 -std=c++17 tests.cpp threadmanager.cpp grep_interface_functions.cpp grep_utils.cpp -o tests -lpthread
```

### Run

```sh
./simple_grep pattern path
```

Example using all the parameters:
```sh
./simple_grep match ./ 
```

Run tests:
```sh
./tests
```
## Features

 - Similar to grep -r, this tool outputs the filename followed by lines containing the search pattern.
 - Pattern Highlighting: All occurrences of the pattern in a line are highlighted in red.
 - Organized Output: Lines from one file are grouped together and are not mixed with lines from other files.
 - Recursive Search: The tool recursively searches through directories if a folder path is provided. It can also accept file paths directly.
 - Symbolic Links: Symbolic links to files and folders are ignored, except when the path points directly to a file (similar behavior to grep -r on Ubuntu 18).

## Architecture

The main code is organized into free functions, mostly placed in grep_utils.cpp. The multithreaded operation is managed by a ThreadManager class that synchronizes interactions between threads. Below is an overview of the architecture and workflow.

                  +--------------------+
                  |    find_files()     |
                  | (File Discovery)    |
                  +--------------------+
                            |
                            v
               +--------------------------+
               |      fileQueue (Mutex)    |
               +--------------------------+
                            |
                            v
+--------------------+      +--------------------+      +--------------------+
| Thread 1           |      | Thread 2           |      | Thread N           |
| search_in_file()   | ---  | search_in_file()   | ---  | search_in_file()   |
+--------------------+      +--------------------+      +--------------------+
                            |
                            v
               +--------------------------+
               |    resultQueue (Mutex)    |
               +--------------------------+
                            |
                            v
                  +--------------------+
                  |  Output Thread      |
                  | (Prints Results)    |
                  +--------------------+

## Workflow Overview

 - File Discovery: The find_files function recursively scans directories for files and submits them to a fileQueue.
 - File Processing: Files are processed by a member function called collect which retrieves file names from the fileQueue and launches 
   the search_in_file free function.
 - Thread Management: One thread is responsible for finding files.
   A thread pool (number of threads = number of CPU cores) is used to search through files. Each thread retrieves files from fileQueue, performs the search, and pushes results into resultQueue.
   Result Output: An output thread retrieves results from resultQueue and prints them to the console.

## Improvments

 - add more features , like regex support, options like -h etc.
 - improve tests coverage:
       test on large set of files
       performance tests
       test symlinks
       namely test thread manager class by mocking functions, getting access to queues
       testing for synchronisation   

 - add proper set up and tear down to tests, i.e. make them create and remove their own ressources or have a clearly defined test asset folder
 - replace micro test framework by google test framework
 - add Cmake
 - improve exception handling, collect exceptions in threads

 ## Considerations , open questions

 - probably change to a more functional programming design with some struct containing queues and mutexes that is along free functions and embedded in lambdas 
   like submit_result, get_data_to_process. Probably, there might be benefits in testing. In current design though it is already quiet a functional one so it would be easy to change enhance free functions that are decoupled from thread managment.
 - Look into disk I/O optimisations: potentially decouple reading from files and line processing.

