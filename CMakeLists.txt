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

# Gurobi and Mathplot++ are configured below
# They need to be configured after add_executable(Harmony ...)

# Group header files
set(Element_Header_Files src/Elements/Element.h)
#set(Element_Admittance_Header_Files src/Admittance.h)
set(Element_Converter_Header_Files
    src/Elements/Converter/Converter.h
    src/Elements/Converter/MMC.h
    src/Elements/Converter/Simple_MMC.h
)
set(Element_Impedance_Header_Files
    src/Elements/Impedance/Impedance_base.h
    src/Elements/Impedance/Impedance.h
    src/Elements/Impedance/Resistor.h
    src/Elements/Impedance/Admittance.h
    src/Elements/Impedance/Capacitor.h
    src/Elements/Impedance/Inductor.h
)
set(Element_Load_Header_Files
    src/Elements/Load/Load_base.h
    src/Elements/Load/Load.h
    src/Elements/Load/Load_PQ.h
)
set(Element_Switch_Header_Files
    src/Elements/Switch/Switch.h
)
set(Element_Source_Header_Files 
    src/Elements/Source/Source_base.h
    src/Elements/Source/AC_source.h
    src/Elements/Source/Generator.h
    src/Elements/Source/DC_source.h
)
set(Element_Transformer_Header_Files
    src/Elements/Transformer/Transformer_base.h
    src/Elements/Transformer/Transformer_classic.h
    src/Elements/Transformer/Transformer_Delta_Delta.h
    src/Elements/Transformer/Transformer_Delta_Y.h
    src/Elements/Transformer/Transformer_Delta_Y_real.h
    src/Elements/Transformer/Transformer_real.h
    src/Elements/Transformer/Transformer_Y_Delta.h
    src/Elements/Transformer/Transformer_Y_Y.h
    src/Elements/Transformer/Transformer_Y_Y_real.h
)
set(Element_Transmission_Line_Header_Files
    src/Elements/Transmission_Line/Cable.h
    src/Elements/Transmission_Line/overhead_line.h
    src/Elements/Transmission_Line/Transmissionline.h
)
set(Element_RES_Header_Files
    src/Elements/RES/RES_base.h
    src/Elements/RES/WT_type_3.h
    src/Elements/RES/WT_type_4.h
    src/Elements/RES/WP_plant.h
    src/Elements/RES/PV_plant.h
)
set(Solver_State_Space_Model_Header_Files
    src/Solver/State_Space_Model/State_Space_Model.h
)
set(Solver_OPF_Header_Files
    src/Solver/OPF/Powerflow.h
)
set(Solver_DQsym_Header_Files
    src/Solver/DQsym/DQsym.h
)
set(Solver_Stability_Estimate_Header_Files
    src/Solver/Stability_Estimate/Stability_estimate.h
)
set(Solver_Helper_Functions_Header_Files
    src/Solver/Helper_Functions/Helper_Functions.h
    src/Solver/Helper_Functions/Standard_functions.h
    src/Solver/Helper_Functions/Differential_equations.h
    src/Solver/Helper_Functions/Symbolic_functions.h
    src/Solver/Helper_Functions/Visualization.h
    src/Solver/Helper_Functions/Writer.h
    src/Solver/Helper_Functions/stb_image_write.h
    src/Solver/Helper_Functions/DQsym_Conversion_Functions.h
)
set(Control_Header_Files
    src/Control/Control_block.h
    src/Control/Filter.h
    src/Control/Integrator.h
)
set(Controller_Header_Files
    src/Control/Controller/Controller.h
    src/Control/Controller/ProportionalIntegral_controller.h
    src/Control/Controller/Proportional_controller.h
)
set(Header_Files
    src/Bus.h
    src/Constants.h
    src/Include_components.h
    src/Include_control_blocks.h
    src/network.h
    src/SubNetwork.h
)
set(Examples_Header_files
    src/examples/Examples.h
)

