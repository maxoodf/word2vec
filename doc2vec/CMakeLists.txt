project (${LIB_D2V})
cmake_minimum_required(VERSION 2.6)
 
set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR})
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
 
set(PRJ_D2V_SRCS 
    ${PROJECT_SOURCE_DIR}/doc2vec.h
    ${PROJECT_SOURCE_DIR}/doc2vec.cpp
)
 
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")
 
#add_library(${PROJECT_NAME} SHARED ${PRJ_D2V_SRCS})
add_library(${PROJECT_NAME} STATIC ${PRJ_D2V_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIB_WR} ${LIB_W2V})
