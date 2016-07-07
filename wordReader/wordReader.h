//
//  wordReader.h
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef wordReader_h
#define wordReader_h

#include <string>

class baseMapper_t {
protected:
    const char *m_data;
    off_t m_size;
    
    baseMapper_t(const char *_data, off_t _size): m_data(_data), m_size(_size) {;}
    
public:
    virtual ~baseMapper_t() {;}
    
    inline off_t size() const {return m_size;}
    inline const char *data() const {return m_data;}
};

class stringMapper_t: public baseMapper_t {
public:
    stringMapper_t(const std::string &_source): baseMapper_t(_source.c_str(), _source.length()) {;}
    ~stringMapper_t() {;}
};

class fileMapper_t: public baseMapper_t {
private:
    std::string m_fileName;
    int m_fd;
    bool m_wrFlag;
    
public:
    fileMapper_t(const std::string &_fileName, bool _wrFlag = false, off_t _size = 0);
    ~fileMapper_t();
    
private:
    void open();
    void close();
    void resize();
};

template<class mapper_t>
class wordReader_t {
private:
    const mapper_t &m_mapper;
    uint16_t m_maxWordLen;
    off_t m_offset;
    std::string m_word;
    
public:
    wordReader_t(const mapper_t &_mapper, uint16_t _maxWordLen = 100): m_mapper(_mapper), m_maxWordLen(_maxWordLen),
                                                                       m_offset(0), m_word() {;}
    ~wordReader_t() {;}
    
    inline off_t size() const {return m_mapper.size();}
    inline bool setOffset(off_t _offset);
    inline bool eof() const {return m_offset >= m_mapper.size();}
    inline bool nextWord(std::string &_word);
};

template<class mapper_t>
bool wordReader_t<mapper_t>::setOffset(off_t _offset) {
    if (_offset < m_mapper.size()) {
        m_offset = _offset;
        
        return true;
    }
    
    return false;
}

template<class mapper_t>
bool wordReader_t<mapper_t>::nextWord(std::string &_word) {
    if (m_word.length() > 0) {
        _word = m_word;
        m_word.clear();
        return true;
    }
    for (auto i = m_offset; i < m_mapper.size(); ++i) {
        ++m_offset;
        char ch = m_mapper.data()[i];
        if (std::ispunct(ch) || std::isspace(ch)) {
            if (ch == '\n') {
                _word = "</s>";
                return true;
            }
            if (m_word.length() > 0) {
                _word = m_word;
                m_word.clear();
                return true;
            }
            continue;
        }
        if (m_word.length() < m_maxWordLen) {
            m_word += ch;
        }
    }
    if (m_word.length() > 0) {
        _word = m_word;
        return true;
    }
    
    return false;
}

#endif /* wordReader_h */
