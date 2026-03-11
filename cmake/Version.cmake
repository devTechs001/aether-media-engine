# Version.cmake
# Version management for AETHER Media Engine

set(AETHER_VERSION_MAJOR 1)
set(AETHER_VERSION_MINOR 0)
set(AETHER_VERSION_PATCH 0)
set(AETHER_VERSION_CODENAME "Prometheus")
set(AETHER_VERSION_STRING "${AETHER_VERSION_MAJOR}.${AETHER_VERSION_MINOR}.${AETHER_VERSION_PATCH}")

# Build information
string(TIMESTAMP AETHER_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP AETHER_BUILD_TIME "%H:%M:%S")

# Git information (if available)
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE AETHER_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE AETHER_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT AETHER_GIT_COMMIT)
    set(AETHER_GIT_COMMIT "unknown")
endif()

if(NOT AETHER_GIT_BRANCH)
    set(AETHER_GIT_BRANCH "unknown")
endif()

# Compiler information
set(AETHER_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
set(AETHER_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

# Platform information
set(AETHER_PLATFORM ${CMAKE_SYSTEM_NAME})
set(AETHER_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# Configure version header
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.hpp.in
    ${CMAKE_BINARY_DIR}/include/aether/version.hpp
    @ONLY
)
