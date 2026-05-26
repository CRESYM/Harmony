# This File will generate the Visual Studio Solution for Harmony.
# The path to Eigen and symengine will be configured automatically by CMake
# The path to Gurobi and Mathplot++ are supplied by the user and
# configured by by CMake

cmake_minimum_required(VERSION 4.0)
project(Harmony)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Eigen3 library on local system.
find_package(Eigen3 REQUIRED)

# Find Sundials on local system and include directories.
find_package(SUNDIALS 7.0 REQUIRED)

# Find symengine on local system and include directories.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   #symengine requires this
find_package(SymEngine CONFIG)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
include_directories(${SYMENGINE_INCLUDE_DIRS})

# 1. ImGui (needed by Implot)
include(FetchContent)
FetchContent_Declare(
    imgui_source
    URL      https://github.com/ocornut/imgui/archive/refs/tags/v1.92.3.zip
)
FetchContent_MakeAvailable(imgui_source)
set(IMGUI_BACKEND_DIR "${imgui_source_SOURCE_DIR}/backends")
include_directories(${IMGUI_BACKEND_DIR})
if (APPLE)
    message(STATUS "macOS detected: Adding ImGui backend source files to build")
    list(APPEND IMGUI_BACKEND_SOURCES 
        ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
        ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
    )
endif()

# Gurobi is configured below
# They need to be configured after add_executable(Harmony ...)

# Define an executable named 'Harmony' made up of source and header files
include(sources.cmake)
add_executable(
    Harmony
    ${HARMONY_HEADER_FILES}
    ${HARMONY_SOURCE_FILES}
    # Implot
    ${IMGUI_BACKEND_SOURCES}
)
target_precompile_headers(Harmony PRIVATE
    <Eigen/Dense>
    <symengine/basic.h>
    "./src/Constants.h"
    "./src/Include_components.h"
    "./src/Solver/Helper_Functions/Helper_Functions.h"
)

# Set Harmony as a starup project on Visual Studio Solution
# Add the MP compiler flag for parallel compilation
if (MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Harmony)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Implot (requires various libraries)
find_package(imgui REQUIRED)
find_package(implot REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW) # Optional on Mac, but good for Win/Linux

# macOS specific framework linking
if(APPLE)
    target_link_libraries(Harmony "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
endif()

# Tell the ImGui OpenGL backend to use GLEW
target_compile_definitions(Harmony PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

# If using Conda GLEW on Windows, link statically
if(MSVC)
    target_compile_definitions(Harmony PRIVATE GLEW_STATIC)
endif()

# Specify libraries that need to be linked to the harmony executable
# Although Eigen is a header-only lib, this will setup the include directories 
target_link_libraries(
    Harmony
    ${SYMENGINE_LIBRARIES}
    Eigen3::Eigen
    SUNDIALS::cvode
    SUNDIALS::kinsol
    SUNDIALS::sunmatrixdense
    SUNDIALS::sunlinsoldense
    SUNDIALS::nvecserial
    imgui::imgui
    implot::implot
    glfw
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
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
            Harmony
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
            Harmony
            "${GUROBI_CPP_LIB}"
            "${GUROBI_CORE_LIB}"
        )
    endif()


endif()


if(MSVC)
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