/* $Id: stp222x-iommu.c,v 1.3 2010/06/05 18:59:29 fredette Exp $ */

/* ic/stp222x-iommu.c - emulation of the IOMMU of the UPA to SBus
   interface controller (STP2220) and the UPA to PCI interface
   controller (STP2222): */

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
_TME_RCSID("$Id: stp222x-iommu.c,v 1.3 2010/06/05 18:59:29 fredette Exp $");

/* includes: */
#include "stp222x-impl.h"

/* macros: */

/* IOMMU register offsets: */
#define TME_STP222X_IOMMU_REGGROUP_INDEX_CR		TME_STP222X_REGGROUP_INDEX(0x00)
#define TME_STP222X_IOMMU_REGGROUP_INDEX_TSB		TME_STP222X_REGGROUP_INDEX(0x08)
#define TME_STP222X_IOMMU_REGGROUP_INDEX_FLUSH		TME_STP222X_REGGROUP_INDEX(0x10)
#define TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_VA	TME_STP222X_REGGROUP_INDEX(0x00)
#define TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_COMPARE	TME_STP222X_REGGROUP_INDEX(0x08)

/* the IOMMU control register: */
#define TME_STP222X_IOMMU_CR_XLT_ERR_MASK		((2 << 26) - (1 << 25))
#define TME_STP222X_IOMMU_CR_XLT_ERR			(1 << 24)
#define TME_STP222X_IOMMU_CR_LRU_LCKEN			(1 << 23)
#define TME_STP222X_IOMMU_CR_LRU_LCKPTR			((2 << 22) - (1 << 19))
#define TME_STP222X_IOMMU_CR_TSB_SIZE_MASK		((2 << 18) - (1 << 16))
#define TME_STP222X_IOMMU_CR_TBW_SIZE_MASK		(1 << 2)
#define  TME_STP222X_IOMMU_CR_TBW_SIZE_64KB		 (1 << 2)
#define  TME_STP222X_IOMMU_CR_TBW_SIZE_8KB		 (0 << 2)
#define TME_STP222X_IOMMU_CR_MMU_DE			(1 << 1)
#define TME_STP222X_IOMMU_CR_MMU_EN			(1 << 0)
#define TME_STP222X_IOMMU_CR_MBZ			\
  (((((tme_uint32_t) 2) << 31) - (1 << 27))		\
   | ((2 << 15) - (1 << 3)))

/* an IOMMU TTE: */
#define TME_STP222X_IOMMU_TTE_DATA_V			(((tme_uint64_t) 1) << 63)
#define TME_STP222X_IOMMU_TTE_DATA_SIZE_MASK		(((tme_uint64_t) 1) << 61)
#define  TME_STP222X_IOMMU_TTE_DATA_SIZE_64KB		 (((tme_uint64_t) 1) << 61)
#define  TME_STP222X_IOMMU_TTE_DATA_SIZE_8KB		 (((tme_uint64_t) 0) << 61)
#define TME_STP222X_IOMMU_TTE_STREAM			(((tme_uint64_t) 1) << 60)
#define TME_STP2220_IOMMU_TTE_LOCALBUS			(((tme_uint64_t) 1) << 59)
#define TME_STP222X_IOMMU_TTE_DATA_PA			((((tme_uint64_t) 2) << 40) - (1 << 13))
#define TME_STP222X_IOMMU_TTE_CACHEABLE			(1 << 4)
#define TME_STP222X_IOMMU_TTE_DATA_W			(1 << 1)

/* an IOMMU TLB data: */
#define TME_STP222X_IOMMU_TLB_DATA_V			(1 << 30)
#define TME_STP2220_IOMMU_TLB_DATA_LOCAL		(1 << 29)
#define TME_STP222X_IOMMU_TLB_DATA_C			(1 << 28)
#define TME_STP222X_IOMMU_TLB_DATA_PPN			((2 << 27) - (1 << 0))

/* an IOMMU TLB tag: */
#define TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK		((2 << 24) - (1 << 23))
#define  TME_STP222X_IOMMU_TLB_TAG_ERROR_PROTECTION	 (0 << 23)
#define  TME_STP222X_IOMMU_TLB_TAG_ERROR_INVALID	 (1 << 23)
#define  TME_STP222X_IOMMU_TLB_TAG_ERROR_TIMEOUT	 (2 << 23)
#define  TME_STP222X_IOMMU_TLB_TAG_ERROR_ECC_UE		 (3 << 23)
#define TME_STP222X_IOMMU_TLB_TAG_ERROR			(1 << 22)
#define TME_STP222X_IOMMU_TLB_TAG_W			(1 << 21)
#define TME_STP222X_IOMMU_TLB_TAG_S			(1 << 20)
#define TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK		(1 << 19)
#define  TME_STP222X_IOMMU_TLB_TAG_SIZE_64KB		 (1 << 19)
#define  TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB		 (0 << 19)
#define TME_STP222X_IOMMU_TLB_TAG_VPN			((2 << 18) - (1 << 0))

/* _tme_stp222x_iommu_lookup() returns a mash of TLB tag and TLB data
   information.  the non-page-number fields of tag and data don't
   overlap, and we don't include the page numbers, which leaves bits
   left over.  we use these bits for additional information: */
/* NB: the specific values for TME_STP222X_IOMMU_TLB_MASH_UPA_* are
   chosen to make it easy to quickly generate the corresponding
   address mask: */
#define TME_STP222X_IOMMU_TLB_MASH_UPA_41		(1 << 27)
#define TME_STP222X_IOMMU_TLB_MASH_UPA_31		(1 << 17)
#define TME_STP222X_IOMMU_TLB_MASH_FIXED		(1 << 16)
#define TME_STP222X_IOMMU_TLB_MASH_INVALID_REQUEST	(1 << 15)
#define TME_STP222X_IOMMU_TLB_MASH_MISS			(1 << 14)
#define TME_STP222X_IOMMU_TLB_MASH_TLB_I(n)		(n)
#define TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(x)	((x) % TME_STP222X_IOMMU_TLB_SIZE)

/* sizes: */
#define TME_STP222X_IOMMU_SIZE_LOG2_8KB			(13)
#define TME_STP222X_IOMMU_SIZE_LOG2_64KB		(16)
#define TME_STP222X_IOMMU_SIZE_8KB			(1 << TME_STP222X_IOMMU_SIZE_LOG2_8KB)
#define TME_STP222X_IOMMU_SIZE_64KB			(1 << TME_STP222X_IOMMU_SIZE_LOG2_64KB)

