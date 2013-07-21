/* $Id: am9513.h,v 1.4 2009/08/29 21:22:47 fredette Exp $ */

/* tme/ic/am9513.h - public header file for Am9513 emulation */

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

#ifndef _TME_IC_AM9513_H
#define _TME_IC_AM9513_H

#include <tme/common.h>
_TME_RCSID("$Id: am9513.h,v 1.4 2009/08/29 21:22:47 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/generic/bus.h>

/* macros: */
#define TME_AM9513_SOCKET_0	(0)

/* structures: */
struct tme_am9513_socket {
  
  /* the version number of this structure: */
  unsigned int tme_am9513_socket_version;

  /* the bus address of the command register: */
  tme_bus_addr32_t tme_am9513_socket_address_cmd;

  /* the bus address of the data register: */
  tme_bus_addr32_t tme_am9513_socket_address_data;

  /* the system bus byte lane the chip is wired to: */
  unsigned int tme_am9513_socket_port_least_lane;

  /* the basic clock provided to the chip: */
  tme_uint32_t tme_am9513_socket_basic_clock;

  /* this maps counter numbers to bus signals: */
  unsigned int tme_am9513_socket_counter_signals[5];
};

#endif /* !_TME_IC_AM9513_H */
