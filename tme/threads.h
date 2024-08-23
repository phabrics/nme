/* $Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $ */

/* tme/threads.h - header file for threads: */

/*
 * Copyright (c) 2003 Matt Fredette
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Matt Fredette.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TME_THREADS_H
#define _TME_THREADS_H

#include <tme/common.h>
_TME_RCSID("$Id: threads.h,v 1.10 2010/06/05 19:36:35 fredette Exp $");

/* includes: */
#include <errno.h>

/* setjmp/longjmp threading: */
#ifdef TME_THREADS_POSIX
#include "threads-posix.h"
#elif defined(TME_THREADS_GLIB)
#include "threads-glib.h"
#elif defined(TME_THREADS_FIBER)
#include "threads-fiber.h"
#endif

#ifdef _TME_HAVE_ZLIB
#include "zlib.h"
struct tme_zlib_handle {
  gzFile handle;
  int fd;
};
struct tme_zlib_handle  *tme_zlib_open _TME_P((const char *path, int flags));
#ifdef TME_THREADS_FIBER
ssize_t tme_zlib_read _TME_P((struct tme_zlib_handle  *hand, void *buf, size_t len, tme_mutex_t *mutex));
ssize_t tme_zlib_write _TME_P((struct tme_zlib_handle  *hand, const void *buf, size_t len, tme_mutex_t *mutex));
#endif
#endif

typedef void (*tme_threads_fn) _TME_P((void));
typedef int (*tme_threads_fn1) _TME_P((void *));

void tme_threads_init _TME_P((tme_threads_fn1 run, void *arg));
void tme_threads_run _TME_P((void));
void tme_thread_enter _TME_P((tme_mutex_t *mutex));

static _tme_inline void tme_mutex_lock _TME_P((tme_mutex_t *mutex)) { 
  _tme_thread_suspended();
  
  _tme_mutex_lock(mutex);

  _tme_thread_resumed();
}

static _tme_inline int tme_thread_sleep_yield _TME_P((tme_time_t time, tme_mutex_t *mutex)) { 
  if(mutex) tme_mutex_unlock(mutex);
  
  _tme_thread_suspended();

  tme_thread_sleep(time);
  
  if(mutex) _tme_mutex_lock(mutex);

  _tme_thread_resumed();

  return 0;
}

/* I/O: */
#ifdef WIN32
/* file flags: */
#define TME_FILE_RO		GENERIC_READ
#define TME_FILE_WO		GENERIC_WRITE
#define TME_FILE_RW		GENERIC_READ | GENERIC_WRITE
#define TME_FILE_NB		0

#ifdef TME_HAVE_INT64_T
typedef tme_int64_t tme_off_t;
#else
#error "No support for 32-bit file offsets on Windows"
#endif

typedef struct tme_win32_handle *tme_event_t;
#define TME_INVALID_EVENT NULL

#define TME_WIN32_HANDLE(hand) (*(HANDLE *)(hand))

extern tme_event_t win32_stdin;
extern tme_event_t win32_stdout;
extern tme_event_t win32_stderr;
tme_event_t tme_win32_open _TME_P((const char *path, int flags, int attr, size_t size));
void tme_win32_close _TME_P((tme_event_t));

static _tme_inline ssize_t tme_read _TME_P((HANDLE hand, void *buf, size_t len)) {
  int ret;
  return (ReadFile(hand, buf, len, &ret, NULL)) ? (ret) : (-1);
}

static _tme_inline ssize_t tme_write _TME_P((HANDLE hand, const void *buf, size_t len)) {
  int ret;
  return (WriteFile(hand, buf, len, &ret, NULL)) ? (ret) : (-1);
}

#define tme_thread_fd(hand, flags) _open_osfhandle((intptr_t)TME_THREAD_HANDLE(hand), flags);
#define TME_SEEK_SET FILE_BEGIN
#define TME_SEEK_CUR FILE_CURRENT
#define TME_SEEK_END FILE_END

#define TME_STD_HANDLE(hand) win32_##hand

#ifdef TME_THREADS_FIBER

typedef tme_event_t tme_thread_handle_t;
#define TME_STD_THREAD_HANDLE TME_STD_HANDLE
#define TME_STD_EVENT_HANDLE TME_STD_HANDLE
#define TME_THREAD_HANDLE TME_WIN32_HANDLE
#define TME_EVENT_HANDLE TME_THREAD_HANDLE
#define TME_INVALID_HANDLE NULL
#define tme_thread_open(path, flags) tme_win32_open(path, flags, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0)
#define tme_event_open(path, flags) tme_win32_open(path, flags, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 1024)
#define tme_thread_close tme_win32_close
#define tme_event_close tme_thread_close
#ifdef TME_THREADS_FIBER
#define tme_thread_read tme_fiber_read
#define tme_thread_write tme_fiber_write
#endif
tme_off_t tme_thread_seek _TME_P((tme_thread_handle_t hand, tme_off_t off, int where));

#else /* TME_THREADS_FIBER */

#define TME_THREADS_DIRECTIO
#define TME_STD_THREAD_HANDLE(hand) TME_WIN32_HANDLE(win32_##hand)
#define TME_STD_EVENT_HANDLE TME_STD_HANDLE
#define TME_EVENT_HANDLE TME_WIN32_HANDLE
#define TME_THREAD_HANDLE(hand) hand
#define TME_INVALID_HANDLE INVALID_HANDLE_VALUE
typedef HANDLE tme_thread_handle_t;

