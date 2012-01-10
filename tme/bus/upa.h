/* $Id: upa.h,v 1.1 2009/02/28 16:48:11 fredette Exp $ */

/* tme/bus/upa.h - public header file for UPA bus emulation */

/*
 * Copyright (c) 2008 Matt Fredette
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

#ifndef _TME_BUS_UPA_H
#define _TME_BUS_UPA_H

#include <tme/common.h>
_TME_RCSID("$Id: upa.h,v 1.1 2009/02/28 16:48:11 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/completion.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>

/* macros: */

/* UPA capabilities: */
#define TME_UPA_UPACAP_HANDLERSLAVE		TME_BIT(4)
#define TME_UPA_UPACAP_INTERRUPTMASTER		TME_BIT(3)
#define TME_UPA_UPACAP_SLAVE_INT_L		TME_BIT(2)
#define TME_UPA_UPACAP_CACHEMASTER		TME_BIT(1)
#define TME_UPA_UPACAP_MASTER			TME_BIT(0)

/* fields in a UPA port ID register: */
#define TME_UPA_PORT_ID_COOKIE			(((tme_uint64_t) 0xfc) << 56)
#define TME_UPA_PORT_ID_ECC_NOT_VALID		(((tme_uint64_t) 1) << 34)
#define TME_UPA_PORT_ID_ONEREAD			(((tme_uint64_t) 1) << 33)
#define TME_UPA_PORT_ID_PINT_RDQ		((((tme_uint64_t) 1) << 32) - (((tme_uint64_t) 1) << 31))
#define TME_UPA_PORT_ID_PREQ_DQ			((((tme_uint32_t) 2) << 30) - (1 << 25))
#define TME_UPA_PORT_ID_PREQ_RQ			((2 << 24) - (1 << 21))
#define TME_UPA_PORT_ID_UPACAP			((2 << 20) - (1 << 16))
#define TME_UPA_PORT_ID_ID			((2 << 15) - (1 << 0))

/* fields in a UPA port config register: */
#define TME_UPA_PORT_CONFIG_SCIQ1		((2 << 7) - (1 << 4))
#define TME_UPA_PORT_CONFIG_SCIQ0		((2 << 3) - (1 << 0))

/* types: */

/* a UPA bus connection: */
struct tme_upa_bus_connection {

  /* a generic bus connection: */
  struct tme_bus_connection tme_upa_bus_connection;

  /* the master ID for this connection: */
  tme_uint32_t tme_upa_bus_connection_mid;

  /* the UPA interrupt function: */
#ifdef TME_HAVE_INT64_T
  void (*tme_upa_bus_interrupt) _TME_P((struct tme_upa_bus_connection *,
					tme_uint32_t,
					const tme_uint64_t *,
					struct tme_completion *));
#endif /* TME_HAVE_INT64_T */
};

#endif /* !_TME_BUS_UPA_H */
