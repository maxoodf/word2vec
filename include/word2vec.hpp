/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 15.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_WORD2VEC_HPP
#define WORD2VEC_WORD2VEC_HPP

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <functional>
#include <cmath>

namespace w2v {
    /**
     * @brief trainSettings structure holds all training parameters
     */
    struct trainSettings_t final {
        uint16_t minWordFreq = 5; ///< discard words that appear less than minWordFreq times
        uint16_t size = 100; ///< word vector size
        uint8_t window = 5; ///< skip length between words
        uint16_t expTableSize = 1000; ///< exp(x) / (exp(x) + 1) values lookup table size
        uint8_t expValueMax = 6; ///< max value in the lookup table
        float sample = 1e-3f; ///< threshold for occurrence of words
        bool withHS = false; ///< use hierarchical softmax instead of negative sampling
        uint8_t negative = 5; ///< negative examples number
        uint8_t threads = 12; ///< train threads number
        uint8_t iterations = 5; ///< train iterations
        float alpha = 0.05f; ///< starting learn rate
        bool withSG = false; ///< use Skip-Gram instead of CBOW
        std::string wordDelimiterChars = " \n,.-!?:;/\"#$%&'()*+<=>@[]\\^_`{|}~\t\v\f\r";
        std::string endOfSentenceChars = ".\n?!";
        trainSettings_t() = default;
    };

    /**
     * @brief base class of a vector itself and vector operations
     *
     * w2vBase class implements basic vector operations such us "+", "-", "=" and "=="
    */
    class vector_t: public std::vector<float> {
    public:
        vector_t(): std::vector<float>() {}
        /// Constructs an empty vector with size _size
        explicit vector_t(std::size_t _size): std::vector<float>(_size, 0.0f) {}

        /// @returns a copy of _from object
        vector_t &operator=(const vector_t &_from) {
            assert(empty() || (size() == _from.size()));

            if (this != &_from) {
                if (empty()) {
                    resize(_from.size(), 0);
                }
                std::copy(_from.begin(), _from.end(), begin());
            }
            return *this;
        }

        /// @returns summarized w2vBase object
        vector_t &operator+=(const vector_t &_from) {
            if (this != &_from) {
                assert(size() == _from.size());

                for (std::size_t i = 0; i <  size(); ++i) {
                    (*this)[i] += _from[i];
                }
                float med = 0.0f;
                for (auto const &i:(*this)) {
                    med += i * i;
                }
                if (med <= 0.0f) {
                    throw std::runtime_error("word2vec: can not create vector");
                }
                med = std::sqrt(med / this->size());
                for (auto &i:(*this)) {
                    i /= med;
                }
            }

            return *this;
        }

        /// @returns substracted w2vBase object
        vector_t &operator-=(const vector_t &_from) {
            if (this != &_from) {
                assert(size() == _from.size());

                for (std::size_t i = 0; i < size(); ++i) {
                    (*this)[i] -= _from[i];
                }
                float med = 0.0f;
                for (auto const &i:(*this)) {
                    med += i * i;
                }
                if (med <= 0.0f) {
                    throw std::runtime_error("word2vec: can not create vector");
                }
                med = std::sqrt(med / this->size());
                for (auto &i:(*this)) {
                    i /= med;
                }
            }

            return *this;
        }

        friend vector_t operator+(vector_t _what, const vector_t &_with) {
            _what += _with;
            return _what;
        }

        friend vector_t operator-(vector_t _what, const vector_t &_with) {
            _what -= _with;
            return _what;
        }
    };

    /**
     * @brief base class of a vectors model
     *
     * Model is a storage of pairs key&vector, it implements some basic functionality related to vectors storage -
     * model size, vector size, get vector by key, calculate distance between two vectors and find nearest vectors
     * to a specified vector
    */
    template <class key_t>
    class model_t {
    protected:
        using map_t = std::unordered_map<key_t, vector_t>;

        map_t m_map;
        uint16_t m_vectorSize = 0;
        std::size_t m_mapSize = 0;
        mutable std::string m_errMsg;

