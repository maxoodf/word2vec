/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 16.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <getopt.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "word2vec.hpp"

int main(int argc, char * const *argv) {
    if (argc != 3) {
        std::cerr << "Usage:" << std::endl
                  << argv[0] << " [word2vec_model_file_name] [analogies_test_set_file_name]" << std::endl;
        return 1;
    }

    std::unique_ptr<w2v::w2vModel_t> model;
    try {
        model.reset(new w2v::w2vModel_t());
        if (!model->load(argv[1])) {
            throw std::runtime_error(model->errMsg());
        }
    } catch (const std::exception &_e) {
        std::cerr << _e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 2;
    }

    std::ifstream ifs;
    try {
//        ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        ifs.exceptions(std::ifstream::failbit);
        ifs.open(argv[2]);
    } catch (...) {
        std::cerr << "Can not open file: " << argv[2] << std::endl;
        return 2;
    }

    float modelAccuracy = 0.0f;
    float sectionAccuracy = 0.0f;
    std::size_t testSets = 0;
    std::size_t sectionSets = 0;
    std::string word1, word2, word3, word4;
    while (!ifs.eof()) {
        std::size_t idx = 0;
        std::size_t pos = model->modelSize();
        ifs >> word1;
        if (word1.empty()) {
            continue;
        }
        if (word1 == ":") {
            if (sectionAccuracy > 0.0f) {
                sectionAccuracy = std::sqrt(sectionAccuracy / sectionSets);
                std::cout << "section accuracy: " << sectionAccuracy << std::endl;
                sectionAccuracy = 0.0f;
                sectionSets = 0;
            }
            char sectionName[1024];
            ifs.getline(sectionName, 1023);
            std::cout << sectionName << std::endl;
            continue;
        }
        ifs >> word2 >> word3 >> word4;
        if (word2.empty() || word3.empty() || word4.empty()) {
            continue;
        }
        std::transform(word1.begin(), word1.end(), word1.begin(), tolower);
        std::transform(word2.begin(), word2.end(), word2.begin(), tolower);
        std::transform(word3.begin(), word3.end(), word3.begin(), tolower);
        std::transform(word4.begin(), word4.end(), word4.begin(), tolower);
        try {
            w2v::word2vec_t vec(model, word2);
            vec -= w2v::word2vec_t(model, word1);
            vec += w2v::word2vec_t(model, word3);

            std::vector<std::pair<std::string, float>> nearests;
            model->nearest(vec, nearests, model->modelSize());
            for (auto const &i:nearests) {
                if ((i.first == word1) || (i.first == word2) || (i.first == word3)) {
                    continue;
                }
                if (i.first == word4) {
                    pos = idx;
                    break;
                }
                idx++;
            }
        } catch(...) {}
        float accuracy = 1.0f - static_cast<float>(pos * pos)
                                / static_cast<float>(model->modelSize() * model->modelSize());
        modelAccuracy += accuracy * accuracy;
        sectionAccuracy += accuracy * accuracy;
        testSets++;
        sectionSets++;
    }
    ifs.close();

    sectionAccuracy = std::sqrt(sectionAccuracy / sectionSets);
    std::cout << "section accuracy: " << sectionAccuracy << std::endl;

    modelAccuracy = std::sqrt(modelAccuracy / testSets);
    std::cout << "Model accuracy: " << modelAccuracy << std::endl << std::endl;

    return 0;
}
