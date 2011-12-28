/* $Id: m68k-insns.c,v 1.16 2007/08/25 22:05:02 fredette Exp $ */

/* ic/m68k/m68k-insns.c - m68k instruction functions: */

/*
 * Copyright (c) 2002, 2003 Matt Fredette
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

/* includes: */
#include "m68k-impl.h"

_TME_RCSID("$Id: m68k-insns.c,v 1.16 2007/08/25 22:05:02 fredette Exp $");

#define TME_M68K_STD_FLAGS \
do { \
  ic->tme_m68k_ireg_ccr = ((ic->tme_m68k_ireg_ccr \
			     & TME_M68K_FLAG_X) \
			    | (res < 0 ? TME_M68K_FLAG_N : 0) \
			    | (res == 0 ? TME_M68K_FLAG_Z : 0)); \
} while (/* CONSTCOND */ 0)

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_exg)
{
  tme_uint32_t tmp;
  tmp = TME_M68K_INSN_OP0(tme_uint32_t);
  TME_M68K_INSN_OP0(tme_uint32_t) = TME_M68K_INSN_OP1(tme_uint32_t);
  TME_M68K_INSN_OP1(tme_uint32_t) = tmp;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_extw)
{
  tme_int16_t res;
  res = TME_EXT_S8_S16((tme_int8_t) TME_M68K_INSN_OP1(tme_int16_t));
  TME_M68K_INSN_OP1(tme_int16_t) = res;
  TME_M68K_STD_FLAGS;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_extl)
{
  tme_int32_t res;
  res = TME_EXT_S16_S32((tme_int16_t) TME_M68K_INSN_OP1(tme_int32_t));
  TME_M68K_INSN_OP1(tme_int32_t) = res;
  TME_M68K_STD_FLAGS;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_extbl)
{
  tme_int32_t res;
  res = TME_EXT_S8_S32((tme_int8_t) TME_M68K_INSN_OP1(tme_int32_t));
  TME_M68K_INSN_OP1(tme_int32_t) = res;
  TME_M68K_STD_FLAGS;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_lea)
{
  TME_M68K_INSN_OP0(tme_uint32_t) = TME_M68K_INSN_OP1(tme_uint32_t);
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_move_from_ccr)
{
  TME_M68K_INSN_OP1(tme_uint16_t) = ic->tme_m68k_ireg_ccr;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_move_from_sr)
{
  TME_M68K_INSN_PRIV;
  TME_M68K_INSN_OP1(tme_uint16_t) = ic->tme_m68k_ireg_sr;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
/* this is the 68000 version, which isn't privileged: */
TME_M68K_INSN(tme_m68k_move_from_sr0)
{
  TME_M68K_INSN_OP1(tme_uint16_t) = ic->tme_m68k_ireg_sr;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_swap)
{
  tme_uint32_t tmp;
  tme_int32_t res;
  tmp = TME_M68K_INSN_OP1(tme_uint32_t);
  tmp = (TME_FIELD_EXTRACTU(tmp, 0, 16) << 16) | TME_FIELD_EXTRACTU(tmp, 16, 16);
  TME_M68K_INSN_OP1(tme_uint32_t) = tmp;
  res = (tme_int32_t) tmp;
  TME_M68K_STD_FLAGS;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_nop)
{
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_scc)
{
  TME_M68K_INSN_OP1(tme_uint8_t) =
    (TME_M68K_COND_TRUE(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 8, 4))
     ? 0xff
     : 0x00);
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_dbcc)
{
  if (!TME_M68K_COND_TRUE(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 8, 4))) {
    if (--TME_M68K_INSN_OP0(tme_int16_t) != -1) {
      TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_pc
			   + 2
			   + TME_EXT_S16_U32(TME_M68K_INSN_OP1(tme_int16_t)));
    }
  }
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
static void
_tme_m68k_bcc(struct tme_m68k *ic, tme_int32_t disp)
{
  if (TME_M68K_COND_TRUE(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 8, 4))) {
    TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_pc + 2 + disp);
  }
  TME_M68K_INSN_OK;
}
TME_M68K_INSN(tme_m68k_bcc)
{
  _tme_m68k_bcc(ic, TME_EXT_S8_S32((tme_int8_t) TME_M68K_INSN_OPCODE));
}
TME_M68K_INSN(tme_m68k_bccl)
{
  _tme_m68k_bcc(ic, TME_M68K_INSN_OP0(tme_int32_t));
}
    
