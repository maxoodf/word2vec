//
//  word2vecTrainer.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//

#include <iostream>
#include <thread>
#include <cmath>
#include <algorithm>

#include "wordReader.h"
#include "word2vecTrainer.h"

static const uint16_t EXP_TABLE_SIZE = 1000;
static const uint8_t MAX_EXP = 6;
static const uint8_t MAX_STRING = 32;
static const uint8_t MAX_CODE_LENGTH = 40;
static const uint64_t UNIGRAM_TABLE_SIZE = 1e8;
static const uint16_t MAX_SENTENCE_LENGTH = 16384;

std::string trainFileName;
std::string modelFileName;
std::string stopWordsFileName;
uint16_t layer1Size = 100;
bool cbow = false;
bool hs = true;
uint8_t negative = 5;
uint8_t threads = 12;
float alpha = 0.05;
float sample = 1e-3;
uint8_t window = 5;
uint8_t iter = 5;
uint8_t minWordFreq = 5;
bool debug = false;

void word2vecTrainer_t::train() {
    if (stopWordsFileName.length() > 0) {
        m_stopWords = true;
        loadStopWords(stopWordsFileName);
    }
    loadWords(trainFileName);
    
    createBinaryTree();
    
    if (debug) {
        std::cout << "trainWords = " << m_trainWords << std::endl;
        std::cout << "Vocabrary size: " << m_voc.size() << std::endl;
        
        std::cout << "Training parameters: " << std::endl;
        std::cout << "trainFileName = " << trainFileName << std::endl;
        std::cout << "modelFileName = " << modelFileName << std::endl;
        std::cout << "stopWordsFileName = " << stopWordsFileName << std::endl;
        std::cout << "layer size = " << (int)layer1Size << std::endl;
        std::cout << "threads = " << (int)threads << std::endl;
        std::cout << "alpha = " << alpha << std::endl;
        std::cout << "sample = " << sample << std::endl;
        std::cout << "window = " << (int)window << std::endl;
        std::cout << "iter = " << (int)iter << std::endl;
        std::cout << "minWordFreq = " << (int)minWordFreq << std::endl;
        std::cout << "cbow = " << cbow << std::endl;
        std::cout << "hs = " << hs << std::endl;
        std::cout << "negative = " << (int)negative << std::endl;
    }
    
    neuralNetwork_t neuralNetwork(m_voc.size(), layer1Size, cbow, hs, negative, threads, alpha, sample, window, iter);
    neuralNetwork.train(m_trainWords, m_wordsMap, m_voc, m_fileMapper.get());
    
    neuralNetwork.saveModel(modelFileName, m_voc);
}

void word2vecTrainer_t::loadWords(const std::string &_fileName) {
    m_fileMapper = std::unique_ptr<fileMapper_t>(new fileMapper_t(_fileName));
    wordReader_t<fileMapper_t> wordReader(*(m_fileMapper.get()), MAX_STRING);
    
    std::string word;
    m_wordsMap["</s>"];
    while (wordReader.nextWord(word)) {
        if (!m_stopWords || (m_stopWordsSet.find(word) == m_stopWordsSet.end())) {
            m_wordsMap[word].m_freq++;
        }
    }
    
    wordsMap_t::const_iterator ciWM = m_wordsMap.find("</s>");
    m_voc.push_back(vocRec_t(ciWM->second.m_freq, ciWM->first));
    m_wordsMap.erase(ciWM);
    ciWM = m_wordsMap.begin();
    while (ciWM != m_wordsMap.end()) {
        //        std::cout << ciWM->first << ": " << ciWM->second.m_freq << std::endl;
        if (ciWM->second.m_freq < minWordFreq) {
            //            std::cout << ciWM->first << ": " << ciWM->second.m_freq << std::endl;
            m_wordsMap.erase(ciWM++);
        } else {
            m_voc.push_back(vocRec_t(ciWM->second.m_freq, ciWM->first));
            m_trainWords += ciWM->second.m_freq;
            ++ciWM;
        }
    }
    std::sort(++m_voc.begin(), m_voc.end());
    
    for (std::size_t i = 0; i < m_voc.size(); ++i) {
        m_wordsMap[m_voc[i].m_word].m_idx = i;
        //        std::cout << m_voc[i].m_word << ": " << m_voc[i].m_cn << std::endl;
    }
}

