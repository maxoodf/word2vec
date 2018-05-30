/**
 * @file
 * @brief wordReader class - fast text parsing
 * @author Max Fomichev
 * @date 19.04.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_WORDREADER_H
#define WORD2VEC_WORDREADER_H

#include <string>
#include <cstring>

#include "mapper.hpp"

namespace w2v {
    /**
     * @brief Text parser (word by word)
     *
     * wordReader class is a word by word parser of a file mapped into memory by mapper_t derived class object.
     * It makes easy to parse a file like memory allocated char array without any read/write calls etc)
    */
    template <class dataMapper_t>
    class wordReader_t final {
    private:
        const dataMapper_t &m_mapper; // reference to mapper_t derived class object
        std::string m_wordDelimiterChars;
        std::string m_endOfSentenceChars;
        const uint16_t m_maxWordLen; // max word length
        off_t m_offset; // current offset
        const off_t m_startFrom; // start from position
        const off_t m_stopAt; // stop at position
        std::string m_word; // current word buffer
        std::size_t m_wordPos = 0; // position in the current word buffer
        bool m_prvEOS = false; // is the previous char a sentence delimiter char?

    public:
        /**
         * Constructs a wordReader of a memory mapped file (_mapper object)
         * @param _mapper mapper_t derived class object that provides read access to a mapped memory
         * @param _offset start parsing from this offset position
         * @param _stopAt stop parsing at this position
         * @param _maxWordLen max length of a parsing word
         * @throws std::range_error In case of _offset or/and _stopAt are out of bounds
        */
        wordReader_t(const dataMapper_t &_mapper,
                     std::string _wordDelimiterChars,
                     std::string _endOfSentenceChars,
                     off_t _offset = 0, off_t _stopAt = 0, uint16_t _maxWordLen = 100):
                m_mapper(_mapper),
                m_wordDelimiterChars(std::move(_wordDelimiterChars)),
                m_endOfSentenceChars(std::move(_endOfSentenceChars)),
                m_maxWordLen(_maxWordLen), m_offset(_offset),
                m_startFrom(m_offset), m_stopAt((_stopAt == 0)?_mapper.size() - 1:_stopAt),
                m_word(m_maxWordLen, 0) {

            if (m_stopAt >= m_mapper.size()) {
                throw std::range_error("wordReader: bounds are out of the file size");
            }
            if (m_offset > m_stopAt) {
                throw std::range_error("wordReader: offset is out of the bounds");
            }
        }

        // copying prohibited
        wordReader_t(const wordReader_t &) = delete;
        void operator=(const wordReader_t &) = delete;

        /// @returns current offset
        inline off_t offset() const noexcept {return m_offset;}

        /// Resets parser state, start parsing from the begining
        inline void reset() noexcept {
            m_offset = m_startFrom;
            m_wordPos = 0;
            m_prvEOS = false;
        }

        /**
         * Reads next word
         * @param[out] _word  string where the next parsed word to be stored. Empty string means end of sentence.
         * @returns true if word is succesfuly parsed, false in case of EOF or end of parsing block reached (_stopAt).
        */
        inline bool nextWord(std::string &_word) noexcept {
            while (m_offset <= m_stopAt) {
                char ch = m_mapper.data()[m_offset++];
                if (m_wordDelimiterChars.find(ch) != std::string::npos) { // is it a word/sentence delimiter?
                    if (m_endOfSentenceChars.find(ch) != std::string::npos) { // is it the end of sentence (EOS)?
                        if (m_wordPos > 0) { // is here any buffered word? if yes - return this word and move back
                            m_offset--;
                            m_prvEOS = false;
                            break;
                        } else {
                            if (!m_prvEOS) { // Do not return repeated EOS, return only the first occurrence.
                                _word.clear();
                                m_prvEOS = true;
                                return true;
                            } else {
                                continue; // skip this EOS
                            }
                        }
                    }
                    if (m_wordPos > 0) { // it is a word delimiter, is here any buffered word?
                        m_prvEOS = false;
                        break;
                    } else {
                        continue; // skip repeated word delimiters
                    }
                }
                if (m_wordPos < m_maxWordLen) { // check bounds
                    m_word[m_wordPos++] = ch; // it's next char of buffered word
                }
            }
            if (m_wordPos > 0) { // return buffered word
                try {
                    _word.resize(m_wordPos);
                    std::copy(m_word.data(), m_word.data() + m_wordPos, &_word[0]);
                } catch (...) { // bad_alloc
                    return false;
                }
                m_wordPos = 0;
                return true;
            }

            return false; // eof or end of the requested block
        }
    };
}

#endif // WORD2VEC_WORDREADER_H
