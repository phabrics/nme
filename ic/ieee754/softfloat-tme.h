/* $Id: softfloat-tme.h,v 1.1 2005/02/17 12:17:58 fredette Exp $ */

/* ic/ieee754/softfloat-tme.h - this glues tme to SoftFloat: */

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

/*============================================================================

This C header file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

This C source fragment is part of the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/

#ifndef _IC_IEEE754_SOFTFLOAT_TME_H
#define _IC_IEEE754_SOFTFLOAT_TME_H

#include <tme/common.h>
_TME_RCSID("$Id: softfloat-tme.h,v 1.1 2005/02/17 12:17:58 fredette Exp $");

/* includes: */
#include <tme/ic/ieee754.h>

#ifdef _TME_IEEE754_SOFTFLOAT_GLUE

/* create an alternate definition of struct _tme_ieee754_extended80 to
   use as the SoftFloat 80-bit extended precision type: */
#undef TME_FLOAT_FORMAT_IEEE754_EXTENDED80
#define tme_float_ieee754_extended80 _floatx80
#define tme_float_ieee754_extended80_significand low
#define tme_float_ieee754_extended80_sexp high

/* actually create the alternate definitions: */
#include <tme/generic/float.h>

#endif /* _TME_IEEE754_SOFTFLOAT_GLUE */

/* if a 64-bit integral type is available: */
#ifdef TME_HAVE_INT64_T

/* enable SoftFloat support for the extended and quad formats: */
#define FLOATX80
#define FLOAT128

/* the SoftFloat types: */
typedef tme_uint32_t float32;
typedef tme_uint64_t float64;
typedef struct tme_float_ieee754_extended80 floatx80;
typedef struct {
#ifdef _TME_WORDS_BIGENDIAN
  tme_uint64_t high;
  tme_uint64_t low;
#else  /* !_TME_WORDS_BIGENDIAN */
  tme_uint64_t low;
  tme_uint64_t high;
#endif /* !_TME_WORDS_BIGENDIAN */
} float128;

#else  /* !TME_HAVE_INT64_T */

/* the SoftFloat types: */
typedef tme_uint32_t float32;
typedef struct {
#ifdef _TME_WORDS_BIGENDIAN
  tme_uint32_t high;
  tme_uint32_t low;
#else  /* !_TME_WORDS_BIGENDIAN */
  tme_uint32_t low;
  tme_uint32_t high;
#endif /* !_TME_WORDS_BIGENDIAN */
} float64;

#endif /* !TME_HAVE_INT64_T */

/* undefine all SoftFloat glue magic: */
#undef tme_float_ieee754_extended80
#undef tme_float_ieee754_extended80_significand
#undef tme_float_ieee754_extended80_sexp

/*----------------------------------------------------------------------------
| Each of the following `typedef's defines the most convenient type that holds
| integers of at least as many bits as specified.  For example, `uint8' should
| be the most convenient type that can hold unsigned integers of as many as
| 8 bits.  The `flag' type must be able to hold either a 0 or 1.  For most
| implementations of C, `flag', `uint8', and `int8' should all be `typedef'ed
| to the same as `int'.
*----------------------------------------------------------------------------*/
typedef tme_uint8_t flag;
typedef tme_uint8_t uint8;
typedef tme_int8_t int8;
typedef int uint16;
typedef int int16;
typedef tme_uint32_t uint32;
typedef tme_int32_t int32;
#ifdef TME_HAVE_INT64_T
typedef tme_uint64_t uint64;
typedef tme_int64_t int64;
#endif /* TME_HAVE_INT64_T */

/*----------------------------------------------------------------------------
| Each of the following `typedef's defines a type that holds integers
| of _exactly_ the number of bits specified.  For instance, for most
| implementation of C, `bits16' and `sbits16' should be `typedef'ed to
| `unsigned short int' and `signed short int' (or `short int'), respectively.
*----------------------------------------------------------------------------*/
typedef tme_uint8_t bits8;
typedef tme_int8_t sbits8;
typedef tme_uint16_t bits16;
typedef tme_int16_t sbits16;
typedef tme_uint32_t bits32;
typedef tme_int32_t sbits32;
#ifdef TME_HAVE_INT64_T
typedef tme_uint64_t bits64;
typedef tme_int64_t sbits64;
#endif /* TME_HAVE_INT64_T */

