project (word2vecTrainer)
cmake_minimum_required(VERSION 2.6)
 
set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR})
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
 
set(PRJ_W2V_TRAINER_SRCS
    ${PROJECT_SOURCE_DIR}/word2vecTrainer.h
    ${PROJECT_SOURCE_DIR}/word2vecTrainer.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

if(UNIX AND NOT APPLE)
    set(LIBS "pthread")
endif()

add_executable(${PROJECT_NAME} ${PRJ_W2V_TRAINER_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIB_WR} ${LIBS})
