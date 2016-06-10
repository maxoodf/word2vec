//
//  doc2vec.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#include <string.h>

#include <cmath>
#include <map>

#include "wordReader.h"
#include "doc2vec.h"

bool doc2vec_t::docVector(const std::string _docText, docVector_t &_docVector) const {
    stringMapper_t stringMapper(_docText);
    wordReader_t<stringMapper_t> wordReader(stringMapper);
    
    bool hasWords = false;
    std::string word;
    _docVector.resize(m_word2vec.wordVectorSize(), 0.0);
    while (wordReader.nextWord(word)) {
        std::shared_ptr<word2vec_t::wordVector_t> wordVector;
        if (m_word2vec.wordVector(word, wordVector) && word != "</s>") {
            for (std::size_t i = 0; i < wordVector->size(); ++i) {
                _docVector[i] += (*wordVector)[i];
            }
            hasWords = true;
        }
    }
    if (!hasWords) {
        return false;
    }
    
    float len = 0.0;
    for (auto i:_docVector) {
        len += i * i;
    }
    len = std::sqrt(len);
    for (std::size_t i = 0; i < _docVector.size(); ++i) {
        _docVector[i] /= len;
    }
    
    return true;
}

bool doc2vec_t::docVector(int64_t _id, docVector_t &_docVector) const {
    auto i = m_docsMap.find(_id);
    if (i == m_docsMap.end()) {
        return false;
    }
    
    _docVector = i->second;
    
    return true;
}

bool doc2vec_t::insert(int64_t _id, const std::string &_docText) {
    if (m_docsMap.find(_id) != m_docsMap.end()) {
        return false;
    }
    
    docVector_t docVec;
    if (!docVector(_docText, docVec)) {
        return false;
    }
    
    m_docsMap[_id] = docVec;
    
    return true;
}

bool doc2vec_t::nearest(int64_t _id, std::vector<int64_t> &_docIDs, std::size_t _amount, float _distance) const {
    if (m_docsMap.find(_id) == m_docsMap.end()) {
        return false;
    }
    
    docVector_t docVec;
    if (!docVector(_id, docVec)) {
        return false;
    }

    return nearest(docVec, _docIDs, _amount, _distance);
}

bool doc2vec_t::nearest(const docVector_t &_docVector, std::vector<int64_t> &_docIDs,
                        std::size_t _amount, float _distance) const {
    std::multimap<float, int64_t> tmpMap;
    for (auto i:m_docsMap) {
        float dist = distance(_docVector, i.second);
        if (dist >= _distance) {
            tmpMap.insert(std::make_pair(dist, i.first));
            if (tmpMap.size() > _amount) {
                tmpMap.erase(tmpMap.begin());
            }
        }
    }
    
    _docIDs.clear();
    for (auto i:tmpMap) {
        _docIDs.push_back(i.second);
    }
    
    return (_docIDs.size() > 0);
}

float doc2vec_t::distance(int64_t _what, int64_t _with) const {
    float ret = 0.0;
    auto i = m_docsMap.find(_what);
    auto j = m_docsMap.find(_with);
    if ((i == m_docsMap.end()) || (j == m_docsMap.end())) {
        return ret;
    }
    
    return distance(i->second, j->second);
}

float doc2vec_t::distance(const docVector_t &_what, const docVector_t &_with) const {
    float ret = 0.0;
    if ((_what.size() != _with.size()) || (_what.size() == 0)) {
        return ret;
    }
    
    for (std::size_t k = 0; k < _what.size(); ++k) {
        ret += _what[k] * _with[k];
    }
    
    return std::sqrt(ret);
}

bool doc2vec_t::save(const std::string &_fileName) const {
    FILE *fo = fopen(_fileName.c_str(), "wb");
    
    fprintf(fo, "%ld %ld\n", m_docsMap.size(), m_word2vec.wordVectorSize());
    for (auto i:m_docsMap) {
        fwrite(&(i.first), sizeof(i.first), 1, fo);
        for (auto j:i.second) {
            fwrite(&j, sizeof(j), 1, fo);
        }
        fprintf(fo, "\n");
    }
    fclose(fo);
    
    return true;
}

bool doc2vec_t::load(const std::string &_fileName) {
    fileMapper_t fileMapper(_fileName);
    uint64_t fileSize = fileMapper.size();
    
    uint64_t shift = 0;
    const char *header = fileMapper.data();
    
    const char *pos = static_cast<const char *>(memchr(header, ' ', fileSize));
    if (pos == NULL) {
        return false;
    }
    std::size_t mapSize = std::stoul(std::string(header, pos - header));
    shift += pos - header + 1;
    header = pos + 1;
    
    pos = static_cast<const char *>(memchr(header, '\n', fileSize - shift));
    if (pos == NULL) {
        return false;
    }
    uint16_t layerSize = std::stoi(std::string(header, pos - header));
    shift += pos - header + 1;
    header = pos + 1;
    
    //    std::cout << vocSize << " " << layerSize << std::endl;
    
    docVector_t docVector(layerSize, 0.0);
    for (std::size_t i = 0; i < mapSize; ++i) {
        const uint64_t *id = (const uint64_t *)header;
        shift += sizeof(uint64_t);
        header += sizeof(uint64_t);
        
        float len = 0.0;
        const float *syn0 = (const float*) header;
        for (auto j = 0; j < layerSize; ++j) {
            docVector[j] = syn0[j];
            len += syn0[j] * syn0[j];
        }
        len = std::sqrt(len);
        for (auto j = 0; j < layerSize; ++j) {
            docVector[j] /= len;
        }
        header += layerSize * sizeof(float) + 1;
        shift += layerSize * sizeof(float) + 1;
        
        m_docsMap[*id] = docVector;
    }
    //    std::cout << m_wordsMap.size() << std::endl;
    return true;
}
