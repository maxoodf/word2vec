//
//  wordReader.cpp
//  word2vecpp
//
//  Created by Max Fomichev on 29/04/16.
//  Copyright © 2016 Pieci Kvadrati. All rights reserved.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>

#include "wrErrors.h"
#include "wordReader.h"

fileMapper_t::fileMapper_t(const std::string &_fileName, bool _wrFlag, off_t _size):
baseMapper_t(nullptr, 0), m_fileName(_fileName), m_fd(-1), m_wrFlag(_wrFlag) {
    if (m_wrFlag) {
        m_size = _size;
    }
    
    open();
}

void fileMapper_t::open() {
    // open file
    m_fd = ::open(m_fileName.c_str(), m_wrFlag?(O_RDWR | O_CREAT):O_RDONLY, 0600);
    if (m_fd < 0) {
        throw errorWR_t(std::strerror(errno));
    }
    
    // get file size
    struct stat fst;
    if (fstat(m_fd, &fst) < 0) {
        throw errorWR_t(std::strerror(errno));
    }

    if (!m_wrFlag) {
        if (fst.st_size <= 0) {
            throw errorWR_t("file is empty");
        }
        
        m_size = fst.st_size;
    } else {
        if (ftruncate(m_fd, m_size) == -1) {
            throw errorWR_t(std::strerror(errno));
        }
    }
    
    // map file to memory
    m_data = static_cast<const char *>(mmap(0, m_size,
                                            m_wrFlag?(PROT_READ | PROT_WRITE):PROT_READ , MAP_SHARED, m_fd, 0));
    if (m_data == MAP_FAILED) {
        throw errorWR_t(std::strerror(errno));
    }
}

void fileMapper_t::close() {
    munmap((void *) m_data, m_size);
    ::close(m_fd);
}

fileMapper_t::~fileMapper_t() {
    close();
}
