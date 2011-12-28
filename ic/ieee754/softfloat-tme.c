/* $Id: softfloat-tme.c,v 1.2 2005/05/14 01:42:28 fredette Exp $ */

/* ic/ieee754/softfloat-tme.c - this glues tme to SoftFloat: */

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

#include <tme/common.h>
_TME_RCSID("$Id: softfloat-tme.c,v 1.2 2005/05/14 01:42:28 fredette Exp $");

/* includes: */
#define _TME_IEEE754_SOFTFLOAT_GLUE
#include "softfloat-tme.h"

/* types: */
typedef struct tme_ieee754_nan commonNaNT;

/* globals: */

/* the softfloat lock: */
tme_mutex_t tme_ieee754_global_mutex;

/* the softfloat global control: */
struct tme_ieee754_ctl *tme_ieee754_global_ctl;

/* the softfloat global exceptions: */
tme_int8_t tme_ieee754_global_exceptions;

#ifdef TME_HAVE_INT64_T
/*----------------------------------------------------------------------------
| The `LIT64' macro takes as its argument a textual integer literal and
| if necessary ``marks'' the literal as having a 64-bit integer type.
| For example, the GNU C Compiler (`gcc') requires that 64-bit literals be
| appended with the letters `LL' standing for `long long', which is `gcc's
| name for the 64-bit integer type.  Some compilers may allow `LIT64' to be
| defined as the identity macro:  `#define LIT64( a ) a'.
*----------------------------------------------------------------------------*/
#if defined(__GNUC__) && (_TME_SIZEOF_LONG == 4)
#define LIT64( a ) a##LL
#else
#define LIT64( a ) a
#endif
#endif /* TME_HAVE_INT64_T */

/*----------------------------------------------------------------------------
| The macro `INLINE' can be used before functions that should be inlined.  If
| a compiler does not support explicit inlining, this macro should be defined
| to be `static'.
*----------------------------------------------------------------------------*/
#define INLINE static inline

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point underflow tininess-detection mode.
*----------------------------------------------------------------------------*/
#define float_detect_tininess		(tme_ieee754_global_ctl->tme_ieee754_ctl_detect_tininess)
#define float_tininess_after_rounding	TME_IEEE754_CTL_DETECT_TININESS_AFTER_ROUNDING
#define float_tininess_before_rounding	TME_IEEE754_CTL_DETECT_TININESS_BEFORE_ROUNDING

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point rounding mode.
*----------------------------------------------------------------------------*/
#define float_rounding_mode		(tme_ieee754_global_ctl->tme_ieee754_ctl_rounding_mode)
#define float_round_nearest_even	TME_FLOAT_ROUND_NEAREST_EVEN
#define float_round_down		TME_FLOAT_ROUND_DOWN
#define float_round_up			TME_FLOAT_ROUND_UP
#define float_round_to_zero		TME_FLOAT_ROUND_TO_ZERO

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point exception flags.
*----------------------------------------------------------------------------*/
#define float_exception_flags		tme_ieee754_global_exceptions
#define float_flag_invalid		TME_FLOAT_EXCEPTION_INVALID
#define float_flag_divbyzero		TME_FLOAT_EXCEPTION_DIVBYZERO
#define float_flag_overflow		TME_FLOAT_EXCEPTION_OVERFLOW
#define float_flag_underflow		TME_FLOAT_EXCEPTION_UNDERFLOW
#define float_flag_inexact		TME_FLOAT_EXCEPTION_INEXACT

/*----------------------------------------------------------------------------
| Routine to raise any or all of the software IEC/IEEE floating-point
| exception flags.
*----------------------------------------------------------------------------*/
#define float_raise(excp)			\
do {						\
  tme_ieee754_global_exceptions |= (excp);	\
  (*tme_ieee754_global_ctl->tme_ieee754_ctl_exception)(tme_ieee754_global_ctl, tme_ieee754_global_exceptions);\
} while (/* CONSTCOND */ 0)

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision rounding precision.  Valid
| values are 32, 64, and 80.
*----------------------------------------------------------------------------*/
#define floatx80_rounding_precision	(tme_ieee754_global_ctl->tme_ieee754_ctl_extended80_rounding_precision)

/* include either the 32-bit or 64-bit SoftFloat macros: */
#ifdef TME_HAVE_INT64_T
#include "dist/softfloat/softfloat/bits64/softfloat-macros"
#else  /* !TME_HAVE_INT64_T */
#include "dist/softfloat/softfloat/bits32/softfloat-macros"
#endif /* !TME_HAVE_INT64_T */

/*----------------------------------------------------------------------------
| The pattern for a default generated single-precision NaN.
*----------------------------------------------------------------------------*/

