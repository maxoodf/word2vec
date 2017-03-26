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
* `-e [chars]` or `--end-of-sentence [chars]` - end of sentence chars, default chars are ".\n?!". Optional parameter.
* `-d [chars]` or `--word-delimiter [chars]` - words delimiter chars, default chars are " \n,.-!?:;/\"#$%&'()\*+<=>@[]\\^\_\`{|}~\t\v\f\r". Note, end of sentence chars must be included in word delimiters. Optional parameter.
* `-v` or `--verbose` - show training process details, default is false. Optional parameter.

For example, train the model from corpus.txt file and save it to model.w2v. Use Skip-Gram & Negative Sampling, vector size 500, downsampling threshold 1e-5, 3 iterations, all other parameters by default: 
`./w2v_trainer -f ./corpus.txt -o ./model.w2v -g -n 10 -s 500 -l 1e-5 -i 3`

### Basic usage

### Implementation improvements vs original C code
- #### Negative sampling

- #### Hierarchical Softmax

- #### Subsampling (down-sampling)
[Mikolov et al](https://arxiv.org/pdf/1310.4546.pdf) used a simple subsampling approach: each word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1) in the training set is ***discarded*** with probability computed by the formula  ![P(w)](https://www.dropbox.com/s/ms8g7zz8ink2krm/f1.png?raw=1) where ![f(wi)](https://www.dropbox.com/s/mjnohff0ewdyb78/f8.png?raw=1) is the frequency of word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1) and ![t](https://www.dropbox.com/s/2pkwgism8101n3a/f10.png?raw=1) is a chosen threshold, typically around ![10e−5](https://www.dropbox.com/s/ugsghly2s3k4t9s/f11.png?raw=1).
But [the original C code](https://github.com/svn2github/word2vec) implements another equation, the probability of ***keeping*** the word: ![P(w)](https://www.dropbox.com/s/z1umpdl9h6qe559/f2.png?raw=1) where ![r(wi)](https://www.dropbox.com/s/sm6ag6nx6wq44oc/f3.png?raw=1) and ![T](https://www.dropbox.com/s/8jg6t3rvtjvqbis/f12.png?raw=1) is the total words in the corpus, so: ![P(w)](https://www.dropbox.com/s/sshui81t3q6xi28/f4.png?raw=1)  
**Probability of word keeping distribution:**
![Subsampling probability (keeping the word)](https://www.dropbox.com/s/fgjduwpjkvzi3a3/g1.png?raw=1)
We can see that probability of infrequent words keeping equals to 1 and it is not needed to compute probability for these words. We can find boundaries where do we need to compute a probability by solving the inequality: ![P(wi)>=1](https://www.dropbox.com/s/ghegeqzuo1ls7pq/f6.png?raw=1) than,
**probability computation is needed only for the following word frequencies:** ![f(wi)](https://www.dropbox.com/s/b5slcxb4fihh509/f7.png?raw=1)
- #### Vocabulary

- #### File operations

### Performance
