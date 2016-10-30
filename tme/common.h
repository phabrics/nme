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
#ifdef _TME_IMPL
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#endif /* _TME_IMPL */
#include <assert.h>
#include <unistd.h>
#include <tmeconfig.h>
#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef _TME_HAVE_BYTESWAP_H
/* byteswap.h is needed for the bswap functions: */
#include <byteswap.h>
#endif /* _TME_HAVE_BYTESWAP_H */

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
#ifdef _TME_HAVE___BUILTIN_BSWAP16
#define tme_bswap_u16(x) ((tme_uint16_t) __builtin_bswap16((tme_uint16_t) (x)))
#else  /* !_TME_HAVE___BUILTIN_BSWAP16 */
#ifdef _TME_HAVE_BSWAP16
#define tme_bswap_u16(x) ((tme_uint16_t) bswap16((tme_uint16_t) (x)))
#else  /* !_TME_HAVE_BSWAP16 */
#ifdef _TME_HAVE_BSWAP_16
#define tme_bswap_u16(x) ((tme_uint16_t) bswap_16((tme_uint16_t) (x)))
#else  /* !_TME_HAVE_BSWAP_16 */
#ifdef _TME_HAVE_SWAP16
#define tme_bswap_u16(x) ((tme_uint16_t) swap16((tme_uint16_t) (x)))
#else  /* !_TME_HAVE_SWAP16 */
static _tme_inline tme_uint16_t
tme_bswap_u16(tme_uint16_t x)
{
  return ((((x) & 0xff00) >> 8)
	  | (((x) & 0x00ff) << 8));
}
#endif /* !_TME_HAVE_SWAP16 */
#endif /* !_TME_HAVE_BSWAP_16 */
#endif /* !_TME_HAVE_BSWAP16 */
#endif /* !_TME_HAVE___BUILTIN_BSWAP16 */

#ifdef _TME_HAVE___BUILTIN_BSWAP32
#define tme_bswap_u32(x) ((tme_uint32_t) __builtin_bswap32((tme_uint32_t) (x)))
#else  /* !_TME_HAVE___BUILTIN_BSWAP32 */
#ifdef _TME_HAVE_BSWAP32
#define tme_bswap_u32(x) ((tme_uint32_t) bswap32((tme_uint32_t) (x)))
#else  /* !_TME_HAVE_BSWAP32 */
#ifdef _TME_HAVE_BSWAP_32
#define tme_bswap_u32(x) ((tme_uint32_t) bswap_32((tme_uint32_t) (x)))
#else  /* !_TME_HAVE_BSWAP_32 */
#ifdef _TME_HAVE_SWAP32
#define tme_bswap_u32(x) ((tme_uint32_t) swap32((tme_uint32_t) (x)))
#else  /* !_TME_HAVE_SWAP32 */
static _tme_inline tme_uint32_t
tme_bswap_u32(tme_uint32_t x)
{
  return ((((x) & 0xff000000) >> 24)
	  | (((x) & 0x00ff0000) >> 8)
	  | (((x) & 0x0000ff00) << 8)
	  | (((x) & 0x000000ff) << 24));
}
#endif /* !_TME_HAVE_SWAP32 */
#endif /* !_TME_HAVE_BSWAP_32 */
#endif /* !_TME_HAVE_BSWAP32 */
#endif /* !_TME_HAVE___BUILTIN_BSWAP32 */

/* endian conversion: */
#ifndef _TME_WORDS_BIGENDIAN
#define tme_htobe_u16(x) tme_bswap_u16(x)
#define tme_htobe_u32(x) tme_bswap_u32(x)
#define tme_htobe_u64(x) tme_bswap_u64(x)
#define tme_htobe_u128(x) tme_bswap_u128(x)
#define tme_htole_u16(x) (x)
#define tme_htole_u32(x) (x)
#define tme_htole_u64(x) (x)
#define tme_htole_u128(x) (x)
#else  /* _TME_WORDS_BIGENDIAN */
#define tme_htobe_u16(x) (x)
#define tme_htobe_u32(x) (x)
#define tme_htobe_u64(x) (x)
#define tme_htobe_u128(x) (x)
#define tme_htole_u16(x) tme_bswap_u16(x)
#define tme_htole_u32(x) tme_bswap_u32(x)
#define tme_htole_u64(x) tme_bswap_u64(x)
#define tme_htole_u128(x) tme_bswap_u128(x)
#endif /* _TME_WORDS_BIGENDIAN */
#define tme_betoh_u16(x) tme_htobe_u16(x)
#define tme_betoh_u32(x) tme_htobe_u32(x)
#define tme_betoh_u64(x) tme_htobe_u64(x)
#define tme_betoh_u128(x) tme_htobe_u128(x)
#define tme_letoh_u16(x) tme_htole_u16(x)
#define tme_letoh_u32(x) tme_htole_u32(x)
#define tme_letoh_u64(x) tme_htole_u64(x)
#define tme_letoh_u128(x) tme_htole_u128(x)

