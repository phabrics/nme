/* $Id: posix-disk.c,v 1.6 2010/06/05 14:28:57 fredette Exp $ */

/* host/posix/posix-disk.c - implementation of disks on a POSIX system: */

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

/* this might enable large-file support: */
#define _FILE_OFFSET_BITS 64

#include <tme/common.h>
_TME_RCSID("$Id: posix-disk.c,v 1.6 2010/06/05 14:28:57 fredette Exp $");

/* includes: */
#include <tme/generic/disk.h>
#include <tme/generic/bus.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/uio.h>
#ifdef HAVE_MMAP
#include <sys/types.h>
#include <sys/mman.h>
#endif /* HAVE_MMAP */
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */

/* macros: */

/* the maximum block size: */
#define TME_POSIX_DISK_BLOCK_SIZE_MAX	(16384)

/* disk flags: */
#define TME_POSIX_DISK_FLAG_RO		TME_BIT(0)

/* buffer flags: */
#define TME_POSIX_DISK_BUFFER_READABLE	TME_BIT(0)
#define TME_POSIX_DISK_BUFFER_DIRTY	TME_BIT(1)
#define TME_POSIX_DISK_BUFFER_MMAPPED	TME_BIT(2)

/* default buffer parameters: */
#define TME_POSIX_DISK_BUFFER_DEFAULT_COUNT	(16)
#define TME_POSIX_DISK_BUFFER_DEFAULT_AGG_PRE	(128UL * 1024UL)
#define TME_POSIX_DISK_BUFFER_DEFAULT_AGG_POST	(1UL * 1024UL * 1024UL)

/* types: */

/* a posix disk buffer: */
struct tme_posix_disk_buffer {

  /* buffers are kept on a doubly linked list: */
  struct tme_posix_disk_buffer *tme_posix_disk_buffer_next;
  struct tme_posix_disk_buffer **tme_posix_disk_buffer_prev;

  /* buffer flags: */
  int tme_posix_disk_buffer_flags;

  /* the file position, size, and data of this buffer.  a size of zero
     means this buffer is free: */

  /* we're paranoid about autoconf's ability to actually determine if
     off_t and size_t are available, and it's possible that a system
     that predates off_t and size_t may use other types to achieve
     large file support anyways, that autoconf's default off_t = long
     and size_t = unsigned definitions may not match.  the bizarre
     solution is to declare structs with appropriately-typed members,
     and then use those members: */
  struct stat _tme_posix_disk_buffer_stat;
  struct iovec _tme_posix_disk_buffer_iov;
#define tme_posix_disk_buffer_pos _tme_posix_disk_buffer_stat.st_size
#define tme_posix_disk_buffer_size _tme_posix_disk_buffer_iov.iov_len
#define tme_posix_disk_buffer_data _tme_posix_disk_buffer_iov.iov_base
};

/* a posix disk: */
struct tme_posix_disk {

  /* backpointer to our element: */
  struct tme_element *tme_posix_disk_element;

  /* our mutex: */
  tme_mutex_t tme_posix_disk_mutex;

  /* our flags: */
  int tme_posix_disk_flags;

  /* the file descriptor: */
  int tme_posix_disk_fd;

  /* the stat buffer: */
  struct stat tme_posix_disk_stat;

  /* our connection: */
  struct tme_disk_connection *tme_posix_disk_connection;

  /* our disk buffers.  the most-recently-used buffer
     is at the front of the list: */
  struct tme_posix_disk_buffer *tme_posix_disk_buffers;

  /* how much we aggregate behind and ahead when we will
     a new buffer: */
  struct stat _tme_posix_disk_stat0;
  struct stat _tme_posix_disk_stat1;
#define tme_posix_disk_buffer_agg_pre _tme_posix_disk_stat0.st_size
#define tme_posix_disk_buffer_agg_post _tme_posix_disk_stat1.st_size
};

