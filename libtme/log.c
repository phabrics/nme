/* $Id: log.c,v 1.3 2009/08/30 17:04:51 fredette Exp $ */

/* libtme/log.c - logging functions: */

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

#include <tme/common.h>
_TME_RCSID("$Id: log.c,v 1.3 2009/08/30 17:04:51 fredette Exp $");

/* includes: */
#include <tme/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */
#include <tme/hash.h>

/* argument codes: */
#define TME_LOG_ARG_CODE_NULL			(0)
#define TME_LOG_ARG_CODE_STRING			(1)
#define TME_LOG_ARG_CODE_CHAR			(2)
#define TME_LOG_ARG_CODE_INT			(3)
#define TME_LOG_ARG_CODE_LONG_INT		(4)
#define TME_LOG_ARG_CODE_LONG_LONG_INT		(5)

/* if long long int is supported, this macro expands its argument,
   otherwise it doesn't: */
#if defined(__GNUC__) && (__GNUC__ >= 2) && (_TME_SIZEOF_INT == 4)
#define prf_lld(x) x
#else  /* defined(__GNUC__) && (__GNUC__ >= 2) && (_TME_SIZEOF_INT == 4) */
#define prf_lld(x) /**/
#endif /* defined(__GNUC__) && (__GNUC__ >= 2) && (_TME_SIZEOF_INT == 4) */

/* if tme_int64_t is supported, this macro expands its argument,
   otherwise it doesn't: */
#ifdef TME_HAVE_INT64_T
#define _TME_LOG_IF_INT64_T(x) x
#else  /* !TME_HAVE_INT64_T */
#define _TME_LOG_IF_INT64_T(x) /**/
#endif /* !TME_HAVE_INT64_T */

/* this returns nonzero if the type of the given expression can have
   any alignment: */
#define _TME_LOG_ALIGN_ANY(x)					\
  (sizeof(x) <= sizeof(tme_int32_t)				\
   ? _TME_ALIGNOF_INT32_T == 1					\
   _TME_LOG_IF_INT64_T(: sizeof(x) == sizeof(tme_int64_t)	\
		       ? _TME_ALIGNOF_INT64_T == 1)		\
   : FALSE)

/* the locals needed for log-prf.c: */
#define LOG_PRF_LOCALS				\
  va_list prf_args;				\
  int prf_state;				\
  const char *prf_agg;				\
  char prf_char;				\
  int prf_width;				\
  const char *prf_flag_ls;			\
  int prf_flag_0;				\
  int prf_digit;				\
  int prf_value_d;				\
  long int prf_value_ld;			\
  prf_lld(long long int prf_value_lld;)		\
  const char *prf_value_s;			\
  char prf_value_c;				\
  char *prf_value_buffer;			\
  /* the largest prf_format_buffer needed is:	\
     % 0 INT ll CONVERSION NUL: */		\
  char prf_format_buffer[1 + 1 + (sizeof(int) * 3) + 2 + 1 + 1]

/* the argument code type: */
typedef tme_uint8_t tme_log_arg_code_t;

/* this appends raw output: */
void
tme_output_append_raw(char **_output, const char *output_new, unsigned int output_new_length)
{
  char *output;
  unsigned int output_length;
  int saved_errno;

  saved_errno = errno;
  output = *_output;
  if (output == NULL) {
    output_length = 0;
    output = tme_new(char, output_new_length + 1);
  }
  else {
    output_length = strlen(output);
    output = tme_renew(char, output, output_length + output_new_length + 1);
  }
  memcpy(output + output_length, output_new, output_new_length);
  output[output_length + output_new_length] = '\0';
  *_output = output;
  errno = saved_errno;
}

/* this prepends raw output: */
void
tme_output_prepend_raw(char **_output, const char *output_new, unsigned int output_new_length)
{
  char *output;
  unsigned int output_length;
  int saved_errno;

  saved_errno = errno;
  output = *_output;
  if (output == NULL) {
    output_length = 0;
    output = tme_new(char, output_new_length + 1);
  }
  else {
    output_length = strlen(output);
    output = tme_renew(char, output, output_length + output_new_length + 1);
  }
  memmove(output + output_new_length, output, output_length);
  memcpy(output, output_new, output_new_length);
  output[output_length + output_new_length] = '\0';
  *_output = output;
  errno = saved_errno;
}