/* i18n: */
#define _(x) x

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
#ifdef _TME_HAVE___BUILTIN_BSWAP64
#define tme_bswap_u64(x) ((tme_uint64_t) __builtin_bswap64((tme_uint64_t) (x)))
#else  /* !_TME_HAVE___BUILTIN_BSWAP64 */
#ifdef _TME_HAVE_BSWAP64
#define tme_bswap_u64(x) ((tme_uint64_t) bswap64((tme_uint64_t) (x)))
#else  /* !_TME_HAVE_BSWAP64 */
#ifdef _TME_HAVE_BSWAP_64
#define tme_bswap_u64(x) ((tme_uint64_t) bswap_64((tme_uint64_t) (x)))
#else  /* !_TME_HAVE_BSWAP64 */
#ifdef _TME_HAVE_SWAP64
#define tme_bswap_u64(x) ((tme_uint64_t) swap64((tme_uint64_t) (x)))
#else  /* !_TME_HAVE_SWAP64 */
static _tme_inline tme_uint64_t
tme_bswap_u64(tme_uint64_t x)
{
  return ((((tme_uint64_t) tme_bswap_u32((tme_uint32_t) x)) << 32)
	  | tme_bswap_u32((tme_uint32_t) (x >> 32)));
}
#endif /* !_TME_HAVE_SWAP64 */
#endif /* !_TME_HAVE_BSWAP_64 */
#endif /* !_TME_HAVE_BSWAP64 */
#endif /* !_TME_HAVE___BUILTIN_BSWAP64 */
#endif /* TME_HAVE_INT64_T */

/* 128-bit byte swapping: */
#ifdef TME_HAVE_INT128_T
static _tme_inline tme_uint128_t
tme_bswap_u128(tme_uint128_t x)
{
  return ((((tme_uint128_t) tme_bswap_u64((tme_uint64_t) x)) << 64)
	  | tme_bswap_u64((tme_uint64_t) (x >> 64)));
}
#endif /* TME_HAVE_INT128_T */

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
#ifdef HAVE_INTTYPES_H
#define TME_PRIx8 PRIx8 
#define TME_PRIx16 PRIx16 
#define TME_PRIx32 PRIx32 
#define TME_PRIx64 PRIx64 
#define TME_PRIx128 PRIx128 
#else
#define _TME_PRI8 ""
#define _TME_PRI16 ""
#define TME_PRIx8 _TME_PRI8 "x"
#define TME_PRIx16 _TME_PRI16 "x"
#define TME_PRIx32 _TME_PRI32 "x"
#define TME_PRIx64 _TME_PRI64 "x"
#define TME_PRIx128 _TME_PRI128 "x"
#endif
  
/* miscellaneous: */
#define TME_ARRAY_ELS(x)	(sizeof(x) / sizeof(x[0]))
#define TME_EMULATOR_OFF_UNDEF	((void *) (-1))
#define TME_RANGES_OVERLAP(low0, high0, low1, high1)	\
  (((high0) >= (low1)) && ((high1) >= (low0)))
#define TME_ARG_IS(s, x)	((s) != NULL && !strcmp(s, x))
#define TME_OK			(0)

/* time: */
#define _TME_TIME_EQ(x,y,sec,frac) ((x).sec == (y).sec	\
			  && (x).frac == (y).frac)
#define _TME_TIME_EQV(a,x,y,sec,frac) ((a).sec == (x)	\
			     && (a).frac == (y))
#define _TME_TIME_GT(x,y,sec,frac) ((x).sec > (y).sec		\
			  || ((x).sec == (y).sec		\
			      && (x).frac > (y).frac))
