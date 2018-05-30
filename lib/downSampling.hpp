/**
 * @file
 * @brief frequent words down-sampling class
 * @author Max Fomichev
 * @date 08.03.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/
#ifndef WORD2VEC_DOWNSAMPLING_H
#define WORD2VEC_DOWNSAMPLING_H

#include <random>

namespace w2v {
    /**
     * @brief downSampling class - randomly down-sampling frequent words
     *
     * Randomly discard a frequent word from a training sentence
    */
    class downSampling_t {
    private:
        const float m_sample;
        const std::size_t m_trainWords;
        const std::size_t m_unfrequentSince;
        mutable std::uniform_real_distribution<float> m_freqWordsDistribution;

    public:
        /**
         * Constructs a downSampling object
         * @param _sample defines boundary of frequent words, small values (1e-5) make high boundary while
         * bigger values (1e-3) make low boundary
         * @param _trainWords defines total train words in a corpus
         */
        downSampling_t(float _sample, std::size_t _trainWords) :
                m_sample(_sample), m_trainWords(_trainWords),
                m_unfrequentSince(
                        static_cast<std::size_t>((m_sample / (1.5f - 0.5f * std::sqrt(5.0f))) * m_trainWords)),
                m_freqWordsDistribution(0.0f, 1.0f) {
        }

        /**
         * Generates a random decision to discard a word with _wordFreq frequency from a train sentence
         * @param _wordFreq word frequency
         * @param _randomGenerator random generator object instantiated outside of the downSampling object
         * @returns skip (true) or include (false) word into a training sentence
         */
        inline bool operator()(std::size_t _wordFreq, std::mt19937_64 &_randomGenerator) const noexcept {
            if (_wordFreq > m_unfrequentSince) {
                float z = ((float) _wordFreq) / m_trainWords;
                float dist = (std::sqrt(z / m_sample) + 1) * m_sample / z;
                auto ret = dist < m_freqWordsDistribution(_randomGenerator);
                return ret;
            }

            return false;
        }
    };
}

#endif //WORD2VEC_DOWNSAMPLING_H
