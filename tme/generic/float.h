/* $Id: float.h,v 1.5 2009/08/29 21:19:43 fredette Exp $ */

/* tme/generic/float.h - public header file for floating-point emulation */

/*
 * Copyright (c) 2004 Matt Fredette
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

#ifndef _TME_GENERIC_FLOAT_H
#define _TME_GENERIC_FLOAT_H

#include <tme/common.h>
_TME_RCSID("$Id: float.h,v 1.5 2009/08/29 21:19:43 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <setjmp.h>
#include <math.h>
#ifdef _TME_HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* _TME_HAVE_IEEEFP_H */
#ifdef _TME_HAVE_FLOAT_H
#include <float.h>
#endif /* _TME_HAVE_FLOAT_H */
#ifdef _TME_HAVE_LIMITS_H
#include <limits.h>
#endif /* _TME_HAVE_LIMITS_H */

/* macros: */

/* floating-point formats.  even though these are individual bits, we
   can't use TME_BIT() because these are used in preprocessor ifs: */
#define TME_FLOAT_FORMAT_NULL				(0)
#ifndef TME_FLOAT_FORMAT_FLOAT
#define TME_FLOAT_FORMAT_FLOAT				(1)
#endif /* !TME_FLOAT_FORMAT_FLOAT */
#ifndef TME_FLOAT_FORMAT_DOUBLE
#define TME_FLOAT_FORMAT_DOUBLE				(2)
#endif /* !TME_FLOAT_FORMAT_FLOAT */
#ifdef _TME_HAVE_LONG_DOUBLE
#ifndef TME_FLOAT_FORMAT_LONG_DOUBLE
#define TME_FLOAT_FORMAT_LONG_DOUBLE			(4)
#endif /* !TME_FLOAT_FORMAT_LONG_DOUBLE */
#endif /* _TME_HAVE_LONG_DOUBLE */
#define TME_FLOAT_FORMAT_IEEE754_SINGLE			(8)
#define TME_FLOAT_FORMAT_IEEE754_DOUBLE			(16)
#define TME_FLOAT_FORMAT_IEEE754_EXTENDED80_I387	(32)
#define TME_FLOAT_FORMAT_IEEE754_EXTENDED80_M68881	(64)
#define TME_FLOAT_FORMAT_IEEE754_QUAD			(128)

/* if the long double type is available, this expands to the code
   fragment in x, else it expands to nothing: */
#ifdef _TME_HAVE_LONG_DOUBLE
#define TME_FLOAT_IF_LONG_DOUBLE(x) x
#else  /* !_TME_HAVE_LONG_DOUBLE */
#define TME_FLOAT_IF_LONG_DOUBLE(x) /* */
#endif /* !_TME_HAVE_LONG_DOUBLE */

/* a mask of all builtin floating-point formats: */
#define TME_FLOAT_FORMATS_BUILTIN			(TME_FLOAT_FORMAT_FLOAT				\
							 | TME_FLOAT_FORMAT_DOUBLE			\
							 TME_FLOAT_IF_LONG_DOUBLE(| TME_FLOAT_FORMAT_LONG_DOUBLE))

/* floating-point rounding modes: */
#ifdef _TME_HAVE_FPSETROUND
#define TME_FLOAT_ROUND_NULL			(0xdeadbeef)
#define TME_FLOAT_ROUND_NEAREST_EVEN		(FP_RN)
#define TME_FLOAT_ROUND_DOWN			(FP_RM)
#define TME_FLOAT_ROUND_UP			(FP_RP)
#define TME_FLOAT_ROUND_TO_ZERO			(FP_RZ)
#else  /* !_TME_HAVE_FPSETROUND */
#define TME_FLOAT_ROUND_NULL			(0)
#define TME_FLOAT_ROUND_NEAREST_EVEN		(1)
#define TME_FLOAT_ROUND_DOWN			(2)
#define TME_FLOAT_ROUND_UP			(3)
#define TME_FLOAT_ROUND_TO_ZERO			(4)
#endif /* !_TME_HAVE_FPSETROUND */

/* floating-point exceptions: */
#define TME_FLOAT_EXCEPTION_GENERIC		TME_BIT(0)
#define TME_FLOAT_EXCEPTION_INVALID		TME_BIT(1)
#define TME_FLOAT_EXCEPTION_DIVBYZERO		TME_BIT(2)
#define TME_FLOAT_EXCEPTION_OVERFLOW		TME_BIT(3)
#define TME_FLOAT_EXCEPTION_UNDERFLOW		TME_BIT(4)
#define TME_FLOAT_EXCEPTION_INEXACT		TME_BIT(5)
#define TME_FLOAT_EXCEPTION_OVERFLOW_INT	TME_BIT(6)
#define TME_FLOAT_EXCEPTION_DENORMAL		TME_BIT(7)

