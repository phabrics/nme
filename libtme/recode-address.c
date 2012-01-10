/* $Id: recode-address.c,v 1.1 2010/02/07 14:07:25 fredette Exp $ */

/* libtme/recode-rws.c - generic support for recode reads and writes: */

/*
 * Copyright (c) 2008 Matt Fredette
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
_TME_RCSID("$Id: recode-address.c,v 1.1 2010/02/07 14:07:25 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this generic function checks an address type: */
void
tme_recode_address_type_check(const struct tme_recode_ic *ic,
			      const struct tme_recode_address_type *address_type)
{
  tme_uint32_t address_mask_tlb_index_one;

  /* the address size can't be bigger than the guest register size: */
  assert (address_type->tme_recode_address_type_size
	  <= ic->tme_recode_ic_reg_size);

  /* the minimum alignment must be a power of two: */
  /* NB: this is a value in bytes, not a TME_RECODE_SIZE_: */
  assert (address_type->tme_recode_address_type_align_min > 0
	  && (address_type->tme_recode_address_type_align_min
	      & (address_type->tme_recode_address_type_align_min - 1)) == 0);

  /* the TLB page size must be a power of two: */
  assert (ic->tme_recode_ic_tlb_page_size > 0
	  && (ic->tme_recode_ic_tlb_page_size
	      & (ic->tme_recode_ic_tlb_page_size - 1)) == 0);

  /* the TLB index address mask must be some number of consecutive
     one bits, all greater than or equal to the TLB page size: */
  address_mask_tlb_index_one
    = (address_type->tme_recode_address_type_mask_tlb_index
       & (0 - address_type->tme_recode_address_type_mask_tlb_index));
  assert (address_mask_tlb_index_one != 0
	  && (((address_type->tme_recode_address_type_mask_tlb_index
		+ address_mask_tlb_index_one)
	       & address_type->tme_recode_address_type_mask_tlb_index)
	      == 0));
  assert (address_mask_tlb_index_one
	  >= ic->tme_recode_ic_tlb_page_size);
}

/* this generic function returns zero if two address types are the
   same: */
int
tme_recode_address_type_compare(const struct tme_recode_ic *ic,
				const struct tme_recode_address_type *address_type,
				const struct tme_recode_address_type *address_type_other)
{
  return (TRUE

	  /* compare ic offset of the context register: */
	  && ((address_type_other->tme_recode_address_type_context_ic_offset < 0)
	      ? (address_type->tme_recode_address_type_context_ic_offset < 0)
	      : (address_type_other->tme_recode_address_type_context_ic_offset
		 == address_type->tme_recode_address_type_context_ic_offset))

	  /* compare context register size: */
	  && ((address_type_other->tme_recode_address_type_context_ic_offset < 0)
	      || (address_type_other->tme_recode_address_type_context_size
		  == address_type->tme_recode_address_type_context_size))

	  /* compare address size: */
	  && (address_type_other->tme_recode_address_type_size
	      == address_type->tme_recode_address_type_size)

	  /* compare address signedness: */
	  && ((address_type->tme_recode_address_type_size
	       == ic->tme_recode_ic_reg_size)
	      || (!address_type_other->tme_recode_address_type_signed
		  == !address_type->tme_recode_address_type_signed))

	  /* compare minimum alignment: */
	  && (address_type_other->tme_recode_address_type_align_min
	      == address_type->tme_recode_address_type_align_min)

	  /* compare the fixed TLB flags: */
	  && (address_type_other->tme_recode_address_type_tlb_flags
	      == address_type->tme_recode_address_type_tlb_flags)

	  /* compare ic offset of the TLB flags mask: */
	  && (address_type_other->tme_recode_address_type_tlb_flags_ic_offset
	      == address_type->tme_recode_address_type_tlb_flags_ic_offset)

	  /* compare TLB index mask: */
	  && (address_type_other->tme_recode_address_type_mask_tlb_index
	      == address_type->tme_recode_address_type_mask_tlb_index)

	  /* compare ic offset of TLB zero: */
	  && (address_type_other->tme_recode_address_type_tlb0_ic_offset
	      == address_type->tme_recode_address_type_tlb0_ic_offset)

	  );
}

