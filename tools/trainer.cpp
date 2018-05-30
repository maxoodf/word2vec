/**
 * @file
 * @brief
 * @author Max Fomichev
 * @date 15.02.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <getopt.h>

#include <iostream>
#include <iomanip>

#include "word2vec.hpp"

static void usage(const char *_name) {
    std::cout
            << _name << " [options]" << std::endl
            << "Options:" << std::endl
            << "  -f, --train-file <file>" << std::endl
            << "\tUse text data from <file> to train the model" << std::endl
            << "  -o, --model-file <name>" << std::endl
            << "\tUse <file> to save the resulting word vectors" << std::endl
            << "  -x, --stop-words-file <name>" << std::endl
            << "\tUse <file> with words to be excluded from training" << std::endl
            << "  -s, --size <value>" << std::endl
            << "\tSet size of word vectors; default is 100" << std::endl
            << "  -w, --window <value>" << std::endl
            << "\tSet max skip length between words; default is 5" << std::endl
            << "  -l, --sample <value>" << std::endl
            << "\tSet threshold for occurrence of words. Those that appear with higher frequency in the" << std::endl
            << "\ttraining data will be randomly down-sampled; default is 1e-3, useful range is (0, 1e-5)" << std::endl
            << "  -h, --with-hs" << std::endl
            << "\tUse Hierarchical Softmax instead of default Negative Sampling" << std::endl
            << "  -n, --negative <value>" << std::endl
            << "\tNumber of negative examples; default is 5, common values are 3 - 10" << std::endl
            << "  -t, --threads <value>" << std::endl
            << "\tUse <int> threads (default 12)" << std::endl
            << "  -i, --iter <value>" << std::endl
            << "\tRun more training iterations (default 5)" << std::endl
            << "  -m, --min-word-freq <value>" << std::endl
            << "\tThis will discard words that appear less than <int> times; default is 5" << std::endl
            << "  -a, --alpha <value>" << std::endl
            << "\tSet the starting learning rate; default is 0.05" << std::endl
            << "  -g, --with-skip-gram" << std::endl
            << "\tUse skip-gram model instead of the default continuous bag of words model" << std::endl
            << "  -d, --word-delimiter <chars>" << std::endl
            << "\tSet the word delimiter chars; default is \" \\n,.-!?:;/\\\"#$%&'()*+<=>@[]\\\\^_`{|}~\\t\\v\\f\\r\"" << std::endl
            << "  -e, --end-of-sentence <chars>" << std::endl
            << "\tSet the end of sentence chars; default is \".\\n?!\"" << std::endl
            << "  -v, --verbose " << std::endl
            << "\tShow training process details; default is false" << std::endl;
}

static struct option longopts[] = {
        {"train-file",      required_argument,  nullptr,   'f' },
        {"model-file",      required_argument,  nullptr,   'o' },
        {"stop-words-file", required_argument,  nullptr,   'x' },
        {"size",            required_argument,  nullptr,   's' },
        {"window",          required_argument,  nullptr,   'w' },
        {"sample",          required_argument,  nullptr,   'l' },
        {"with-hs",         no_argument,        nullptr,   'h' },
        {"negative",        required_argument,  nullptr,   'n' },
        {"threads",         required_argument,  nullptr,   't' },
        {"iter",            required_argument,  nullptr,   'i' },
        {"min-word-freq",   required_argument,  nullptr,   'm' },
        {"alpha",           required_argument,  nullptr,   'a' },
        {"with-skip-gram",  no_argument,        nullptr,   'g' },
        {"word-delimiters", required_argument,  nullptr,   'd' },
        {"end-of-sentence", required_argument,  nullptr,   'e' },
        {"verbose",         no_argument,        nullptr,   'v' },
        { nullptr, 0, nullptr, 0 }
};

int main(int argc, char * const *argv) {
    std::string trainFile;
    std::string modelFile;
    std::string stopWordsFile;
    bool verbose = false;
    w2v::trainSettings_t trainSettings;

    int ch = 0;
    while ((ch = getopt_long(argc, argv, "f:o:x:s:w:l:hn:t:i:m:a:gd:e:v?", longopts, nullptr)) != -1) {
        switch (ch) {
            case 'f':
                trainFile = optarg;
                break;
            case 'o':
                modelFile = optarg;
                break;
            case 'x':
                stopWordsFile = optarg;
                break;
            case 's':
                trainSettings.size = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'w':
                trainSettings.window = static_cast<uint8_t>(std::stoi(optarg));
                break;
            case 'l':
                trainSettings.sample = std::stof(optarg);
                break;
            case 'h':
                trainSettings.withHS = true;
                break;
            case 'n':
                trainSettings.negative = static_cast<uint8_t>(std::stoi(optarg));
                break;
            case 't':
                trainSettings.threads = static_cast<uint8_t>(std::stoi(optarg));
                break;
            case 'i':
                trainSettings.iterations = static_cast<uint8_t>(std::stoi(optarg));
                break;
            case 'm':
                trainSettings.minWordFreq = static_cast<uint16_t>(std::stoi(optarg));
                break;
            case 'a':
                trainSettings.alpha = std::stof(optarg);
                break;
            case 'g':
                trainSettings.withSG = true;
                break;
            case 'd':
                trainSettings.wordDelimiterChars = optarg;
                break;
            case 'e':
                trainSettings.endOfSentenceChars = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case ':':
            case '?':
            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (trainFile.empty() || modelFile.empty()) {
        usage(argv[0]);
        return 1;
    }

    if (verbose) {
        std::cout << "Train data file: " << trainFile << std::endl;
        std::cout << "Output model file: " << modelFile << std::endl;
        std::cout << "Stop-words file: " << stopWordsFile << std::endl;
        std::cout << "Training model: " << (trainSettings.withSG?"Skip-Gram":"CBOW") << std::endl;
        std::cout << "Sample approximation method: ";
        if (trainSettings.withHS) {
            std::cout << "Hierarchical softmax" << std::endl;
        } else {
            std::cout << "Negative sampling with number of negative examples = "
                      << static_cast<int>(trainSettings.negative) << std::endl;
        }
        std::cout << "Number of training threads: " << static_cast<int>(trainSettings.threads) << std::endl;
        std::cout << "Number of training iterations: " << static_cast<int>(trainSettings.iterations) << std::endl;
        std::cout << "Min word frequency: " << static_cast<int>(trainSettings.minWordFreq) << std::endl;
        std::cout << "Vector size: " << static_cast<int>(trainSettings.size) << std::endl;
        std::cout << "Max skip length: " << static_cast<int>(trainSettings.window) << std::endl;
        std::cout << "Threshold for occurrence of words: " << trainSettings.sample << std::endl;
        std::cout << "Starting learning rate: " << trainSettings.alpha << std::endl;
        std::cout << std::endl << std::flush;
    }

    w2v::w2vModel_t model;
    bool trained;
    if (verbose) {
        trained = model.train(trainSettings, trainFile, stopWordsFile,
                              [] (float _percent) {
                                  std::cout << "\rParsing train data... "
                                            << std::fixed << std::setprecision(2)
                                            << _percent << "%" << std::flush;
                              },
                              [] (std::size_t _vocWords, std::size_t _trainWords, std::size_t _totalWords) {
                                  std::cout << std::endl
                                            << "Vocabulary size: " << _vocWords << std::endl
                                            << "Train words: " << _trainWords << std::endl
                                            << "Total words: " << _totalWords << std::endl
                                            << std::endl;
                              },
                              [] (float _alpha, float _percent) {
                                  std::cout << '\r'
                                            << "alpha: "
                                            << std::fixed << std::setprecision(6)
                                            << _alpha
                                            << ", progress: "
                                            << std::fixed << std::setprecision(2)
                                            << _percent << "%"
                                            << std::flush;
                              }
        );
        std::cout << std::endl;
    } else {
        trained = model.train(trainSettings, trainFile, stopWordsFile, nullptr, nullptr, nullptr);
    }
    if (!trained) {
        std::cerr << "Training failed: " << model.errMsg() << std::endl;
        return 2;
    }

    if (!model.save(modelFile)) {
        std::cerr << "Model file saving failed: " << model.errMsg() << std::endl;
        return 3;
    }

    return 0;
}
