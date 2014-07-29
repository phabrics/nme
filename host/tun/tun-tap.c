/* $Id: tun-tap.c,v 1.9 2007/02/21 01:24:50 fredette Exp $ */

/* host/tun/tun-tap.c - TUN TAP Ethernet support: */

/*
 * Copyright (c) 2001, 2003 Matt Fredette
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
_TME_RCSID("$Id: tun-tap.c,v 1.9 2007/02/21 01:24:50 fredette Exp $");

/* includes: */
#include <tme/generic/ethernet.h>
#include <tme/threads.h>
#include <tme/misc.h>
#include "eth-impl.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#if defined(HAVE_SYS_SOCKIO_H)
#include <sys/sockio.h>
#elif defined(HAVE_SYS_SOCKETIO_H)
#include <sys/socketio.h> 
#endif /* HAVE_SYS_SOCKETIO_H */
#include <sys/ioctl.h>
#ifdef HAVE_IOCTLS_H
#include <ioctls.h>
#endif /* HAVE_IOCTLS_H */
#ifdef HAVE_NET_IF_ETHER_H
#include <net/if_ether.h>
#endif /* HAVE_NET_IF_ETHER_H */
#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif /* HAVE_NETPACKET_PACKET_H */
#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif /* HAVE_NET_ETHERNET_H */
#include <netinet/ip.h>
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif /* HAVE_NET_IF_DL_H */
#include <arpa/inet.h>
#ifdef HAVE_NET_IF_TAP_H
#include <net/if_tap.h>
#endif
#ifdef HAVE_NET_TAP_IF_TAP_H
#include <net/tap/if_tap.h>
#endif
#ifdef HAVE_NET_IF_TUN_H
#include <net/if_tun.h>
#endif
#ifdef HAVE_LINUX_IF_TUN_H
#include <linux/if_tun.h>
#define TME_TUN_TAP_INSN tme_uint8_t
#define TME_TUN_TAP_PROG struct tun_filter
#define TME_TUN_TAP_INSNS(x) (x)->addr
#define TME_TUN_TAP_LEN(x) (x)->count
#endif


/* macros: */

/* ARP and RARP opcodes: */
#define TME_NET_ARP_OPCODE_REQUEST	(0x0001)
#define TME_NET_ARP_OPCODE_REPLY	(0x0002)
#define TME_NET_ARP_OPCODE_REV_REQUEST	(0x0003)
#define TME_NET_ARP_OPCODE_REV_REPLY	(0x0004)

/* the callout flags: */
#define TME_TUN_TAP_CALLOUT_CHECK	(0)
#define TME_TUN_TAP_CALLOUT_RUNNING	TME_BIT(0)
#define TME_TUN_TAP_CALLOUTS_MASK	(-2)
#define  TME_TUN_TAP_CALLOUT_CTRL	TME_BIT(1)
#define  TME_TUN_TAP_CALLOUT_READ	TME_BIT(2)

/* structures: */

/* our internal data structure: */
struct tme_tun_tap {

  /* backpointer to our element: */
  struct tme_element *tme_tun_tap_element;

  /* our mutex: */
  tme_mutex_t tme_tun_tap_mutex;

  /* our reader condition: */
  tme_cond_t tme_tun_tap_cond_reader;

  /* the callout flags: */
  unsigned int tme_tun_tap_callout_flags;

  /* our Ethernet connection: */
  struct tme_ethernet_connection *tme_tun_tap_eth_connection;

  /* the TAP file descriptor: */
  int tme_tun_tap_fd;

  /* the size of the packet buffer for the interface: */
  size_t tme_tun_tap_buffer_size;

  /* the packet buffer for the interface: */
  tme_uint8_t *tme_tun_tap_buffer;

  /* the next offset within the packet buffer, and the end of the data
     in the packet buffer: */
  size_t tme_tun_tap_buffer_offset;
  size_t tme_tun_tap_buffer_end;

  /* when nonzero, the packet delay time, in microseconds: */
  unsigned long tme_tun_tap_delay_time;

  /* all packets received on or before this time can be released: */
  struct timeval tme_tun_tap_delay_release;

  /* when nonzero, the packet delay sleep time, in microseconds: */
  unsigned long tme_tun_tap_delay_sleep;

  /* when nonzero, the packet delay is sleeping: */
  int tme_tun_tap_delay_sleeping;
};