/* types: */

/* each of these types describes a floating point format supported on
   hosts.  each of these types may also be used in code that emulates
   the a floating point format, but each type's first purpose is to
   overlap exactly with one or more of the host's builtin C types that
   uses the format.

   for example, if the host has a builtin C type that uses the i387
   style of the IEEE 754 80-bit extended precision format, struct
   tme_float_ieee754_extended80 matches that style, not the m68k
   style.  if no builtin C type uses any style of the IEEE 754 80-bit
   extended precision format, struct tme_float_ieee754_extended80
   matches a random style.

   member values in these structures are always in host byte and word
   order.

   we also allow for these parts of the header file to be multiply
   included, so that these types can be incorporated into emulation
   code under different names: */

/* tme_uint32_t is used for the IEEE 754 single precision format. */

/* union tme_value64 is used for the IEEE 754 double precision format. */

#undef _TME_GENERIC_FLOAT_H
#endif /* !_TME_GENERIC_FLOAT_H */

#ifndef TME_FLOAT_FORMAT_IEEE754_EXTENDED80

/* making the IEEE 754 80-bit extended precision float structure
   overlap with a native type is complicated, since there are four
   possible representations: i387-style (where the 16 bits of sign and
   exponent are adjacent to the significand) and m68881-style (where
   there is a 16-bit gap between the significand and the 16 bits of
   sign and exponent), plus big- and little-endian variants of each: */

/* decide where the 16-bit sign-exponent word and any 16-bit padding
   words are in relation to the significand: */
#if ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_EXTENDED80_M68881) != 0)
#define TME_FLOAT_FORMAT_IEEE754_EXTENDED80	TME_FLOAT_FORMAT_IEEE754_EXTENDED80_M68881
#define _tme_float_ieee754_extended80_word_1 tme_float_ieee754_extended80_sexp
#define _tme_float_ieee754_extended80_word_0 tme_float_ieee754_extended80_pad
#else  /* ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_EXTENDED80_M68881) == 0) */
#define TME_FLOAT_FORMAT_IEEE754_EXTENDED80	TME_FLOAT_FORMAT_IEEE754_EXTENDED80_I387
#define _tme_float_ieee754_extended80_word_0 tme_float_ieee754_extended80_sexp
#endif /* ((TME_FLOAT_FORMATS_BUILTIN & TME_FLOAT_FORMAT_IEEE754_EXTENDED80_M68881) == 0) */

/* the IEEE 754 80-bit extended precision format: */
struct tme_float_ieee754_extended80 {

  /* if this is a big-endian host: */
#ifdef _TME_WORDS_BIGENDIAN

  /* up to four 16-bit words preceding the significand: */
#ifdef _tme_float_ieee754_extended80_word_3
  tme_uint16_t _tme_float_ieee754_extended80_word_3;
#endif /* _tme_float_ieee754_extended80_word_3 */
#ifdef _tme_float_ieee754_extended80_word_2
  tme_uint16_t _tme_float_ieee754_extended80_word_2;
#endif /* _tme_float_ieee754_extended80_word_2 */
#ifdef _tme_float_ieee754_extended80_word_1
  tme_uint16_t _tme_float_ieee754_extended80_word_1;
#endif /* _tme_float_ieee754_extended80_word_1 */
  tme_uint16_t _tme_float_ieee754_extended80_word_0;

#endif /* _TME_WORDS_BIGENDIAN */

  /* the significand: */
#ifndef tme_float_ieee754_extended80
  union tme_value64 tme_float_ieee754_extended80_significand;
#else  /* !tme_float_ieee754_extended80 */
  tme_uint64_t tme_float_ieee754_extended80_significand;
#endif /* !tme_float_ieee754_extended80 */

  /* if this is a little-endian host: */
#ifndef _TME_WORDS_BIGENDIAN

  /* up to four 16-bit words following the significand: */
  tme_uint16_t _tme_float_ieee754_extended80_word_0;
#ifdef _tme_float_ieee754_extended80_word_1
  tme_uint16_t _tme_float_ieee754_extended80_word_1;
#endif /* _tme_float_ieee754_extended80_word_1 */
#ifdef _tme_float_ieee754_extended80_word_2
  tme_uint16_t _tme_float_ieee754_extended80_word_2;
#endif /* _tme_float_ieee754_extended80_word_2 */
#ifdef _tme_float_ieee754_extended80_word_3
  tme_uint16_t _tme_float_ieee754_extended80_word_3;
#endif /* _tme_float_ieee754_extended80_word_3 */

#endif /* !_TME_WORDS_BIGENDIAN */
#undef _tme_float_ieee754_extended80_word_0
#undef _tme_float_ieee754_extended80_word_1
#undef _tme_float_ieee754_extended80_word_2
#undef _tme_float_ieee754_extended80_word_3
};