/* this frees a buffer: */
static void
_tme_posix_disk_buffer_free(struct tme_posix_disk *posix_disk,
			    struct tme_posix_disk_buffer *buffer)
{
  int rc;
  struct iovec iovecs[1];
#define ssize iovecs[0].iov_len

  /* if this buffer is mmapped: */
  if (buffer->tme_posix_disk_buffer_flags 
      & TME_POSIX_DISK_BUFFER_MMAPPED) {
#ifdef HAVE_MMAP
	
    /* munmap the buffer: */
    rc = munmap(buffer->tme_posix_disk_buffer_data,
		buffer->tme_posix_disk_buffer_size);
    assert (rc == 0);
	
    /* free this buffer: */
    buffer->tme_posix_disk_buffer_size = 0;
#endif /* HAVE_MMAP */
  }

  /* otherwise, this buffer is not mmapped: */
  else {

    /* if this buffer is dirty, we need to write it out: */
    if (buffer->tme_posix_disk_buffer_flags 
	& TME_POSIX_DISK_BUFFER_DIRTY) {
	    
      /* seek to the buffer's position: */
      rc = (lseek(posix_disk->tme_posix_disk_fd,
		  buffer->tme_posix_disk_buffer_pos,
		  SEEK_SET) < 0);
      assert (rc == 0);
	    
      /* write out the buffer: */
      ssize = write(posix_disk->tme_posix_disk_fd,
		    buffer->tme_posix_disk_buffer_data,
		    buffer->tme_posix_disk_buffer_size);
      assert (ssize == buffer->tme_posix_disk_buffer_size);
    }

    /* free this buffer: */
    buffer->tme_posix_disk_buffer_flags = 0;
  }
#undef ssize
}

