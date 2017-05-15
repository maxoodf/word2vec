project (word2vec)
cmake_minimum_required(VERSION 3.1)

set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${PROJECT_INCLUDE_DIR}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(ADD_SRCS
            ${PROJECT_SOURCE_DIR}/win/mman.h
            ${PROJECT_SOURCE_DIR}/win/mman.cpp
            )
endif()

set(PRJ_SRCS
        ${PROJECT_INCLUDE_DIR}/word2vec.h
        ${PROJECT_SOURCE_DIR}/word2vec.cpp
        ${PROJECT_SOURCE_DIR}/mapper.h
        ${PROJECT_SOURCE_DIR}/mapper.cpp
        ${PROJECT_SOURCE_DIR}/wordReader.h
        ${PROJECT_SOURCE_DIR}/vocabulary.h
        ${PROJECT_SOURCE_DIR}/vocabulary.cpp
        ${PROJECT_SOURCE_DIR}/huffmanTree.h
        ${PROJECT_SOURCE_DIR}/huffmanTree.cpp
        ${PROJECT_SOURCE_DIR}/nsDistribution.h
        ${PROJECT_SOURCE_DIR}/nsDistribution.cpp
        ${PROJECT_SOURCE_DIR}/downSampling.h
        ${PROJECT_SOURCE_DIR}/trainer.h
        ${PROJECT_SOURCE_DIR}/trainer.cpp
        ${PROJECT_SOURCE_DIR}/trainThread.h
        ${PROJECT_SOURCE_DIR}/trainThread.cpp
        ${ADD_SRCS}
        )

add_library(${PROJECT_NAME} STATIC ${PRJ_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${PROJECT_INCLUDE_DIR}/word2vec.h DESTINATION include)