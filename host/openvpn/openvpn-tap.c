/* host/openvpn/openvpn-tap.c - OpenVPN TUN TAP Ethernet support: */

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
#include "eth-if.h"
#include "openvpn-setup.h"

typedef struct _tme_openvpn_tun {
  struct tme_ethernet *eth;
  struct tuntap *tt;
  struct frame *frame;
  u_char flags;
  tme_event_set_t *event_set;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_tun;

static int _tme_openvpn_tun_write(void *data) {
  tme_openvpn_tun *tun = data;
  
  tun->outbuf.len = tun->eth->tme_eth_data_length;
  if(!(tun->flags & (OPENVPN_CAN_WRITE | OPENVPN_FAST_IO))) {
    /* unlock our mutex: */
    tme_mutex_unlock(&tun->eth->tme_eth_mutex);
      
    /* sleep for the delay sleep time (msec): */
    tme_thread_sleep_yield(0, 500);
      
    /* lock our mutex: */
    tme_mutex_lock(&tun->eth->tme_eth_mutex);
  }

  if(tun->flags & OPENVPN_CAN_WRITE) tun->flags &= ~OPENVPN_CAN_WRITE;
  
#ifdef TUN_PASS_BUFFER
  return write_tun_buffered(tun->tt, &tun->outbuf);
#else
  return write_tun(tun->tt, BPTR(&tun->outbuf), BLEN(&tun->outbuf));
#endif
}

static int _tme_openvpn_tun_read(void *data) {
  int rc;
  unsigned int flags;
  struct timeval tv;
  struct event_set_return esr;
  tme_openvpn_tun *tun = data;

  while(1) {
    tme_event_reset(tun->event_set);
    
    flags = EVENT_READ;
    
    if((tun->flags & OPENVPN_CAN_WRITE) && !(tun->flags & OPENVPN_FAST_IO)) {
      tv.tv_sec = 0;
      tv.tv_usec = 500000;
    } else {
      /* wait for signal transition to write */
      if(!(tun->flags & OPENVPN_FAST_IO)) flags |= EVENT_WRITE;
      tv.tv_sec = BIG_TIMEOUT;
      tv.tv_usec = 0;
    }

#if defined(WIN32)
    tun_show_debug(tun->tt);
#endif

    tun_set(tun->tt, tme_event_set(tun->event_set), flags, (void*)0, NULL);

    tme_event_ctl(tun->event_set, tun_event_handle(tun->tt), flags, 0);

    rc = tme_event_wait_yield(tun->event_set, &tv, &esr, 1);
  
    if(esr.rwflags & EVENT_WRITE)
      tun->flags |= OPENVPN_CAN_WRITE;
    if(esr.rwflags & EVENT_READ) {
#ifdef TUN_PASS_BUFFER
      read_tun_buffered(tun->tt, &tun->inbuf, MAX_RW_SIZE_TUN(tun->frame));
#else
      ASSERT(buf_init(&tun->inbuf, FRAME_HEADROOM(tun->frame)));
      tun->inbuf.len = read_tun(tun->tt, BPTR(&tun->inbuf), MAX_RW_SIZE_TUN(tun->frame));
#endif
      tun->eth->tme_eth_buffer = BPTR(&tun->inbuf);
      return tun->inbuf.len;
    }
  }
}

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_openvpn,tun_tap) {
  int rc;
  unsigned char *hwaddr = NULL;
  int fd = 0;
  void *data = NULL;
  struct tuntap *tt;
  u_char flags;
  tme_event_set_t *event_set;
  struct frame *frame = openvpn_setup(args, &tt, NULL, NULL, &flags, &event_set);
  int sz = BUF_SIZE(frame);

  tme_openvpn_tun *tun = data = tme_new0(tme_openvpn_tun, 1);
  
  tun->tt = tt;
  tun->frame = frame;
  tun->flags = flags | OPENVPN_CAN_WRITE;
  tun->inbuf = alloc_buf(sz);
  tun->outbuf = alloc_buf(sz);
  tun->event_set = event_set;
  
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
  rc = tme_eth_init(element,
		    fd,
		    sz,
		    data,
		    hwaddr,
		    NULL);
  
  if(rc == TME_OK) {
    /* recover our data structure: */
    tun->eth = (struct tme_ethernet *) element->tme_element_private;
    tun->eth->tme_ethernet_write = _tme_openvpn_tun_write;
    tun->eth->tme_ethernet_read = _tme_openvpn_tun_read;
    ASSERT(buf_init(&tun->inbuf, FRAME_HEADROOM(tun->frame)));
    ASSERT(buf_safe(&tun->inbuf, MAX_RW_SIZE_TUN(tun->frame)));
    tun->eth->tme_eth_buffer = BPTR(&tun->inbuf);
    ASSERT(buf_init(&tun->outbuf, FRAME_HEADROOM_ADJ(tun->frame, FRAME_HEADROOM_MARKER_READ_LINK)));
    ASSERT(buf_safe(&tun->outbuf, MAX_RW_SIZE_TUN(tun->frame)));
    tun->eth->tme_eth_out = BPTR(&tun->outbuf);
  }

  return rc;
}
