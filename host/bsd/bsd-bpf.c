/* $Id: bsd-bpf.c,v 1.9 2007/02/21 01:24:50 fredette Exp $ */

/* host/bsd/bsd-bpf.c - BSD Berkeley Packet Filter Ethernet support: */

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
_TME_RCSID("$Id: bsd-bpf.c,v 1.9 2007/02/21 01:24:50 fredette Exp $");

/* includes: */
#include "eth-if.h"
#if defined(HAVE_AF_PACKET) && defined(HAVE_LINUX_FILTER_H)
#include <linux/filter.h>
#include <linux/if_ether.h>
#define TME_BSD_BPF_INSN struct sock_filter 
#define TME_BSD_BPF_PROG struct sock_fprog
#define TME_BSD_BPF_INSNS(x) x.filter
#define TME_BSD_BPF_LEN(x) x.len
#define HAVE_LSF 1
#else
#include <net/bpf.h>
#define TME_BSD_BPF_INSN struct bpf_insn
#define TME_BSD_BPF_PROG struct bpf_program
#define TME_BSD_BPF_INSNS(x) x.bf_insns
#define TME_BSD_BPF_LEN(x) x.bf_len
#endif

/* the accept and reject packet insns: */
static const TME_BSD_BPF_INSN _tme_bsd_bpf_insn_accept = BPF_STMT(BPF_RET + BPF_K, (u_int) -1);
static const TME_BSD_BPF_INSN _tme_bsd_bpf_insn_reject = BPF_STMT(BPF_RET + BPF_K, 0);

/* this creates a BPF filter that accepts Ethernet packets with
   destination addresses in the configured set.  the broadcast address
   must be in this set, it isn't accepted automatically: */
static int
_tme_bsd_bpf_filter(struct tme_ethernet_config *config, 
		    const tme_uint8_t *prefix,
		    unsigned int prefix_len,
		    TME_BSD_BPF_INSN *bpf_filter,
		    int bpf_filter_size,
		    int *_first_pc)
{
  unsigned int addr_i;
  tme_uint8_t byte;
  tme_uint8_t byte_bitmap[(1 << (8 * sizeof(byte))) >> 3];
  int match_pc, miss_pc, this_pc;

  /* clear the byte bitmap: */
  memset(byte_bitmap, 0, sizeof(byte_bitmap));

  /* the last instruction jumps to the reject insn when it fails: */
  miss_pc = bpf_filter_size - 1;

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
		? bpf_filter_size - 2
		: _tme_bsd_bpf_filter(config,
				      config->tme_ethernet_config_addrs[addr_i],
				      prefix_len + 1,
				      bpf_filter,
				      bpf_filter_size,
				      _first_pc));

    /* add this testing instruction: */
    this_pc = --(*_first_pc);
    assert(this_pc >= 0);
    bpf_filter[this_pc].code = BPF_JMP + BPF_JEQ + BPF_K;
    bpf_filter[this_pc].jt = match_pc - (this_pc + 1);
    bpf_filter[this_pc].jf = miss_pc - (this_pc + 1);
    bpf_filter[this_pc].k = byte;

    /* update the miss pc: */
    miss_pc = this_pc;
  }

  /* add this load instruction: */
  this_pc = --(*_first_pc);
  assert(this_pc >= 0);
  bpf_filter[this_pc].code = BPF_LD + BPF_B + BPF_ABS;
  bpf_filter[this_pc].k = prefix_len;

  /* return our pc: */
  return (this_pc);
}

/* this dumps a BPF filter.  not all insns are supported, just
   those used by our address matching filters: */