void word2vecTrainer_t::loadStopWords(const std::string &_fileName) {
    fileMapper_t fileMapper(_fileName);
    wordReader_t<fileMapper_t> wordReader(fileMapper, MAX_STRING);
    
    std::string word;
    while (wordReader.nextWord(word)) {
        if (word != "</s>") {
            m_stopWordsSet.insert(word);
        }
    }
}

// Create binary Huffman tree using the word counts
// Frequent words will have short uniqe binary codes
void word2vecTrainer_t::createBinaryTree() {
    std::vector<int64_t> point(MAX_CODE_LENGTH, 0);
    std::vector<int8_t> code(MAX_CODE_LENGTH, 0);
    
    std::vector<int64_t> count;
    std::vector<int64_t> binary(m_voc.size() * 2 + 1, 0);
    std::vector<int64_t> parentNode(m_voc.size() * 2 + 1, 0);
    
    for (auto i:m_voc) {
        count.push_back(i.m_cn);
    }
    
    for (auto i = m_voc.size(); i < m_voc.size() * 2; ++i) {
        count.push_back(1e15);
    }
    
    int64_t pos1 = m_voc.size() - 1;
    int64_t pos2 = m_voc.size();
    int64_t min1i = 0;
    int64_t min2i = 0;
    // Following algorithm constructs the Huffman tree by adding one node at a time
    for (std::size_t i = 0; i < m_voc.size() - 1; ++i) {
        // First, find two smallest nodes 'min1, min2'
        if (pos1 >= 0) {
            if (count[pos1] < count[pos2]) {
                min1i = pos1;
                pos1--;
            } else {
                min1i = pos2;
                pos2++;
            }
        } else {
            min1i = pos2;
            pos2++;
        }
        if (pos1 >= 0) {
            if (count[pos1] < count[pos2]) {
                min2i = pos1;
                pos1--;
            } else {
                min2i = pos2;
                pos2++;
            }
        } else {
            min2i = pos2;
            pos2++;
        }
        count[m_voc.size() + i] = count[min1i] + count[min2i];
        parentNode[min1i] = m_voc.size() + i;
        parentNode[min2i] = m_voc.size() + i;
        binary[min2i] = 1;
    }
    
    // Now assign binary code to each vocabulary word
    uint64_t i = 0;
    //    for (voc_t::iterator iv = m_voc.begin(); iv != m_voc.end(); ++iv) {
    for (auto &iv:m_voc) {
        auto b = i;
        uint64_t j = 0;
        while (true) {
            code[j] = binary[b];
            point[j] = b;
            j++;
            b = parentNode[b];
            if (b == m_voc.size() * 2 - 2) {
                break;
            }
        }
        
        iv.m_code.resize(j, 0);
        iv.m_point.resize(j + 1, 0);
        iv.m_point[0] = m_voc.size() - 2;
        for (std::size_t c = 0; c < j; c++) {
            iv.m_code[j - c - 1] = code[c];
            iv.m_point[j - c] = point[c] - m_voc.size();
        }
        ++i;
    }
}

neuralNetwork_t::neuralNetwork_t(std::size_t _vocSize, uint16_t _layer1Size, bool _cbow, bool _hs, uint8_t _negative,
                                 uint8_t _threads, float _alpha, float _sample, uint8_t _window, uint8_t _iter):
