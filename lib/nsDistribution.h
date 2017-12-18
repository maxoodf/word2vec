/**
 * @file
 * @brief linear subintervals random distribution class
 * @author Max Fomichev
 * @date 02.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_NSDISTRIBUTION_H
#define WORD2VEC_NSDISTRIBUTION_H

#include <memory>
#include <random>

namespace w2v {
    /**
     * @brief nsDistribution class - linear subintervals random distribution
     *
     * Generates a random value based on a sequence of subintervals bounds with different probability densities.
    */
    class nsDistribution_t final {
    private:
        std::unique_ptr<std::piecewise_linear_distribution<float>> m_nsDistribution;

    public:
        /**
         * Constructs a nsDistribution object with approximated probability densities powered 0.75
         * @param _input vector of probability densities for their indexes
         */
        explicit nsDistribution_t(const std::vector<std::size_t> &_input);

        /**
         * Generates a random value inside of subintervals bounds
         * @param _randomGenerator random generator object instantiated outside of the nsDistribution object
         * @returns a random value
         */
        inline std::size_t operator()(std::mt19937_64 &_randomGenerator) const noexcept {
            return static_cast<std::size_t>((*m_nsDistribution)(_randomGenerator));
        }
    };
}

#endif // WORD2VEC_NSDISTRIBUTION_H
