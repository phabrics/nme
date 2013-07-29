/* $Id: stp222x-stc.c,v 1.4 2010/06/05 18:59:29 fredette Exp $ */

/* ic/stp222x-stc.c - emulation of the STC of the UPA to SBus
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
_TME_RCSID("$Id: stp222x-stc.c,v 1.4 2010/06/05 18:59:29 fredette Exp $");

/* includes: */
#include "stp222x-impl.h"

/* macros: */

/* streaming buffer register offsets: */
#define TME_STP222X_STC_REGGROUP_INDEX_CR		TME_STP222X_REGGROUP_INDEX(0x00)
#define TME_STP222X_STC_REGGROUP_INDEX_PGFLUSH		TME_STP222X_REGGROUP_INDEX(0x08)
#define TME_STP222X_STC_REGGROUP_INDEX_FLUSHSYNC	TME_STP222X_REGGROUP_INDEX(0x10)

/* an STC control register: */
#define TME_STP222X_STC_CR_LPTR				((2 << 7) - (1 << 4))
#define TME_STP222X_STC_CR_LE				(1 << 3)
#define TME_STP222X_STC_CR_RR_DIS			(1 << 2)
#define TME_STP222X_STC_CR_DE				(1 << 1)
#define TME_STP222X_STC_CR_SB_EN			(1 << 0)

/* the size of the stp222x flush blocks: */
#define TME_STP2220_STC_FLUSHSYNC_SIZE			((tme_uint64_t) 4)
#define TME_STP2222_STC_FLUSHSYNC_SIZE			((tme_uint64_t) 64)

/* the number of entries in each streaming buffer: */
#define TME_STP222X_STC_ENTRY_COUNT			(16)

/* this flushes a streaming cache: */
int
tme_stp222x_stc_flush(struct tme_stp222x *stp222x)
{
  struct tme_stp222x_stc *stc;
  tme_uint64_t address;
  struct tme_bus_tlb *tlb;
  struct tme_bus_connection *slave_conn_bus;
  struct tme_bus_connection *slave_conn_bus_other;
#if TME_STP22XX_BUS_TRANSITION
  int rc;
#endif /* TME_STP22XX_BUS_TRANSITION */
  tme_shared tme_uint8_t *memory;

  stc = &stp222x->tme_stp222x_stcs[0];
  if (!stc->tme_stp222x_stc_pgflush) {
    stc = &stp222x->tme_stp222x_stcs[1];
    if (!stc->tme_stp222x_stc_pgflush) {
      return (FALSE);
    }
  }

  /* get the flushsync address: */
  address
    = (stc->tme_stp222x_stc_flushsync
       & (TME_STP222X_IS_2220(stp222x)
	  ? ((((tme_uint64_t) 2) << 40)
	     - TME_STP2220_STC_FLUSHSYNC_SIZE)
	  : ((((tme_uint64_t) 2) << 40)
	     - TME_STP2222_STC_FLUSHSYNC_SIZE)));

  /* busy the flushsync TLB: */
  tlb = &stc->tme_stp222x_stc_flushsync_tlb;
  tme_bus_tlb_busy(tlb);

  /* if the flushsync TLB is invalid or doesn't apply: */
  if (tme_bus_tlb_is_invalid(tlb)
      || address < (tme_bus_addr64_t) tlb->tme_bus_tlb_addr_first
      || address > (tme_bus_addr64_t) tlb->tme_bus_tlb_addr_last) {

    /* unbusy the flushsync TLB for filling: */
    tme_bus_tlb_unbusy_fill(tlb);

    /* busy the UPA connection: */
    slave_conn_bus = tme_stp222x_slave_busy_bus(stp222x, TME_STP222X_CONN_UPA);

    /* leave: */
    tme_stp222x_leave(stp222x);

    /* fill the TLB entry: */
    slave_conn_bus_other = (struct tme_bus_connection *) slave_conn_bus->tme_bus_connection.tme_connection_other;
#if TME_STP22XX_BUS_TRANSITION
     rc =
#endif /* TME_STP22XX_BUS_TRANSITION */
     (*slave_conn_bus_other->tme_bus_tlb_fill)
       (slave_conn_bus_other,
	tlb,
	address,
	TME_BUS_CYCLE_WRITE);
#if TME_STP22XX_BUS_TRANSITION
     assert (rc == TME_OK);
#endif /* TME_STP22XX_BUS_TRANSITION */

     /* reenter: */
     stp222x = tme_stp222x_enter_bus(slave_conn_bus);

     /* unbusy the UPA connection: */
     tme_stp222x_slave_unbusy(stp222x);

     return (TRUE);
  }

  /* the flushsync TLB must allow fast writing of the entire block: */
  memory = tlb->tme_bus_tlb_emulator_off_write;
  assert (memory != TME_EMULATOR_OFF_UNDEF
	  && ((address
	       + (TME_STP222X_IS_2220(stp222x)
		  ? TME_STP2220_STC_FLUSHSYNC_SIZE
		  : TME_STP2222_STC_FLUSHSYNC_SIZE)
	       - 1)
	      <= (tme_bus_addr64_t) tlb->tme_bus_tlb_addr_last));

  /* write the flush doublewords: */
  memory += address;

  /* if this is an stp2220: */
  if (TME_STP222X_IS_2220(stp222x)) {

    /* the stp2220 only writes four bytes: */
    tme_memory_atomic_write32(((tme_shared tme_uint32_t *) memory) + 0,
			      tme_htobe_u32(1),
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint32_t));
  }

  /* otherwise, this is an stp2222: */
  else {

    /* the stp2222 writes 64 bytes: */

    /* XXX FIXME - all of these doublewords are actually written as
       one atomic transaction.  the best we can do is either 64- or
       128-bit atomic writes: */
#ifdef tme_memory_atomic_write128
    tme_memory_atomic_write128(((tme_shared tme_uint128_t *) memory) + 0,
			       tme_htobe_u128(((tme_uint128_t) 1) << (128 - 32)),
			       tlb->tme_bus_tlb_rwlock,
			       sizeof(tme_uint128_t));
    tme_memory_atomic_write128(((tme_shared tme_uint128_t *) memory) + 1,
			       0,
			       tlb->tme_bus_tlb_rwlock,
			       sizeof(tme_uint128_t));
    tme_memory_atomic_write128(((tme_shared tme_uint128_t *) memory) + 2,
			       0,
			       tlb->tme_bus_tlb_rwlock,
			       sizeof(tme_uint128_t));
    tme_memory_atomic_write128(((tme_shared tme_uint128_t *) memory) + 3,
			       0,
			       tlb->tme_bus_tlb_rwlock,
			       sizeof(tme_uint128_t));
#else  /* !tme_memory_atomic_write128 */
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 0,
			      tme_htobe_u64(((tme_uint64_t) 1) << (64 - 32)),
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 1,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 2,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 3,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 4,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 5,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 6,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
    tme_memory_atomic_write64(((tme_shared tme_uint64_t *) memory) + 7,
			      0,
			      tlb->tme_bus_tlb_rwlock,
			      sizeof(tme_uint64_t));
#endif /* !tme_memory_atomic_write128 */
  }

  /* unbusy the flushsync TLB: */
  tme_bus_tlb_unbusy(tlb);

  /* the flush has been done: */
  stc->tme_stp222x_stc_pgflush = FALSE;

  return (TRUE);
}

