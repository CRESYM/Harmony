# This File will generate the Visual Studio Solution for Harmony.
# The paths to the dependencies (symengne, casadi, etc.) will be 
# automatically configured by CMake

cmake_minimum_required(VERSION 4.0)
project(Harmony)

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

# Find nlohmann_json
find_package(nlohmann_json 3.12.0 REQUIRED)

# Find casadi on local system
find_package(casadi CONFIG)

# Gurobi and Mathplot++ are configured below
# They need to be configured after add_executable(Harmony ...)

# Define HARMONY_SOURCE_FILES variable (all of Harmony's' *.cpp files except main.cpp)
file(GLOB HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../*.cpp")
# Remove main.cpp from source file list (we are testing on a class basis)
list(REMOVE_ITEM HARMONY_SOURCE_FILES "${CMAKE_SOURCE_DIR}/../main.cpp")

# Define an executable named 'Harmony' made up of source and header files
add_executable(
    Harmony
    main.cpp
    component_builder.h
    component_builder.cpp
    simulation_builder.h
    simulation_builder.cpp
    ${HARMONY_SOURCE_FILES}
)

# Set Harmony as a starup project on Visual Studio Solution
if (WIN32) # works for win64
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Harmony)
endif()

# Specify libraries that need to be linked to the harmony executable
# Note that Eigen doesn't need to be linked because it's a header-only lib
target_link_libraries(
    Harmony
    casadi::casadi
    nlohmann_json::nlohmann_json
    ${SYMENGINE_LIBRARIES}
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
        Harmony
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
    # Add Matplot++ header files
    #include_directories("${MATPLOT_PATH}/include")
    # Link the library
    target_link_libraries(Harmony Matplot++::matplot)

endif()