# Group source files
set(Element_Source_Files src/Elements/Element.cpp)
set(Element_Converter_Source_Files
    src/Elements/Converter/Converter.cpp
    src/Elements/Converter/MMC.cpp
    src/Elements/Converter/Simple_MMC.cpp
)
set(Element_Impedance_Source_Files
    src/Elements/Impedance/Impedance_base.cpp
    src/Elements/Impedance/Impedance.cpp
    src/Elements/Impedance/Resistor.cpp
    src/Elements/Impedance/Admittance.cpp
    src/Elements/Impedance/Capacitor.cpp
    src/Elements/Impedance/Inductor.cpp
)
set(Element_Load_Source_Files
    src/Elements/Load/Load_base.cpp
    src/Elements/Load/Load.cpp
    src/Elements/Load/Load_PQ.cpp
)
set(Element_Switch_Source_Files
    src/Elements/Switch/Switch.cpp
)
set(Element_Source_Source_Files 
    src/Elements/Source/Source_base.cpp
    src/Elements/Source/AC_source.cpp
    src/Elements/Source/Generator.cpp
    src/Elements/Source/DC_source.cpp
)
set(Element_Transformer_Source_Files
    src/Elements/Transformer/Transformer_base.cpp
    src/Elements/Transformer/Transformer_classic.cpp
    src/Elements/Transformer/Transformer_Delta_Delta.cpp
    src/Elements/Transformer/Transformer_Delta_Y.cpp
    src/Elements/Transformer/Transformer_Delta_Y_real.cpp
    src/Elements/Transformer/Transformer_real.cpp
    src/Elements/Transformer/Transformer_Y_Delta.cpp
    src/Elements/Transformer/Transformer_Y_Y.cpp
    src/Elements/Transformer/Transformer_Y_Y_real.cpp
)
set(Element_Transmission_Line_Source_Files
    src/Elements/Transmission_Line/Cable.cpp
    src/Elements/Transmission_Line/overhead_line.cpp
    src/Elements/Transmission_Line/Transmissionline.cpp
)
set(Element_RES_Source_Files
    src/Elements/RES/RES_base.cpp
    src/Elements/RES/WT_type_3.cpp
    src/Elements/RES/WT_type_4.cpp
    src/Elements/RES/WP_plant.cpp
    src/Elements/RES/PV_plant.cpp
)
set(Solver_State_Space_Model_Source_Files
    src/Solver/State_Space_Model/State_Space_Model.cpp
)
set(Solver_Stability_Estimate_Source_Files
    src/Solver/Stability_Estimate/Stability_estimate.cpp
)
set(Solver_OPF_Source_Files
    src/Solver/OPF/Powerflow_data.cpp
    src/Solver/OPF/Powerflow_network_params.cpp
    src/Solver/OPF/Powerflow_solver.cpp
)
set(Solver_DQsym_Source_Files
    src/Solver/DQsym/DQsym.cpp
)
set(Solver_Helper_Functions_Source_Files
    src/Solver/Helper_Functions/Standard_functions.cpp
    src/Solver/Helper_Functions/Differential_equations.cpp
    src/Solver/Helper_Functions/Symbolic_functions.cpp
    src/Solver/Helper_Functions/Visualization.cpp
    src/Solver/Helper_Functions/Writer.cpp
    src/Solver/Helper_Functions/DQsym_Conversion_Functions.cpp
)
set(Control_Source_Files
    src/Control/Control_block.cpp
    src/Control/Filter.cpp
    src/Control/Integrator.cpp
)
set(Controller_Source_Files
    src/Control/Controller/Controller.cpp
    src/Control/Controller/ProportionalIntegral_controller.cpp
    src/Control/Controller/Proportional_controller.cpp
)
set(Source_Files
    src/Bus.cpp
    src/Constants.cpp
    src/main.cpp
    src/network.cpp
    src/SubNetwork.cpp
)
set(Examples_Source_files
    src/examples/example_OPF.cpp
    src/examples/example_OPF_csv.cpp
    src/examples/example_OPF_1.cpp
    src/examples/example_OPF_csv_1.cpp
    src/examples/example_DQsym_math_operations.cpp
    src/examples/example_generator.cpp
    src/examples/example_MMC.cpp
    src/examples/example_WT_type_3.cpp
    src/examples/example_WT_type_4.cpp
    src/examples/example_PV_plant.cpp
    src/examples/example_OHL.cpp
    src/examples/example_cable.cpp
    src/examples/example_transformer.cpp
    src/examples/example_constructors.cpp
    src/examples/example_visuals.cpp
    src/examples/example_stability_check.cpp
    src/examples/example_admittance_parameters.cpp
    src/examples/example_point2point_case.cpp
    src/examples/example_DQsym_DSSS2.cpp
    src/examples/example_DQsym_RLC.cpp
    src/examples/example_DQsym_Simple_MMC.cpp
    src/examples/example_state_space.cpp
)

