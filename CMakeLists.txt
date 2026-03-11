# ═══════════════════════════════════════════════════════════════════════════════
# AETHER Media Engine - Root CMakeLists.txt
# ═══════════════════════════════════════════════════════════════════════════════

cmake_minimum_required(VERSION 3.25)

# Project definition
project(AetherMediaEngine
    VERSION 1.0.0
    DESCRIPTION "Enterprise-Grade Cross-Platform AI-Powered Media Player Suite"
    HOMEPAGE_URL "https://github.com/devTechs001/aether-media-engine"
    LANGUAGES CXX C
)

# ═══════════════════════════════════════════════════════════════════════════════
# Build Options
# ═══════════════════════════════════════════════════════════════════════════════

option(AETHER_BUILD_GUI "Build GUI application" ON)
option(AETHER_BUILD_TESTS "Build tests" OFF)
option(AETHER_BUILD_DOCS "Build documentation" OFF)
option(AETHER_WITH_FFMPEG "Enable FFmpeg support" ON)
option(AETHER_WITH_VULKAN "Enable Vulkan rendering" ON)
option(AETHER_WITH_ONNX "Enable ONNX Runtime for AI" ON)
option(AETHER_WITH_CUDA "Enable CUDA support" OFF)
option(AETHER_WITH_DRM "Enable DRM support" ON)
option(ENABLE_SANITIZERS "Enable sanitizers for debugging" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# ═══════════════════════════════════════════════════════════════════════════════
# CMake Configuration
# ═══════════════════════════════════════════════════════════════════════════════

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include paths
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# ═══════════════════════════════════════════════════════════════════════════════
# Load CMake Modules
# ═══════════════════════════════════════════════════════════════════════════════

include(cmake/CompilerFlags.cmake)
include(cmake/Version.cmake)

# ═══════════════════════════════════════════════════════════════════════════════
# Find Dependencies
# ═══════════════════════════════════════════════════════════════════════════════

include(cmake/Dependencies.cmake)

# ═══════════════════════════════════════════════════════════════════════════════
# AETHER Core Library
# ═══════════════════════════════════════════════════════════════════════════════

add_library(aether_core INTERFACE)
target_include_directories(aether_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(aether_core INTERFACE
    Qt6::Core
    Threads::Threads
)

# ═══════════════════════════════════════════════════════════════════════════════
# MediaGUI Application (Existing Code)
# ═══════════════════════════════════════════════════════════════════════════════

if(AETHER_BUILD_GUI)
    # Create QML resource file
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qml.qrc"
        '<RCC>\n  <qresource prefix="/qml">\n    <file>qml/Main.qml</file>\n  </qresource>\n</RCC>'
    )

    qt_add_executable(mediagui
        WIN32 MACOSX_BUNDLE
        src/main.cpp
        src/player-core.cpp
        src/player-core.h
        src/playlist-model.cpp
        src/playlist-model.h
        src/config.cpp
        src/core/error.cpp
        src/utils/logging.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
    )

    target_link_libraries(mediagui PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Multimedia
        Qt6::Widgets
        aether_core
    )

    target_include_directories(mediagui PRIVATE src)

    # Set properties
    set_target_properties(mediagui PROPERTIES
        OUTPUT_NAME "mediagui"
        MACOSX_BUNDLE_BUNDLE_NAME "MediaGUI"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.darkhat.mediagui"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
    )

    # Install target
    install(TARGETS mediagui
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# Tests
# ═══════════════════════════════════════════════════════════════════════════════

if(AETHER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# Documentation
# ═══════════════════════════════════════════════════════════════════════════════

if(AETHER_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/api)
        doxygen_add_docs(docs
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Generate API documentation"
        )
    endif()
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# Installation
# ═══════════════════════════════════════════════════════════════════════════════

include(GNUInstallDirs)

# Install headers
install(DIRECTORY include/aether
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install generated version header
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/aether/version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/aether
)

# Install CMake configuration
install(EXPORT AetherMediaEngineTargets
    FILE AetherMediaEngineTargets.cmake
    NAMESPACE AetherMediaEngine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AetherMediaEngine
)

# ═══════════════════════════════════════════════════════════════════════════════
# Packaging
# ═══════════════════════════════════════════════════════════════════════════════

include(cmake/Packaging.cmake)

# ═══════════════════════════════════════════════════════════════════════════════
# Summary
# ═══════════════════════════════════════════════════════════════════════════════

message(STATUS "")
message(STATUS "AETHER Media Engine ${PROJECT_VERSION}")
message(STATUS "────────────────────────────────────────")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build GUI:         ${AETHER_BUILD_GUI}")
message(STATUS "Build Tests:       ${AETHER_BUILD_TESTS}")
message(STATUS "Build Docs:        ${AETHER_BUILD_DOCS}")
message(STATUS "With FFmpeg:       ${AETHER_WITH_FFMPEG}")
message(STATUS "With Vulkan:       ${AETHER_WITH_VULKAN}")
message(STATUS "With ONNX:         ${AETHER_WITH_ONNX}")
message(STATUS "With CUDA:         ${AETHER_WITH_CUDA}")
message(STATUS "With DRM:          ${AETHER_WITH_DRM}")
message(STATUS "Sanitizers:        ${ENABLE_SANITIZERS}")
message(STATUS "")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