void
_tme_bsd_bpf_dump_filter(const TME_BSD_BPF_PROG *program)
{
  unsigned int pc;
  FILE *fp;
  const TME_BSD_BPF_INSN *insn;
  char ldsize;
  const char *opc;

  fp = stderr;
  for (pc = 0, insn = TME_BSD_BPF_INSNS((*program));
       pc < (unsigned int) TME_BSD_BPF_LEN((*program));
       pc++, insn++) {
    
    /* the PC: */
    fprintf(fp, "%d:\t", pc);

    /* dispatch on the instruction class: */
    switch (BPF_CLASS(insn->code)) {

    case BPF_LD:

      switch (BPF_SIZE(insn->code)) {
      case BPF_B: ldsize = 'b'; break;
      case BPF_H: ldsize = 'w'; break;
      case BPF_W: ldsize = 'l'; break;
      default: ldsize = '?'; break;
      }
      fprintf(fp, "ld.%c ", ldsize);

      switch (BPF_MODE(insn->code)) {
      case BPF_ABS: fprintf(fp, "0x%x", insn->k); break;
      default: fprintf(fp, "??");
      }

      break;

    case BPF_JMP:

      switch (BPF_OP(insn->code)) {
      case BPF_JEQ: opc = "jeq"; break;
      default: opc = "??"; break;
      }
      fprintf(fp, "%s ", opc);

      switch (BPF_SRC(insn->code)) {
      case BPF_K: fprintf(fp, "#0x%x", insn->k); break;
      case BPF_X: fprintf(fp, "x"); break;
      default: fprintf(fp, "??"); break;
      }

      fprintf(fp, ", %d, %d", pc + 1 + insn->jt, pc + 1 + insn->jf);
      break;

    case BPF_RET:
      switch (BPF_RVAL(insn->code)) {
      case BPF_A: fprintf(fp, "ret a"); break;
      case BPF_X: fprintf(fp, "ret x"); break;
      case BPF_K: fprintf(fp, "ret #0x%x", insn->k); break;
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

/* this is called when the ethernet configuration changes: */
static int
_tme_bsd_bpf_config(struct tme_ethernet_connection *conn_eth, 
		    struct tme_ethernet_config *config)
{
  struct tme_ethernet *bpf;
  TME_BSD_BPF_INSN *bpf_filter;
  TME_BSD_BPF_PROG program;
  int bpf_filter_size, first_pc;
  int rc;

  /* recover our data structures: */
  bpf = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume we will succeed: */
  rc = TME_OK;

  /* lock the mutex: */
  tme_mutex_lock(&bpf->tme_eth_mutex);

  /* allocate space for the worst-case filter: one insn for the packet
     accept, one insn for the packet reject, and TME_ETHERNET_ADDR_SIZE
     * 2 insns for each address - one insn to load an address byte and
     one insn to test it and branch: */
  bpf_filter_size = (1
		     + 1
		     + ((1 + 1)
			* TME_ETHERNET_ADDR_SIZE
			* config->tme_ethernet_config_addr_count));
  bpf_filter = tme_new(TME_BSD_BPF_INSN, bpf_filter_size);
  first_pc = bpf_filter_size;

  /* if this Ethernet is promiscuous, we will accept all packets: */
  if (config->tme_ethernet_config_flags & TME_ETHERNET_CONFIG_PROMISC) {
    bpf_filter[--first_pc] = _tme_bsd_bpf_insn_accept;
  }

  /* if this Ethernet does have a set of addresses, we will accept all
     packets for one of those addresses: */
  else if (config->tme_ethernet_config_addr_count > 0) {

    /* the last insn in the filter is always the packet reject,
       and the next-to-last insn in the filter is always the
       packet accept.  _tme_bsd_bpf_filter depends on this: */
    bpf_filter[--first_pc] = _tme_bsd_bpf_insn_reject;
    bpf_filter[--first_pc] = _tme_bsd_bpf_insn_accept;

    /* make the address filter: */
    _tme_bsd_bpf_filter(config, 
			NULL,
			0,
			bpf_filter,
			bpf_filter_size,
			&first_pc);
  }

  /* otherwise this filter doesn't need to accept any packets: */
  else {
    bpf_filter[--first_pc] = _tme_bsd_bpf_insn_reject;
  }

  /* set the filter on the BPF device: */
  TME_BSD_BPF_LEN(program) = bpf_filter_size - first_pc;
  TME_BSD_BPF_INSNS(program) = bpf_filter + first_pc;
#ifdef HAVE_LSF
  if (setsockopt(bpf->tme_eth_handle, SOL_SOCKET, SO_ATTACH_FILTER, &program, sizeof(program)) == -1) {
#else
  if (ioctl(bpf->tme_eth_handle, BIOCSETF, &program) < 0) {
#endif
    tme_log(&bpf->tme_eth_element->tme_element_log_handle, 0, errno,
	    (&bpf->tme_eth_element->tme_element_log_handle,
	     _("failed to set the filter")));
    rc = errno;
  }

  tme_log(&bpf->tme_eth_element->tme_element_log_handle, 0, TME_OK,
	  (&bpf->tme_eth_element->tme_element_log_handle,
	   _("set the filter")));

  /* free the filter: */
  tme_free(bpf_filter);

  /* unlock the mutex: */
  tme_mutex_unlock(&bpf->tme_eth_mutex);

  /* done: */
  return (rc);
}

#ifndef HAVE_LSF
/* this is called to read a frame: */
static int
_tme_bsd_bpf_read(struct tme_ethernet_connection *conn_eth, 
		  tme_ethernet_fid_t *_frame_id,
		  struct tme_ethernet_frame_chunk *frame_chunks,
		  unsigned int flags)
{
  struct tme_ethernet *bpf;
  tme_time_t tstamp;
#ifdef BIOCSTSTAMP
  // Assume timespec (nanosecond-accuracy) macros
  struct bpf_xhdr the_bpf_header;
#define TME_BPF_TIME_SEC(a) _TME_TIME_GET_FRAC(a,bt_sec)
#define TME_BPF_TIME_EQ(x,y) _TME_TIME_EQ(x,y,bt_sec,bt_frac)
#define TME_BPF_TIME_EQV(a,x,y) _TME_TIME_EQV(a,x,y,bt_sec,bt_frac)
#define TME_BPF_TIME_GT(x,y) _TME_TIME_GT(x,y,bt_sec,bt_frac)
#define TME_BPF_TIME_GET_FRAC(a) _TME_TIME_GET_FRAC(a,bt_frac)
#define TME_BPF_TIME_SET_FRAC(a,x) _TME_TIME_SET_FRAC(a,x,bt_frac)
#define TME_BPF_TIME_INC_FRAC(a,x) _TME_TIME_INC_FRAC(a,x,bt_frac)
#define TME_BPF_TIME_FRAC_LT(x,y,t) _TME_TIME_FRAC_LT(x,y,t,bt_frac)
#define TME_BPF_TIME_SETV(a,s,u) _TME_TIME_SETV(a,s,u,bt_sec,bt_frac)
#define TME_BPF_TIME_ADD(a,x,y) _TME_TIME_ADD(a,x,y,bt_sec,bt_frac)
#define TME_BPF_TIME_ADDV(a,s,u) _TME_TIME_ADDV(a,s,u,bt_sec,bt_frac)
#define TME_BPF_TIME_INC(a,x) _TME_TIME_INC(a,x,bt_sec,bt_frac)
#define TME_BPF_TIME_SUB(a,x,y) _TME_TIME_SUB(a,x,y,bt_sec,bt_frac)
#define TME_BPF_TIME_DEC(a,x) _TME_TIME_DEC(a,x,bt_sec,bt_frac)
#else
  // Make timeval (microsecond-accuracy) macros
  struct bpf_hdr the_bpf_header;
#define TME_BPF_TIME_SEC(a) _TME_TIME_GET_FRAC(a,tv_sec)
#define TME_BPF_TIME_EQ(x,y) _TME_TIME_EQ(x,y,tv_sec,tv_usec)
#define TME_BPF_TIME_EQV(a,x,y) _TME_TIME_EQV(a,x,y,tv_sec,tv_usec)
#define TME_BPF_TIME_GT(x,y) _TME_TIME_GT(x,y,tv_sec,tv_usec)
#define TME_BPF_TIME_GET_FRAC(a) _TME_TIME_GET_FRAC(a,tv_usec)
#define TME_BPF_TIME_SET_FRAC(a,x) _TME_TIME_SET_FRAC(a,x,tv_usec)
#define TME_BPF_TIME_INC_FRAC(a,x) _TME_TIME_INC_FRAC(a,x,tv_usec)
#define TME_BPF_TIME_FRAC_LT(x,y) _TME_TIME_FRAC_LT(x,y,tv_usec)
#define TME_BPF_TIME_SETV(a,s,u) _TME_TIME_SETV(a,s,u,tv_sec,tv_usec)
#define TME_BPF_TIME_ADD(a,x,y) _TME_TIME_ADD(a,x,y,tv_sec,tv_usec)
#define TME_BPF_TIME_ADDV(a,s,u) _TME_TIME_ADDV(a,s,u,tv_sec,tv_usec)
#define TME_BPF_TIME_INC(a,x) _TME_TIME_INC(a,x,tv_sec,tv_usec)
#define TME_BPF_TIME_SUB(a,x,y) _TME_TIME_SUB(a,x,y,tv_sec,tv_usec)
#define TME_BPF_TIME_DEC(a,x) _TME_TIME_DEC(a,x,tv_sec,tv_usec)
#endif
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  size_t buffer_offset_next;
  unsigned int count;
  unsigned long delay_time;
  int rc;

  /* recover our data structure: */
  bpf = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&bpf->tme_eth_mutex);

  /* assume that we won't be able to return a packet: */
  rc = -ENOENT;

  delay_time = *(unsigned long *)bpf->tme_eth_data;
  /* loop until we have a good captured packet or until we 
     exhaust the buffer: */
  for (;;) {
    buffer_offset_next = bpf->tme_eth_buffer_end;
    /* if there's not enough for a BPF header, flush the buffer: */
    if ((bpf->tme_eth_buffer_offset + sizeof(the_bpf_header)) > bpf->tme_eth_buffer_end)
    {
      if (bpf->tme_eth_buffer_offset
	  != bpf->tme_eth_buffer_end) {
	tme_log(&bpf->tme_eth_element->tme_element_log_handle, 1, TME_OK,
		(&bpf->tme_eth_element->tme_element_log_handle,
		 _("flushed garbage BPF header bytes")));
	bpf->tme_eth_buffer_offset = bpf->tme_eth_buffer_end;
      }
      break;
    }

    /* get the BPF header and check it: */
    memcpy(&the_bpf_header,
	   bpf->tme_eth_buffer
	   + bpf->tme_eth_buffer_offset,
	   sizeof(the_bpf_header));
    if(((bpf->tme_eth_buffer_offset
	   + the_bpf_header.bh_hdrlen
	   + the_bpf_header.bh_datalen)
	!= bpf->tme_eth_buffer_end))
      buffer_offset_next =
	bpf->tme_eth_buffer_offset
	+ BPF_WORDALIGN(the_bpf_header.bh_hdrlen
			+ the_bpf_header.bh_datalen);
    
    bpf->tme_eth_buffer_offset += the_bpf_header.bh_hdrlen;

    /* if we're missing some part of the packet: */
    if (the_bpf_header.bh_caplen != the_bpf_header.bh_datalen
	|| ((bpf->tme_eth_buffer_offset + the_bpf_header.bh_datalen)
	    > bpf->tme_eth_buffer_end)) {
      tme_log(&bpf->tme_eth_element->tme_element_log_handle, 1, TME_OK,
	      (&bpf->tme_eth_element->tme_element_log_handle,
	       _("flushed truncated BPF packet")));
      bpf->tme_eth_buffer_offset = buffer_offset_next;
      continue;
    }

    /* if this packet isn't big enough to even have an Ethernet header: */
    if (the_bpf_header.bh_datalen < sizeof(struct tme_ethernet_header)) {
      tme_log(&bpf->tme_eth_element->tme_element_log_handle, 1, TME_OK,
	      (&bpf->tme_eth_element->tme_element_log_handle,
	       _("flushed short BPF packet")));
      bpf->tme_eth_buffer_offset = buffer_offset_next;
      continue;
    }

    /* if packets need to be delayed: */
    if (delay_time > 0) {
      TME_TIME_SETV(tstamp, 
		    TME_BPF_TIME_SEC(the_bpf_header.bh_tstamp), 
		    TME_BPF_TIME_GET_FRAC(the_bpf_header.bh_tstamp));
      /* if the current release time is before this packet's time: */
      if (TME_TIME_GT(tstamp, bpf->tme_eth_delay_release)) {
	/* update the current release time, by taking the current time
	   and subtracting the delay time: */
	tme_get_time(&bpf->tme_eth_delay_release);
	if (TME_TIME_GET_FRAC(bpf->tme_eth_delay_release) < delay_time) {
	  TME_TIME_ADDV(bpf->tme_eth_delay_release, -1, 1000000UL);
	}
	TME_TIME_INC_FRAC(bpf->tme_eth_delay_release, -delay_time);
      }

      /* if the current release time is still before this packet's
         time: */
      if (TME_TIME_GT(tstamp, bpf->tme_eth_delay_release)) {
	/* set the sleep time: */
	assert (TME_TIME_GET_SEC(bpf->tme_eth_delay_release) - TME_TIME_GET_SEC(tstamp) <= 1);
	bpf->tme_eth_delay_sleep
	  = ((TME_TIME_GET_SEC(bpf->tme_eth_delay_release)
	      == TME_TIME_GET_SEC(tstamp))
	      ? 0
	     : 1000000UL)
	  + TME_TIME_GET_FRAC(tstamp) - 
	  TME_TIME_GET_FRAC(bpf->tme_eth_delay_release);

	/* rewind the buffer pointer: */
	bpf->tme_eth_buffer_offset -= the_bpf_header.bh_hdrlen;

	/* stop now: */
	break;
      }
    }

    /* form the single frame chunk: */
    frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
      = bpf->tme_eth_buffer + bpf->tme_eth_buffer_offset;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
      = the_bpf_header.bh_datalen;

    /* copy out the frame: */
    count = tme_ethernet_chunks_copy(frame_chunks, &frame_chunk_buffer);

    /* if this is a peek: */
    if (flags & TME_ETHERNET_READ_PEEK) {
      /* rewind the buffer pointer: */
      bpf->tme_eth_buffer_offset -= the_bpf_header.bh_hdrlen;
    }

    /* otherwise, this isn't a peek: */
    else {

      /* update the buffer pointer: */
      bpf->tme_eth_buffer_offset = buffer_offset_next;
    }

    /* success: */
    rc = count;
    break;
  }

  /* if the buffer is empty, or if we failed to read a packet,
     wake up the reader: */
  if ((bpf->tme_eth_buffer_offset
       >= bpf->tme_eth_buffer_end)
      || rc <= 0) {
    tme_cond_notify(&bpf->tme_eth_cond_reader, TRUE);
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&bpf->tme_eth_mutex);

  /* done: */
  return (rc);
}
#endif

/* this makes a new connection side for a ETH: */
static int
_tme_bsd_bpf_connections_new(struct tme_element *element, 
			     const char * const *args, 
			     struct tme_connection **_conns)
{
  struct tme_ethernet_connection *conn_eth;

  tme_eth_connections_new(element, args, _conns);
  conn_eth = (struct tme_ethernet_connection *) (*_conns);
  conn_eth->tme_ethernet_connection_config = _tme_bsd_bpf_config;
#ifndef HAVE_LSF
  conn_eth->tme_ethernet_connection_read = _tme_bsd_bpf_read;
#endif  
  return (TME_OK);
}

/* retrieve ethernet arguments */
int _tme_bsd_bpf_args(const char * const args[], 
		      struct ifreq *ifr,
		      u_int *rbufsz,
		      unsigned long *delay,
		      char **_output)
{
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = 0;
  ifr->ifr_name[0] = '\0';
  *delay = 0;

  arg_i = 1;

  for (;;) {
    /* the interface we're supposed to use: */
    if (TME_ARG_IS(args[arg_i + 0], "interface")
	&& args[arg_i + 1] != NULL) {
      strncpy(ifr->ifr_name, args[arg_i + 1], sizeof(ifr->ifr_name));
      arg_i += 2;
    }

    /* the buffer size to use for reads: */
    else if (TME_ARG_IS(args[arg_i + 0], "rbufsz")
	     && (*rbufsz = tme_misc_unumber_parse(args[arg_i + 1], 0)) > 0) {
      arg_i += 2;
    }

    /* a delay time in microseconds: */
    else if (TME_ARG_IS(args[arg_i + 0], "delay")
	     && (*delay = tme_misc_unumber_parse(args[arg_i + 1], 0)) > 0) {
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
  return (TME_OK);
}

/* the new BPF function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_bsd,bpf) {
  int bpf_fd;
#ifdef HAVE_LSF
  struct sockaddr_ll sll;
  struct packet_mreq mr;
#else
#define DEV_BPF_FORMAT "/dev/bpf"
  char dev_bpf_filename[sizeof(DEV_BPF_FORMAT) + 4];
#endif
  int saved_errno;
  u_int bpf_opt;
#ifndef HAVE_LSF
  struct bpf_version version;
#endif
  struct ifreq ifr;
  struct ifaddrs *ifa;
  u_int packet_buffer_size;
  unsigned long *delay_time;
  int rc;

  packet_buffer_size = 16384;
  delay_time = tme_new0(unsigned long, 1);

  /* get the arguments: */
  rc = _tme_bsd_bpf_args(args, &ifr, &packet_buffer_size, delay_time, _output);

  /* find the interface we will use: */
  rc = tme_eth_ifaddrs_find(ifr.ifr_name, AF_UNSPEC, &ifa, NULL, NULL);

  if (rc != TME_OK) {
    tme_output_append_error(_output, _("couldn't find an interface %s"), ifr.ifr_name);
    return (ENOENT);
  }

  strncpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name));

  tme_log(&element->tme_element_log_handle, 0, TME_OK, 
	  (&element->tme_element_log_handle, 
	   "using interface %s",
	   ifr.ifr_name));

#ifdef HAVE_LSF
  if ((bpf_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) >= 0) {
      tme_log(&element->tme_element_log_handle, 0, TME_OK,
	      (&element->tme_element_log_handle,
	       "opened packet socket"));
  }
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_protocol = htons(ETH_P_ALL);
  sll.sll_ifindex = if_nametoindex(ifr.ifr_name);
  if (bind(bpf_fd, (struct sockaddr *)&sll, sizeof(sll)) == -1) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to bind packet socket to interface")));
  }

  memset(&mr, 0, sizeof(mr));
  mr.mr_ifindex = if_nametoindex(ifr.ifr_name);
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(bpf_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
    tme_log(&element->tme_element_log_handle, 0, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set promiscuous mode on interface")));
    saved_errno = errno;
    close(bpf_fd);
    errno = saved_errno;
    return (errno);
  }
#else
  sprintf(dev_bpf_filename, DEV_BPF_FORMAT);
  
  bpf_fd = tme_eth_alloc(dev_bpf_filename, _output);

  if(bpf_fd < 0) {
    saved_errno = errno;
    tme_log(&element->tme_element_log_handle, 1, saved_errno,
	    (&element->tme_element_log_handle,
	     _("failed to open BPF device %s"),
	     dev_bpf_filename));
    return (saved_errno);
  }

  /* this macro helps in closing the BPF socket on error: */
#define _TME_BPF_RAW_OPEN_ERROR(x) saved_errno = errno; x; errno = saved_errno

  /* check the BPF version: */
  if (ioctl(bpf_fd, BIOCVERSION, &version) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to get the BPF version on %s"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }
  if (version.bv_major != BPF_MAJOR_VERSION
      || version.bv_minor < BPF_MINOR_VERSION) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("kernel BPF version is %d.%d, my BPF version is %d.%d"),
	     version.bv_major, version.bv_minor,
	     BPF_MAJOR_VERSION, BPF_MINOR_VERSION));
    close(bpf_fd);
    return (ENXIO);
  }
 
  /* put the BPF device into immediate mode: */
  bpf_opt = TRUE;
  if (ioctl(bpf_fd, BIOCIMMEDIATE, &bpf_opt) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to put %s into immediate mode"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }

  /* tell the BPF device we're providing complete Ethernet headers: */
  bpf_opt = TRUE;
  if (ioctl(bpf_fd, BIOCSHDRCMPLT, &bpf_opt) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to put %s into complete-headers mode"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }

#ifdef BIOCSTSTAMP
  /* prefer timespec to timeval: */
  bpf_opt = BPF_T_NANOTIME;
  if (ioctl(bpf_fd, BIOCSTSTAMP, &bpf_opt) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to put %s into nanotime (timespec) mode"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }
#endif

  /* get the BPF read buffer size: */
  if (ioctl(bpf_fd, BIOCSBLEN, &packet_buffer_size) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to read the buffer size for %s"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }
  tme_log(&element->tme_element_log_handle, 0, errno,
	  (&element->tme_element_log_handle,
	   _("buffer size for %s is %u"),
	   dev_bpf_filename, packet_buffer_size));

  /* point the BPF device at the interface we're using: */
  if (ioctl(bpf_fd, BIOCSETIF, &ifr) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to point BPF socket at %s"),
	     ifr.ifr_name));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }

  /* set the interface into promiscuous mode: */
  if (ioctl(bpf_fd, BIOCPROMISC) < 0) {
    tme_log(&element->tme_element_log_handle, 1, errno,
	    (&element->tme_element_log_handle,
	     _("failed to set promiscuous mode on %s"),
	     dev_bpf_filename));
    _TME_BPF_RAW_OPEN_ERROR(close(bpf_fd));
    return (errno);
  }
  
#endif
  
  return tme_eth_init(element, bpf_fd, packet_buffer_size, delay_time, NULL, _tme_bsd_bpf_connections_new);

#undef _TME_BPF_RAW_OPEN_ERROR
}
