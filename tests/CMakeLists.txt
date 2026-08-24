# -----------------------------------------------------------------------------
# 1. Project Setup
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 4.0)
project(testharmony)

# Set the C++ Standard (GoogleTest requires >= 14)
# std::variant included in Constants.h requires >= 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing in CMake
enable_testing()

# -----------------------------------------------------------------------------
# 2. Define testharmony Executable
# -----------------------------------------------------------------------------

# Load the list of harmony source and header files.
include(${CMAKE_CURRENT_SOURCE_DIR}/../sources.cmake)

# Remove examples and main.cpp since they are not needed by the tests
list(REMOVE_ITEM HARMONY_SOURCE_FILES "src/main.cpp")

# Prepend the relative path ("../") to harmony files
set(CORRECTED_HARMONY_SOURCES "")
set(CORRECTED_HARMONY_HEADERS "")
foreach(source_file IN LISTS HARMONY_SOURCE_FILES)
    list(APPEND CORRECTED_HARMONY_SOURCES "../${source_file}")
endforeach()
foreach(header_file IN LISTS HARMONY_HEADER_FILES)
    list(APPEND CORRECTED_HARMONY_HEADERS "../${header_file}")
endforeach()

# Create the Harmony executable from the project source files.
add_executable(
    testharmony
    testSimpleNetwork.cpp
    teststatespacemodel.cpp
    testABCDforming.cpp
    testelement.cpp
    testimpedance.cpp
    testadmittance.cpp
    testload.cpp
    testcapacitor.cpp
    testresistor.cpp
    testinductor.cpp
    testswitch.cpp
    testrealtransformer.cpp
    testtransformerclassic.cpp
    testMMC.cpp
    testStabilityEstimate.cpp
    testCertificate.cpp
    testOPF.cpp
    testJsonPipeline.cpp
    testexamples.cpp
    utils.cpp
    ${CORRECTED_HARMONY_HEADERS}     # defined in sources.cmake
    ${CORRECTED_HARMONY_SOURCES}     # defined in sources.cmake
)

# -----------------------------------------------------------------------------
# 3. Compiler and Project Options
# -----------------------------------------------------------------------------

