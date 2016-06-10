//
//  word2vec.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 19/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//
#include <string.h>

#include <cmath>

#include "wordReader.h"
#include "word2vec.h"

word2vec_t::word2vec_t(const std::string &_fileName): m_wordsMap() {
    fileMapper_t fileMapper(_fileName);
    uint64_t fileSize = fileMapper.size();
    
    uint64_t shift = 0;
    const char *header = fileMapper.data();
    
    const char *pos = static_cast<const char *>(memchr(header, ' ', fileSize));
    if (pos == NULL) {
        return;
    }
    std::size_t vocSize = std::stoul(std::string(header, pos - header));
    shift += pos - header + 1;
    header = pos + 1;
    
    pos = static_cast<const char *>(memchr(header, '\n', fileSize - shift));
    if (pos == NULL) {
        return;
    }
    uint16_t layerSize = std::stoi(std::string(header, pos - header));
    shift += pos - header + 1;
    header = pos + 1;

//    std::cout << vocSize << " " << layerSize << std::endl;
    
    wordVector_t wordVector(layerSize, 0.0);
    for (std::size_t i = 0; i < vocSize; ++i) {
        pos = static_cast<const char *>(memchr(header, ' ', fileSize - shift));
        if (pos == NULL) {
            return;
        }
        std::string word(header, pos - header);
        shift += pos - header + 1;
        header = pos + 1;
        
        float len = 0.0;
        const float *syn0 = (const float*) header;
        for (auto j = 0; j < layerSize; ++j) {
            wordVector[j] = syn0[j];
            len += syn0[j] * syn0[j];
        }
        len = std::sqrt(len);
        for (auto j = 0; j < layerSize; ++j) {
            wordVector[j] /= len;
        }
        header += layerSize * sizeof(float) + 1;
        shift += layerSize * sizeof(float) + 1;
        
        m_wordsMap[word] = wordVector;
    }
//    std::cout << m_wordsMap.size() << std::endl;
}

std::size_t word2vec_t::wordVectorSize() const {
    auto i = m_wordsMap.begin();
    if (i == m_wordsMap.end()) {
        return 0;
    }
    
    return i->second.size();
}

/*
#include <chrono>
std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
std::cout << "It took me " << time_span.count() << " seconds.";
std::cout << std::endl;
*/
bool word2vec_t::wordVector(const std::string &_word, std::shared_ptr<wordVector_t> &_wordVector) const {
    auto i = m_wordsMap.find(_word);
    if (i == m_wordsMap.end()) {
        return false;
    }
    
    _wordVector = std::make_shared<wordVector_t>(i->second);

    return true;
}

float word2vec_t::distance(const std::string &_what, const std::string &_with) const {
    auto i = m_wordsMap.find(_what);
    auto j = m_wordsMap.find(_with);
    if ((i == m_wordsMap.end()) || ((j == m_wordsMap.end()))) {
        return 0.0;
    }
    
    float ret = 0.0;
    for (std::size_t k = 0; k < i->second.size(); ++k) {
        ret += i->second[k] * j->second[k];
    }
    
    return std::sqrt(ret);
}
