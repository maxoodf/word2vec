# **word2vec++**

### Introduction
***word2vec++*** is a [Distributed Representations of Words (word2vec)](https://arxiv.org/pdf/1310.4546.pdf) library and tools implementation.
- ***word2vec++*** code is simple and well documented. It is written in pure **C++11** from the scratch. It does not depend on external libraries (excepting STL).
- ***word2vec++*** is fast. It uses file mapping into memory for fast text corpus parsing / model training, std::unordered_map for vocabulary implementation, std::piecewise_linear_distribution for Negative Sampling implementation and C++11 random generators where random values are needed, Huffman encoding based on std::priority_queue for Hierarchical Softmax implementation, improved Subsampling and many more.
- ***word2vec++*** model files are binary compatible with [the original](https://github.com/svn2github/word2vec) model format.
- ***word2vec++*** train utility is more flexible than [the original](https://github.com/svn2github/word2vec) one. It supports all original settings plus stop-words, word delimiter chars set and end of sentence chars set.
- ***word2vec++*** is cross-platform. Some platform/compiler combinations which have been tested:
  - Linux/GCC 5.4+
  - Linux/Clang 3.4+
  - OSX/Clang 3.4+
  - FreeBSD/Clang 3.4+
  - Windows/MinGW (GCC 6.3)

### Building
You will need C++11 compatible compiler and cmake 3.1 or higher.
Execute the following commands:
1. `git clone https://github.com/maxoodf/word2vec.git word2vec++`
2. `cd word2vec++`
3. `mkdir build`
4. `cd build`
4. `cmake -DCMAKE_BUILD_TYPE=Release ../`
5. `make`
6. `cd ../`

On successful build you will find compiled tools in `./bin` directory, `libword2vec.a` in `./bin/lib` and examples in `./bin/examples`.

### Training the model
Training utility name is `w2v_trainer` and you can find it at the project's `bin` directory.
Execute `./w2v_trainer` without parameters to output a brief help information.
The following training parameters are available.
* `-f [file]` or `--train-file [file]` - filename of a train text corpus. You can use your own corpus or some other available to download, for example [English text corpus (2.3 billion words)](https://drive.google.com/file/d/0B1shHLc2QTzzRkxULXBIb0J3VTA/view?usp=sharing) or [Russian text corpus (0.5 billion words)](https://github.com/maxoodf/russian_news_corpus). Required parameter.
* `-o [file]` or `--model-file [file]` - filename of the resulting word vectors (model). This file will be created on successful training completion and it contains words and their vector representations. File format is binary compatible with the [the original](https://github.com/svn2github/word2vec) format. Required parameter.
* `-x [file]` or `--stop-words-file [file]` - filename of the stop-words set. These words will be excluded from training vocabulary. Stop-words are separated by any of word delimiter char (see below). Optional parameter.
* `-g` or `--with-skip-gram` - choose of the learning model. Here are two options - Continuous Bag of Words (CBOW) and Skip-Gram. CBOW is used by default. The CBOW architecture predicts the current word based on the context, and the Skip-gram predicts surrounding words given the current word. Optional parameter.
* `-h` or `--with-hs` - choose of the computationally efficient approximation. Here are two options - Negative Sampling (NS) and Hierarchical Softmax (HS). NS is used by default. Optional parameter.
* `-n [value]` or `--negative [value]` - number of negative examples (NS option), default value is 5. Values in the range 5–20 are useful for small training datasets, while for large datasets the value can be as small as 2–5. Optional parameter.
* `-s [value]` or `--size [value]` - words vector dimension, default value is 100. Large vectors are usually better, but it requires more training data. Optional parameter.
* `-w [value]` or `--window [value]` - nearby words frame or window, default value is 5. It defines how many words we will include in training of the word inside of corpus - [value] words behind and [value] words ahead ([value]\*2 in total). Optional parameter.
* `-l [value]` or `--sample [value]` - threshold for occurrence of words, default value is 1e-3. Those that appear with higher frequency in the training data will be randomly down-sampled. You can find more details in [Subsampling (down-sampling)](#subsampling-down-sampling) section. Optional parameter.
* `-a [value]` or `--alpha [value]` - starting learning rate, default value is 0.05. Optional parameter.
* `-i [value]` or `--iter [value]` - number of training iterations, default value is 5. More iterations makes a more precise model, but computational cost is linearly proportional to iterations. Optional parameter.
* `-t [value]` or `--threads [value]` -  number of training threads, default value is 12. Optional parameter.
* `-m [value]` or `--min-word-freq [value]` - exclude words that appear less than [value] times from vocabulary, default value is 5. Optional parameter.
* `-e [chars]` or `--end-of-sentence [chars]` - end of sentence (EOS) chars, default chars are ".\n?!". Original C code EOS chars are "\n". Optional parameter.
* `-d [chars]` or `--word-delimiter [chars]` - words delimiter (WD) chars, default chars are " \n,.-!?:;/\"#$%&'()\*+<=>@[]\\^\_\`{|}~\t\v\f\r". Note, end of sentence chars must be included in word delimiters. Original C code WD chars are " \t\n". Optional parameter.
* `-v` or `--verbose` - show training process details, default is false. Optional parameter.

For example, train the model from corpus.txt file and save it to model.w2v. Use Skip-Gram, Negative Sampling with 10 examples, vector size 500, downsampling threshold 1e-5, 3 iterations, all other parameters by default:  
`./w2v_trainer -f ./corpus.txt -o ./model.w2v -g -n 10 -s 500 -l 1e-5 -i 3`

### Basic usage
You can download one or more models (833MB each) trained on [11.8GB English texts corpus](https://drive.google.com/file/d/0B1shHLc2QTzzRkxULXBIb0J3VTA/view?usp=sharing):
- [CBOW, Hierarchical Softmax, vector size 500, window 10](https://drive.google.com/file/d/0B1shHLc2QTzzV1dhaVk1MUt2cmc/view?usp=sharing)
- [CBOW, Negative Sampling, vector size 500, window 10](https://drive.google.com/file/d/0B1shHLc2QTzzTVZESDFpQk5jNG8/view?usp=sharing)
- [Skip-Gram, Hierarchical Softmax, vector size 500, window 10](https://drive.google.com/file/d/0B1shHLc2QTzzZl9vSS1FOFh1N0k/view?usp=sharing)
- [Skip-Gram, Negative Sampling, vector size 500, window 10](https://drive.google.com/file/d/0B1shHLc2QTzzWFhpX2kwbWRkaWs/view?usp=sharing)

- #### Distance
`w2v_distance` utility from the project's `bin` directory. Usage: `w2v_distance [model_file_name]`.
This utility displays nearest words and vector distances to an entered word or a phrase.
- #### Analogy
`w2v_analogy` utility from the project's `bin` directory. Usage: `w2v_analogy [model_file_name]`.
This utility displays nearest word analogies and vector distances. For example, the nearest expected analogy for "man king woman" is "queen", ie if "man" is "king" what is the analogy for "woman": vector("king") - vector("man") + vector("woman") ~= vector("queen").
- #### Accuracy
`w2v_accuracy` utility from the project's `bin` directory. Usage: `w2v_accuracy [model_file_name] [analogies_file_name]`.
This is the model accuracy test utility. It works like `w2v_analogy` and compares a computed analogy result word with a given expected word.
`[analogies_file_name]` file format is the following:
each line of the file contains 4 words - `word1 word2 word3 word4` where `word1` is related to `word2` like "king" is "man" and `word3` is related to `word4` like "queen" is "woman" and both pairs have the same analogy. The utility finds nearest words for the result vector of vector(word2) - vector(word1) + vector(word3) and compares it with the vector(word4) for each line of the `[analogies_file_name]`. The model error is the middle square error of distances from the given `word4` and the computed word position.
- #### Examples
  - ###### [king - man + woman = queen](https://github.com/maxoodf/word2vec/blob/master/examples/word2vec/main.cpp)
  This is the simplest example of word vectors usage.

  - ###### [Nearest documents](https://github.com/maxoodf/word2vec/blob/master/examples/doc2vec/main.cpp)
  This is the simplest example of document vectors usage.

### Implementation improvements VS original C code
- #### Negative sampling
[Mikolov et al 2.2](https://arxiv.org/pdf/1310.4546.pdf) introduced a simplified Noise Contrastive Estimation called Negative sampling. The key feature of the Negative Sampling implementation is an array referred as the Unigram table. This is the lookup table of negative samples randomly selected during the training process. I found that the same probability distribution could be implemented with [std::piecewise_linear_distribution](http://www.cplusplus.com/reference/random/piecewise_linear_distribution/) and it outperforms the original implementation in more than 2 times.
<img src="https://www.dropbox.com/s/qps9rjbsq6zv32k/g2.png?raw=1" width="759">

- #### Subsampling (down-sampling)
[Mikolov et al 2.3](https://arxiv.org/pdf/1310.4546.pdf) used a simple subsampling approach: each word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1
  ) in the training set is ***discarded*** with probability computed by the formula  ![P(w)](https://www.dropbox.com/s/ms8g7zz8ink2krm/f1.png?raw=1) where ![f(wi)](https://www.dropbox.com/s/mjnohff0ewdyb78/f8.png?raw=1) is the frequency of word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1) and ![t](https://www.dropbox.com/s/2pkwgism8101n3a/f10.png?raw=1) is a chosen threshold, typically around ![10e−5](https://www.dropbox.com/s/ugsghly2s3k4t9s/f11.png?raw=1).
But [the original C code](https://github.com/svn2github/word2vec) implements another equation, the probability of ***keeping*** the word: ![P(w)](https://www.dropbox.com/s/z1umpdl9h6qe559/f2.png?raw=1) where ![r(wi)](https://www.dropbox.com/s/sm6ag6nx6wq44oc/f3.png?raw=1) and ![T](https://www.dropbox.com/s/8jg6t3rvtjvqbis/f12.png?raw=1) is the total words in the corpus, so: ![P(w)](https://www.dropbox.com/s/sshui81t3q6xi28/f4.png?raw=1)  
**Probability of word keeping distribution:**
<img src="https://www.dropbox.com/s/fgjduwpjkvzi3a3/g1.png?raw=1" width="659">  
We can see that probability of infrequent words keeping equals to 1 and it is not needed to compute probability for these words. We can find boundaries where do we need to compute a probability by solving the inequality: ![P(wi)>=1](https://www.dropbox.com/s/ghegeqzuo1ls7pq/f6.png?raw=1) than,
**probability computation is needed only for the following word frequencies:** ![f(wi)](https://www.dropbox.com/s/b5slcxb4fihh509/f7.png?raw=1)
- #### File operations
All file I/O operations in word2vec++ are mapped into memory. It's well known that sequential reading/writing is extremely fast on mapped files. For example, 900MB model file loading takes only 0.82 sec while the same operation takes 6.76 sec with the original code.  
<img src="https://www.dropbox.com/s/avsn72obtv1j4oc/c1.png?raw=1" width="400">
<img src="https://www.dropbox.com/s/e5qrh4gyryrwlhq/c2.png?raw=1" width="400">  
### Performance
The overall training performance, comparing to the original C code, is increased about 17% in average, depending on train settings.  
All tests are done with perf utility on Ubuntu 16.04 (linux 4.4.0), GCC 5.4.1, compiler optimization flags: -Ofast -march=native -funroll-loops -ftree-vectorize", [11.8GB English texts corpus](https://drive.google.com/file/d/0B1shHLc2QTzzRkxULXBIb0J3VTA/view?usp=sharing).  
<img src="https://www.dropbox.com/s/3aa7oyk5e3tufht/c3.png?raw=1" width="610">
