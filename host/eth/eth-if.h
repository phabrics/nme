/* host/eth/eth-if.h - Common Ethernet host interface header */

/*
 * Copyright (c) 2014, 2015, 2016 Ruben Agin
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
#include <tme/generic/ethernet.h>
#include <tme/threads.h>
#include <tme/misc.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>	/* for offsetof */
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_LINKER_H
#include <sys/linker.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#elif defined(HAVE_SYS_SOCKETIO_H)
#include <sys/socketio.h> 
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_IOCTLS_H
#include <ioctls.h>
#endif
#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif
#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

/* structures: */

/* our internal data structure: */
struct tme_ethernet {

  /* backpointer to our element: */
  struct tme_element *tme_eth_element;

  /* our mutex: */
  tme_mutex_t tme_eth_mutex;

  /* our reader condition: */
  tme_cond_t tme_eth_cond_reader;

  /* our thread: */
  tme_threadid_t tme_eth_thread;

  /* the callout flags: */
  unsigned int tme_eth_callout_flags;

  /* our Ethernet connection: */
  struct tme_ethernet_connection *tme_eth_eth_connection;

  /* the Ethernet file descriptor: */
  tme_thread_handle_t tme_eth_handle;

  /* callbacks for ethernet i/o */
  int (*tme_ethernet_read)(void *data);
  int (*tme_ethernet_write)(void *data);

  /* the size of the packet buffer for the interface: */
  size_t tme_eth_buffer_size;

  /* the length of the packet buffer data for the interface: */
  size_t tme_eth_data_length;
  
  /* the packet buffer for the interface: */
  tme_uint8_t *tme_eth_buffer;
  tme_uint8_t *tme_eth_out;

  /* the next offset within the packet buffer, and the end of the data
     in the packet buffer: */
  size_t tme_eth_buffer_offset;
  size_t tme_eth_buffer_end;

  /* whether we can write to ethernet device */
  int tme_eth_can_write;
  
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
#if 0
int tme_eth_if_find _TME_P((_tme_const char *, 
			    struct ifreq **, 
			    tme_uint8_t **, 
			    unsigned int *));

#endif
#ifdef HAVE_IFADDRS_H
int tme_eth_ifaddrs_find _TME_P((_tme_const char *,
				 int family,
				 struct ifaddrs **, 
				 tme_uint8_t **, 
				 unsigned int *));
#endif

int tme_eth_alloc _TME_P((char *dev_filename,
			  char **_output));

int tme_eth_connections_new _TME_P((struct tme_element *element, 
				    const char * const *args, 
				    struct tme_connection **_conns));

int tme_eth_init _TME_P((struct tme_element *element,
			 tme_thread_handle_t hand,
			 unsigned int sz, 
			 void *data,
			 unsigned char *addr,
			 typeof(tme_eth_connections_new) eth_connections_new));

#endif /* !_HOST_ETH_IMPL_H */
