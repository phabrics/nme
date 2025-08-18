/* libtme/threads.c - threads management: */

/*
 * Copyright (c) 2015 Ruben Agin
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

/* includes: */
#include <tme/threads.h>
#include <tme/openvpn-setup.h>

#ifdef TME_THREADS_POSIX
pthread_attr_t *attrp;
#endif

int thread_mode;
tme_rwlock_t tme_rwlock_suspere;

int tme_rwlock_timedlock(tme_rwlock_t *l, unsigned long sec, int write) { 
  tme_time_t sleep;
  tme_timeout_t timeout;
  int rc = TME_OK;

  sleep = TME_TIME_SET_SEC(sec) + tme_thread_get_time();

  tme_get_timeout(sleep, &timeout);
  
  _tme_thread_suspended();
  
  if (write)
    tme_thread_op2(rwlock_timedwrlock, &(l)->lock, &timeout);
  else
    tme_thread_op2(rwlock_timedrdlock, &(l)->lock, &timeout);

  _tme_thread_resumed();

  return rc;
}

int tme_mutex_timedlock(tme_mutex_t *m, unsigned long sec) {
  tme_time_t sleep;
  tme_timeout_t timeout;
  int rc;

  sleep = TME_TIME_SET_SEC(sec) + tme_thread_get_time();

  tme_get_timeout(sleep, &timeout);

  _tme_thread_suspended();
  
  rc = tme_thread_op2(mutex_timedlock, m, &timeout);

  _tme_thread_resumed();

  return rc;
}

int tme_rwlock_rdlock(tme_rwlock_t *l) {
  if((l)->writer == tme_thread_self())
    // simulates deadlock return when current thread has the write lock
    return TME_EDEADLK;

  _tme_thread_suspended();
  tme_thread_op(rwlock_rdlock,&(l)->lock);
  _tme_thread_resumed();
  
  /* TODO: insert some kind of timer to interrupt at the end of the timeout */
  return TME_OK;  
}

int tme_rwlock_wrlock(tme_rwlock_t *l) {
  if((l)->writer == tme_thread_self())
    // simulates deadlock return when current thread has the write lock
    return TME_EDEADLK;

  _tme_thread_suspended();
  tme_thread_op(rwlock_wrlock,&(l)->lock);
  _tme_thread_resumed();
  (l)->writer = tme_thread_self();
  return TME_OK;
}

int tme_rwlock_trywrlock(tme_rwlock_t *l) {
  if(!tme_thread_op(rwlock_trywrlock,&(l)->lock))
    return TME_EBUSY;
  (l)->writer = tme_thread_self();
  return TME_OK;
}

int tme_rwlock_wrunlock(tme_rwlock_t *l) {
  (l)->writer = 0;
  tme_thread_op(rwlock_wrunlock,&(l)->lock);
  return TME_OK;
}

int tme_cond_wait_yield(tme_cond_t *cond, tme_mutex_t *mutex) {
  int rc = TME_OK;
  
  _tme_thread_suspended();

  tme_thread_op2(cond_wait, cond, mutex);

  _tme_thread_resumed();

  return rc;
}

int tme_cond_sleep_yield(tme_cond_t *cond, tme_mutex_t *mutex,
			 tme_time_t sleep) {
  tme_timeout_t timeout;
  int rc = TME_OK;

  sleep += tme_thread_get_time();

  tme_get_timeout(sleep, &timeout);
  
  _tme_thread_suspended();

  tme_thread_op3(cond_wait_until, cond, mutex, &timeout);

  _tme_thread_resumed();

  return rc;
}

void tme_thread_yield(void) {
  if(!tme_thread_cooperative()) return;
  
  _tme_thread_suspended();

  if(thread_mode)
    _tme_thread_yield();
  else
    tme_fiber_yield();

  _tme_thread_resumed();
}

int tme_thread_sleep_yield _TME_P((tme_time_t sleep, tme_mutex_t *mutex)) { 
  tme_timeout_t timeout;

  if(mutex) tme_mutex_unlock(mutex);

  tme_get_timeout(sleep, &timeout);
  
  _tme_thread_suspended();

  tme_thread_op(sleep, &timeout);
  
  if(mutex) tme_thread_op(mutex_lock, mutex);

  _tme_thread_resumed();

  return 0;
}

#ifdef WIN32

struct tme_win32_handle {
  HANDLE handle;
  struct overlapped_io reads;
  struct overlapped_io writes;
  struct rw_handle rw_handle;
};

