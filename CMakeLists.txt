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

cmake_minimum_required(VERSION 4.0)
project(Harmony)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(sources.cmake)

# -----------------------------------------------------------------------------
# 2. Compiler cache + HarmonyCore static library
# -----------------------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ccache.cmake)

add_library(HarmonyCore STATIC
    ${HARMONY_HEADER_FILES}
    ${HARMONY_CORE_SOURCE_FILES}
)

add_library(HarmonyExamples STATIC
    ${Examples_Source_files}
)

target_precompile_headers(HarmonyCore PRIVATE
    <Eigen/Dense>
    "./src/HarmonyTypes.h"
    "./src/SymEngineTypes.h"
    "./src/Include_components.h"
    "./src/Solver/Helper_Functions/Helper_Functions.h"
)

if (MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Harmony)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies.cmake)

link_harmony_dependencies(HarmonyCore)

target_include_directories(HarmonyCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HarmonyExamples PUBLIC HarmonyCore)
link_harmony_dependencies(HarmonyExamples)

# -----------------------------------------------------------------------------
# 3. Harmony and HarmonyUI executables
# -----------------------------------------------------------------------------

add_executable(Harmony src/main.cpp)
target_link_libraries(Harmony PRIVATE HarmonyCore HarmonyExamples)

add_executable(HarmonyUI ${HARMONY_UI_EXECUTABLE_SOURCES})
target_link_libraries(HarmonyUI PRIVATE HarmonyCore HarmonyExamples)
link_harmony_dependencies(HarmonyUI)

# -----------------------------------------------------------------------------
# 4. Optional API documentation (Doxygen)
# -----------------------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/doxygen.cmake)

# ==============================================================================
# 5. Create MSI Installer on Windows (currently disabled)
# ==============================================================================
if(MSVC AND FALSE)
    get_target_property(SYMENGINE_DLL_PATH symengine LOCATION)
    get_filename_component(SYMENGINE_DLL_DIR "${SYMENGINE_DLL_PATH}" DIRECTORY)

    file(GLOB FLINT_DLL_PATH "${SYMENGINE_DLL_DIR}/flint*.dll")
    file(GLOB LIBGMP_DLL_PATH "${SYMENGINE_DLL_DIR}/libgmp*.dll")
    file(GLOB LIBMPFR_DLL_PATH "${SYMENGINE_DLL_DIR}/libmpfr*.dll")
    file(GLOB LIBWINPTHREAD_DLL_PATH "${SYMENGINE_DLL_DIR}/libwinpthread*.dll")
    file(GLOB LIBMPC_DLL_PATH "${SYMENGINE_DLL_DIR}/libmpc*.dll")
    file(GLOB LIBGCC_DLL_PATH "${SYMENGINE_DLL_DIR}/libgcc*.dll")

    file(GLOB HARMONY_USER_GUIDE ${CMAKE_SOURCE_DIR}/*.pdf)

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
