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

# Find casadi on local system
find_package(casadi CONFIG)

# Gurobi and Mathplot++ are configured below
# They need to be configured after add_executable(Harmony ...)

# Group header files
set(Element_Header_Files src/Elements/Element.h)
#set(Element_Admittance_Header_Files src/Admittance.h)
set(Element_Converter_Header_Files
    src/Elements/Converter/MMC.h
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
    src/Solver/OPF/viz_opf.h
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
    src/Solver/Helper_Functions/Differential_equations.h
    src/Solver/Helper_Functions/Symbolic_functions.h
    src/Solver/Helper_Functions/Visualization.h
)
set(Control_Header_Files
    src/Control/Control_block.h
    src/Control/Filter.h
    src/Control/Controller.h
    src/Control/Integrator.h
)
set(Header_Files
    src/Bus.h
    src/Constants.h
    src/Include_components.h
    src/Include_control_blocks.h
    src/network.h
)
set(Examples_Header_files
    src/examples/Examples.h
)

# Group source files
set(Element_Source_Files src/Elements/Element.cpp)
set(Element_Converter_Source_Files
    src/Elements/Converter/MMC.cpp
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
    src/Solver/OPF/viz_opf.cpp
    src/Solver/OPF/Powerflow_data.cpp
    src/Solver/OPF/Powerflow_network_params.cpp
    src/Solver/OPF/Powerflow_solver.cpp
)
set(Solver_DQsym_Source_Files
    src/Solver/DQsym/DQsym.cpp
)
set(Solver_Helper_Functions_Source_Files
    src/Solver/Helper_Functions/Differential_equations.cpp
    src/Solver/Helper_Functions/Symbolic_functions.cpp
    src/Solver/Helper_Functions/Visualization.cpp
)
set(Control_Source_Files
    src/Control/Control_block.cpp
    src/Control/Filter.cpp
    src/Control/Controller.cpp
    src/Control/Integrator.cpp
)
set(Source_Files
    src/Bus.cpp
    src/Constants.cpp
    src/main.cpp
    src/network.cpp
)
set(Examples_Source_files
    src/examples/example_OPF.cpp
    src/examples/example_OPF_csv.cpp
    src/examples/example_DQsym_math_operations.cpp
    src/examples/example_MMC.cpp
    src/examples/example_WT_type_3.cpp
    src/examples/example_WT_type_4.cpp
    src/examples/example_OHL.cpp
    src/examples/example_constructors.cpp
    src/examples/example_visuals.cpp
)

# Add source_groups to create file filters (groups) in Visual Studio project
if(WIN32) # workds for x64 as well
    source_group("Header Files" FILES ${Header_Files})
    source_group("Header Files\\Elements" FILES ${Element_Header_Files})
    #source_group("Header Files\\Elements\\Admittance" FILES ${Element_Admittance_Header_Files})
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
    ${Solver_OPF_Source_Files}
    ${Solver_DQsym_Source_Files}
    ${Solver_Stability_Estimate_Source_Files}
    ${Solver_Helper_Functions_Source_Files}
    ${Solver_State_Space_Model_Source_Files}
    ${Examples_Source_files}

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