#define float32_default_nan	(tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_single)

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is a signaling
| NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

INLINE flag
float32_is_signaling_nan( float32 a )
{
  return ((*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_single)(&a));
}

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point NaN
| `a' to the canonical NaN format.  If `a' is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_single_to_common(tme_uint32_t a, 
					 struct tme_ieee754_nan *z)
{
  z->tme_ieee754_nan_sign = a >> 31;
  z->tme_ieee754_nan_hi.tme_value64_uint32_hi = a << 9;
  z->tme_ieee754_nan_hi.tme_value64_uint32_lo = 0;
  z->tme_ieee754_nan_lo.tme_value64_uint32_hi = 0;
  z->tme_ieee754_nan_lo.tme_value64_uint32_lo = 0;
}

INLINE commonNaNT
float32ToCommonNaN( float32 a )
{
  commonNaNT z;

  if (tme_ieee754_global_ctl->tme_ieee754_ctl_check_snan_on_conversion
      && (*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_single)(&a)) {
    float_raise( float_flag_invalid );
  }
  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_single_to_common)(a, &z);
  return (z);
}

/*----------------------------------------------------------------------------
| Returns the result of converting the canonical NaN `a' to the single-
| precision floating-point format.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_common_to_single(const struct tme_ieee754_nan *z, 
					 tme_uint32_t *a)
{
  *a = ((((bits32) z->tme_ieee754_nan_sign) << 31)
	| 0x7FC00000
	| (z->tme_ieee754_nan_hi.tme_value64_uint32_hi >> 9)
	);
}

INLINE float32
commonNaNToFloat32( commonNaNT a )
{
  float32 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_common_to_single)(&a, &z);
  return (z);
}

/*----------------------------------------------------------------------------
| Takes two single-precision floating-point values `a' and `b', one of which
| is a NaN, and returns the appropriate NaN result.  If either `a' or `b' is a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/

INLINE float32
propagateFloat32NaN( float32 a, float32 b )
{
  float32 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_from_nans_single)(tme_ieee754_global_ctl, &a, &b, &z);
  return (z);
}

/*----------------------------------------------------------------------------
| The pattern for a default generated double-precision NaN.
*----------------------------------------------------------------------------*/
#ifdef TME_HAVE_INT64_T
#define float64_default_nan	 (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint)
#else  /* !TME_HAVE_INT64_T */
#define float64_default_nan_high (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_hi)
#define float64_default_nan_low  (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_double.tme_value64_uint32_lo)
#endif /* !TME_HAVE_INT64_T */

#define TME_FLOAT64_OUT(a)	((union tme_value64 *) (&(a)))

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is a signaling
| NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

INLINE flag
float64_is_signaling_nan( float64 a )
{
  return ((*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_double)(TME_FLOAT64_OUT(a)));
}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point NaN
| `a' to the canonical NaN format.  If `a' is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_double_to_common(const union tme_value64 *a, 
					 struct tme_ieee754_nan *z)
{
  z->tme_ieee754_nan_sign = a->tme_value64_uint32_hi >> 31;
#ifdef TME_HAVE_INT64_T
  z->tme_ieee754_nan_hi.tme_value64_uint = a->tme_value64_uint << 12;
#else  /* !TME_HAVE_INT64_T */
  shortShift64Left( a->tme_value64_uint32_hi, 
		    a->tme_value64_uint32_lo, 
		    12, 
		    &z->tme_ieee754_nan_hi.tme_value64_uint32_hi, 
		    &z->tme_ieee754_nan_hi.tme_value64_uint32_lo );
#endif /* !TME_HAVE_INT64_T */
  z->tme_ieee754_nan_lo.tme_value64_uint32_hi = 0;
  z->tme_ieee754_nan_lo.tme_value64_uint32_lo = 0;
}

INLINE commonNaNT
float64ToCommonNaN( float64 a )
{
  commonNaNT z;

  if (tme_ieee754_global_ctl->tme_ieee754_ctl_check_snan_on_conversion
      && (*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_double)(TME_FLOAT64_OUT(a))) {
    float_raise( float_flag_invalid );
  }
  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_double_to_common)(TME_FLOAT64_OUT(a), &z);
  return (z);
}

/*----------------------------------------------------------------------------
| Returns the result of converting the canonical NaN `a' to the double-
| precision floating-point format.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_common_to_double(const struct tme_ieee754_nan *a,
					 union tme_value64 *z)
{
#ifdef TME_HAVE_INT64_T
  z->tme_value64_uint = a->tme_ieee754_nan_hi.tme_value64_uint >> 12;
#else  /* TME_HAVE_INT64_T */
  shift64Right( a->tme_ieee754_nan_hi.tme_value64_uint32_hi, 
		a->tme_ieee754_nan_hi.tme_value64_uint32_lo, 
		12, 
		&z->tme_value64_uint32_hi, 
		&z->tme_value64_uint32_lo );
