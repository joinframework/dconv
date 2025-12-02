cmake_minimum_required(VERSION 3.28.3)

set(PUBLIC_HEADER_DIRS
    include
)

set(PUBLIC_HEADERS
    include/dconv/view.hpp
    include/dconv/diyfp.hpp
    include/dconv/power.hpp
    include/dconv/atod.hpp
    include/dconv/dtoa.hpp
)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PUBLIC_HEADER_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION include/dconv)

if(DCONV_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