/* this returns the log2 of the TBW size: */
static tme_uint32_t
_tme_stp222x_iommu_tbw_size_log2(tme_uint32_t iommu_cr)
{
  tme_uint32_t tbw_size;

  /* shift TBW_SIZE down to bit zero and clear all other bits: */
  tbw_size
    = ((iommu_cr
	/ TME_STP222X_IOMMU_CR_TBW_SIZE_MASK)
       & 1);

  /* multiply this by the difference between log2(64KB) and log2(8KB),
     and then add log2(8KB).  if TBW_SIZE was set, the result will be
     log2(64KB), otherwise it will be log2(8KB): */
  return ((tbw_size
	   * (TME_STP222X_IOMMU_SIZE_LOG2_64KB
	      - TME_STP222X_IOMMU_SIZE_LOG2_8KB))
	  + TME_STP222X_IOMMU_SIZE_LOG2_8KB);
}

/* this returns the VPN mask for a TLB entry: */
static tme_uint32_t
_tme_stp222x_iommu_tlb_tag_vpn_mask(tme_uint32_t tlb_tag)
{
  tme_uint32_t tlb_tag_vpn_mask;

  /* copy the TLB entry tag into the TLB tag VPN mask, and shift the
     page size bit down to bit zero and clear all other bits: */
  tlb_tag_vpn_mask
    = ((tlb_tag
	/ TME_STP222X_IOMMU_TLB_TAG_SIZE_64KB)
       & 1);

  /* add in (64KB / 8KB) - 1.  if the page size is 64KB, this will
     leave bits 0..2 zero and bit 3 set, otherwise the page size is
     8KB and this will leave bits 0..2 one and bit 3 clear: */
  tlb_tag_vpn_mask
    += ((TME_STP222X_IOMMU_SIZE_64KB
	 / TME_STP222X_IOMMU_SIZE_8KB)
	- 1);

  /* set bits 3..18 of the TLB tag VPN mask: */
  tlb_tag_vpn_mask
    |= (TME_STP222X_IOMMU_TLB_TAG_VPN
	- ((TME_STP222X_IOMMU_SIZE_64KB
	    / TME_STP222X_IOMMU_SIZE_8KB)
	   - 1));

  /* return the VPN mask: */
  return (tlb_tag_vpn_mask);
}

/* this returns the page size from a TLB tag: */
static tme_uint32_t
_tme_stp222x_iommu_tlb_page_size(tme_uint32_t tlb_tag)
{
  tme_uint32_t page_size;

  /* shift the page size bit down into bit 13 (8KB) and clear all
     other bits: */
#if TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK < TME_STP222X_IOMMU_SIZE_8KB
#error "TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK changed"
#endif
  page_size
    = ((tlb_tag
	/ (TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK
	   / TME_STP222X_IOMMU_SIZE_8KB))
       & TME_STP222X_IOMMU_SIZE_8KB);

  /* add 64KB - 8KB to this value.  the result will be 64KB if the
     page size is 64KB, and (64KB - 8KB) if the page size is 8KB: */
#if TME_STP222X_IOMMU_TLB_TAG_SIZE_64KB != TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK
#error "TME_STP222X_IOMMU_TLB_TAG_SIZE_ values changed"
#endif
  page_size
    += (TME_STP222X_IOMMU_SIZE_64KB
	- TME_STP222X_IOMMU_SIZE_8KB);

  /* clear all other bits except for 64KB and 8KB.  only one of them
     will be set: */
  page_size
    &= (TME_STP222X_IOMMU_SIZE_64KB
	| TME_STP222X_IOMMU_SIZE_8KB);

  return (page_size);
}

/* this looks up an address in the IOMMU and returns a mash of TLB
   entry tag and data information: */
