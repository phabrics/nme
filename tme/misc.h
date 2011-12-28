/* $Id: misc.h,v 1.4 2009/11/08 17:25:47 fredette Exp $ */

/* tme/misc.h - public header file for miscellaneous things: */

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

#ifndef _TME_MISC_H
#define _TME_MISC_H

#include <tme/common.h>
_TME_RCSID("$Id: misc.h,v 1.4 2009/11/08 17:25:47 fredette Exp $");

/* types: */

/* a cycles scaling: */
#ifdef _TME_HAVE_LONG_DOUBLE
typedef long double tme_misc_cycles_scaling_t;
#else  /* !_TME_HAVE_LONG_DOUBLE */
typedef double tme_misc_cycles_scaling_t;
#endif /* !_TME_HAVE_LONG_DOUBLE */

/* prototypes: */
int tme_init _TME_P((void));
char ** tme_misc_tokenize _TME_P((_tme_const char *, char comment, int *));
#ifdef TME_HAVE_INT64_T
#define _tme_unumber_t tme_uint64_t
#define _tme_number_t tme_int64_t
#else  /* !TME_HAVE_INT64_T */
#define _tme_unumber_t tme_uint32_t
#define _tme_number_t tme_int32_t
#endif /* !TME_HAVE_INT64_T */
_tme_unumber_t tme_misc_unumber_parse_any _TME_P((_tme_const char *, int *));
_tme_number_t tme_misc_number_parse_any _TME_P((_tme_const char *, int *));
_tme_unumber_t tme_misc_unumber_parse _TME_P((_tme_const char *, _tme_unumber_t));
_tme_number_t tme_misc_number_parse _TME_P((_tme_const char *, _tme_number_t));
#undef _tme_unumber_t
#undef _tme_number_t
union tme_value64 tme_misc_cycles_scaled _TME_P((const tme_misc_cycles_scaling_t *, const union tme_value64 *));
void tme_misc_cycles_scaling _TME_P((tme_misc_cycles_scaling_t *, tme_uint32_t, tme_uint32_t));
tme_uint32_t tme_misc_cycles_per_ms _TME_P((void));
union tme_value64 tme_misc_cycles _TME_P((void));
void tme_misc_cycles_spin_until _TME_P((const union tme_value64 *));

#endif /* !_TME_MISC_H */