/* this gets a buffer: */
static int
_tme_posix_disk_buffer_get(struct tme_posix_disk *posix_disk,
			   const union tme_value64 *_pos,
			   unsigned long _size,
			   int readable,
			   tme_uint8_t **_buffer)
{
  struct stat statbufs[3];
#define pos_least statbufs[0].st_size
#define pos_most statbufs[1].st_size
#define pos_last statbufs[2].st_size
  struct iovec iovecs[3];
#define data iovecs[0].iov_base
#define size iovecs[0].iov_len
#define size_agg iovecs[1].iov_len
#define ssize iovecs[2].iov_len
  unsigned long agg_pre, agg_post;
  struct tme_posix_disk_buffer *buffer;
  struct tme_posix_disk_buffer *buffer_free_nosize;
  struct tme_posix_disk_buffer *buffer_free_sized;
  int have_least, have_most;
  int rc;

  /* form the size, and least and most positions: */
  size = _size;
  assert (size > 0);
#ifdef TME_HAVE_INT64_T
  pos_least = _pos->tme_value64_int;
#else  /* !TME_HAVE_INT64_T */
  pos_least = _pos->tme_value64_int32_hi;
  pos_least
    = ((pos_least << 32)
       | _pos->tme_value64_int32_lo);
#endif /* !TME_HAVE_INT64_T */
  pos_most = (pos_least + size) - 1;

  /* if we have to fill a new buffer, decide from where and how much
     we are going to fill: */
  agg_pre
    = TME_MIN(pos_least,
	      posix_disk->tme_posix_disk_buffer_agg_pre);
  agg_pre
    += ((pos_least - agg_pre)
	& (posix_disk->tme_posix_disk_stat.st_blksize - 1));
  agg_post
    = posix_disk->tme_posix_disk_buffer_agg_post;
  size_agg
    = (((agg_pre + size) + agg_post
	+ (posix_disk->tme_posix_disk_stat.st_blksize - 1))
       & ~(posix_disk->tme_posix_disk_stat.st_blksize - 1));
  agg_post = (size_agg - (agg_pre + size));

  /* start with no best free buffers: */
  buffer_free_nosize = NULL;
  buffer_free_sized = NULL;

  /* walk all of the buffers: */
  for (buffer = posix_disk->tme_posix_disk_buffers;
       buffer != NULL; ) {

    /* a buffer with no size is free: */
    if (buffer->tme_posix_disk_buffer_size == 0) {
      
      /* remember this nosize free buffer and continue: */
      buffer_free_nosize = buffer;
      buffer = buffer->tme_posix_disk_buffer_next;
      continue;
    }

    /* a buffer with some size, but no flags, is also free: */
    else if (buffer->tme_posix_disk_buffer_flags == 0) {

      /* take this buffer as our best sized free buffer, */

      /* if we have no best sized free buffer yet: */
      if (buffer_free_sized == NULL

	  || ((buffer->tme_posix_disk_buffer_size
	       >= buffer_free_sized->tme_posix_disk_buffer_size)

	      /* if this buffer is bigger than our best sized free
		 buffer, and the best sized free buffer is smaller
		 than the new-fill aggregate buffer size: */
	      ? (buffer_free_sized->tme_posix_disk_buffer_size
		 < size_agg)

	      /* if this buffer is smaller than our best sized
		 free buffer, but it is still at least as big as
		 the new-fill aggregate buffer size: */
	      : (buffer->tme_posix_disk_buffer_size
		 >= size_agg))) {
	buffer_free_sized = buffer;
      }

      /* continue: */
      buffer = buffer->tme_posix_disk_buffer_next;
      continue;
    }

    /* calculate the last position in this buffer: */
    pos_last = ((buffer->tme_posix_disk_buffer_pos
		 + buffer->tme_posix_disk_buffer_size)
		- 1);

    /* see if this buffer contains the least position we want: */
    have_least
      = (buffer->tme_posix_disk_buffer_pos <= pos_least
	 && pos_least <= pos_last);

    /* see if this buffer contains the most position we want: */
    have_most
      = (buffer->tme_posix_disk_buffer_pos <= pos_most
	 && pos_most <= pos_last);

    /* if this buffer covers all of the positions we want, and
       is readable or we don't need a readable buffer, stop: */
    if (have_least
	&& have_most
	&& ((buffer->tme_posix_disk_buffer_flags
	     & TME_POSIX_DISK_BUFFER_READABLE)
	    || !readable)) {
      break;
    }
    
    /* otherwise, if this buffer covers any of the positions we want,
       or if this is the last buffer on the list and we don't have any
       best free buffer, we need to free this buffer: */
    else if (have_least
	     || have_most
	     || (buffer->tme_posix_disk_buffer_next == NULL
		 && buffer_free_nosize == NULL
		 && buffer_free_sized == NULL)) {

      /* free this buffer: */
      _tme_posix_disk_buffer_free(posix_disk,
				  buffer);
      
      /* continue without updating buffer, so that this now-free
	 buffer is revisited, to possibly become a best free buffer: */
    }

    /* otherwise, this is a non-free buffer that doesn't
       cover any of the positions we want.  just continue: */
    else {
      buffer = buffer->tme_posix_disk_buffer_next;
    }
  }

  /* if we didn't find an applicable buffer: */
  if (buffer == NULL) {
    
    /* we must have some free buffer: */
    assert (buffer_free_nosize != NULL
	    || buffer_free_sized != NULL);

#ifdef HAVE_MMAP

    /* try to mmap this region.  if the map fails with more read-ahead
       than is needed to meet the block size, try the map one more
       time with just that needed read-ahead: */
    data = mmap(NULL,
		size_agg,
		PROT_READ
		| ((posix_disk->tme_posix_disk_flags
		    & TME_POSIX_DISK_FLAG_RO)
		   ? 0
		   : PROT_WRITE),
		MAP_SHARED,
		posix_disk->tme_posix_disk_fd,
		(pos_least
		 - agg_pre));
    if (data == MAP_FAILED) {
      size_agg
	= (((agg_pre + size)
	    + (posix_disk->tme_posix_disk_stat.st_blksize - 1))
	   & ~(posix_disk->tme_posix_disk_stat.st_blksize - 1));
      data = mmap(NULL,
		  size_agg,
		  PROT_READ
		  | ((posix_disk->tme_posix_disk_flags
		      & TME_POSIX_DISK_FLAG_RO)
		     ? 0
		     : PROT_WRITE),
		  MAP_SHARED,
		  posix_disk->tme_posix_disk_fd,
		  (pos_least
		   - agg_pre));
      if (data == MAP_FAILED) {
	size_agg = (agg_pre + size) + agg_post;
      }
      else {
	agg_post = size_agg - (agg_pre + size);
      }
    }

    /* if we were able to mmap this region: */
    if (data != MAP_FAILED) {

      /* if we have a free nosize buffer, reuse it, else free the
	 free sized buffer's data and reuse that: */
      if (buffer_free_nosize != NULL) {
	buffer = buffer_free_nosize;
      }
      else {
	tme_free(buffer_free_sized->tme_posix_disk_buffer_data);
	buffer = buffer_free_sized;
      }

      /* do the mmapped-specific initialization of this buffer: */
      buffer->tme_posix_disk_buffer_flags
	= (TME_POSIX_DISK_BUFFER_READABLE
	   | TME_POSIX_DISK_BUFFER_MMAPPED);
    }

    /* otherwise, we were unable to map this region: */
    else
#endif /* HAVE_MMAP */
      {

	/* if we have a free sized buffer, resize it, else malloc 
	   data for the free nosize buffer: */
	if (buffer_free_sized != NULL) {
	  data = buffer_free_sized->tme_posix_disk_buffer_data;
	  if (buffer_free_sized->tme_posix_disk_buffer_size
	      != size_agg) {
	    data = tme_realloc(data, size_agg);
	  }
	  buffer = buffer_free_sized;
	}
	else {
	  data = tme_malloc(size_agg);
	  buffer = buffer_free_nosize;
	}

	/* do the malloced-specific initialization of this buffer: */
	buffer->tme_posix_disk_buffer_flags = 0;
	if (readable) {
	  buffer->tme_posix_disk_buffer_flags
	    = TME_POSIX_DISK_BUFFER_READABLE;

	  /* seek to the buffer's position: */
	  rc = (lseek(posix_disk->tme_posix_disk_fd,
		      (pos_least
		       - agg_pre),
		      SEEK_SET) < 0);
	  assert (rc == 0);
	  
	  /* read in the buffer.  if the read fails with more
	     read-ahead than is needed to meet the block size, try the
	     read one more time with just that needed read-ahead: */
	  for (;;) {
	    ssize = read(posix_disk->tme_posix_disk_fd,
			 data,
			 size_agg);
	    if (ssize == size_agg) {
	      break;
	    }
	    size_agg
	      = (((agg_pre + size)
		  + (posix_disk->tme_posix_disk_stat.st_blksize - 1))
		 & ~(posix_disk->tme_posix_disk_stat.st_blksize - 1));
	    assert (agg_post > (size_agg - (agg_pre + size)));
	    agg_post = (size_agg - (agg_pre + size));
	  }
	}
      }

    /* do the common initialization of this buffer: */
    buffer->tme_posix_disk_buffer_pos
      = (pos_least
	 - agg_pre);
    buffer->tme_posix_disk_buffer_size
      = size_agg;
    buffer->tme_posix_disk_buffer_data
      = data;
  }

  /* if this buffer doesn't need to be readable, that means that it's
     being written to, so mark it dirty: */
  if (!readable) {
    buffer->tme_posix_disk_buffer_flags
      |= TME_POSIX_DISK_BUFFER_DIRTY;
  }

  /* remove this buffer from the list: */
  *buffer->tme_posix_disk_buffer_prev
    = buffer->tme_posix_disk_buffer_next;
  if (buffer->tme_posix_disk_buffer_next != NULL) {
    buffer->tme_posix_disk_buffer_next->tme_posix_disk_buffer_prev
      = buffer->tme_posix_disk_buffer_prev;
  }

  /* add this buffer to the front of the list: */
  buffer->tme_posix_disk_buffer_prev
    = &posix_disk->tme_posix_disk_buffers;
  buffer->tme_posix_disk_buffer_next
    = posix_disk->tme_posix_disk_buffers;
  if (buffer->tme_posix_disk_buffer_next != NULL) {
    buffer->tme_posix_disk_buffer_next->tme_posix_disk_buffer_prev
      = &buffer->tme_posix_disk_buffer_next;
  }
  *buffer->tme_posix_disk_buffer_prev
    = buffer;

  /* return the desired pointer into the buffer: */
  *_buffer
    = (buffer->tme_posix_disk_buffer_data
       + (pos_least
	  - buffer->tme_posix_disk_buffer_pos));

  return (TME_OK);
#undef pos_least
#undef pos_most
#undef data
#undef size
#undef size_agg
}

