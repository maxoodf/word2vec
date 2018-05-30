/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 18.03.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <fstream>
#include <iostream>

#include "word2vec.hpp"


/*
 * Let's try to find news articles about Orly airport shooting accident.
 * There are 8 text samples in {project}/examples/doc2vec/texts folder.
 * We will load 7 of them to a doc2vec model and will use one article to find nearest articles from the model.
 */
void readFile(const std::string &_fileName, std::string &_data) {
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    ifs.open(_fileName.c_str());
    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    _data.resize(static_cast<std::size_t>(size), 0);
    ifs.seekg(0, ifs.beg);
    ifs.read(&_data[0], size);
    ifs.close();
}

int main(int argc, char * const *argv) {
    // Two arguments required - word2vec model file name (1) and path to sample text documents (2)
    if (argc != 3) {
        std::cerr << "Usage:" << std::endl
                  << argv[0] << " [word2vec_model_file_name] [path_to_sample_texts]" << std::endl;
        return 1;
    }

    // load wor2vec model
    std::unique_ptr<w2v::w2vModel_t> w2vModel;
    try {
        w2vModel.reset(new w2v::w2vModel_t());
        // load w2v model file
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

    // create doc2vec model
    w2v::d2vModel_t d2vModel(w2vModel->vectorSize());
    try {
        std::string fileText;
        {
            /* load bbc_brazil_meat.txt text
             * It's not about Orly shooting accident, but here are a lot of related words, such us -
             * police, investigation, raids, incidents etc.
            */
            readFile(std::string(argv[2]) + "/bbc_brazil_meat.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 1 to the model
            d2vModel.set(1, doc2vec);
        }
        {
            /* load bbc_india_cannabis.txt text
             * It's also not about Orly shooting accident, but it also contains some related words -
             * guardian and attacks.
            */
            readFile(std::string(argv[2]) + "/bbc_india_cannabis.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 2 to the model
            d2vModel.set(2, doc2vec);
        }
        {
            /* load bbc_orly_shooting.txt text
             * It's about Orly shooting accident
            */
            readFile(std::string(argv[2]) + "/bbc_orly_shooting.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 3 to the model
            d2vModel.set(3, doc2vec);
        }
        {
            /* load cnbc_orly_shooting.txt text
             * It's about Orly shooting accident
            */
            readFile(std::string(argv[2]) + "/cnbc_orly_shooting.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 4 to the model
            d2vModel.set(4, doc2vec);
        }
        {
            /* load cnn_formula1.txt text
             * It's about Finnish Formula 1 pilot. It's too far from Orly accident
            */
            readFile(std::string(argv[2]) + "/cnn_formula1.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 5 to the model
            d2vModel.set(5, doc2vec);
        }
        {
            /* load cnn_orly_shooting.txt text
             * It's about Orly shooting accident
            */
            readFile(std::string(argv[2]) + "/cnn_orly_shooting.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 6 to the model
            d2vModel.set(6, doc2vec);
        }
        {
            /* load nyt_orly_bombing.txt text
             * It's also about Orly accident. But it's another story.
            */
            readFile(std::string(argv[2]) + "/nyt_orly_bombing.txt", fileText);
            //  text to vector
            w2v::doc2vec_t doc2vec(w2vModel, fileText);
            // add vector with ID = 7 to the model
            d2vModel.set(7, doc2vec);
        }

        /* load independent_orly_shooting.txt text
         * We will try to find articles closest to this one.
        */
        readFile(std::string(argv[2]) + "/independent_orly_shooting.txt", fileText);
        //  text to vector
        w2v::doc2vec_t doc2vec(w2vModel, fileText);

        // get nearest article IDs from the model
        std::vector<std::pair<std::size_t, float>> nearest;
        d2vModel.nearest(doc2vec, nearest, d2vModel.modelSize());

        // output result set
        for (auto const &i:nearest) {
            std::cout << i.first << ": " << i.second << std::endl;
        }
/*
 * Output should looks like -
 * 4: 0.976313
 * 6: 0.971176
 * 3: 0.943542
 * 7: 0.850593
 * 1: 0.749066
 * 2: 0.724662
 * 5: 0.587743
 *
 * You can see that articles 4, 6 and 3 are very close to independent_orly_shooting.txt
 * Article 7 is not so close, it's also about Orly accident, but it was another accident.
 * From the other hand article 5 (Formula 1) is quite far away and it's true.
*/
        // finaly, save our doc2vec model
        if (!d2vModel.save("model.d2v")) {
            std::cerr << "Can not save model: " << d2vModel.errMsg() << std::endl;
        }
    } catch (const std::exception &_e) {
        std::cerr << _e.what() << std::endl;
        return 3;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 3;
    }
    return 0;
}