tme_event_t win32_stdin;
tme_event_t win32_stdout;
tme_event_t win32_stderr;

int
tme_read_queue (tme_event_t hand, int maxsize)
{
  if (hand->reads.iostate == IOSTATE_INITIAL)
    {
      struct buffer *buf = &hand->reads.buf;
      DWORD len = BLEN(buf);
      BOOL status = TRUE;
      int err;

      /* the overlapped read will signal this event on I/O completion */
      ASSERT (ResetEvent (hand->reads.overlapped.hEvent));

      hand->reads.size = 0;

      if(!len) {
	buf_reset_len(buf);
	len = buf_forward_capacity(buf);
	if(maxsize) 
	  len = TME_MIN(len, maxsize);
	status = ReadFile(
			  hand->handle,
			  BPTR (buf),
			  len,
			  &hand->reads.size,
			  &hand->reads.overlapped
			  );
      }
      
      if (status) /* operation completed immediately? */
	{
	  /* since we got an immediate return, we must signal the event object ourselves */
	  ASSERT (SetEvent (hand->reads.overlapped.hEvent));

	  hand->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
	  hand->reads.status = 0;

	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Read immediate return [%d,%d]",
		(int) len,
		(int) hand->reads.size);   
	  buf_inc_len(buf, hand->reads.size);   
	}
      else
	{
	  err = GetLastError (); 
	  if (err == ERROR_IO_PENDING) /* operation queued? */
	    {
	      hand->reads.iostate = IOSTATE_QUEUED;
	      hand->reads.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Read queued [%d]",
		    (int) len);
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->reads.overlapped.hEvent));
	      hand->reads.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->reads.status = err;
	      hand->reads.size = 0;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Read error [%d] : %s",
		    (int) len,
		    strerror_win32 (status, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->reads.iostate;
}

int
tme_write_queue (tme_event_t hand, struct buffer *ibuf)
{
  struct buffer *buf = &hand->writes.buf;
  
  if (hand->writes.iostate == IOSTATE_INITIAL)
    {
      BOOL status;
      int err;
 
      /* make a private copy of buf */
      buf_reset_len(buf);
      ASSERT (buf_copy (buf, ibuf));

      /* the overlapped write will signal this event on I/O completion */
      ASSERT (ResetEvent (hand->writes.overlapped.hEvent));

      status = WriteFile(
			 hand->handle,
			 BPTR (buf),
			 BLEN (buf),
			 &hand->writes.size,
			 &hand->writes.overlapped
			 );

      if (status) /* operation completed immediately? */
	{
	  hand->writes.iostate = IOSTATE_IMMEDIATE_RETURN;

	  /* since we got an immediate return, we must signal the event object ourselves */
	  ASSERT (SetEvent (hand->writes.overlapped.hEvent));

	  hand->writes.status = 0;

	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Write immediate return [%d,%d]",
		BLEN (buf),
		(int) hand->writes.size);	       
	}
      else
	{
	  err = GetLastError (); 
	  if (err == ERROR_IO_PENDING) /* operation queued? */
	    {
	      hand->writes.iostate = IOSTATE_QUEUED;
	      hand->writes.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Write queued [%d]",
		    BLEN (buf));
	    }
	  else /* error occurred */
	    {
	      struct gc_arena gc = gc_new ();
	      ASSERT (SetEvent (hand->writes.overlapped.hEvent));
	      hand->writes.iostate = IOSTATE_IMMEDIATE_RETURN;
	      hand->writes.status = err;
	      dmsg (D_WIN32_IO, "WIN32 I/O: TME Write error [%d] : %s",
		    BLEN (buf),
		    strerror_win32 (err, &gc));
	      gc_free (&gc);
	    }
	}
    }
  return hand->writes.iostate;
}

tme_off_t tme_event_seek (HANDLE hand, struct overlapped_io *io, tme_off_t off, int where)
{
  LARGE_INTEGER pos;
  
  switch(where) {
  case TME_SEEK_SET:
    pos.QuadPart = 0;
    break;
  case TME_SEEK_CUR:
    pos.u.LowPart = io->overlapped.Offset;
    pos.u.HighPart = io->overlapped.OffsetHigh;
    break;
  case TME_SEEK_END:
    GetFileSizeEx(hand, &pos);
    break;
  }

  pos.QuadPart += off;
  io->overlapped.Offset = pos.u.LowPart;
  io->overlapped.OffsetHigh = pos.u.HighPart;
 
  return pos.QuadPart;
}

