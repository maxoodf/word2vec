//
//  doc2vecTrainer.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#include "doc2vecTrainer.h"

static const uint8_t MAX_STRING = 100;

doc2vecTrainer_t::doc2vecTrainer_t(const word2vec_t &_word2vec, const std::string &_fileName):
m_word2vec(_word2vec),
m_fileMapper(),
m_doc2vec(m_word2vec, _fileName) {
}

doc2vecTrainer_t::~doc2vecTrainer_t() {
}

void doc2vecTrainer_t::train(const std::string &_fileName) {
    m_fileMapper = std::unique_ptr<fileMapper_t>(new fileMapper_t(_fileName));
    wordReader_t<fileMapper_t> wordReader(*(m_fileMapper.get()), MAX_STRING);
    
    uint64_t id = 1;
    std::string doc;
    std::string word;
    while (wordReader.nextWord(word)) {
        if (word == "</s>") {
            m_doc2vec.insert(id, doc);
            doc.clear();
            id++;
        } else {
            doc += word + ' ';
        }
    }
}