/* this returns a read buffer: */
static int
_tme_posix_disk_read(struct tme_disk_connection *conn_disk,
		     const union tme_value64 *pos,
		     unsigned long size,
		     const tme_uint8_t **_buffer)
{
  struct tme_posix_disk *posix_disk;
  tme_uint8_t *buffer;
  int rc;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) conn_disk->tme_disk_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_disk->tme_posix_disk_mutex);

  /* get the buffer: */
  rc = _tme_posix_disk_buffer_get(posix_disk,
				  pos,
				  size,
				  TRUE,
				  &buffer);
  assert (rc == TME_OK);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_disk->tme_posix_disk_mutex);

  /* return the buffer: */
  *_buffer = buffer;
  return (TME_OK);
}

/* this returns a write buffer: */
static int
_tme_posix_disk_write(struct tme_disk_connection *conn_disk,
		      const union tme_value64 *pos,
		      unsigned long size,
		      tme_uint8_t **_buffer)
{
  struct tme_posix_disk *posix_disk;
  int rc;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) conn_disk->tme_disk_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_disk->tme_posix_disk_mutex);

  /* get the buffer: */
  rc = _tme_posix_disk_buffer_get(posix_disk,
				  pos,
				  size,
				  FALSE,
				  _buffer);
  assert (rc == TME_OK);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_disk->tme_posix_disk_mutex);

  return (TME_OK);
}

