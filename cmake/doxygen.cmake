# -----------------------------------------------------------------------------
# Doxygen API documentation target (optional)
#
# Build with:  cmake .. -DBUILD_DOCS=ON
# Generate:    cmake --build . --target harmony-docs
# Output:       build/docs/html/index.html
# -----------------------------------------------------------------------------

option(BUILD_DOCS "Build Doxygen API documentation" OFF)

if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN  ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
        file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

        add_custom_target(harmony-docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating Harmony API documentation with Doxygen"
            VERBATIM
        )
        message(STATUS "Doxygen found: 'harmony-docs' target available (cmake --build . --target harmony-docs)")
    else()
        message(WARNING "BUILD_DOCS=ON but Doxygen was not found. Install doxygen package.")
    endif()
endif()