m_expTable(), m_unigramTable(), m_syn0(), m_syn1(), m_syn1neg(), m_vocSize(_vocSize),
m_layer1Size(_layer1Size), m_cbow(_cbow), m_hs(_hs), m_negative(_negative), m_threads(_threads),
m_alpha(_alpha), m_sample(_sample), m_window(_window), m_iter(_iter), m_wordCountActual(0) {
    
    for (auto i = 0; i < EXP_TABLE_SIZE; ++i) {
        // Precompute the exp() table
        float val = std::exp(((float) i / (float) EXP_TABLE_SIZE * 2 - 1) * MAX_EXP);
        // Precompute f(x) = x / (x + 1)
        m_expTable.push_back(val / (val + 1));
    }
    
    if (m_hs) {
        m_syn1.resize(m_vocSize * m_layer1Size, 0.0);
    }
    
    if (m_negative > 0) {
        m_syn1neg.resize(m_vocSize * m_layer1Size, 0.0);
    }
    
    uint64_t nextRandom = 1;
    for (std::size_t i = 0; i < m_vocSize * m_layer1Size; ++i) {
        nextRandom = nextRandom * (uint64_t) 25214903917 + 11;
        m_syn0.push_back((((nextRandom & 0xFFFF) / (float) 65536.0) - 0.5) / m_layer1Size);
    }
}

neuralNetwork_t::~neuralNetwork_t() {
}

void neuralNetwork_t::initUnigramTable(const word2vecTrainer_t::voc_t &_voc) {
    auto twp = 0;
    for (auto i:_voc) {
        twp += std::pow(i.m_cn, 0.75);
    }
    
    std::size_t i = 0;
    auto d1 = std::pow(_voc[i].m_cn, 0.75) / twp;
    for (std::size_t a = 0; a < UNIGRAM_TABLE_SIZE; ++a) {
        m_unigramTable.push_back(i);
        if (a / (double) UNIGRAM_TABLE_SIZE > d1) {
            i++;
            d1 += std::pow(_voc[i].m_cn, 0.75) / twp;
        }
        if (i >= _voc.size()) {
            i = _voc.size() - 1;
        }
    }
}

void neuralNetwork_t::train(uint64_t _trainWords, const word2vecTrainer_t::wordsMap_t &_wordsMap,
                            const word2vecTrainer_t::voc_t &_voc, const fileMapper_t *_fileMapper) {
    if (m_negative > 0) {
        initUnigramTable(_voc);
    }
    
    std::vector<std::shared_ptr<std::thread>> trainThreads;
    for (auto i = 0; i < m_threads; ++i) {
        std::shared_ptr<std::thread> thr(new std::thread(&neuralNetwork_t::trainThread, this, i, _trainWords,
                                                         std::ref(_wordsMap), std::ref(_voc), _fileMapper));
        trainThreads.push_back(thr);
    }
    for (auto i:trainThreads) {
        i->join();
    }
    if (debug) {
        std::cout << std::endl;
    }
}