/* this can fault: */
static void
_tme_m68k_bsr(struct tme_m68k *ic, tme_int32_t disp)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_push32(ic, ic->tme_m68k_ireg_pc_next);
  /* while the above push can fault, we don't have to worry about
     restarting here, because after this point, nothing can fault for
     the remainder of this instruction (the executor makes no stores
     on behalf of a bsr): */
  tme_m68k_log(ic, 250, TME_OK,
	       (TME_M68K_LOG_HANDLE(ic),
		_("bsr 0x%08x"),
		(ic->tme_m68k_ireg_pc + 2 + disp)));
  TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_pc + 2 + disp);
  TME_M68K_INSN_OK;
}
TME_M68K_INSN(tme_m68k_bsr)
{
  _tme_m68k_bsr(ic, TME_EXT_S8_S32((tme_int8_t) TME_M68K_INSN_OPCODE));
}
TME_M68K_INSN(tme_m68k_bsrl)
{
  _tme_m68k_bsr(ic, TME_M68K_INSN_OP0(tme_int32_t));
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_pea)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_push32(ic, TME_M68K_INSN_OP1(tme_uint32_t));
  /* while the above push can fault, we don't have to worry about
     restarting here, because after this point, nothing can fault for
     the remainder of this instruction (the executor makes no stores
     on behalf of a pea): */
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_bkpt)
{
  TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_illegal)
{
  TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_tas)
{
  struct tme_m68k_rmw rmw;
  struct tme_m68k_tlb *tlb;
  tme_shared tme_int8_t *mem;
  tme_uint8_t flags;
  tme_int8_t value;
  tme_int8_t value_written;
  tme_int8_t value_verify;

  /* start the read/modify/write cycle: */
  rmw.tme_m68k_rmw_addresses[0] = ic->_tme_m68k_ea_address;
  rmw.tme_m68k_rmw_address_count = 1;
  rmw.tme_m68k_rmw_size = sizeof(tme_uint8_t);
  if (tme_m68k_rmw_start(ic,
			 &rmw))
    TME_M68K_INSN_OK;

  /* if this TLB entry allows fast reading: */
  if (!rmw.tme_m68k_rmw_slow_reads[0]) {

    /* get this TLB entry: */
    tlb = rmw.tme_m68k_rmw_tlbs[0];

    /* make the emulator memory pointer: */
    mem = (tme_shared tme_int8_t *) (tlb->tme_m68k_tlb_emulator_off_read + ic->_tme_m68k_ea_address);

    /* read memory: */
    value
      = tme_memory_atomic_read8((tme_shared tme_uint8_t *) mem,
				tlb->tme_m68k_tlb_bus_rwlock,
				sizeof(tme_uint8_t));

    /* spin the tas in a compare-and-exchange loop: */
    for (;;) {

      /* make the value to write: */
      value_written = value | 0x80;

      /* try the compare-and-exchange: */
      value_verify
	= tme_memory_atomic_cx8((tme_shared tme_uint8_t *) mem,
				value,
				value_written,
				tlb->tme_m68k_tlb_bus_rwlock,
				sizeof(tme_uint8_t));

      /* if the compare-and-exchange failed: */
      if (__tme_predict_false(value_verify != value)) {
	
	/* loop with the new value read from the memory: */
	value = value_verify;
	continue;
      }

      /* stop now: */
      break;
    }

    /* store the value read: */
    ic->tme_m68k_ireg_memx8 = value;
  }
      
  /* the modify part of the read/modify/write cycle: */
  value = ic->tme_m68k_ireg_memx8;
  flags = ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X;
  if (value < 0) flags |= TME_M68K_FLAG_N;
  if (value == 0) flags |= TME_M68K_FLAG_Z;
  ic->tme_m68k_ireg_ccr = flags;
  ic->tme_m68k_ireg_memx8 |= 0x80;

  /* finish the read/modify/write cycle: */
  tme_m68k_rmw_finish(ic,
		      &rmw,
		      TRUE);

  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_tas_r)
{
  tme_uint8_t flags;
  tme_int8_t value;
  value = TME_M68K_INSN_OP1(tme_int8_t);
  flags = ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X;
  if (value < 0) flags |= TME_M68K_FLAG_N;
  if (value == 0) flags |= TME_M68K_FLAG_Z;
  ic->tme_m68k_ireg_ccr = flags;
  TME_M68K_INSN_OP1(tme_int8_t) = value | 0x80;
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_move_usp)
{
  TME_M68K_INSN_PRIV;
  if (TME_M68K_INSN_OPCODE & TME_BIT(3)) {
    TME_M68K_INSN_OP1(tme_uint32_t) = ic->tme_m68k_ireg_usp;
  }
  else {
    ic->tme_m68k_ireg_usp = TME_M68K_INSN_OP1(tme_uint32_t);
  }
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_trap)
{
  ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
  TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_TRAP_0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 4)));
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_trapv)
{
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_V) {
    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_TRAP));
  }
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_link)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_push32(ic, TME_M68K_INSN_OP1(tme_uint32_t));
  TME_M68K_INSN_OP1(tme_uint32_t) = ic->tme_m68k_ireg_a7;
  ic->tme_m68k_ireg_a7 += TME_M68K_INSN_OP0(tme_uint32_t);
  TME_M68K_INSN_OK;
}
  
