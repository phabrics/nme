/* host/openvpn/openvpn-link.c - OpenVPN socket p2p support: */

/*
 * Copyright (c) 2016 Ruben Agin
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

#include <tme/common.h>

/* includes: */
#include "eth-if.h"
#include "openvpn-setup.h"

#ifndef TME_THREADS_SJLJ
typedef struct _tme_openvpn_sock {
  struct tme_ethernet *eth;
  struct link_socket *ls;
  struct env_set *es;
  struct frame *frame;
  struct event_set *event_set;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_sock;

static int _tme_openvpn_sock_write(void *data) {
  tme_openvpn_sock *sock = data;
  struct link_socket_actual *to_addr;	/* IP address of remote */
  
  sock->outbuf.len = sock->eth->tme_eth_data_length;
  /*
   * Get the address we will be sending the packet to.
   */
  link_socket_get_outgoing_addr(&sock->outbuf, &sock->ls->info,
				&to_addr);
  
  return link_socket_write(sock->ls, &sock->outbuf, to_addr);
}

static int _tme_openvpn_sock_read(void *data) {
  int status, rc, can_write, i;
  unsigned int flags;
  struct timeval tv;
  tv.tv_sec = BIG_TIMEOUT;
  tv.tv_usec = 0;
  struct event_set_return esr[4];
  tme_openvpn_sock *sock = data;
  struct link_socket_actual from;               /* address of incoming datagram */

  event_reset(sock->event_set);
    
  flags = EVENT_READ;
    
  can_write = sock->eth->tme_eth_can_write;
  if(!can_write) {
    /* wait for signal transition to write */
    flags |= EVENT_WRITE;
  }

  socket_set(sock->ls, sock->event_set, flags, (void*)0, NULL);
  rc = event_wait(sock->event_set, &tv, esr, SIZE(esr));

  for (i = 0; i < rc; ++i) {
    if(esr[i].rwflags & EVENT_READ) {
      ASSERT(buf_init(&sock->inbuf, 0));
      status = link_socket_read(sock->ls,
				&sock->inbuf,
				MAX_RW_SIZE_LINK(sock->frame),
				&from);
      //      if (socket_connection_reset (link_socket, status))
      /* check recvfrom status */
      check_status (status, "read", sock->ls, NULL);
      if(sock->inbuf.len > 0) {
	if(!link_socket_verify_incoming_addr(&sock->inbuf, &sock->ls->info, &from))
	  link_socket_bad_incoming_addr(&sock->inbuf, &sock->ls->info, &from);
	link_socket_set_outgoing_addr(&sock->inbuf, &sock->ls->info, &from, NULL, sock->es);
	/* Did we just receive an openvpn ping packet? */
	if (is_ping_msg (&sock->inbuf)) {
	  dmsg (D_PING, "RECEIVED PING PACKET");
	  sock->inbuf.len = 0; /* drop packet */
	}
      }
    }
    if(esr[i].rwflags & EVENT_WRITE)
      can_write = TRUE;
  }
  return sock->inbuf.len;
}
#endif // !TME_THREADS_SJLJ

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,socket_link) {
  int rc;
  int fd = 0;
  void *data = NULL;
  struct link_socket *ls;
  struct env_set *es;
  struct frame *frame = openvpn_setup(args, NULL, &ls, &es);
  int sz = BUF_SIZE(frame);
  
#ifdef TME_THREADS_SJLJ
  fd = ls->fd;
#else
  int event_set_max = 4;
  unsigned int flags = EVENT_METHOD_FAST;
  tme_openvpn_sock *sock = data = tme_new0(tme_openvpn_sock, 1);
  
  sock->ls = ls;
  sock->es = es;
  sock->frame = frame;
  sock->inbuf = alloc_buf(sz);
  sock->outbuf = alloc_buf(sz);
  sock->event_set = event_set_init(&event_set_max, flags);
#endif
  rc = tme_eth_init(element,
		    fd,
		    sz,
		    data,
		    NULL,
		    NULL);
  
#ifndef TME_THREADS_SJLJ
  if(rc == TME_OK) {
    /* recover our data structure: */
    sock->eth = (struct tme_ethernet *) element->tme_element_private;
    sock->eth->tme_ethernet_write = _tme_openvpn_sock_write;
    sock->eth->tme_ethernet_read = _tme_openvpn_sock_read;
    sock->eth->tme_eth_buffer = BPTR(&sock->inbuf);
    sock->eth->tme_eth_out = BPTR(&sock->outbuf);
  }
#endif
  return rc;
}
