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

doc2vec_t::doc2vec_t(const word2vec_t &_word2vec, const std::string &_fileName, bool _loadData):
m_word2vec(_word2vec), m_docsMap(), m_fileName(_fileName) {
    if (_loadData) {
        load();
    }
}

doc2vec_t::~doc2vec_t() {
    save();
}

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
    if (len > 0.0) {
        len = std::sqrt(len);
        for (std::size_t i = 0; i < _docVector.size(); ++i) {
            _docVector[i] /= len;
        }
        return true;
    } else {
        return false;
    }
}

bool doc2vec_t::docVector(int64_t _id, docVector_t &_docVector) const {
    auto i = m_docsMap.find(_id);
    if (i == m_docsMap.end()) {
        return false;
    }
    
    _docVector = i->second;
    
    return true;
}

bool doc2vec_t::insert(int64_t _id, const docVector_t &_docVector) {
    if (m_docsMap.find(_id) != m_docsMap.end()) {
        return false;
    }
    
    m_docsMap[_id] = _docVector;
    
    return true;
}

bool doc2vec_t::insert(int64_t _id, const std::string &_docText) {
    docVector_t docVec;
    if (!docVector(_docText, docVec)) {
        return false;
    }
    
    return insert(_id, docVec);
}

bool doc2vec_t::nearest(int64_t _id, std::vector<int64_t> &_docIDs, float _distance, std::size_t _amount) const {
    if (m_docsMap.find(_id) == m_docsMap.end()) {
        return false;
    }
    
    docVector_t docVec;
    if (!docVector(_id, docVec)) {
        return false;
    }

    return nearest(docVec, _docIDs, _distance, _amount);
}

bool doc2vec_t::nearest(const docVector_t &_docVector, std::vector<int64_t> &_docIDs,
                        float _distance, std::size_t _amount) const {
    std::multimap<float, int64_t, std::greater<float>> tmpMap;
    for (auto i:m_docsMap) {
        float dist = distance(_docVector, i.second);
        if (dist >= _distance) {
            tmpMap.insert(std::make_pair(dist, i.first));
        }
    }
    
    _docIDs.clear();
    for (auto i:tmpMap) {
        if ((_amount > 0) && (_docIDs.size() == _amount)) {
            break;
        }
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

    if (ret > 0.0) {
        return std::sqrt(ret);
    } else {
        return 0.0;
    }
}

void doc2vec_t::save() const {
    off_t fileSize = sizeof(m_docsMap.size()) + sizeof(m_word2vec.wordVectorSize()) + //header
                     (sizeof(int64_t) + sizeof(word2vec_t::wordVectorValue_t) * m_word2vec.wordVectorSize()) *
                                                                                                m_docsMap.size();
    fileMapper_t fileMapper(m_fileName, true, fileSize);
    uint64_t shift = 0;
    const char *data = fileMapper.data();
    
    std::size_t mapSize = m_docsMap.size();
    memmove((void *) (data + shift), &mapSize, sizeof(mapSize));
    shift += sizeof(mapSize);

    std::size_t vecSize = m_word2vec.wordVectorSize();
    memmove((void *) (data + shift), &vecSize, sizeof(vecSize));
    shift += sizeof(vecSize);
    
    for (auto i:m_docsMap) {
        memmove((void *) (data + shift), &(i.first), sizeof(i.first));
        shift += sizeof(i.first);
        for (auto j:i.second) {
            memmove((void *) (data + shift), &j, sizeof(j));
            shift += sizeof(j);
        }
    }
}

void doc2vec_t::load() {
    fileMapper_t fileMapper(m_fileName);
    off_t fileSize = fileMapper.size();
    
    // header must be hear at least
    if (fileSize < sizeof(std::size_t) * 2) {
        throw std::runtime_error("wrong file format");
    }
    
    uint64_t shift = 0;
    const char *data = fileMapper.data();
    
    std::size_t mapSize = 0;
    memmove(&mapSize, data + shift, sizeof(mapSize));
    shift += sizeof(mapSize);
    
    std::size_t vecSize = m_word2vec.wordVectorSize();
    memmove(&vecSize, data + shift, sizeof(vecSize));
    shift += sizeof(vecSize);
    
    off_t expFileSize = sizeof(mapSize) + sizeof(vecSize) + //header
                        (sizeof(int64_t) + sizeof(word2vec_t::wordVectorValue_t) * vecSize) * mapSize; //records
    if (fileSize != expFileSize) {
        throw std::runtime_error("wrong file format");
    }

    docVector_t docVector(vecSize, 0.0);
    for (std::size_t i = 0; i < mapSize; ++i) {
        docsMap_t::key_type key = 0;
        memmove(&key, data + shift, sizeof(key));
        shift += sizeof(key);
        for (std::size_t j = 0; j < vecSize; ++j) {
            word2vec_t::wordVectorValue_t value = 0;
            memmove(&value, data + shift, sizeof(value));
            shift += sizeof(value);
            docVector[j] = value;
        }
        m_docsMap[key] = docVector;
    }
}