static tme_uint32_t
_tme_stp222x_iommu_tlb_mash(const struct tme_stp222x *stp222x,
			    tme_bus_addr64_t io_address,
			    unsigned int cycle_type)
{
  tme_uint32_t iommu_cr;
  tme_uint32_t io_space_size_log2;
  tme_uint32_t io_address_0_31;
  tme_uint32_t io_address_tag;
  unsigned long tlb_i;
  unsigned int tlb_count;
  tme_uint32_t tlb_tag;

  /* get the IOMMU control register: */
  iommu_cr = stp222x->tme_stp222x_iommu_cr;

  /* if this is an stp2220: */
  if (TME_STP222X_IS_2220(stp222x)) {

    /* if the IOMMU is disabled: */
    if ((iommu_cr & TME_STP222X_IOMMU_CR_MMU_EN) == 0) {
      
      /* XXX FIXME - what does the stp2220 do if the IOMMU is
	 disabled? */
      abort();
    }

    /* otherwise, this address is translated: */
    /* XXX FIXME - does the STP2220 IOMMU have any address ranges that
       are bypassed, even when the IOMMU is enabled? */
  }

  /* otherwise, this is an stp2222: */
  else {

    /* if this lookup is for a Dual Address Cycle: */
    if (FALSE) {

      /* if this is a bypass address: */
      if ((io_address >> 50) == 0x3fff) {

	/* bits 0..40 of the address become the UPA address.  if bit
	   40 is clear, the address is cacheable: */
	return ((io_address & (((tme_uint64_t) 1) << 40))
		? (TME_STP222X_IOMMU_TLB_MASH_FIXED
		   | TME_STP222X_IOMMU_TLB_MASH_UPA_41
		   | TME_STP222X_IOMMU_TLB_DATA_V
		   | TME_STP222X_IOMMU_TLB_TAG_W
		   | TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB)
		: (TME_STP222X_IOMMU_TLB_MASH_FIXED
		   | TME_STP222X_IOMMU_TLB_MASH_UPA_41
		   | TME_STP222X_IOMMU_TLB_DATA_V
		   | TME_STP222X_IOMMU_TLB_DATA_C
		   | TME_STP222X_IOMMU_TLB_TAG_W
		   | TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB));
      }

      /* otherwise, this address is local: */
      else {
	return (TME_STP222X_IOMMU_TLB_MASH_FIXED
		| TME_STP222X_IOMMU_TLB_DATA_V
		| TME_STP2220_IOMMU_TLB_DATA_LOCAL
		| TME_STP222X_IOMMU_TLB_TAG_W
		| TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB);
      }
    }

    /* otherwise, this is a Single Address Cycle: */
    else {

      /* if this address is translated or passed-through: */
      if (io_address & (((tme_uint32_t) 1) << 31)) {

	/* if this address is passed-through, bits 0..30 of the
	   address become the cacheable UPA address: */
	if ((iommu_cr & TME_STP222X_IOMMU_CR_MMU_EN) == 0) {
	  return (TME_STP222X_IOMMU_TLB_MASH_FIXED
		  | TME_STP222X_IOMMU_TLB_MASH_UPA_31
		  | TME_STP222X_IOMMU_TLB_DATA_V
		  | TME_STP222X_IOMMU_TLB_TAG_W
		  | TME_STP222X_IOMMU_TLB_DATA_C
		  | TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB);
	}

	/* otherwise, this address is translated: */
      }

      /* otherwise, this address is local: */
      else {
	return (TME_STP222X_IOMMU_TLB_MASH_FIXED
		| TME_STP222X_IOMMU_TLB_DATA_V
		| TME_STP2220_IOMMU_TLB_DATA_LOCAL
		| TME_STP222X_IOMMU_TLB_TAG_W
		| TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB);
      }
    }
  }

  /* truncate the I/O address to 32 bits: */
  io_address_0_31 = io_address;

  /* get the size of the translatable I/O address space: */
  io_space_size_log2
    = (_tme_stp222x_iommu_tbw_size_log2(iommu_cr)
       + (10	/* log2(one TSB table size unit entry count) */
	  + TME_FIELD_MASK_EXTRACTU(iommu_cr, TME_STP222X_IOMMU_CR_TSB_SIZE_MASK)));

  /* if this I/O address is not within the translatable space: */
  /* "Hardware does not prevent illegal combinations [of TSB_SIZ and
     TBW_SIZ] from being programmed.  If an illegal combination is
     programmed into the IOMMU, all translation requests will be
     rejected as invalid." */
  if (io_space_size_log2 >= 32
      || (io_address_0_31
	  < (0
	     - (((tme_uint32_t) 1) << io_space_size_log2)))) {

    /* this translation request is invalid: */
    return (TME_STP222X_IOMMU_TLB_MASH_FIXED
	    | TME_STP222X_IOMMU_TLB_MASH_INVALID_REQUEST
	    | TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB);
  }

  /* convert the I/O address into something that can be easily
     compared to a TLB tag: */
  io_address_tag = io_address_0_31 / TME_STP222X_IOMMU_SIZE_8KB;

  /* loop over the TLB entries from most to least recently used: */
  tlb_i = stp222x->tme_stp222x_iommu_tlb_i_mru;
  tlb_count = TME_STP222X_IOMMU_TLB_SIZE;
  for (;;) {

    /* get this TLB entry tag: */
    tlb_tag = stp222x->tme_stp222x_iommu_tlb_tags[tlb_i];

    /* if we hit in this TLB entry: */
    if (((tlb_tag
	  ^ io_address_tag)
	 & _tme_stp222x_iommu_tlb_tag_vpn_mask(tlb_tag)) == 0) {

      /* this address hit in the TLB: */
      return (TME_STP222X_IOMMU_TLB_MASH_TLB_I(tlb_i)
	      | (tlb_tag
		 & (TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK
		    | TME_STP222X_IOMMU_TLB_TAG_ERROR
		    | TME_STP222X_IOMMU_TLB_TAG_W
		    | TME_STP222X_IOMMU_TLB_TAG_S
		    | TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK))
	      | (stp222x->tme_stp222x_iommu_tlb_data[tlb_i]
		 & (TME_STP222X_IOMMU_TLB_DATA_V
		    | TME_STP2220_IOMMU_TLB_DATA_LOCAL
		    | TME_STP222X_IOMMU_TLB_DATA_C)));
    }

    /* advance to the next TLB entry in the LRU list: */
    tlb_i = stp222x->tme_stp222x_iommu_lru_next(tlb_i);
    tlb_count--;
    assert ((tlb_count == 0) == (tlb_i == stp222x->tme_stp222x_iommu_tlb_i_mru));

    /* if we missed in all of the TLB entries: */
    if (__tme_predict_false(tlb_count == 0)) {

      /* this address missed in the TLB: */
      return (TME_STP222X_IOMMU_TLB_MASH_MISS
	      | TME_STP222X_IOMMU_TLB_TAG_SIZE_8KB);
    }
  }
  /* NOTREACHED */
}

/* this looks up an address in the IOMMU and returns a mash of TLB
   entry tag and data information and any slave connection index.  if
   there is a slave connection index, the address is converted to be
   connection-relative: */
