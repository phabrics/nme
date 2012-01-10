/* $Id: common.h,v 1.16 2010/02/18 01:23:20 fredette Exp $ */

/* tme/common.h - header file for common things: */

/*
 * Copyright (c) 2002, 2003 Matt Fredette
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

#ifndef _TME_COMMON_H
#define _TME_COMMON_H

/* includes: */
#include <assert.h>
#include <unistd.h>
#include <tmeconfig.h>
#ifdef _TME_IMPL
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#endif /* _TME_IMPL */
#include <sys/types.h>

/* netinet/in.h is needed to get the hton and ntoh functions: */
#include <netinet/in.h>

#ifdef _TME_HAVE_SYS_BSWAP_H
/* sys/bswap.h is needed for the bswap functions: */
#include <sys/bswap.h>
#endif /* _TME_HAVE_SYS_BSWAP_H */

/* macros: */
#undef FALSE
#undef TRUE
#define FALSE (0)
#define TRUE (!FALSE)

/* RCS IDs: */
#ifdef notyet
#define _TME_RCSID(x) static const char _tme_rcsid[] = x
_TME_RCSID("$Id: common.h,v 1.16 2010/02/18 01:23:20 fredette Exp $");
#else  /* !_TME_IMPL */
#define _TME_RCSID(x)
#endif /* !_TME_IMPL */

/* concatenation: */
#if ((defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)) && !defined(UNIXCPP)) || defined(ANSICPP)
#define __TME_CONCAT(a,b) a ## b
#define _TME_CONCAT(a,b) __TME_CONCAT(a,b)
#else
#define _TME_CONCAT(a,b) a/**/b
#endif
#define _TME_CONCAT5(a,b,c,d,e) _TME_CONCAT(a,_TME_CONCAT(b,_TME_CONCAT(c,_TME_CONCAT(d,e))))
#define _TME_CONCAT4(a,b,c,d) _TME_CONCAT(a,_TME_CONCAT(b,_TME_CONCAT(c,d)))
#define _TME_CONCAT3(a,b,c) _TME_CONCAT(a,_TME_CONCAT(b,c))

/* prototypes: */
#if defined(__STDC__) || defined(__cplusplus)
#define _TME_P(x) x
#else  /* !__STDC__ && !__cplusplus */
#define _TME_P(x) ()
#endif /* !__STDC__ && !__cplusplus */

/* const, inline, and volatile: */
#if defined(_TME_IMPL) || defined(__STDC__) || defined(__cplusplus)
#define _tme_const const
#define _tme_inline inline
#define _tme_volatile volatile
#else  /* !_TME_IMPL && !__STDC__ && !__cplusplus */
#define _tme_const 
#define _tme_inline
#define _tme_volatile
#endif /* !_TME_IMPL && !__STDC__ && !__cplusplus */

/* bits: */
#define _TME_BIT(t, x)		(((t) 1) << (x))
#define TME_BIT(x)		_TME_BIT(unsigned int, x)

/* alignment: */
#define TME_ALIGN(x, y)		(((x) + ((y) - 1)) & -(y))
#define TME_ALIGN_MAX		(8)

/* endianness: */
#define TME_ENDIAN_LITTLE	(0)
#define TME_ENDIAN_BIG		(1)
#ifdef _TME_WORDS_BIGENDIAN
#define TME_ENDIAN_NATIVE TME_ENDIAN_BIG
#else  /* !_TME_WORDS_BIGENDIAN */
#define TME_ENDIAN_NATIVE TME_ENDIAN_LITTLE
#endif /* !_TME_WORDS_BIGENDIAN */

/* cast auditing: */
#ifndef TME_NO_AUDIT_CASTS
#define _tme_audit_type(e, t)	(1 ? (e) : ((t) 0))
#else  /* TME_NO_AUDIT_CASTS */
#define _tme_audit_type(e, t)	(e)
#endif /* TME_AUDIT_CASTS */

/* branch prediction: */
#if defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 96) || (__GNUC__ >= 3))
#define __tme_predict_true(e)	(__builtin_expect(((e) != 0), 1))
#define __tme_predict_false(e)	(__builtin_expect(((e) != 0), 0))
#endif /* __GNUC__ >= 2.96 */
#ifndef __tme_predict_true
#define __tme_predict_true(e)	(e)
#endif  /* !__tme_predict_true */
#ifndef __tme_predict_false
#define __tme_predict_false(e)	(e)
#endif /* !__tme_predict_false */

