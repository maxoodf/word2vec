project (wordvecpp)
cmake_minimum_required(VERSION 2.6)
 
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
 
#set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/wordReader)
#set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/doc2vec)
include_directories("${PROJECT_SOURCE_DIR}/wordReader")
include_directories("${PROJECT_SOURCE_DIR}/word2vec")

if(UNIX AND APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -march=native -Wall -funroll-loops ${CMAKE_CXX_FLAGS}")

set(LIB_WR "wordReader")
add_subdirectory(wordReader)

set(LIB_W2V "word2vec")
add_subdirectory(word2vec)

add_subdirectory(word2vecTrainer)

include_directories("${PROJECT_SOURCE_DIR}/word2vec")
set(LIB_D2V "doc2vec")
add_subdirectory(doc2vec)

include_directories("${PROJECT_SOURCE_DIR}/doc2vec")
add_subdirectory(doc2vecTrainer)

#add_subdirectory(examples/postgresql)
