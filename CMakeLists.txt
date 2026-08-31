cmake_minimum_required(VERSION 4.0)

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

# Circular static archives: cli in core needs the example registry; examples
# need certificate/element symbols from core. GNU ld does not rescan an
# archive already visited, so list core again after examples.
target_link_libraries(Harmony PRIVATE harmony_core harmony_examples harmony_core)
target_link_libraries(HarmonyUI PRIVATE harmony_core harmony_examples harmony_core)

# -----------------------------------------------------------------------------
# 5. Tests (testharmony links harmony_core + harmony_examples)
# -----------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

# -----------------------------------------------------------------------------
# 6. Optional API documentation (Doxygen)
# -----------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/doxygen.cmake)
