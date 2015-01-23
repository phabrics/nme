/* $Id: eth-impl.h,v 1.1 2003/05/18 00:02:23 fredette Exp $ */

/* host/eth/eth-impl.h - Ethernet host internal header */

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

#ifndef _HOST_ETH_IMPL_H
#define _HOST_ETH_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: eth-impl.h,v 1.1 2003/05/18 00:02:23 fredette Exp $");

/* includes: */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#include <ifaddrs.h>

/* structures: */

/* our internal data structure: */
struct tme_ethernet {

  /* backpointer to our element: */
  struct tme_element *tme_eth_element;

  /* our mutex: */
  tme_mutex_t tme_eth_mutex;

  /* our reader condition: */
  tme_cond_t tme_eth_cond_reader;

  /* the callout flags: */
  unsigned int tme_eth_callout_flags;

  /* our Ethernet connection: */
  struct tme_ethernet_connection *tme_eth_eth_connection;

  /* the Ethernet file descriptor: */
  int tme_eth_fd;

  /* the size of the packet buffer for the interface: */
  size_t tme_eth_buffer_size;

  /* the packet buffer for the interface: */
  tme_uint8_t *tme_eth_buffer;

  /* the next offset within the packet buffer, and the end of the data
     in the packet buffer: */
  size_t tme_eth_buffer_offset;
  size_t tme_eth_buffer_end;

  /* ethernet interface type-specific data */
  void *tme_eth_data;

  /* ethernet interface hardware address */
  unsigned char *tme_eth_addr;
  
  /* all packets received on or before this time can be released: */
  tme_time_t tme_eth_delay_release;

  /* when nonzero, the packet delay sleep time, in microseconds: */
  unsigned long tme_eth_delay_sleep;

  /* when nonzero, the packet delay is sleeping: */
  int tme_eth_delay_sleeping;
};

/* prototypes: */
int tme_eth_if_find _TME_P((_tme_const char *, 
			    struct ifreq **, 
			    tme_uint8_t **, 
			    unsigned int *));

int tme_eth_ifaddrs_find _TME_P((_tme_const char *, 
				 struct ifaddrs **, 
				 tme_uint8_t **, 
				 unsigned int *));

int tme_eth_alloc _TME_P((char *dev_filename,
			  char **_output));

int tme_eth_connections_new _TME_P((struct tme_element *element, 
				    const char * const *args, 
				    struct tme_connection **_conns));

int tme_eth_init _TME_P((struct tme_element *element, 
			 int fd, 
			 u_int sz, 
			 void *data,
			 unsigned char *addr,
			 typeof(tme_eth_connections_new) eth_connections_new));

#endif /* !_HOST_ETH_IMPL_H */
