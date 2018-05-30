/**
 * @file
 * @brief fileMapper & wordReader classes - fast text file/memory parsing
 * @author Max Fomichev
 * @date 19.04.2016
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include "win/mman.h"
#else
#include <sys/mman.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

#include <stdexcept>
#include <cstring>

#include "mapper.hpp"

namespace w2v {
    fileMapper_t::fileMapper_t(const std::string &_fileName, bool _wrFlag, off_t _size):
            mapper_t(), m_fileName(_fileName), m_wrFlag(_wrFlag) {

        if (m_wrFlag) {
            m_size = _size;
        }

        // open file
        m_fd = ::open(m_fileName.c_str(), m_wrFlag?(O_RDWR | O_CREAT):O_RDONLY, 0600);
        if (m_fd < 0) {
            std::string err = std::string("fileMapper: ") + _fileName + " - " + std::strerror(errno);
            throw std::runtime_error(err);
        }

        // get file size
        struct stat fst{};
        if (fstat(m_fd, &fst) < 0) {
            std::string err = std::string("fileMapper: ") + _fileName + " - " + std::strerror(errno);
            throw std::runtime_error(err);
        }

        if (!m_wrFlag) {
            if (fst.st_size <= 0) {
                throw std::runtime_error(std::string("fileMapper: file ") + _fileName + " is empty, nothing to read");
            }

            m_size = fst.st_size;
        } else {
            if (ftruncate(m_fd, m_size) == -1) {
                std::string err = std::string("fileMapper: ") + _fileName + " - " + std::strerror(errno);
                throw std::runtime_error(err);
            }
        }

        // map file to memory
        m_data.rwData = static_cast<char *>(mmap(nullptr, static_cast<size_t>(m_size),
                                                 m_wrFlag?(PROT_READ | PROT_WRITE):PROT_READ , MAP_SHARED,
                                                 m_fd, 0));
        if (m_data.rwData == static_cast<char *>(MAP_FAILED)) {
            std::string err = std::string("fileMapper: ") + _fileName + " - " + std::strerror(errno);
            throw std::runtime_error(err);
        }
    }

    fileMapper_t::~fileMapper_t() {
        munmap(reinterpret_cast<void *>(m_data.rwData), static_cast<size_t>(m_size));
        close(m_fd);
    }
}