/* the disk control handler: */
#ifdef HAVE_STDARG_H
static int _tme_posix_disk_control(struct tme_disk_connection *conn_disk,
				   unsigned int control,
				   ...)
#else  /* HAVE_STDARG_H */
static int _tme_posix_disk_control(conn_disk, control, va_alist)
     struct tme_disk_connection *conn_disk;
     unsigned int control;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  struct tme_posix_disk *posix_disk;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) conn_disk->tme_disk_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_disk->tme_posix_disk_mutex);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_disk->tme_posix_disk_mutex);

  return (TME_OK);
}

/* this opens a disk: */
static int
_tme_posix_disk_open(struct tme_posix_disk *posix_disk,
		     const char *filename,
		     int flags,
		     char **_output)
{
  int fd;
  struct stat statbuf;
  tme_uint8_t *block;
#ifdef HAVE_MMAP
  int page_size;
#endif /* HAVE_MMAP */

  /* open the file: */
  fd = open(filename,
	    ((flags
	      & TME_POSIX_DISK_FLAG_RO)
	     ? O_RDONLY
	     : O_RDWR));
  if (fd < 0) {
    tme_output_append_error(_output,
			    "%s",
			    filename);
    return (errno);
  }

  /* stat the file: */
  if (fstat(fd, &statbuf) < 0) {
    tme_output_append_error(_output,
			    "%s",
			    filename);
    close(fd);
    return (errno);
  }

  /* we will handle a character device, but not a block device: */
  if (S_ISBLK(statbuf.st_mode)) {
    tme_output_append_error(_output,
			    "%s",
			    filename);
    close(fd);
    return (EINVAL);
  }

  /* if this is a character device, determine its block size: */
  if (S_ISCHR(statbuf.st_mode)) {

    /* the block size must be at least one: */
    statbuf.st_blksize = 1;

    /* allocate space for the block: */
    block = tme_new(tme_uint8_t, statbuf.st_blksize);

    /* loop trying to read a block at offset zero, doubling the block
       size until we succeed: */
    for (; statbuf.st_blksize <= TME_POSIX_DISK_BLOCK_SIZE_MAX; ) {

      /* do the read: */
      if (read(fd, block, statbuf.st_blksize) >= 0) {
	break;
      }

      /* seek back to the beginning: */
      if (lseek(fd, 0, SEEK_SET) < 0) {
	tme_free(block);
	tme_output_append_error(_output,
				"%s",
				filename);
	close(fd);
	return (errno);
      }

      /* resize the block: */
      statbuf.st_blksize <<= 1;
      block = tme_renew(tme_uint8_t, block, statbuf.st_blksize);
    }

    /* free the block: */
    tme_free(block);

    /* if we failed: */
    if (statbuf.st_blksize > TME_POSIX_DISK_BLOCK_SIZE_MAX) {
      tme_output_append_error(_output,
			      "%s",
			      filename);
      close(fd);
      return (EINVAL);
    }
  }

#ifdef HAVE_MMAP
  /* if we're mmapping, the block size must be at least the page size: */
  for (page_size = getpagesize();
       page_size < statbuf.st_blksize;
       page_size <<= 1);
  statbuf.st_blksize = page_size;
#endif /* HAVE_MMAP */

  /* update the disk structure: */
  posix_disk->tme_posix_disk_flags = flags;
  posix_disk->tme_posix_disk_fd = fd;
  posix_disk->tme_posix_disk_stat = statbuf;
  return (TME_OK);
}

