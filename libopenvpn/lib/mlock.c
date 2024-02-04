#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(_MSC_VER)
#include "config-msvc.h"
#endif

#ifndef HAVE_MLOCK

#include "mman.c"

#ifdef WIN32

int mlock(const void *addr, size_t len)
{
    if (VirtualLock((LPVOID)addr, len))
        return 0;
        
    errno =  __map_mman_error(GetLastError(), EPERM);
    
    return -1;
}
#else

#error no emulation for mlock

#endif

#endif
