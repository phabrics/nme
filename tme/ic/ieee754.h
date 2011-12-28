/* $Id: ieee754.h,v 1.3 2006/11/16 02:54:40 fredette Exp $ */

/* tme/ic/ieee754.h - public header file for IEEE 754 emulation */

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

#ifndef _TME_IC_IEEE754_H
#define _TME_IC_IEEE754_H

#include <tme/common.h>
_TME_RCSID("$Id: ieee754.h,v 1.3 2006/11/16 02:54:40 fredette Exp $");

/* includes: */
#include <tme/generic/float.h>
#include <tme/threads.h>

/* macros: */

/* underflow tininess-detection modes: */
#define TME_IEEE754_CTL_DETECT_TININESS_AFTER_ROUNDING	(0)
#define TME_IEEE754_CTL_DETECT_TININESS_BEFORE_ROUNDING	(1)

/* floating point register file formats: */
#define TME_IEEE754_FPREG_FORMAT_NULL		(0)
#define TME_IEEE754_FPREG_FORMAT_SINGLE		(1)
#define TME_IEEE754_FPREG_FORMAT_DOUBLE		(2)
#define TME_IEEE754_FPREG_FORMAT_QUAD		(4)
#define TME_IEEE754_FPREG_FORMAT_ENDIAN_BIG	TME_BIT(5)
#define TME_IEEE754_FPREG_FORMAT_ENDIAN_LITTLE	(0)
#define TME_IEEE754_FPREG_FORMAT_BUILTIN	TME_BIT(6)

/* structures: */

/* the IEEE 754 canonical NaN type: */
struct tme_ieee754_nan {
  tme_uint8_t tme_ieee754_nan_sign;
  tme_uint16_t tme_ieee754_nan_exponent;
  union tme_value64 tme_ieee754_nan_hi;
  union tme_value64 tme_ieee754_nan_lo;
};

/* an IEEE 754 double precision constant: */
struct tme_ieee754_double_constant {
  
  /* the two 32-bit words of the double: */
  tme_uint32_t tme_ieee754_double_constant_hi;
  tme_uint32_t tme_ieee754_double_constant_lo;
};

/* an IEEE 754 extended80 precision constant: */
struct tme_ieee754_extended80_constant {

  tme_uint16_t tme_ieee754_extended80_constant_sexp;
  tme_uint32_t tme_ieee754_extended80_constant_significand_hi;
  tme_uint32_t tme_ieee754_extended80_constant_significand_lo;
};

/* an IEEE 754 quad precision constant: */
struct tme_ieee754_quad_constant {
  tme_uint32_t tme_ieee754_quad_constant_hi_hi;
  tme_uint32_t tme_ieee754_quad_constant_hi_lo;
  tme_uint32_t tme_ieee754_quad_constant_lo_hi;
  tme_uint32_t tme_ieee754_quad_constant_lo_lo;
};  

/* the IEEE 754 control: */
struct tme_ieee754_ctl {

  /* a private data structure: */
  void *tme_ieee754_ctl_private;

  /* this is one of the TME_IEEE754_CTL_DETECT_TININESS_* macros,
     describing the underflow tininess-detection mode: */
  tme_int8_t tme_ieee754_ctl_detect_tininess;

  /* the rounding mode: */
  tme_int8_t tme_ieee754_ctl_rounding_mode;

  /* the extended80 rounding precision: */
  tme_int8_t tme_ieee754_ctl_extended80_rounding_precision;

  /* whether or not a value is checked for a signaling NaN when
     converting it from one precision to another: */
  tme_int8_t tme_ieee754_ctl_check_snan_on_conversion;

  /* the exception function: */
  void (*tme_ieee754_ctl_exception) _TME_P((struct tme_ieee754_ctl *, tme_int8_t));

  /* any lock unlocking function: */
  int (*tme_ieee754_ctl_lock_unlock) _TME_P((void));

  /* the default generated NaN patterns: */
  tme_uint32_t tme_ieee754_ctl_default_nan_single;
  union tme_value64 tme_ieee754_ctl_default_nan_double;
  struct tme_float_ieee754_extended80 tme_ieee754_ctl_default_nan_extended80;
  struct tme_float_ieee754_quad tme_ieee754_ctl_default_nan_quad;

  /* signaling NaN test and nonsignaling conversion: */
  tme_int8_t (*tme_ieee754_ctl_is_snan_single) _TME_P((tme_uint32_t *));
  tme_int8_t (*tme_ieee754_ctl_is_snan_double) _TME_P((union tme_value64 *));
  tme_int8_t (*tme_ieee754_ctl_is_snan_extended80) _TME_P((struct tme_float_ieee754_extended80 *));
  tme_int8_t (*tme_ieee754_ctl_is_snan_quad) _TME_P((struct tme_float_ieee754_quad *));

  /* NaN canonicalization: */
  void (*tme_ieee754_ctl_nan_single_to_common) _TME_P((tme_uint32_t, struct tme_ieee754_nan *));
  void (*tme_ieee754_ctl_nan_common_to_single) _TME_P((_tme_const struct tme_ieee754_nan *, tme_uint32_t *));
  void (*tme_ieee754_ctl_nan_double_to_common) _TME_P((_tme_const union tme_value64 *, struct tme_ieee754_nan *));
  void (*tme_ieee754_ctl_nan_common_to_double) _TME_P((_tme_const struct tme_ieee754_nan *, union tme_value64 *));
  void (*tme_ieee754_ctl_nan_extended80_to_common) _TME_P((_tme_const struct tme_float_ieee754_extended80 *, struct tme_ieee754_nan *));
  void (*tme_ieee754_ctl_nan_common_to_extended80) _TME_P((_tme_const struct tme_ieee754_nan *, struct tme_float_ieee754_extended80 *));
  void (*tme_ieee754_ctl_nan_quad_to_common) _TME_P((_tme_const struct tme_float_ieee754_quad *, struct tme_ieee754_nan *));
  void (*tme_ieee754_ctl_nan_common_to_quad) _TME_P((_tme_const struct tme_ieee754_nan *, struct tme_float_ieee754_quad *));

