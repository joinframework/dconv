cmake_minimum_required(VERSION 3.28.3)

set(POWGEN_VERSION ${DCONV_VERSION})

set(POWGEN_VERSION_MAJOR ${DCONV_VERSION_MAJOR})
set(POWGEN_VERSION_MINOR ${DCONV_VERSION_MINOR})
set(POWGEN_VERSION_PATCH ${DCONV_VERSION_PATCH})

message(STATUS "powgen version: ${POWGEN_VERSION}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp")

set(PRIVATE_HEADER_DIRS
    src
)

set(PUBLIC_HEADER_DIRS
    include
)

set(PUBLIC_HEADERS
    include/version.hpp
    include/bigint.hpp
)

set(SOURCES
    src/main.cpp
)

add_executable(powgen ${SOURCES})
set_target_properties(powgen PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(powgen PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(powgen PRIVATE ${PRIVATE_HEADER_DIRS})

install(TARGETS powgen DESTINATION /bin)
