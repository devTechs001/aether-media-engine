# FindFFmpeg.cmake
# Find the native FFmpeg includes and libraries
#
# This module defines:
#   FFMPEG_FOUND        - True if FFmpeg found
#   FFMPEG_INCLUDE_DIRS - Include directories
#   FFMPEG_LIBRARIES    - Libraries to link
#   FFMPEG_VERSION      - Version string

find_package(PkgConfig QUIET)

# FFmpeg components
set(FFMPEG_COMPONENTS
    avcodec
    avdevice
    avfilter
    avformat
    avutil
    postproc
    swresample
    swscale
)

foreach(COMP ${FFMPEG_COMPONENTS})
    string(TOUPPER ${COMP} COMP_UPPER)
    pkg_check_modules(PC_FFMPEG_${COMP_UPPER} QUIET lib${COMP})
    set(FFMPEG_${COMP_UPPER}_FOUND ${PC_FFMPEG_${COMP_UPPER}_FOUND})
endforeach()

# Find include directories
find_path(FFMPEG_avcodec_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    HINTS ${PC_FFMPEG_AVCODEC_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include /opt/local/include
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_avformat_INCLUDE_DIR
    NAMES libavformat/avformat.h
    HINTS ${PC_FFMPEG_AVFORMAT_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include /opt/local/include
    PATH_SUFFIXES ffmpeg
)

find_path(FFMPEG_avutil_INCLUDE_DIR
    NAMES libavutil/avutil.h
    HINTS ${PC_FFMPEG_AVUTIL_INCLUDE_DIRS}
    PATHS /usr/include /usr/local/include /opt/local/include
    PATH_SUFFIXES ffmpeg
)

set(FFMPEG_INCLUDE_DIRS
    ${FFMPEG_avcodec_INCLUDE_DIR}
    ${FFMPEG_avformat_INCLUDE_DIR}
    ${FFMPEG_avutil_INCLUDE_DIR}
)

# Find libraries
find_library(FFMPEG_avcodec_LIBRARY
    NAMES avcodec
    HINTS ${PC_FFMPEG_AVCODEC_LIBRARY_DIRS}
    PATHS /usr/lib /usr/local/lib /opt/local/lib
)

find_library(FFMPEG_avformat_LIBRARY
    NAMES avformat
    HINTS ${PC_FFMPEG_AVFORMAT_LIBRARY_DIRS}
    PATHS /usr/lib /usr/local/lib /opt/local/lib
)

find_library(FFMPEG_avutil_LIBRARY
    NAMES avutil
    HINTS ${PC_FFMPEG_AVUTIL_LIBRARY_DIRS}
    PATHS /usr/lib /usr/local/lib /opt/local/lib
)

set(FFMPEG_LIBRARIES
    ${FFMPEG_avcodec_LIBRARY}
    ${FFMPEG_avformat_LIBRARY}
    ${FFMPEG_avutil_LIBRARY}
)

# Version
if(FFMPEG_avcodec_INCLUDE_DIR)
    file(STRINGS "${FFMPEG_avcodec_INCLUDE_DIR}/libavcodec/version.h"
        FFMPEG_VERSION_LINE REGEX "^#define LIBAVCODEC_VERSION_MAJOR")
    string(REGEX MATCH "[0-9]+" FFMPEG_VERSION_MAJOR "${FFMPEG_VERSION_LINE}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg
    REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS
    VERSION_VAR FFMPEG_VERSION_MAJOR
)

mark_as_advanced(
    FFMPEG_avcodec_INCLUDE_DIR
    FFMPEG_avformat_INCLUDE_DIR
    FFMPEG_avutil_INCLUDE_DIR
    FFMPEG_avcodec_LIBRARY
    FFMPEG_avformat_LIBRARY
    FFMPEG_avutil_LIBRARY
)
