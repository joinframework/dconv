cmake_minimum_required(VERSION 3.22.1)

find_package(GTest REQUIRED)

add_executable(string_probe.gtest string_probe_test.cpp)
target_link_libraries(string_probe.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(string_probe.gtest)

add_executable(stream_probe.gtest stream_probe_test.cpp)
target_link_libraries(stream_probe.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(stream_probe.gtest)

add_executable(atod.gtest atod_test.cpp)
target_link_libraries(atod.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(atod.gtest)

add_executable(dtoa.gtest dtoa_test.cpp)
target_link_libraries(dtoa.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(dtoa.gtest)

add_executable(roundtrip.gtest roundtrip_test.cpp)
target_link_libraries(roundtrip.gtest ${PROJECT_NAME} GTest::gtest_main pthread)
gtest_discover_tests(roundtrip.gtest)
