/**
 * @file
 * @brief vocabulary class containing word map, words frequencies and word indexes
 * @author Max Fomichev
 * @date 16.12.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/
#ifndef WORD2VEC_VOCABULARY_H
#define WORD2VEC_VOCABULARY_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "word2vec.hpp"
#include "mapper.hpp"

namespace w2v {
    /**
     * @brief vocabulary class - implements fast access to a words storage with their data - index and frequency.
     *
     * Vocabulary contains parsed words with minimum defined frequency, excluding stop words defined in a text file.
     * Base word storage is the std::unordered_map object.
     *
    */
    class vocabulary_t final {
    public:
        /**
         * @brief wordData structure is a stored word parameters - index and frequency
        */
        struct wordData_t final {
            std::size_t index; ///< word index (more frequent words have lower index value)
            std::size_t frequency; ///< word frequency in a train data set

            /// Constructs an empty wordData object
            wordData_t() noexcept: index(0), frequency(0) {}
            /// Constructs a wordObject with the specified parameters
            wordData_t(std::size_t _index, std::size_t _frequency) noexcept:
                    index(_index), frequency(_frequency) {}
        };

    private:
        // word (key) with its index and frequency
        using wordMap_t = std::unordered_map<std::string, wordData_t>;

        std::size_t m_trainWords = 0;
        std::size_t m_totalWords = 0;

        wordMap_t m_words;

    public:
        /**
         * Constructs a vocabulary object from the specified files and parameters
         * @param _trainWordsMapper smart pointer to fileMapper object related to a train data set file
         * @param _stopWordsMapper smart pointer to fileMapper object related to a file with stop-words.
         * In case of unititialized pointer, _stopWordsMapper will be ignored.
         * @param _minFreq minimum word frequency to include into vocabulary
         * @param _progressCallback callback function to be called on each new 0.01% processed train data
         * @param _statsCallback callback function to be called on train data loaded event to pass vocabulary size,
         * train words and total words amounts.
        */
        vocabulary_t(std::shared_ptr<fileMapper_t> &_trainWordsMapper,
                     std::shared_ptr<fileMapper_t> &_stopWordsMapper,
                     const std::string &_wordDelimiterChars,
                     const std::string &_endOfSentenceChars,
                     uint16_t _minFreq,
                     w2vModel_t::vocabularyProgressCallback_t _progressCallback,
                     w2vModel_t::vocabularyStatsCallback_t _statsCallback) noexcept;

        /**
         * Requests a data (index, frequency, word) associated with the _word
         * @param[in] _word key value
         * @return pointer to a wordData object or nullptr if the word is not a member of vocabulary
        */
        inline const wordData_t *data(const std::string &_word) const noexcept {
            auto i = m_words.find(_word);
            if (i != m_words.end()) {
                return &(i->second);
            } else {
                return nullptr;
            }
        }

        /// @retrns vocabulary size
        inline std::size_t size() const noexcept {
            return m_words.size();
        }

        /// @returns total words amount parsed from a train data set
        inline std::size_t totalWords() const noexcept  {
            return m_totalWords;
        }

        /// @returns train words amount (totalWords - amount(stop words) - amount(words with low frequency))
        inline std::size_t trainWords() const noexcept  {
            return m_trainWords;
        }

        /**
         * Requests word frequencies
         * @param[out] _output - vector of word frequencies where vector indexes are word indexes and vector values
         * are word frequencies
        */
        inline void frequencies(std::vector<std::size_t> &_output) const noexcept {
            _output.resize(m_words.size());
            for (auto const &i:m_words) {
                _output[i.second.index] = i.second.frequency;
            }
        }

        /**
         * Requests words descending sorted by their frequencies
         * @param[out] _words vector of word descending sorted by their frequencies
        */
        inline void words(std::vector<std::string> &_words) const noexcept {
            _words.clear();
            std::vector<std::pair<std::size_t, std::string>> indexedWords;
            for (auto const &i:m_words) {
                indexedWords.emplace_back(std::pair<std::size_t, std::string>(i.second.index, i.first));
            }
            std::sort(indexedWords.begin(), indexedWords.end(), [](const std::pair<std::size_t, std::string> &_what,
                                                                   const std::pair<std::size_t, std::string> &_with) {
                return _what.first < _with.first;
            });
            for (auto const &i:indexedWords) {
                _words.push_back(i.second);
            }
        }
    };
}

#endif // WORD2VEC_VOCABULARY_H
