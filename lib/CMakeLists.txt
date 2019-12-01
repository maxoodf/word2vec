project (word2vec)
cmake_minimum_required(VERSION 3.1)

set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${PROJECT_INCLUDE_DIR}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(ADD_SRCS
            ${PROJECT_SOURCE_DIR}/win/mman.hpp
            ${PROJECT_SOURCE_DIR}/win/mman.cpp
            )
endif()

set(PRJ_SRCS
        ${PROJECT_INCLUDE_DIR}/word2vec.hpp
        ${PROJECT_SOURCE_DIR}/word2vec.cpp
#        ${PROJECT_INCLUDE_DIR}/word2vec.h
#        ${PROJECT_SOURCE_DIR}/c_binding.cpp
        ${PROJECT_SOURCE_DIR}/mapper.hpp
        ${PROJECT_SOURCE_DIR}/mapper.cpp
        ${PROJECT_SOURCE_DIR}/wordReader.hpp
        ${PROJECT_SOURCE_DIR}/vocabulary.hpp
        ${PROJECT_SOURCE_DIR}/vocabulary.cpp
        ${PROJECT_SOURCE_DIR}/huffmanTree.hpp
        ${PROJECT_SOURCE_DIR}/huffmanTree.cpp
        ${PROJECT_SOURCE_DIR}/nsDistribution.hpp
        ${PROJECT_SOURCE_DIR}/nsDistribution.cpp
        ${PROJECT_SOURCE_DIR}/downSampling.hpp
        ${PROJECT_SOURCE_DIR}/trainer.hpp
        ${PROJECT_SOURCE_DIR}/trainer.cpp
        ${PROJECT_SOURCE_DIR}/trainThread.hpp
        ${PROJECT_SOURCE_DIR}/trainThread.cpp
        ${ADD_SRCS}
        )

add_library(${PROJECT_NAME} STATIC ${PRJ_SRCS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${PROJECT_INCLUDE_DIR}/word2vec.hpp DESTINATION include)
