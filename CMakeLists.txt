cmake_minimum_required(VERSION 3.0)
project(CTML)

set (CMAKE_CXX_STANDARD 11)

set(CTML_TEST_SOURCE ${PROJECT_SOURCE_DIR}/tests/tests.cpp)

add_library(CTML INTERFACE)
target_include_directories(CTML INTERFACE include/)

option(CTML_TESTS_ENABLE "Whether or not to build tests for CTML." ON)

if (CTML_TESTS_ENABLE)
    enable_testing()
    add_executable(CTMLTest ${CTML_TEST_SOURCE})
    target_link_libraries(CTMLTest CTML)
    add_test(NAME CTMLTests COMMAND CTMLTest)
endif()