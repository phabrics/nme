/* $Id: misc.c,v 1.8 2010/06/05 19:02:38 fredette Exp $ */

/* libtme/misc.c - miscellaneous: */

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
_TME_RCSID("$Id: misc.c,v 1.8 2010/06/05 19:02:38 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/module.h>
#include <tme/misc.h>
#include <ctype.h>

/* this initializes libtme: */
int
tme_init(void)
{
  int rc;
  
  /* initialize the threading system: */
  tme_threads_init();

  /* initialize the module system: */
  tme_module_init();

  rc = TME_OK;
  return (rc);
}

/* this tokenizes a string by whitespace: */
char **
tme_misc_tokenize(const char *string,
		  char comment,
		  int *_tokens_count)
{
  int tokens_count;
  int tokens_size;
  char **tokens;
  const char *p1;
  const char *p2;
  char c;

  /* we initially have no tokens: */
  tokens_count = 0;
  tokens_size = 1;
  tokens = tme_new(char *, tokens_size);

  /* tokenize this line by whitespace and watch for comments: */
  p1 = NULL;
  for (p2 = string;; p2++) {
    c = *p2;

    /* if this is a token delimiter: */
    if (c == '\0'
	|| isspace((unsigned char) c)
	|| c == comment) {

      /* if we had been collecting a token, it's finished: */
      if (p1 != NULL) {
	  
	/* save this token: */
	tokens[tokens_count] = tme_dup(char, p1, (p2 - p1) + 1);
	tokens[tokens_count][p2 - p1] = '\0';
	p1 = NULL;

	/* resize the tokens array if needed: */
	if (++tokens_count == tokens_size) {
	  tokens_size += (tokens_size >> 1) + 1;
	  tokens = tme_renew(char *, tokens, tokens_size);
	}
      }

      /* stop if this is the end of the line or the beginning of a
	 comment: */
      if (c == '\0'
	  || c == comment) {
	break;
      }
    }

    /* otherwise this is part of a token: */
    else {
      if (p1 == NULL) {
	p1 = p2;
      }
    }
  }

  /* done: */
  *_tokens_count = tokens_count;
  tokens[tokens_count] = NULL;
  return (tokens);
}

/* this frees an array of strings: */
void
tme_free_string_array(char **array, int length)
{
  char *string;
  int i;

  if (length < 0) {
    for (i = 0;
	 (string = array[i]) != NULL;
	 i++) {
      tme_free(string);
    }
  }
  else {
    for (i = 0;
	 i < length;
	 i++) {
      tme_free(array[i]);
    }
  }
  tme_free(array);
}
    
#ifdef TME_HAVE_INT64_T
#define _tme_unumber_t tme_uint64_t
#define _tme_number_t tme_int64_t
#else  /* !TME_HAVE_INT64_T */
#define _tme_unumber_t tme_uint32_t
#define _tme_number_t tme_int32_t
#endif /* !TME_HAVE_INT64_T */

/* this internal function parses a number: */
static _tme_unumber_t
_tme_misc_number_parse(const char *string,
		       _tme_unumber_t max_positive,
		       _tme_unumber_t max_negative,
		       _tme_unumber_t underflow,
		       int *_failed)
{
  char c;
  int negative;
  unsigned int base;
  _tme_unumber_t value, max, max_pre_shift;
  tme_uint32_t units;
  unsigned long digit;
  int failed;
  char cbuf[2], *p1;

  /* assume simple conversion failure: */
  *_failed = TRUE;
  errno = 0;

  /* return simple conversion failure for a NULL string: */
  if (string == NULL) {
    return (0);
  }

  /* XXX parts of this might be ASCII-centric: */

  /* skip leading whitespace: */
  for (; (c = *string) != '\0' && isspace((unsigned char) c); string++);

  /* check for a leading '-' or '+' character: */
  if ((negative = (c == '-'))
      || c == '+') {
    c = *(++string);
  }

  /* check for a leading 0x or 0X, indicating hex, or a leading 0,
     indicating octal.  in the octal case, we don't skip the leading
     zero, because it may be the only digit to convert: */
  base = 10;
  if (c == '0') {
    base = 8;
    c = *(string + 1);
    if (c == 'x'
	|| c == 'X') {
      base = 16;
      string += 2;
    }
  }

  /* determine the maximum magnitude of the converted value, and the
     maximum magnitude past which we cannot shift it to add another
     digit in this base without overflowing: */
  max = (negative ? max_negative : max_positive);
  max_pre_shift = max / base;

  /* prepare the strtoul character buffer: */
  cbuf[1] = '\0';

  /* convert characters: */
  value = 0;  
  for (failed = TRUE;
       (c = *string) != '\0';
       failed = FALSE, string++) {
    
    /* stop if we can't convert this character into a digit: */
    cbuf[0] = c;
    digit = strtoul(cbuf, &p1, base);
    if (*p1 != '\0') {
      break;
    }

    /* return ERANGE if this digit causes an overflow: */
    if (value > max_pre_shift
	|| digit > (max - (value *= base))) {
      errno = ERANGE;
      return (negative ? underflow : max_positive);
    }
    value += digit;
  }

  /* get any units: */
  units = 1;
  if (!strcmp(string, "GB")) {
    units = 1024 * 1024 * 1024;
  }
  else if (!strcmp(string, "MB")) {
    units = 1024 * 1024;
  }
  else if (!strcmp(string, "KB")) {
    units = 1024;
  }
  else if (!strcmp(string, "G")) {
    units = 1000000000;
  }
  else if (!strcmp(string, "M")) {
    units = 1000000;
  }
  else if (!strcmp(string, "K")) {
    units = 1000;
  }
  else if (*string != '\0') {
    failed = TRUE;
  }

  /* return any simple conversion failure: */
  if (failed) {
    return (0);
  }

  /* return ERANGE if the units cause an overflow: */
  if (value > (max / units)) {
    errno = ERANGE;
    return (negative ? underflow : max_positive);
  }

  /* return success: */
  *_failed = FALSE;
  value *= units;
  return (negative ? 0 - value : value);
}

/* this parses an unsigned number: */
_tme_unumber_t
tme_misc_unumber_parse_any(const char *string, 
			   int *_failed)
{
  _tme_unumber_t max;
  max = 0;
  max -= 1;
  return (_tme_misc_number_parse(string,
				 max,
				 max,
				 max,
				 _failed));
}

/* this parses a signed number: */
_tme_number_t 
tme_misc_number_parse_any(const char *string,
			  int *_failed)
{
  _tme_unumber_t max_positive;
  _tme_unumber_t max_negative;
  max_positive = 1;
  max_positive = (max_positive << ((sizeof(max_positive) * 8) - 1)) - 1;
  max_negative = max_positive + 1;
  return (_tme_misc_number_parse(string,
				 max_positive,
				 max_negative,
				 max_negative,
				 _failed));
}

/* this parses an unsigned number that has a restricted range: */
_tme_unumber_t
tme_misc_unumber_parse(const char *string, 
		       _tme_unumber_t failure_value)
{
  int failed;
  _tme_unumber_t value;
  value = tme_misc_unumber_parse_any(string, &failed);
  return (failed ? failure_value : value);
}

/* this parses a signed number that has a restricted range: */
_tme_number_t
tme_misc_number_parse(const char *string, 
		      _tme_number_t failure_value)
{
  int failed;
  _tme_number_t value;
  value = tme_misc_number_parse_any(string, &failed);
  return (failed ? failure_value : value);
}

/* this returns a scaled cycle counter: */
union tme_value64
tme_misc_cycles_scaled(const tme_misc_cycles_scaling_t *scaling,
		       const union tme_value64 *_cycles_u)
{
  tme_misc_cycles_scaling_t two_to_the_thirtysecond;
  tme_misc_cycles_scaling_t cycles;
  tme_misc_cycles_scaling_t cycles_scaled;
  union tme_value64 cycles_u;

  /* make 2^32: */
  two_to_the_thirtysecond = 65536;
  two_to_the_thirtysecond *= 65536;

  /* get the cycles: */
#ifdef TME_HAVE_INT64_T
  cycles
    = (_cycles_u == NULL
       ? tme_misc_cycles().tme_value64_uint
       : _cycles_u->tme_value64_uint);
#else  /* !TME_HAVE_INT64_T */
  if (_cycles_u == NULL) {
    cycles_u = tme_misc_cycles();
    _cycles_u = &cycles_u;
  }
  cycles = _cycles_u->tme_value64_uint32_hi * two_to_the_thirtysecond;
  cycles += _cycles_u->tme_value64_uint32_lo;
#endif /* !TME_HAVE_INT64_T */

  /* return the scaled cycles: */
  cycles_scaled = cycles * *scaling;
#ifdef TME_HAVE_INT64_T
  cycles_u.tme_value64_uint = cycles_scaled;
#else  /* !TME_HAVE_INT64_T */
  cycles_u.tme_value64_uint32_lo = (cycles_scaled % two_to_the_thirtysecond);
  cycles_u.tme_value64_uint32_hi = (cycles_scaled / two_to_the_thirtysecond);
#endif /* !TME_HAVE_INT64_T */
  return (cycles_u);
}

/* this returns a scaling factor for the cycle counter: */
void
tme_misc_cycles_scaling(tme_misc_cycles_scaling_t *scaling,
			tme_uint32_t numerator,
			tme_uint32_t denominator)
{
  *scaling = numerator;
  *scaling /= denominator;
}

#ifndef TME_HAVE_MISC_CYCLES_PER_MS

/* this returns the cycle counter rate per millisecond: */
int tme_misc_cycles_per_ms_spin;
tme_uint32_t
tme_misc_cycles_per_ms(void)
{
  union tme_value64 cycles_start;
  struct timeval timeval_start;
  union tme_value64 cycles_finish;
  struct timeval timeval_finish;
  tme_uint32_t ms_elapsed;
  tme_misc_cycles_scaling_t cycles_elapsed;

  /* sample the cycle counter and the current time: */
  cycles_start = tme_misc_cycles();
  gettimeofday(&timeval_start, NULL);

  /* spin until at least a second has passed: */
  do {
    tme_misc_cycles_per_ms_spin++;
    cycles_finish = tme_misc_cycles();
    gettimeofday(&timeval_finish, NULL);
  } while ((timeval_finish.tv_sec == timeval_start.tv_sec)
	   || (timeval_finish.tv_sec == (timeval_start.tv_sec + 1)
	       && timeval_finish.tv_usec < timeval_start.tv_usec));

  /* return the approximate cycle counter rate per millisecond: */
  timeval_finish.tv_sec--;
  timeval_finish.tv_usec += 1000000;
  ms_elapsed = (timeval_finish.tv_sec - timeval_start.tv_sec) * 1000;
  ms_elapsed += (timeval_finish.tv_usec - timeval_start.tv_usec) / 1000;
  (void) tme_value64_sub(&cycles_finish, &cycles_start);
  cycles_elapsed = cycles_finish.tme_value64_uint32_hi;
  cycles_elapsed *= 65536;
  cycles_elapsed *= 65536;
  cycles_elapsed += cycles_finish.tme_value64_uint32_lo;
  return (cycles_elapsed / ms_elapsed);
}

#endif /* !TME_HAVE_MISC_CYCLES_PER_MS */

#ifndef TME_HAVE_MISC_CYCLES

/* this returns the cycle counter: */
union tme_value64
tme_misc_cycles(void)
{
#ifdef TME_HAVE_INT64_T
  struct timeval now;
  tme_uint64_t cycles;
  union tme_value64 value;

  gettimeofday(&now, NULL);
  cycles = now.tv_sec;
  cycles *= 1000000;
  cycles += now.tv_usec;
  value.tme_value64_uint = cycles;
  return (value);
#else  /* !TME_HAVE_INT64_T */
  struct timeval now;
  tme_misc_cycles_scaling_t two_to_the_thirtysecond;
  tme_misc_cycles_scaling_t cycles_sec;
  tme_uint32_t cycles_lo;
  tme_uint32_t usec;
  union tme_value64 value;

  /* get the current time: */
  gettimeofday(&now, NULL);

  /* make 2^32: */
  two_to_the_thirtysecond = 65536;
  two_to_the_thirtysecond *= 65536;

  /* get the seconds part of the cycles: */
  cycles_sec = now.tv_sec;
  cycles_sec *= 1000000;

  /* return the cycles: */
  cycles_lo = (cycles_sec % two_to_the_thirtysecond);
  usec = now.tv_usec;
  value.tme_value64_uint32_hi
    = (((tme_uint32_t) (cycles / two_to_the_thirtysecond))
       + (usec > ~cycles_lo));
  value.tme_value64_uint32_lo = cycles_lo + usec;
  return (value);
#endif /* !TME_HAVE_INT64_T */
}

#endif /* !defined(TME_HAVE_MISC_CYCLES) */

/* this spins until the cycle counter reaches a threshold: */
void
tme_misc_cycles_spin_until(const union tme_value64 *cycles_until)
{
  union tme_value64 cycles;

  do {
    cycles = tme_misc_cycles();
  } while (tme_value64_cmp(&cycles, <, cycles_until));
}

/* this adds two 64-bit values: */
#undef tme_value64_add
union tme_value64 *
tme_value64_add(union tme_value64 *a,
		const union tme_value64 *b)
{
  tme_uint32_t a_part;
  tme_uint32_t b_part;

  /* do the addition: */
  a_part = a->tme_value64_uint32_lo;
  b_part = b->tme_value64_uint32_lo;
  a->tme_value64_uint32_lo = a_part + b_part;
  a->tme_value64_uint32_hi
    = (a->tme_value64_uint32_hi
       + (b->tme_value64_uint32_hi
	  + (b_part > ~a_part)));
  return (a);
}

/* this subtracts two 64-bit values: */
#undef tme_value64_sub
union tme_value64 *
tme_value64_sub(union tme_value64 *a,
		const union tme_value64 *b)
{
  tme_uint32_t a_part;
  tme_uint32_t b_part;

  /* do the subtraction: */
  a_part = a->tme_value64_uint32_lo;
  b_part = b->tme_value64_uint32_lo;
  a->tme_value64_uint32_lo = a_part - b_part;
  a->tme_value64_uint32_hi
    = (a->tme_value64_uint32_hi
       - (b->tme_value64_uint32_hi
	  + (b_part > a_part)));
  return (a);
}
