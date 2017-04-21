project (word2vec++)
cmake_minimum_required(VERSION 3.1)
 
set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

set(CMAKE_BINARY_DIR ${PROJECT_ROOT_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
# add -fsanitize=address -fno-omit-frame-pointer options to anable AddressSanitizer
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -funroll-loops -ftree-vectorize -DNDEBUG")

set(PROJECT_INCLUDE_DIR ${PROJECT_ROOT_DIR}/include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(LIBS "-pthread")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(examples/word2vec)
add_subdirectory(examples/doc2vec)
