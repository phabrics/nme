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
#include <tme/eth-if.h>
#include <net/netmap_user.h>
#include <sys/mman.h>

static int _tme_netmap_write(struct tme_ethernet *eth) {
  struct netmap_if *nifp = (struct netmap_if *)eth->tme_eth_data;
  struct netmap_ring *ring = NETMAP_TXRING(nifp, 0);
  int i = ring->cur;
  
  ring->slot[i].len = eth->tme_eth_data_length;
  ring->head = ring->cur = i = nm_ring_next(ring, i);
  eth->tme_eth_out = (tme_uint8_t *)NETMAP_BUF(ring, ring->slot[i].buf_idx);

  return eth->tme_eth_data_length;
}

static int _tme_netmap_read(struct tme_ethernet *eth) {
  struct netmap_if *nifp = (struct netmap_if *)eth->tme_eth_data;
  struct netmap_ring *ring = NETMAP_RXRING(nifp, 0);
  int i = ring->cur;

  eth->tme_eth_buffer = (tme_uint8_t *)NETMAP_BUF(ring, ring->slot[i].buf_idx);
  ring->cur = nm_ring_next(ring, i);
  ring->head = i;
  return ring->slot[i].len;
}

/* the netmap function: */
NME_ELEMENT_SUB_NEW_DECL(host_netmap,vale) {
  int rc, i;
  int saved_errno;
  struct netmap_if *nifp;
  struct netmap_ring *ring;
  struct nmreq nmr;
  struct ifaddrs *ifa;
  const char *port = NULL;
  char *p;
  struct tme_ethernet *eth;
  int nm_fd, usage = 0, arg_i = 1;
#define DEV_NM_FORMAT "/dev/netmap"
  char dev_nm_filename[sizeof(DEV_NM_FORMAT) + 4];

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "port")
	&& args[arg_i + 1] != NULL) {
      port = args[arg_i + 1];
      arg_i += 2;
    }
    /* if we ran out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
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

  sprintf(dev_nm_filename, DEV_NM_FORMAT);
  
  nm_fd = tme_eth_alloc(dev_nm_filename, _output);

  if(nm_fd < 0) {
    saved_errno = errno;
    tme_log(&element->tme_element_log_handle, 1, saved_errno,
	    (&element->tme_element_log_handle,
	     _("failed to open netmap device %s"),
	     dev_nm_filename));
    return (saved_errno);
  }
  
  bzero(&nmr, sizeof(nmr));

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

  nmr.nr_version = NETMAP_API;
  ioctl(nm_fd, NIOCREGIF, &nmr);
  p = mmap(0, nmr.nr_memsize, PROT_READ | PROT_WRITE, MAP_SHARED, nm_fd, 0);
  nifp = NETMAP_IF(p, nmr.nr_offset);

  eth = tme_new0(struct tme_ethernet, 1);
  eth->tme_eth_write = _tme_netmap_write;
  eth->tme_eth_read = _tme_netmap_read;

  ring = NETMAP_TXRING(nifp, 0);
  eth->tme_eth_out = (tme_uint8_t *)NETMAP_BUF(ring, ring->slot[ring->cur].buf_idx);

  ring = NETMAP_RXRING(nifp, 0);
  i = ring->cur;
  eth->tme_eth_buffer = (tme_uint8_t *)NETMAP_BUF(ring, ring->slot[i].buf_idx);
  if (!nm_ring_empty(ring)) {
    ring->cur = nm_ring_next(ring, i);
    ring->head = i;
    eth->tme_eth_buffer_end = ring->slot[i].len;
    tme_log(&eth->tme_eth_element->tme_element_log_handle, 1, TME_OK,
	    (&eth->tme_eth_element->tme_element_log_handle,
	     _("read %ld bytes of first packet"), (long)eth->tme_eth_buffer_end));
  }
  
  return tme_eth_init(element, eth, nm_fd, ring->nr_buf_size, nifp);
}
