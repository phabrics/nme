/* $Id: nec765.h,v 1.1 2006/11/15 23:06:54 fredette Exp $ */

/* tme/ic/nec765.h - public header file for NEC 765 (and Intel 8207x) emulation */

/*
 * Copyright (c) 2006 Matt Fredette
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
#if !defined(TME_NEC765_VERSION) || TME_X_VERSION_CURRENT(TME_NEC765_VERSION) != 0
#error "check your sources; <tme/ic/nec765.h> version is now 0"
#endif

#ifndef _TME_IC_NEC765_H
#define _TME_IC_NEC765_H

#include <tme/common.h>
_TME_RCSID("$Id: nec765.h,v 1.1 2006/11/15 23:06:54 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>

/* macros: */

/* the socket version: */
#define TME_NEC765_SOCKET_0	(0)

/* the nec765 bus signals set: */
#define TME_NEC765_SIGNAL_DENSITY_HIGH	TME_BUS_SIGNAL_X(0)
#define TME_NEC765_SIGNAL_DISK_CHANGED	TME_BUS_SIGNAL_X(1)
#define TME_NEC765_SIGNAL_DRIVE_SELECT	TME_BUS_SIGNAL_X(2)
#define TME_NEC765_SIGNAL_DRIVE_TC	TME_BUS_SIGNAL_X(3)
#define TME_NEC765_SIGNAL_DISK_EJECT	TME_BUS_SIGNAL_X(4)
#define TME_BUS_SIGNALS_NEC765	{ TME_BUS_SIGNALS_ID_NEC765, TME_NEC765_VERSION, 8, 0 }

/* structures: */

/* the device socket: */
struct tme_nec765_socket {
  
  /* the version number of this structure: */
  unsigned int tme_nec765_socket_version;
};

#endif /* !_TME_IC_NEC765_H */
