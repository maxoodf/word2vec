/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 15.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include "word2vec.hpp"
#include "wordReader.hpp"
#include "vocabulary.hpp"
#include "trainer.hpp"

namespace w2v {
    bool w2vModel_t::train(const trainSettings_t &_trainSettings,
                           const std::string &_trainFile,
                           const std::string &_stopWordsFile,
                           vocabularyProgressCallback_t _vocabularyProgressCallback,
                           vocabularyStatsCallback_t _vocabularyStatsCallback,
                           trainProgressCallback_t _trainProgressCallback) noexcept {
        try {
            // map train data set file to memory
            std::shared_ptr<fileMapper_t> trainWordsMapper(new fileMapper_t(_trainFile));
            // map stop-words file to memory
            std::shared_ptr<fileMapper_t> stopWordsMapper;
            if (!_stopWordsFile.empty()) {
                stopWordsMapper.reset(new fileMapper_t(_stopWordsFile));
            }

            // build vocabulary, skip stop-words and words with frequency < minWordFreq
            std::shared_ptr<vocabulary_t> vocabulary(new vocabulary_t(trainWordsMapper,
                                                                      stopWordsMapper,
                                                                      _trainSettings.wordDelimiterChars,
                                                                      _trainSettings.endOfSentenceChars,
                                                                      _trainSettings.minWordFreq,
                                                                      _vocabularyProgressCallback,
                                                                      _vocabularyStatsCallback));
            // key words descending ordered by their indexes
            std::vector<std::string> words;
            vocabulary->words(words);
            m_vectorSize = _trainSettings.size;
            m_mapSize = vocabulary->size();

            // train model
            std::vector<float> _trainMatrix;
            trainer_t(std::make_shared<trainSettings_t>(_trainSettings),
                      vocabulary,
                      trainWordsMapper,
                      _trainProgressCallback)(_trainMatrix);

            std::size_t wordIndex = 0;
            for (auto const &i:words) {
                auto &v = m_map[i];
                v.resize(m_vectorSize);
                std::copy(&_trainMatrix[wordIndex * m_vectorSize],
                          &_trainMatrix[(wordIndex + 1) * m_vectorSize],
                          &v[0]);
                wordIndex++;
            }

            return true;
        } catch (const std::exception &_e) {
            m_errMsg = _e.what();
        } catch (...) {
            m_errMsg = "unknown error";
        }

        return false;
    }

    bool w2vModel_t::save(const std::string &_modelFile) const noexcept {
        try {
            // save trained data in original word2vec format
            // file header
            std::string fileHeader = std::to_string(m_mapSize)
                                     + " "
                                     + std::to_string(m_vectorSize)
                                     + "\n";
            // calc output size
            // header size
            auto outputSize = static_cast<off_t>(fileHeader.length() * sizeof(char));
            for (auto const &i:m_map) {
                // size of (word + space char + vector size + size of cartridge return char)
                outputSize += (i.first.length() + 2) * sizeof(char) + m_vectorSize * sizeof(float);
            }
            // write data to the file
            fileMapper_t output(_modelFile, true, outputSize);
            char sp = ' ';
            char cr = '\n';
            off_t offset = 0;
            // write file header
            std::memcpy(reinterpret_cast<void *>(output.data() + offset),
                        fileHeader.data(), fileHeader.length() * sizeof(char));
            offset += fileHeader.length() * sizeof(char);

            // write words and their vectors
            for (auto const &i:m_map) {
                std::memcpy(reinterpret_cast<void *>(output.data() + offset),
                            i.first.data(), i.first.length() * sizeof(char));
                offset += i.first.length() * sizeof(char);
                std::memcpy(reinterpret_cast<void *>(output.data() + offset), &sp, sizeof(char));
                offset += sizeof(char);

                auto shift = m_vectorSize * sizeof(float);
                std::memcpy(reinterpret_cast<void *>(output.data() + offset), i.second.data(), shift);
                offset += shift;

                std::memcpy(reinterpret_cast<void *>(output.data() + offset), &cr, sizeof(char));
                offset += sizeof(char);
            }

            return true;
        } catch (const std::exception &_e) {
            m_errMsg = _e.what();
        } catch (...) {
            m_errMsg = "unknown error";
        }

        return false;
    }