/* this can fault: */
TME_M68K_INSN(tme_m68k_unlk)
{
  TME_M68K_INSN_CANFAULT;
  ic->tme_m68k_ireg_a7 = TME_M68K_INSN_OP0(tme_uint32_t);
  tme_m68k_pop32(ic, &TME_M68K_INSN_OP0(tme_uint32_t));
  TME_M68K_INSN_OK;
}
  
/* this cannot fault: */
TME_M68K_INSN(tme_m68k_movec)
{
  int ireg;
  tme_uint32_t *creg;
  tme_uint32_t mask;
  int illegal;
  TME_M68K_INSN_PRIV;
  /* in case we're reading the msp or isp and we're on that stack,
     this flushes %a7 to that register: */
  tme_m68k_change_sr(ic, ic->tme_m68k_ireg_sr);
  ireg = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 4);
  mask = 0xffffffff;
  illegal = FALSE;
  switch (TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 0, 12)) {
  case 0x000: creg = &ic->tme_m68k_ireg_sfc; mask = TME_M68K_FC_7; break;
  case 0x001: creg = &ic->tme_m68k_ireg_dfc; mask = TME_M68K_FC_7; break;
  case 0x002: creg = &ic->tme_m68k_ireg_cacr; mask = 0x3; illegal = (ic->tme_m68k_type < TME_M68K_M68020); break;
  case 0x800: creg = &ic->tme_m68k_ireg_usp; break;
  case 0x801: creg = &ic->tme_m68k_ireg_vbr; break;
  case 0x802: creg = &ic->tme_m68k_ireg_caar; illegal = (ic->tme_m68k_type != TME_M68K_M68020); break;
  case 0x803: creg = &ic->tme_m68k_ireg_msp; illegal = (ic->tme_m68k_type < TME_M68K_M68020); break;
  case 0x804: creg = &ic->tme_m68k_ireg_isp; illegal = (ic->tme_m68k_type < TME_M68K_M68020); break;
  default: illegal = TRUE; creg = NULL;
  }
  if (illegal) {
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }
  if (TME_M68K_INSN_OPCODE & TME_BIT(0)) {
    *creg = ic->tme_m68k_ireg_uint32(ireg) & mask;
  }
  else {
    ic->tme_m68k_ireg_uint32(ireg) = *creg;
  }
  /* in case we're writing the msp or isp and we're on that stack,
     this flushes that register to %a7: */
  tme_m68k_change_sr(ic, ic->tme_m68k_ireg_sr);
  TME_M68K_INSN_OK;    
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_reset)
{
  struct tme_bus_connection *conn_bus;
  int rc;

  TME_M68K_INSN_PRIV;
  
  /* get the bus connection: */
  conn_bus = &ic->_tme_m68k_bus_connection->tme_m68k_bus_connection;

  /* unlock for the callout: */
  tme_m68k_callout_unlock(ic);

  /* assert the RESET line: */
  rc = (*conn_bus->tme_bus_signal)
    (conn_bus,
     (TME_BUS_SIGNAL_RESET
      | TME_BUS_SIGNAL_LEVEL_ASSERTED));
  assert (rc == TME_OK);

  /* XXX RESET is supposed to be asserted for 512 clocks, 
     so a sleep is needed here: */

  /* negate the RESET line: */
  rc = (*conn_bus->tme_bus_signal)
    (conn_bus,
     (TME_BUS_SIGNAL_RESET
      | TME_BUS_SIGNAL_LEVEL_NEGATED));
  assert (rc == TME_OK);

  /* relock after the callout: */
  tme_m68k_callout_relock(ic);
  
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_rtd)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_pop32(ic, &ic->tme_m68k_ireg_memx32);
  /* while the above pop can fault, we don't have to worry about
     restarting here, because after this point, nothing can fault for
     the remainder of this instruction (the executor makes no stores
     on behalf of an rtd): */
  ic->tme_m68k_ireg_a7 += TME_M68K_INSN_OP0(tme_uint32_t);
  TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_memx32);
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_rtr)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_pop16(ic, &ic->tme_m68k_ireg_memx16);
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->tme_m68k_ireg_ccr = ic->tme_m68k_ireg_memx8 & TME_M68K_FLAG_CCR;
  }
  tme_m68k_pop32(ic, &ic->tme_m68k_ireg_memx32);
  /* while the above pop can fault, we don't have to worry about
     restarting here, because after this point, nothing can fault for
     the remainder of this instruction (the executor makes no stores
     on behalf of an rtr): */
  TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_memx32);
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_rts)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_pop32(ic, &ic->tme_m68k_ireg_memx32);
  tme_m68k_log(ic, 250, TME_OK,
	       (TME_M68K_LOG_HANDLE(ic),
		_("rts 0x%08x"),
		ic->tme_m68k_ireg_memx32));
  TME_M68K_INSN_BRANCH(ic->tme_m68k_ireg_memx32);
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_jsr)
{
  TME_M68K_INSN_CANFAULT;
  tme_m68k_push32(ic, ic->tme_m68k_ireg_pc_next);
  tme_m68k_log(ic, 250, TME_OK,
	       (TME_M68K_LOG_HANDLE(ic),
		_("jsr 0x%08x"),
		ic->_tme_m68k_ea_address));
  TME_M68K_INSN_BRANCH(ic->_tme_m68k_ea_address);
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_jmp)
{
  TME_M68K_INSN_BRANCH(ic->_tme_m68k_ea_address);
  TME_M68K_INSN_OK;
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_rte)
{
  TME_M68K_INSN_PRIV;
  ic->_tme_m68k_mode = TME_M68K_MODE_RTE;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_stop)
{
  TME_M68K_INSN_PRIV;
  ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
  ic->tme_m68k_ireg_sr = TME_M68K_INSN_SPECOP;
  ic->_tme_m68k_mode = TME_M68K_MODE_STOP;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_priv)
{
  TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_PRIV);
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_cmp2_chk2)
{
  tme_uint32_t ireg;
  unsigned int size_bytes, size_name, size_ireg;
  tme_uint32_t uvalue, ulower, uupper;

  TME_M68K_INSN_CANFAULT;

  /* get the register to check and the operand size: */
  ireg = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 4);
  size_bytes = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 2);
  size_ireg = 2 - size_bytes;
  size_bytes = 1 << size_bytes;