/* this closes a disk: */
static void
_tme_posix_disk_close(struct tme_posix_disk *posix_disk)
{
  struct tme_posix_disk_buffer *buffer;

  /* free all of the buffers: */
  for (buffer = posix_disk->tme_posix_disk_buffers;
       buffer != NULL;
       buffer = buffer->tme_posix_disk_buffer_next) {
    _tme_posix_disk_buffer_free(posix_disk,
				buffer);
  }

  /* close the disk: */
  close(posix_disk->tme_posix_disk_fd);
  posix_disk->tme_posix_disk_fd = -1;
}

/* our internal command function: */
static int
__tme_posix_disk_command(struct tme_posix_disk *posix_disk,
			 const char * const * args, 
			 char **_output)
{
  int usage;
  int arg_i;
  const char *filename;
  int flags;
  int rc;

  /* check the command: */
  usage = FALSE;
  arg_i = 1;

  /* the "load" command: */
  if (TME_ARG_IS(args[arg_i], "load")) {
    arg_i++;

    /* if a disk is currently loaded, it must be unloaded first: */
    if (posix_disk->tme_posix_disk_fd >= 0) {
      tme_output_append_error(_output,
			      _("%s: disk already loaded; must unload first"),
			      args[0]);
      return (EBUSY);
    }

    /* the first argument is the filename: */
    filename = args[arg_i];
    arg_i += (filename != NULL);

    /* any remaining arguments are flags: */
    flags = 0;
    for (;;) {
      
      /* the "read-only" flag: */
      if (TME_ARG_IS(args[arg_i], "read-only")) {
	flags |= TME_POSIX_DISK_FLAG_RO;
	arg_i++;
      }

      else {
	break;
      }
    }

    /* if we don't have a filename, or if there are more arguments: */
    if (filename == NULL
	|| args[arg_i] != NULL) {

      tme_output_append_error(_output,
			      "%s %s load { %s | %s } [read-only]",
			      _("usage:"),
			      args[0],
			      _("DEVICE"),
			      _("FILENAME"));
      rc = EINVAL;
    }

    /* otherwise, if we can open the disk: */
    else if ((rc = _tme_posix_disk_open(posix_disk,
					filename,
					flags,
					_output)) == TME_OK) {

      /* nothing to do */
    }
  }

  /* the "unload" command: */
  else if (TME_ARG_IS(args[arg_i], "unload")) {

    /* if no disk is currently loaded: */
    if (posix_disk->tme_posix_disk_fd < 0) {
      tme_output_append_error(_output,
			      _("%s: no disk loaded"),
			      args[0]);
      return (ENXIO);
    }

    /* we must have no arguments: */
    if (args[arg_i + 1] != NULL) {
      tme_output_append_error(_output,
			      "%s %s unload",
			      _("usage:"),
			      args[0]);
      return (EINVAL);
    }

    /* close the disk: */
    _tme_posix_disk_close(posix_disk);
    rc = TME_OK;
  }

  /* any other command: */
  else {
    if (args[arg_i] != NULL) {
      tme_output_append_error(_output,
			      "%s '%s', ",
			      _("unknown command"),
			      args[1]);
    }
    tme_output_append_error(_output,
			    _("available %s commands: %s"),
			    args[0],
			    "load unload");
    return (EINVAL);
  }

  return (rc);
}

