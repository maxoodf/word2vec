# word2vecpp
Just another implementation of Google's word2vec - https://github.com/svn2github/word2vec<br>
The following features make it a little bit faster (5-6% in avarage) - <br>
1. Words dictionary implemented using C++11 std::unordered_map<br>
2. Train text corpus reading implemented with mmap system call insted of fstream<br>
3. Stop words supported<br>
