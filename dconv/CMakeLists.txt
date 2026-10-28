cmake_minimum_required(VERSION 3.22.1)

set(PUBLIC_HEADERS
    include/dconv/probe.hpp
    include/dconv/diyfp.hpp
    include/dconv/atod.hpp
    include/dconv/atodpow.hpp
    include/dconv/dtoa.hpp
    include/dconv/dtoapow.hpp
)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
install(TARGETS ${PROJECT_NAME}
    EXPORT dconvTargets
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dconv COMPONENT dev
)

if(DCONV_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
