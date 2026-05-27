# -----------------------------------------------------------------------------
# Harmony CMake Configuration
#
# This file configures how Harmony is built. It makes use of:
#
#   - sources.cmake
#       Defines project source (.cpp) and header files (.h)
#       Groups source files into groups
#
#   - cmake/dependencies.cmake
#       Handles external libraries and dependencies
#
# User-configurable options:
#
#   GUROBI_PATH
#       Path to the local Gurobi installation.
#
#       Example:
#       cmake -DGUROBI_PATH="/path/to/gurobi" ..
#
#       Common default paths:
#         Windows: C:/gurobi1200/win64
#         macOS:   /Library/gurobi1200/macos_universal2
#         Linux:   /opt/gurobi1301/linux64
#
# In short:
#   - Build settings are configured here
#   - Source files are listed in sources.cmake
#   - External libraries are configured in dependencies.cmake
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# 1. Project Setup
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 4.0)
project(Harmony)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 2. Define Harmony Executable
# -----------------------------------------------------------------------------

# Load the list of source and header files.
include(sources.cmake)

# Create the Harmony executable from the project source files.
add_executable(
    Harmony
    ${HARMONY_HEADER_FILES}     # defined in sources.cmake
    ${HARMONY_SOURCE_FILES}     # defined in sources.cmake
)

# -----------------------------------------------------------------------------
# 3. Pre-compiled Headers and Windows Options
# -----------------------------------------------------------------------------

# 3.1 Precompiled headers 
# Improve compilation speed by caching commonly included files that rarely change.
target_precompile_headers(Harmony PRIVATE
    <Eigen/Dense>
    <symengine/basic.h>
    "./src/Constants.h"
    "./src/Include_components.h"
    "./src/Solver/Helper_Functions/Helper_Functions.h"
)

# 3.2 Windows options
if (MSVC)
    # Set Harmony as a starup project on Visual Studio Solution
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Harmony)
    
    # Add the MP compiler flag for parallel compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# -----------------------------------------------------------------------------
# 4. Link Dependencies
# -----------------------------------------------------------------------------

# Load helper functions that configure and link third-party libraries
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

# Link required external libraries
link_eigen(Harmony)
link_symengine(Harmony)
link_sundials(Harmony)
link_implot(Harmony)
link_gurobi(Harmony)

# ==============================================================================
# 5. Create MSI Installer on Windows (currently disabled)
# ==============================================================================
if(MSVC AND FALSE)
    # Find path to symengine dll and add it as a link directory
    get_target_property(SYMENGINE_DLL_PATH symengine LOCATION)
    get_filename_component(SYMENGINE_DLL_DIR "${SYMENGINE_DLL_PATH}" DIRECTORY)
    target_link_directories(Harmony PRIVATE ${SYMENGINE_DLL_DIR})


    # Find path to dlls required by symengine, which are in the same dir as the symengine dll
    file(GLOB FLINT_DLL_PATH "${SYMENGINE_DLL_DIR}/flint*.dll")
    file(GLOB LIBGMP_DLL_PATH "${SYMENGINE_DLL_DIR}/libgmp*.dll")
    file(GLOB LIBMPFR_DLL_PATH "${SYMENGINE_DLL_DIR}/libmpfr*.dll")
    file(GLOB LIBWINPTHREAD_DLL_PATH "${SYMENGINE_DLL_DIR}/libwinpthread*.dll")
    file(GLOB LIBMPC_DLL_PATH "${SYMENGINE_DLL_DIR}/libmpc*.dll")
    file(GLOB LIBMPC_DLL_PATH "${SYMENGINE_DLL_DIR}/libmpc*.dll")
    file(GLOB LIBGCC_DLL_PATH "${SYMENGINE_DLL_DIR}/libgcc*.dll")

    # Find path to Harmony User Manual
    file(GLOB HARMONY_USER_GUIDE ${CMAKE_SOURCE_DIR}/*.pdf)

    # Print msgs to screen
    message(STATUS "Copying Harmony binary, LICENSE, and dependencies to install/Harmony")
    message(STATUS "Copying Harmony.exe to install/Harmony/bin")
    message(STATUS "Copying LICENSE to install/Harmony")
    message(STATUS "Copying Harmony user guide to install/Harmony/doc")
    message(STATUS "Copying ${SYMENGINE_DLL_PATH} to install/Harmony/bin")
    message(STATUS "Copying symengine dependencies to install/Harmony/bin")

    # Copy the files to the installation directory
    set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/install/Harmony)
    add_custom_command(TARGET Harmony POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/doc
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/build/Release/Harmony.exe ${INSTALL_DIR}/bin/Harmony.exe
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${INSTALL_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${SYMENGINE_DLL_PATH} ${INSTALL_DIR}/bin/
        COMMAND ${CMAKE_COMMAND} -E copy ${FLINT_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGMP_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBMPFR_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBWINPTHREAD_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBMPC_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${LIBGCC_DLL_PATH} ${INSTALL_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${HARMONY_USER_GUIDE} ${INSTALL_DIR}/doc
    )
endif()