int
tme_finalize (
	      HANDLE h,
	      struct overlapped_io *io,
	      struct buffer *buf)
{
  int ret = -1;
  BOOL status;

  switch (io->iostate)
    {
    case IOSTATE_QUEUED:
      status = GetOverlappedResult(
				   h,
				   &io->overlapped,
				   &io->size,
				   FALSE
				   );
      if (status)
	{
	  /* successful return for a queued operation */
	  buf_inc_len(&io->buf, io->size);
	  if (buf) {
	    ret = TME_MIN(buf_forward_capacity(buf),BLEN(&io->buf));
	    buf_copy_n(buf, &io->buf, ret);
	  } else
	    ret = io->size;
	  tme_event_seek(h, io, ret, TME_SEEK_CUR);
	  io->iostate = IOSTATE_INITIAL;
	  ASSERT (ResetEvent (io->overlapped.hEvent));
	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion success [%d]", ret);
	}
      else
	{
	  /* error during a queued operation */
	  ret = -1;
	  if (GetLastError() != ERROR_IO_INCOMPLETE)
	    {
	      /* if no error (i.e. just not finished yet),
		 then DON'T execute this code */
	      io->iostate = IOSTATE_INITIAL;
	      ASSERT (ResetEvent (io->overlapped.hEvent));
	      msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TME Completion error");
	    }
	}
      break;

    case IOSTATE_IMMEDIATE_RETURN:
      io->iostate = IOSTATE_INITIAL;
      ASSERT (ResetEvent (io->overlapped.hEvent));
      if (io->status)
	{
	  /* error return for a non-queued operation */
	  SetLastError (io->status);
	  ret = -1;
	  msg (D_WIN32_IO | M_ERRNO, "WIN32 I/O: TME Completion non-queued error");
	}
      else
	{
	  /* successful return for a non-queued operation */
	  if (buf) {
	    ret = TME_MIN(buf_forward_capacity(buf), BLEN(&io->buf));
	    buf_copy_n(buf, &io->buf, ret);
	  } else
	    ret = io->size;
	  tme_event_seek(h, io, ret, TME_SEEK_CUR);
	  dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion non-queued success [%d]", ret);
	}
      break;

    case IOSTATE_INITIAL: /* were we called without proper queueing? */
      SetLastError (ERROR_INVALID_FUNCTION);
      ret = -1;
      dmsg (D_WIN32_IO, "WIN32 I/O: TME Completion BAD STATE");
      break;

    default:
      ASSERT (0);
    }

  if (GetLastError() == ERROR_HANDLE_EOF)
    ret = 0;
  if (buf)
    buf->len = ret;
  return ret;
}