#endif /* TME_FLOAT_FORMAT_IEEE754_EXTENDED80 */

#ifndef _TME_GENERIC_FLOAT_H
#define _TME_GENERIC_FLOAT_H

/* the IEEE 754 quad precision format: */
struct tme_float_ieee754_quad {
#ifdef _TME_WORDS_BIGENDIAN
  union tme_value64 tme_float_ieee754_quad_hi;
  union tme_value64 tme_float_ieee754_quad_lo;
#else  /* !_TME_WORDS_BIGENDIAN */
  union tme_value64 tme_float_ieee754_quad_lo;
  union tme_value64 tme_float_ieee754_quad_hi;
#endif /* !_TME_WORDS_BIGENDIAN */
};

/* the generic float: */
struct tme_float {

  /* the format of this float: */
  unsigned int tme_float_format;

  /* the value of this float: */
  union {
    
    /* the builtin formats: */
    float _tme_float_union_float;
    double _tme_float_union_double;
#ifdef _TME_HAVE_LONG_DOUBLE
    long double _tme_float_union_long_double;
#endif /* _TME_HAVE_LONG_DOUBLE */

    /* the IEEE 754 formats: */
    tme_uint32_t _tme_float_union_ieee754_single;
    union tme_value64 _tme_float_union_ieee754_double;
    struct tme_float_ieee754_extended80 _tme_float_union_ieee754_extended80;
    struct tme_float_ieee754_quad _tme_float_union_ieee754_quad;
  } _tme_float_union;

  /* the public member names: */
#define tme_float_value_float _tme_float_union._tme_float_union_float
#define tme_float_value_double _tme_float_union._tme_float_union_double
#ifdef _TME_HAVE_LONG_DOUBLE
#define tme_float_value_long_double _tme_float_union._tme_float_union_long_double
#endif /* _TME_HAVE_LONG_DOUBLE */
#define tme_float_value_ieee754_single _tme_float_union._tme_float_union_ieee754_single
#define tme_float_value_ieee754_double _tme_float_union._tme_float_union_ieee754_double
#define tme_float_value_ieee754_extended80 _tme_float_union._tme_float_union_ieee754_extended80
#define tme_float_value_ieee754_quad _tme_float_union._tme_float_union_ieee754_quad
};

/* prototypes: */

/* this enters native floating-point operation: */
void tme_float_enter _TME_P((int, void (*)(int, void *), void *));

/* this returns the current native floating-point exceptions: */
int tme_float_exceptions _TME_P((void));

/* this leaves native floating-point operation: */
int tme_float_leave _TME_P((void));

/* this asserts that the float is in one of the expected formats: */
#ifndef NDEBUG
static _tme_inline int
tme_float_assert_formats(_tme_const struct tme_float *x, unsigned int formats)
{
  assert (x->tme_float_format & formats);
  return (TRUE);
}
#else  /* NDEBUG */
#define tme_float_assert_formats(x, formats) (TRUE)
#endif /* NDEBUG */

/* this evaluates to nonzero if the float is in a certain format, if
   the float is known to be one of the given formats: */
#define tme_float_is_format(x, formats, format)	\
  (((formats) == (format)) || (((formats) & (format)) && ((x)->tme_float_format & (format))))

/* this sets a float to a given builtin value: */
#define tme_float_value_builtin_set(x, format, y) \
  do { \
    if ((format) == TME_FLOAT_FORMAT_FLOAT) { \
      (x)->tme_float_value_float = (y); \
    } \
    TME_FLOAT_IF_LONG_DOUBLE(else if ((format) == TME_FLOAT_FORMAT_LONG_DOUBLE) { \
      (x)->tme_float_value_long_double = (y); \
    }) \
    else { \
      assert((format) == TME_FLOAT_FORMAT_DOUBLE); \
      (x)->tme_float_value_double = (y); \
    } \
    (x)->tme_float_format = (format); \
  } while (/* CONSTCOND */ 0)

/* these return the exponents of values in the IEEE 754 formats: */
#define tme_float_value_ieee754_exponent_single(x) \
  TME_FIELD_MASK_EXTRACTU((x)->tme_float_value_ieee754_single, 0x7f800000)