/* the tap callout function.  it must be called with the mutex locked: */
static void
_tme_tun_tap_callout(struct tme_tun_tap *tap, int new_callouts)
{
  struct tme_ethernet_connection *conn_eth;
  int callouts, later_callouts;
  unsigned int ctrl;
  int rc;
  int status;
  tme_ethernet_fid_t frame_id;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  tme_uint8_t frame[TME_ETHERNET_FRAME_MAX];
  
  /* add in any new callouts: */
  tap->tme_tun_tap_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (tap->tme_tun_tap_callout_flags & TME_TUN_TAP_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  tap->tme_tun_tap_callout_flags |= TME_TUN_TAP_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = tap->tme_tun_tap_callout_flags) & TME_TUN_TAP_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    tap->tme_tun_tap_callout_flags = callouts & ~TME_TUN_TAP_CALLOUTS_MASK;
    callouts &= TME_TUN_TAP_CALLOUTS_MASK;

    /* get our Ethernet connection: */
    conn_eth = tap->tme_tun_tap_eth_connection;

    /* if we need to call out new control information: */
    if (callouts & TME_TUN_TAP_CALLOUT_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (tap->tme_tun_tap_buffer_offset
	  < tap->tme_tun_tap_buffer_end) {
	ctrl |= TME_ETHERNET_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&tap->tme_tun_tap_mutex);
      
      /* do the callout: */
      rc = (conn_eth != NULL
	    ? ((*conn_eth->tme_ethernet_connection_ctrl)
	       (conn_eth,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&tap->tme_tun_tap_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_TUN_TAP_CALLOUT_CTRL;
      }
    }
      
    /* if the Ethernet is readable: */
    if (callouts & TME_TUN_TAP_CALLOUT_READ) {

      /* unlock the mutex: */
      tme_mutex_unlock(&tap->tme_tun_tap_mutex);
      
      /* make a frame chunk to receive this frame: */
      frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes = frame;
      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
	= sizeof(frame);

      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_read)
	       (conn_eth,
		&frame_id,
		&frame_chunk_buffer,
		TME_ETHERNET_READ_NEXT)));
      
      /* lock the mutex: */
      tme_mutex_lock(&tap->tme_tun_tap_mutex);
      
      /* if the read was successful: */
      if (rc > 0) {

	/* check the size of the frame: */
	assert(rc <= sizeof(frame));

	/* do the write: */
	status = tme_thread_write(tap->tme_tun_tap_fd, frame, rc);

	/* writes must succeed: */
	assert (status == rc);

	/* mark that we need to loop to callout to read more frames: */
	tap->tme_tun_tap_callout_flags |= TME_TUN_TAP_CALLOUT_READ;
      }

      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_READ flag: */
    }

  }
  
  /* put in any later callouts, and clear that callouts are running: */
  tap->tme_tun_tap_callout_flags = later_callouts;
}

/* the TAP reader thread: */
static void
_tme_tun_tap_th_reader(struct tme_tun_tap *tap)
{
  ssize_t buffer_end;
  unsigned long sleep_usec;
  
  /* lock the mutex: */
  tme_mutex_lock(&tap->tme_tun_tap_mutex);

  /* loop forever: */
  for (;;) {

    /* if the delay sleeping flag is set: */
    if (tap->tme_tun_tap_delay_sleeping) {

      /* clear the delay sleeping flag: */
      tap->tme_tun_tap_delay_sleeping = FALSE;
      
      /* call out that we can be read again: */
      _tme_tun_tap_callout(tap, TME_TUN_TAP_CALLOUT_CTRL);
    }

    /* if a delay has been requested: */
    sleep_usec = tap->tme_tun_tap_delay_sleep;
    if (sleep_usec > 0) {

      /* clear the delay sleep time: */
      tap->tme_tun_tap_delay_sleep = 0;

      /* set the delay sleeping flag: */
      tap->tme_tun_tap_delay_sleeping = TRUE;

      /* unlock our mutex: */
      tme_mutex_unlock(&tap->tme_tun_tap_mutex);
      
      /* sleep for the delay sleep time: */
      tme_thread_sleep_yield(0, sleep_usec);
      
      /* lock our mutex: */
      tme_mutex_lock(&tap->tme_tun_tap_mutex);
      
      continue;
    }

    /* if the buffer is not empty, wait until either it is,
       or we're asked to do a delay: */
    if (tap->tme_tun_tap_buffer_offset
	< tap->tme_tun_tap_buffer_end) {
      tme_cond_wait_yield(&tap->tme_tun_tap_cond_reader,
			  &tap->tme_tun_tap_mutex);
      continue;
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&tap->tme_tun_tap_mutex);

    /* read the TAP socket: */
    tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, TME_OK,
	    (&tap->tme_tun_tap_element->tme_element_log_handle,
	     _("calling read")));
    buffer_end = 
      tme_thread_read_yield(tap->tme_tun_tap_fd,
			    tap->tme_tun_tap_buffer,
			    tap->tme_tun_tap_buffer_size);

    /* lock the mutex: */
    tme_mutex_lock(&tap->tme_tun_tap_mutex);

    /* if the read failed: */
    if (buffer_end <= 0) {
      tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, errno,
	      (&tap->tme_tun_tap_element->tme_element_log_handle,
	       _("failed to read packets")));
      continue;
    }

    /* the read succeeded: */
    tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, TME_OK,
	    (&tap->tme_tun_tap_element->tme_element_log_handle,
	     _("read %ld bytes of packets"), (long) buffer_end));
    tap->tme_tun_tap_buffer_offset = 0;
    tap->tme_tun_tap_buffer_end = buffer_end;

    /* call out that we can be read again: */
    _tme_tun_tap_callout(tap, TME_TUN_TAP_CALLOUT_CTRL);
  }
  /* NOTREACHED */
}

