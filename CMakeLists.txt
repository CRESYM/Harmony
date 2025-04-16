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
find_package(Symengine CONFIG)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} "-std=c++0x")
include_directories(${SYMENGINE_INCLUDE_DIRS})

# Find casadi on local system
find_package(casadi CONFIG)

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
set(Source_Files
    src/Bus.cpp
    src/Constants.cpp
    src/main.cpp
    src/network.cpp
)


# Add source_groups to create file filters (groups) in Visual Studio project
source_group("Header Files" FILES ${Header_Files})
source_group("Header Files\\Elements" FILES ${Element_Header_Files})
source_group("Header Files\\Elements\\Admittance" FILES ${Element_Admittance_Header_Files})
source_group("Header Files\\Elements\\Converter" FILES ${Element_Converter_Header_Files})
source_group("Header Files\\Elements\\Generator" FILES ${Element_Generator_Header_Files})
source_group("Header Files\\Elements\\Impedance" FILES ${Element_Impedance_Header_Files})
source_group("Header Files\\Elements\\Source" FILES ${Element_Source_Header_Files})
source_group("Header Files\\Elements\\Transformer" FILES ${Element_Transformer_Header_Files})
source_group("Header Files\\Elements\\Transmission Line" FILES ${Element_Transmission_Line_Header_Files})

source_group("Source Files" FILES ${Source_Files})
source_group("Source Files\\Elements" FILES ${Element_Source_Files})
source_group("Source Files\\Elements\\Admittance" FILES ${Element_Admittance_Source_Files})
source_group("Source Files\\Elements\\Converter" FILES ${Element_Converter_Source_Files})
source_group("Source Files\\Elements\\Generator" FILES ${Element_Generator_Source_Files})
source_group("Source Files\\Elements\\Impedance" FILES ${Element_Impedance_Source_Files})
source_group("Source Files\\Elements\\Source" FILES ${Element_Source_Source_Files})
source_group("Source Files\\Elements\\Transformer" FILES ${Element_Transformer_Source_Files})
source_group("Source Files\\Elements\\Transmission_Line" FILES ${Element_Transmission_Line_Source_Files})


# Define an executable 'Harmony' made up of source and header files
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
    ${Source_Files}
    ${Element_Source_Files}
    ${Element_Admittance_Source_Files}
    ${Element_Converter_Source_Files}
    ${Element_Generator_Source_Files}
    ${Element_Impedance_Source_Files}
    ${Element_Source_Source_Files}
    ${Element_Transformer_Source_Files}
    ${Element_Transmission_Line_Source_Files}

)

# Specify libraries that need to be linked to the harmony executable
# Note that Eigen doesn't need to be linked because it's a header-only lib
target_link_libraries(
    Harmony
    casadi::casadi
    ${SYMENGINE_LIBRARIES}
)