#if TME_M68K_SIZE_8 != 1
#error "TME_M68K_SIZE_8 must be 1"
#endif
#if TME_M68K_SIZE_16 != 2
#error "TME_M68K_SIZE_16 must be 2"
#endif
#if TME_M68K_SIZE_32 != 4
#error "TME_M68K_SIZE_32 must be 4"
#endif
  size_name = size_bytes;

  /* read in the two bounds: */
  (*_tme_m68k_read_mem[size_name])(ic, TME_M68K_IREG_MEMX32 << size_ireg);
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->_tme_m68k_ea_address += size_bytes;
  }
  (*_tme_m68k_read_mem[size_name])(ic, TME_M68K_IREG_MEMY32 << size_ireg);

  /* if we have an address register, sign-extend the bounds to 32
     bits: */
  if (ireg >= TME_M68K_IREG_A0) {
    if (size_name == TME_M68K_SIZE_8) {
      ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMX32) = TME_EXT_S8_S32(ic->tme_m68k_ireg_int8(TME_M68K_IREG_MEMX8));
      ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMY32) = TME_EXT_S8_S32(ic->tme_m68k_ireg_int8(TME_M68K_IREG_MEMY8));
    }
    else if (size_name == TME_M68K_SIZE_16) {
      ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMX32) = TME_EXT_S16_S32(ic->tme_m68k_ireg_int16(TME_M68K_IREG_MEMX16));
      ic->tme_m68k_ireg_int32(TME_M68K_IREG_MEMY32) = TME_EXT_S16_S32(ic->tme_m68k_ireg_int16(TME_M68K_IREG_MEMY16));
    }
    size_bytes = sizeof(tme_uint32_t);
    size_name = TME_M68K_SIZE_32;
  }

  /* get the values to check: */
  switch (size_name) {
  case TME_M68K_SIZE_8:
    uvalue = ic->tme_m68k_ireg_uint8(ireg << 2);
    ulower = ic->tme_m68k_ireg_uint8(TME_M68K_IREG_MEMX8);
    uupper = ic->tme_m68k_ireg_uint8(TME_M68K_IREG_MEMY8);
    break;
  case TME_M68K_SIZE_16:
    uvalue = ic->tme_m68k_ireg_uint16(ireg << 1);
    ulower = ic->tme_m68k_ireg_uint16(TME_M68K_IREG_MEMX16);
    uupper = ic->tme_m68k_ireg_uint16(TME_M68K_IREG_MEMY16);
    break;
  case TME_M68K_SIZE_32:
    uvalue = ic->tme_m68k_ireg_uint32(ireg);
    ulower = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_MEMX32);
    uupper = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_MEMY32);
    break;
  default: abort();
  }

  /* do the comparison.  if the value is out-of-bounds and this is
     a chk2 instruction, trap: */
  ic->tme_m68k_ireg_ccr = (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X);
  if (uvalue == ulower
      || uvalue == uupper) {
    ic->tme_m68k_ireg_ccr |= TME_M68K_FLAG_Z;
  }
  else if ((ulower > uupper)
	   ? (uvalue < ulower && uvalue > uupper)
	   : (uvalue < ulower || uvalue > uupper)) {
    ic->tme_m68k_ireg_ccr |= TME_M68K_FLAG_C;
    if (TME_M68K_INSN_SPECOP & TME_BIT(11)) {
      ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
      ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
      TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_CHK));
    }
  }

  TME_M68K_INSN_OK;
}

