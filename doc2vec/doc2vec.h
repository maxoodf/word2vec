//
//  doc2vec.h
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef doc2vec_h
#define doc2vec_h

#include "word2vec.h"

class doc2vec_t {
public:
    using docVector_t = word2vec_t::wordVector_t;
    using docsMap_t = std::unordered_map<int64_t, word2vec_t::wordVector_t>;
    
private:
    const word2vec_t &m_word2vec;
    docsMap_t m_docsMap;
    std::string m_fileName;
    
public:
    doc2vec_t(const word2vec_t &_word2vec, const std::string &_fileName, bool _loadData = false);
    virtual ~doc2vec_t();
    
    bool docVector(const std::string &_docText, docVector_t &_docVector) const;
    bool docVector(int64_t _id, docVector_t &_docVector) const;

    bool insert(int64_t _id, const docVector_t &_docVector);
    bool insert(int64_t _id, const std::string &_docText);

    // Find nearest documents to document _id. Nearest documents are documents with distance() more than _distance.
    // Return only _amount of nearest documents or all of them if _amount == 0;
    bool nearest(int64_t _id, std::vector<int64_t> &_docIDs, float _distance = 0.95, std::size_t _amount = 30) const;
    bool nearest(const docVector_t &_docVector, std::vector<int64_t> &_docIDs,
                 float _distance = 0.95, std::size_t _amount = 30) const;
    float distance(int64_t _what, int64_t _with) const;
    float distance(const docVector_t &_what, const docVector_t &_with) const;
    
private:
    void save() const;
    void load();
};


#endif /* doc2vec_h */
