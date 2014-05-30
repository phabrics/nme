/* $Id: stp222x-aspace.c,v 1.1 2009/02/28 16:29:25 fredette Exp $ */

/* ic/stp222x-aspace.c - address space utilities for emulation of the
   IOMMU of the UPA to SBus interface controller (STP2220) and the UPA
   to PCI interface controller (STP2222): */

/*
 * Copyright (c) 2009 Matt Fredette
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
_TME_RCSID("$Id: stp222x-aspace.c,v 1.1 2009/02/28 16:29:25 fredette Exp $");

/* includes: */
#include "stp222x-impl.h"

/* this searches an address space: */
tme_uint32_t
tme_stp222x_aspace_search(const struct tme_stp222x_aspace *aspace,
			  tme_bus_addr64_t address)
{

  /* if this is a 32-bit address: */
  if (__tme_predict_true(address < (((tme_bus_addr64_t) 1) << 32))) {
    return (tme_stp222x_asearch32(aspace->tme_stp222x_aspace_aranges,
				  aspace->tme_stp222x_aspace_arange32_count,
				  address));
  }

  /* otherwise, this is a 64-bit address: */
  return (tme_stp222x_asearch64(aspace->tme_stp222x_aspace_aranges,
				aspace->tme_stp222x_aspace_arange_count,
				address));
}

/* this looks up the slave connection for an address in a space, and
   adjusts the address to be slave-relative: */
tme_uint32_t
tme_stp222x_aspace_lookup(const struct tme_stp222x *stp222x,
			  unsigned int aspace_i,
			  tme_bus_addr64_t *_agent_to_slave_address)
{
  const struct tme_stp222x_aspace *aspace;
  tme_uint32_t search;
  tme_uint32_t conn_index;

  /* get the space: */
  aspace = &stp222x->tme_stp222x_aspaces[aspace_i];

  /* if this address isn't claimed in the space: */
  search = tme_stp222x_aspace_search(aspace, *_agent_to_slave_address);
  if (__tme_predict_false(search & TME_STP222X_ASEARCH_MISS)) {
    return (TME_STP222X_CONN_NULL);
  }

  /* get the connection index and adjust the address to be
     connection-relative: */
  conn_index = aspace->tme_stp222x_aspace_aranges[search].tme_stp222x_arange_key;
  assert (conn_index < TME_STP222X_CONN_NULL);
  *_agent_to_slave_address -= aspace->tme_stp222x_aspace_conn_offset[conn_index];
  return (conn_index);
}

/* this rebuilds the address spaces: */
int
tme_stp222x_aspaces_rebuild(struct tme_stp222x *stp222x)
{
  unsigned int aspace_i;
  struct tme_stp222x_aspace *aspace;
  tme_uint32_t conn_index;
  const struct tme_bus_connection *conn_bus;
  const struct tme_bus_connection *conn_bus_other;
  tme_bus_addr32_t conn_offset;
  const struct tme_bus_subregion *subregion;
  tme_bus_addr64_t address_first;
  tme_bus_addr64_t address_last;
  tme_uint32_t search;
  tme_uint32_t arange_count;
  struct tme_stp222x_arange *aranges;

  /* loop over the address spaces that apply to this part: */
  for (aspace_i = 0; aspace_i < TME_STP222X_ASPACE_NULL; aspace_i++) {
    if ((aspace_i == TME_STP2220_ASPACE_SBUS)
	== !TME_STP222X_IS_2220(stp222x)) {
      continue;
    }
    aspace = &stp222x->tme_stp222x_aspaces[aspace_i];

    /* free any existing address ranges: */
    if (aspace->tme_stp222x_aspace_arange_count != 0) {
      tme_free(aspace->tme_stp222x_aspace_aranges);
    }
    aspace->tme_stp222x_aspace_arange_count = 0;
    aspace->tme_stp222x_aspace_arange32_count = 0;

    /* loop over the connections: */
    for (conn_index = 0; conn_index < TME_STP222X_CONN_NULL; conn_index++) {
      conn_bus = stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus;
      if (conn_bus == NULL) {
	continue;
      }
      conn_bus_other = (struct tme_bus_connection *) conn_bus->tme_bus_connection.tme_connection_other;

      /* get the offset and subregions for this connection in this
	 space: */
      conn_offset = 0;
      switch (aspace_i) {
      case TME_STP2220_ASPACE_SBUS:
	conn_offset = stp222x->tme_stp2220_conn_offset[conn_index];
	/* FALLTHROUGH */
      case TME_STP2222_ASPACE_PCI_MEMORY(0):
      case TME_STP2222_ASPACE_PCI_MEMORY(1):
	subregion = &conn_bus_other->tme_bus_subregions;
	break;
      case TME_STP2222_ASPACE_PCI_IO(0):
      case TME_STP2222_ASPACE_PCI_IO(1):
	abort();
      default:
	assert (aspace_i == TME_STP2222_ASPACE_PCI_CONFIGURATION);
	abort();
      }

      /* set the offset for this connection in this space: */
      aspace->tme_stp222x_aspace_conn_offset[conn_index] = conn_offset;

      /* loop over the subregions: */
      for (; subregion != NULL; subregion = subregion->tme_bus_subregion_next) {

	/* get the addresses for this subregion: */
	address_first = conn_offset + subregion->tme_bus_subregion_address_first;
	address_last = conn_offset + subregion->tme_bus_subregion_address_last;
	assert (address_first <= address_last);

	/* if this first address is already claimed in the space: */
	search = tme_stp222x_aspace_search(aspace, address_first);
	if ((search & TME_STP222X_ASEARCH_MISS) == 0) {
	  return (-1);
	}
	search &= ~TME_STP222X_ASEARCH_MISS;

	/* grow the address ranges: */
	arange_count = ++aspace->tme_stp222x_aspace_arange_count;
	aranges
	  = (arange_count == 1
	     ? tme_new(struct tme_stp222x_arange, 1)
	     : tme_renew(struct tme_stp222x_arange,
			 aspace->tme_stp222x_aspace_aranges,
			 arange_count));
	aspace->tme_stp222x_aspace_aranges = aranges;

	/* if there are later address ranges: */
	if ((search + 1) < arange_count) {

	  /* if an address after the first address is already claimed
	     in the space: */
	  if (address_last >= aranges[search].tme_stp222x_arange_first) {
	    return (-1);
	  }

	  /* move down all of the other address ranges: */
	  memmove(aranges + search + 1,
		  aranges + search,
		  (sizeof(aranges[0])
		   * (arange_count
		      - (search + 1))));
	}

	/* fill this address range: */
	aranges[search].tme_stp222x_arange_first = address_first;
	aranges[search].tme_stp222x_arange_size_m1 = address_last - address_first;
	aranges[search].tme_stp222x_arange_key = conn_index;

	/* if this is a 32-bit address range, update the count: */
	if (address_first == (tme_uint32_t) address_first) {
	  assert (address_last == (tme_uint32_t) address_last);
	  aspace->tme_stp222x_aspace_arange32_count++;
	}
      }
    }
  }

  return (0);
}
