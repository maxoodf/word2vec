/**
 * @file
 * @brief map/unmap files into memory, Win32 API wrappers for mmap and munmap sys calls
 * @author Max Fomichev
 * @date 12.03.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#ifndef WORD2VEC_MMAN_H
#define WORD2VEC_MMAN_H

#include <sys/types.h>

static const int PROT_READ = 1;
static const int PROT_WRITE = 2;
static const int MAP_SHARED = 1;
static void *MAP_FAILED = reinterpret_cast<void *>(-1);

void *mmap(void *, size_t _size, int _prot, int _flags, int _fd, off_t);
int munmap(void *_addr, size_t _size);

#endif //WORD2VEC_MMAN_H