        const std::string wrongFormatErrMsg = "model: wrong model file format";

    private:
        struct nearestCmp_t final {
            inline bool operator()(const std::pair<key_t, float> &_left,
                                   const std::pair<key_t, float> &_right) const noexcept {
                return _left.second > _right.second;
            }
        };

    public:
        /// constructs a model
        model_t(): m_map(), m_errMsg() {}
        /// virtual destructor
        virtual ~model_t() = default;

        /// Direct access to the word-vector map
        const map_t &map() {return m_map;}

        /// pure virtual method to save model of a derived class
        virtual bool save(const std::string &_modelFile) const noexcept = 0;
        /// pure virtual method to load model of a derived class
        virtual bool load(const std::string &_modelFile) noexcept = 0;

        /**
         * Vector access by key value
         * @param _key key value uniquely identifying vector in model
         * @returns pointer to the vector or nullptr if no such key found
        */
        inline const vector_t *vector(const key_t &_key) const noexcept {
            auto const &i = m_map.find(_key);
            if (i != m_map.end()) {
                return &i->second;
            }

            return nullptr;
        }

        /**
         * Calculates distance between two vectors.
         * @param _what first vector
         * @param _whith second vector
         * @returns distance
        */
        inline float distance(const vector_t &_what, const vector_t &_with) const noexcept {
            assert(m_vectorSize == _what.size());
            assert(m_vectorSize == _with.size());

            float ret = 0.0f;
            for (uint16_t i = 0; i < m_vectorSize; ++i) {
                ret += _what[i] * _with[i];
            }
            if (ret > 0.0f) {
                return  std::sqrt(ret / m_vectorSize);
            }
            return 0.0f;
        }

        /**
         * Finds nearest vectors to a specified one
         * @param _vec find nearest vectors for this specified vector
         * @param _nearest storage of found nearest vectors ordered descending by distance to specified vector
         * @param _amount max. amount of nearest vectors
         * @param _minDistance min. distance between vectors
        */
        inline void nearest(const vector_t &_vec,
                            std::vector<std::pair<key_t, float>> &_nearest,
                            std::size_t _amount,
                            float _minDistance = 0.0f) const noexcept {
            assert(m_vectorSize == _vec.size());

            _nearest.clear();

            std::priority_queue<std::pair<key_t, float>,
                    std::vector<std::pair<key_t, float>>,
                    nearestCmp_t> nearestVecs;

            float entryLevel = 0.0f;
            for (auto const &i:m_map) {
                auto match = distance(_vec, i.second);
                if ((match > 0.9999f) || (match < _minDistance)) { // 1.0f is not guarantied
                    continue;
                }
                if (match > entryLevel) {
                    nearestVecs.emplace(std::pair<key_t, float>(i.first, match));
                    if (nearestVecs.size() > _amount) {
                        nearestVecs.pop();
                        entryLevel = nearestVecs.top().second;
                    }
                }
            }

            auto nSize = nearestVecs.size();
            _nearest.resize(nSize);
            for (auto j = nSize; j > 0; --j) {
                _nearest[j - 1] = nearestVecs.top();
                nearestVecs.pop();
            }
        }

        /// @returns vector size of model
        inline uint16_t vectorSize() const noexcept {return m_vectorSize;}
        /// @returns model size (number of stored vectors)
        inline std::size_t modelSize() const noexcept {return m_mapSize;}
        /// @returns error message
        inline std::string errMsg() const noexcept {return m_errMsg;}
    };

    /**
     * @brief storage model of pairs key&vector where key type is std::string (word)
     *
     * Model is derived from model_t class and implements save/load methods and train model method
    */
    class w2vModel_t: public model_t<std::string> {
    public:
        /// type of callback function to be called on train data file parsing progress events
        using vocabularyProgressCallback_t = std::function<void(float)>;
        /// type of callback function to be called on train data file parsed event
        using vocabularyStatsCallback_t = std::function<void(std::size_t, std::size_t, std::size_t)>;
        /// type of callback function to be called on training progress events
        using trainProgressCallback_t = std::function<void(float, float)>;

