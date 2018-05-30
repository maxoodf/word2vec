/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 16.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <getopt.h>

#include <iostream>
#include <iomanip>

#include "word2vec.hpp"

int main(int argc, char * const *argv) {
    if (argc != 2) {
        std::cerr << "Usage:" << std::endl
                  << argv[0] << " [word2vec_model_file_name]" << std::endl;
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

    while (true) {
        try {
            std::cout << "Enter 3 words (EXIT to break): ";
            std::string word1, word2, word3;
            std::cin >> word1;
            if (word1 == "EXIT") {
                break;
            }
            std::cin >> word2;
            std::cin >> word3;

            w2v::word2vec_t vec1(model, word1);
            w2v::word2vec_t vec2(model, word2);
            w2v::word2vec_t vec3(model, word3);
            w2v::vector_t vec = vec2 - vec1 + vec3;

            std::cout << std::right << std::setw(19) << "Word" << std::left << std::setw(9) << " Distance" << std::endl;
            std::cout << std::right << std::setw(28) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;

            std::vector<std::pair<std::string, float>> nearests;
            model->nearest(vec, nearests, 30);
            for (auto const &i:nearests) {
                if ((i.first == word1) || (i.first == word2) || (i.first == word3)) {
                    continue;
                }
                std::cout << std::right << std::setw(19) << i.first << " "
                          << std::left << std::setw(9) << i.second << std::endl;
            }
            std::cout << std::endl;
        } catch (const std::exception &_e) {
            std::cerr << _e.what() << std::endl;
        } catch (...) {
            std::cerr << "unknown error" << std::endl;
        }
    }

    return 0;
}
