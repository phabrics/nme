/* host/eth/openvpn/openvpn-tap.c - OpenVPN TUN TAP Ethernet support: */

/*
 * Copyright (c) 2015, 2016 Ruben Agin
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

typedef struct _tme_openvpn_tun {
  struct frame *frame;
  u_char flags;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_tun;

/* conditionally yield this thread if the event is not ready: */
static
int _tme_openvpn_tun_yield(struct tme_ethernet *eth, bool read) {
  int status = 1;
  struct tme_event_set *es;
  struct event_set_return esr;
  unsigned int flags = (read) ? (EVENT_READ) : (EVENT_WRITE);
  struct tuntap *tt = (struct tuntap *)eth->tme_eth_handle;

  es = tme_event_set_init(&status, EVENT_METHOD_FAST);  
  tme_event_reset(es);
  tme_tun_set(tt, es, flags, (void *)0, NULL);
  status = tme_event_wait(es, NULL, &esr, 1, &eth->tme_eth_mutex);
  tme_event_free(es);
  
  return status;
}

static int _tme_openvpn_tun_write(struct tme_ethernet *eth) {
  tme_openvpn_tun *tun = (tme_openvpn_tun *)eth->tme_eth_data;
  struct tuntap *tt = (struct tuntap *)eth->tme_eth_handle;
  int status;
  
  tun->outbuf.len = eth->tme_eth_data_length;
#if defined(WIN32)
  tun_show_debug(tt);
#endif

#ifdef TUN_PASS_BUFFER
  status = write_tun_buffered(tt, &tun->outbuf);
#else
  status = write_tun(tt, BPTR(&tun->outbuf), BLEN(&tun->outbuf));
#endif

  check_status (status, "write to TUN/TAP", NULL, tt);
  return status;

}

static int _tme_openvpn_tun_read(struct tme_ethernet *eth) {
  tme_openvpn_tun *tun = (tme_openvpn_tun *)eth->tme_eth_data;
  struct tuntap *tt = (struct tuntap *)eth->tme_eth_handle;
  int status = 1;
    
#if defined(WIN32)
  tun_show_debug(tt);
#endif

  tun->inbuf.len = -1;
  
#ifdef TUN_PASS_BUFFER
  read_tun_buffered(tt, &tun->inbuf, MAX_RW_SIZE_TUN(tun->frame));
#else
  ASSERT(buf_init(&tun->inbuf, FRAME_HEADROOM(tun->frame)));
  tun->inbuf.len = read_tun(tt, BPTR(&tun->inbuf), MAX_RW_SIZE_TUN(tun->frame));
#endif
  eth->tme_eth_buffer = BPTR(&tun->inbuf);

  check_status (tun->inbuf.len, "read from TUN/TAP", NULL, tt);
  return tun->inbuf.len;
}

/* the new TAP function: */
NME_ELEMENT_SUB_NEW_DECL(host_openvpn,tun_tap) {
  int rc;
  unsigned char *hwaddr = NULL;
  struct tuntap *tt;
  struct env_set *es;
  u_char flags;
  struct frame *frame;
  int sz;
  struct tme_ethernet *eth;
  struct options *options = options_new();
  tme_openvpn_tun *tun = tme_new0(tme_openvpn_tun, 1);
  int arg_i = 0;

  while(args[++arg_i] != NULL);
  
  es = openvpn_setup(args, arg_i, options);
  frame = openvpn_setup_frame(options, &tt, NULL, es, &flags);
  free(options);

  sz = BUF_SIZE(frame);

  tun->frame = frame;
  tun->flags = flags | OPENVPN_CAN_WRITE;
  tun->inbuf = alloc_buf(sz);
  tun->outbuf = alloc_buf(sz);
  
  /* find the interface we will use: */
#ifdef HAVE_IFADDRS_H
  unsigned int hwaddr_len;
  struct ifaddrs *ifa;
  
  rc = tme_eth_ifaddrs_find(tt->actual_name, AF_UNSPEC, &ifa, &hwaddr, &hwaddr_len);
    
  if(hwaddr_len == TME_ETHERNET_ADDR_SIZE) {
    tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	    (&element->tme_element_log_handle, 
	     "hardware address on tap interface %s set to %02x:%02x:%02x:%02x:%02x:%02x",
	     ifa->ifa_name, 
	     hwaddr[0],
	     hwaddr[1],
	     hwaddr[2],
	     hwaddr[3],
	     hwaddr[4],
	     hwaddr[5]));
  }
#endif
  
  eth = tme_new0(struct tme_ethernet, 1);
  eth->tme_eth_yield = _tme_openvpn_tun_yield;
  eth->tme_eth_write = _tme_openvpn_tun_write;
  eth->tme_eth_read = _tme_openvpn_tun_read;
  ASSERT(buf_init(&tun->inbuf, FRAME_HEADROOM(tun->frame)));
  ASSERT(buf_safe(&tun->inbuf, MAX_RW_SIZE_TUN(tun->frame)));
  eth->tme_eth_buffer = BPTR(&tun->inbuf);
  ASSERT(buf_init(&tun->outbuf, FRAME_HEADROOM_ADJ(tun->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
  ASSERT(buf_safe(&tun->outbuf, MAX_RW_SIZE_TUN(tun->frame)));
  eth->tme_eth_out = BPTR(&tun->outbuf);

  return tme_eth_init(element, eth, (tme_uintptr_t)tt, sz, tun);
}