#endif /* TME_HAVE_INT64_T */
  z->tme_value64_uint32_hi |= ( ( (bits32) a->tme_ieee754_nan_sign ) << 31 ) | 0x7FF80000;
}

INLINE float64
commonNaNToFloat64( commonNaNT a )
{
  float64 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_common_to_double)(&a, TME_FLOAT64_OUT(z));
  return (z);
}

/*----------------------------------------------------------------------------
| Takes two double-precision floating-point values `a' and `b', one of which
| is a NaN, and returns the appropriate NaN result.  If either `a' or `b' is a
| signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/

INLINE float64 
propagateFloat64NaN( float64 a, float64 b )
{
  float64 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_from_nans_double)(tme_ieee754_global_ctl, 
								  TME_FLOAT64_OUT(a), 
								  TME_FLOAT64_OUT(b),
								  TME_FLOAT64_OUT(z));
  return (z);
}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| The pattern for a default generated extended double-precision NaN.  The
| `high' and `low' values hold the most- and least-significant bits,
| respectively.
*----------------------------------------------------------------------------*/
#define floatx80_default_nan_high (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_extended80.tme_float_ieee754_extended80_sexp)
#define floatx80_default_nan_low  (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_extended80.tme_float_ieee754_extended80_significand.tme_value64_uint)

#define TME_FLOATX80_OUT(a)	((struct tme_float_ieee754_extended80 *) &(a))

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is a
| signaling NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

INLINE flag 
floatx80_is_signaling_nan( floatx80 a )
{
  return ((*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_extended80)(TME_FLOATX80_OUT(a)));
}

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point NaN `a' to the canonical NaN format.  If `a' is a signaling NaN, the
| invalid exception is raised.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_extended80_to_common(const struct tme_float_ieee754_extended80 *a,
					     struct tme_ieee754_nan *z)
{
  z->tme_ieee754_nan_sign = a->tme_float_ieee754_extended80_sexp >> 15;
#ifdef TME_HAVE_INT64_T
  z->tme_ieee754_nan_hi.tme_value64_uint = a->tme_float_ieee754_extended80_significand.tme_value64_uint << 1;
#else  /* !TME_HAVE_INT64_T */
  shortShift64Left(a->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi,
		   a->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo,
		   1,
		   &z->tme_ieee754_nan_hi.tme_value64_uint32_hi,
		   &z->tme_ieee754_nan_hi.tme_value64_uint32_lo);
#endif /* !TME_HAVE_INT64_T */
  z->tme_ieee754_nan_lo.tme_value64_uint32_hi = 0;
  z->tme_ieee754_nan_lo.tme_value64_uint32_lo = 0;
}

INLINE commonNaNT 
floatx80ToCommonNaN( floatx80 a )
{
  commonNaNT z;

  if (tme_ieee754_global_ctl->tme_ieee754_ctl_check_snan_on_conversion
      && (*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_extended80)(TME_FLOATX80_OUT(a))) {
    float_raise( float_flag_invalid );
  }
  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_extended80_to_common)(TME_FLOATX80_OUT(a), &z);
  return (z);
}

/*----------------------------------------------------------------------------
| Returns the result of converting the canonical NaN `a' to the extended
| double-precision floating-point format.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_common_to_extended80(const struct tme_ieee754_nan *a,
					     struct tme_float_ieee754_extended80 *z)
{
  z->tme_float_ieee754_extended80_sexp = (((tme_uint16_t) a->tme_ieee754_nan_sign) << 15) | 0x7FFF;
#ifdef TME_HAVE_INT64_T
  z->tme_float_ieee754_extended80_significand.tme_value64_uint = a->tme_ieee754_nan_hi.tme_value64_uint >> 1;
#else  /* !TME_HAVE_INT64_T */
  shift64Right(a->tme_ieee754_nan_hi.tme_value64_uint32_hi,
	       a->tme_ieee754_nan_hi.tme_value64_uint32_lo,
	       1,
	       z->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi,
	       z->tme_float_ieee754_extended80_significand.tme_value64_uint32_lo);
#endif /* !TME_HAVE_INT64_T */
  z->tme_float_ieee754_extended80_significand.tme_value64_uint32_hi |= 0xC0000000;
}

INLINE floatx80
commonNaNToFloatx80( commonNaNT a )
{
  floatx80 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_common_to_extended80)(&a, TME_FLOATX80_OUT(z));
  return (z);
}

