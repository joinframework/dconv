cmake_minimum_required(VERSION 3.28.3)
project(dconv)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" DCONV_VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" DCONV_VERSION_MAJOR ${DCONV_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" DCONV_VERSION_MINOR ${DCONV_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" DCONV_VERSION_PATCH ${DCONV_VERSION})

message(STATUS "dconv version: ${DCONV_VERSION}")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to Release as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options(
    -fstack-protector-strong
    -Wformat
    -Wformat-security
    -Wall
    -Wextra
    -Werror
    -Wno-address-of-packed-member
)

add_link_options(
    -Wl,--as-needed
    -Wl,-z,relro,-z,now
)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -D_FORTIFY_SOURCE=2 -O3")

option(DCONV_ENABLE_TESTS "Enable tests." OFF)
option(DCONV_ENABLE_COVERAGE "Enable coverage." OFF)

if(DCONV_ENABLE_TESTS)
    enable_testing()
endif()

if(DCONV_ENABLE_COVERAGE)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(FATAL_ERROR "Code coverage results with a non-Debug build may be misleading")
    endif()
    if(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(--coverage -fprofile-update=atomic)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "Code coverage requires GCC")
    endif()
endif()

add_subdirectory(dconv)
add_subdirectory(tools)