/* this appends or prepends a character: */
static void
tme_output_xpend_char(char **_output, char output_char, int prepend)
{
  char *output;
  unsigned int output_length;
  int saved_errno;

  saved_errno = errno;
  output = *_output;
  if (output == NULL) {
    output_length = 0;
    output = tme_new(char, 2);
  }
  else {
    output_length = strlen(output);
    output = tme_renew(char, output, output_length + 2);
  }
  if (prepend) {
    memmove(output + 1, output, output_length);
    output[0] = output_char;
  }
  else {
    output[output_length] = output_char;
  }
  output[output_length + 1] = '\0';
  *_output = output;
  errno = saved_errno;
}

/* this appends printf-style output: */
#ifdef HAVE_STDARG_H
void tme_output_append(char **_output, const char *prf_format, ...)
#else  /* HAVE_STDARG_H */
void tme_output_append(_output, prf_format, va_alist)
     char **_output;
     const char *prf_format;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  LOG_PRF_LOCALS;

#define PRF_OUT_ARG_CODE(arg_code) do { } while (/* CONSTCOND */ 0 && (arg_code))
#define PRF_OUT_MEM(s, len) tme_output_append_raw(_output, s, len)
#define PRF_OUT_CHAR(c) tme_output_xpend_char(_output, c, FALSE)

  do
#include "log-prf.c"
  while (/* CONSTCOND */ 0);

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR
}

/* this appends printf-style output for an error: */
#ifdef HAVE_STDARG_H
void tme_output_append_error(char **_output, const char *prf_format, ...)
#else  /* HAVE_STDARG_H */
void tme_output_append_error(_output, prf_format, va_alist)
     char **_output;
     const char *prf_format;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  LOG_PRF_LOCALS;

#define PRF_OUT_ARG_CODE(arg_code) do { } while (/* CONSTCOND */ 0 && (arg_code))
#define PRF_OUT_MEM(s, len) tme_output_append_raw(_output, s, len)
#define PRF_OUT_CHAR(c) tme_output_xpend_char(_output, c, FALSE)

  do
#include "log-prf.c"
  while (/* CONSTCOND */ 0);

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR
}

/* this prepends printf-style output: */
#ifdef HAVE_STDARG_H
void tme_output_prepend(char **_output, const char *prf_format, ...)
#else  /* HAVE_STDARG_H */
void tme_output_prepend(_output, prf_format, va_alist)
     char **_output;
     const char *prf_format;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  LOG_PRF_LOCALS;

#define PRF_OUT_ARG_CODE(arg_code) do { } while (/* CONSTCOND */ 0 && (arg_code))
#define PRF_OUT_MEM(s, len) tme_output_prepend_raw(_output, s, len)
#define PRF_OUT_CHAR(c) tme_output_xpend_char(_output, c, TRUE)

  do
#include "log-prf.c"
  while (/* CONSTCOND */ 0);

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR
}

/* this prepends printf-style output for an error: */
#ifdef HAVE_STDARG_H
void tme_output_prepend_error(char **_output, const char *prf_format, ...)
#else  /* HAVE_STDARG_H */
void tme_output_prepend_error(_output, prf_format, va_alist)
     char **_output;
     const char *prf_format;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  LOG_PRF_LOCALS;

#define PRF_OUT_ARG_CODE(arg_code) do { } while (/* CONSTCOND */ 0 && (arg_code))
#define PRF_OUT_MEM(s, len) tme_output_prepend_raw(_output, s, len)
#define PRF_OUT_CHAR(c) tme_output_xpend_char(_output, c, TRUE)

  do
#include "log-prf.c"
  while (/* CONSTCOND */ 0);

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR
}

