# FindONNXRuntime.cmake
# Find the ONNX Runtime library
#
# This module defines:
#   ONNXRuntime_FOUND        - True if ONNX Runtime found
#   ONNXRuntime_INCLUDE_DIRS - Include directories
#   ONNXRuntime_LIBRARIES    - Libraries to link
#   ONNXRuntime_VERSION      - Version string

find_package(PkgConfig QUIET)

# Find ONNX Runtime headers
find_path(ONNXRuntime_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    HINTS
        $ENV{ONNXRUNTIME_DIR}/include
        ${PC_ONNXRuntime_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES
        onnxruntime
        onnxruntime/core/session
)

# Find ONNX Runtime library
if(WIN32)
    find_library(ONNXRuntime_LIBRARY
        NAMES onnxruntime
        HINTS
            $ENV{ONNXRUNTIME_DIR}/lib
        PATHS
            /usr/lib
            /usr/local/lib
    )
elseif(APPLE)
    find_library(ONNXRuntime_LIBRARY
        NAMES onnxruntime
        HINTS
            $ENV{ONNXRUNTIME_DIR}/lib
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )
else()
    find_library(ONNXRuntime_LIBRARY
        NAMES onnxruntime
        HINTS
            $ENV{ONNXRUNTIME_DIR}/lib
            ${PC_ONNXRuntime_LIBRARY_DIRS}
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )
endif()

set(ONNXRuntime_INCLUDE_DIRS ${ONNXRuntime_INCLUDE_DIR})
set(ONNXRuntime_LIBRARIES ${ONNXRuntime_LIBRARY})

# Version
set(ONNXRuntime_VERSION "")
if(ONNXRuntime_INCLUDE_DIR AND EXISTS "${ONNXRuntime_INCLUDE_DIR}/onnxruntime_c_api.h")
    file(STRINGS "${ONNXRuntime_INCLUDE_DIR}/onnxruntime_c_api.h"
        ONNXRuntime_VERSION_LINE REGEX "^#define ORT_API_VERSION")
    string(REGEX MATCH "[0-9]+" ONNXRuntime_VERSION "${ONNXRuntime_VERSION_LINE}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ONNXRuntime
    REQUIRED_VARS ONNXRuntime_LIBRARIES ONNXRuntime_INCLUDE_DIRS
    VERSION_VAR ONNXRuntime_VERSION
)

if(ONNXRuntime_FOUND AND NOT TARGET ONNXRuntime::ONNXRuntime)
    add_library(ONNXRuntime::ONNXRuntime UNKNOWN IMPORTED)
    set_target_properties(ONNXRuntime::ONNXRuntime PROPERTIES
        IMPORTED_LOCATION "${ONNXRuntime_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRuntime_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(
    ONNXRuntime_INCLUDE_DIR
    ONNXRuntime_LIBRARY
)
