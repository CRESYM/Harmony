cmake_minimum_required(VERSION 4.0)
project(Harmony)

# GoogleTest requires at least C++ 14
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


# #######
# Find the Conda environment path
# execute_process(
#     COMMAND conda info --base
#     OUTPUT_VARIABLE CONDA_BASE_PATH
#     OUTPUT_STRIP_TRAILING_WHITESPACE
# )
# set(CONDA_ENV_PATH "${CONDA_BASE_PATH}/envs/harmony")
# message(STATUS ${CONDA_ENV_PATH})

# Add the include directories and library paths for CasADi
# include_directories(${CONDA_ENV_PATH}/include)
# include_directories(${CONDA_ENV_PATH}/include/eigen3)
# link_directories(${CONDA_ENV_PATH}/lib)
# ########



# Define an executable 'Harmony' made up of various cpp files
add_executable(
    Harmony
    src/AC_source.cpp
    src/Admittance.cpp
    src/Bus.cpp
    src/Cable.cpp
    src/Constants.cpp
    src/Controller.cpp
    src/Element.cpp
    src/Filter.cpp
    src/Generator.cpp
    src/Impedance.cpp
    src/Load.cpp
    src/main.cpp
    src/MMC.cpp
    src/network.cpp
    src/overhead_line.cpp
    src/Transformer_base.cpp
    src/Transformer_classic.cpp
    src/Transformer_Delta_Delta.cpp
    src/Transformer_Delta_Y.cpp
    src/Transformer_real.cpp
    src/Transformer_Y_Delta.cpp
    src/Transformer_Y_Y.cpp
    src/Transmissionline.cpp
)

# Header files 
set(HEADER_FILES_FOR_INSTALLER
    src/AC_source.h
    src/Admittance.h
    src/Bus.h
    src/Cable.h
    src/constants.h
    src/Controller.h
    src/Element.h
    src/Filter.h
    src/Generator.h
    src/Impedance.h
    src/Include_components.h
    src/Load.h
    src/MMC.h
    src/network.h
    src/overhead_line.h
    src/symengine_n.h
    src/Transformer_base.h
    src/Transformer_classic.h
    src/Transformer_Delta_Delta.h
    src/Transformer_Delta_Y.h
    src/Transformer_real.h
    src/Transformer_Y_Delta.h
    src/Transformer_Y_Y.h
    src/Transmissionline.h
)

# Specify libraries that need to be linked to the harmony executable
target_link_libraries(
    Harmony
    casadi
    ${SYMENGINE_LIBRARIES}
)



message(STATUS "*** casadi ${casadi_IMPORT_PREFIX}")
include(CMakePrintHelpers)
#cmake_print_variables(casadi_LIBRARY)
cmake_print_variables(SYMENGINE_CXX_FLAGS_RELEASE)
cmake_print_variables(SYMENGINE_LIBRARIES)
cmake_print_variables(SYMENGINE_INCLUDE_DIRS)

# # Specify directories from which to import C++ header files
# include_directories(
#     ${CMAKE_SOURCE_DIR}/../Harmony
#     ${CMAKE_SOURCE_DIR}/../../../eigen-3.4.0/eigen-3.4.0
#     "D:/Softwares/vcpkg/installed/x64-windows/include"
# )

# # Specify directory from which to link dependencies (symengine, etc.)
# target_link_directories(
#     testharmony PUBLIC "D:/Softwares/vcpkg/installed/x64-windows/lib"
#     testharmony PUBLIC "D:/Softwares/vcpkg/installed/x64-windows/bin"
# )

# # Specify libraries that need to be linked to the testharmony executable
# target_link_libraries(
#     testharmony
#     symengine
#     flint                   # needed by symengine
#     GTest::gtest_main       # GoogleTest library
# )




install(TARGETS Harmony DESTINATION bin)
install(FILES ${HEADER_FILES_FOR_INSTALLER} DESTINATION include)