    bool w2vModel_t::load(const std::string &_modelFile) noexcept {
        try {
            m_map.clear();

            // map model file, exception will be thrown on empty file
            fileMapper_t input(_modelFile);

            // parse header
            off_t offset = 0;
            // get words number
            std::string nwStr;
            char ch = 0;
            while ((ch = (*(input.data() + offset))) != ' ') {
                nwStr += ch;
                if (++offset >= input.size()) {
                    throw std::runtime_error(wrongFormatErrMsg);
                }
            }

            // get vector size
            offset++; // skip ' ' char
            std::string vsStr;
            while ((ch = (*(input.data() + offset))) != '\n') {
                vsStr += ch;
                if (++offset >= input.size()) {
                    throw std::runtime_error(wrongFormatErrMsg);
                }
            }

            try {
                m_mapSize = static_cast<std::size_t>(std::stoll(nwStr));
                m_vectorSize = static_cast<uint16_t>(std::stoi(vsStr));
            } catch (...) {
                throw std::runtime_error(wrongFormatErrMsg);
            }

            // get pairs of word and vector
            offset++; // skip last '\n' char
            std::string word;
            for (std::size_t i = 0; i < m_mapSize; ++i) {
                // get word
                word.clear();
                while ((ch = (*(input.data() + offset))) != ' ') {
                    if (ch != '\n') {
                        word += ch;
                    }
                    // move to the next char and check boundaries
                    if (++offset >= input.size()) {
                        throw std::runtime_error(wrongFormatErrMsg);
                    }
                }

                // skip last ' ' char and check boundaries
                if (static_cast<off_t>(++offset + m_vectorSize * sizeof(float)) > input.size()) {
                    throw std::runtime_error(wrongFormatErrMsg);
                }

                // get word's vector
                auto &v = m_map[word];
                v.resize(m_vectorSize);
                std::memcpy(v.data(), input.data() + offset, m_vectorSize * sizeof(float));
                offset += m_vectorSize * sizeof(float); // vector size

                // normalize vector
                float med = 0.0f;
                for (auto const &j:v) {
                    med += j * j;
                }
                if (med <= 0.0f) {
                    throw std::runtime_error("failed to normalize vectors");
                }
                med = std::sqrt(med / v.size());
                for (auto &j:v) {
                    j /= med;
                }
            }

            return true;
        } catch (const std::exception &_e) {
            m_errMsg = _e.what();
        } catch (...) {
            m_errMsg = "model: unknown error";
        }

        return false;
    }

    bool d2vModel_t::save(const std::string &_modelFile) const noexcept {
        try {
            auto msSize = sizeof(m_mapSize);
            auto vsSize = sizeof(m_vectorSize);
            off_t fileSize = msSize + vsSize  // header size
                                   + (sizeof(std::size_t)
                                      + m_vectorSize * sizeof(float)) * m_mapSize; // record size
            // write data to the file
            fileMapper_t output(_modelFile, true, fileSize);
            off_t offset = 0;
            std::memcpy(output.data() + offset, &m_mapSize, msSize);
            offset += msSize;
            std::memcpy(output.data() + offset, &m_vectorSize, vsSize);
            offset += vsSize;
            auto idSize = sizeof(std::size_t);
            auto elmSize = sizeof(float);
            for (const auto &i:m_map) {
                std::memcpy(output.data() + offset, &(i.first), idSize);
                offset += idSize;
                for (const auto &j:i.second) {
                    std::memcpy(output.data() + offset, &j, elmSize);
                    offset += elmSize;
                }
            }

            return true;
        } catch (const std::exception &_e) {
            m_errMsg = _e.what();
        } catch (...) {
            m_errMsg = "model: unknown error";
        }

        return false;
    }

    bool d2vModel_t::load(const std::string &_modelFile) noexcept {
        try {
            m_map.clear();

            // map model file
            fileMapper_t input(_modelFile);

            auto msSize = sizeof(m_mapSize);
            auto vsSize = sizeof(m_vectorSize);
            if (static_cast<off_t>(msSize + vsSize) > input.size()) {
                throw std::runtime_error(wrongFormatErrMsg);
            }
            off_t offset = 0;
            std::memcpy(&m_mapSize, input.data() + offset, msSize);
            offset += msSize;
            std::memcpy(&m_vectorSize, input.data() + offset, vsSize);
            offset += vsSize;

            auto idSize = sizeof(std::size_t);
            auto elmSize = sizeof(float);
            if (static_cast<off_t>(msSize + vsSize + (idSize + elmSize * m_vectorSize) * m_mapSize) != input.size()) {
                throw std::runtime_error(wrongFormatErrMsg);
            }

            for (std::size_t i = 0; i < m_mapSize; ++i) {
                std::size_t id = 0;
                std::memcpy(&id, input.data() + offset, idSize);
                offset += idSize;
                auto &v = m_map[id];
                v.resize(m_vectorSize);
                std::memcpy(v.data(), input.data() + offset, m_vectorSize * sizeof(float));
                offset += m_vectorSize * sizeof(float);
            }

            return true;
        } catch (const std::exception &_e) {
            m_errMsg = _e.what();
        } catch (...) {
            m_errMsg = "model: unknown error";
        }

        return false;
    }

    doc2vec_t::doc2vec_t(const std::unique_ptr<w2vModel_t> &_model,
                         const std::string &_doc,
                         const std::string &_wordDelimiterChars): vector_t(_model->vectorSize()) {
        stringMapper_t stringMapper(_doc);
        wordReader_t<stringMapper_t> wordReader(stringMapper, _wordDelimiterChars, "");
        std::string word;
        while(wordReader.nextWord(word)) {
            if (word.empty()) {
                continue;
            }
            auto next = _model->vector(word);
            if (next == nullptr) {
                continue;
            }
            for (uint16_t i = 0; i < _model->vectorSize(); ++i) {
                (*this)[i] += (*next)[i];
            }
        }
        float med = 0.0f;
        for (auto const &i:(*this)) {
            med += i * i;
        }
        if (med <= 0.0) {
            throw std::runtime_error("doc2vec: can not create vector");
        }
        med = std::sqrt(med / this->size());
        for (auto &i:(*this)) {
            i /= med;
        }
    }
}
