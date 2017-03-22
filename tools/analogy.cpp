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

#include "word2vec.h"

int main(int argc, char * const *argv) {
    if (argc != 2) {
        return 1;
    }

    std::unique_ptr<w2v::w2vModel_t> model;
    try {
        model.reset(new w2v::w2vModel_t());
        model->load(argv[1]);
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
            std::string word;
            std::cin >> word;
            if (word == "EXIT") {
                break;
            }
            w2v::word2vec_t vec1(model.get(), word);
            std::cin >> word;
            w2v::word2vec_t vec2(model.get(), word);
            std::cin >> word;
            w2v::word2vec_t vec3(model.get(), word);

            vec2 -= vec1;
            vec2 += vec3;

            std::cout << std::right << std::setw(19) << "Word" << std::left << std::setw(9) << " Distance" << std::endl;
            std::cout << std::right << std::setw(28) << std::setfill('-') << "-" << std::setfill(' ') << std::endl;

            std::vector<std::pair<std::string, float>> nearests;
            model->nearest(vec2, nearests, 30);
            for (auto const &i:nearests) {
                std::cout << std::right << std::setw(19) << i.first << " " << std::left << std::setw(9) << i.second << std::endl;
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
