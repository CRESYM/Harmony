# -----------------------------------------------------------------------------
# testharmony — GoogleTest suite linked against harmony_core and harmony_examples.
#
# Configure from the repository root:
#   cmake -S . -B build -DGUROBI_PATH=...
#   cmake --build build --config Release --target testharmony
#   ctest --test-dir build -C Release
# -----------------------------------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR
        "testharmony is built from the repository root (it links harmony_core).\n"
        "  cmake -S . -B build -DGUROBI_PATH=\"<gurobi>\"\n"
        "  cmake --build build --config Release --target testharmony\n"
        "  ctest --test-dir build -C Release")
endif()

set(TESTHARMONY_SOURCES
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
)

add_executable(testharmony ${TESTHARMONY_SOURCES})

target_include_directories(testharmony PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# /EHa: catch Windows SEH (KINSOL failures) via catch(...) in GFM/certificate tests.
if(MSVC)
    target_compile_options(testharmony PRIVATE /EHa)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/files")

link_eigen(testharmony)
link_symengine(testharmony)
link_sundials(testharmony)
link_implot(testharmony)
link_gurobi(testharmony)
link_nlohmann_json(testharmony)

target_link_libraries(testharmony PRIVATE harmony_core harmony_examples)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
target_link_libraries(testharmony PRIVATE GTest::gtest_main)

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
        "libwinpthread*.dll" "libgcc*.dll"
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

    add_test(NAME testharmony COMMAND testharmony --gtest_filter=-TestExamples.*)
    add_test(NAME TestExamples COMMAND testharmony --gtest_filter=TestExamples.*)
    set(_harmony_path_mods "")
    foreach(_dir IN LISTS _harmony_dll_bin_dirs)
        list(APPEND _harmony_path_mods "PATH=path_list_prepend:${_dir}")
    endforeach()
    set_tests_properties(testharmony TestExamples PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
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
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
