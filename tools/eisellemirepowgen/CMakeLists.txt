cmake_minimum_required(VERSION 3.22.1)

set(EISELLEMIREPOWGEN_VERSION ${DCONV_VERSION})

set(EISELLEMIREPOWGEN_VERSION_MAJOR ${DCONV_VERSION_MAJOR})
set(EISELLEMIREPOWGEN_VERSION_MINOR ${DCONV_VERSION_MINOR})
set(EISELLEMIREPOWGEN_VERSION_PATCH ${DCONV_VERSION_PATCH})

message(STATUS "eisellemirepowgen version: ${EISELLEMIREPOWGEN_VERSION}")
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

add_executable(eisellemirepowgen ${SOURCES})
set_target_properties(eisellemirepowgen PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_include_directories(eisellemirepowgen PUBLIC  ${PUBLIC_HEADER_DIRS})
target_include_directories(eisellemirepowgen PRIVATE ${PRIVATE_HEADER_DIRS})

install(TARGETS eisellemirepowgen DESTINATION /bin)