    public:
        /// Constructs w2vModel object
        w2vModel_t(): model_t<std::string>() {}

        /**
         * Trains model
         * @param _trainSettings trainSettings_t structure with training parameters
         * @param _trainFile file name of train corpus data
         * @param _stopWordsFile file name with stop words
         * @param _vocabularyProgressCallback callback function reporting train corpus data parsing progress,
         * nullptr if progress statistic is not needed
         * @param _vocabularyStatsCallback callback function reporting train corpus statistic,
         * nullptr if train data corpus statistic is not needed
         * @param _trainProgressCallback callback function reporting training progress,
         * nullptr if training progress statistic is not needed
         * @returns true on successful completion or false otherwise
        */
        bool train(const trainSettings_t &_trainSettings,
                   const std::string &_trainFile,
                   const std::string &_stopWordsFile,
                   vocabularyProgressCallback_t _vocabularyProgressCallback,
                   vocabularyStatsCallback_t _vocabularyStatsCallback,
                   trainProgressCallback_t _trainProgressCallback) noexcept;

        /// saves word vectors to file with _modelFile name
        bool save(const std::string &_modelFile) const noexcept override;
        /// loads word vectors from file with _modelFile name
        bool load(const std::string &_modelFile) noexcept override;
    };

    /**
     * @brief storage model of pairs key&vector where key type is std::size_t (document ID)
     *
     * Model is derived from model_t class and implements save/load methods
    */
    class d2vModel_t: public model_t<std::size_t> {
    public:
        explicit d2vModel_t(uint16_t _vectorSize): model_t<std::size_t>() {
            m_vectorSize = _vectorSize;
        }

        /// add/replace new _vector with unique _id to the model
        void set(std::size_t _id, const vector_t &_vector, bool _checkUnique = false) {
            if (_checkUnique) {
                for (auto const &i:m_map) {
                    auto match = distance(_vector, i.second);
                    if (match > 0.9999f) { // 1.0f is not guarantied
                        return;
                    }
                }
            }

            m_map[_id] = _vector;
            m_mapSize = m_map.size();
        }
        /// remove _vector with unique _id from the model
        void erase(std::size_t _id) {
            m_map.erase(_id);
            m_mapSize = m_map.size();
        }
        /// saves document vectors to file with _modelFile name
        bool save(const std::string &_modelFile) const noexcept override;
        /// loads document vectors from file with _modelFile name
        bool load(const std::string &_modelFile) noexcept override;
    };

    /**
     * @brief word2vec class implements vector representation of a word
     *
     * word2vec class is derived from w2vBase_t and inherits vector operations
    */
    class word2vec_t: public vector_t {
    public:
        /** Constructs an empty word2vec object
         * @param _model w2vModel_t type object of a pretrained model
         */
        explicit word2vec_t(const std::unique_ptr<w2vModel_t> &_model): vector_t(_model->vectorSize()) {}

        /** Constructs a word2vec object
         * @param _model w2vModel_t type object of a pretrained model
         * @param _word word to be converted to a vector
         */
        word2vec_t(const std::unique_ptr<w2vModel_t> &_model, const std::string &_word):
                vector_t(_model->vectorSize()) {
            auto i = _model->vector(_word);
            if (i != nullptr) {
                std::copy(i->begin(), i->end(), begin());
            }
        }
    };

    /**
     * @brief doc2vec class implements vector representation of a document
     *
     * doc2vec class is derived from w2vBase_t and inherits vector operations
    */
    class doc2vec_t: public vector_t {
    public:
        /** Constructs doc2vec object
         * @param _model w2vModel_t type object of a pretrained model
         * @param _doc text document to be converted to a vector
         */
        doc2vec_t(const std::unique_ptr<w2vModel_t> &_model,
                  const std::string &_doc,
                  const std::string &_wordDelimiterChars = " \n,.-!?:;/\"#$%&'()*+<=>@[]\\^_`{|}~\t\v\f\r");
    };

}
#endif // WORD2VEC_WORD2VEC_HPP