/* this returns the recode TLB type for an address type: */
void
tme_recode_address_type_tlb_type(const struct tme_recode_ic *ic,
				 const struct tme_recode_address_type *address_type,
				 struct tme_recode_tlb_type *tlb_type)
{
  unsigned long tlb_offset_memory;
  unsigned long tlb_offset_page;
  unsigned long tlb_offset_flags;
  unsigned long tlb_offset_token;
  unsigned long tlb_offset_context;
  unsigned long tlb_sizeof;

  /* get information about this guest's recode TLB entries: */
#define _TME_RECODE_TLB_INFO_Cn_Am_KEY(csize, asize) (((csize) * (TME_RECODE_SIZE_GUEST_MAX + 1)) + (asize))
#define _TME_RECODE_TLB_INFO_C0_Am_KEY(asize) _TME_RECODE_TLB_INFO_Cn_Am_KEY(TME_RECODE_SIZE_GUEST_MAX + 1, asize)
  switch (address_type->tme_recode_address_type_context_ic_offset < 0
	  ? _TME_RECODE_TLB_INFO_C0_Am_KEY(ic->tme_recode_ic_reg_size)
	  : _TME_RECODE_TLB_INFO_Cn_Am_KEY(address_type->tme_recode_address_type_context_size,
					   ic->tme_recode_ic_reg_size)) {
  default: abort();
#define _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,field)			\
  (((tme_uint8_t *)								\
    &(((struct _TME_CONCAT4(tme_recode_tlb_c,csize,_a,asize) *) 0)		\
      ->_TME_CONCAT5(tme_recode_tlb_c,csize,_a,asize,field)))			\
   - (tme_uint8_t *) 0)
#define _TME_RECODE_TLB_INFO(csize,asize)					\
  do {										\
    tlb_offset_memory = _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,_memory);	\
    tlb_offset_page = _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,_page);		\
    tlb_offset_flags = _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,_flags);	\
    tlb_offset_token = _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,_token);	\
    tlb_sizeof = sizeof(struct _TME_CONCAT4(tme_recode_tlb_c,csize,_a,asize));	\
  } while (/* CONSTCOND */ 0)
#define _TME_RECODE_TLB_INFO_Cn_Am(csize,asize)					\
  do {										\
    _TME_RECODE_TLB_INFO(csize,asize);						\
    tlb_offset_context = _TME_RECODE_TLB_INFO_OFFSETOF(csize,asize,_context);	\
  } while (/* CONSTCOND */ 0)
#define _TME_RECODE_TLB_INFO_Am(asize)						\
  do {										\
    _TME_RECODE_TLB_INFO(0,asize);						\
    tlb_offset_context = 0;							\
  } while (/* CONSTCOND */ 0)

#ifdef TME_RECODE_SIZE_128
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128
  case _TME_RECODE_TLB_INFO_Cn_Am_KEY(TME_RECODE_SIZE_16, TME_RECODE_SIZE_128):
    _TME_RECODE_TLB_INFO_Cn_Am(16,128);
    break;
#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128 */
#endif /* TME_RECODE_SIZE_128 */
#ifdef TME_RECODE_SIZE_64
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64
  case _TME_RECODE_TLB_INFO_Cn_Am_KEY(TME_RECODE_SIZE_16, TME_RECODE_SIZE_64):
    _TME_RECODE_TLB_INFO_Cn_Am(16,64);
    break;
#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64 */
#endif /* TME_RECODE_SIZE_64 */
  case _TME_RECODE_TLB_INFO_Cn_Am_KEY(TME_RECODE_SIZE_16, TME_RECODE_SIZE_32):
    _TME_RECODE_TLB_INFO_Cn_Am(16,32);
    break;
  }

  /* return the information: */
  tlb_type->tme_recode_tlb_type_offset_memory = tlb_offset_memory;
  tlb_type->tme_recode_tlb_type_offset_page = tlb_offset_page;
  tlb_type->tme_recode_tlb_type_offset_flags = tlb_offset_flags;
  tlb_type->tme_recode_tlb_type_offset_token = tlb_offset_token;
  tlb_type->tme_recode_tlb_type_offset_context = tlb_offset_context;
  tlb_type->tme_recode_tlb_type_sizeof = tlb_sizeof;
}

#endif /* TME_HAVE_RECODE */
