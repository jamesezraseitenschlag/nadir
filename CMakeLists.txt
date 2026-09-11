cmake_minimum_required(VERSION 3.15)
project(nadir VERSION 1.0.0 LANGUAGES C)

# Set C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB NADIR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Executable target
add_executable(nadir ${NADIR_SOURCES})

# Target include directories
target_include_directories(nadir PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compiler warnings and options
if(MSVC)
    target_compile_options(nadir PRIVATE /W4 /utf-8)
    target_compile_definitions(nadir PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(nadir PRIVATE -Wall -Wextra -Wno-unused-parameter)
    # Link math library on POSIX platforms (Linux, macOS, BSD)
    if(NOT WIN32)
        target_link_libraries(nadir PRIVATE m)
    endif()
endif()

# Output binary naming
set_target_properties(nadir PROPERTIES
    OUTPUT_NAME "nadir"
    C_VISIBILITY_PRESET hidden
)

# Installation rules
include(GNUInstallDirs)
install(TARGETS nadir
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
