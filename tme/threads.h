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
#include <tme/libopenvpn/syshead.h>
#include <tme/libopenvpn/event.h>

/* setjmp/longjmp threading: */
#ifdef TME_THREADS_POSIX
#include "threads-posix.h"
#elif defined(TME_THREADS_GLIB)
#include "threads-glib.h"
#elif defined(TME_THREADS_SJLJ)
#include "threads-sjlj.h"
#endif

typedef void (*tme_threads_fn) _TME_P((void));
typedef int (*tme_threads_fn1) _TME_P((void *));

void tme_threads_init _TME_P((tme_threads_fn1 run, void *arg));
void tme_threads_run _TME_P((void));
void tme_thread_enter _TME_P((tme_mutex_t *mutex));

/* I/O: */
/* file flags: */
#define TME_FILE_FLAG_RO		TME_BIT(0)

#ifdef WIN32
#define TME_INVALID_HANDLE INVALID_HANDLE_VALUE
typedef HANDLE tme_handle_t;
static _tme_inline tme_handle_t tme_open _TME_P((const char *path, int flags, int *fd)) {
  tme_handle_t hand;
  
  hand = CreateFile(path,
		    ((flags & TME_FILE_FLAG_RO)
		     ? GENERIC_READ		
		     : GENERIC_READ | GENERIC_WRITE),
		    0, /* was: FILE_SHARE_READ */
		    0,			
		    OPEN_EXISTING,		
		    FILE_ATTRIBUTE_NORMAL,	
		    0);
  if(fd)
    *fd = _open_osfhandle((intptr_t)hand, ((flags & TME_FILE_FLAG_RO)
					   ? O_RDONLY  
					   : O_RDWR));
  return hand;
}
typedef struct tme_win32_handle {
  HANDLE hand;
  struct overlapped_io reads;
  struct overlapped_io writes;
  struct rw_handle rw_handle;
} *tme_win32_handle_t;  
tme_win32_handle_t tme_win32_open _TME_P((const char *path, int flags, int *fd));
#define tme_close CloseHandle
#define TME_SEEK_SET FILE_BEGIN
#define TME_SEEK_CUR FILE_CURRENT
#define TME_SEEK_END FILE_END
static _tme_inline off_t tme_seek _TME_P((tme_handle_t hand, off_t off, int where)) {
  LARGE_INTEGER pos, ret;
#ifdef TME_HAVE_INT64_T
  pos.QuadPart = off;
#else
  pos.u.LowPart = (DWORD)off;
  pos.u.HighPart = (LONG)(off>>32);
#endif
  return (SetFilePointerEx(hand, pos, &ret, where)) ?
#ifdef TME_HAVE_INT64_T
    (ret.QuadPart)
#else
    ((ret.u.LowPart) | (ret.u.HighPart << 32))
#endif
    : (-1);
}
static _tme_inline ssize_t _tme_read _TME_P((tme_handle_t hand, void *buf, size_t count)) {
  int ret;
  return (ReadFile(hand, buf, count, &ret, NULL)) ? (ret) : (-1);
}

static _tme_inline ssize_t _tme_write _TME_P((tme_handle_t hand, const void *buf, size_t count)) {
  int ret;
  return (WriteFile(hand, buf, count, &ret, NULL)) ? (ret) : (-1);
}
int tme_read_queue (tme_win32_handle_t hand, int maxsize);
int tme_write_queue (tme_win32_handle_t hand, struct buffer *buf);
int tme_finalize (HANDLE h, struct overlapped_io *io, struct buffer *buf);

static inline int
tme_write_win32 (tme_win32_handle_t hand, struct buffer *buf)
{
  int err = 0;
  int status = 0;
  if (overlapped_io_active (&hand->writes))
    {
      status = tme_finalize (hand->hand, &hand->writes, NULL);
      if (status < 0)
	err = GetLastError ();
    }
  tme_write_queue (hand, buf);
  if (status < 0)
    {
      SetLastError (err);
      return status;
    }
  else
    return BLEN (buf);
}

static inline int
read_tme_buffered (tme_win32_handle_t hand, struct buffer *buf, int maxsize)
{
  return tme_finalize (hand->hand, &hand->reads, buf);
}

static inline int
write_tme_buffered (tme_win32_handle_t hand, struct buffer *buf)
{
  return tme_write_win32 (hand, buf);
}
#else
#define TME_INVALID_HANDLE -1
typedef int tme_handle_t;
static _tme_inline tme_handle_t tme_open _TME_P((const char *path, int flags, int *fd)) {
  tme_handle_t hand;

  hand = open(path, ((flags & TME_FILE_FLAG_RO)
                    ? O_RDONLY  
		    : O_RDWR));
  if(fd) *fd = hand;
  return hand;
}
#define tme_close close
#define TME_SEEK_SET SEEK_SET
#define TME_SEEK_CUR SEEK_CUR
#define TME_SEEK_END SEEK_END
#define tme_seek lseek
#define _tme_read read
#define _tme_write write
#endif

