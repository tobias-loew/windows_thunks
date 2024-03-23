cmake_minimum_required(VERSION 3.25)

project(thunk_test)
set(CMAKE_CXX_STANDARD 23)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(TARGET_NAME thunk_test)
add_executable(${TARGET_NAME} thunk_test.cpp)

target_link_libraries(${TARGET_NAME} Boost::unit_test_framework)
