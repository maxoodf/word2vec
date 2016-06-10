project (${LIB_WR})
cmake_minimum_required(VERSION 2.6)
 
set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR})
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
 
set(PRJ_WR_SRCS 
    ${PROJECT_SOURCE_DIR}/wrErrors.h
    ${PROJECT_SOURCE_DIR}/wordReader.h
    ${PROJECT_SOURCE_DIR}/wordReader.cpp
)
 
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")
 
add_library(${PROJECT_NAME} SHARED ${PRJ_WR_SRCS})
