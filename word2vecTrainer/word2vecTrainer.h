//
//  word2vecTrainer.h
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#ifndef word2vecTrainer_h
#define word2vecTrainer_h

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "wordReader.h"

struct vocRec_t {
    uint32_t m_cn;
    std::string m_word;
    std::vector<int64_t> m_point;
    std::vector<int8_t> m_code;
    
    explicit vocRec_t(): m_cn(0), m_word(), m_point(), m_code() {;}
    vocRec_t(uint32_t _cn, const std::string &_word): m_cn(_cn), m_word(_word), m_point(), m_code() {;}
    vocRec_t(const vocRec_t &_other): m_cn(_other.m_cn), m_word(_other.m_word), m_point(_other.m_point),
    m_code(_other.m_code) {;}
    
    vocRec_t &operator=(const vocRec_t &_vocRec) {
        if (this != &_vocRec) {
            m_cn = _vocRec.m_cn;
            m_word = _vocRec.m_word;
            m_point = _vocRec.m_point;
            m_code = _vocRec.m_code;
        }
        return *this;
    };
    
    bool operator==(const vocRec_t &_with) {return m_cn == _with.m_cn;}
    // reverse ordering
    friend bool operator<(const vocRec_t &_what, const vocRec_t &_with) {return _what.m_cn > _with.m_cn;}
};

class word2vecTrainer_t {
public:
    struct freqIdx_t {
        uint32_t m_freq;
        std::size_t m_idx;
        
        freqIdx_t(): m_freq(0), m_idx(0) {;}
        freqIdx_t(uint32_t _freq, std::size_t _idx = 0): m_freq(_freq), m_idx(_idx) {;}
    };
    
    using wordsMap_t = std::unordered_map<std::string, freqIdx_t>;
    using voc_t = std::vector<vocRec_t>;
    
private:
    wordsMap_t m_wordsMap;
    voc_t m_voc;
    uint64_t m_trainWords;
    std::unique_ptr<fileMapper_t> m_fileMapper;
    bool m_stopWords;
    std::unordered_set<std::string> m_stopWordsSet;
    
public:
    word2vecTrainer_t(): m_wordsMap(), m_voc(), m_trainWords(0), m_fileMapper(), m_stopWords(false),
                         m_stopWordsSet() {;}
    ~word2vecTrainer_t() {;}
    
    void train();
    
private:
    inline void loadWords(const std::string &_fileName);
    inline void loadStopWords(const std::string &_fileName);
    inline void createBinaryTree();
};


class neuralNetwork_t {
private:
    std::vector<float> m_expTable;
    std::vector<std::size_t> m_unigramTable;
    std::vector<float> m_syn0;
    std::vector<float> m_syn1;
    std::vector<float> m_syn1neg;
    std::size_t m_vocSize;
    uint16_t m_layer1Size;
    bool m_cbow;
    bool m_hs;
    uint8_t m_negative;
    uint8_t m_threads;
    float m_alpha;
    float m_sample;
    uint8_t m_window;
    uint8_t m_iter;
    uint64_t m_wordCountActual;
    
public:
    neuralNetwork_t(std::size_t _vocSize, uint16_t _layer1Size, bool _cbow, bool _hs, uint8_t _negative,
                    uint8_t _threads, float _alpha, float _sample, uint8_t _window, uint8_t _iter);
    ~neuralNetwork_t();
    
    void train(uint64_t _trainWords, const word2vecTrainer_t::wordsMap_t &_wordsMap, const word2vecTrainer_t::voc_t &_voc,
               const fileMapper_t *_fileMapper);
    void saveModel(const std::string &_fileName, const word2vecTrainer_t::voc_t &_voc) const;
    
private:
    void trainThread(uint8_t _id, uint64_t _trainWords, const word2vecTrainer_t::wordsMap_t &_wordsMap,
                     const word2vecTrainer_t::voc_t &_voc, const fileMapper_t *_fileMapper);
    inline void initUnigramTable(const word2vecTrainer_t::voc_t &_voc);
    inline void softmax(const vocRec_t &_vocRec, std::vector<float> &_v1, std::vector<float> &_v2,
                        const float &_alpha, const std::size_t &_shift);
    inline void nSampling(std::vector<float> &_v1, std::vector<float> &_v2, uint64_t &_next_random,
                          const std::size_t &_idx, const float &_alpha, const std::size_t &_shift);
};

#endif /* wor2vecTrainer_h */