  /* NaN propagation: */
  void (*tme_ieee754_ctl_nan_from_nans_single) _TME_P((struct tme_ieee754_ctl *, _tme_const tme_uint32_t *, _tme_const tme_uint32_t *, tme_uint32_t *));
  void (*tme_ieee754_ctl_nan_from_nans_double) _TME_P((struct tme_ieee754_ctl *, _tme_const union tme_value64 *, _tme_const union tme_value64 *, union tme_value64 *));
  void (*tme_ieee754_ctl_nan_from_nans_extended80) _TME_P((struct tme_ieee754_ctl *, _tme_const struct tme_float_ieee754_extended80 *, _tme_const struct tme_float_ieee754_extended80 *, struct tme_float_ieee754_extended80 *));
  void (*tme_ieee754_ctl_nan_from_nans_quad) _TME_P((struct tme_ieee754_ctl *, _tme_const struct tme_float_ieee754_quad *, _tme_const struct tme_float_ieee754_quad *, struct tme_float_ieee754_quad *));
};

/* include the operation sets: */
#ifdef _TME_IMPL
#include <ic/ieee754/ieee754-auto.h>
#include <ic/ieee754/ieee754-ops-auto.h>
#else
#include <tme/ic/ieee754-auto.h>
#include <tme/ic/ieee754-ops-auto.h>
#endif

/* prototypes: */

/* this looks up an operations structure: */
_tme_const struct tme_ieee754_ops *tme_ieee754_ops_lookup _TME_P((_tme_const char *));

/* NaN canonicalization defaults: */
void tme_ieee754_default_nan_single_to_common _TME_P((tme_uint32_t, struct tme_ieee754_nan *));
void tme_ieee754_default_nan_common_to_single _TME_P((_tme_const struct tme_ieee754_nan *, tme_uint32_t *));
void tme_ieee754_default_nan_double_to_common _TME_P((_tme_const union tme_value64 *, struct tme_ieee754_nan *));
void tme_ieee754_default_nan_common_to_double _TME_P((_tme_const struct tme_ieee754_nan *, union tme_value64 *));
void tme_ieee754_default_nan_extended80_to_common _TME_P((_tme_const struct tme_float_ieee754_extended80 *, struct tme_ieee754_nan *));
void tme_ieee754_default_nan_common_to_extended80 _TME_P((_tme_const struct tme_ieee754_nan *, struct tme_float_ieee754_extended80 *));
void tme_ieee754_default_nan_quad_to_common _TME_P((_tme_const struct tme_float_ieee754_quad *, struct tme_ieee754_nan *));
void tme_ieee754_default_nan_common_to_quad _TME_P((_tme_const struct tme_ieee754_nan *, struct tme_float_ieee754_quad *));

/* the native floating-point exception function: */
void tme_ieee754_exception_float _TME_P((int, void *));

/* the softfloat unlock function: */
int tme_ieee754_unlock_softfloat _TME_P((void));

/* for processors that manage a fundamentally single-precision
   floating-point register file, but that allow size-aligned sets of
   registers to combine into double- and quad-precision registers,
   this manages the register set and converts register contents
   between formats: */
void tme_ieee754_fpreg_format _TME_P((struct tme_float *, unsigned int *, unsigned int, unsigned int));

/* globals: */

/* this is a compliance options string: */
extern _tme_const char * _tme_const tme_ieee754_compliance_options;

/* the softfloat lock: */
extern tme_mutex_t tme_ieee754_global_mutex;

/* the softfloat global control: */
extern struct tme_ieee754_ctl *tme_ieee754_global_ctl;

/* the softfloat global exceptions: */
extern tme_int8_t tme_ieee754_global_exceptions;

/* constants: */
extern _tme_const tme_uint32_t tme_ieee754_single_constant_2e2ex[];
extern _tme_const tme_uint32_t tme_ieee754_single_constant_2e_minus_2ex[];
extern _tme_const tme_uint32_t tme_ieee754_single_constant_10e2ex[];
extern _tme_const tme_uint32_t tme_ieee754_single_constant_pi;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_log10_2;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_e;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_log2_e;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_log10_e;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_zero;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_ln_2;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_ln_10;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_one;
extern _tme_const tme_uint32_t tme_ieee754_single_constant_10e_minus_2ex[];
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_2e2ex[];
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_2e_minus_2ex[];
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_10e2ex[];
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_10e_minus_2ex[];
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_pi;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_log10_2;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_e;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_log2_e;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_log10_e;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_zero;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_ln_2;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_ln_10;
extern _tme_const struct tme_ieee754_double_constant tme_ieee754_double_constant_one;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_2e2ex[];
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_2e_minus_2ex[];
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_10e2ex[];
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_10e_minus_2ex[];
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_pi;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_log10_2;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_e;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_log2_e;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_log10_e;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_zero;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_ln_2;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_ln_10;
extern _tme_const struct tme_ieee754_extended80_constant tme_ieee754_extended80_constant_one;

#endif /* !_TME_IC_IEEE754_H */
