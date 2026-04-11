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
#include <tme/eth-if.h>
#include <tme/events.h>

typedef struct _tme_openvpn_link {
  struct env_set *es;
  struct frame *frame;
  u_char flags;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_link;

/* conditionally yield this thread if the event is not ready: */
static int _tme_openvpn_link_yield(struct tme_ethernet *eth, bool read) {
  int status = 1;
  struct tme_event_set *es;
  struct event_set_return esr;
  unsigned int flags = (read) ? (EVENT_READ) : (EVENT_WRITE);
  struct link_socket *ls = (struct link_socket *)eth->tme_eth_handle;
  
  if(socket_read_residual(ls)) return 0;
  
  es = tme_event_set_init(&status, EVENT_METHOD_FAST);  
  tme_event_reset(es);
  tme_socket_set(ls, es, flags, (void *)0, NULL);
  status = tme_event_wait(es, NULL, &esr, 1, &eth->tme_eth_mutex);
  tme_event_free(es);
  
  return status;
}

static int _tme_openvpn_link_write(struct tme_ethernet *eth) {
  tme_openvpn_link *link = (tme_openvpn_link *)eth->tme_eth_data;
  struct link_socket *ls = (struct link_socket *)eth->tme_eth_handle;
  struct link_socket_actual *to_addr;	/* IP address of remote */
  int status = -1;
  
  ASSERT(buf_init(&link->outbuf, FRAME_HEADROOM(link->frame)));

  link->outbuf.len = eth->tme_eth_data_length;
  /*
   * Get the address we will be sending the packet to.
   */
  link_socket_get_outgoing_addr(&link->outbuf, &ls->info,
				&to_addr);

  status = link_socket_write(ls, &link->outbuf, to_addr);

  return status;
}

static int _tme_openvpn_link_read(struct tme_ethernet *eth) {
  tme_openvpn_link *link = (tme_openvpn_link *)eth->tme_eth_data;
  struct link_socket *ls = (struct link_socket *)eth->tme_eth_handle;
  struct link_socket_actual from;               /* address of incoming datagram */
  int status = 1;
  
  link->inbuf.len = -1;

  ASSERT(buf_init(&link->inbuf, FRAME_HEADROOM_ADJ(link->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
  status = link_socket_read(ls,
			    &link->inbuf,
			    MAX_RW_SIZE_LINK(link->frame),
			    &from);
  if(socket_connection_reset(ls, status)) {
    //	register_signal (c, SIGUSR1, "connection-reset"); /* SOFT-SIGUSR1 -- TCP connection reset */
    msg (D_STREAM_ERRORS, "Connection reset, restarting [%d]", status);
    return status;
  }
    
  /* check recvfrom status */
  check_status (status, "read", ls, NULL);
  if(link->inbuf.len > 0) {
    if(!link_socket_verify_incoming_addr(&link->inbuf, &ls->info, &from))
      link_socket_bad_incoming_addr(&link->inbuf, &ls->info, &from);
    link_socket_set_outgoing_addr(&link->inbuf, &ls->info, &from, NULL, link->es);
    /* Did we just receive an openvpn ping packet? */
    if (is_ping_msg (&link->inbuf)) {
      dmsg (D_PING, "RECEIVED PING PACKET");
      link->inbuf.len = 0; /* drop packet */
    }
  }
  eth->tme_eth_buffer = BPTR(&link->inbuf);

  return link->inbuf.len;
}

/* the new TAP function: */
NME_ELEMENT_SUB_NEW_DECL(host_openvpn,socket_link) {
  int rc;
  struct link_socket *ls;
  struct env_set *es;
  u_char flags = 0;
  struct frame *frame;
  int sz;
  struct tme_ethernet *eth;
  struct options *options = options_new();
  tme_openvpn_link *link = tme_new0(tme_openvpn_link, 1);
  int arg_i = 0;

  while(args[++arg_i] != NULL);

  es = openvpn_setup(args, arg_i, options);
  frame = openvpn_setup_frame(options, NULL, &ls, es, &flags);
  free(options);

  sz = BUF_SIZE(frame);
  
  link->es = es;
  link->frame = frame;
  link->flags = flags | OPENVPN_CAN_WRITE;
  link->inbuf = alloc_buf(sz);
  link->outbuf = alloc_buf(sz);

  eth = tme_new0(struct tme_ethernet, 1);
  eth->tme_eth_yield = _tme_openvpn_link_yield;
  eth->tme_eth_write = _tme_openvpn_link_write;
  eth->tme_eth_read = _tme_openvpn_link_read;
  ASSERT(buf_init(&link->inbuf, FRAME_HEADROOM_ADJ(link->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
  ASSERT(buf_safe(&link->inbuf, MAX_RW_SIZE_LINK(link->frame)));
  eth->tme_eth_buffer = BPTR(&link->inbuf);
  ASSERT(buf_init(&link->outbuf, FRAME_HEADROOM(link->frame)));
  ASSERT(buf_safe(&link->outbuf, MAX_RW_SIZE_LINK(link->frame)));
  eth->tme_eth_out = BPTR(&link->outbuf);
  
  return tme_eth_init(element, eth, (tme_uintptr_t)ls, sz, link);
}
