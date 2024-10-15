cmake_minimum_required(VERSION 3.14)
project(GrepProject)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)  # Use C++17 for filesystem
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module
include(FetchContent)

# Fetch GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/b514bdc.zip   # Replace with the desired commit hash
)

# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# Specify your source files
set(SOURCES
    main.cpp
    threadmanager.cpp
    grep_interface_functions.cpp
    grep_utils.cpp
)

# Specify test source files
set(TEST_SOURCES
    tests/tests.cpp
    # Include any other source files needed for testing
    grep_interface_functions.cpp
    grep_utils.cpp
    threadmanager.cpp
)

# Create the main executable
add_executable(GrepApp ${SOURCES})

# Link against pthread for threading support
target_link_libraries(GrepApp pthread)

# Include directories for your project
include_directories(
    ${CMAKE_SOURCE_DIR}  # Your project headers
)

# Add tests
enable_testing()
add_executable(GrepTests ${TEST_SOURCES})
target_link_libraries(GrepTests gtest gmock gtest_main pthread)

# Output executables to 'bin' directory
set_target_properties(GrepApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_target_properties(GrepTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