void neuralNetwork_t::trainThread(uint8_t _id, uint64_t _trainWords, const word2vecTrainer_t::wordsMap_t &_wordsMap,
                                  const word2vecTrainer_t::voc_t &_voc, const fileMapper_t *_fileMapper) {
    uint16_t sentenceLength = 0;
    uint16_t sentencePosition = 0;
    uint64_t wordCount = 0;
    uint64_t lastWordCount = 0;
    std::size_t sen[MAX_SENTENCE_LENGTH + 1];
    sen[0] = -1;
    uint64_t nextRandom = 0;
    
    clock_t now;
    clock_t start = clock();
    
    auto local_iter = m_iter;
    auto alpha = m_alpha;
    
    std::vector<float> neu1(m_layer1Size, 0.0);
    std::vector<float> neu1e(m_layer1Size, 0.0);
    
    wordReader_t<fileMapper_t> wordReader(*_fileMapper, MAX_STRING);
    wordReader.setOffset(wordReader.size() / m_threads * _id);
    
    while (true) {
        if (wordCount - lastWordCount > 10000) {
            m_wordCountActual += wordCount - lastWordCount;
            lastWordCount = wordCount;
            if (debug) {
                now = clock();
                printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, alpha,
                       m_wordCountActual / (float)(m_iter * _trainWords + 1) * 100,
                       m_wordCountActual / ((float)(now - start + 1) / (float)CLOCKS_PER_SEC * 1000));
                fflush(stdout);
            }
            alpha = m_alpha * (1 - m_wordCountActual / (float)(m_iter * _trainWords + 1));
            if (alpha < m_alpha * 0.0001) {
                alpha = m_alpha * 0.0001;
            }
            //            std::cout << m_wordCountActual << " " << lastWordCount << " " << alpha << std::endl;
        }
        if (sentenceLength == 0) {
            std::string word;
            while (wordReader.nextWord(word)) {
                word2vecTrainer_t::wordsMap_t::const_iterator ciWM = _wordsMap.find(word);
                if (ciWM == _wordsMap.end()) {
                    continue;
                }
                wordCount++;
                std::size_t idx = ciWM->second.m_idx;
                if (idx == 0) {
                    break;
                }
                
                // The subsampling randomly discards frequent words while keeping the ranking same
                if (m_sample > 0) {
                    float ran = (std::sqrt(ciWM->second.m_freq / (m_sample * _trainWords)) + 1) *
                    (m_sample * _trainWords) / ciWM->second.m_freq;
                    nextRandom = nextRandom * (uint64_t) 25214903917 + 11;
                    if (ran < (nextRandom & 0xFFFF) / (float) 65536.0) {
                        continue;
                    }
                }
                sen[sentenceLength] = idx;
                sentenceLength++;
                if (sentenceLength >= MAX_SENTENCE_LENGTH) {
                    break;
                }
            }
            sentencePosition = 0;
        }
        
        if (wordReader.eof() || (wordCount > _trainWords / m_threads)) {
            m_wordCountActual += wordCount - lastWordCount;
            local_iter--;
            if (local_iter == 0) {
                break;
            }
            wordCount = 0;
            lastWordCount = 0;
            sentenceLength = 0;
            wordReader.setOffset(wordReader.size() / m_threads * _id);
            continue;
        }
        std::size_t idx = sen[sentencePosition];
//        if (idx == -1) {
//            continue;
//        }
        neu1.assign(m_layer1Size, 0.0);
        neu1e.assign(m_layer1Size, 0.0);
        
        nextRandom = nextRandom * (uint64_t) 25214903917 + 11;
        auto b = nextRandom % m_window;
        if (m_cbow) {  //train the cbow architecture
            // in -> hidden
            auto cw = 0;
            for (auto a = b; a < m_window * 2 + 1 - b; a++) {
                if (a != m_window) {
                    int64_t c = sentencePosition - m_window + a;
                    if ((c < 0) || (c >= sentenceLength)) {
                        continue;
                    }
                    std::size_t last_word = sen[c];
                    for (c = 0; c < m_layer1Size; c++) {
                        neu1[c] += m_syn0[c + last_word * m_layer1Size];
                    }
                    cw++;
                }
            }
            if (cw) {
                for (auto c = 0; c < m_layer1Size; c++) {
                    neu1[c] /= cw;
                }
                // HIERARCHICAL SOFTMAX
                if (m_hs) {
                    softmax(_voc[idx], neu1, neu1e, alpha, (std::size_t) 0);
                }
                // NEGATIVE SAMPLING
                if (m_negative > 0) {
                    nSampling(neu1, neu1e, nextRandom, idx, alpha, (std::size_t) 0);
                }
                // hidden -> in
                for (auto a = b; a < m_window * 2 + 1 - b; a++) {
                    if (a != m_window) {
                        int64_t c = sentencePosition - m_window + a;
                        if ((c < 0) || (c >= sentenceLength)) {
                            continue;
                        }
                        
                        std::size_t last_word = sen[c];
                        for (auto c = 0; c < m_layer1Size; c++) {
                            m_syn0[c + last_word * m_layer1Size] += neu1e[c];
                        }
                    }
                }
            }
        } else {  //train skip-gram
            for (auto a = b; a < m_window * 2 + 1 - b; a++) {
                if (a != m_window) {
                    int64_t c = sentencePosition - m_window + a;
                    if ((c < 0) || (c >= sentenceLength)) {
                        continue;
                    }
                    
                    std::size_t lastWord = sen[c];
                    std::size_t l1 = lastWord * m_layer1Size;
                    neu1e.assign(m_layer1Size, 0.0);
                    
                    // HIERARCHICAL SOFTMAX
                    if (m_hs) {
                        softmax(_voc[idx], m_syn0, neu1e, alpha, l1);
                    }
                    // NEGATIVE SAMPLING
                    if (m_negative > 0) {
                        nSampling(m_syn0, neu1e, nextRandom, idx, alpha, l1);
                    }
                    // Learn weights input -> hidden
                    for (auto i = 0; i < m_layer1Size; ++i) {
                        m_syn0[i + l1] += neu1e[i];
                    }
                }
            }
        }
        sentencePosition++;
        if (sentencePosition >= sentenceLength) {
            sentenceLength = 0;
            continue;
        }
    }
    return;
}