/* this is called when the ethernet configuration changes: */
static int
_tme_tun_tap_config(struct tme_ethernet_connection *conn_eth, 
		    struct tme_ethernet_config *config)
{
  struct tme_tun_tap *tap;
#ifdef HAVE_LINUX_IF_TUN_H
  TME_TUN_TAP_INSN *tap_filter;
#endif
  int tap_filter_size;
  int rc;

  /* recover our data structures: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume we will succeed: */
  rc = TME_OK;

#ifdef HAVE_LINUX_IF_TUN_H
  /* lock the mutex: */
  tme_mutex_lock(&tap->tme_tun_tap_mutex);

  /* allocate space for the worst-case filter: one insn for the packet
     accept, one insn for the packet reject, and TME_ETHERNET_ADDR_SIZE
     * 2 insns for each address - one insn to load an address byte and
     one insn to test it and branch: */
  tap_filter_size = TME_ETHERNET_ADDR_SIZE * config->tme_ethernet_config_addr_count;
  tap_filter = tme_new0(TME_TUN_TAP_INSN, tap_filter_size + sizeof(TME_TUN_TAP_PROG));

  /* if this Ethernet is promiscuous, we will accept all packets: */
  if (config->tme_ethernet_config_flags & TME_ETHERNET_CONFIG_PROMISC) {
    tap_filter_size = 0;
  }

  /* if this Ethernet does have a set of addresses, we will accept all
     packets for one of those addresses: */
  else if (config->tme_ethernet_config_addr_count > 0) {
    memcpy(tap_filter + sizeof(TME_TUN_TAP_PROG), config->tme_ethernet_config_addrs, tap_filter_size);
  }

  /* otherwise this filter doesn't need to accept any packets: */
  else {
    // how do we reject all packets???
  }

  /* set the filter on the TAP device: */
  TME_TUN_TAP_LEN((TME_TUN_TAP_PROG *)tap_filter) = tap_filter_size;
  if (ioctl(tap->tme_tun_tap_fd, TUNSETTXFILTER, tap_filter) < 0) {
    tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 0, errno,
	    (&tap->tme_tun_tap_element->tme_element_log_handle,
	     _("failed to set the filter")));
    rc = errno;
  }

  tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 0, TME_OK,
	  (&tap->tme_tun_tap_element->tme_element_log_handle,
	   _("set the filter")));

  /* free the filter: */
  tme_free(tap_filter);

  /* unlock the mutex: */
  tme_mutex_unlock(&tap->tme_tun_tap_mutex);

#endif
  /* done: */
  return (rc);
}

/* this is called when control lines change: */
static int
_tme_tun_tap_ctrl(struct tme_ethernet_connection *conn_eth, 
		  unsigned int ctrl)
{
  struct tme_tun_tap *tap;
  int new_callouts;

  /* recover our data structures: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&tap->tme_tun_tap_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_ETHERNET_CTRL_OK_READ) {
    new_callouts |= TME_TUN_TAP_CALLOUT_READ;
  }

  /* make any new callouts: */
  _tme_tun_tap_callout(tap, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&tap->tme_tun_tap_mutex);

  return (TME_OK);
}