/*----------------------------------------------------------------------------
| Takes two extended double-precision floating-point values `a' and `b', one
| of which is a NaN, and returns the appropriate NaN result.  If either `a' or
| `b' is a signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/

INLINE floatx80
propagateFloatx80NaN( floatx80 a, floatx80 b )
{
  floatx80 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_from_nans_extended80)(tme_ieee754_global_ctl,
								      TME_FLOATX80_OUT(a), 
								      TME_FLOATX80_OUT(b),
								      TME_FLOATX80_OUT(z));
  return (z);
}

#endif /* FLOATX80 */

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| The pattern for a default generated quadruple-precision NaN.  The `high' and
| `low' values hold the most- and least-significant bits, respectively.
*----------------------------------------------------------------------------*/
#define float128_default_nan_high (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_hi.tme_value64_uint)
#define float128_default_nan_low  (tme_ieee754_global_ctl->tme_ieee754_ctl_default_nan_quad.tme_float_ieee754_quad_lo.tme_value64_uint)

#define TME_FLOAT128_OUT(a)	((struct tme_float_ieee754_quad *) &(a))

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is a
| signaling NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

INLINE flag
float128_is_signaling_nan( float128 a )
{
  return ((*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_quad)(TME_FLOAT128_OUT(a)));
}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point NaN
| `a' to the canonical NaN format.  If `a' is a signaling NaN, the invalid
| exception is raised.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_quad_to_common(const struct tme_float_ieee754_quad *a,
				       struct tme_ieee754_nan *z)
{
  z->tme_ieee754_nan_sign = a->tme_float_ieee754_quad_hi.tme_value64_uint32_hi >> 31;
  shortShift128Left( a->tme_float_ieee754_quad_hi.tme_value64_uint, 
		     a->tme_float_ieee754_quad_lo.tme_value64_uint, 
		     16, 
		     &z->tme_ieee754_nan_hi.tme_value64_uint, 
		     &z->tme_ieee754_nan_lo.tme_value64_uint );
}

INLINE commonNaNT
float128ToCommonNaN( float128 a )
{
  commonNaNT z;

  if (tme_ieee754_global_ctl->tme_ieee754_ctl_check_snan_on_conversion
      && (*tme_ieee754_global_ctl->tme_ieee754_ctl_is_snan_quad)(TME_FLOAT128_OUT(a))) {
    float_raise( float_flag_invalid );
  }
  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_quad_to_common)(TME_FLOAT128_OUT(a), &z);
  return (z);
}

/*----------------------------------------------------------------------------
| Returns the result of converting the canonical NaN `a' to the quadruple-
| precision floating-point format.
*----------------------------------------------------------------------------*/

void
tme_ieee754_default_nan_common_to_quad(const struct tme_ieee754_nan *a,
				       struct tme_float_ieee754_quad *z)
{
  shift128Right( a->tme_ieee754_nan_hi.tme_value64_uint, 
		 a->tme_ieee754_nan_lo.tme_value64_uint, 
		 16, 
		 &z->tme_float_ieee754_quad_hi.tme_value64_uint, 
		 &z->tme_float_ieee754_quad_lo.tme_value64_uint );
  z->tme_float_ieee754_quad_hi.tme_value64_uint32_hi |= (((tme_uint32_t) a->tme_ieee754_nan_sign) << 31) | 0x7FFF8000;
}

INLINE float128
commonNaNToFloat128( commonNaNT a )
{
  float128 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_common_to_quad)(&a, TME_FLOAT128_OUT(z));
  return (z);
}

/*----------------------------------------------------------------------------
| Takes two quadruple-precision floating-point values `a' and `b', one of
| which is a NaN, and returns the appropriate NaN result.  If either `a' or
| `b' is a signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/

INLINE float128 
propagateFloat128NaN( float128 a, float128 b )
{
  float128 z;

  (*tme_ieee754_global_ctl->tme_ieee754_ctl_nan_from_nans_quad)(tme_ieee754_global_ctl, 
								TME_FLOAT128_OUT(a), 
								TME_FLOAT128_OUT(b),
								TME_FLOAT128_OUT(z));
  return (z);
}

#endif /* FLOAT128 */

/* include either the 32-bit or 64-bit SoftFloat implementation: */
#ifdef TME_HAVE_INT64_T
#include "dist/softfloat/softfloat/bits64/softfloat.c"
#else  /* !TME_HAVE_INT64_T */
#include "dist/softfloat/softfloat/bits32/softfloat.c"
#endif /* !TME_HAVE_INT64_T */
