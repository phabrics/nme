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
#include "tun-impl.h"
#include <tme/generic/ethernet.h>
#include <tme/threads.h>
#include <tme/misc.h>
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
#ifdef HAVE_AF_PACKET
#include <linux/filter.h>
#include <linux/if_ether.h>
#define TME_TUN_TAP_INSN struct sock_filter 
#define TME_TUN_TAP_PROG struct sock_fprog
#define TME_TUN_TAP_INSNS(x) x.filter
#define TME_TUN_TAP_LEN(x) x.len
#else
#include <net/tap.h>
#define TME_TUN_TAP_INSN struct tap_insn
#define TME_TUN_TAP_PROG struct tap_program
#define TME_TUN_TAP_INSNS(x) x.bf_insns
#define TME_TUN_TAP_LEN(x) x.bf_len
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

/* a crude ARP header: */
struct tme_net_arp_header {
  tme_uint8_t tme_net_arp_header_hardware[2];
  tme_uint8_t tme_net_arp_header_protocol[2];
  tme_uint8_t tme_net_arp_header_hardware_length;
  tme_uint8_t tme_net_arp_header_protocol_length;
  tme_uint8_t tme_net_arp_header_opcode[2];
};

/* a crude partial IPv4 header: */
struct tme_net_ipv4_header {
  tme_uint8_t tme_net_ipv4_header_v_hl;
  tme_uint8_t tme_net_ipv4_header_tos;
  tme_uint8_t tme_net_ipv4_header_length[2];
};

/* this creates a TAP filter that accepts Ethernet packets with
   destination addresses in the configured set.  the broadcast address
   must be in this set, it isn't accepted automatically: */
static int
_tme_tun_tap_filter(struct tme_ethernet_config *config, 
		    const tme_uint8_t *prefix,
		    unsigned int prefix_len,
		    TME_TUN_TAP_INSN *tap_filter,
		    int tap_filter_size,
		    int *_first_pc)
{
  unsigned int addr_i;
  tme_uint8_t byte;
  tme_uint8_t byte_bitmap[(1 << (8 * sizeof(byte))) >> 3];
  int match_pc, miss_pc, this_pc;

  /* clear the byte bitmap: */
  memset(byte_bitmap, 0, sizeof(byte_bitmap));

  /* the last instruction jumps to the reject insn when it fails: */
  miss_pc = tap_filter_size - 1;

  /* loop over all of the addresses: */
  for (addr_i = 0;
       addr_i < config->tme_ethernet_config_addr_count;
       addr_i++) {

    /* skip this address if it doesn't match the prefix: */
    if (prefix_len > 0
	&& memcmp(config->tme_ethernet_config_addrs[addr_i],
		  prefix,
		  prefix_len)) {
      continue;
    }

    /* get the next byte, and skip this address if this byte has
       already been done: */
    byte = config->tme_ethernet_config_addrs[addr_i][prefix_len];
    if (byte_bitmap[byte >> 3] & TME_BIT(byte & 7)) {
      continue;
    }
    byte_bitmap[byte >> 3] |= TME_BIT(byte & 7);

    /* get the PC of the instruction to branch to if this byte
       matches.  if this is the last byte of the address, the branch
       target is the accept insn, otherwise recurse and get the first
       insn of the rest of the matcher: */
    match_pc = ((prefix_len == (TME_ETHERNET_ADDR_SIZE - 1))
		? tap_filter_size - 2
		: _tme_tun_tap_filter(config,
				      config->tme_ethernet_config_addrs[addr_i],
				      prefix_len + 1,
				      tap_filter,
				      tap_filter_size,
				      _first_pc));

    /* add this testing instruction: */
    this_pc = --(*_first_pc);
    assert(this_pc >= 0);
    tap_filter[this_pc].code = TAP_JMP + TAP_JEQ + TAP_K;
    tap_filter[this_pc].jt = match_pc - (this_pc + 1);
    tap_filter[this_pc].jf = miss_pc - (this_pc + 1);
    tap_filter[this_pc].k = byte;

    /* update the miss pc: */
    miss_pc = this_pc;
  }

  /* add this load instruction: */
  this_pc = --(*_first_pc);
  assert(this_pc >= 0);
  tap_filter[this_pc].code = TAP_LD + TAP_B + TAP_ABS;
  tap_filter[this_pc].k = prefix_len;

  /* return our pc: */
  return (this_pc);
}

/* this dumps a TAP filter.  not all insns are supported, just
   those used by our address matching filters: */