#ifdef TME_THREADS_DIRECT_IO
typedef tme_handle_t tme_thread_handle_t;
#ifdef WIN32
typedef tme_win32_handle_t tme_event_t;
#else
typedef tme_handle_t tme_event_t;
#endif
#define tme_thread_open tme_open
#define tme_thread_close tme_close
#define tme_thread_seek tme_seek
#define tme_read _tme_read
#define tme_write _tme_write
#define tme_event_read _tme_read
#define tme_event_write _tme_write

/* Events: */
typedef struct event_set tme_event_set_t;

#define tme_event_set(s) (s)
#define tme_event_set_init event_set_init
#define tme_event_free event_free
#define tme_event_reset event_reset
#define tme_event_del event_del
#define tme_event_ctl event_ctl

static _tme_inline
int tme_event_wait _TME_P((tme_event_set_t *es, const struct timeval *tv, struct event_set_return *out, int outlen)) {
  int rc;

  _tme_thread_suspended();
  
  rc = event_wait(es, tv, out, outlen);

  _tme_thread_resumed();

  return rc;
}

static _tme_inline
int tme_event_wait_yield _TME_P((tme_event_set_t *es, const struct timeval *tv, struct event_set_return *out, int outlen, tme_mutex_t *mutex)) {
  int rc;

  if(mutex) {
    tme_mutex_unlock(mutex);
  
    rc = tme_event_wait(es, tv, out, outlen);
    
    tme_mutex_lock(mutex);
  } else rc = event_wait(es, tv, out, outlen);

  return rc;
}

static _tme_inline ssize_t tme_thread_read _TME_P((tme_thread_handle_t hand, void *buf, size_t count)) {
  int rc;

  _tme_thread_suspended();
  
  rc = tme_read(hand, buf, count);

  _tme_thread_resumed();
  
  return rc;
}

static _tme_inline ssize_t tme_thread_read_yield _TME_P((tme_thread_handle_t hand, void *buf, size_t count, tme_mutex_t *mutex)) {
  int rc;

  if(mutex) {
    tme_mutex_unlock(mutex);
    
    rc = tme_thread_read(hand, buf, count);
    
    tme_mutex_lock(mutex);
  } else rc = tme_read(hand, buf, count);

  return rc;
}

static _tme_inline ssize_t tme_thread_write _TME_P((tme_thread_handle_t hand, const void *buf, size_t count)) {
  int rc;

  _tme_thread_suspended();
  
  rc = tme_write(hand, buf, count);

  _tme_thread_resumed();

  return rc;
}

static _tme_inline ssize_t tme_thread_write_yield _TME_P((tme_thread_handle_t hand, const void *buf, size_t count, tme_mutex_t *mutex)) {
  int rc;

  if(mutex) {
    tme_mutex_unlock(mutex);
    
    rc = tme_thread_write(hand, buf, count);
    
    tme_mutex_lock(mutex);
  } else rc = tme_write(hand, buf, count);

  return rc;
}
#else
#ifdef WIN32
typedef tme_win32_handle_t tme_thread_handle_t;
typedef tme_win32_handle_t tme_event_t;
#define tme_win32_open tme_open
#define tme_win32_close tme_close
#define tme_thread_seek(hand, off, flags) tme_seek(hand->hand, off, flags)
#define tme_read(hand,buf,size) _tme_read(hand->hand, BPTR(buf), size)
#define tme_write(hand,buf,size) _tme_write(hand->hand,BPTR(buf),BLEN(buf))
#define tme_event_read(hand,buf,size) read_tme_buffered(hand, buf, size)
#define tme_event_write(hand,buf,size) write_tme_buffered(hand, buf)
#define TME_PASS_BUFFER
#else
typedef tme_handle_t tme_thread_handle_t;
typedef tme_handle_t tme_event_t;
#define tme_thread_open tme_open
#define tme_thread_close tme_close
#define tme_thread_seek tme_seek
#define tme_read _tme_read
#define tme_write _tme_write
#define tme_event_read _tme_read
#define tme_event_write _tme_write
#endif /* !WIN32 */

ssize_t tme_event_yield _TME_P((tme_event_t, void *, size_t, unsigned int, tme_mutex_t *));
#define tme_thread_read_yield(hand, data, count, mutex) ((mutex) ? (tme_event_yield(hand, data, count, EVENT_READ, mutex)) : (tme_read(hand, data, count)))
#define tme_thread_write_yield(hand, data, count, mutex) ((mutex) ? (tme_event_yield(hand, data, count, EVENT_WRITE, mutex)) : (tme_write(hand, data, count)))
#define tme_thread_read(hand, data, count) tme_event_yield(hand, data, count, EVENT_READ, NULL)
#define tme_thread_write(hand, data, count) tme_event_yield(hand, data, count, EVENT_WRITE, NULL)
#endif /* !TME_THREADS_DIRECT_IO */

static inline event_t
tme_event_handle (const tme_event_t hand)
{
#ifdef WIN32
  return &hand->rw_handle;
#else
  return hand;
#endif
}

#endif /* !_TME_THREADS_H */
