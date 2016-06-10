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

fileMapper_t::fileMapper_t(const std::string &_fileName): baseMapper_t(nullptr, 0), m_fd(-1) {
    // open file
    m_fd = open(_fileName.c_str(), O_RDONLY);
    if (m_fd < 0) {
        throw errorWR_t(std::strerror(errno));
    }
    
    // get file size
    struct stat fst;
    if (fstat(m_fd, &fst) < 0) {
        throw errorWR_t(std::strerror(errno));
    }
    if (fst.st_size <= 0) {
        throw errorWR_t("file is empty");
    }
    m_size = fst.st_size;
    
    // map file to memory
    m_data = static_cast<const char *>(mmap(0, m_size, PROT_READ, MAP_SHARED, m_fd, 0));
    if (m_data == MAP_FAILED) {
        throw errorWR_t(std::strerror(errno));
    }
}

fileMapper_t::~fileMapper_t() {
    munmap((void *) m_data, m_size);
    close(m_fd);
}
