#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#ifndef HAVE_MUNMAP

#include "mman.c"

#ifdef WIN32

int munmap(void *addr, size_t len)
{
    if (UnmapViewOfFile(addr))
        return 0;
        
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}
#else

#error no emulation for munmap

#endif

#endif
