//
//  main.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 19/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//
#include <getopt.h>

#include <string>
#include <iostream>

#include "wrErrors.h"
#include "word2vec.h"
#include "doc2vec.h"
#include "doc2vecTrainer.h"

static inline void usage(const char *_name) {
    std::cout   << _name << " [options]" << std::endl
    << "Options:" << std::endl
    << "  -f, --train-file <file>" << std::endl
        << "\tUse text data from <file> to train the model" << std::endl
    << "  -o, --model-file <name>" << std::endl
        << "\tUse <file> to save the resulting word vectors" << std::endl
    << "  -x, --stop-words-file <name>" << std::endl
        << "\tUse <file> to save the resulting word vectors" << std::endl
    << "  -s, --size <value>" << std::endl
        << "\tSet size of word vectors; default is 100" << std::endl
    << "  -w, --window <value>" << std::endl
        << "\tSet max skip length between words; default is 5" << std::endl
    << "  -l, --sample <value>" << std::endl
        << "\tSet threshold for occurrence of words. Those that appear with higher frequency in the" << std::endl
        << "\ttraining data will be randomly down-sampled; default is 1e-3, useful range is (0, 1e-5)" << std::endl
    << "  -h, --without-hs" << std::endl
        << "\tUse Hierarchical Softmax; default is true" << std::endl
    << "  -n, --negative <value>" << std::endl
        << "\tNumber of negative examples; default is 5, common values are 3 - 10 (0 = not used)" << std::endl
    << "  -t, --threads <value>" << std::endl
        << "\tUse <int> threads (default 12)" << std::endl
    << "  -i, --iter <value>" << std::endl
        << "\tRun more training iterations (default 5)" << std::endl
    << "  -m, --min-word-freq <value>" << std::endl
        << "\tThis will discard words that appear less than <int> times; default is 5" << std::endl
    << "  -a, --alpha <value>" << std::endl
        << "\tSet the starting learning rate; default is 0.05" << std::endl
    << "  -c, --cbow" << std::endl
        << "\tUse the continuous bag of words model instead of skip-gram model; default is false" << std::endl
    << "  -d, --debug " << std::endl
        << "\tSet the debug mode; default is false" << std::endl;
}

static struct option longopts[] = {
    {"train-file",      required_argument,  NULL,   'f' },
    {"model-file",      required_argument,  NULL,   'o' },
    {"stop-words-file", required_argument,  NULL,   'x' },
    {"size",            required_argument,  NULL,   's' },
    {"window",          required_argument,  NULL,   'w' },
    {"sample",          required_argument,  NULL,   'l' },
    {"without-hs",      no_argument,        NULL,   'h' },
    {"negative",        required_argument,  NULL,   'n' },
    {"threads",         required_argument,  NULL,   't' },
    {"iter",            required_argument,  NULL,   'i' },
    {"min-word-freq",   required_argument,  NULL,   'm' },
    {"alpha",           required_argument,  NULL,   'a' },
    {"cbow",            no_argument,        NULL,   'c' },
    {"debug",           no_argument,        NULL,   'd' },
    {NULL,              0,                  NULL,   0 }
};

std::string trainFileName;
/*
extern std::string modelFileName;
extern std::string stopWordsFileName;
extern uint16_t layer1Size;
extern bool cbow;
extern bool hs;
extern uint8_t negative;
extern uint8_t threads;
extern float alpha;
extern float sample;
extern uint8_t window;
extern uint8_t iter;
extern uint8_t minWordFreq;
extern bool debug;
*/
#include <chrono>
int main(int argc, char * const *argv) {

    int ch;
    while ((ch = getopt_long(argc, argv, "f:d", longopts, NULL)) != -1) {
        switch (ch) {
            case 'f':
                trainFileName = optarg;
                break;
/*
            case 'o':
                modelFileName = optarg;
                break;
            case 'x':
                stopWordsFileName = optarg;
                break;
            case 's':
                layer1Size = std::stoi(optarg);
                break;
            case 'w':
                window = std::stoi(optarg);
                break;
            case 'l':
                sample = std::stof(optarg);
                break;
            case 'h':
                hs = false;
                break;
            case 'n':
                negative = std::stoi(optarg);
                break;
            case 't':
                threads = std::stoi(optarg);
                break;
            case 'i':
                iter = std::stoi(optarg);
                break;
            case 'm':
                minWordFreq = std::stoi(optarg);
                break;
            case 'a':
                alpha = std::stof(optarg);
                break;
            case 'c':
                cbow = true;
                break;
*/
            case 'd':
//                debug = true;
                break;
            case ':':
            case '?':
            default:
                usage(argv[0]);
                return 1;
        }
    }

//    if (!(trainFileName.size() > 0) || !(modelFileName.size() > 0)) {
//        usage(argv[0]);
//        return 1;
//    }

    try {
//        word2vecTrainer_t word2vecTrainer;
//        word2vecTrainer.train();
        {
            word2vec_t word2vec("./model.w2v");
            doc2vecTrainer_t doc2vecTrainer(word2vec, "./model.d2v");
            doc2vecTrainer.train(trainFileName);
        }
/*
        doc2vec_t doc2vec(word2vec);
        doc2vec.load("./model.d2v");
        
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        std::vector<uint64_t> docIDs;
        if (doc2vec.nearest(231049, docIDs, 30, 0.955)) {
            for (auto i:docIDs) {
                std::cout << i << std::endl;
            }
        }
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "It took me " << time_span.count() << " seconds." << std::endl;
*/
    } catch (const errorWR_t &_err) {
        std::cerr << _err.err() << std::endl;
    } catch (const std::exception &_err) {
        std::cerr << _err.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
    }
    
    return 0;
}
