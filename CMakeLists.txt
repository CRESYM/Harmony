# -----------------------------------------------------------------------------
# Harmony CMake Configuration
#
# This file configures how Harmony is built. It makes use of:
#
#   - cmake/sources.cmake
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
# 2. Shared core library + executables
# -----------------------------------------------------------------------------

# Load the list of source and header files.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sources.cmake)

add_library(
    harmony_core
    STATIC
    ${HARMONY_HEADER_FILES}
    ${HARMONY_CORE_SOURCE_FILES}
)

add_executable(Harmony src/main.cpp)

add_executable(
    HarmonyUI
    ${HARMONY_UI_SOURCE_FILES}
    ${Ui_Header_Files}
)

# C++ examples live outside harmony_core so the library does not rebuild when
# a demonstration case changes. Harmony, HarmonyUI, and testharmony link this.
add_library(
    harmony_examples
    STATIC
    ${Examples_Header_files}
    ${Examples_Source_files}
)

# -----------------------------------------------------------------------------
# 3. Pre-compiled Headers and Windows Options
# -----------------------------------------------------------------------------

# Slim PCH: Eigen + SymEngine + Constants only. Include_components.h and
# Helper_Functions.h change too often and pull too much to be worth caching.
target_precompile_headers(harmony_core PRIVATE
    <Eigen/Dense>
    <symengine/basic.h>
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Constants.h"
)
target_precompile_headers(Harmony REUSE_FROM harmony_core)
target_precompile_headers(HarmonyUI REUSE_FROM harmony_core)
target_precompile_headers(harmony_examples REUSE_FROM harmony_core)

if (MSVC)
    # Set Harmony as a startup project on Visual Studio Solution
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Harmony)

    # Add the MP compiler flag for parallel compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# -----------------------------------------------------------------------------
# 4. Link Dependencies
# -----------------------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

# Single include root: quoted includes are src-relative
# (e.g. "core/Constants.h", "network/network.h", "Elements/Impedance/Resistor.h").
set(HARMONY_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

foreach(HARMONY_TARGET IN ITEMS harmony_core harmony_examples Harmony HarmonyUI)
    link_eigen(${HARMONY_TARGET})
    link_symengine(${HARMONY_TARGET})
    link_sundials(${HARMONY_TARGET})
    link_implot(${HARMONY_TARGET})
    link_gurobi(${HARMONY_TARGET})
    link_nlohmann_json(${HARMONY_TARGET})
    target_include_directories(${HARMONY_TARGET} PRIVATE ${HARMONY_INCLUDE_DIRS})
endforeach()

# MSVC static linking is left-to-right: core first (cli.obj needs the registry),
# then examples (defines harmonyCppExampleRegistry and the example_* symbols).
target_link_libraries(Harmony PRIVATE harmony_core harmony_examples)
target_link_libraries(HarmonyUI PRIVATE harmony_core harmony_examples)

# -----------------------------------------------------------------------------
# 5. Tests (testharmony links harmony_core + harmony_examples)
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

# -----------------------------------------------------------------------------
# 6. Optional API documentation (Doxygen)
# -----------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/doxygen.cmake)

# ==============================================================================
# 7. Create MSI Installer on Windows (currently disabled)
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