# Add all header directories to the include path
set(HARMONY_HEADER_DIRS "")
foreach(header ${CORRECTED_HARMONY_HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND HARMONY_HEADER_DIRS ${dir})
endforeach()
list(REMOVE_DUPLICATES HARMONY_HEADER_DIRS)
include_directories(${HARMONY_HEADER_DIRS})

# Add the MP compiler flag for parallel compilation.
# /EHa enables catching of Windows structured exceptions (SEH) via catch(...),
# which prevents GFM/certificate tests from crashing on KINSOL failures.
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHa")
endif()

# Specify the output directory where the test executable will be placed
set_target_properties(testharmony PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

# Create the 'files' directory needed by the Element::WriteValues function
# The function should be modified to avoid hardcoding the output directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/files")

# -----------------------------------------------------------------------------
# 4. Link Dependencies
# -----------------------------------------------------------------------------

# Load helper functions that configure and link third-party libraries
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/dependencies.cmake)

# Link required external libraries
link_eigen(testharmony)
link_symengine(testharmony)
link_sundials(testharmony)
link_implot(testharmony)
link_gurobi(testharmony)

# Google Test library
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
# Windows: Prevent overriding the parent project's compiler/linker settings
# Currently no parent CMake project, but good to keep for the future
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
target_link_libraries(
    testharmony
    PRIVATE
    GTest::gtest_main
)

# -----------------------------------------------------------------------------
# 5. Register tests with CTest
# -----------------------------------------------------------------------------
include(GoogleTest)

# Windows CI: do not use gtest_discover_tests. Conda CMake 4.4 runs the exe
# through DiscoverTests.cmake and aborts with 0xc0000409 / stack smashing
# before any test starts. Copy runtime DLLs beside the exe (Windows loads
# those first) and register the two CTest names used by CI filters
# (-E TestExamples / -R TestExamples).
if(WIN32)
    set(_harmony_dll_bin_dirs "")
    if(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/Library/bin")
        list(APPEND _harmony_dll_bin_dirs "$ENV{CONDA_PREFIX}/Library/bin")
    endif()
    if(Eigen3_DIR)
        get_filename_component(_eigen_library "${Eigen3_DIR}/../../.." ABSOLUTE)
        if(EXISTS "${_eigen_library}/bin")
            list(APPEND _harmony_dll_bin_dirs "${_eigen_library}/bin")
        endif()
    endif()
    if(GUROBI_PATH AND EXISTS "${GUROBI_PATH}/bin")
        list(APPEND _harmony_dll_bin_dirs "${GUROBI_PATH}/bin")
    endif()
    list(REMOVE_DUPLICATES _harmony_dll_bin_dirs)

    set(_harmony_dll_patterns
        "symengine*.dll"
        "sundials*.dll"
        "glfw*.dll"
        "libgmp*.dll" "gmp*.dll" "mpir*.dll"
        "libmpfr*.dll" "mpfr*.dll"
        "flint*.dll"
        "libmpc*.dll" "mpc*.dll"
        "libwinpthread*.dll" "libgcc*.dll" "libstdc++*.dll" "libssp*.dll"
        "zlib*.dll" "libz*.dll" "zstd*.dll" "libpng*.dll" "png*.dll"
        "imgui*.dll" "implot*.dll"
        "gurobi*.dll"
    )
    set(_harmony_runtime_dlls "")
    foreach(_dir IN LISTS _harmony_dll_bin_dirs)
        foreach(_pat IN LISTS _harmony_dll_patterns)
            file(GLOB _found "${_dir}/${_pat}")
            list(APPEND _harmony_runtime_dlls ${_found})
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES _harmony_runtime_dlls)

    if(_harmony_runtime_dlls)
        set(_copy_dlls_script "${CMAKE_CURRENT_BINARY_DIR}/copy_testharmony_dlls.cmake")
        set(_script_body "set(_dlls \"\")\n")
        foreach(_dll IN LISTS _harmony_runtime_dlls)
            string(REPLACE "\\" "/" _dll_fwd "${_dll}")
            string(APPEND _script_body "list(APPEND _dlls \"${_dll_fwd}\")\n")
        endforeach()
        string(APPEND _script_body [[
if(NOT DEST)
  message(FATAL_ERROR "DEST not set")
endif()
foreach(_dll IN LISTS _dlls)
  if(EXISTS "${_dll}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_dll}" "${DEST}")
  endif()
endforeach()
]])
        file(WRITE "${_copy_dlls_script}" "${_script_body}")
        add_custom_command(
            TARGET testharmony POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DDEST=$<TARGET_FILE_DIR:testharmony>
                -P "${_copy_dlls_script}"
            COMMENT "Copy conda/Gurobi DLLs next to testharmony.exe"
        )
    endif()

    add_custom_command(
        TARGET testharmony POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
            $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:testharmony>>,copy,true>
            $<TARGET_RUNTIME_DLLS:testharmony>
            $<$<BOOL:$<TARGET_RUNTIME_DLLS:testharmony>>:$<TARGET_FILE_DIR:testharmony>>
        COMMAND_EXPAND_LISTS
        COMMENT "Copy imported runtime DLLs next to testharmony.exe"
    )

    set(_harmony_test_launcher ${CMAKE_COMMAND} -E env)
    foreach(_dir IN LISTS _harmony_dll_bin_dirs)
        list(APPEND _harmony_test_launcher --modify "PATH=path_list_prepend:${_dir}")
    endforeach()
    list(APPEND _harmony_test_launcher --)
    set_target_properties(testharmony PROPERTIES TEST_LAUNCHER "${_harmony_test_launcher}")

    # Names match CI: ctest -E TestExamples / ctest -R TestExamples
    add_test(NAME testharmony COMMAND testharmony --gtest_filter=-TestExamples.*)
    add_test(NAME TestExamples COMMAND testharmony --gtest_filter=TestExamples.*)
    set(_harmony_path_mods "")
    foreach(_dir IN LISTS _harmony_dll_bin_dirs)
        list(APPEND _harmony_path_mods "PATH=path_list_prepend:${_dir}")
    endforeach()
    set_tests_properties(testharmony TestExamples PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    if(_harmony_path_mods)
        set_tests_properties(testharmony TestExamples PROPERTIES
            ENVIRONMENT_MODIFICATION "${_harmony_path_mods}"
        )
    endif()
else()
    gtest_discover_tests(testharmony
        DISCOVERY_MODE PRE_TEST
        DISCOVERY_TIMEOUT 60
    )
endif()
