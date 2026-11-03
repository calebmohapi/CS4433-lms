cmake_minimum_required(VERSION 3.15)
project(LMS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SQLite3 REQUIRED)

add_library(lms_core
    src/util.cpp
    src/domain.cpp
    src/data.cpp
    src/services.cpp
    src/ui.cpp
)
target_include_directories(lms_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lms_core PUBLIC SQLite3::SQLite3)

add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_executable(lms_tests tests/test_main.cpp)
    target_link_libraries(lms_tests PRIVATE lms_core)
    add_test(NAME lms_tests COMMAND lms_tests)
endif()