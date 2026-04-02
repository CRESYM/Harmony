cmake_minimum_required(VERSION 4.0)
project(testharmony)

# CMake module needed to download Google Test
include(FetchContent)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eigen3 library on local system
find_package(Eigen3 REQUIRED)

# Find symengine on local system and include directories.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   #symengine requires this
find_package(SymEngine CONFIG)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
include_directories(${SYMENGINE_INCLUDE_DIRS})

# Gurobi and Mathplot++ are configured below
# They need to be configured after add_executable(Harmony ...)

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
file(GLOB_RECURSE HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../src/*.cpp")
list(REMOVE_ITEM HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../src/main.cpp")

# Specify directories from which to import Harmony header files
# Recursively find all directories containing header files in ../src
file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/../src/*.h" "${CMAKE_SOURCE_DIR}/../src/*.hpp")

# Extract unique directories from the header files
set(HEADER_DIRS "")
foreach(header ${HEADER_FILES})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND HEADER_DIRS ${dir})
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS)

# Add all header directories to the include path
include_directories(${HEADER_DIRS})

# Define an executable testharmony made up of various cpp files
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
    utils.cpp
    ${HARMONY_SOURCE_FILES}
)

# Specify libraries that need to be linked to the testharmony executable
# Note that Eigen doesn't need to be linked because it's a header-only lib
target_link_libraries(
    testharmony
    ${SYMENGINE_LIBRARIES}
    Eigen3::Eigen
    GTest::gtest_main       # GoogleTest library
)

# Configure Gurobi
set(GUROBI_PATH "" CACHE PATH "Path to Gurobi installation")
if(NOT GUROBI_PATH)
    # User did not provide a path for the Gurobi installation
    message(WARNING 
        "Gurobi path not specified. The project may fail to link.\n"
        "To fix this, provide the Gurobi installation directory via CMake:\n"
        "  cmake -DGUROBI_PATH=\"/path/to/gurobi\" ..\n\n"
        "Common default paths:\n"
        "  - Windows: C:\gurobi1200\win64\n"
        "  - macOS:   /Library/gurobi1200/macos_universal2\n"
        "  - Linux:   /opt/gurobi1301/linux64"
    )
elseif(NOT EXISTS "${GUROBI_PATH}")
    # Path was provided but is invalid
    message(FATAL_ERROR 
        " Invalid GUROBI_PATH: \"${GUROBI_PATH}\"\n"
        " The directory does not exist. Please check for typos or ensure Gurobi is correctly installed."
    )
else()
    message(STATUS "Found Gurobi in: ${GUROBI_PATH}")

    # Add Gurobi header files
    include_directories("${GUROBI_PATH}/include")
    
    if(MSVC)
        # 1. Find gurobi*.lib that is NOT the C++ wrapper
        file(GLOB GUROBI_CORE_LIB "${GUROBI_PATH}/lib/gurobi[0-9]*.lib")

        # 2. Based on build config, choose the C++ wrapper lib pattern
        file(GLOB GUROBI_CPP_DEBUG_LIB "${GUROBI_PATH}/lib/gurobi_c++mdd*.lib")
        file(GLOB GUROBI_CPP_RELEASE_LIB "${GUROBI_PATH}/lib/gurobi_c++md[0-9]*.lib")

        # Fail if either is missing
        if(NOT GUROBI_CORE_LIB)
            message(FATAL_ERROR "Could not find Gurobi core library (gurobi*.lib) in ${GUROBI_PATH}/lib")
        endif()

        if(NOT GUROBI_CPP_DEBUG_LIB OR NOT GUROBI_CPP_RELEASE_LIB)
            message(FATAL_ERROR "Could not find Gurobi C++ wrapper library (gurobi_c++md*.lib or gurobi_c++mdd*.lib) in ${GUROBI_PATH}/lib")
        endif()

        message(STATUS "Using Gurobi core lib: ${GUROBI_CORE_LIB}")
        message(STATUS "Using Gurobi C++ Debug lib: ${GUROBI_CPP_DEBUG_LIB}")
        message(STATUS "Using Gurobi C++ Release lib: ${GUROBI_CPP_RELEASE_LIB}")

        # Link the libraries
        target_link_libraries(
            testharmony
            "${GUROBI_CORE_LIB}"
            $<$<CONFIG:Debug>:${GUROBI_CPP_DEBUG_LIB}>
            $<$<CONFIG:Release>:${GUROBI_CPP_RELEASE_LIB}>
        )
    else()
        # 1. Find the Core Library (Dynamic)
        if (APPLE)
            # On Mac this is libgurobi[ver].dylib
            file(GLOB GUROBI_CORE_LIB "${GUROBI_PATH}/lib/libgurobi[0-9]*.dylib")
        else()
            # On Linux this is libgurobi[ver].so
            file(GLOB GUROBI_CORE_LIB "${GUROBI_PATH}/lib/libgurobi[0-9]*.so")
        endif()

        # 2. Find the C++ Wrapper (Static)
        # Usually just libgurobi_c++.a. Unlike Windows, there isn't usually a separate debug version.
        set(GUROBI_CPP_LIB "${GUROBI_PATH}/lib/libgurobi_c++.a")

        # Fail if either is missing
        if(NOT GUROBI_CORE_LIB)
            if (APPLE)
                message(FATAL_ERROR "Could not find Gurobi core library (libgurobi*.dylib) in ${GUROBI_PATH}/lib")
            else()
                message(FATAL_ERROR "Could not find Gurobi core library (libgurobi*.so) in ${GUROBI_PATH}/lib")
            endif()
        endif()

        if(NOT EXISTS "${GUROBI_CPP_LIB}")
            message(FATAL_ERROR "Could not find Gurobi C++ wrapper (libgurobi_c++.a) in ${GUROBI_PATH}/lib")
        endif()

        message(STATUS "Using Gurobi core lib: ${GUROBI_CORE_LIB}")
        message(STATUS "Using Gurobi C++ lib: ${GUROBI_CPP_LIB}")

        # Link the libraries
        target_link_libraries(
            testharmony
            "${GUROBI_CPP_LIB}"
            "${GUROBI_CORE_LIB}"
        )
    endif()


endif()


# Set Matplot++ path 
set(MATPLOT_PATH "" CACHE PATH "Path to Matplot++ installation")
if(NOT MATPLOT_PATH)
    # User did not provide a path for the Matplot installation
    message(WARNING 
        "Matplot++ path not specified. The project may fail to link.\n"
        "To fix this, provide the Matplot++ installation directory via CMake:\n"
        "  cmake -DMATPLOT_PATH=\"/path/to/matplotplusplus\" ..\n\n"
        "Common installation hints:\n"
        "  - Windows (msi): C:\Program Files\Matplot++\n"
        "  - macOS (brew):    /opt/homebrew/opt/matplotplusplus\n"
        "  - Linux (manual):  /usr/local/include/matplot"
    )
elseif(NOT EXISTS "${MATPLOT_PATH}")
    # Path was provided but is invalid
    message(FATAL_ERROR 
        " Invalid MATPLOT_PATH: \"${MATPLOT_PATH}\"\n"
        " The directory does not exist. Please check for typos or ensure Matplot++ is correctly installed."
    )
else()
    message(STATUS "Found Matplot++ in: ${MATPLOT_PATH}")

    # Matplot supports CMake. We will use Matplot++Config.cmake contained in MATPLOT_PATH to configure the package
    list(APPEND CMAKE_PREFIX_PATH "${MATPLOT_PATH}")
    find_package(Matplot++ REQUIRED)
    # Add Matplot++ header files (not needed due to cmake)
    #include_directories("${MATPLOT_PATH}/include")
    # Link the library
    target_link_libraries(testharmony Matplot++::matplot)
endif()



# Specify the output directory where the test executable will be placed
set_target_properties(testharmony PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)

# Create the 'files' directory needed by the Element::WriteValues function
# The function should be modified to avoid hardcoding the output directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/files")

# Enable CMake’s test runner to discover the tests included in the binary,
# using the GoogleTest CMake module.
include(GoogleTest)
gtest_discover_tests(testharmony)