void neuralNetwork_t::softmax(const vocRec_t &_vocRec, std::vector<float> &_v1, std::vector<float> &_v2,
                              const float &_alpha, const std::size_t &_shift) {
    for (std::size_t d = 0; d < _vocRec.m_code.size(); d++) {
        float f = 0.0;
        int64_t l2 = _vocRec.m_point[d] * m_layer1Size;
        // Propagate hidden -> output
        for (auto c = 0; c < m_layer1Size; ++c) {
            f += _v1[c + _shift] * m_syn1[c + l2];
        }
        if (f <= -MAX_EXP) {
            continue;
        } else if (f >= MAX_EXP) {
            continue;
        } else {
            f = m_expTable[(std::size_t)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
        }
        // 'g' is the gradient multiplied by the learning rate
        float g = (1 - _vocRec.m_code[d] - f) * _alpha;
        // Propagate errors output -> hidden
        for (auto c = 0; c < m_layer1Size; c++) {
            _v2[c] += g * m_syn1[c + l2];
        }
        // Learn weights hidden -> output
        for (auto c = 0; c < m_layer1Size; c++) {
            m_syn1[c + l2] += g * _v1[c + _shift];
        }
    }
}

void neuralNetwork_t::nSampling(std::vector<float> &_v1, std::vector<float> &_v2, uint64_t &_next_random,
                                const std::size_t &_idx, const float &_alpha, const std::size_t &_shift) {
    std::size_t target = 0;
    int8_t label = 0;
    for (auto d = 0; d < m_negative + 1; d++) {
        if (d == 0) {
            target = _idx;
            label = 1;
        } else {
            _next_random = _next_random * (uint64_t) 25214903917 + 11;
            target = m_unigramTable[(_next_random >> 16) % m_unigramTable.size()];
            if (target == 0) {
                target = _next_random % (m_vocSize - 1) + 1;
            }
            if (target == _idx) {
                continue;
            }
            label = 0;
        }
        
        std::size_t l2 = target * m_layer1Size;
        float f = 0.0;
        for (auto c = 0; c < m_layer1Size; c++) {
            f += _v1[c + _shift] * m_syn1neg[c + l2];
        }
        
        float g = 0.0;
        if (f > MAX_EXP) {
            g = (label - 1) * _alpha;
        } else if (f < -MAX_EXP) {
            g = (label - 0) * _alpha;
        } else {
            g = (label - m_expTable[(std::size_t)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * _alpha;
        }
        
        for (auto c = 0; c < m_layer1Size; c++) {
            _v2[c] += g * m_syn1neg[c + l2];
        }
        
        for (auto c = 0; c < m_layer1Size; c++) {
            m_syn1neg[c + l2] += g * _v1[c + _shift];
        }
    }
}

void neuralNetwork_t::saveModel(const std::string &_fileName, const word2vecTrainer_t::voc_t &_voc) const {
    FILE *fo = fopen(_fileName.c_str(), "wb");
    
    fprintf(fo, "%ld %hu\n", _voc.size(), m_layer1Size);
    for (std::size_t a = 0; a < _voc.size(); ++a) {
        fprintf(fo, "%s ", _voc[a].m_word.c_str());
        for (auto b = 0; b < m_layer1Size; ++b) {
            fwrite(&(m_syn0[a * m_layer1Size + b]), sizeof(float), 1, fo);
        }
        fprintf(fo, "\n");
    }
    fclose(fo);
}
