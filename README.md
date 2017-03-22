# word2vec++

## Introduction
word2vec++ is a [Distributed Representations of Words (word2vec)](https://arxiv.org/pdf/1310.4546.pdf) library and tools implementation.
- word2vec++ code is simple and well documented. It is written in pure C++11 from the scratch. It does not depend on external libraries (excepting STL).
- word2vec++ is fast. It uses file mapping into memory for fast text corpus parsing / model training, std::unordered_map for vocabulary implementation, std::piecewise_linear_distribution for Negative Sampling implementation and C++11 random generators where random values are needed, Huffman encoding based on std::priority_queue for Hierarchical Softmax implementation, improved Subsampling and many more.
- word2vec++ model files are binary compatible with [the original](https://github.com/svn2github/word2vec) model format.
- word2vec++ train utility is more flexible than [the original](https://github.com/svn2github/word2vec) one. It supports all original settings plus stop-words, word delimiter chars set and end of sentence chars set.
- word2vec++ is cross-platform. Some platform/compiler combinations which have been tested:
  - Linux/GCC 5.4+
  - Linux/Clang 3.4+
  - OSX/Clang 3.4+
  - FreeBSD/Clang 3.4+
  - Windows/MinGW (GCC 6.3)

## Building
You will need C++11 compatible compiler and cmake 3.1 or higher.
Execute the following commands:
1. `git clone https://github.com/maxoodf/word2vec.git word2vec++`
2. `cd word2vec++`
3. `mkdir build`
4. `cmake -DCMAKE_BUILD_TYPE=Release ../`
5. `make`
6. `cd ../`

On successful build you will find compiled tools in `./bin` directory, `libword2vec.a` in `./bin/lib` and examples in `./bin/examples`.

## Training

## Basic usage

## Implementation improvements vs original C code
- ### Subsampling (down-sampling)
[Mikolov et al](https://arxiv.org/pdf/1310.4546.pdf) used a simple subsampling approach: each word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1) in the training set is ***discarded*** with probability computed by the formula  ![P(w)](https://www.dropbox.com/s/ms8g7zz8ink2krm/f1.png?raw=1) where ![f(wi)](https://www.dropbox.com/s/mjnohff0ewdyb78/f8.png?raw=1) is the frequency of word ![wi](https://www.dropbox.com/s/is6askf96sj3lhs/f9.png?raw=1) and ![t](https://www.dropbox.com/s/2pkwgism8101n3a/f10.png?raw=1) is a chosen threshold, typically around ![10e−5](https://www.dropbox.com/s/ugsghly2s3k4t9s/f11.png?raw=1).
But [the original C code](https://github.com/svn2github/word2vec) implements another equation, the probability of ***keeping*** the word: ![P(w)](https://www.dropbox.com/s/z1umpdl9h6qe559/f2.png?raw=1) where ![r(wi)](https://www.dropbox.com/s/sm6ag6nx6wq44oc/f3.png?raw=1) and ![T](https://www.dropbox.com/s/8jg6t3rvtjvqbis/f12.png?raw=1) is the total words in the corpus, so: ![P(w)](https://www.dropbox.com/s/sshui81t3q6xi28/f4.png?raw=1)  
**Probability of word keeping distribution:**
![Subsampling probability (keeping the word)](https://www.dropbox.com/s/fgjduwpjkvzi3a3/g1.png?raw=1)
We can see that probability of infrequent words keeping equals to 1. So we can find boundaries where do we need to compute a probability by solving the inequality: ![P(wi)>=1](https://www.dropbox.com/s/ghegeqzuo1ls7pq/f6.png?raw=1) than,
**probability computation is needed only for the following word frequencies:** ![f(wi)](https://www.dropbox.com/s/b5slcxb4fihh509/f7.png?raw=1)
- ### Negative sampling

- ### Hierarchical Softmax

- ### Vocabulary

- ### File operations

## Performance
