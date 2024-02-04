#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#ifndef HAVE_MSYNC

#include "mman.c"

#ifdef WIN32

int msync(void *addr, size_t len, int flags)
{
    if (FlushViewOfFile(addr, len))
        return 0;
    
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}
#else

#error no emulation for msync

#endif

#endif