#define _TME_TIME_GET_FRAC(a,frac) (a).frac
#define _TME_TIME_SET_FRAC(a,x,frac) ((a).frac = (x))
#define _TME_TIME_INC_FRAC(a,x,frac) ((a).frac += (x))
#define _TME_TIME_FRAC_LT(x,y,frac) ((x).frac < (y).frac)
#define _TME_TIME_SETV(a,s,u,sec,frac) (a).sec = (s); (a).frac = (u);
#define _TME_TIME_ADD(a,x,y,sec,frac) (a).sec = (x).sec + (y).sec; (a).frac = (x).frac + (y).frac;
#define _TME_TIME_ADDV(a,s,u,sec,frac) (a).sec += (s); (a).frac += (u);
#define _TME_TIME_INC(a,x,sec,frac) (a).sec += (x).sec; (a).frac += (x).frac;
#define _TME_TIME_SUB(a,x,y,sec,frac) (a).sec = (x).sec - (y).sec; (a).frac = (x).frac - (y).frac;
#define _TME_TIME_DEC(a,x,sec,frac) (a).sec -= (x).sec; (a).frac -= (x).frac;

#if defined(WIN32) && !defined(USE_GETTIMEOFDAY) || defined(USE_GLIB_TIME) && defined(_TME_HAVE_GLIB)
#ifdef USE_GLIB_TIME
#define tme_get_time(x) (*(x) = g_get_real_time(), TME_OK)
#else
static _tme_inline int tme_get_time _TME_P((tme_time_t *time)) {
  FILETIME filetime;
  ULARGE_INTEGER _time;
  GetSystemTimeAsFileTime(&filetime);
  _time.u.HighPart = filetime.dwHighDateTime;
  _time.u.LowPart = filetime.dwLowDateTime;
#ifdef TME_HAVE_INT64_T
  *time = _time.QuadPart;
#else
  *time = (_time.u.LowPart) | (_time.u.HighPart << 32);
#endif
  return TME_OK;
}
#endif
#define TME_TIME_T(x) typeof(x)
#define TME_TIME_GET_SEC(a) ((a) / TME_FRAC_PER_SEC)
#define TME_TIME_SET_SEC(a,x) TME_TIME_SETV(a,x,TME_TIME_GET_FRAC(a))
#define TME_TIME_EQ(x,y) (x == y)
#define TME_TIME_EQV(a,s,u) ((a) == (u) + (s) * TME_FRAC_PER_SEC)
#define TME_TIME_GT(x,y) (x > y)
#define TME_TIME_SETV(a,s,u) ((a) = (u) + (s) * TME_FRAC_PER_SEC)
#define TME_TIME_ADDV(a,s,u) TME_TIME_SETV(a, TME_TIME_GET_SEC(a) + s, TME_TIME_GET_FRAC(a) + u)
#define TME_TIME_INC_SEC(a,x) TME_TIME_ADDV(a,x,0)
#define TME_TIME_GET_FRAC(a) ((a) % TME_FRAC_PER_SEC)
#define TME_TIME_SET_FRAC(a,x) TME_TIME_SETV(a,TME_TIME_GET_SEC(a), x)
#define TME_TIME_INC_FRAC(a,x) TME_TIME_ADDV(a,0,x)
#define TME_TIME_FRAC_LT(x,y) (TME_TIME_GET_FRAC(x) < TME_TIME_GET_FRAC(y))
#define TME_TIME_ADD(a,x,y) TME_TIME_SETV(a, TME_TIME_GET_SEC(x) + TME_TIME_GET_SEC(y), TME_TIME_GET_FRAC(x) + TME_TIME_GET_FRAC(y))
#define TME_TIME_INC(a,x) TME_TIME_ADDV(a,TME_TIME_GET_SEC(x),TME_TIME_GET_FRAC(x))
#define TME_TIME_SUB(a,x,y) TME_TIME_SETV(a, TME_TIME_GET_SEC(x) - TME_TIME_GET_SEC(y), TME_TIME_GET_FRAC(x) - TME_TIME_GET_FRAC(y))
#define TME_TIME_SUBV(a,s,u) TME_TIME_SETV(a, TME_TIME_GET_SEC(a) - s, TME_TIME_GET_FRAC(a) - u)
#define TME_TIME_DEC(a,x) TME_TIME_SUBV(a,TME_TIME_GET_SEC(x),TME_TIME_GET_FRAC(x))
#elif defined(USE_GETTIMEOFDAY) || !defined(_TME_HAVE_CLOCK_GETTIME)
#define tme_get_time(x) gettimeofday(x, NULL)
#define TME_TIME_T(x) typeof((x).tv_sec)
#define TME_TIME_GET_SEC(a) _TME_TIME_GET_FRAC(a,tv_sec)
#define TME_TIME_SET_SEC(a,x) _TME_TIME_SET_FRAC(a,x,tv_sec)
#define TME_TIME_INC_SEC(a,x) _TME_TIME_INC_FRAC(a,x,tv_sec)
#define TME_TIME_EQ(x,y) _TME_TIME_EQ(x,y,tv_sec,tv_usec)
#define TME_TIME_EQV(a,x,y) _TME_TIME_EQV(a,x,y,tv_sec,tv_usec)
#define TME_TIME_GT(x,y) _TME_TIME_GT(x,y,tv_sec,tv_usec)
#define TME_TIME_GET_FRAC(a) _TME_TIME_GET_FRAC(a,tv_usec)
#define TME_TIME_SET_FRAC(a,x) _TME_TIME_SET_FRAC(a,x,tv_usec)
#define TME_TIME_INC_FRAC(a,x) _TME_TIME_INC_FRAC(a,x,tv_usec)
#define TME_TIME_FRAC_LT(x,y) _TME_TIME_FRAC_LT(x,y,tv_usec)
#define TME_TIME_SETV(a,s,u) _TME_TIME_SETV(a,s,u,tv_sec,tv_usec)
#define TME_TIME_ADD(a,x,y) _TME_TIME_ADD(a,x,y,tv_sec,tv_usec)
#define TME_TIME_ADDV(a,s,u) _TME_TIME_ADDV(a,s,u,tv_sec,tv_usec)
#define TME_TIME_INC(a,x) _TME_TIME_INC(a,x,tv_sec,tv_usec)
#define TME_TIME_SUB(a,x,y) _TME_TIME_SUB(a,x,y,tv_sec,tv_usec)
#define TME_TIME_DEC(a,x) _TME_TIME_DEC(a,x,tv_sec,tv_usec)
#else
#define tme_get_time(x) clock_gettime(CLOCK_REALTIME, x)
#define TME_TIME_T(x) typeof((x).tv_sec)
#define TME_TIME_GET_SEC(a) _TME_TIME_GET_FRAC(a,tv_sec)
#define TME_TIME_SET_SEC(a,x) _TME_TIME_SET_FRAC(a,x,tv_sec)
#define TME_TIME_INC_SEC(a,x) _TME_TIME_INC_FRAC(a,x,tv_sec)
#define TME_TIME_EQ(x,y) _TME_TIME_EQ(x,y,tv_sec,tv_nsec)
#define TME_TIME_EQV(a,x,y) _TME_TIME_EQV(a,x,y,tv_sec,tv_nsec)
#define TME_TIME_GT(x,y) _TME_TIME_GT(x,y,tv_sec,tv_nsec)
#define TME_TIME_GET_FRAC(a) (_TME_TIME_GET_FRAC(a,tv_nsec) / 1000)
#define TME_TIME_SET_FRAC(a,x) _TME_TIME_SET_FRAC(a,(x) * 1000,tv_nsec)
#define TME_TIME_INC_FRAC(a,x) _TME_TIME_INC_FRAC(a,(x) * 1000,tv_nsec)
#define TME_TIME_FRAC_LT(x,y) _TME_TIME_FRAC_LT(x,y,tv_nsec)
#define TME_TIME_SETV(a,s,u) _TME_TIME_SETV(a,s,(u) * 1000,tv_sec,tv_nsec)
#define TME_TIME_ADD(a,x,y) _TME_TIME_ADD(a,x,y,tv_sec,tv_nsec)
#define TME_TIME_ADDV(a,s,u) _TME_TIME_ADDV(a,s,(u) * 1000,tv_sec,tv_nsec)
#define TME_TIME_INC(a,x) _TME_TIME_INC(a,x,tv_sec,tv_nsec)
#define TME_TIME_SUB(a,x,y) _TME_TIME_SUB(a,x,y,tv_sec,tv_nsec)
#define TME_TIME_DEC(a,x) _TME_TIME_DEC(a,x,tv_sec,tv_nsec)
#endif
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