static _tme_inline int
tme_write_win32 (tme_event_t hand, struct buffer *buf)
{
  int err = 0;
  int status = 0;
  if (overlapped_io_active (&hand->writes))
    {
      status = tme_finalize (hand->handle, &hand->writes, NULL);
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

tme_event_t tme_win32_open(const char *path, int flags, int attr, size_t size) {
  tme_event_t hand;
  HANDLE handle = CreateFile(path, flags, 0, 0, OPEN_EXISTING, attr, 0);

  if(handle == INVALID_HANDLE_VALUE)
    return NULL;
  
  hand = tme_new0(struct tme_win32_handle, 1);

  hand->handle = handle;
  
  CLEAR(hand->reads);
  CLEAR(hand->writes);
  
  /* manual reset event, initially set according to event_state */
  hand->reads.overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if(hand->reads.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");

  hand->writes.overlapped.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
  if (hand->writes.overlapped.hEvent == NULL)
    msg (M_ERR, "Error: overlapped_io_init: CreateEvent failed");
  
  hand->rw_handle.read = hand->reads.overlapped.hEvent;
  hand->rw_handle.write = hand->writes.overlapped.hEvent;

  hand->reads.buf = alloc_buf(size);
  ASSERT (buf_init (&hand->reads.buf, 0));
  ASSERT (buf_safe (&hand->reads.buf, 0));

  hand->writes.buf = alloc_buf(size);
  ASSERT (buf_init (&hand->writes.buf, 0));
  ASSERT (buf_safe (&hand->writes.buf, 0));
  
  return hand;
}

void tme_win32_close(tme_event_t hand) {
  if (hand->handle != NULL)
    {
      dmsg (D_WIN32_IO_LOW, "Attempting CancelIO on TAP-Windows adapter");
      if (!CancelIo (hand->handle))
	msg (M_WARN | M_ERRNO, "Warning: CancelIO failed on TAP-Windows adapter");
    }

  dmsg (D_WIN32_IO_LOW, "Attempting close of overlapped read event on TAP-Windows adapter");

  if (hand->reads.overlapped.hEvent)
    {
      if (!CloseHandle (hand->reads.overlapped.hEvent))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on overlapped I/O event object");
    }

  dmsg (D_WIN32_IO_LOW, "Attempting close of overlapped write event on TAP-Windows adapter");

  if (hand->writes.overlapped.hEvent)
    {
      if (!CloseHandle (hand->writes.overlapped.hEvent))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on overlapped I/O event object");
    }

  if (hand->handle != NULL)
    {
      dmsg (D_WIN32_IO_LOW, "Attempting CloseHandle on TAP-Windows adapter");
      if (!CloseHandle (hand->handle))
	msg (M_WARN | M_ERRNO, "Warning: CloseHandle failed on TAP-Windows adapter");
    }

  free_buf (&hand->reads.buf);
  free_buf (&hand->writes.buf);
}

tme_off_t tme_thread_seek (tme_thread_handle_t hand, tme_off_t off, int where) {
  tme_event_seek(hand->handle, &hand->reads, off, where);
  return tme_event_seek(hand->handle, &hand->writes, off, where);
}

static _tme_inline int
tme_event_read (tme_event_t hand, void *data, int len)
{
  int rc;
  struct buffer buf;

  if (hand == TME_STD_HANDLE(stdin)) 
    rc = tme_read(hand->handle, data, len);
  else {
    buf_set_write(&buf, data, len);
    rc = tme_finalize (hand->handle, &hand->reads, &buf);
  }
  return rc;
}

static _tme_inline int
tme_event_write (tme_event_t hand, void *data, int len)
{
  int rc;
  struct buffer buf;

  if(hand == TME_STD_HANDLE(stdout) || hand == TME_STD_HANDLE(stderr))
    rc = tme_write(hand->handle, data, len);
  else {
    buf_set_read(&buf, data, len);
    rc = tme_write_win32 (hand, &buf);
  }
  return rc;  
}
#else
#define tme_event_read tme_read
#define tme_event_write tme_write
#endif // !WIN32

static 
int tme_thread_event_wait(tme_event_set_t *es, const struct timeval *tv, struct event_set_return *out, int outlen, tme_mutex_t *mutex) {
  int rc;
  struct timeval _tv;

  _tv.tv_sec = (tv) ? (tv->tv_sec) : (BIG_TIMEOUT);
  _tv.tv_usec = (tv) ? (tv->tv_usec) : (0);
  
  if(mutex) tme_thread_mutex_unlock(&mutex->thread);
  
  _tme_thread_suspended();
  
  rc = event_wait(es, &_tv, out, outlen);

  if(mutex) tme_thread_mutex_lock(&mutex->thread);

  _tme_thread_resumed();
    
  return rc;
}

void tme_threads_init(int mode) {
  if((thread_mode=mode)) {
    /* initialize the runtime event callback handlers: */
    tme_event_set_init = event_set_init;
    tme_event_free = event_free;
    tme_event_reset = event_reset;
    tme_event_del = event_del;
    tme_event_ctl = event_ctl;
    tme_event_wait = tme_thread_event_wait;
  } else
    tme_fiber_threads_init();
  
  tme_rwlock_init(&tme_rwlock_suspere);

#ifdef WIN32
  win32_stdin = tme_new0(struct tme_win32_handle, 1);
  win32_stdout = tme_new0(struct tme_win32_handle, 1);
  win32_stderr = tme_new0(struct tme_win32_handle, 1);
  win32_stdin->rw_handle.read =
    win32_stdin->handle = GetStdHandle(STD_INPUT_HANDLE);
  win32_stdout->rw_handle.write =
    win32_stdout->handle = GetStdHandle(STD_OUTPUT_HANDLE);
  win32_stderr->rw_handle.write =
    win32_stderr->handle = GetStdHandle(STD_ERROR_HANDLE);
  win32_stdin->reads.overlapped.hEvent =
    win32_stdout->writes.overlapped.hEvent =
    win32_stderr->reads.overlapped.hEvent = NULL;
#endif
}

/* this reads or writes, yielding if the event is not ready: */
static _tme_inline ssize_t
tme_event_yield(tme_event_t hand, void *data, size_t len, unsigned int rwflags, tme_mutex_t *mutex)
{
  int i, rc = 1, key_event = FALSE;
  struct event_set_return esr;
  tme_event_set_t *tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);
  event_t handle = hand;
  
  do {
#ifdef WIN32
    handle = &hand->rw_handle;
    if (rwflags & EVENT_READ) {
      if (hand == TME_STD_HANDLE(stdin)) {
	INPUT_RECORD record[128];
	DWORD numRead;
	key_event = TRUE;
	if((rc = GetNumberOfConsoleInputEvents(hand->handle, &numRead)) &&
	   numRead>0 &&
	   (rc = PeekConsoleInput(hand->handle, record, 128, &numRead))) {
	  for(i=0;i<numRead;i++) {
	    if(record[i].EventType != KEY_EVENT) {
	      // don't care about other console events
	      continue;
	    }
      
	    if(!record[i].Event.KeyEvent.bKeyDown) {
	      // really only care about keydown
	      continue;
	    }
	    break;
	  }
	  if(i!=numRead) break;
	  rc = ReadConsoleInput(hand->handle, record, i, &numRead);
	}
	hand->reads.status = (rc) ? (rc) : (GetLastError());
	// if you're setup for ASCII, process this:
	//record.Event.KeyEvent.uChar.AsciiChar
      } else {
	tme_read_queue (hand, 0);
      }
    }
#endif
    tme_event_reset(tme_events);
    tme_event_ctl(tme_events, handle, rwflags, 0);
    rc = tme_event_wait(tme_events, NULL, &esr, 1, mutex);
  } while(key_event);

  tme_event_free(tme_events);
  
  /* do the i/o: */
  if(esr.rwflags & EVENT_WRITE)
    rc = tme_event_write(hand, data, len);
  if(esr.rwflags & EVENT_READ)
    rc = tme_event_read(hand, data, len);
  return rc;
}

ssize_t tme_thread_read(tme_thread_handle_t hand, void *buf, size_t len, tme_mutex_t *mutex) {
  return tme_event_yield(hand, buf, len, EVENT_READ, mutex);
}

ssize_t tme_thread_write(tme_thread_handle_t hand, const void *buf, size_t len, tme_mutex_t *mutex) {
  return tme_event_yield(hand, buf, len, EVENT_WRITE, mutex);
}

#ifdef _TME_HAVE_ZLIB
struct tme_zlib_handle  *tme_zlib_open(const char *path, int flags) {
  struct tme_zlib_handle  *hand;
  gzFile handle;
  int fd = open(path, flags);
  char *mode;
  
  if(fd == -1)
    return NULL;
  
  switch(flags) {
  case TME_FILE_RO: mode="rb"; break;
  case TME_FILE_WO: mode="wb"; break;
  case TME_FILE_RW: mode="+"; break;
  default: mode="a"; break;
  }

  handle = gzdopen(fd, mode);

  if(handle == NULL)
    return NULL;
  
  hand = tme_new0(struct tme_zlib_handle, 1);
  hand->fd = fd;
  hand->handle = handle;
  return hand;
}

/* this reads or writes, yielding if the event is not ready: */
static _tme_inline ssize_t
tme_zlib_yield(struct tme_zlib_handle  *hand, void *data, size_t len, unsigned int rwflags, tme_mutex_t *mutex)
{
  int rc = 1;
  struct event_set_return esr;
  tme_event_set_t *tme_events = tme_event_set_init(&rc, EVENT_METHOD_FAST);

  tme_event_reset(tme_events);
  tme_event_ctl(tme_events, hand->fd, rwflags, 0);
  rc = tme_event_wait(tme_events, NULL, &esr, 1, mutex);
  tme_event_free(tme_events);
  
  /* do the i/o: */
  if(esr.rwflags & EVENT_WRITE)
    rc = gzwrite(hand->handle, data, len);
  if(esr.rwflags & EVENT_READ)
    rc = gzread(hand->handle, data, len);
  return rc;
}

ssize_t tme_zlib_read(struct tme_zlib_handle  *hand, void *buf, size_t len, tme_mutex_t *mutex) {
  return tme_zlib_yield(hand, buf, len, EVENT_READ, mutex);
}

ssize_t tme_zlib_write(struct tme_zlib_handle  *hand, const void *buf, size_t len, tme_mutex_t *mutex) {
  return tme_zlib_yield(hand, buf, len, EVENT_WRITE, mutex);
}

#endif // HAVE_ZLIB