/* this saves an argument code: */
static void
_tme_log_arg_code(tme_log_arg_code_t **_arg_codes,
		  tme_log_arg_code_t arg_code)
{
  tme_log_arg_code_t *arg_codes;
  unsigned long arg_code_count;

  /* count the existing argument codes: */
  arg_codes = *_arg_codes;
  for (arg_code_count = 0;
       arg_codes[arg_code_count] != TME_LOG_ARG_CODE_NULL;
       arg_code_count++);

  /* resize the argument codes: */
  arg_codes = tme_renew(tme_log_arg_code_t, arg_codes, arg_code_count + 1);
  *_arg_codes = arg_codes;

  /* add in this new argument code: */
  arg_codes[arg_code_count] = arg_code;
  arg_codes[arg_code_count + 1] = TME_LOG_ARG_CODE_NULL;
}

/* this logs printf-style output: */
#ifdef HAVE_STDARG_H
void tme_log_part(struct tme_log_handle *handle, const char *prf_format, ...)
#else  /* HAVE_STDARG_H */
void tme_log_part(handle, prf_format, va_alist)
     struct tme_log_handle *handle;
     const char *prf_format;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  LOG_PRF_LOCALS;
  char *message;
  unsigned long message_size;
  const char *prf_format_saved;
  tme_log_arg_code_t *arg_codes;
  tme_log_arg_code_t arg_code;
  unsigned long string_length;
  int saved_errno;

  /* do nothing if we have no format: */
  if (prf_format == NULL) {
    return;
  }

  /* if this handle is in binary mode: */
  if (handle->tme_log_handle_mode == TME_LOG_MODE_BINARY) {

    /* get the current message: */
    message = handle->tme_log_handle_message;
    message_size = handle->tme_log_handle_message_size;

    /* add the format to the message: */
    prf_format_saved = prf_format;
    assert ((message_size + sizeof(prf_format_saved)) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
    if (_TME_LOG_ALIGN_ANY(prf_format_saved)) {
      *((const char **) (message + message_size)) = prf_format_saved;
    }
    else {
      memcpy(message + message_size,
	     &prf_format_saved,
	     sizeof(prf_format_saved));
    }
    message_size += sizeof(prf_format_saved);

    /* if this format isn't known: */
    arg_codes = tme_hash_lookup(handle->tme_log_handle_hash_format, (void *) prf_format);
    if (__tme_predict_false(arg_codes == NULL)) {

      /* start the argument codes: */
      arg_codes = tme_new(tme_log_arg_code_t, 1);
      arg_codes[0] = TME_LOG_ARG_CODE_NULL;

      /* get the argument codes for this format: */
#define PRF_OUT_ARG_CODE(arg_code) _tme_log_arg_code(&arg_codes, (arg_code))
#define PRF_OUT_MEM(s, len) do { } while (/* CONSTCOND */ 0 && (s) && (len))
#define PRF_OUT_CHAR(c) do { } while (/* CONSTCOND */ 0 && (c))

      saved_errno = errno;
      do
#include "log-prf.c"
      while (/* CONSTCOND */ 0);
      errno = saved_errno;

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR

      /* insert this format into the hash: */
      tme_hash_insert(handle->tme_log_handle_hash_format, (void *) prf_format_saved, arg_codes);
    }
  
    /* start the variable arguments: */
#ifdef HAVE_STDARG_H
    va_start(prf_args, prf_format);
#else  /* HAVE_STDARG_H */
    va_start(prf_args);
#endif /* HAVE_STDARG_H */

    /* while we have variable arguments: */
    for (; (arg_code = *(arg_codes++)) != TME_LOG_ARG_CODE_NULL; ) {

      /* if this argument is a string: */
      if (__tme_predict_false(arg_code == TME_LOG_ARG_CODE_STRING)) {

	/* get the string argument: */
	prf_value_s = va_arg(prf_args, const char *);
	string_length = strlen(prf_value_s);

	/* add the string argument to the message: */
	assert ((message_size + string_length + 1) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
	memcpy(message + message_size,
	       prf_value_s,
	       string_length + 1);
	message_size += string_length + 1;
      }

      /* otherwise, if this argument is a char: */
      else if (arg_code == TME_LOG_ARG_CODE_CHAR) {

	/* get the (promoted) int-sized integral argument: */
	prf_value_d = va_arg(prf_args, int);

	/* add the char-sized argument to the message: */
	assert ((message_size + sizeof(char)) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
	*((char *) (message + message_size)) = prf_value_d;
	message_size += sizeof(char);
      }

      /* otherwise, if this argument has the same size as an int: */
      /* NB: this assumes that all integral types that have the same
	 size as an int are equivalent for va_arg(): */
      else if (sizeof(int) != sizeof(long)
	       ? arg_code == TME_LOG_ARG_CODE_INT
	       : (prf_lld(arg_code != TME_LOG_ARG_CODE_LONG_LONG_INT &&) TRUE)) {

	/* get the int-sized integral argument: */
	prf_value_d = va_arg(prf_args, int);

	/* add the int-sized argument to the message: */
	assert ((message_size + sizeof(prf_value_d)) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
	if (_TME_LOG_ALIGN_ANY(prf_value_d)) {
	  *((int *) (message + message_size)) = prf_value_d;
	}
	else {
	  memcpy(message + message_size,
		 &prf_value_d,
		 sizeof(prf_value_d));
	}
	message_size += sizeof(prf_value_d);
      }

      /* otherwise, if this argument has the same size as a long int: */
      /* NB: this assumes that all integral types that have the same
	 size as a long int are equivalent for va_arg(): */
      else if (sizeof(int) != sizeof(long)
	       prf_lld(&& (((sizeof(int) * 2) == sizeof(long))
			   || arg_code == TME_LOG_ARG_CODE_LONG_INT))) {

	/* get the long-int-sized integral argument: */
	prf_value_ld = va_arg(prf_args, long int);

	/* add the long-int-sized argument to the message: */
	assert ((message_size + sizeof(prf_value_ld)) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
	if (_TME_LOG_ALIGN_ANY(prf_value_ld)) {
	  *((long int *) (message + message_size)) = prf_value_ld;
	}
	else {
	  memcpy(message + message_size,
		 &prf_value_ld,
		 sizeof(prf_value_ld));
	}
	message_size += sizeof(prf_value_ld);
      }

      /* otherwise, this argument must have the same size as a long
	 long int: */
      /* NB: this assumes that all integral types that have the same
	 size as a long int are equivalent for va_arg(): */
      else {
	assert (arg_code == TME_LOG_ARG_CODE_LONG_LONG_INT);

#if (0 prf_lld(+ 1))

	/* get the long-long-int-sized integral argument: */
	prf_value_lld = va_arg(prf_args, long long int);

	/* add the long-long-int-sized argument to the message: */
	assert ((message_size + sizeof(prf_value_lld)) <= TME_LOG_MESSAGE_SIZE_MAX_BINARY);
	if (_TME_LOG_ALIGN_ANY(prf_value_lld)) {
	  *((long long int *) (message + message_size)) = prf_value_lld;
	}
	else {
	  memcpy(message + message_size,
		 &prf_value_lld,
		 sizeof(prf_value_lld));
	}
	message_size += sizeof(prf_value_lld);

#else  /* long long int not supported */
	abort();
#endif /* long long int not supported */
      }
    }

    /* end the variable arguments: */
    va_end(prf_args);

    /* update the current message: */
    handle->tme_log_handle_message_size = message_size;

    return;
  }

#define PRF_OUT_ARG_CODE(arg_code) do { } while (/* CONSTCOND */ 0 && (arg_code))
#define PRF_OUT_MEM(s, len) tme_output_append_raw(&handle->tme_log_handle_message, s, len)
#define PRF_OUT_CHAR(c) tme_output_xpend_char(&handle->tme_log_handle_message, c, FALSE)

  saved_errno = errno;
  do
#include "log-prf.c"
  while (/* CONSTCOND */ 0);
  errno = saved_errno;

#undef PRF_OUT_ARG_CODE
#undef PRF_OUT_MEM
#undef PRF_OUT_CHAR
}
