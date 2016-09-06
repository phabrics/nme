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
#include <tme/libopenvpn/openvpn-setup.h>

typedef struct _tme_openvpn_sock {
  struct tme_ethernet *eth;
  struct link_socket *ls;
  struct env_set *es;
  struct frame *frame;
  u_char flags;
  tme_event_set_t *event_set;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_sock;

static int _tme_openvpn_sock_write(void *data) {
  int status;
  unsigned int flags;
  struct event_set *es;
  struct event_set_return esr;
  tme_openvpn_sock *sock = data;
  struct link_socket_actual *to_addr;	/* IP address of remote */
  
  ASSERT(buf_init(&sock->outbuf, FRAME_HEADROOM(sock->frame)));

  sock->outbuf.len = sock->eth->tme_eth_data_length;
  /*
   * Get the address we will be sending the packet to.
   */
  link_socket_get_outgoing_addr(&sock->outbuf, &sock->ls->info,
				&to_addr);

  tme_event_reset(sock->event_set);
    
  flags = EVENT_WRITE;
    
  tme_event_ctl(sock->event_set, socket_event_handle(sock->ls), flags, 0);
    
  status = tme_event_wait_yield(sock->event_set, NULL, &esr, 1, &sock->eth->tme_eth_mutex);
  if(status<0) return status;

  status = -1;

  if(esr.rwflags & EVENT_WRITE) {
    status = link_socket_write(sock->ls, &sock->outbuf, to_addr);
  }
  return status;
}

static int _tme_openvpn_sock_read(void *data) {
  int status;
  unsigned int flags;
  struct event_set *es;
  struct event_set_return esr;
  tme_openvpn_sock *sock = data;
  struct link_socket_actual from;               /* address of incoming datagram */
  
  tme_event_reset(sock->event_set);
  
  /*
   * On win32 we use the keyboard or an event object as a source
   * of asynchronous signals.
   */
  //    wait_signal(sock->event_set, (void*)&err_shift);
  
  flags = EVENT_READ;
  
  es = tme_event_set(sock->event_set);

  socket_set(sock->ls, es, flags, (void*)0, NULL);

  if(es != sock->event_set) {
    tme_event_set(sock->event_set) = NULL;
    tme_event_ctl(sock->event_set, socket_event_handle(sock->ls), flags, 0);
    tme_event_set(sock->event_set) = es;
  }

  if(socket_read_residual(sock->ls))
    esr.rwflags = EVENT_READ;
  else {
    status = tme_event_wait_yield(sock->event_set, NULL, &esr, 1, &sock->eth->tme_eth_mutex);

    if(status<0) return status;
  }
    
  sock->inbuf.len = -1;

  if(esr.rwflags & EVENT_READ) {
    ASSERT(buf_init(&sock->inbuf, FRAME_HEADROOM_ADJ(sock->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
    status = link_socket_read(sock->ls,
			      &sock->inbuf,
			      MAX_RW_SIZE_LINK(sock->frame),
			      &from);
    if(socket_connection_reset(sock->ls, status)) {
      //	register_signal (c, SIGUSR1, "connection-reset"); /* SOFT-SIGUSR1 -- TCP connection reset */
      msg (D_STREAM_ERRORS, "Connection reset, restarting [%d]", status);
      return status;
    }
    
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
    sock->eth->tme_eth_buffer = BPTR(&sock->inbuf);
  }
  return sock->inbuf.len;
}

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,socket_link) {
  int rc;
  void *data = NULL;
  struct link_socket *ls;
  struct env_set *es;
  u_char flags = 0;
  tme_event_set_t *event_set;
  struct frame *frame;
  int sz;
  struct options options;
  tme_openvpn_sock *sock = data = tme_new0(tme_openvpn_sock, 1);
  int arg_i = 0;

  while(args[++arg_i] != NULL);

  es = openvpn_setup(args, arg_i, &options);
  frame = openvpn_setup_frame(&options, NULL, &ls, es, &flags, &event_set);
  sz = BUF_SIZE(frame);
  
  sock->ls = ls;
  sock->es = es;
  sock->frame = frame;
  sock->flags = flags | OPENVPN_CAN_WRITE;
  sock->inbuf = alloc_buf(sz);
  sock->outbuf = alloc_buf(sz);
  sock->event_set = event_set;

  rc = tme_eth_init(element,
		    TME_INVALID_HANDLE,
		    sz,
		    data,
		    NULL,
		    NULL);
  
  if(rc == TME_OK) {
    /* recover our data structure: */
    sock->eth = (struct tme_ethernet *) element->tme_element_private;
    sock->eth->tme_ethernet_write = _tme_openvpn_sock_write;
    sock->eth->tme_ethernet_read = _tme_openvpn_sock_read;
    ASSERT(buf_init(&sock->inbuf, FRAME_HEADROOM_ADJ(sock->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
    ASSERT(buf_safe(&sock->inbuf, MAX_RW_SIZE_LINK(sock->frame)));
    sock->eth->tme_eth_buffer = BPTR(&sock->inbuf);
    ASSERT(buf_init(&sock->outbuf, FRAME_HEADROOM(sock->frame)));
    ASSERT(buf_safe(&sock->outbuf, MAX_RW_SIZE_LINK(sock->frame)));
    sock->eth->tme_eth_out = BPTR(&sock->outbuf);
  }
  return rc;
}
