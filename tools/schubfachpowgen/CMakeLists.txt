cmake_minimum_required(VERSION 3.22.1)

set(SCHUBFACHPOWGEN_VERSION ${DCONV_VERSION})

set(SCHUBFACHPOWGEN_VERSION_MAJOR ${DCONV_VERSION_MAJOR})
set(SCHUBFACHPOWGEN_VERSION_MINOR ${DCONV_VERSION_MINOR})
set(SCHUBFACHPOWGEN_VERSION_PATCH ${DCONV_VERSION_PATCH})

message(STATUS "schubfachpowgen version: ${SCHUBFACHPOWGEN_VERSION}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp")

set(PRIVATE_HEADER_DIRS
    src
)

set(PUBLIC_HEADER_DIRS
    include
    ../eisellemirepowgen/include
)

set(PUBLIC_HEADERS
    include/version.hpp
)

set(SOURCES
    src/main.cpp
)

add_executable(schubfachpowgen ${SOURCES})
set_target_properties(schubfachpowgen PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(schubfachpowgen PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(schubfachpowgen PRIVATE ${PRIVATE_HEADER_DIRS})

install(TARGETS schubfachpowgen DESTINATION /bin)