/* memory allocation: */
#define tme_new(t, x)		((t *) tme_malloc(sizeof(t) * (x)))
#define tme_new0(t, x)		((t *) tme_malloc0(sizeof(t) * (x)))
#define tme_renew(t, m, x)	((t *) tme_realloc(m, sizeof(t) * (x)))
#define tme_dup(t, m, x)	((t *) tme_memdup(m, sizeof(t) * (x)))

/* minimum/maximum: */
#define TME_MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define TME_MAX(a, b)		(((a) > (b)) ? (a) : (b))

/* shifts: */
/* NB: count is evaluated multiple times and must be greater than
   zero: */
#define TME_SHIFT(type, value, shift, count)	\
  ((type)					\
   ((((type) (value))				\
     shift TME_MIN((count) - 1,			\
		   (sizeof(type) * 8) - 1))	\
    shift 1))

/* sign extension: */
#define TME_EXT_S8_S16(x)	((tme_int16_t) _tme_audit_type(x, tme_int8_t))
#define TME_EXT_S8_S32(x)	((tme_int32_t) _tme_audit_type(x, tme_int8_t))
#define TME_EXT_S16_S32(x)	((tme_int32_t) _tme_audit_type(x, tme_int16_t))
#define TME_EXT_S8_U16(x)	((tme_uint16_t) TME_EXT_S8_S16(x))
#define TME_EXT_S8_U32(x)	((tme_uint32_t) TME_EXT_S8_S32(x))
#define TME_EXT_S16_U32(x)	((tme_uint32_t) TME_EXT_S16_S32(x))

/* bitfields: */
#define _TME_FIELD_EXTRACTU(t, v, s, l) ((((t) (v)) >> (s)) & (_TME_BIT(t, l) - 1))
#define TME_FIELD_EXTRACTU(v, s, l) _TME_FIELD_EXTRACTU(unsigned int, v, s, l)
#define _TME_FIELD_DEPOSIT(t, v, s, l, x) ((v) = (((v) & ~((_TME_BIT(t, l) - 1) << (s))) | ((x) << (s))))
#define TME_FIELD_DEPOSIT8(v, s, l, x) _TME_FIELD_DEPOSIT(tme_uint8_t, v, s, l, x)
#define TME_FIELD_DEPOSIT16(v, s, l, x) _TME_FIELD_DEPOSIT(tme_uint16_t, v, s, l, x)
#define TME_FIELD_DEPOSIT32(v, s, l, x) _TME_FIELD_DEPOSIT(tme_uint32_t, v, s, l, x)
#define _TME_FIELD_MASK_FACTOR(m) (((m) | ((m) << 1)) ^ ((m) << 1))
#define TME_FIELD_MASK_EXTRACTU(v, m) (((v) & (m)) / _TME_FIELD_MASK_FACTOR(m))
#define TME_FIELD_MASK_DEPOSITU(v, m, x) ((v) = (((v) & ~(m)) | (((x) * _TME_FIELD_MASK_FACTOR(m)) & (m))))
#define _TME_FIELD_MASK_MSBIT(m) (((m) | ((m) >> 1)) ^ ((m) >> 1))
#define TME_FIELD_MASK_EXTRACTS(v, m)  ((TME_FIELD_MASK_EXTRACTU(v, m) ^ _TME_FIELD_MASK_MSBIT(m)) + (0 - _TME_FIELD_MASK_MSBIT(m)))

/* byteswapping: */
#ifndef _TME_WORDS_BIGENDIAN
#define tme_bswap_u16(x) ((tme_uint16_t) htons((tme_uint16_t) (x)))
#define tme_bswap_u32(x) ((tme_uint32_t) htonl((tme_uint32_t) (x)))
#else  /* _TME_WORDS_BIGENDIAN */
#ifdef _TME_HAVE_BSWAP16
#define tme_bswap_u16(x) ((tme_uint16_t) bswap16((tme_uint16_t) (x)))
#else  /* !_TME_HAVE_BSWAP16 */
static _tme_inline tme_uint16_t
tme_bswap_u16(tme_uint16_t x)
{
  return ((((x) & 0xff00) >> 8)
	  | (((x) & 0x00ff) << 8));
}
#endif /* !_TME_HAVE_BSWAP16 */
#ifdef _TME_HAVE_BSWAP32
#define tme_bswap_u32(x) ((tme_uint32_t) bswap32((tme_uint32_t) (x)))
#else  /* !_TME_HAVE_BSWAP32 */
static _tme_inline tme_uint32_t
tme_bswap_u32(tme_uint32_t x)
{
  return ((((x) & 0xff000000) >> 24)
	  | (((x) & 0x00ff0000) >> 8)
	  | (((x) & 0x0000ff00) << 8)
	  | (((x) & 0x000000ff) << 24));
}
#endif /* !_TME_HAVE_BSWAP32 */
#endif /* _TME_WORDS_BIGENDIAN */