/* our command function: */
static int
_tme_posix_disk_command(struct tme_element *element,
			const char * const * args, 
			char **_output)
{
  struct tme_posix_disk *posix_disk;
  int rc;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&posix_disk->tme_posix_disk_mutex);

  /* call the internal command function: */
  rc = __tme_posix_disk_command(posix_disk,
				args, 
				_output);

  /* unlock the mutex: */
  tme_mutex_unlock(&posix_disk->tme_posix_disk_mutex);

  return (rc);
}

/* this breaks a posix disk connection: */
static int
_tme_posix_disk_connection_break(struct tme_connection *conn,
				 unsigned int state)
{
  abort();
}

/* this makes a posix disk connection: */
static int
_tme_posix_disk_connection_make(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_posix_disk *posix_disk;
  struct tme_disk_connection *conn_disk;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) conn->tme_connection_element->tme_element_private;

  /* both sides must be disk connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_DISK);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_DISK);

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock the mutex: */
    tme_mutex_lock(&posix_disk->tme_posix_disk_mutex);

    /* save this connection to our list of connections: */
    conn_disk = (struct tme_disk_connection *) conn->tme_connection_other;
    posix_disk->tme_posix_disk_connection = conn_disk;

    /* unlock the mutex: */
    tme_mutex_unlock(&posix_disk->tme_posix_disk_mutex);
  }

  return (TME_OK);
}