static tme_uint32_t
_tme_stp222x_iommu_tlb_mash_slave(struct tme_bus_connection *io_conn_bus,
				  tme_bus_addr64_t *_io_to_slave_address,
				  unsigned int cycle_type,
				  tme_uint32_t *_slave_conn_index)
{
  struct tme_stp222x *stp222x;
  tme_uint32_t tlb_mash;
  unsigned long tlb_i;
  unsigned long tlb_i_next;
  unsigned long tlb_i_prev;
  tme_bus_addr64_t upa_address;
  tme_uint32_t page_size_m1;

  /* recover our data structure: */
  stp222x = io_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* look up this address in the IOMMU TLB: */
  tlb_mash = _tme_stp222x_iommu_tlb_mash(stp222x,
					 *_io_to_slave_address,
					 cycle_type);

  /* XXX FIXME - is the LRU only updated on valid translations? */

  /* if this address hit in the IOMMU TLB: */
  if ((tlb_mash
       & (TME_STP222X_IOMMU_TLB_MASH_FIXED
	  | TME_STP222X_IOMMU_TLB_MASH_MISS)) == 0) {

    /* if the hit TLB entry is not already the most recently used: */
    tlb_i = TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash);
    if (tlb_i != stp222x->tme_stp222x_iommu_tlb_i_mru) {

      /* remove this TLB entry from the LRU list: */
      tlb_i_next = stp222x->tme_stp222x_iommu_lru_next(tlb_i);
      tlb_i_prev = stp222x->tme_stp222x_iommu_lru_prev(tlb_i);
      stp222x->tme_stp222x_iommu_lru_next(tlb_i_prev) = tlb_i_next;
      stp222x->tme_stp222x_iommu_lru_prev(tlb_i_next) = tlb_i_prev;

      /* add this TLB entry in the LRU list before the old most
	 recently used TLB entry: */
      tlb_i_next = stp222x->tme_stp222x_iommu_tlb_i_mru;
      tlb_i_prev = stp222x->tme_stp222x_iommu_lru_prev(tlb_i_next);
      stp222x->tme_stp222x_iommu_lru_next(tlb_i_prev) = tlb_i;
      stp222x->tme_stp222x_iommu_lru_prev(tlb_i) = tlb_i_prev;
      stp222x->tme_stp222x_iommu_lru_next(tlb_i) = tlb_i_next;
      stp222x->tme_stp222x_iommu_lru_prev(tlb_i_next) = tlb_i;

      /* this TLB entry is now the most recently used: */
      stp222x->tme_stp222x_iommu_tlb_i_mru = tlb_i;
    }
  }

  /* if we don't have a valid translation for this address: */
  if ((tlb_mash & TME_STP222X_IOMMU_TLB_DATA_V) == 0) {

    /* return no connection index: */
    *_slave_conn_index = TME_STP222X_CONN_NULL;
    return (tlb_mash);
  }

  /* if this address is translated to itself, on its local bus: */
  if (tlb_mash & TME_STP2220_IOMMU_TLB_DATA_LOCAL) {

    /* look up this address on the connection's bus: */
    *_slave_conn_index
      = tme_stp222x_aspace_lookup(stp222x,
				  (TME_STP222X_IS_2220(stp222x)
				   ? TME_STP2220_ASPACE_SBUS
				   : ((io_conn_bus->tme_bus_connection.tme_connection_id 
				       & TME_STP2222_CONNID_BUS_WHICH) == 0)
				   ? TME_STP2222_ASPACE_PCI_MEMORY(0)
				   : TME_STP2222_ASPACE_PCI_MEMORY(1)),
				  _io_to_slave_address);
  }

  /* otherwise, this address is not local: */
  else {

    /* if this address is translated directly into a UPA address: */
    if (tlb_mash
	& (TME_STP222X_IOMMU_TLB_MASH_UPA_41
	   | TME_STP222X_IOMMU_TLB_MASH_UPA_31)) {

      /* convert the address into the UPA address: */
#if (TME_STP222X_IOMMU_TLB_MASH_UPA_41 / TME_STP222X_IOMMU_TLB_MASH_UPA_31) != (1 << (41 - 31))
#error "TME_STP222X_IOMMU_TLB_MASH_ values changed"
#endif
      upa_address = *_io_to_slave_address;
      upa_address
	&= (((tlb_mash
	      & (TME_STP222X_IOMMU_TLB_MASH_UPA_41
		 | TME_STP222X_IOMMU_TLB_MASH_UPA_31))
	     * ((((tme_bus_addr64_t) 1) << 31)
		/ TME_STP222X_IOMMU_TLB_MASH_UPA_31))
	    - 1);
    }

    /* otherwise, this address is translated through the TLB entry data
       into a UPA address: */
    else {

      /* get the page size mask: */
      page_size_m1 = _tme_stp222x_iommu_tlb_page_size(tlb_mash) - 1;

      /* convert the TLB entry data into the base UPA address: */
      upa_address
	= (stp222x->tme_stp222x_iommu_tlb_data[TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash)]
	   & TME_STP222X_IOMMU_TLB_DATA_PPN);
      upa_address *= TME_STP222X_IOMMU_SIZE_8KB;
      upa_address &= ~ (tme_bus_addr64_t) page_size_m1;

      /* add in the offset: */
      upa_address |= ((tme_bus_addr32_t) *_io_to_slave_address) & page_size_m1;

      /* return the UPA bus and address: */
      *_slave_conn_index = TME_STP222X_CONN_UPA;
      *_io_to_slave_address = upa_address;
    }
  }

  return (tlb_mash);
}

/* this handles an IOMMU bus cycle: */
void
tme_stp222x_iommu_cycle(struct tme_bus_connection *master_conn_bus,
			struct tme_bus_cycle *master_cycle,
			tme_uint32_t *_master_fast_cycle_types,
			struct tme_completion *master_completion)
{
  struct tme_stp222x *stp222x;
  struct tme_bus_tlb *tsb_tlb;
  unsigned long tlb_i_must_hit;
  tme_bus_addr64_t slave_address;
  unsigned int slave_conn_index;
  tme_uint32_t tlb_mash;
  tme_uint32_t iommu_cr;
  tme_uint32_t tsb_index;
  tme_bus_addr64_t tsb_address;
  struct tme_bus_connection *slave_conn_bus;
  struct tme_bus_connection *slave_conn_bus_other;
  struct tme_bus_tlb tsb_tlb_local;
#if TME_STP22XX_BUS_TRANSITION
  int rc;
#endif /* TME_STP22XX_BUS_TRANSITION */
  tme_uint64_t tte;
  unsigned long tlb_i;
  struct tme_stp222x_tlb_list *tlb_list;
  signed long tlb_list_i;
  struct tme_token *token;
  tme_uint32_t tlb_data;
  tme_uint32_t tlb_tag;
  tme_uint32_t tlb_tag_error;

  /* enter: */
  stp222x = tme_stp222x_enter_master_bus(master_conn_bus);

  /* start this cycle: */
  assert (stp222x->tme_stp222x_master_completion == NULL);
  stp222x->tme_stp222x_master_completion = &master_completion;

  /* start out using the stored TSB TLB: */
  tsb_tlb = &stp222x->tme_stp222x_iommu_tsb_tlb;

  /* start out allowing the address to miss in the IOMMU TLB: */
  tlb_i_must_hit = TME_STP222X_IOMMU_TLB_SIZE;

  /* loop forever: */
  for (;;) {

    /* if this cycle has been aborted: */
    if (stp222x->tme_stp222x_master_completion != &master_completion) {
      tme_stp222x_leave(stp222x);
      return;
    }

    /* translate this address: */
    slave_address = master_cycle->tme_bus_cycle_address;
    tlb_mash = _tme_stp222x_iommu_tlb_mash_slave(master_conn_bus,
						 &slave_address,
						 master_cycle->tme_bus_cycle_type,
						 &slave_conn_index);

    /* if the translation for this address is valid: */
    if (tlb_mash & TME_STP222X_IOMMU_TLB_DATA_V) {

      /* stop now: */
      break;
    }

    /* if the translation could not miss in the IOMMU TLB: */
    if (tlb_i_must_hit < TME_STP222X_IOMMU_TLB_SIZE) {

      /* the translation must have hit the specified IOMMU TLB entry,
	 even though that TLB entry is apparently invalid: */
      assert ((tlb_mash & TME_STP222X_IOMMU_TLB_MASH_FIXED) == 0
	      && TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash) == tlb_i_must_hit);
      
      /* stop now: */
      break;
    }

    /* get the IOMMU control register: */
    iommu_cr = stp222x->tme_stp222x_iommu_cr;

