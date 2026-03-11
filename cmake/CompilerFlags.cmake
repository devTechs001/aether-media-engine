# CompilerFlags.cmake
# Compiler configuration for AETHER Media Engine

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Warning flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )

    # Clang-specific
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(
            -Wimplicit-fallthrough
            -Wdocumentation
        )
    endif()

    # GCC-specific
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(
            -Wlogical-op
            -Wuseless-cast
        )
    endif()

    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

    # Release flags
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

    # RelWithDebInfo flags
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

    # MinSizeRel flags
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

elseif(MSVC)
    # MSVC warning flags
    add_compile_options(
        /W4
        /WX-
        /permissive-
    )

    # MSVC-specific definitions
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )

    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /MDd")

    # Release flags
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /MD")

    # RelWithDebInfo flags
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /O2 /MD")
endif()

# Link-time optimization for Release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_OUTPUT)
if(LTO_SUPPORTED)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
endif()

# Sanitizers (optional)
option(ENABLE_SANITIZERS "Enable sanitizers for debugging" OFF)
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
    )
    add_link_options(
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

# Cache line size
set(CACHE_LINE_SIZE 64 CACHE STRING "Cache line size for alignment")
