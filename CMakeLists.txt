cmake_minimum_required(VERSION 2.8)

# Options. Turn on with 'cmake -Dtest=ON'
option(test "Build all tests." OFF)

set(PROJECT_NAME echo)
project(${PROJECT_NAME} CXX)
find_package(Threads REQUIRED)

# Set options for build
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Location of the .h headers
include_directories(${echo_SOURCE_DIR}/include ${echo_SOURCE_DIR})

# Set echo lib
set(echo_LIB ${PROJECT_NAME}_lib)
file(GLOB SRC_FILES ${echo_SOURCE_DIR}/src/*.cpp)
add_library(${echo_LIB} ${SRC_FILES})

# Set testing files
set(echo_TEST_NAME ${PROJECT_NAME}_test)
file(GLOB TEST_SRC_FILES ${echo_SOURCE_DIR}/tests/*.cpp)

# Add logging
include_directories(${echo_SOURCE_DIR}/lib/spdlog ${echo_SOURCE_DIR})

# tinyxml2 library
set(tinyxml2_LIB tinyxml2_lib)
file(GLOB TINYXML2_SRC_FILES ${echo_SOURCE_DIR}/lib/tinyxml2/tinyxml2.cpp)
include_directories(${echo_SOURCE_DIR}/lib/tinyxml2 ${echo_SOURCE_DIR})
add_library(${tinyxml2_LIB} ${TINYXML2_SRC_FILES})

# Check for testing or not
if (test)
    enable_testing()
    
    add_executable(${echo_TEST_NAME} ${TEST_SRC_FILES})
    add_subdirectory(${echo_SOURCE_DIR}/lib/gtest)

    add_dependencies(${echo_TEST_NAME} gtest)

    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    target_link_libraries(${echo_TEST_NAME} gtest gtest_main)
    target_link_libraries(${echo_TEST_NAME} ${echo_LIB})

    add_custom_target(unittest 
        COMMAND ${echo_TEST_NAME}
        DEPENDS ${echo_TEST_NAME})

else()
    # Set executable for normal operation
    add_executable(${PROJECT_NAME} ${echo_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(${echo_LIB} ${tinyxml2_LIB})
    target_link_libraries(${PROJECT_NAME} ${echo_LIB})
    target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
endif()
