/* $Id: z8530.h,v 1.5 2009/08/29 21:22:47 fredette Exp $ */

/* tme/ic/z8530.h - public header file for Zilog 8530 emulation */

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

#ifndef _TME_IC_Z8530_H
#define _TME_IC_Z8530_H

#include <tme/common.h>
_TME_RCSID("$Id: z8530.h,v 1.5 2009/08/29 21:22:47 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/generic/bus.h>

/* macros: */
#define TME_Z8530_SOCKET_0	(0)

/* socket flags: */
#define TME_Z8530_SOCKET_FLAG_IEI_TIED_LOW	(1 << 0)

/* structures: */
struct tme_z8530_socket {
  
  /* the version number of this structure: */
  unsigned int tme_z8530_socket_version;

  /* flags: */
  unsigned int tme_z8530_socket_flags;

  /* the bus address of channel A: */
  tme_bus_addr32_t tme_z8530_socket_address_chan_a;

  /* the bus address of channel B: */
  tme_bus_addr32_t tme_z8530_socket_address_chan_b;

  /* within each channel, the offset of the csr register: */
  tme_bus_addr32_t tme_z8530_socket_offset_csr;

  /* within each channel, the offset of the data register: */
  tme_bus_addr32_t tme_z8530_socket_offset_data;

  /* the system bus byte lane the chip is wired to: */
  unsigned int tme_z8530_socket_port_least_lane;

  /* the clock rate provided to the chip: */
  tme_uint32_t tme_z8530_socket_pclk;
};

#endif /* !_TME_IC_Z8530_H */
