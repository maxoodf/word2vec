/**
 * @file
 * @brief map/unmap files into memory, Win32 API wrappers for mmap and munmap sys calls
 * @author Max Fomichev
 * @date 12.03.2017
 * @copyright Apache License v.2 (http://www.apache.org/licenses/LICENSE-2.0)
*/

#include <windows.h>
#include <io.h>

#include "mman.h"

void *mmap(void *, size_t _size, int _prot, int, int _fd, off_t) {
    if (_size == 0) {
        return MAP_FAILED;
    }

    HANDLE mappedFD = CreateFileMapping(reinterpret_cast<HANDLE>(_get_osfhandle(_fd)), nullptr,
                                        (_prot & PROT_WRITE)?PAGE_READWRITE:PAGE_READONLY,
                                        0, 0, nullptr);
    if (mappedFD == nullptr) {
        return MAP_FAILED;
    }

    void *mappedAddr = MapViewOfFile(mappedFD, (_prot & PROT_WRITE)?FILE_MAP_WRITE:FILE_MAP_READ, 0, 0, _size);
    CloseHandle(mappedFD);
    if (mappedAddr == nullptr) {
        return MAP_FAILED;
    }

    return mappedAddr;
}

int munmap(void *_addr, size_t) {
    if (UnmapViewOfFile(_addr)) {
        return 0;
    }

    return -1;
}
