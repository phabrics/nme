/* $Id: ethernet.h,v 1.6 2007/02/15 01:27:13 fredette Exp $ */

/* tme/generic/ethernet.h - header file for generic ethernet support: */

/*
 * Copyright (c) 2003 Matt Fredette
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

#ifndef _TME_GENERIC_ETHERNET_H
#define _TME_GENERIC_ETHERNET_H

#include <tme/common.h>
_TME_RCSID("$Id: ethernet.h,v 1.6 2007/02/15 01:27:13 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* the size of an address: */
#define TME_ETHERNET_ADDR_SIZE		(6)

/* the size of a length field: */
#define TME_ETHERNET_LENGTH_SIZE	(2)

/* the size of a CRC: */
#define TME_ETHERNET_CRC_SIZE		(4)

/* the size of an Ethernet header: */
#define TME_ETHERNET_HEADER_SIZE	(TME_ETHERNET_ADDR_SIZE + TME_ETHERNET_ADDR_SIZE + TME_ETHERNET_LENGTH_SIZE)

/* the minimum and maximum sizes of an Ethernet frame, including the
   complete header and CRC: */
#define TME_ETHERNET_FRAME_MIN		(64)
#define TME_ETHERNET_FRAME_MAX		(1518)

/* Ethernet types: */
#define TME_ETHERNET_TYPE_IPV4		(0x0800)
#define TME_ETHERNET_TYPE_ARP		(0x0806)
#define TME_ETHERNET_TYPE_RARP		(0x8035)
   
/* Ethernet config flags: */
#define TME_ETHERNET_CONFIG_NORMAL	(0)
#define TME_ETHERNET_CONFIG_PROMISC	TME_BIT(0)

/* Ethernet control flags: */
#define TME_ETHERNET_CTRL_COLL_INPUT	TME_BIT(0)
#define TME_ETHERNET_CTRL_COLL_OUTPUT	TME_BIT(1)
#define TME_ETHERNET_CTRL_OK_READ	TME_BIT(2)

/* Ethernet read flags: */
#define TME_ETHERNET_READ_SPECIFIC	(0)
#define TME_ETHERNET_READ_NEXT		TME_BIT(0)
#define TME_ETHERNET_READ_PEEK		TME_BIT(1)

/* types: */

/* an ethernet frame identifier: */
typedef tme_uint32_t tme_ethernet_fid_t;

/* an ethernet header: */
struct tme_ethernet_header {

  /* the destination address: */
  tme_uint8_t tme_ethernet_header_dst[TME_ETHERNET_ADDR_SIZE];

  /* the source address: */
  tme_uint8_t tme_ethernet_header_src[TME_ETHERNET_ADDR_SIZE];

  /* the type: */
  tme_uint8_t tme_ethernet_header_type[2];
};

/* a ethernet configuration: */
struct tme_ethernet_config {
  
  /* the config flags: */
  tme_uint32_t tme_ethernet_config_flags;

  /* the set of specific Ethernet addresses to receive packets for: */
  unsigned int tme_ethernet_config_addr_count;
  _tme_const tme_uint8_t **tme_ethernet_config_addrs;
};

/* an ethernet frame chunk: */
struct tme_ethernet_frame_chunk {

  /* the next chunk in the frame: */
  struct tme_ethernet_frame_chunk *tme_ethernet_frame_chunk_next;

  /* a pointer to the bytes in this chunk: */
  tme_uint8_t *tme_ethernet_frame_chunk_bytes;

  /* the number of bytes in this chunk: */
  unsigned int tme_ethernet_frame_chunk_bytes_count;
};

/* a ethernet connection: */
struct tme_ethernet_connection {

  /* the generic connection side: */
  struct tme_connection tme_ethernet_connection;

  /* this is called when the ethernet configuration changes: */
  int (*tme_ethernet_connection_config) _TME_P((struct tme_ethernet_connection *, 
						struct tme_ethernet_config *));

  /* this is called when control flags change: */
  int (*tme_ethernet_connection_ctrl) _TME_P((struct tme_ethernet_connection *, 
					      unsigned int));

  /* this is called to read data: */
  int (*tme_ethernet_connection_read) _TME_P((struct tme_ethernet_connection *, 
					      tme_ethernet_fid_t *,
					      struct tme_ethernet_frame_chunk *,
					      unsigned int));
};

/* globals: */
extern _tme_const tme_uint8_t tme_ethernet_addr_broadcast[TME_ETHERNET_ADDR_SIZE];

/* prototypes: */
int tme_ethernet_addr_parse _TME_P((_tme_const char *, tme_uint8_t *));
int tme_ethernet_connection_score _TME_P((struct tme_connection *, unsigned int *));
unsigned int tme_ethernet_chunks_copy _TME_P((_tme_const struct tme_ethernet_frame_chunk *, _tme_const struct tme_ethernet_frame_chunk *));
tme_uint32_t tme_ethernet_crc32_el _TME_P((_tme_const tme_uint8_t *, unsigned int));

#endif /* !_TME_GENERIC_ETHERNET_H */
