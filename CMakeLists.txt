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
set(Element_Header_Files src/Element.h)
set(Element_Admittance_Header_Files src/Admittance.h)
set(Element_Converter_Header_Files
    src/Controller.h
    src/Filter.h
    src/MMC.h
)
set(Element_Generator_Header_Files src/Generator.h)
set(Element_Impedance_Header_Files
    src/Impedance.h
    src/Load.h
    src/Resistor.h
)
set(Element_Source_Header_Files src/AC_source.h)
set(Element_Transformer_Header_Files
    src/Transformer_base.h
    src/Transformer_classic.h
    src/Transformer_Delta_Delta.h
    src/Transformer_Delta_Y.h
    src/Transformer_real.h
    src/Transformer_Y_Delta.h
    src/Transformer_Y_Y.h
)
set(Element_Transmission_Line_Header_Files
    src/Cable.h
    src/overhead_line.h
    src/Transmissionline.h
)
set(Solver_State_Space_Model_Header_Files
    src/State_Space_Model.h
)
set(Solver_OPF_Header_Files
    src/create_ac.h
    src/create_dc.h
    src/csv_reader.h
    src/makeYbus.h
    src/params_ac.h
    src/params_dc.h
    src/solve_opf.h
    src/viz_opf.h
)
set(Header_Files
    src/Bus.h
    src/Constants.h
    src/Include_components.h
    src/network.h
)

# Group source files
set(Element_Source_Files src/Element.cpp)
set(Element_Admittance_Source_Files src/Admittance.cpp)
set(Element_Converter_Source_Files
    src/Controller.cpp
    src/Filter.cpp
    src/MMC.cpp
)
set(Element_Generator_Source_Files src/Generator.cpp)
set(Element_Impedance_Source_Files
    src/Impedance.cpp
    src/Load.cpp
    src/Resistor.cpp
)
set(Element_Source_Source_Files src/AC_source.cpp)
set(Element_Transformer_Source_Files
    src/Transformer_base.cpp
    src/Transformer_classic.cpp
    src/Transformer_Delta_Delta.cpp
    src/Transformer_Delta_Y.cpp
    src/Transformer_real.cpp
    src/Transformer_Y_Delta.cpp
    src/Transformer_Y_Y.cpp
)
set(Element_Transmission_Line_Source_Files
    src/Cable.cpp
    src/overhead_line.cpp
    src/Transmissionline.cpp
)
set(Solver_State_Space_Model_Source_Files
    src/State_Space_Model.cpp
)
set(Solver_OPF_Source_Files
    src/create_ac.cpp
    src/create_dc.cpp
    src/csv_reader.cpp
    src/makeYbus.cpp
    src/params_ac.cpp
    src/params_dc.cpp
    src/solve_opf.cpp
    src/viz_opf.cpp
)
set(Source_Files
    src/Bus.cpp
    src/Constants.cpp
    src/main.cpp
    src/network.cpp
    src/network_powerflow.cpp
    src/network_stability.cpp
)

# Add source_groups to create file filters (groups) in Visual Studio project
if(WIN32) # workds for x64 as well
    source_group("Header Files" FILES ${Header_Files})
    source_group("Header Files\\Elements" FILES ${Element_Header_Files})
    source_group("Header Files\\Elements\\Admittance" FILES ${Element_Admittance_Header_Files})
    source_group("Header Files\\Elements\\Converter" FILES ${Element_Converter_Header_Files})
    source_group("Header Files\\Elements\\Generator" FILES ${Element_Generator_Header_Files})
    source_group("Header Files\\Elements\\Impedance" FILES ${Element_Impedance_Header_Files})
    source_group("Header Files\\Elements\\Source" FILES ${Element_Source_Header_Files})
    source_group("Header Files\\Elements\\Transformer" FILES ${Element_Transformer_Header_Files})
    source_group("Header Files\\Elements\\Transmission Line" FILES ${Element_Transmission_Line_Header_Files})
    source_group("Header Files\\Solver" FILES)
    source_group("Header Files\\Solver\\OPF" FILES ${Solver_OPF_Header_Files})
    source_group("Header Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Header_Files})


    source_group("Source Files" FILES ${Source_Files})
    source_group("Source Files\\Elements" FILES ${Element_Source_Files})
    source_group("Source Files\\Elements\\Admittance" FILES ${Element_Admittance_Source_Files})
    source_group("Source Files\\Elements\\Converter" FILES ${Element_Converter_Source_Files})
    source_group("Source Files\\Elements\\Generator" FILES ${Element_Generator_Source_Files})
    source_group("Source Files\\Elements\\Impedance" FILES ${Element_Impedance_Source_Files})
    source_group("Source Files\\Elements\\Source" FILES ${Element_Source_Source_Files})
    source_group("Source Files\\Elements\\Transformer" FILES ${Element_Transformer_Source_Files})
    source_group("Source Files\\Elements\\Transmission_Line" FILES ${Element_Transmission_Line_Source_Files})
    source_group("Source Files\\Solver" FILES)
    source_group("Source Files\\Solver\\OPF" FILES ${Solver_OPF_Source_Files})
    source_group("Source Files\\Solver\\State Space Model" FILES ${Solver_State_Space_Model_Source_Files})
endif()


# Define an executable named 'Harmony' made up of source and header files
add_executable(
    Harmony
    ${Header_Files}
    ${Element_Header_Files}
    ${Element_Admittance_Header_Files}
    ${Element_Converter_Header_Files}
    ${Element_Generator_Header_Files}
    ${Element_Impedance_Header_Files}
    ${Element_Source_Header_Files}
    ${Element_Transformer_Header_Files}
    ${Element_Transmission_Line_Header_Files}
    ${Solver_OPF_Header_Files}
    ${Solver_State_Space_Model_Header_Files}

    ${Source_Files}
    ${Element_Source_Files}
    ${Element_Admittance_Source_Files}
    ${Element_Converter_Source_Files}
    ${Element_Generator_Source_Files}
    ${Element_Impedance_Source_Files}
    ${Element_Source_Source_Files}
    ${Element_Transformer_Source_Files}
    ${Element_Transmission_Line_Source_Files}
    ${Solver_OPF_Source_Files}
    ${Solver_State_Space_Model_Source_Files}

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