/* this is called to read a frame: */
static int
_tme_tun_tap_read(struct tme_ethernet_connection *conn_eth, 
		  tme_ethernet_fid_t *_frame_id,
		  struct tme_ethernet_frame_chunk *frame_chunks,
		  unsigned int flags)
{
  struct tme_tun_tap *tap;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  size_t buffer_offset_next;
  const struct tme_ethernet_header *ethernet_header;
  const struct tme_net_arp_header *arp_header;
  const struct tme_net_ipv4_header *ipv4_header;
  tme_uint16_t ethertype;
  unsigned int count;
  int rc;

  /* recover our data structure: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&tap->tme_tun_tap_mutex);

  /* assume that we won't be able to return a packet: */
  rc = -ENOENT;

  /* loop until we have a good captured packet or until we 
     exhaust the buffer: */
  for (;;) {
    buffer_offset_next = tap->tme_tun_tap_buffer_end;
    /* if there's not enough for a TAP header, flush the buffer: */
    if (tap->tme_tun_tap_buffer_offset >= tap->tme_tun_tap_buffer_end)
    {
      if (tap->tme_tun_tap_buffer_offset
	  != tap->tme_tun_tap_buffer_end) {
	tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, TME_OK,
		(&tap->tme_tun_tap_element->tme_element_log_handle,
		 _("flushed garbage TAP header bytes")));
	tap->tme_tun_tap_buffer_offset = tap->tme_tun_tap_buffer_end;
      }
      break;
    }

    /* form the single frame chunk: */
    frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
      = tap->tme_tun_tap_buffer + tap->tme_tun_tap_buffer_offset;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
      = buffer_offset_next;

    /* some network interfaces haven't removed the CRC yet when they
       pass a packet to TAP.  packets in a tme ethernet connection
       never have CRCs, so here we attempt to detect them and strip
       them off.

       unfortunately there's no general way to do this.  there's a
       chance that the last four bytes of an actual packet just
       happen to be the Ethernet CRC of all of the previous bytes in
       the packet, so we can't just strip off what looks like a
       valid CRC, plus the CRC calculation itself isn't cheap.

       the only way to do this well seems to be to look at the
       protocol.  if we can determine what the correct minimum size
       of the packet should be based on the protocol, and the size
       we got is four bytes more than that, assume that the last four
       bytes are a CRC and strip it off: */

    /* assume that we won't be able to figure out the correct minimum
       size of the packet: */
    count = 0;

    /* get the Ethernet header and packet type: */
    ethernet_header = (struct tme_ethernet_header *) (tap->tme_tun_tap_buffer + tap->tme_tun_tap_buffer_offset);
    ethertype = ethernet_header->tme_ethernet_header_type[0];
    ethertype = (ethertype << 8) + ethernet_header->tme_ethernet_header_type[1];

    /* dispatch on the packet type: */
    switch (ethertype) {

      /* an ARP or RARP packet: */
    case TME_ETHERNET_TYPE_ARP:
    case TME_ETHERNET_TYPE_RARP:
      arp_header = (struct tme_net_arp_header *) (ethernet_header + 1);
      switch ((((tme_uint16_t) arp_header->tme_net_arp_header_opcode[0]) << 8)
	      + arp_header->tme_net_arp_header_opcode[1]) {
      case TME_NET_ARP_OPCODE_REQUEST:
      case TME_NET_ARP_OPCODE_REPLY:
      case TME_NET_ARP_OPCODE_REV_REQUEST:
      case TME_NET_ARP_OPCODE_REV_REPLY:
	count = (TME_ETHERNET_HEADER_SIZE
		 + sizeof(struct tme_net_arp_header)
		 + (2 * arp_header->tme_net_arp_header_hardware_length)
		 + (2 * arp_header->tme_net_arp_header_protocol_length));
      default:
	break;
      }
      break;

      /* an IPv4 packet: */
    case TME_ETHERNET_TYPE_IPV4:
      ipv4_header = (struct tme_net_ipv4_header *) (ethernet_header + 1);
      count = ipv4_header->tme_net_ipv4_header_length[0];
      count = (count << 8) + ipv4_header->tme_net_ipv4_header_length[1];
      count += TME_ETHERNET_HEADER_SIZE;
      break;

    default:
      break;
    }

    /* if we were able to figure out the correct minimum size of the
       packet, and the packet from TAP is exactly that minimum size
       plus the CRC size, set the length of the packet to be the
       correct minimum size.  NB that we can't let the packet become
       smaller than (TME_ETHERNET_FRAME_MIN - TME_ETHERNET_CRC_SIZE): */
    if (count != 0) {
      count = TME_MAX(count,
		      (TME_ETHERNET_FRAME_MIN
		       - TME_ETHERNET_CRC_SIZE));
      if (frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
	  == (count + TME_ETHERNET_CRC_SIZE)) {
	frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count = count;
      }
    }

    /* copy out the frame: */
    count = tme_ethernet_chunks_copy(frame_chunks, &frame_chunk_buffer);

    /* if this is a peek: */
    if (flags & TME_ETHERNET_READ_PEEK) {
    }

    /* otherwise, this isn't a peek: */
    else {

      /* update the buffer pointer: */
      tap->tme_tun_tap_buffer_offset = buffer_offset_next;
    }

    /* success: */
    rc = count;
    break;
  }

  /* if the buffer is empty, or if we failed to read a packet,
     wake up the reader: */
  if ((tap->tme_tun_tap_buffer_offset
       >= tap->tme_tun_tap_buffer_end)
      || rc <= 0) {
    tme_cond_notify(&tap->tme_tun_tap_cond_reader, TRUE);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&tap->tme_tun_tap_mutex);

  /* done: */
  return (rc);
}