    /* get the index in the TSB table to read: */
    tsb_index = master_cycle->tme_bus_cycle_address;
    tsb_index >>= _tme_stp222x_iommu_tbw_size_log2(iommu_cr);
    tsb_index
      &= ((2 <<
	   (22
	    + TME_FIELD_MASK_EXTRACTU(iommu_cr, TME_STP222X_IOMMU_CR_TSB_SIZE_MASK)
	    - TME_STP222X_IOMMU_SIZE_LOG2_8KB))
	  - 1);

    /* get the address in the TSB table to read: */
    tsb_address = stp222x->tme_stp222x_iommu_tsb + (tsb_index * sizeof(tme_uint64_t));

    /* busy the TSB TLB: */
    tme_bus_tlb_busy(tsb_tlb);

    /* if the TSB TLB is invalid or doesn't apply: */
    if (tme_bus_tlb_is_invalid(tsb_tlb)
	|| tsb_address < (tme_bus_addr64_t) tsb_tlb->tme_bus_tlb_addr_first
	|| tsb_address > (tme_bus_addr64_t) tsb_tlb->tme_bus_tlb_addr_last) {

      /* force the TLB entry to be invalid, since after we clear its
	 invalid token, we may abort this cycle without storing it: */
      stp222x->tme_stp222x_iommu_tsb_tlb.tme_bus_tlb_addr_first = 1;
      stp222x->tme_stp222x_iommu_tsb_tlb.tme_bus_tlb_addr_last = 0;

      /* unbusy the TSB TLB for filling: */
      tme_bus_tlb_unbusy_fill(tsb_tlb);

      /* busy the UPA connection: */
      slave_conn_bus = tme_stp222x_slave_busy_bus(stp222x, TME_STP222X_CONN_UPA);

      /* leave: */
      tme_stp222x_leave(stp222x);

      /* fill the local TLB entry: */
      slave_conn_bus_other = (struct tme_bus_connection *) slave_conn_bus->tme_bus_connection.tme_connection_other;
      tsb_tlb_local.tme_bus_tlb_token = &stp222x->tme_stp222x_iommu_tsb_tlb_token;
#if TME_STP22XX_BUS_TRANSITION
      rc =
#endif /* TME_STP22XX_BUS_TRANSITION */
      (*slave_conn_bus_other->tme_bus_tlb_fill)
	(slave_conn_bus_other,
	 &tsb_tlb_local,
	 tsb_address,
	 TME_BUS_CYCLE_READ);
#if TME_STP22XX_BUS_TRANSITION
      assert (rc == TME_OK);
#endif /* TME_STP22XX_BUS_TRANSITION */

      /* reenter: */
      stp222x = tme_stp222x_enter_bus(master_conn_bus);

      /* unbusy the UPA connection: */
      tme_stp222x_slave_unbusy(stp222x);

      /* switch to using the newly filled local TLB entry: */
      tsb_tlb = &tsb_tlb_local;

      /* loop now, to make sure we're still running: */
      continue;
    }

    /* if we switched to the local TLB entry: */
    if (tsb_tlb == &tsb_tlb_local) {

      /* store the local TLB entry: */
      stp222x->tme_stp222x_iommu_tsb_tlb = tsb_tlb_local;
    }

    /* the TSB TLB must allow fast reading: */
    if (tsb_tlb->tme_bus_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF) {
      abort();
    }

    /* read the TTE: */
    tte = tme_memory_bus_read64(((_tme_const tme_shared tme_uint64_t *) 
				 (tsb_tlb->tme_bus_tlb_emulator_off_read
				  + tsb_address)),
				tsb_tlb->tme_bus_tlb_rwlock,
				sizeof(tme_uint64_t),
				sizeof(tme_uint64_t));
    tte = tme_betoh_u64(tte);

    /* unbusy the TSB TLB: */
    tme_bus_tlb_unbusy(tsb_tlb);

    /* if a TLB entry is already allocated to this address: */
    if ((tlb_mash & TME_STP222X_IOMMU_TLB_MASH_MISS) == 0) {

      /* replace the same TLB entry: */
      tlb_i = TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash);
    }

    /* otherwise, a TLB entry isn't already allocated to this address: */
    else {

      /* get the IOMMU control register: */
      iommu_cr = stp222x->tme_stp222x_iommu_cr;

      /* if the LRU is locked: */
      if (__tme_predict_false(iommu_cr & TME_STP222X_IOMMU_CR_LRU_LCKEN)) {

	/* replace the locked TLB entry: */
	tlb_i = TME_FIELD_MASK_EXTRACTU(iommu_cr, TME_STP222X_IOMMU_CR_LRU_LCKPTR);
      }

      /* otherwise, the LRU is not locked: */
      else {

	/* replace the least recently used TLB entry: */
	tlb_i = stp222x->tme_stp222x_iommu_lru_prev(stp222x->tme_stp222x_iommu_tlb_i_mru);
      }
    }

    /* invalidate all of the TLBs associated with this IOMMU TLB
       entry: */
    tlb_list = &stp222x->tme_stp222x_iommu_tlb_list[tlb_i];
    tlb_list_i = TME_STP222X_TLB_LIST_TOKENS_COUNT - 1;
    do {
      token = tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_i];
      if (token != NULL) {
	tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_i] = NULL;
	tme_token_invalidate(token);
      }
    } while (--tlb_list_i >= 0);

    /* make and store the TLB data: */
#if (TME_STP222X_IOMMU_TLB_DATA_PPN & 1) == 0
#error "TME_STP222X_IOMMU_TLB_DATA_PPN changed"
#endif
    tlb_data = TME_FIELD_MASK_EXTRACTU(tte, TME_STP222X_IOMMU_TTE_DATA_PA);
    if (tte & TME_STP222X_IOMMU_TTE_DATA_V) {
      tlb_data += TME_STP222X_IOMMU_TLB_DATA_V;
    }
    if (tte & TME_STP2220_IOMMU_TTE_LOCALBUS) {
      if (TME_STP222X_IS_2220(stp222x)) {
	tlb_data += TME_STP2220_IOMMU_TLB_DATA_LOCAL;
      }
    }
    if (tte & TME_STP222X_IOMMU_TTE_CACHEABLE) {
      tlb_data += TME_STP222X_IOMMU_TLB_DATA_C;
    }
    stp222x->tme_stp222x_iommu_tlb_data[tlb_i] = tlb_data;

    /* make and store the TLB tag: */
    tlb_tag = (tme_uint32_t) master_cycle->tme_bus_cycle_address;
    tlb_tag /= TME_STP222X_IOMMU_SIZE_8KB;
    if (tte & TME_STP222X_IOMMU_TTE_DATA_W) {
      tlb_tag += TME_STP222X_IOMMU_TLB_TAG_W;
    }
    if (tte & TME_STP222X_IOMMU_TTE_STREAM) {
      tlb_tag += TME_STP222X_IOMMU_TLB_TAG_S;
    }
    if (tte & TME_STP222X_IOMMU_TTE_DATA_SIZE_MASK) {
      tlb_tag += TME_STP222X_IOMMU_TLB_TAG_SIZE_MASK;
    }
    stp222x->tme_stp222x_iommu_tlb_tags[tlb_i] = tlb_tag;

    /* loop to look up this address in the IOMMU again.  this time, it
       must hit this entry: */
    tlb_i_must_hit = tlb_i;

