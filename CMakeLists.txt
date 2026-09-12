cmake_minimum_required(VERSION 3.15)
project(nadir VERSION 1.0.1 LANGUAGES C)

# Set ISO C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Source files
file(GLOB NADIR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Executable target
add_executable(nadir ${NADIR_SOURCES})

# Target include directories
target_include_directories(nadir PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compiler warnings and options
if(MSVC)
    target_compile_options(nadir PRIVATE /W4 /utf-8 /wd4996)
    target_compile_definitions(nadir PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(nadir PRIVATE ws2_32)
else()
    target_compile_options(nadir PRIVATE -Wall -Wextra -Wno-unused-parameter)
    # Link standard math library on POSIX platforms (Linux, macOS, BSD)
    if(APPLE)
        target_compile_definitions(nadir PRIVATE _DARWIN_C_SOURCE)
        target_link_libraries(nadir PRIVATE m)
    elseif(NOT WIN32)
        target_compile_definitions(nadir PRIVATE _POSIX_C_SOURCE=200809L _DEFAULT_SOURCE)
        target_link_libraries(nadir PRIVATE m)
    else()
        target_link_libraries(nadir PRIVATE ws2_32)
    endif()
endif()

# Output binary properties
set_target_properties(nadir PROPERTIES
    OUTPUT_NAME "nadir"
    C_VISIBILITY_PRESET hidden
)

# Installation rules
include(GNUInstallDirs)
install(TARGETS nadir
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
install(DIRECTORY examples/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/nadir/examples
)

# CPack configuration for distributable packages
set(CPACK_PACKAGE_NAME "nadir")
set(CPACK_PACKAGE_VENDOR "James Ezra Seitenschlag")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Salesforce Apex runtime engine in ISO C11")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;7Z")
else()
    set(CPACK_GENERATOR "TGZ;ZIP")
endif()

include(CPack)
