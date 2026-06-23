# -----------------------------------------------------------------------------
# 1. Project Setup
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0)
project(testharmony)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing in CMake
enable_testing()

# -----------------------------------------------------------------------------
# 2. Define testharmony Executable
# -----------------------------------------------------------------------------

# Load the list of harmony source and header files.
include(${CMAKE_CURRENT_SOURCE_DIR}/../sources.cmake)

# Remove examples and main.cpp since they are not needed by the tests
list(REMOVE_ITEM HARMONY_SOURCE_FILES "src/main.cpp")

# Prepend the relative path ("../") to harmony files
set(CORRECTED_HARMONY_SOURCES "")
set(CORRECTED_HARMONY_HEADERS "")
foreach(source_file IN LISTS HARMONY_SOURCE_FILES)
    list(APPEND CORRECTED_HARMONY_SOURCES "../${source_file}")
endforeach()
foreach(header_file IN LISTS HARMONY_HEADER_FILES)
    list(APPEND CORRECTED_HARMONY_HEADERS "../${header_file}")
endforeach()

# Create the Harmony executable from the project source files.
add_executable(
    testharmony
    testSimpleNetwork.cpp
    teststatespacemodel.cpp
    testABCDforming.cpp
    testelement.cpp
    testimpedance.cpp
    testadmittance.cpp
    testload.cpp
    testcapacitor.cpp
    testresistor.cpp
    testinductor.cpp
    testswitch.cpp
    testrealtransformer.cpp
    testtransformerclassic.cpp
    testMMC.cpp
    testStabilityEstimate.cpp
    testOPF.cpp
    testJsonPipeline.cpp
    testexamples.cpp
    utils.cpp
    ${CORRECTED_HARMONY_HEADERS}     # defined in sources.cmake
    ${CORRECTED_HARMONY_SOURCES}     # defined in sources.cmake
)

# -----------------------------------------------------------------------------
# 3. Compiler and Project Options
# -----------------------------------------------------------------------------

# Add all header directories to the include path
set(HARMONY_HEADER_DIRS "")
foreach(header ${CORRECTED_HARMONY_HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND HARMONY_HEADER_DIRS ${dir})
endforeach()
list(REMOVE_DUPLICATES HARMONY_HEADER_DIRS)
include_directories(${HARMONY_HEADER_DIRS})

# Add the MP compiler flag for parallel compilation
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Specify the output directory where the test executable will be placed
set_target_properties(testharmony PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

# Create the 'files' directory needed by the Element::WriteValues function
# The function should be modified to avoid hardcoding the output directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/files")

# -----------------------------------------------------------------------------
# 4. Link Dependencies
# -----------------------------------------------------------------------------

# Load helper functions that configure and link third-party libraries
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/dependencies.cmake)

# Link required external libraries
link_eigen(testharmony)
link_symengine(testharmony)
link_sundials(testharmony)
link_implot(testharmony)
link_gurobi(testharmony)

# Google Test library
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
# Windows: Prevent overriding the parent project's compiler/linker settings
# Currently no parent CMake project, but good to keep for the future
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
target_link_libraries(
    testharmony
    PRIVATE
    GTest::gtest_main
)
# Enable CMake’s test runner to discover the tests included in the binary,
# using the GoogleTest CMake module.
include(GoogleTest)
gtest_discover_tests(testharmony)
