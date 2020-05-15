/**
 * @file
 * @brief mapper classes - mapping wrappers
 * @author Max Fomichev
 * @date 19.04.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_MAPPER_H
#define WORD2VEC_MAPPER_H

#include <string>

namespace w2v {
    /// @brief base class for different data sources (file, std::string etc) to be mapped
    class mapper_t {
    protected:
        union { // mapped memory
            char *rwData; // read/write access
            const char *roData; // read only access
        } m_data;
        off_t m_size = 0; // mapped memory size

    public:
        mapper_t(): m_data() {}
        mapper_t(char *_data, off_t _size): m_data(), m_size(_size) {m_data.rwData = _data;}
        mapper_t(const char *_data, off_t _size): m_data(), m_size(_size) {m_data.roData = _data;}
        virtual ~mapper_t() = default;

        /// @returns pointer to mapped data in read-only mode
        inline const char *data() const noexcept {return m_data.roData;}
        /// @returns pointer to mapped data in read/write mode
        inline char *data() noexcept {return m_data.rwData;}
        /// @returns mapped memory size
        inline off_t size() const noexcept {return m_size;}
    };

    class stringMapper_t final: public mapper_t {
    public:
        /**
         * Constructs a fileMapper object for reading or writing, depending on parameters
         * @param _fileName file name to be opened for reading or created for writing
         * @param _wrFlag create file for writing (default is false - open for reading)
         * @param _size size of a new created file (_wrFlag == true)
         * @throws std::runtime_error In case of failed file or mapping operations
        */
        explicit stringMapper_t(const std::string &_source):
                mapper_t(_source.c_str(), static_cast<off_t>(_source.length())) {}

        // copying prohibited
        stringMapper_t(const stringMapper_t &) = delete;
        void operator=(const stringMapper_t &) = delete;
    };
    /**
     * @brief C++ wrapper on mmap() system call
     *
     * fileMapper class is a simple wrapper on mmap() system call. Both reading from and writing to file are supported.
    */
    class fileMapper_t final: public mapper_t {
    private:
        const std::string m_fileName; // name of the file to be mapped
        int m_fd = -1; // file descriptor
        const bool m_wrFlag = false; // write mode

    public:
        /**
         * Constructs a fileMapper object for reading or writing, depending on parameters
         * @param _fileName file name to be opened for reading or created for writing
         * @param _wrFlag create file for writing (default is false, open for reading)
         * @param _size size of a new created file (_wrFlag must be true)
         * @throws std::runtime_error In case of failed file or mapping operations
        */
        explicit fileMapper_t(const std::string &_fileName, bool _wrFlag = false, off_t _size = 0);
        ~fileMapper_t() final;

        // copying prohibited
        fileMapper_t(const fileMapper_t &) = delete;
        void operator=(const fileMapper_t &) = delete;
    };
}

#endif //WORD2VEC_MAPPER_H
