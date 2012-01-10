/* $Id: i825x6.h,v 1.1 2004/05/04 01:17:27 fredette Exp $ */

/* tme/ic/i825x6.h - public header file for Intel 825x6 emulation */

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

/* version enforcement: */
#if !defined(TME_I825X6_VERSION) || TME_X_VERSION_CURRENT(TME_I825X6_VERSION) != 0
#error "check your sources; <tme/ic/i825x6.h> version is now 0"
#endif

#ifndef _TME_IC_I825X6_H
#define _TME_IC_I825X6_H

#include <tme/common.h>
_TME_RCSID("$Id: i825x6.h,v 1.1 2004/05/04 01:17:27 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>

/* macros: */

/* the i825x6 bus signals set: */
#define TME_I825X6_SIGNAL_CA	TME_BUS_SIGNAL_X(0)
#define TME_I825X6_SIGNAL_LOOP	TME_BUS_SIGNAL_X(1)
#define TME_BUS_SIGNALS_I825X6	{ TME_BUS_SIGNALS_ID_I825X6, TME_I825X6_VERSION, 8, 0 }

#endif /* !_TME_IC_I825X6_H */
