cmake_minimum_required(VERSION 3.14)
project(testharmony)

# GoogleTest requires at least C++ 14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch and download Google Test library
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
# Windows: Prevent overriding the parent project's compiler/linker settings
# Currently no parent CMake project, but good to keep for the future
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing in CMake
enable_testing()

# Define HARMONY_SOURCE_FILES variable (all of Harmony's' *.cpp files except main.cpp)
file(GLOB HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../Harmony/*.cpp")
list(REMOVE_ITEM HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../Harmony/main.cpp")

# Define an executable testharmony made up of various cpp files
add_executable(
    testharmony
    testimpedance.cpp
    testload.cpp
    ${HARMONY_SOURCE_FILES}
)

# Specify directories from which to import C++ header files
include_directories(
    ${CMAKE_SOURCE_DIR}/../Harmony
    ${CMAKE_SOURCE_DIR}/../../../eigen-3.4.0/eigen-3.4.0
    "D:/Softwares/vcpkg/installed/x64-windows/include"
)

# Specify directory from which to link dependencies (symengine, etc.)
target_link_directories(
    testharmony PUBLIC "D:/Softwares/vcpkg/installed/x64-windows/lib"
    testharmony PUBLIC "D:/Softwares/vcpkg/installed/x64-windows/bin"
)

# Specify libraries that need to be linked to the testharmony executable
target_link_libraries(
    testharmony
    symengine
    flint                   # needed by symengine
    GTest::gtest_main       # GoogleTest library
)

# Enable CMake’s test runner to discover the tests included in the binary,
# using the GoogleTest CMake module.
include(GoogleTest)
gtest_discover_tests(testharmony)
