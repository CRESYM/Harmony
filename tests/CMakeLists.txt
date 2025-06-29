cmake_minimum_required(VERSION 4.0)
project(testharmony)

# CMake module needed to download Google Test
include(FetchContent)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eigen3 library on local system and include directories.
find_package(Eigen3 REQUIRED)
include_directories("${EIGEN3_INCLUDE_DIR}")

# Find symengine on local system and include directories.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   #symengine requires this
find_package(SymEngine CONFIG)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
include_directories(${SYMENGINE_INCLUDE_DIRS})

# Find casadi on local system
find_package(casadi CONFIG)

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
file(GLOB HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../src/*.cpp")
# Remove main.cpp from source file list (we are testing on a class basis)
list(REMOVE_ITEM HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../src/main.cpp")

# Specify directories from which to import Harmony header files
include_directories(${CMAKE_SOURCE_DIR}/../src)

# Define an executable testharmony made up of various cpp files
add_executable(
    testharmony
    testSimpleNetwork.cpp
    teststatespacemodel.cpp
    testelement.cpp
    testimpedance.cpp
    testadmittance.cpp
    testload.cpp
    utils.cpp
    ${HARMONY_SOURCE_FILES}
)

# Specify libraries that need to be linked to the testharmony executable
# Note that Eigen doesn't need to be linked because it's a header-only lib
target_link_libraries(
    testharmony
    casadi::casadi
    ${SYMENGINE_LIBRARIES}
    GTest::gtest_main       # GoogleTest library
)

# Configure Gurobi
set(GUROBI_PATH "" CACHE PATH "Path to Gurobi installation")
if(NOT GUROBI_PATH)
    # User did not provide a path for the Gurobi installation
    message(WARNING "Path to Gurobi was not specified. You will need to configure it manually in the Visual Studio Project Properties")
elseif(NOT EXISTS "${GUROBI_PATH}")
    message(FATAL_ERROR "The specified GUROBI_PATH does not exist: ${GUROBI_PATH}")
else()
    message(STATUS "The specified GUROBI_PATH is: ${GUROBI_PATH}")

    # Add Gurobi header files
    include_directories("${GUROBI_PATH}/include")
    
    # Find gurobi*.lib that is NOT the C++ wrapper
    file(GLOB GUROBI_CORE_LIB "${GUROBI_PATH}/lib/gurobi[0-9]*.lib")

    # Based on build config, choose the C++ wrapper lib pattern
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
endif()


# Set Matplot++ path 
set(MATPLOT_PATH "" CACHE PATH "Path to MatPlot++ installation")
if(NOT MATPLOT_PATH)
    # User did not provide a path for the Matplot installation
    message(WARNING "Path to Matplot++ was not specified. You will need to configure it manually in the Visual Studio Project Properties")
elseif(NOT EXISTS "${MATPLOT_PATH}")
    message(FATAL_ERROR "The specified MATPLOT_PATH does not exist: ${MATPLOT_PATH}")
else()
    message(STATUS "The specified MATPLOT_PATH is: ${MATPLOT_PATH}")

    # Matplot supports CMake. We will use Matplot++Config.cmake contained in MATPLOT_PATH to configure the package
    list(APPEND CMAKE_PREFIX_PATH "${MATPLOT_PATH}")
    find_package(Matplot++ REQUIRED)
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