#define tme_thread_open(path,flags)		\
  CreateFile(path, \
	     flags, \
	     0, /* was: FILE_SHARE_READ */ \
	     0, \
	     OPEN_EXISTING, \
	     FILE_ATTRIBUTE_NORMAL, \
	     0)
#define tme_thread_close CloseHandle
#define tme_event_open(path, flags) tme_win32_open(path, flags, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 1024)
#define tme_event_close tme_win32_close
static _tme_inline tme_off_t tme_thread_seek _TME_P((tme_thread_handle_t hand, tme_off_t off, int where)) {
  LARGE_INTEGER ret;

  return (SetFilePointerEx(hand, (LARGE_INTEGER)off, &ret, where)) ? (ret.QuadPart) : (-1);
}

#endif /* !TME_THREADS_FIBER */

#elif defined(USE_ZLIB) && defined(_TME_HAVE_ZLIB)
/* file flags: */
typedef struct tme_zlib_handle  *tme_event_t;
#define TME_FILE_RO		O_RDONLY
#define TME_FILE_WO		O_WRONLY
#define TME_FILE_RW		O_RDWR
#define TME_FILE_NB		0
#define TME_THREAD_HANDLE(hand) hand->handle
#define TME_EVENT_HANDLE(hand) hand->fd
#define TME_INVALID_HANDLE NULL
#define TME_INVALID_EVENT NULL
#define TME_STD_HANDLE(hand) fileno(hand)
#define TME_STD_THREAD_HANDLE(hand) fileno(hand)
#define TME_STD_EVENT_HANDLE(hand) fileno(hand)
typedef tme_event_t tme_thread_handle_t;
#define tme_thread_fd(hand, flags) hand->fd
#define tme_read gzread
#define tme_write gzwrite
#ifdef TME_THREADS_FIBER
#define tme_thread_read tme_zlib_read
#define tme_thread_write tme_zlib_write
#endif
#define TME_SEEK_SET SEEK_SET
#define TME_SEEK_CUR SEEK_CUR
#define TME_SEEK_END SEEK_END
typedef z_off_t tme_off_t;
#define tme_thread_seek(hand,off,where) gzseek(TME_THREAD_HANDLE(hand),off,where)
#define tme_thread_open tme_zlib_open
#define tme_thread_close(hand) gzclose(TME_THREAD_HANDLE(hand))
#define tme_event_open gzopen
#define tme_event_close gzclose

#else // HAVE_ZLIB
/* file flags: */
#define TME_FILE_RO		O_RDONLY
#define TME_FILE_WO		O_WRONLY
#define TME_FILE_RW		O_RDWR
#define TME_FILE_NB		O_NONBLOCK
#define TME_THREAD_HANDLE(hand) hand
#define TME_EVENT_HANDLE(hand) hand
#define TME_INVALID_HANDLE -1
#define TME_INVALID_EVENT -1
#define TME_STD_HANDLE(hand) fileno(hand)
#define TME_STD_THREAD_HANDLE(hand) fileno(hand)
#define TME_STD_EVENT_HANDLE(hand) fileno(hand)
typedef int tme_event_t;
typedef tme_event_t tme_thread_handle_t;
#define tme_thread_fd(hand, flags) hand
#define tme_read read
#define tme_write write
#ifdef TME_THREADS_FIBER
#define tme_thread_read tme_fiber_read
#define tme_thread_write tme_fiber_write
#endif
#define TME_SEEK_SET SEEK_SET
#define TME_SEEK_CUR SEEK_CUR
#define TME_SEEK_END SEEK_END
typedef off_t tme_off_t;
#define tme_thread_seek lseek
#define tme_thread_open open
#define tme_thread_close close
#define tme_event_open open
#define tme_event_close close
#endif // !WIN32

ssize_t tme_event_yield _TME_P((tme_event_t, void *, size_t, unsigned int, tme_mutex_t *, void **));

#ifdef TME_THREADS_FIBER

ssize_t tme_fiber_read _TME_P((tme_thread_handle_t hand, void *buf, size_t len, tme_mutex_t *mutex));
ssize_t tme_fiber_write _TME_P((tme_thread_handle_t hand, const void *buf, size_t len, tme_mutex_t *mutex));

#else

static _tme_inline ssize_t tme_thread_read _TME_P((tme_thread_handle_t hand, void *buf, size_t len, tme_mutex_t *mutex)) {
  int rc;

  if(mutex) tme_mutex_unlock(mutex);

  _tme_thread_suspended();
  
  rc = tme_read(TME_THREAD_HANDLE(hand), buf, len);
  
  _tme_thread_resumed();

  if(mutex) tme_mutex_lock(mutex);

  return rc;
}

static _tme_inline ssize_t tme_thread_write _TME_P((tme_thread_handle_t hand, const void *buf, size_t len, tme_mutex_t *mutex)) {
  int rc;

  if(mutex) tme_mutex_unlock(mutex);
  
  _tme_thread_suspended();
  
  rc = tme_write(TME_THREAD_HANDLE(hand), buf, len);
  
  _tme_thread_resumed();
  
  if(mutex) tme_mutex_lock(mutex);

  return rc;
}

static _tme_inline void tme_thread_exit _TME_P((tme_mutex_t *mutex)) {
  _tme_thread_suspended();  
  if(mutex)
    tme_mutex_unlock(mutex);
}
#endif /* !TME_THREADS_FIBER */

#endif /* !_TME_THREADS_H */