#if TME_STP22XX_BUS_TRANSITION

    /* invalidate the last TLB that missed the IOMMU TLB (because we
       just filled the IOMMU TLB, we should get the master to refill
       its TLB): */
    if (stp222x->tme_stp222x_iommu_tlb_missed_token != NULL) {
      tme_token_invalidate(stp222x->tme_stp222x_iommu_tlb_missed_token);
      stp222x->tme_stp222x_iommu_tlb_missed_token = NULL;
    }

#endif /* TME_STP22XX_BUS_TRANSITION */
  }

  /* assume that there is no IOMMU error: */
  tlb_tag_error = !TME_STP222X_IOMMU_TLB_TAG_ERROR;

  /* if the address is not writable: */
  if (__tme_predict_false((tlb_mash & TME_STP222X_IOMMU_TLB_TAG_W) == 0)) {

    /* the master can't do fast writes: */
    *_master_fast_cycle_types &= ~TME_BUS_CYCLE_WRITE;

    /* if this is a write: */
    if (__tme_predict_false(master_cycle->tme_bus_cycle_type & TME_BUS_CYCLE_WRITE)) {

      /* there is an IOMMU protection error: */
      tlb_tag_error
	= (TME_STP222X_IOMMU_TLB_TAG_ERROR
	   + TME_STP222X_IOMMU_TLB_TAG_ERROR_PROTECTION);
    }
  }

  /* if we don't have a valid translation: */
  if (__tme_predict_false((tlb_mash & TME_STP222X_IOMMU_TLB_DATA_V) == 0)) {

    /* there is an IOMMU invalid error: */
    tlb_tag_error
      = (TME_STP222X_IOMMU_TLB_TAG_ERROR
	 + TME_STP222X_IOMMU_TLB_TAG_ERROR_INVALID);
  }

  /* if there is an IOMMU error: */
  if (tlb_tag_error != !TME_STP222X_IOMMU_TLB_TAG_ERROR) {

    /* if this isn't a fixed translation: */
    if ((tlb_mash & TME_STP222X_IOMMU_TLB_MASH_FIXED) == 0) {

      /* update the error in the tag for the TLB entry: */
      tlb_i = TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash);
      stp222x->tme_stp222x_iommu_tlb_tags[tlb_i]
	= ((stp222x->tme_stp222x_iommu_tlb_tags[tlb_i]
	    & ~(TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK
		| TME_STP222X_IOMMU_TLB_TAG_ERROR))
	   + tlb_tag_error);
    }

    /* update the error in the IOMMU control register: */
#if (TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK * (TME_STP222X_IOMMU_CR_XLT_ERR_MASK / TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK)) != TME_STP222X_IOMMU_CR_XLT_ERR_MASK
#error "IOMMU error masks changed"
#endif
#if (TME_STP222X_IOMMU_TLB_TAG_ERROR * (TME_STP222X_IOMMU_CR_XLT_ERR_MASK / TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK)) != TME_STP222X_IOMMU_CR_XLT_ERR
#error "IOMMU error flags changed"
#endif
    stp222x->tme_stp222x_iommu_cr
      = ((stp222x->tme_stp222x_iommu_cr
	  & ~(TME_STP222X_IOMMU_CR_XLT_ERR_MASK
	      | TME_STP222X_IOMMU_CR_XLT_ERR))
	 + (tlb_tag_error
	    * (TME_STP222X_IOMMU_CR_XLT_ERR_MASK
	       / TME_STP222X_IOMMU_TLB_TAG_ERROR_MASK)));

    /* force no connection: */
    slave_conn_index = TME_STP222X_CONN_NULL;
  }

  /* run the slave bus cycle: */
  master_cycle->tme_bus_cycle_address = slave_address;
  tme_stp22xx_slave_cycle(master_conn_bus,
			  slave_conn_index,
			  master_cycle,
			  _master_fast_cycle_types,
			  &master_completion);
 
  /* leave: */
  tme_stp222x_leave(stp222x);
}