/* endian conversion: */
#ifndef _TME_WORDS_BIGENDIAN
#define tme_htobe_u16(x) tme_bswap_u16(x)
#define tme_htobe_u32(x) tme_bswap_u32(x)
#define tme_htobe_u64(x) tme_bswap_u64(x)
#define tme_htole_u16(x) (x)
#define tme_htole_u32(x) (x)
#define tme_htole_u64(x) (x)
#else  /* _TME_WORDS_BIGENDIAN */
#define tme_htobe_u16(x) (x)
#define tme_htobe_u32(x) (x)
#define tme_htobe_u64(x) (x)
#define tme_htole_u16(x) tme_bswap_u16(x)
#define tme_htole_u32(x) tme_bswap_u32(x)
#define tme_htole_u64(x) tme_bswap_u64(x)
#endif /* _TME_WORDS_BIGENDIAN */
#define tme_betoh_u16(x) tme_htobe_u16(x)
#define tme_betoh_u32(x) tme_htobe_u32(x)
#define tme_betoh_u64(x) tme_htobe_u64(x)
#define tme_letoh_u16(x) tme_htole_u16(x)
#define tme_letoh_u32(x) tme_htole_u32(x)
#define tme_letoh_u64(x) tme_htole_u64(x)

/* i18n: */
#define _(x) x

/* 64-bit values: */
#ifndef TME_HAVE_INT64_T

/* gcc has a `long long' type that is defined to be twice as long as
   an int: */
/* XXX when exactly did this feature appear? */
#if defined(__GNUC__) && (__GNUC__ >= 2) && (_TME_SIZEOF_INT == 4)
#define TME_HAVE_INT64_T
#define _TME_ALIGNOF_INT64_T _TME_ALIGNOF_INT32_T
#define _TME_SHIFTMAX_INT64_T (63)
#define _TME_PRI64 "ll"
typedef signed long long int tme_int64_t;
typedef unsigned long long int tme_uint64_t;
#endif /* __GNUC__ && __GNUC__ >= 2 */

#endif /* TME_HAVE_INT64_T */
union tme_value64 {
#ifdef TME_HAVE_INT64_T
  tme_int64_t tme_value64_int;
  tme_uint64_t tme_value64_uint;
#endif /* TME_HAVE_INT64_T */
  tme_int32_t tme_value64_int32s[2];
  tme_uint32_t tme_value64_uint32s[2];
#ifndef _TME_WORDS_BIGENDIAN
#define tme_value64_int32_lo tme_value64_int32s[0]
#define tme_value64_int32_hi tme_value64_int32s[1]
#define tme_value64_uint32_lo tme_value64_uint32s[0]
#define tme_value64_uint32_hi tme_value64_uint32s[1]
#else  /* _TME_WORDS_BIGENDIAN */
#define tme_value64_int32_lo tme_value64_int32s[1]
#define tme_value64_int32_hi tme_value64_int32s[0]
#define tme_value64_uint32_lo tme_value64_uint32s[1]
#define tme_value64_uint32_hi tme_value64_uint32s[0]
#endif /* _TME_WORDS_BIGENDIAN */
};

/* 64-bit math: */
union tme_value64 *tme_value64_add _TME_P((union tme_value64 *, _tme_const union tme_value64 *));
union tme_value64 *tme_value64_sub _TME_P((union tme_value64 *, _tme_const union tme_value64 *));
union tme_value64 *tme_value64_mul _TME_P((union tme_value64 *, _tme_const union tme_value64 *));
union tme_value64 *tme_value64_div _TME_P((union tme_value64 *, _tme_const union tme_value64 *));
union tme_value64 *_tme_value64_set _TME_P((union tme_value64 *, _tme_const tme_uint8_t *, int));
#ifdef TME_HAVE_INT64_T
#define tme_value64_add(a, b) (((a)->tme_value64_uint += (b)->tme_value64_uint), (a))
#define tme_value64_sub(a, b) (((a)->tme_value64_uint -= (b)->tme_value64_uint), (a))
#define tme_value64_mul(a, b) (((a)->tme_value64_uint *= (b)->tme_value64_uint), (a))
#define tme_value64_div(a, b) (((a)->tme_value64_uint /= (b)->tme_value64_uint), (a))
#define tme_value64_imul(a, b) (((a)->tme_value64_int *= (b)->tme_value64_int), (a))
#define tme_value64_idiv(a, b) (((a)->tme_value64_int /= (b)->tme_value64_int), (a))
#define tme_value64_set(a, b) (((b) >= 0) ? ((a)->tme_value64_uint = (b), (a)) : ((a)->tme_value64_int = (b), (a)))
#define tme_value64_cmp(a, cmp, b) ((a)->tme_value64_uint cmp (b)->tme_value64_uint)
#else  /* !TME_HAVE_INT64_T */
#define tme_value64_set(a, b) _tme_value64_set(a, (_tme_const tme_uint8_t *) &(b), ((b) >= 0 ? sizeof(b) : -sizeof(b)))
#define _tme_value64_cmp32(half, a, cmp, b)			\
  ((a)->tme_value64_uint._TME_CONCAT(tme_value64_uint32_,half)	\
   cmp (b)->tme_value64_uint._TME_CONCAT(tme_value64_uint32_,half))