#define tme_float_value_ieee754_exponent_double(x) \
  TME_FIELD_MASK_EXTRACTU((x)->tme_float_value_ieee754_double.tme_value64_uint32_hi, 0x7ff00000)
#define tme_float_value_ieee754_exponent_extended80(x) \
  TME_FIELD_MASK_EXTRACTU((x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp, 0x7fff)
#define tme_float_value_ieee754_exponent_quad(x) \
  TME_FIELD_MASK_EXTRACTU((x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_hi, 0x7fff0000)

/* these return a bitwise-or of the fraction bits in values in the
   IEEE 754 formats (and, for the IEEE 754 80-bit extended precision
   format, a bitwise-or of the entire significand, including the
   explicit integer bit): */
#define tme_float_value_ieee754_fracor_single(x) \
  ((x)->tme_float_value_ieee754_single & 0x007fffff)
#define tme_float_value_ieee754_fracor_double(x) \
  (((x)->tme_float_value_ieee754_double.tme_value64_uint32_hi & 0x000fffff) | (x)->tme_float_value_ieee754_double.tme_value64_uint32_lo)
#define tme_float_value_ieee754_fracor_extended80(x) \
  (((x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi << 1) | (x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo)
#define tme_float_value_ieee754_sigor_extended80(x) \
  ((x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_hi | (x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint32_lo)
#define tme_float_value_ieee754_fracor_quad(x) \
  (((x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_hi & 0x0000ffff) \
   | (x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_lo \
   | (x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_hi \
   | (x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_lo.tme_value64_uint32_lo)

/* this evaluates to nonzero if the float is a NaN: */
#define tme_float_is_nan(x, formats) \
  (tme_float_assert_formats(x, formats) \
   && (tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_SINGLE) \
       ? (tme_float_value_ieee754_exponent_single(x) == 0xff \
	  && tme_float_value_ieee754_fracor_single(x) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_DOUBLE) \
       ? (tme_float_value_ieee754_exponent_double(x) == 0x7ff \
	  && tme_float_value_ieee754_fracor_double(x) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_EXTENDED80) \
       ? (tme_float_value_ieee754_exponent_extended80(x) == 0x7fff \
	  && tme_float_value_ieee754_fracor_extended80(x) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_QUAD) \
       ? (tme_float_value_ieee754_exponent_quad(x) == 0x7fff \
	  && tme_float_value_ieee754_fracor_quad(x) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_FLOAT) \
       ? isnanf((x)->tme_float_value_float) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_DOUBLE) \
       ? isnan((x)->tme_float_value_double) \
       : TME_FLOAT_IF_LONG_DOUBLE(isnan((x)->tme_float_value_long_double) ||) FALSE))

/* this evaluates to nonzero if the float is an infinity: */
#define tme_float_is_inf(x, formats) \
  (tme_float_assert_formats(x, formats) \
   && (tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_SINGLE) \
       ? (tme_float_value_ieee754_exponent_single(x) == 0xff \
	  && tme_float_value_ieee754_fracor_single(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_DOUBLE) \
       ? (tme_float_value_ieee754_exponent_double(x) == 0x7ff \
	  && tme_float_value_ieee754_fracor_double(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_EXTENDED80) \
       ? (tme_float_value_ieee754_exponent_extended80(x) == 0x7fff \
	  && tme_float_value_ieee754_fracor_extended80(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_QUAD) \
       ? (tme_float_value_ieee754_exponent_quad(x) == 0x7fff \
	  && tme_float_value_ieee754_fracor_quad(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_FLOAT) \
       ? isinff((x)->tme_float_value_float) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_DOUBLE) \
       ? isinf((x)->tme_float_value_double) \
       : TME_FLOAT_IF_LONG_DOUBLE(isinf((x)->tme_float_value_long_double) ||) FALSE))

/* this evaluates to nonzero if the float is a zero: */
#define tme_float_is_zero(x, formats) \
  (tme_float_assert_formats(x, formats) \
   && (tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_SINGLE) \
       ? (tme_float_value_ieee754_exponent_single(x) == 0 \
	  && tme_float_value_ieee754_fracor_single(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_DOUBLE) \
       ? (tme_float_value_ieee754_exponent_double(x) == 0 \
	  && tme_float_value_ieee754_fracor_double(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_EXTENDED80) \
       ? (tme_float_value_ieee754_exponent_extended80(x) == 0 \
	  && tme_float_value_ieee754_sigor_extended80(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_QUAD) \
       ? (tme_float_value_ieee754_exponent_quad(x) == 0 \
	  && tme_float_value_ieee754_fracor_quad(x) == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_FLOAT) \
       ? ((x)->tme_float_value_float == 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_DOUBLE) \
       ? ((x)->tme_float_value_double == 0) \
       : TME_FLOAT_IF_LONG_DOUBLE(((x)->tme_float_value_long_double == 0) ||) FALSE))

/* this evaluates to nonzero if the float is negative: */
#define tme_float_is_negative(x, formats) \
  (tme_float_assert_formats(x, formats) \
   && (tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_SINGLE) \
       ? (((x)->tme_float_value_ieee754_single & 0x80000000) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_DOUBLE) \
       ? (((x)->tme_float_value_ieee754_double.tme_value64_uint32_hi & 0x80000000) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_EXTENDED80) \
       ? (((x)->tme_float_value_ieee754_extended80.tme_float_ieee754_extended80_sexp & 0x8000) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_IEEE754_QUAD) \
       ? (((x)->tme_float_value_ieee754_quad.tme_float_ieee754_quad_hi.tme_value64_uint32_hi & 0x80000000) != 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_FLOAT) \
       ? ((x)->tme_float_value_float < 0) \
       : tme_float_is_format(x, formats, TME_FLOAT_FORMAT_DOUBLE) \
       ? ((x)->tme_float_value_double < 0) \
       : TME_FLOAT_IF_LONG_DOUBLE(((x)->tme_float_value_long_double < 0) ||) FALSE))

/* if possible, this returns a positive or negative infinity
   float, otherwise, this returns the float value
   closest to that infinity: */
float tme_float_infinity_float _TME_P((int));

/* if possible, this returns a negative zero float.
   otherwise, this returns the negative float value closest
   to zero: */
float tme_float_negative_zero_float _TME_P((void));

/* this returns the radix 2 mantissa and exponent of an in-range float.
   the mantissa is either zero, or in the range [1,2): */
float tme_float_radix2_mantissa_exponent_float _TME_P((float, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
float tme_float_radix2_scale_float _TME_P((float, tme_int32_t));

/* this returns the radix 10 mantissa and exponent of an in-range float.
   the mantissa is either zero, or in the range [1,10): */
float tme_float_radix10_mantissa_exponent_float _TME_P((float, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
float tme_float_radix10_scale_float _TME_P((float, tme_int32_t));

/* if possible, this returns a positive or negative infinity
   double, otherwise, this returns the double value
   closest to that infinity: */
double tme_float_infinity_double _TME_P((int));

/* if possible, this returns a negative zero double.
   otherwise, this returns the negative double value closest
   to zero: */
double tme_float_negative_zero_double _TME_P((void));

/* this returns the radix 2 mantissa and exponent of an in-range double.
   the mantissa is either zero, or in the range [1,2): */
double tme_float_radix2_mantissa_exponent_double _TME_P((double, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
double tme_float_radix2_scale_double _TME_P((double, tme_int32_t));

/* this returns the radix 10 mantissa and exponent of an in-range double.
   the mantissa is either zero, or in the range [1,10): */
double tme_float_radix10_mantissa_exponent_double _TME_P((double, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
double tme_float_radix10_scale_double _TME_P((double, tme_int32_t));

#ifdef _TME_HAVE_LONG_DOUBLE

/* if possible, this returns a positive or negative infinity
   long double, otherwise, this returns the long double value
   closest to that infinity: */
long double tme_float_infinity_long_double _TME_P((int));

/* if possible, this returns a negative zero long double.
   otherwise, this returns the negative long double value closest
   to zero: */
long double tme_float_negative_zero_long_double _TME_P((void));

/* this returns the radix 2 mantissa and exponent of an in-range long double.
   the mantissa is either zero, or in the range [1,2): */
long double tme_float_radix2_mantissa_exponent_long_double _TME_P((long double, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
long double tme_float_radix2_scale_long_double _TME_P((long double, tme_int32_t));

/* this returns the radix 10 mantissa and exponent of an in-range long double.
   the mantissa is either zero, or in the range [1,10): */
long double tme_float_radix10_mantissa_exponent_long_double _TME_P((long double, tme_int32_t *));

/* this scales a value by adding n to its exponent: */
long double tme_float_radix10_scale_long_double _TME_P((long double, tme_int32_t));

#endif /* _TME_HAVE_LONG_DOUBLE */

/* prototypes for missing standard functions: */
#ifndef _TME_HAVE_ISINFF
int isinff _TME_P((float));
#endif /* !_TME_HAVE_ISINFF */

#endif /* _TME_GENERIC_FLOAT_H */