void
_tme_tun_tap_dump_filter(const TME_TUN_TAP_PROG *program)
{
  unsigned int pc;
  FILE *fp;
  const TME_TUN_TAP_INSN *insn;
  char ldsize;
  const char *opc;

  fp = stderr;
  for (pc = 0, insn = TME_TUN_TAP_INSNS((*program));
       pc < (unsigned int) TME_TUN_TAP_LEN((*program));
       pc++, insn++) {
    
    /* the PC: */
    fprintf(fp, "%d:\t", pc);

    /* dispatch on the instruction class: */
    switch (TAP_CLASS(insn->code)) {

    case TAP_LD:

      switch (TAP_SIZE(insn->code)) {
      case TAP_B: ldsize = 'b'; break;
      case TAP_H: ldsize = 'w'; break;
      case TAP_W: ldsize = 'l'; break;
      default: ldsize = '?'; break;
      }
      fprintf(fp, "ld.%c ", ldsize);

      switch (TAP_MODE(insn->code)) {
      case TAP_ABS: fprintf(fp, "0x%x", insn->k); break;
      default: fprintf(fp, "??");
      }

      break;

    case TAP_JMP:

      switch (TAP_OP(insn->code)) {
      case TAP_JEQ: opc = "jeq"; break;
      default: opc = "??"; break;
      }
      fprintf(fp, "%s ", opc);

      switch (TAP_SRC(insn->code)) {
      case TAP_K: fprintf(fp, "#0x%x", insn->k); break;
      case TAP_X: fprintf(fp, "x"); break;
      default: fprintf(fp, "??"); break;
      }

      fprintf(fp, ", %d, %d", pc + 1 + insn->jt, pc + 1 + insn->jf);
      break;

    case TAP_RET:
      switch (TAP_RVAL(insn->code)) {
      case TAP_A: fprintf(fp, "ret a"); break;
      case TAP_X: fprintf(fp, "ret x"); break;
      case TAP_K: fprintf(fp, "ret #0x%x", insn->k); break;
      default: fprintf(fp, "ret ??"); break;
      }
      break;

    default:
      fprintf(fp, "??");
      break;
    }

    putc('\n', fp);
  }
}

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
  TME_TUN_TAP_INSN *tap_filter;
  TME_TUN_TAP_PROG program;
  int tap_filter_size, first_pc;
  int rc;

  /* recover our data structures: */
  tap = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume we will succeed: */
  rc = TME_OK;

  /* lock the mutex: */
  tme_mutex_lock(&tap->tme_tun_tap_mutex);

  /* allocate space for the worst-case filter: one insn for the packet
     accept, one insn for the packet reject, and TME_ETHERNET_ADDR_SIZE
     * 2 insns for each address - one insn to load an address byte and
     one insn to test it and branch: */
  tap_filter_size = (1
		     + 1
		     + ((1 + 1)
			* TME_ETHERNET_ADDR_SIZE
			* config->tme_ethernet_config_addr_count));
  tap_filter = tme_new(TME_TUN_TAP_INSN, tap_filter_size);
  first_pc = tap_filter_size;

  /* if this Ethernet is promiscuous, we will accept all packets: */
  if (config->tme_ethernet_config_flags & TME_ETHERNET_CONFIG_PROMISC) {
    tap_filter[--first_pc] = _tme_tun_tap_insn_accept;
  }

  /* if this Ethernet does have a set of addresses, we will accept all
     packets for one of those addresses: */
  else if (config->tme_ethernet_config_addr_count > 0) {

    /* the last insn in the filter is always the packet reject,
       and the next-to-last insn in the filter is always the
       packet accept.  _tme_tun_tap_filter depends on this: */
    tap_filter[--first_pc] = _tme_tun_tap_insn_reject;
    tap_filter[--first_pc] = _tme_tun_tap_insn_accept;

    /* make the address filter: */
    _tme_tun_tap_filter(config, 
			NULL,
			0,
			tap_filter,
			tap_filter_size,
			&first_pc);
  }

  /* otherwise this filter doesn't need to accept any packets: */
  else {
    tap_filter[--first_pc] = _tme_tun_tap_insn_reject;
  }

  /* set the filter on the TAP device: */
  TME_TUN_TAP_LEN(program) = tap_filter_size - first_pc;
  TME_TUN_TAP_INSNS(program) = tap_filter + first_pc;