/* this makes a new Ethernet connection: */
static int
_tme_tun_tap_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_tun_tap *tap;
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection *conn_eth_other;

  /* recover our data structures: */
  tap = conn->tme_connection_element->tme_element_private;
  conn_eth = (struct tme_ethernet_connection *) conn;
  conn_eth_other = (struct tme_ethernet_connection *) conn->tme_connection_other;

  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_ETHERNET);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_ETHERNET);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&tap->tme_tun_tap_mutex);

    /* save our connection: */
    tap->tme_tun_tap_eth_connection = conn_eth_other;

    /* unlock our mutex: */
    tme_mutex_unlock(&tap->tme_tun_tap_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_tun_tap_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a TAP: */
static int
_tme_tun_tap_connections_new(struct tme_element *element, 
			     const char * const *args, 
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_tun_tap *tap;
  struct tme_ethernet_connection *conn_eth;
  struct tme_connection *conn;

  /* recover our data structure: */
  tap = (struct tme_tun_tap *) element->tme_element_private;

  /* if we already have an Ethernet connection, do nothing: */
  if (tap->tme_tun_tap_eth_connection != NULL) {
    return (TME_OK);
  }

  /* allocate the new Ethernet connection: */
  conn_eth = tme_new0(struct tme_ethernet_connection, 1);
  conn = &conn_eth->tme_ethernet_connection;
  
  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_ETHERNET;
  conn->tme_connection_score = tme_ethernet_connection_score;
  conn->tme_connection_make = _tme_tun_tap_connection_make;
  conn->tme_connection_break = _tme_tun_tap_connection_break;

  /* fill in the Ethernet connection: */
  conn_eth->tme_ethernet_connection_config = _tme_tun_tap_config;
  conn_eth->tme_ethernet_connection_ctrl = _tme_tun_tap_ctrl;
  conn_eth->tme_ethernet_connection_read = _tme_tun_tap_read;

  /* return the connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* the new TAP function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_tun,tap) {
  struct tme_tun_tap *tap;
  int tap_fd;
  char dev_tap_filename[sizeof(DEV_TAP_FILENAME) + 4];
  int saved_errno;
  u_int tap_opt;
  struct ifreq ifr;
  unsigned long delay_time;
  int arg_i;
  int usage;
  int rc;

  memset(&ifr, 0, sizeof(ifr));

  /* get the arguments: */
  rc = tme_eth_args(args, &ifr, &delay_time, _output);

  sprintf(dev_tap_filename, DEV_TAP_FILENAME);
#ifndef HAVE_LINUX_IF_TUN_H
  if(ifr.ifr_name[0] != '\0')
    tap_fd = tme_eth_alloc(element, ifr.ifr_name, 0);
  else
#endif
    tap_fd = tme_eth_alloc(element, dev_tap_filename, dev_tap_filename + sizeof(DEV_TAP_FILENAME));

  if (tap_fd < 0) {
    saved_errno = errno;
    tme_log(&element->tme_element_log_handle, 1, saved_errno,
	    (&element->tme_element_log_handle,
	     _("failed to open TAP device %s"),
	     dev_tap_filename));
    return (saved_errno);
  }

  /* this macro helps in closing the TAP socket on error: */
#define _TME_TAP_RAW_OPEN_ERROR(x) saved_errno = errno; x; errno = saved_errno

#ifdef HAVE_LINUX_IF_TUN_H
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
  
  /* try to create the device */
  if (ioctl(tap_fd, TUNSETIFF, (void *) &ifr) < 0 )
#elif defined(HAVE_NET_IF_TAP_H)
  if (ioctl(tap_fd, TAPGIFNAME, (void *) &ifr) < 0 )
#endif
#if defined(HAVE_LINUX_IF_TUN_H) || defined(HAVE_NET_IF_TAP_H)
  {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set the TAP interface on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
#endif
  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using tap interface %s",
	   ifr.ifr_name));

  return tme_eth_init(element, tap_fd, 4096, delay_time, _tme_tun_tap_connections_new);

#undef _TME_TAP_RAW_OPEN_ERROR
}