/* this fills a TLB entry from the IOMMU: */
void
tme_stp222x_iommu_tlb_fill(struct tme_bus_connection *io_conn_bus,
			   struct tme_bus_tlb *tlb,
			   tme_bus_addr_t io_address_wider,
			   unsigned int cycle_type)
{
  struct tme_stp222x *stp222x;
  tme_bus_addr64_t slave_address;
  tme_uint32_t tlb_mash;
  tme_uint32_t slave_conn_index;
  struct tme_stp222x_tlb_list *tlb_list;
  unsigned int tlb_list_head;
  struct tme_token *token;
  struct tme_token *token_other;
  struct tme_bus_tlb tlb_mapping;
  tme_uint32_t page_size_m1;
  tme_bus_addr64_t io_address;

  /* enter: */
  stp222x = tme_stp222x_enter_bus(io_conn_bus);

  /* translate this address: */
  slave_address = io_address_wider;
  tlb_mash = _tme_stp222x_iommu_tlb_mash_slave(io_conn_bus,
					       &slave_address,
					       cycle_type,
					       &slave_conn_index);

  /* if the translation for this address is valid: */
  if (tlb_mash & TME_STP222X_IOMMU_TLB_DATA_V) {

    /* if this translation is fixed: */
    if (tlb_mash & TME_STP222X_IOMMU_TLB_MASH_FIXED) {

      /* track this TLB entry in the fixed list: */
      tlb_list = &stp222x->tme_stp222x_iommu_tlb_list_fixed;
    }

    /* otherwise, this translation is not fixed: */
    else {

      /* track this TLB entry in the list for the IOMMU TLB entry: */
      tlb_list = &stp222x->tme_stp222x_iommu_tlb_list[TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash)];
    }

    /* track this TLB entry: */
    tlb_list_head = tlb_list->tme_stp222x_tlb_list_head;
    token = tlb->tme_bus_tlb_token;
    token_other = tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_head];
    tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_head] = token;
    tlb_list->tme_stp222x_tlb_list_head = (tlb_list_head + 1) % TME_STP222X_TLB_LIST_TOKENS_COUNT;
    if (token_other != NULL
	&& token_other != token) {
      tme_token_invalidate(token_other);
    }
  }

  /* otherwise, the translation for this address is not valid: */
  else {

    /* we must not have any connection for this address: */
    assert (slave_conn_index == TME_STP222X_CONN_NULL);

#if TME_STP22XX_BUS_TRANSITION

    /* track the TLB for the most recent address that didn't have a
       valid translation, so we can invalidate it when the slow cycle
       happens (assuming the slow cycle will fill the IOMMU TLB with a
       valid translation, we want the slave to then refill its
       TLB): */
    token = tlb->tme_bus_tlb_token;
    token_other = stp222x->tme_stp222x_iommu_tlb_missed_token;
    stp222x->tme_stp222x_iommu_tlb_missed_token = token;
    if (token_other != NULL
	&& token_other != token) {
      tme_token_invalidate(token_other);
    }

#endif /* TME_STP22XX_BUS_TRANSITION */
  }

  /* fill the TLB entry: */
  tme_stp22xx_tlb_fill(io_conn_bus,
		       tlb,
		       slave_conn_index,
		       slave_address,
		       cycle_type);

  /* leave: */
  tme_stp222x_leave(stp222x);

  /* get the page size mask: */
  page_size_m1 = _tme_stp222x_iommu_tlb_page_size(tlb_mash) - 1;

  /* map the filled TLB entry: */
  io_address = ~ (tme_bus_addr64_t) page_size_m1;
  io_address &= io_address_wider;
  tlb_mapping.tme_bus_tlb_addr_first = io_address;
  io_address |= page_size_m1;
  tlb_mapping.tme_bus_tlb_addr_last = io_address;
#if TME_STP22XX_BUS_TRANSITION
  tlb_mapping.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
#endif /* TME_STP22XX_BUS_TRANSITION */
  tme_bus_tlb_map(tlb, slave_address, &tlb_mapping, io_address_wider);

  /* if this address is not writable: */
  if ((tlb_mash & TME_STP222X_IOMMU_TLB_TAG_W) == 0) {

    /* this TLB entry doesn't support writes: */
    tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
#if TME_STP22XX_BUS_TRANSITION
    tlb->tme_bus_tlb_cycles_ok &= ~TME_BUS_CYCLE_WRITE;
#endif /* TME_STP22XX_BUS_TRANSITION */
  }
}

/* the IOMMU register handler: */
void
tme_stp222x_iommu_regs(struct tme_stp222x *stp222x,
		       struct tme_stp222x_reg *reg)
{
  tme_uint32_t reggroup_index;
  const char *name;
  tme_uint32_t io_address;
  tme_uint32_t tlb_mash;
  tme_uint32_t tlb_i;
  struct tme_stp222x_tlb_list *tlb_list;
  signed long tlb_list_i;
  struct tme_token *token;

  /* get the register: */
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    /* dispatch on the register: */
    switch (reggroup_index) {

    case TME_STP222X_IOMMU_REGGROUP_INDEX_CR:
      stp222x->tme_stp222x_iommu_cr
	= (reg->tme_stp222x_reg_value
	   & ~TME_STP222X_IOMMU_CR_MBZ);
      name = "CR";
      break;

    case TME_STP222X_IOMMU_REGGROUP_INDEX_TSB:
      stp222x->tme_stp222x_iommu_tsb
	= (reg->tme_stp222x_reg_value
	   & ((((tme_uint64_t) 2) << 40)
	      - (1 << 13)));
      name = "TSB";
      break;

    case TME_STP222X_IOMMU_REGGROUP_INDEX_FLUSH:

      /* get the address: */
      io_address = 0 - (tme_uint32_t) TME_STP222X_IOMMU_SIZE_8KB;
      io_address &= reg->tme_stp222x_reg_value;

      /* translate this address: */
      tlb_mash
	= _tme_stp222x_iommu_tlb_mash(stp222x,
				      io_address,
				      TME_BUS_CYCLE_READ);

      /* if this address hit an IOMMU TLB entry: */
      if ((tlb_mash
	   & (TME_STP222X_IOMMU_TLB_MASH_FIXED
	      | TME_STP222X_IOMMU_TLB_MASH_MISS)) == 0) {

	/* invalidate this entry: */
	tlb_i = TME_STP222X_IOMMU_TLB_MASH_TLB_I_WHICH(tlb_mash);
	stp222x->tme_stp222x_iommu_tlb_data[tlb_i] &= ~TME_STP222X_IOMMU_TLB_DATA_V;

	/* invalidate all of the TLBs associated with this IOMMU TLB
	   entry: */
	tlb_list = &stp222x->tme_stp222x_iommu_tlb_list[tlb_i];
	tlb_list_i = TME_STP222X_TLB_LIST_TOKENS_COUNT - 1;
	do {
	  token = tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_i];
	  if (token != NULL) {
	    tlb_list->tme_stp222x_tlb_list_tokens[tlb_list_i] = NULL;
	    tme_token_invalidate(token);
	  }
	} while (--tlb_list_i >= 0);
      }
      name = "FLUSH";
      break;

    default:
      return;
    }
  }

  /* otherwise, this is a read: */
  else {

    /* dispatch on the register: */
    switch (reggroup_index) {
    case TME_STP222X_IOMMU_REGGROUP_INDEX_CR:
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_iommu_cr;
      name = "CR";
      break;
    case TME_STP222X_IOMMU_REGGROUP_INDEX_TSB:
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_iommu_tsb;
      name = "TSB";
      break;
/*  case TME_STP222X_IOMMU_REGGROUP_INDEX_FLUSH: */
    default:
      return;
    }
  }

  tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	  (TME_STP222X_LOG_HANDLE(stp222x),
	   _("IOMMU %s %s 0x%" TME_PRIx64),
	   name,
	   (reg->tme_stp222x_reg_write
	    ? "<-"
	    : "->"),
	   reg->tme_stp222x_reg_value));

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* the IOMMU diagnostic register handler: */
void
tme_stp222x_iommu_regs_diag(struct tme_stp222x *stp222x,
			    struct tme_stp222x_reg *reg)
{
  tme_uint32_t reggroup_0_3;
  tme_uint32_t reggroup_index;
  const char *name;
  signed long tlb_i;
  unsigned int lru_i;
  tme_uint32_t io_address;
  tme_uint32_t io_address_tag;
  tme_uint32_t compare;
  tme_uint32_t tlb_tag;

