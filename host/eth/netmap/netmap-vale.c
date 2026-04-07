/* host/eth/netmap/netmap-vale.c - Netmap/VALE Ethernet support: */

/*
 * Copyright (c) 2026 Ruben Agin
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
#include <tme/events.h>

typedef struct _tme_openvpn_tun {
  struct tme_ethernet *eth;
  struct tuntap *tt;
  struct frame *frame;
  u_char flags;
  struct buffer inbuf;
  struct buffer outbuf;
} tme_openvpn_tun;

static int _tme_openvpn_tun_write(void *data) {
  unsigned int flags;
  struct event_set_return esr;
  tme_openvpn_tun *tun = data;
  int status = 1;
  struct tme_event_set *event_set = tme_event_set_init(&status, EVENT_METHOD_FAST);
  
  tun->outbuf.len = tun->eth->tme_eth_data_length;
  tme_event_reset(event_set);
    
  flags = EVENT_WRITE;
    
#if defined(WIN32)
  tun_show_debug(tun->tt);
#endif

  tme_event_ctl(event_set, tun_event_handle(tun->tt), flags, 0);
    
  status = tme_event_wait(event_set, NULL, &esr, status, &tun->eth->tme_eth_mutex);
  tme_event_free(event_set);

  check_status (status, "event_wait", NULL, NULL);
  if(status<=0) return status;

  status = -1;

  if(esr.rwflags & EVENT_WRITE) {
#ifdef TUN_PASS_BUFFER
    status = write_tun_buffered(tun->tt, &tun->outbuf);
#else
    status = write_tun(tun->tt, BPTR(&tun->outbuf), BLEN(&tun->outbuf));
#endif
  }
  check_status (status, "write to TUN/TAP", NULL, tun->tt);
  return status;

}

static int _tme_openvpn_tun_read(void *data) {
  unsigned int flags;
  struct event_set_return esr;
  tme_openvpn_tun *tun = data;
  int status = 1;
  struct tme_event_set *es = tme_event_set_init(&status, EVENT_METHOD_FAST);

  tme_event_reset(es);
    
  flags = EVENT_READ;
    
#if defined(WIN32)
  tun_show_debug(tun->tt);
#endif

  tme_tun_set(tun->tt, es, flags, (void*)0, NULL);
    
  status = tme_event_wait(es, NULL, &esr, status, &tun->eth->tme_eth_mutex);
  tme_event_free(es);

  check_status (status, "event_wait", NULL, NULL);
  if(status<=0) return status;

  tun->inbuf.len = -1;
  
  if(esr.rwflags & EVENT_READ) {
#ifdef TUN_PASS_BUFFER
    read_tun_buffered(tun->tt, &tun->inbuf, MAX_RW_SIZE_TUN(tun->frame));
#else
    ASSERT(buf_init(&tun->inbuf, FRAME_HEADROOM(tun->frame)));
    tun->inbuf.len = read_tun(tun->tt, BPTR(&tun->inbuf), MAX_RW_SIZE_TUN(tun->frame));
#endif
    tun->eth->tme_eth_buffer = BPTR(&tun->inbuf);
  }
  check_status (tun->inbuf.len, "read from TUN/TAP", NULL, tun->tt);
  return tun->inbuf.len;
}

/* the netmap function: */
NME_ELEMENT_SUB_NEW_DECL(host_netmap,vale) {
  int rc;
  struct netmap_if *nifp;
  struct netmap_ring *ring;
  struct nmreq nmr;
  struct ifaddrs *ifa;
  const char *port;
  int usage = 0, arg_i = 1;
  
  nm_fd = tme_eth_alloc("/dev/netmap", _output);
  bzero(&nmr, sizeof(nmr));
  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "port")
	&& args[arg_i + 1] != NULL) {
      port = args[arg_i + 1];
      arg_i += 2;
    }
    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  /* find the interface we will use: */
#ifdef HAVE_IFADDRS_H
  rc = tme_eth_ifaddrs_find(port, AF_UNSPEC, &ifa, NULL, NULL);

  if (rc != TME_OK) {
    tme_output_append_error(_output, _("couldn't find an interface %s"), port);
    return (ENOENT);
  }
  
  strncpy(nmr.nr_name, ifa->ifa_name, sizeof(nmr.nr_name));

  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using interface %s",
	   nmr.nr_name));
#endif
  rc = tme_eth_init(element,
		    TME_INVALID_HANDLE,
		    sz,
		    data,
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
