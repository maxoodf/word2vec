//
//  doc2vecTrainer.hpp
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef doc2vecTrainer_h
#define doc2vecTrainer_h

#include "wordReader.h"
#include "doc2vec.h"

class doc2vecTrainer_t {
private:
    const word2vec_t &m_word2vec;
    std::unique_ptr<fileMapper_t> m_fileMapper;
    doc2vec_t m_doc2vec;

public:
    doc2vecTrainer_t(const word2vec_t &_word2vec);
    ~doc2vecTrainer_t();
    
    void train(const std::string &_fileName);
    void saveModel(const std::string &_fileName);
    
private:
    
};

#endif /* doc2vecTrainer_h */