# Add source_groups to create file filters (groups) in Visual Studio project
if(MSVC)
    source_group("Header Files" FILES ${Header_Files})
    source_group("Header Files\\Elements" FILES ${Element_Header_Files})
    source_group("Header Files\\Elements\\Converter" FILES ${Element_Converter_Header_Files})
    source_group("Header Files\\Elements\\Generator" FILES ${Element_Generator_Header_Files})
    source_group("Header Files\\Elements\\Impedance" FILES ${Element_Impedance_Header_Files})
    source_group("Header Files\\Elements\\Load" FILES ${Element_Load_Header_Files})
    source_group("Header Files\\Elements\\Source" FILES ${Element_Source_Header_Files})
    source_group("Header Files\\Elements\\Transformer" FILES ${Element_Transformer_Header_Files})
    source_group("Header Files\\Elements\\Transmission Line" FILES ${Element_Transmission_Line_Header_Files})
    source_group("Header Files\\Elements\\Switch" FILES ${Element_Switch_Header_Files})
    source_group("Header Files\\Elements\\RES" FILES ${Element_RES_Header_Files})
    source_group("Header Files\\Control" FILES ${Control_Header_Files})
    source_group("Header Files\\Control\\Controller" FILES ${Controller_Header_Files})
    source_group("Header Files\\Solver" FILES)
    source_group("Header Files\\Solver\\OPF" FILES ${Solver_OPF_Header_Files})
    source_group("Header Files\\Solver\\DQsym" FILES ${Solver_DQsym_Header_Files})
    source_group("Header Files\\Solver\\Stability Estimate" FILES ${Solver_Stability_Estimate_Header_Files})
    source_group("Header Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Header_Files})
    source_group("Header Files\\Solver\\Helper Functions" FILES ${Solver_Helper_Functions_Header_Files})
    source_group("Header Files\\Examples" FILES ${Examples_Header_files})


    source_group("Source Files" FILES ${Source_Files})
    source_group("Source Files\\Elements" FILES ${Element_Source_Files})
    source_group("Source Files\\Elements\\Converter" FILES ${Element_Converter_Source_Files})
    source_group("Source Files\\Elements\\Generator" FILES ${Element_Generator_Source_Files})
    source_group("Source Files\\Elements\\Impedance" FILES ${Element_Impedance_Source_Files})
    source_group("Source Files\\Elements\\Load" FILES ${Element_Load_Source_Files})
    source_group("Source Files\\Elements\\Source" FILES ${Element_Source_Source_Files})
    source_group("Source Files\\Elements\\Transformer" FILES ${Element_Transformer_Source_Files})
    source_group("Source Files\\Elements\\Transmission_Line" FILES ${Element_Transmission_Line_Source_Files})
    source_group("Source Files\\Elements\\Switch" FILES ${Element_Switch_Source_Files})
    source_group("Source Files\\Elements\\RES" FILES ${Element_RES_Source_Files})
    source_group("Source Files\\Control" FILES ${Control_Source_Files})
    source_group("Source Files\\Control\\Controller" FILES ${Controller_Source_Files})
    source_group("Source Files\\Solver" FILES)
    source_group("Source Files\\Solver\\OPF" FILES ${Solver_OPF_Source_Files})
    source_group("Source Files\\Solver\\DQsym" FILES ${Solver_DQsym_Source_Files})
    source_group("Source Files\\Solver\\Stability Estimate" FILES ${Solver_Stability_Estimate_Source_Files})
    source_group("Source Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Source_Files})
    source_group("Source Files\\Solver\\Helper Functions" FILES ${Solver_Helper_Functions_Source_Files})
    source_group("Source Files\\Examples" FILES ${Examples_Source_files})
endif()


# Define an executable named 'Harmony' made up of source and header files
add_executable(
    Harmony
    ${Header_Files}
    ${Element_Header_Files}
    ${Element_Converter_Header_Files}
    ${Element_Generator_Header_Files}
    ${Element_Impedance_Header_Files}
    ${Element_Load_Header_Files}
    ${Element_Source_Header_Files}
    ${Element_Transformer_Header_Files}
    ${Element_Transmission_Line_Header_Files}
    ${Element_Switch_Header_Files}
    ${Element_RES_Header_Files}
    ${Control_Header_Files}
    ${Controller_Header_Files}
    ${Solver_OPF_Header_Files}
    ${Solver_DQsym_Header_Files}
    ${Solver_Stability_Estimate_Header_Files}
    ${Solver_Helper_Functions_Header_Files}
    ${Solver_State_Space_Model_Header_Files}
    ${Examples_Header_files}

    ${Source_Files}
    ${Element_Source_Files}
    ${Element_Converter_Source_Files}
    ${Element_Generator_Source_Files}
    ${Element_Impedance_Source_Files}
    ${Element_Load_Source_Files}
    ${Element_Source_Source_Files}
    ${Element_Transformer_Source_Files}
    ${Element_Transmission_Line_Source_Files}
    ${Element_Switch_Source_Files}
    ${Element_RES_Source_Files}
    ${Control_Source_Files}
    ${Controller_Source_Files}
    ${Solver_OPF_Source_Files}
    ${Solver_DQsym_Source_Files}
    ${Solver_Stability_Estimate_Source_Files}
    ${Solver_Helper_Functions_Source_Files}
    ${Solver_State_Space_Model_Source_Files}
    ${Examples_Source_files}

    # Implot
    ${IMGUI_BACKEND_SOURCES}

)

# Set Harmony as a starup project on Visual Studio Solution
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