#define tme_value64_cmp(a, cmp, b)				\
  (((2 cmp 1)							\
    ? _tme_value32_cmp32(hi, a, >, b)				\
    : (1 cmp 2)							\
    ? _tme_value32_cmp32(hi, a, <, b)				\
    : FALSE)							\
   || (_tme_value32_cmp32(hi, a, ==, b)				\
       && _tme_value32_cmp32(lo, a, cmp, b)))
#endif /* !TME_HAVE_INT64_T */

/* 64-bit byte swapping: */
#ifdef TME_HAVE_INT64_T
#ifdef _TME_HAVE_BSWAP64
#define tme_bswap_u64(x) ((tme_uint64_t) bswap64((tme_uint64_t) (x)))
#else  /* !_TME_HAVE_BSWAP64 */
static _tme_inline tme_uint64_t
tme_bswap_u64(tme_uint64_t x)
{
  return ((((tme_uint64_t) tme_bswap_u32((tme_uint32_t) x)) << 32)
	  | tme_bswap_u32((tme_uint32_t) (x >> 32)));
}
#endif /* !_TME_HAVE_BSWAP64 */
#endif /* TME_HAVE_INT64_T */

/* versions: */
#define TME_X_VERSION(current, age)	(((current) << 10) | (age))
#define TME_X_VERSION_CURRENT(version)	((version) >> 10)
#define TME_X_VERSION_AGE(version)	((version) & 0x3ff)
#define TME_X_VERSION_OK(vimpl, vneed)						\
  TME_RANGES_OVERLAP(TME_X_VERSION_CURRENT(vneed) - TME_X_VERSION_AGE(vneed),	\
		     TME_X_VERSION_CURRENT(vneed),				\
		     TME_X_VERSION_CURRENT(vimpl) - TME_X_VERSION_AGE(vimpl),	\
		     TME_X_VERSION_CURRENT(vimpl))

/* printf formats: */
#define _TME_PRI8 ""
#define _TME_PRI16 ""
#define TME_PRIx8 _TME_PRI8 "x"
#define TME_PRIx16 _TME_PRI16 "x"
#define TME_PRIx32 _TME_PRI32 "x"
#define TME_PRIx64 _TME_PRI64 "x"

/* miscellaneous: */
#define TME_ARRAY_ELS(x)	(sizeof(x) / sizeof(x[0]))
#define TME_EMULATOR_OFF_UNDEF	((void *) (-1))
#define TME_RANGES_OVERLAP(low0, high0, low1, high1)	\
  (((high0) >= (low1)) && ((high1) >= (low0)))
#define TME_ARG_IS(s, x)	((s) != NULL && !strcmp(s, x))
#define TME_OK			(0)

/* prototypes: */
void *tme_malloc _TME_P((unsigned int));
void *tme_malloc0 _TME_P((unsigned int));
void *tme_realloc _TME_P((void *, unsigned int));
void *tme_memdup _TME_P((_tme_const void *, unsigned int));
void tme_free _TME_P((void *));
void tme_free_string_array _TME_P((char **, int));
char *tme_strdup _TME_P((_tme_const char *));

#ifdef _TME_IMPL
/* string and memory prototypes: */
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else  /* !STDC_HEADERS */
void *memcpy _TME_P((void *, _tme_const void *, size_t));
void *memset _TME_P((void *, int, size_t));
void *memmove _TME_P((void *, _tme_const void *, size_t));
#endif /* !STDC_HEADERS */
#endif /* _TME_IMPL */

#endif /* !_TME_COMMON_H */
