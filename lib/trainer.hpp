/**
 * @file
 * @brief trainer class of word2vec model
 * @author Max Fomichev
 * @date 20.12.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_TRAINER_H
#define WORD2VEC_TRAINER_H

#include <memory>
#include <vector>
#include <functional>

#include "word2vec.hpp"
#include "wordReader.hpp"
#include "vocabulary.hpp"
#include "trainThread.hpp"

namespace w2v {
    /**
     * @brief trainer class of word2vec model
     *
     * trainer class is responsible for train-specific data instantiation, train threads control and
     * train process itself.
    */
    class trainer_t {
    private:
        std::size_t m_matrixSize = 0;
        std::vector<std::unique_ptr<trainThread_t>> m_threads;

    public:
        /**
         * Constructs a trainer object
         * @param _trainSettings trainSattings object
         * @param _vocabulary vocabulary object
         * @param _fileMapper fileMapper object related to a train data set file
         * @param _progressCallback callback function to be called on each new 0.01% processed train data
        */
        trainer_t(const std::shared_ptr<trainSettings_t> &_trainSettings,
                  const std::shared_ptr<vocabulary_t> &_vocabulary,
                  const std::shared_ptr<fileMapper_t> &_fileMapper,
                  std::function<void(float, float)> _progressCallback);

        /**
         * Runs training process
         * @param[out] _trainMatrix train model matrix
        */
        void operator()(std::vector<float> &_trainMatrix) noexcept;
    };
}

#endif // WORD2VEC_TRAINER_H