TME_M68K_INSN(tme_m68k_callm)
{
  /* TBD */
  abort();
}

TME_M68K_INSN(tme_m68k_rtm)
{
  /* TBD */
  abort();
}

/* this cannot fault: */
TME_M68K_INSN(tme_m68k_trapcc)
{
  if (TME_M68K_COND_TRUE(ic, TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 8, 4))) {
    ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_TRAP));
  }
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfchg)
{
  tme_uint32_t bf_value;
  bf_value = tme_m68k_bitfield_read_unsigned(ic);
  tme_m68k_bitfield_write_unsigned(ic, ~bf_value, FALSE);
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfclr)
{
  (void) tme_m68k_bitfield_read_unsigned(ic);
  tme_m68k_bitfield_write_unsigned(ic, 0, FALSE);
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfexts)
{
  tme_int32_t bf_value;
  bf_value = tme_m68k_bitfield_read_signed(ic);
  ic->tme_m68k_ireg_int32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 3))
    = bf_value;
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfextu)
{
  tme_uint32_t bf_value;
  bf_value = tme_m68k_bitfield_read_unsigned(ic);
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 3))
    = bf_value;
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfffo)
{
  tme_int16_t specop;
  tme_int32_t bf_offset;
  unsigned int bf_width;
  tme_uint32_t bf_value;
  unsigned int bf_pos;
    
  /* get the bitfield offset from a data register or as an immediate: */
  specop = TME_M68K_INSN_SPECOP;
  bf_offset = ((specop & TME_BIT(11))
	       ? ic->tme_m68k_ireg_int32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specop, 6, 3))
	       : (tme_int32_t) TME_FIELD_EXTRACTU(specop, 6, 5));

  /* get the bitfield width: */
  bf_width = tme_m68k_bitfield_width(ic);

  /* get the bitfield value: */
  bf_value = tme_m68k_bitfield_read_unsigned(ic);

  /* find the first set bit: */
  for (bf_pos = 0, bf_value <<= (32 - bf_width);
       bf_pos < bf_width && !(bf_value & 0x80000000);
       bf_pos++, bf_value <<= 1);

  /* set the result: */
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specop, 12, 3))
    = bf_offset + bf_pos;

  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfins)
{
  tme_m68k_bitfield_write_unsigned(ic, 
				   ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0
							    + TME_FIELD_EXTRACTU(TME_M68K_INSN_SPECOP, 12, 3)),
				   TRUE);
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bfset)
{
  (void) tme_m68k_bitfield_read_unsigned(ic);
  tme_m68k_bitfield_write_unsigned(ic, 0xffffffff, FALSE);
  TME_M68K_INSN_OK;
}