  /* XXX FIXME - what happens if diagnostics aren't enabled? */
  if ((stp222x->tme_stp222x_iommu_cr
       & TME_STP222X_IOMMU_CR_MMU_DE) == 0) {
    abort();
  }

  /* get the register: */
  reggroup_0_3 = TME_STP222X_REGGROUP_WHICH(reg->tme_stp222x_reg_address) & 0xf;
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    switch (reggroup_0_3) {
    case 0x4: /* STP2220 0x44, STP2222 0xa4 */
      switch (reggroup_index) {
      case TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_VA:

	/* get the address: */
	io_address = 0 - (tme_uint32_t) TME_STP222X_IOMMU_SIZE_8KB;
	io_address &= reg->tme_stp222x_reg_value;
	stp222x->tme_stp222x_iommu_va = io_address;

	/* convert the address into a VPN: */
	io_address_tag = io_address / TME_STP222X_IOMMU_SIZE_8KB;

	/* compare this address to all TLB tags: */
	compare = 0;
	tlb_i = TME_STP222X_IOMMU_TLB_SIZE - 1;
	do {
	  compare <<= 1;
	  tlb_tag = stp222x->tme_stp222x_iommu_tlb_tags[tlb_i];
	  if (((tlb_tag
		^ io_address_tag)
	       & _tme_stp222x_iommu_tlb_tag_vpn_mask(tlb_tag)) == 0) {
	    compare++;
	  }
	} while (--tlb_i >= 0);
	stp222x->tme_stp222x_iommu_compare = compare;
	name = "VA";
	break;

      case TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_COMPARE:
	reg->tme_stp222x_reg_completed = TRUE;
	return;
      default:
	return;
      }

    default: /* STP2220 0x45, STP2222 0xa5 */
      assert (reggroup_0_3 == 0x5);
      if (__tme_predict_false(reggroup_index < TME_STP222X_IOMMU_TLB_SIZE)) {
	return;
      }
      stp222x->tme_stp222x_iommu_tlb_tags[reggroup_index - TME_STP222X_IOMMU_TLB_SIZE]
	= reg->tme_stp222x_reg_value;
      name = "TLB_TAG";
      break;

    case 0x6: /* STP2220 0x46, STP2222 0xa6 */
      if (__tme_predict_false(reggroup_index >= TME_STP222X_IOMMU_TLB_SIZE)) {
	return;
      }
      stp222x->tme_stp222x_iommu_tlb_data[reggroup_index]
	= reg->tme_stp222x_reg_value;
      name = "TLB_DATA";
      break;
    }
  }

  /* otherwise, this is a read: */
  else {

    switch (reggroup_0_3) {
    case 0x4: /* STP2220 0x44, STP2222 0xa4 */
      switch (reggroup_index) {
      case TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_VA:
	reg->tme_stp222x_reg_value = stp222x->tme_stp222x_iommu_va;
	name = "VA";
	break;
      case TME_STP222X_IOMMU_REGGROUP_INDEX_DIAG4_COMPARE:
	reg->tme_stp222x_reg_value = stp222x->tme_stp222x_iommu_compare;
	name = "COMPARE";
	break;
      default:
	return;
      }

    default: /* STP2220 0x45, STP2222 0xa5 */
      assert (reggroup_0_3 == 0x5);
      if (__tme_predict_false(reggroup_index < TME_STP222X_IOMMU_TLB_SIZE)) {
	tlb_i = stp222x->tme_stp222x_iommu_tlb_i_mru;
	for (lru_i = reggroup_index; ++lru_i != TME_STP222X_IOMMU_TLB_SIZE; ) {
	  tlb_i = stp222x->tme_stp222x_iommu_lru_next(tlb_i);
	}
	reg->tme_stp222x_reg_value = tlb_i;
	name = "LRU";
      }
      else {
	reg->tme_stp222x_reg_value
	  = stp222x->tme_stp222x_iommu_tlb_tags[reggroup_index - TME_STP222X_IOMMU_TLB_SIZE];
	name = "TLB_TAG";
      }
      break;

    case 0x6: /* STP2220 0x46, STP2222 0xa6 */
      if (__tme_predict_false(reggroup_index >= TME_STP222X_IOMMU_TLB_SIZE)) {
	return;
      }
      reg->tme_stp222x_reg_value
	= stp222x->tme_stp222x_iommu_tlb_data[reggroup_index];
      name = "TLB_DATA";
      break;
    }
  }

  if (reggroup_0_3 == 0x4) {  /* STP2220 0x44, STP2222 0xa4 */
    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("IOMMU %s %s 0x%" TME_PRIx64),
	     name,
	     (reg->tme_stp222x_reg_write
	      ? "<-"
	      : "->"),
	     reg->tme_stp222x_reg_value));
  }
  else {
    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("IOMMU %s[%u] %s 0x%" TME_PRIx64),
	     name,
	     (reggroup_index % TME_STP222X_IOMMU_TLB_SIZE),
	     (reg->tme_stp222x_reg_write
	      ? "<-"
	      : "->"),
	     reg->tme_stp222x_reg_value));
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* this initializes the IOMMU: */
void
tme_stp222x_iommu_init(struct tme_stp222x *stp222x)
{
  unsigned long tlb_i;

  /* initialize the TSB TLB: */
  tme_token_init(&stp222x->tme_stp222x_iommu_tsb_tlb_token);
  stp222x->tme_stp222x_iommu_tsb_tlb.tme_bus_tlb_token = &stp222x->tme_stp222x_iommu_tsb_tlb_token;

  /* initialize the LRU list: */
  tlb_i = 0;
  stp222x->tme_stp222x_iommu_tlb_i_mru = tlb_i;
  do {
    stp222x->tme_stp222x_iommu_lru_prev(tlb_i) = (tlb_i - 1) % TME_STP222X_IOMMU_TLB_SIZE;
    stp222x->tme_stp222x_iommu_lru_next(tlb_i) = (tlb_i + 1) % TME_STP222X_IOMMU_TLB_SIZE;
    tlb_i = (tlb_i + 1) % TME_STP222X_IOMMU_TLB_SIZE;
  } while (tlb_i != 0);
}
