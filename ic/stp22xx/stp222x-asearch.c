/* $Id: stp222x-asearch.c,v 1.1 2009/02/28 16:29:25 fredette Exp $ */

/* ic/stp222x-asearch.c - address space search routine for emulation
   of the IOMMU of the UPA to SBus interface controller (STP2220) and
   the UPA to PCI interface controller (STP2222): */

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
_TME_RCSID("$Id: stp222x-asearch.c,v 1.1 2009/02/28 16:29:25 fredette Exp $");

#include "stp222x-impl.h"

#ifndef tme_stp222x_asearch

#define tme_stp222x_asearch tme_stp222x_asearch32
#define _tme_stp222x_asearch_t tme_bus_addr32_t
#include "stp222x-asearch.c"
#undef tme_stp222x_asearch
#undef _tme_stp222x_asearch_t

#define tme_stp222x_asearch tme_stp222x_asearch64
#define _tme_stp222x_asearch_t tme_bus_addr64_t
#include "stp222x-asearch.c"
#undef tme_stp222x_asearch
#undef _tme_stp222x_asearch_t

#else  /* defined(tme_stp222x_asearch) */

/* this searches a set of address ranges for one covering the given
   address: */
tme_uint32_t
tme_stp222x_asearch(const struct tme_stp222x_arange *aranges,
		    tme_uint32_t pivot,
		    _tme_stp222x_asearch_t address)
{
  tme_uint32_t left_p1;
  tme_uint32_t right_p1;
  _tme_stp222x_asearch_t first;
  _tme_stp222x_asearch_t offset;
  _tme_stp222x_asearch_t size_m1;

  /* binary search for the address: */
  left_p1 = 1;
  right_p1 = pivot;
  for (; left_p1 <= right_p1; ) {

    /* get this pivot: */
    pivot = (left_p1 + right_p1 - 2) / 2;

    /* get the first address in the range at the pivot: */
    first = aranges[pivot].tme_stp222x_arange_first;
    assert (first == aranges[pivot].tme_stp222x_arange_first);

    /* if the search address is less than this first address: */
    if (address < first) {

      /* move left: */
      right_p1 = pivot;

      /* if we have exhausted the ranges, any insertion should happen
	 at this last pivot: */
    }

    /* otherwise, the search address is greater than or equal to the
       first address: */
    else {

      /* get the offset of the search address from the first
	 address: */
      offset = address - first;

      /* get the size, minus one, of the range at the pivot: */
      size_m1 = aranges[pivot].tme_stp222x_arange_size_m1;
      assert (size_m1 == aranges[pivot].tme_stp222x_arange_size_m1);

      /* if the search address is in the range at the pivot: */
      if (offset <= size_m1) {
	return (pivot);
      }

      /* move right: */
      left_p1 = pivot + 2;

      /* if we have exhausted the ranges, any insertion should happen
	 after this last pivot: */
      pivot++;
    }
  }

  /* this address missed: */
  return (TME_STP222X_ASEARCH_MISS + pivot);
}

#endif /* defined(tme_stp222x_asearch) */
