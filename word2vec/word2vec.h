//
//  word2vec.h
//  word2vecpp
//
//  Created by Max Fomichev on 19/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef word2vec_h
#define word2vec_h

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class word2vec_t {
public:
    using wordVectorValue_t = float;
    using wordVector_t = std::vector<wordVectorValue_t>;
    using wordsMap_t = std::unordered_map<std::string, wordVector_t>;
    
private:
    wordsMap_t m_wordsMap;
    
public:
    word2vec_t(const std::string &_fileName);
    virtual ~word2vec_t() {;}
    
    std::size_t wordVectorSize() const;
    bool wordVector(const std::string &_word, std::shared_ptr<wordVector_t> &_wordVector) const;
    wordVectorValue_t distance(const std::string &_what, const std::string &_with) const;
};

#endif /* word2vec_h */
