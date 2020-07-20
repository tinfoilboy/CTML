cmake_minimum_required(VERSION 3.0)
project(CTML)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set(CTML_TEST_SOURCES
        ${PROJECT_SOURCE_DIR}/tests/constructionTests.cpp
        ${PROJECT_SOURCE_DIR}/tests/behaviourTests.cpp)

add_library(CTML INTERFACE)
target_include_directories(CTML INTERFACE include/)

option(CTML_TESTS_ENABLE "Whether or not to build tests for CTML." ON)

if (CTML_TESTS_ENABLE)
    enable_testing()
    add_executable(CTMLTest ${CTML_TEST_SOURCES})
    target_link_libraries(CTMLTest CTML)
    add_test(NAME CTMLTests COMMAND CTMLTest)
    set_target_properties(CTMLTest PROPERTIES COMPILE_FLAGS "-D_GLIBCXX_DEBUG -g")
endif()