/* the bitfield helper functions handle faults: */
TME_M68K_INSN(tme_m68k_bftst)
{
  (void) tme_m68k_bitfield_read_unsigned(ic);
  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_pack)
{
  int ireg_x, ireg_y;
  tme_uint16_t value;

  TME_M68K_INSN_CANFAULT;

  /* get the two registers: */
  ireg_x = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);
  ireg_y = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 3);

  /* if this is a memory pack: */
  if (TME_M68K_INSN_OPCODE & TME_BIT(3)) {
    ireg_x += TME_M68K_IREG_A0;
    ireg_y += TME_M68K_IREG_A0;

    /* do the predecrement read of two bytes: */
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      ic->tme_m68k_ireg_uint32(ireg_x) -= sizeof(tme_uint16_t);
      ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);
      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ireg_x);
    }
    tme_m68k_read_memx16(ic);

    /* get the two bytes and add the extension: */
    value = ic->tme_m68k_ireg_memx16 + TME_M68K_INSN_SPECOP;

    /* do the predecrement write of one byte: */
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      ic->tme_m68k_ireg_uint32(ireg_y) -= sizeof(tme_uint8_t);
      ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);
      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ireg_y);
      ic->tme_m68k_ireg_memx8 = (tme_uint8_t) ((value & 0x0f) | (value >> 4));
    }
    tme_m68k_write_memx8(ic);
  }

  /* if this is a register pack: */
  else {
    ireg_x += TME_M68K_IREG_D0;
    ireg_y += TME_M68K_IREG_D0;
    value = ic->tme_m68k_ireg_uint16(ireg_x << 1) + TME_M68K_INSN_SPECOP;
    ic->tme_m68k_ireg_uint8(ireg_y << 2) = (tme_uint8_t) ((value & 0x0f) | (value >> 4));
  }

  TME_M68K_INSN_OK;
}

/* this can fault: */
TME_M68K_INSN(tme_m68k_unpk)
{
  int ireg_x, ireg_y;
  tme_uint16_t value;

  TME_M68K_INSN_CANFAULT;

  /* get the two registers: */
  ireg_x = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 0, 3);
  ireg_y = TME_FIELD_EXTRACTU(TME_M68K_INSN_OPCODE, 9, 3);

  /* if this is a memory pack: */
  if (TME_M68K_INSN_OPCODE & TME_BIT(3)) {
    ireg_x += TME_M68K_IREG_A0;
    ireg_y += TME_M68K_IREG_A0;

    /* do the predecrement read of one byte: */
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      ic->tme_m68k_ireg_uint32(ireg_x) -= sizeof(tme_uint8_t);
      ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);
      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ireg_x);
    }
    tme_m68k_read_memx8(ic);

    /* unpack the byte and add the extension: */
    value = ic->tme_m68k_ireg_memx8;
    value = ((value & 0x0f) | ((value & 0xf0) << 4)) + TME_M68K_INSN_SPECOP;

    /* do the predecrement write of two bytes: */
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      ic->tme_m68k_ireg_uint32(ireg_y) -= sizeof(tme_uint16_t);
      ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);
      ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_uint32(ireg_y);
      ic->tme_m68k_ireg_memx16 = value;
    }
    tme_m68k_write_memx16(ic);
  }

  /* if this is a register pack: */
  else {
    ireg_x += TME_M68K_IREG_D0;
    ireg_y += TME_M68K_IREG_D0;
    value = ic->tme_m68k_ireg_uint8(ireg_x << 2);
    value = ((value & 0x0f) | ((value & 0xf0) << 4)) + TME_M68K_INSN_SPECOP;
    ic->tme_m68k_ireg_uint16(ireg_y << 1) = value;
  }

  TME_M68K_INSN_OK;
}

/* these just redispatch into one of the multiply or divide insns: */
TME_M68K_INSN(tme_m68k_mull)
{
  if (TME_M68K_INSN_SPECOP & TME_BIT(11)) {
    tme_m68k_mulsl(ic, _op0, _op1);
  }
  else {
    tme_m68k_mulul(ic, _op0, _op1);
  }
}
TME_M68K_INSN(tme_m68k_divl)
{
  if (TME_M68K_INSN_SPECOP & TME_BIT(11)) {
    tme_m68k_divsl(ic, _op0, _op1);
  }
  else {
    tme_m68k_divul(ic, _op0, _op1);
  }
}

#include "m68k-bus-auto.c"

#include "m68k-insns-auto.c"
