/* $Id: ethernet.c,v 1.4 2006/11/15 22:17:30 fredette Exp $ */

/* generic/ethernet.c - generic ethernet implementation support: */

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

#include <tme/common.h>
_TME_RCSID("$Id: ethernet.c,v 1.4 2006/11/15 22:17:30 fredette Exp $");

/* includes: */
#include <tme/generic/ethernet.h>
#include <stdlib.h>
#include <errno.h>

/* the Ethernet broadcast address: */
const tme_uint8_t tme_ethernet_addr_broadcast[TME_ETHERNET_ADDR_SIZE] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/* this scores an Ethernet connection: */
int
tme_ethernet_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_ETHERNET);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_ETHERNET);

  /* XXX we don't do any real checking: */
  *_score = 1;
  return (TME_OK);
}

/* this parses an ethernet address: */
int
tme_ethernet_addr_parse(const char *addr_string, tme_uint8_t *addr_bytes)
{
  const char *p1;
  char *p2;
  unsigned long byte;
  int byte_i;

  /* if we were given a NULL string, the parse fails: */
  if (addr_string == NULL) {
    return (EINVAL);
  }

  /* loop converting bytes: */
  p1 = addr_string;
  byte_i = 0;
  for(;;) {
    
    /* convert the next byte: */
    byte = strtoul(p1, &p2, 16);

    /* if some characters were converted: */
    if (p2 != p1) {
      
      /* if this byte is out of range, the parse fails: */
      if (byte > 0xff) {
	return (EINVAL);
      }

      /* if this is one too many bytes for an Ethernet address, the parse fails: */
      else if (byte_i == TME_ETHERNET_ADDR_SIZE) {
	return (EINVAL);
      }

      /* store this byte: */
      addr_bytes[byte_i++] = byte;
    }

    /* if the conversion stopped on a NUL: */
    if (*p2 == '\0') {
      
      /* if we haven't converted enough bytes, the parse fails,
	 otherwise the parse succeeds: */
      return ((byte_i == TME_ETHERNET_ADDR_SIZE)
	      ? TME_OK
	      : EINVAL);
    }

    /* if the conversion stopped on a colon, skip the colon and continue: */
    else if (*p2 == ':') {
      p1 = p2 + 1;
    }

    /* otherwise, the parse fails: */
    else {
      return (EINVAL);
    }
  }
  /* NOTREACHED */
}

/* this copies frame chunks: */
unsigned int
tme_ethernet_chunks_copy(const struct tme_ethernet_frame_chunk *chunks_dst,
			 const struct tme_ethernet_frame_chunk *chunks_src)
{
  const struct tme_ethernet_frame_chunk *chunk_dst;
  const struct tme_ethernet_frame_chunk *chunk_src;
  tme_uint8_t *chunk_bytes_dst;
  const tme_uint8_t *chunk_bytes_src;
  unsigned int chunk_size_dst;
  unsigned int chunk_size_src;
  unsigned int chunk_size;
  unsigned int frame_size_total;

  chunk_src = chunks_src;
  chunk_bytes_src = chunk_src->tme_ethernet_frame_chunk_bytes;
  chunk_size_src = chunk_src->tme_ethernet_frame_chunk_bytes_count;
  
  frame_size_total = 0;

  /* if we have been given a destination: */
  if (chunks_dst != NULL) {

    chunk_dst = chunks_dst;
    chunk_bytes_dst = chunk_dst->tme_ethernet_frame_chunk_bytes;
    chunk_size_dst = chunk_dst->tme_ethernet_frame_chunk_bytes_count;

    /* copy until we run out of source or destination chunks: */
    for (; (chunk_dst != NULL
	    && chunk_src != NULL); ) {
      
      /* get the amount we can copy now: */
      chunk_size = TME_MIN(chunk_size_dst, chunk_size_src);
      assert(chunk_size > 0);
      
      /* copy: */
      memcpy(chunk_bytes_dst, chunk_bytes_src, chunk_size);
      
      /* update: */
      frame_size_total += chunk_size;
      chunk_bytes_src += chunk_size;
      chunk_size_src -= chunk_size;
      if (chunk_size_src == 0
	  && (chunk_src = chunk_src->tme_ethernet_frame_chunk_next) != NULL) {
	chunk_bytes_src = chunk_src->tme_ethernet_frame_chunk_bytes;
	chunk_size_src = chunk_src->tme_ethernet_frame_chunk_bytes_count;
      }
      chunk_bytes_dst += chunk_size;
      chunk_size_dst -= chunk_size;
      if (chunk_size_dst == 0
	  && (chunk_dst = chunk_dst->tme_ethernet_frame_chunk_next) != NULL) {
	chunk_bytes_dst = chunk_dst->tme_ethernet_frame_chunk_bytes;
	chunk_size_dst = chunk_dst->tme_ethernet_frame_chunk_bytes_count;
      }
    }
  }

  /* count up the uncopied bytes: */
  for (; chunk_src != NULL; ) {
    frame_size_total += chunk_size_src;
    if ((chunk_src = chunk_src->tme_ethernet_frame_chunk_next) != NULL) {
      chunk_size_src = chunk_src->tme_ethernet_frame_chunk_bytes_count;
    }
  }

  /* done: */
  return (frame_size_total);
}

/* this calculates a little-endian Ethernet CRC.  this was cribbed
   from NetBSD's src/sys/net/if_ethersubr.c: */
tme_uint32_t
tme_ethernet_crc32_el(const tme_uint8_t *buf, unsigned int len)
{
  static const tme_uint32_t crctab[] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  tme_uint32_t crc;
  unsigned int i;

  crc = ((tme_uint32_t) 0) - 1;	/* initial value */

  for (i = 0; i < len; i++) {
    crc ^= buf[i];
    crc = (crc >> 4) ^ crctab[crc & 0xf];
    crc = (crc >> 4) ^ crctab[crc & 0xf];
  }

  return (crc);
}