/* this makes a new connection side for a posix disk: */
static int
_tme_posix_disk_connections_new(struct tme_element *element,
				const char * const *args,
				struct tme_connection **_conns,
				char **_output)
{
  struct tme_posix_disk *posix_disk;
  struct tme_disk_connection *conn_disk;
  struct tme_connection *conn;

  /* recover our data structure: */
  posix_disk = (struct tme_posix_disk *) element->tme_element_private;

  /* if we already have a connection, there's nothing to do: */
  if (posix_disk->tme_posix_disk_connection != NULL) {
    return (TME_OK);
  }

  /* create our side of a disk connection: */
  conn_disk = tme_new0(struct tme_disk_connection, 1);
  conn = &conn_disk->tme_disk_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_DISK;
  conn->tme_connection_score = tme_disk_connection_score;
  conn->tme_connection_make = _tme_posix_disk_connection_make;
  conn->tme_connection_break = _tme_posix_disk_connection_break;

  /* fill in the disk connection: */
  (void) tme_value64_set(&conn_disk->tme_disk_connection_size,
			 posix_disk->tme_posix_disk_stat.st_size);
  conn_disk->tme_disk_connection_read
    = _tme_posix_disk_read;
  if (!(posix_disk->tme_posix_disk_flags
	& TME_POSIX_DISK_FLAG_RO)) {
    conn_disk->tme_disk_connection_write
      = _tme_posix_disk_write;
  }
  conn_disk->tme_disk_connection_release
    = NULL;
  conn_disk->tme_disk_connection_control
    = _tme_posix_disk_control;
  
  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new posix disk function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_posix,disk) {
  const char *filename;
  int flags;
  unsigned long agg_pre;
  unsigned long agg_post;
  int buffers;
  struct tme_posix_disk *posix_disk;
  int rc;
  struct tme_posix_disk_buffer *buffer, **_prev;
  int arg_i;
  int usage;

  /* check our arguments: */
  filename = NULL;
  flags = 0;
  arg_i = 1;
  buffers = TME_POSIX_DISK_BUFFER_DEFAULT_COUNT;
  agg_pre = TME_POSIX_DISK_BUFFER_DEFAULT_AGG_PRE;
  agg_post = TME_POSIX_DISK_BUFFER_DEFAULT_AGG_POST;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the filename: */
    if (TME_ARG_IS(args[arg_i], "file")
	&& args[arg_i + 1] != NULL
	&& filename == NULL) {
      filename = args[arg_i + 1];
      arg_i += 2;
    }

    /* the read-only flag: */
    else if (TME_ARG_IS(args[arg_i], "read-only")) {
      flags |= TME_POSIX_DISK_FLAG_RO;
      arg_i++;
    }

    /* the buffers count: */
    else if (TME_ARG_IS(args[arg_i + 0], "buffers")
	     && args[arg_i + 1] != NULL
	     && (buffers = atoi(args[arg_i + 1])) > 0) {
      arg_i += 2;
    }

    /* the read-behind value, also called the aggregate-pre: */
    else if (TME_ARG_IS(args[arg_i + 0], "read-behind")) {
      agg_pre = tme_bus_addr_parse_any(args[arg_i + 1], &usage);
      if (usage) {
	break;
      }
      arg_i += 2;
    }

    /* the read-ahead value, also called the aggregate-post: */
    else if (TME_ARG_IS(args[arg_i + 0], "read-ahead")) {
      agg_post = tme_bus_addr_parse_any(args[arg_i + 1], &usage);
      if (usage) {
	break;
      }
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      /* we must have been given a filename: */
      if (filename == NULL) {
	usage = TRUE;
      }
      break;
    }

    /* this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s file %s [read-only] [buffers %s] [read-behind %s] [read-ahead %s]",
			    _("usage:"),
			    args[0],
			    _("FILENAME"),
			    _("BUFFER-COUNT"),
			    _("BYTE-COUNT"),
			    _("BYTE-COUNT"));
    return (EINVAL);
  }

  /* start the disk structure: */
  posix_disk = tme_new0(struct tme_posix_disk, 1);
  posix_disk->tme_posix_disk_element = element;
  tme_mutex_init(&posix_disk->tme_posix_disk_mutex);
  posix_disk->tme_posix_disk_buffer_agg_pre = agg_pre;
  posix_disk->tme_posix_disk_buffer_agg_post = agg_post;

  /* open the disk: */
  rc = _tme_posix_disk_open(posix_disk,
			    filename,
			    flags,
			    _output);
  if (rc != TME_OK) {
    tme_free(posix_disk);
    return (rc);
  }

  /* allocate the buffers: */
  for (_prev = &posix_disk->tme_posix_disk_buffers;
       buffers-- > 0;
       _prev = &buffer->tme_posix_disk_buffer_next) {
    buffer = tme_new0(struct tme_posix_disk_buffer, 1);
    buffer->tme_posix_disk_buffer_prev = _prev;
    *buffer->tme_posix_disk_buffer_prev = buffer;
  }
  *_prev = NULL;

  /* fill the element: */
  element->tme_element_private = posix_disk;
  element->tme_element_connections_new = _tme_posix_disk_connections_new;
  element->tme_element_command = _tme_posix_disk_command;

  return (TME_OK);
}