/*----------------------------------------------------------------------------
| Software IEC/IEEE integer-to-floating-point conversion routines.
*----------------------------------------------------------------------------*/
float32 int32_to_float32 _TME_P(( tme_int32_t ));
float64 int32_to_float64 _TME_P(( tme_int32_t ));
#ifdef FLOATX80
floatx80 int32_to_floatx80 _TME_P(( tme_int32_t ));
#endif /* FLOATX80 */
#ifdef FLOAT128
float128 int32_to_float128 _TME_P(( tme_int32_t ));
#endif /* FLOAT128 */
#ifdef TME_HAVE_INT64_T
float32 int64_to_float32 _TME_P(( tme_int64_t ));
float64 int64_to_float64 _TME_P(( tme_int64_t ));
#endif /* TME_HAVE_INT64_T */
#ifdef FLOATX80
floatx80 int64_to_floatx80 _TME_P(( tme_int64_t ));
#endif /* FLOATX80 */
#ifdef FLOAT128
float128 int64_to_float128 _TME_P(( tme_int64_t ));
#endif /* FLOAT128 */

/*----------------------------------------------------------------------------
| Software IEC/IEEE single-precision conversion routines.
*----------------------------------------------------------------------------*/
tme_int32_t float32_to_int32 _TME_P(( float32 ));
tme_int32_t float32_to_int32_round_to_zero _TME_P(( float32 ));
#ifdef TME_HAVE_INT64_T
tme_int64_t float32_to_int64 _TME_P(( float32 ));
tme_int64_t float32_to_int64_round_to_zero _TME_P(( float32 ));
#endif /* TME_HAVE_INT64_T */
float64 float32_to_float64 _TME_P(( float32 ));
#ifdef FLOATX80
floatx80 float32_to_floatx80 _TME_P(( float32 ));
#endif /* FLOATX80 */
#ifdef FLOAT128
float128 float32_to_float128 _TME_P(( float32 ));
#endif /* FLOAT128 */

/*----------------------------------------------------------------------------
| Software IEC/IEEE single-precision operations.
*----------------------------------------------------------------------------*/
float32 float32_round_to_int _TME_P(( float32 ));
float32 float32_add _TME_P(( float32, float32 ));
float32 float32_sub _TME_P(( float32, float32 ));
float32 float32_mul _TME_P(( float32, float32 ));
float32 float32_div _TME_P(( float32, float32 ));
float32 float32_rem _TME_P(( float32, float32 ));
float32 float32_sqrt _TME_P(( float32 ));
flag float32_eq _TME_P(( float32, float32 ));
flag float32_le _TME_P(( float32, float32 ));
flag float32_lt _TME_P(( float32, float32 ));
flag float32_eq_signaling _TME_P(( float32, float32 ));
flag float32_le_quiet _TME_P(( float32, float32 ));
flag float32_lt_quiet _TME_P(( float32, float32 ));

/*----------------------------------------------------------------------------
| Software IEC/IEEE double-precision conversion routines.
*----------------------------------------------------------------------------*/
tme_int32_t float64_to_int32 _TME_P(( float64 ));
tme_int32_t float64_to_int32_round_to_zero _TME_P(( float64 ));
#ifdef TME_HAVE_INT64_T
tme_int64_t float64_to_int64 _TME_P(( float64 ));
tme_int64_t float64_to_int64_round_to_zero _TME_P(( float64 ));
#endif /* TME_HAVE_INT64_T */
float32 float64_to_float32 _TME_P(( float64 ));
#ifdef FLOATX80
floatx80 float64_to_floatx80 _TME_P(( float64 ));
#endif /* FLOATX80 */
#ifdef FLOAT128
float128 float64_to_float128 _TME_P(( float64 ));
#endif /* FLOAT128 */

