# -----------------------------------------------------------------------------
# Harmony External Dependencies
#
# This file handles all third-party libraries required by Harmony:
#
#   - Eigen      (linear algebra)
#   - SymEngine  (symbolic mathematics)
#   - Sundials   (numerical solvers)
#   - ImPlot     (interactive plotting)
#   - Gurobi     (optimization)
#
# Each function below connects ("links") a library to a build target
# (i.e., a program being compiled).
#
# These functions are called from CMakeLists.txt for:
#   - Harmony            (main Harmony executable)
#   - testharmony        (unit tests)
#   - testexamples       (example/test problems)
# -----------------------------------------------------------------------------


# Eigen
function(link_eigen TARGET_NAME)
    find_package(Eigen3 REQUIRED)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        Eigen3::Eigen
    )
endfunction()

# SymEngine
function(link_symengine TARGET_NAME)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)   #symengine requires this
    find_package(SymEngine CONFIG)
    set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${SYMENGINE_INCLUDE_DIRS}
    )
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        ${SYMENGINE_LIBRARIES}
    )
endfunction()

# Sundials
function(link_sundials TARGET_NAME)
    find_package(SUNDIALS 7.0 REQUIRED)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        SUNDIALS::cvode
        SUNDIALS::kinsol
        SUNDIALS::sunmatrixdense
        SUNDIALS::sunlinsoldense
        SUNDIALS::nvecserial
    )
endfunction()

# Implot
function (link_implot TARGET_NAME)
    # Find Implot (requires various other libraries)
    find_package(imgui REQUIRED)
    find_package(implot REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW) # Optional on Mac, but good for Win/Linux

    # ImGui (needed by Implot)
    include(FetchContent)
    FetchContent_Declare(
        imgui_source
        URL      https://github.com/ocornut/imgui/archive/refs/tags/v1.92.3.zip
    )
    FetchContent_MakeAvailable(imgui_source)
    
    set(IMGUI_BACKEND_DIR "${imgui_source_SOURCE_DIR}/backends")
    set(HARMONY_IMGUI_FONT_PATH "${imgui_source_SOURCE_DIR}/misc/fonts/Cousine-Regular.ttf" CACHE INTERNAL "")
    
    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${IMGUI_BACKEND_DIR}
    )
    if (APPLE)
        message(STATUS "macOS detected: Adding ImGui backend source files to build")
        list(APPEND IMGUI_BACKEND_SOURCES 
            ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
            ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp)
        target_sources(
            ${TARGET_NAME}
            PRIVATE
            ${IMGUI_BACKEND_SOURCES})
    elseif(UNIX AND NOT APPLE)
        message(STATUS "Linux detected: Adding ImGui backend source files to build")
        list(APPEND IMGUI_BACKEND_SOURCES 
            ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
            ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp)
        target_sources(
            ${TARGET_NAME}
            PRIVATE
            ${IMGUI_BACKEND_SOURCES})
    endif()

    # Tell the ImGui OpenGL backend how to load GL entry points.
    if(NOT APPLE)
        target_compile_definitions(${TARGET_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
        if(MSVC)
            target_compile_definitions(${TARGET_NAME} PRIVATE GLEW_STATIC)
        endif()
    endif()

    # Link implot and all its dependencies
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        imgui::imgui
        implot::implot
        glfw
        ${OPENGL_LIBRARIES}
    )
    if(GLEW_FOUND)
        target_link_libraries(${TARGET_NAME} PRIVATE GLEW::GLEW)
    endif()
    if(APPLE)
        target_link_libraries(${TARGET_NAME} PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreFoundation"
            "-framework OpenGL")
    endif()

    if(EXISTS "${HARMONY_IMGUI_FONT_PATH}")
        target_compile_definitions(${TARGET_NAME} PRIVATE
            "HARMONY_IMGUI_FONT_PATH=\"${HARMONY_IMGUI_FONT_PATH}\"")
    endif()

endfunction()


# nlohmann_json
function(link_nlohmann_json TARGET_NAME)
    find_package(nlohmann_json CONFIG REQUIRED)
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        nlohmann_json::nlohmann_json
    )
endfunction()


# Gurobi
set(GUROBI_PATH "" CACHE PATH "Path to Gurobi installation")
function(link_gurobi TARGET_NAME)
    if(NOT GUROBI_PATH)
        # User did not provide a path for the Gurobi installation
        message(WARNING 
            "Gurobi path not specified. The project may fail to link.\n"
            "To fix this, provide the Gurobi installation directory via CMake:\n"
            "  cmake -DGUROBI_PATH=\"/path/to/gurobi\" ..\n\n"
            "Common default paths:\n"
            "  - Windows: C:\gurobi1200\win64\n"
            "  - macOS:   /Library/gurobi1200/macos_universal2\n"
            "  - Linux:   /opt/gurobi1301/linux64")
        return()
    endif()

    if(NOT EXISTS "${GUROBI_PATH}")
        # Path was provided but is invalid
        message(FATAL_ERROR 
            " Invalid GUROBI_PATH: \"${GUROBI_PATH}\"\n"
            " The directory does not exist. Please check for typos or ensure Gurobi is correctly installed.")
        return()
    endif()


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
            ${TARGET_NAME}
            PRIVATE
            "${GUROBI_CORE_LIB}"
            $<$<CONFIG:Debug>:${GUROBI_CPP_DEBUG_LIB}>
            $<$<CONFIG:Release>:${GUROBI_CPP_RELEASE_LIB}>)
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
            ${TARGET_NAME}
            PRIVATE
            "${GUROBI_CPP_LIB}"
            "${GUROBI_CORE_LIB}")
    endif()
endfunction()
