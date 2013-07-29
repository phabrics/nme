/* $Id: log.h,v 1.2 2009/08/30 13:23:39 fredette Exp $ */

/* tme/log.h - public header file for logging: */

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

#ifndef _TME_LOG_H
#define _TME_LOG_H

#include <tme/common.h>
_TME_RCSID("$Id: log.h,v 1.2 2009/08/30 13:23:39 fredette Exp $");

/* includes: */

/* macros: */

/* if TME_NO_LOG has been specified, shut down all logging except for
   level zero: */
#ifdef TME_NO_LOG
#undef TME_LOG_LEVEL_MAX
#define TME_LOG_LEVEL_MAX	(0)
#endif /* TME_NO_LOG */

/* non-ancient GCCs can check the arguments to our printf-like
   functions: */
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5))
#define __tme_printf_attribute __attribute__ ((format (printf, 2, 3)))
#else
#define __tme_printf_attribute
#endif

/* log modes: */
#define TME_LOG_MODE_TEXT		(0)
#define TME_LOG_MODE_BINARY		(1)

/* maximum message sizes: */
#define TME_LOG_MESSAGE_SIZE_MAX_BINARY	(1024)

/* a logging handle: */
struct tme_log_handle {
  
  /* the maximum log level output: */
  unsigned long tme_log_handle_level_max;

  /* the log level of this message: */
  unsigned long tme_log_handle_level;

  /* the log message: */
  char *tme_log_handle_message;
  unsigned long tme_log_handle_message_size;

  /* any errno with this message: */
  int tme_log_handle_errno;

  /* the interface's private data: */
  void *tme_log_handle_private;

  /* the interface's log output function: */
  void (*tme_log_handle_output) _TME_P((struct tme_log_handle *));

  /* the mode of this handle: */
  unsigned int tme_log_handle_mode;

  /* a format hash: */
  struct _tme_hash *tme_log_handle_hash_format;
};

/* prototypes and macros: */

/* this starts logging a message: */
#ifdef TME_LOG_LEVEL_MAX
#define _tme_log_start(handle, level)			\
  ((level) <= TME_LOG_LEVEL_MAX				\
    && (level) <= (handle)->tme_log_handle_level_max)
#else  /* !TME_LOG_LEVEL_MAX */
#define _tme_log_start(handle, level)			\
  ((level) <= (handle)->tme_log_handle_level_max)
#endif /* !TME_LOG_LEVEL_MAX */
#define tme_log_start(handle, level, rc)		\
do {							\
  if (_tme_log_start(handle, level)) {			\
    (handle)->tme_log_handle_level = (level);		\
    (handle)->tme_log_handle_errno = (rc);		\
    do

/* this logs (part of) the body of a message: */
void tme_log_part _TME_P((struct tme_log_handle *, _tme_const char *, ...)) __tme_printf_attribute;

/* this finishes logging a message: */
#define tme_log_finish(handle)				\
    while (/* CONSTCOND */ 0);				\
    (*(handle)->tme_log_handle_output)(handle);		\
  }							\
} while (/* CONSTCOND */ 0)

/* this handles all of the logging of a message: */
#define tme_log(handle, level, rc, x)			\
do {							\
  tme_log_start(handle, level, rc) {			\
    tme_log_part x;					\
  } tme_log_finish(handle);				\
} while (/* CONSTCOND */ 0)

/* these collect output: */
void tme_output_append_raw _TME_P((char **, const char *, unsigned int));
void tme_output_prepend_raw _TME_P((char **, const char *, unsigned int));
void tme_output_append _TME_P((char **, const char *, ...)) __tme_printf_attribute;
void tme_output_prepend _TME_P((char **, const char *, ...)) __tme_printf_attribute;
void tme_output_append_error _TME_P((char **, const char *, ...)) __tme_printf_attribute;
void tme_output_prepend_error _TME_P((char **, const char *, ...)) __tme_printf_attribute;

#endif /* !_TME_LOG_H */