/*----------------------------------------------------------------------------
| Software IEC/IEEE double-precision operations.
*----------------------------------------------------------------------------*/
float64 float64_round_to_int _TME_P(( float64 ));
float64 float64_add _TME_P(( float64, float64 ));
float64 float64_sub _TME_P(( float64, float64 ));
float64 float64_mul _TME_P(( float64, float64 ));
float64 float64_div _TME_P(( float64, float64 ));
float64 float64_rem _TME_P(( float64, float64 ));
float64 float64_sqrt _TME_P(( float64 ));
flag float64_eq _TME_P(( float64, float64 ));
flag float64_le _TME_P(( float64, float64 ));
flag float64_lt _TME_P(( float64, float64 ));
flag float64_eq_signaling _TME_P(( float64, float64 ));
flag float64_le_quiet _TME_P(( float64, float64 ));
flag float64_lt_quiet _TME_P(( float64, float64 ));

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision conversion routines.
*----------------------------------------------------------------------------*/
tme_int32_t floatx80_to_int32 _TME_P(( floatx80 ));
tme_int32_t floatx80_to_int32_round_to_zero _TME_P(( floatx80 ));
#ifdef TME_HAVE_INT64_T
tme_int64_t floatx80_to_int64 _TME_P(( floatx80 ));
tme_int64_t floatx80_to_int64_round_to_zero _TME_P(( floatx80 ));
#endif /* TME_HAVE_INT64_T */
float32 floatx80_to_float32 _TME_P(( floatx80 ));
float64 floatx80_to_float64 _TME_P(( floatx80 ));
#ifdef FLOAT128
float128 floatx80_to_float128 _TME_P(( floatx80 ));
#endif /* FLOAT128 */

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision operations.
*----------------------------------------------------------------------------*/
floatx80 floatx80_round_to_int _TME_P(( floatx80 ));
floatx80 floatx80_add _TME_P(( floatx80, floatx80 ));
floatx80 floatx80_sub _TME_P(( floatx80, floatx80 ));
floatx80 floatx80_mul _TME_P(( floatx80, floatx80 ));
floatx80 floatx80_div _TME_P(( floatx80, floatx80 ));
floatx80 floatx80_rem _TME_P(( floatx80, floatx80 ));
floatx80 floatx80_sqrt _TME_P(( floatx80 ));
flag floatx80_eq _TME_P(( floatx80, floatx80 ));
flag floatx80_le _TME_P(( floatx80, floatx80 ));
flag floatx80_lt _TME_P(( floatx80, floatx80 ));
flag floatx80_eq_signaling _TME_P(( floatx80, floatx80 ));
flag floatx80_le_quiet _TME_P(( floatx80, floatx80 ));
flag floatx80_lt_quiet _TME_P(( floatx80, floatx80 ));

#endif /* FLOATX80 */

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Software IEC/IEEE quadruple-precision conversion routines.
*----------------------------------------------------------------------------*/
tme_int32_t float128_to_int32 _TME_P(( float128 ));
tme_int32_t float128_to_int32_round_to_zero _TME_P(( float128 ));
tme_int64_t float128_to_int64 _TME_P(( float128 ));
tme_int64_t float128_to_int64_round_to_zero _TME_P(( float128 ));
float32 float128_to_float32 _TME_P(( float128 ));
float64 float128_to_float64 _TME_P(( float128 ));
#ifdef FLOATX80
floatx80 float128_to_floatx80 _TME_P(( float128 ));
#endif

/*----------------------------------------------------------------------------
| Software IEC/IEEE quadruple-precision operations.
*----------------------------------------------------------------------------*/
float128 float128_round_to_int _TME_P(( float128 ));
float128 float128_add _TME_P(( float128, float128 ));
float128 float128_sub _TME_P(( float128, float128 ));
float128 float128_mul _TME_P(( float128, float128 ));
float128 float128_div _TME_P(( float128, float128 ));
float128 float128_rem _TME_P(( float128, float128 ));
float128 float128_sqrt _TME_P(( float128 ));
flag float128_eq _TME_P(( float128, float128 ));
flag float128_le _TME_P(( float128, float128 ));
flag float128_lt _TME_P(( float128, float128 ));
flag float128_eq_signaling _TME_P(( float128, float128 ));
flag float128_le_quiet _TME_P(( float128, float128 ));
flag float128_lt_quiet _TME_P(( float128, float128 ));

#endif /* FLOAT128 */

#endif /* _IC_IEEE754_SOFTFLOAT_TME_H */