/* the STC register handler: */
void
tme_stp222x_stc_regs(struct tme_stp222x *stp222x,
		     unsigned long stc_i,
		     struct tme_stp222x_reg *reg)
{
  struct tme_stp222x_stc *stc;
  tme_uint32_t reggroup_index;
  const char *name;

  /* get the streaming cache: */
  stc = &stp222x->tme_stp222x_stcs[stc_i];

  /* get the register: */
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    /* dispatch on the register: */
    switch (reggroup_index) {

    case TME_STP222X_STC_REGGROUP_INDEX_CR:
      stc->tme_stp222x_stc_cr
	= (reg->tme_stp222x_reg_value
	   & (TME_STP222X_STC_CR_LPTR
	      | TME_STP222X_STC_CR_LE
	      | TME_STP222X_STC_CR_RR_DIS
	      | TME_STP222X_STC_CR_DE
	      | TME_STP222X_STC_CR_SB_EN));
      name = "CR";
      break;

    case TME_STP222X_STC_REGGROUP_INDEX_PGFLUSH:
      name = "PGFLUSH";
      break;

    case TME_STP222X_STC_REGGROUP_INDEX_FLUSHSYNC:
      stc->tme_stp222x_stc_flushsync = reg->tme_stp222x_reg_value;
      stc->tme_stp222x_stc_pgflush = TRUE;
      name = "FLUSHSYNC";
      break;

    default:
      return;
    }
  }

  /* otherwise, this is a read: */
  else {

    /* dispatch on the register: */
    switch (reggroup_index) {
    case TME_STP222X_STC_REGGROUP_INDEX_CR:
      reg->tme_stp222x_reg_value = stc->tme_stp222x_stc_cr;
      name = "CR";
      break;
    case TME_STP222X_STC_REGGROUP_INDEX_PGFLUSH:
    case TME_STP222X_STC_REGGROUP_INDEX_FLUSHSYNC:
      reg->tme_stp222x_reg_completed = TRUE;
      return;
    default:
      return;
    }
  }

  tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	  (TME_STP222X_LOG_HANDLE(stp222x),
	   _("STC%lu %s %s 0x%" TME_PRIx64),
	   stc_i,
	   name,
	   (reg->tme_stp222x_reg_write
	    ? "<-"
	    : "->"),
	   reg->tme_stp222x_reg_value));

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* the STC diagnostic register handler: */
void
tme_stp222x_stc_regs_diag(struct tme_stp222x *stp222x,
			  unsigned long stc_i,
			  struct tme_stp222x_reg *reg)
{
  struct tme_stp222x_stc *stc;
  tme_uint32_t reggroup_0_3;
  tme_uint32_t reggroup_index;

  /* get the streaming cache: */
  stc = &stp222x->tme_stp222x_stcs[stc_i];

  /* get the register: */
  reggroup_0_3 = TME_STP222X_REGGROUP_WHICH(reg->tme_stp222x_reg_address) & 0xf;
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    abort();
  }

  /* otherwise, this is a read: */
  else {

    switch (reggroup_0_3) {
    case 0x8: /* STP2220 0x58, STP2222 0xb4 or 0xc4 */
      if (__tme_predict_false(reggroup_index >= TME_STP222X_STC_ENTRY_COUNT)) {
	abort();
      }
      reg->tme_stp222x_reg_value = 0;
      break;
    default:
      abort();
    }
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* this initializes a streaming cache: */
void
tme_stp222x_stc_init(struct tme_stp222x_stc *stc)
{

  /* initialize the flushsync TLB: */
  tme_token_init(&stc->tme_stp222x_stc_flushsync_tlb_token);
  stc->tme_stp222x_stc_flushsync_tlb.tme_bus_tlb_token = &stc->tme_stp222x_stc_flushsync_tlb_token;
}
