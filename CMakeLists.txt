cmake_minimum_required(VERSION 3.22.1)
project(dconv)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${PROJECT_SOURCE_DIR}/VERSION" DCONV_VERSION)
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" DCONV_VERSION_MAJOR ${DCONV_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" DCONV_VERSION_MINOR ${DCONV_VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" DCONV_VERSION_PATCH ${DCONV_VERSION})

message(STATUS "dconv version: ${DCONV_VERSION}")

include(GNUInstallDirs)

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
option(DCONV_ENABLE_ASAN "Enable address and undefined behavior sanitizers." OFF)

if(DCONV_ENABLE_TESTS)
    enable_testing()
endif()

if(DCONV_ENABLE_COVERAGE)
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message(FATAL_ERROR "Code coverage results with a non-Debug build may be misleading")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling coverage for ${CMAKE_CXX_COMPILER_ID}")
        add_compile_options(--coverage -fprofile-update=atomic)
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "Code coverage requires GCC or Clang")
    endif()
endif()

if(DCONV_ENABLE_ASAN)
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        message(FATAL_ERROR "Address sanitizer requires GCC or Clang")
    endif()
    message(STATUS "Enabling address and undefined behavior sanitizers for ${CMAKE_CXX_COMPILER_ID}")
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(dconv)
add_subdirectory(tools)

include(CMakePackageConfigHelpers)

install(EXPORT dconvTargets
    NAMESPACE dconv::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT dev
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/dconvConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/dconvConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/dconvConfigVersion.cmake
    VERSION ${DCONV_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dconvConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/dconvConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT dev
)

include(${PROJECT_SOURCE_DIR}/cmake/packaging.cmake)
