/**
 * @file
 * @brief linear subintervals random distribution class
 * @author Max Fomichev
 * @date 02.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <vector>
#include <cmath>

#include "nsDistribution.hpp"

namespace w2v {
    nsDistribution_t::nsDistribution_t(const std::vector<std::size_t> &_input): m_nsDistribution() {
        std::vector<std::size_t> intervals;
        std::vector<std::size_t> weights;
        std::size_t prvFreq = 0;

        for (std::size_t i = 1; i < _input.size(); ++i) {
            float rms = std::sqrt((prvFreq * prvFreq + _input[i] * _input[i]) / 2.0f);
            if ((_input[i] < rms / 1.3f) || (_input[i] > rms * 1.3f)) {
                intervals.push_back(i);
                weights.push_back(static_cast<std::size_t>(std::pow(_input[i], 0.75)));
                prvFreq = _input[i];
            }
        }

        m_nsDistribution.reset(new std::piecewise_linear_distribution<float>(intervals.begin(),
                                                                             intervals.end(),
                                                                             weights.begin()));
    }
}
