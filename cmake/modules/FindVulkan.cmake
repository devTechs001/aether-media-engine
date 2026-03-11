# FindVulkan.cmake
# Find the Vulkan graphics API
#
# This module defines:
#   Vulkan_FOUND          - True if Vulkan found
#   Vulkan_INCLUDE_DIRS   - Include directories
#   Vulkan_LIBRARIES      - Libraries to link
#   Vulkan_glslc_EXECUTABLE - GLSL compiler

find_package(PkgConfig QUIET)

# Find Vulkan headers
find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    HINTS
        $ENV{VULKAN_SDK}/include
        ${PC_Vulkan_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
)

# Find Vulkan library
if(WIN32)
    find_library(Vulkan_LIBRARY
        NAMES vulkan-1 vulkan
        HINTS
            $ENV{VULKAN_SDK}/Lib
        PATHS
            /usr/lib
            /usr/local/lib
    )
elseif(APPLE)
    find_library(Vulkan_LIBRARY
        NAMES MoltenVK
        HINTS
            $ENV{VULKAN_SDK}/lib
        PATHS
            /usr/lib
            /usr/local/lib
    )
else()
    find_library(Vulkan_LIBRARY
        NAMES vulkan
        HINTS
            $ENV{VULKAN_SDK}/lib
            ${PC_Vulkan_LIBRARY_DIRS}
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
    )
endif()

set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})
set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})

# Find glslc compiler
find_program(Vulkan_glslc_EXECUTABLE
    NAMES glslc
    HINTS
        $ENV{VULKAN_SDK}/bin
    PATHS
        /usr/bin
        /usr/local/bin
        /opt/local/bin
)

# Find glslangValidator
find_program(Vulkan_glslangValidator_EXECUTABLE
    NAMES glslangValidator
    HINTS
        $ENV{VULKAN_SDK}/bin
    PATHS
        /usr/bin
        /usr/local/bin
        /opt/local/bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan
    REQUIRED_VARS Vulkan_LIBRARIES Vulkan_INCLUDE_DIR
)

if(Vulkan_FOUND AND NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(
    Vulkan_INCLUDE_DIR
    Vulkan_LIBRARY
    Vulkan_glslc_EXECUTABLE
    Vulkan_glslangValidator_EXECUTABLE
)