#ifdef HAVE_AF_PACKET
  if (setsockopt(tap->tme_tun_tap_fd, SOL_SOCKET, SO_ATTACH_FILTER, &program, sizeof(program)) == -1) {
#else
  if (ioctl(tap->tme_tun_tap_fd, BIOCSETF, &program) < 0) {
#endif
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
#ifndef HAVE_AF_PACKET
  struct tap_hdr the_tap_header;
#endif
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
#ifdef HAVE_AF_PACKET
    if (tap->tme_tun_tap_buffer_offset >= tap->tme_tun_tap_buffer_end)
#else
    if ((tap->tme_tun_tap_buffer_offset + sizeof(the_tap_header)) > tap->tme_tun_tap_buffer_end)
#endif
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

#ifndef HAVE_AF_PACKET
    /* get the TAP header and check it: */
    memcpy(&the_tap_header,
	   tap->tme_tun_tap_buffer
	   + tap->tme_tun_tap_buffer_offset,
	   sizeof(the_tap_header));
    if(((tap->tme_tun_tap_buffer_offset
	   + the_tap_header.bh_hdrlen
	   + the_tap_header.bh_datalen)
	!= tap->tme_tun_tap_buffer_end))
      buffer_offset_next =
	tap->tme_tun_tap_buffer_offset
	+ TAP_WORDALIGN(the_tap_header.bh_hdrlen
			+ the_tap_header.bh_datalen);
    
    tap->tme_tun_tap_buffer_offset += the_tap_header.bh_hdrlen;

    /* if we're missing some part of the packet: */
    if (the_tap_header.bh_caplen != the_tap_header.bh_datalen
	|| ((tap->tme_tun_tap_buffer_offset + the_tap_header.bh_datalen)
	    > tap->tme_tun_tap_buffer_end)) {
      tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, TME_OK,
	      (&tap->tme_tun_tap_element->tme_element_log_handle,
	       _("flushed truncated TAP packet")));
      tap->tme_tun_tap_buffer_offset = buffer_offset_next;
      continue;
    }

    /* if this packet isn't big enough to even have an Ethernet header: */
    if (the_tap_header.bh_datalen < sizeof(struct tme_ethernet_header)) {
      tme_log(&tap->tme_tun_tap_element->tme_element_log_handle, 1, TME_OK,
	      (&tap->tme_tun_tap_element->tme_element_log_handle,
	       _("flushed short TAP packet")));
      tap->tme_tun_tap_buffer_offset = buffer_offset_next;
      continue;
    }

    /* if packets need to be delayed: */
    if (tap->tme_tun_tap_delay_time > 0) {
      
      /* if the current release time is before this packet's time: */
      if ((tap->tme_tun_tap_delay_release.tv_sec
	   < the_tap_header.bh_tstamp.tv_sec)
	  || ((tap->tme_tun_tap_delay_release.tv_sec
	       == the_tap_header.bh_tstamp.tv_sec)
	      && (tap->tme_tun_tap_delay_release.tv_usec
		  < the_tap_header.bh_tstamp.tv_usec))) {

	/* update the current release time, by taking the current time
	   and subtracting the delay time: */
	gettimeofday(&tap->tme_tun_tap_delay_release, NULL);
	if (tap->tme_tun_tap_delay_release.tv_usec < tap->tme_tun_tap_delay_time) {
	  tap->tme_tun_tap_delay_release.tv_usec += 1000000UL;
	  tap->tme_tun_tap_delay_release.tv_sec--;
	}
	tap->tme_tun_tap_delay_release.tv_usec -= tap->tme_tun_tap_delay_time;
      }

      /* if the current release time is still before this packet's
         time: */
      if ((tap->tme_tun_tap_delay_release.tv_sec
	   < the_tap_header.bh_tstamp.tv_sec)
	  || ((tap->tme_tun_tap_delay_release.tv_sec
	       == the_tap_header.bh_tstamp.tv_sec)
	      && (tap->tme_tun_tap_delay_release.tv_usec
		  < the_tap_header.bh_tstamp.tv_usec))) {

	/* set the sleep time: */
	assert ((tap->tme_tun_tap_delay_release.tv_sec
		 == the_tap_header.bh_tstamp.tv_sec)
		|| ((tap->tme_tun_tap_delay_release.tv_sec + 1)
		    == the_tap_header.bh_tstamp.tv_sec));
	tap->tme_tun_tap_delay_sleep
	  = (((tap->tme_tun_tap_delay_release.tv_sec
	       == the_tap_header.bh_tstamp.tv_sec)
	      ? 0
	      : 1000000UL)
	     + the_tap_header.bh_tstamp.tv_usec
	     - tap->tme_tun_tap_delay_release.tv_usec);

	/* rewind the buffer pointer: */
	tap->tme_tun_tap_buffer_offset -= the_tap_header.bh_hdrlen;

	/* stop now: */
	break;
      }
    }
#endif
    /* form the single frame chunk: */
    frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
      = tap->tme_tun_tap_buffer + tap->tme_tun_tap_buffer_offset;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
#ifdef HAVE_AF_PACKET
      = buffer_offset_next;
#else
      = the_tap_header.bh_datalen;
#endif

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
#ifndef HAVE_AF_PACKET
      /* rewind the buffer pointer: */
      tap->tme_tun_tap_buffer_offset -= the_tap_header.bh_hdrlen;
#endif
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
  char *dev_tap_filename = "/dev/net/tun";
  int saved_errno;
  u_int tap_opt;
#ifndef HAVE_AF_PACKET
  struct tap_version version;
#endif
  struct ifreq ifr;
  u_int packet_buffer_size;
  unsigned long delay_time;
  int arg_i;
  int usage;
  int rc;

  /* check our arguments: */
  usage = 0;
  ifr.ifr_name[0] = '\0';
  delay_time = 0;
  arg_i = 1;
  for (;;) {

    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "interface")
	&& args[arg_i + 1] != NULL) {
      strncpy(ifr.ifr_name, args[arg_i + 1], sizeof(ifr.ifr_name));
      arg_i += 2;
    }

    /* a delay time in microseconds: */
    else if (TME_ARG_IS(args[arg_i + 0], "delay")
	     && (delay_time = tme_misc_unumber_parse(args[arg_i + 1], 0)) > 0) {
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

  if (usage) {
    tme_output_append_error(_output,
			    "%s %s [ interface %s ] [ delay %s ]",
			    _("usage:"),
			    args[0],
			    _("INTERFACE"),
			    _("MICROSECONDS"));
    return (EINVAL);
  }

  tap_fd = tme_eth_alloc(element, dev_tap_filename);

  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
  
  /* this macro helps in closing the TAP socket on error: */
#define _TME_TAP_RAW_OPEN_ERROR(x) saved_errno = errno; x; errno = saved_errno

  /* try to create the device */
  if (ioctl(tap_fd, TUNSETIFF, (void *) &ifr) < 0 ) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set the TAP interface on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
  
  /* check the TAP version: */
  if (ioctl(tap_fd, BIOCVERSION, &version) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to get the TAP version on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
  if (version.bv_major != TAP_MAJOR_VERSION
      || version.bv_minor < TAP_MINOR_VERSION) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("kernel TAP version is %d.%d, my TAP version is %d.%d"),
	     version.bv_major, version.bv_minor,
	     TAP_MAJOR_VERSION, TAP_MINOR_VERSION));
    close(tap_fd);
    return (ENXIO);
  }
 
  /* put the TAP device into immediate mode: */
  tap_opt = TRUE;
  if (ioctl(tap_fd, BIOCIMMEDIATE, &tap_opt) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to put %s into immediate mode"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }

  /* tell the TAP device we're providing complete Ethernet headers: */
  tap_opt = TRUE;
  if (ioctl(tap_fd, BIOCSHDRCMPLT, &tap_opt) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to put %s into complete-headers mode"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }

  /* point the TAP device at the interface we're using: */
  if (ioctl(tap_fd, BIOCSETIF, &ifr) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to point TAP socket at %s"),
	     ifr.ifr_name));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }

  /* get the TAP read buffer size: */
  if (ioctl(tap_fd, BIOCGBLEN, &packet_buffer_size) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to read the buffer size for %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
  tme_log(&element->tme_element_log_handle, 1, errno,
	  (&element->tme_element_log_handle,
	   _("buffer size for %s is %u"),
	   dev_tap_filename, packet_buffer_size));

  /* set the interface into promiscuous mode: */
  if (ioctl(tap_fd, BIOCPROMISC) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set promiscuous mode on %s"),
	     dev_tap_filename));
    _TME_TAP_RAW_OPEN_ERROR(close(tap_fd));
    return (errno);
  }
  
  /* start our data structure: */
  tap = tme_new0(struct tme_tun_tap, 1);
  tap->tme_tun_tap_element = element;
  tap->tme_tun_tap_fd = tap_fd;
  tap->tme_tun_tap_buffer_size = packet_buffer_size;
  tap->tme_tun_tap_buffer = tme_new(tme_uint8_t, packet_buffer_size);
  tap->tme_tun_tap_delay_time = delay_time;

  /* start the threads: */
  tme_mutex_init(&tap->tme_tun_tap_mutex);
  tme_cond_init(&tap->tme_tun_tap_cond_reader);
  tme_thread_create((tme_thread_t) _tme_tun_tap_th_reader, tap);

  /* fill the element: */
  element->tme_element_private = tap;
  element->tme_element_connections_new = _tme_tun_tap_connections_new;

  return (TME_OK);
#undef _TME_TAP_RAW_OPEN_ERROR
}
