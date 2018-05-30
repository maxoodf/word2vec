/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 18.03.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <iostream>

#include "word2vec.hpp"

int main(int argc, char * const *argv) {
    // First argument is a model file name
    if (argc != 2) {
        std::cerr << "Usage:" << std::endl
                  << argv[0] << " [word2vec_model_file_name]" << std::endl;
        return 1;
    }

    // load pre-trained model
    std::unique_ptr<w2v::w2vModel_t> w2vModel;
    try {
        w2vModel.reset(new w2v::w2vModel_t());
        if (!w2vModel->load(argv[1])) {
            throw std::runtime_error(w2vModel->errMsg());
        }
    } catch (const std::exception &_e) {
        std::cerr << _e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 2;
    }

    try {
        w2v::word2vec_t king(w2vModel, "king"); // vector of the word "king"
        w2v::word2vec_t man(w2vModel, "man"); // vector of the word "man"
        w2v::word2vec_t woman(w2vModel, "woman"); // vector of the word "woman"

        w2v::vector_t result = king - man + woman; // result vector

        // get nearest vectors and their words
        std::vector<std::pair<std::string, float>> nearest;
        w2vModel->nearest(result, nearest, 10);

        // output nearest words and distances from the result vector
        for (auto const &i:nearest) {
            // skip source words from the set
            if ((i.first == "king") || (i.first == "man") || (i.first == "woman")) {
                continue;
            }
            // output word and its distance from the result vector
            std::cout << i.first << ": " << i.second << std::endl;
        }
/*
 * The nearest word to the result vector is "queen", our model works well.
 * Output should looks like -
 * queen: 0.542737
 * princess: 0.499493
 * mother: 0.479788
 * daughter: 0.475785
 * her: 0.470077
 * monarch: 0.464647
 * infanta: 0.461744
 * husband: 0.460482
*/
    } catch (const std::exception &_e) {
        std::cerr << _e.what() << std::endl;
        return 3;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 3;
    }
    return 0;
}
