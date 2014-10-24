/* $Id: m68k-execute.c,v 1.24 2009/08/29 19:25:48 fredette Exp $ */

/* ic/m68k/m68k-execute.c - executes Motorola 68k instructions: */

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

_TME_RCSID("$Id: m68k-execute.c,v 1.24 2009/08/29 19:25:48 fredette Exp $");

/* includes: */
#include "m68k-auto.h"

/* the m68k instruction executor: */
static void
_TME_M68K_EXECUTE_NAME(struct tme_m68k *ic)
{
#undef _TME_M68K_SEQUENCE_RESTARTING
#undef _TME_M68K_INSN_FETCH_SAVE
#ifdef _TME_M68K_EXECUTE_FAST
  tme_bus_context_t bus_context;
  struct tme_m68k_tlb *tlb;
  const tme_shared tme_uint8_t *fetch_fast_next;
#define _TME_M68K_INSN_FETCH_SAVE \
do { \
  ic->_tme_m68k_insn_fetch_fast_next = fetch_fast_next; \
} while (/* CONSTCOND */ 0)
#define _TME_M68K_SEQUENCE_RESTARTING	(FALSE)
#else  /* !_TME_M68K_EXECUTE_FAST */
  unsigned int exceptions;
  tme_uint32_t linear_pc;
#define _TME_M68K_INSN_FETCH_SAVE \
do { \
} while (/* CONSTCOND */ 0)
#define _TME_M68K_SEQUENCE_RESTARTING	TME_M68K_SEQUENCE_RESTARTING
#endif /* !_TME_M68K_EXECUTE_FAST */
#if (_TME_M68K_EXECUTE_CPU == TME_M68K_M68020) || (_TME_M68K_EXECUTE_CPU == TME_M68K_M68030)
  unsigned int eai_function_code;
  int ea_post_index;
  unsigned int ea_i_is;
  tme_uint32_t ea_od;
  unsigned int src_specifier;
#else  /* !TME_M68K_M68020 && !TME_M68K_M68030 */
#define eai_function_code ea_function_code
#endif  /* !TME_M68K_M68020 && !TME_M68K_M68030 */
  unsigned int function_code_program;
  unsigned int function_code_data;
  tme_uint16_t opw, extword;
  void (*func) _TME_P((struct tme_m68k *, void *, void *));
  tme_uint32_t params;
  int ea_size;
  int ea_reg, ea_pre_index;
  unsigned int ea_index_long, ea_index_scale;
  tme_uint32_t ea_address;
  unsigned int ea_function_code;
  tme_int32_t ea_bd;
  tme_uint32_t imm32;
  tme_uint16_t transfer_next_before;
  int rc;

  /* silence gcc -Wuninitialized: */
  ea_size = 0;

  /* get the function codes.  if the privilege ever changes as a
     result of any instruction, we must redispatch: */
  if (TME_M68K_PRIV(ic)) {
    function_code_program = TME_M68K_FC_SP;
    function_code_data = TME_M68K_FC_SD;
  }
  else {
    function_code_program = TME_M68K_FC_UP;
    function_code_data = TME_M68K_FC_UD;
  }

  /* if we have used up our burst: */
  if (ic->_tme_m68k_instruction_burst_remaining == 0) {

    /* start a new burst: */
    ic->_tme_m68k_instruction_burst_remaining
      = ic->_tme_m68k_instruction_burst;

    /* if this is a cooperative threading system, yield: */
#if TME_THREADS_COOPERATIVE
    tme_thread_yield();
#endif /* TME_THREADS_COOPERATIVE */
  }

#ifdef _TME_M68K_EXECUTE_FAST

  /* get our instruction TLB entry and reload it: */
  bus_context = ic->_tme_m68k_bus_context;
  tlb = &ic->_tme_m68k_itlb;
  tme_m68k_tlb_busy(tlb);
  if (__tme_predict_false(tme_m68k_tlb_is_invalid(tlb)
			  || tlb->tme_m68k_tlb_bus_context != bus_context
			  || (tlb->tme_m68k_tlb_function_codes_mask
			      & TME_BIT(function_code_program)) == 0
			  || ic->tme_m68k_ireg_pc < (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first
			  || ic->tme_m68k_ireg_pc > (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last
			  || tlb->tme_m68k_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF)) {
    tme_m68k_tlb_fill(ic, tlb,
		      function_code_program,
		      ic->tme_m68k_ireg_pc,
		      TME_BUS_CYCLE_READ);
  }

  /* if we have to go slow, run the slow executor: */
  if (TME_M68K_SEQUENCE_RESTARTING
      || tme_m68k_go_slow(ic)) {
    tme_m68k_tlb_unbusy(tlb);
    _TME_M68K_EXECUTE_SLOW(ic);
    return;
  }

  /* set up to do fast reads from the instruction TLB entry: */
  ic->_tme_m68k_insn_fetch_fast_last = tlb->tme_m68k_tlb_emulator_off_read + ((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last) - (sizeof(tme_uint32_t) - 1);
  ic->_tme_m68k_insn_fetch_fast_itlb = tlb;
  ic->_tme_m68k_group0_hook = tme_m68k_group0_hook_fast;
#else  /* !_TME_M68K_EXECUTE_FAST */

  /* set up to do slow reads from the instruction TLB entry: */
  ic->_tme_m68k_group0_hook = NULL;
#endif /* !_TME_M68K_EXECUTE_FAST */

  /* the execution loop: */
  for (;;) {

    /* reset for this instruction: */
#ifdef _TME_M68K_EXECUTE_FAST
    fetch_fast_next = tlb->tme_m68k_tlb_emulator_off_read + ic->tme_m68k_ireg_pc;
    ic->_tme_m68k_insn_fetch_fast_start = fetch_fast_next;
    assert (ic->_tme_m68k_insn_fetch_fast_itlb == tlb
	    && tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
	    && (tlb->tme_m68k_tlb_function_codes_mask & TME_BIT(function_code_program)) != 0
	    && (fetch_fast_next > ic->_tme_m68k_insn_fetch_fast_last
		|| ((tme_m68k_tlb_is_valid(tlb)
		     || !TME_THREADS_COOPERATIVE)
		    && tlb->tme_m68k_tlb_bus_context == ic->_tme_m68k_bus_context
		    && ic->tme_m68k_ireg_pc >= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first
		    && (ic->tme_m68k_ireg_pc + sizeof(tme_uint16_t) - 1) <= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last)));
    tme_m68k_verify_begin(ic, fetch_fast_next);
#else  /* !_TME_M68K_EXECUTE_FAST */
    linear_pc = ic->tme_m68k_ireg_pc;
    ic->_tme_m68k_insn_fetch_slow_next = 0;
    if (!_TME_M68K_SEQUENCE_RESTARTING) {
      ic->_tme_m68k_insn_fetch_slow_count_fast = 0;
      ic->_tme_m68k_insn_fetch_slow_count_total = 0;
    }
    exceptions = 0;
    if (__tme_predict_false((ic->tme_m68k_ireg_sr & ic->_tme_m68k_sr_mask_t) == TME_M68K_FLAG_T1)) {
      ic->tme_m68k_ireg_pc_last = ic->tme_m68k_ireg_pc;
      exceptions |= TME_M68K_EXCEPTION_TRACE;
    }
    tme_m68k_verify_begin(ic, NULL);
#endif /* _TME_M68K_EXECUTE_FAST */
#ifdef _TME_M68K_VERIFY
    if (ic->tme_m68k_ireg_pc == 0x6000) {
      tme_m68k_verify_hook();
    }
#endif
#ifdef _TME_M68K_STATS
    ic->tme_m68k_stats.tme_m68k_stats_insns_total++;
#ifndef _TME_M68K_EXECUTE_FAST
    ic->tme_m68k_stats.tme_m68k_stats_insns_slow++;
#endif /* !_TME_M68K_EXECUTE_FAST */
#endif /* _TME_M68K_STATS */
    ic->_tme_m68k_instruction_burst_remaining--;
    
    /* fetch and decode the first word of this instruction: */
    _TME_M68K_EXECUTE_FETCH_U16_FIXED(opw, _tme_m68k_insn_opcode);
    ic->_tme_m68k_insn_opcode = opw;
    params = _TME_M68K_EXECUTE_OPMAP[opw];
    func = tme_m68k_opcode_insns[TME_M68K_OPCODE_INSN_WHICH(params)];

    /* now that we no longer need the insn index part of the params,
       replace it with the least significant bits of the opcode, which
       contain any EA mode and reg fields: */
    TME_FIELD_MASK_DEPOSITU(params, 
			    TME_M68K_OPCODE_INSN_MASK, 
			    (opw & (TME_M68K_OPCODE_INSN_MASK / TME_M68K_OPCODE_INSN(1))));
    
    /* if this is a special opcode: */
    if (__tme_predict_false((params & TME_M68K_OPCODE_SPECOP) != 0)) {

#if (_TME_M68K_EXECUTE_CPU == TME_M68K_M68020) || (_TME_M68K_EXECUTE_CPU == TME_M68K_M68030)

      /* a general floating-point instruction: */
      if (__tme_predict_false((opw & 0xffc0) == 0xf200)) {

	/* if there is no FPU present, or if it isn't enabled: */
	if (__tme_predict_false(!ic->tme_m68k_fpu_enabled)) {

	  /* mark this instruction as illegal and use an FPgen command
	     word of zero: */
	  func = tme_m68k_illegal;
	  extword = 0;
	}

	/* otherwise, there is an FPU present and it is enabled: */
	else {

	  /* fetch the FPgen command word: */
	  _TME_M68K_EXECUTE_FETCH_U16_FIXED(ic->_tme_m68k_insn_specop, _tme_m68k_insn_specop);

	  /* temporarily store the FPgen command word in extword: */
	  extword = ic->_tme_m68k_insn_specop;
	}

	/* the goal here is not to decide whether or not an FPgen
	   instruction is legal, although some illegal instructions
	   are caught.  the goal is to only decide if this FPgen
	   instruction uses the EA field.

	   we need to know this here because only the executer can
	   calculate all memory EAs (i.e., absolute addresses,
	   indirect addresses, PC-relative addresses, etc.) and fetch
	   immediates.

	   in general, all other decisions about whether or not an
	   instruction is legal (including whether or not certain EAs
	   are legal, like data register direct or address register
	   direct) or how to dispatch it is done somewhere else.

	   all legal FPgen instructions that use the EA field are
	   handled by one of the following ifs.  all illegal FPgen
	   instructions, and all FPgen instructions that do not use
	   the EA field are handled by the final unconditional else
	   clause that cancels memory EA calculation and immediate
	   fetching.

	   for those FPgen instructions that do use the EA field, they
	   either leave the EA cycles unchanged, as TME_M68K_OPCODE_EA_READ, to
	   indicate that they read the EA, or they change it to
	   TME_M68K_OPCODE_EA_WRITE to indicate that they write the EA.
	   additionally, they either flag any immediate operand as
	   illegal, fetch it themselves, or specify its size for the
	   normal immediate fetching code to use: */

	/* m68k-iset.txt must assume that any EA is not written: */
	assert (!TME_M68K_OPCODE_HAS_EA(params)
		|| ((params
		     & (TME_M68K_OPCODE_EA_READ
			| TME_M68K_OPCODE_EA_WRITE))
		    == TME_M68K_OPCODE_EA_READ));

	/* m68k-iset.txt must assume that any immediate is 32 bits: */
	assert (!TME_M68K_OPCODE_HAS_IMM(params)
		|| (params & (TME_M68K_OPCODE_IMM_16 | TME_M68K_OPCODE_IMM_32)) == TME_M68K_OPCODE_IMM_32);

	/* if this is an FMOVE or FMOVEM of floating-point control
	   registers (command word pattern 10dr rr00 0000 0000): */
	if ((extword & 0xc3ff) == 0x8000) {

	  /* override the function: */
	  func = tme_m68k_fmovemctl;

	  /* if this is a register-to-memory operation: */
	  if (extword & TME_BIT(13)) {

	    /* any EA must be writable: */
	    params |= TME_M68K_OPCODE_EA_WRITE;
	  }

	  /* otherwise, this is a memory-to-register operation: */
	  else {

	    /* if this instruction has an immediate, and this
	       instruction is moving multiple control registers, this
	       is an illegal instruction: */
	    /* NB the trick we use to see if multiple bits are set in
	       the rrr field - we subtract one from the base of the
	       rrr field (0x0400), binary-AND the result with extword,
	       and mask off all bits except the rrr field.  this
	       result will be nonzero iff the rrr field has multiple
	       bits set: */
	    if (__tme_predict_false(TME_M68K_OPCODE_HAS_IMM(params)
				    && ((extword & (extword - 0x0400)) & 0x1c00))) {
	      func = tme_m68k_illegal;
	    }
	  }
	}

	/* if this is an FMOVEM
	   (command word pattern 11dm m000 rrrr rrrr): */
	else if ((extword & 0xc700) == 0xc000) {

	  /* override the function: */
	  func = tme_m68k_fmovem;

	  /* if this instruction has an immediate, this is an
	     illegal instruction: */
	  if (__tme_predict_false(TME_M68K_OPCODE_HAS_IMM(params))) {
	    func = tme_m68k_illegal;
	  }

	  /* if this is a register-to-memory operation: */
	  if (extword & TME_BIT(13)) {

	    /* any EA must be writable: */
	    params |= TME_M68K_OPCODE_EA_WRITE;
	  }
	}

	/* if this is a register-to-memory FMOVE instruction
	   (command word pattern 011d ddss skkk kkkk): */
	else if ((extword & 0xe000) == 0x6000) {

	  /* override the function: */
	  func = tme_m68k_fmove_rm;

	  /* any EA must be writable: */
	  params |= TME_M68K_OPCODE_EA_WRITE;
	}

	/* if this is a memory-to-register true FPgen instruction
	   (command word pattern 010s ssdd dooo oooo): */
	else if ((extword & 0xe000) == 0x4000
		 && (_tme_m6888x_fpgen_opmode_bitmap[TME_FIELD_EXTRACTU(extword, 0, 7) / 8]
		     & (1 << (TME_FIELD_EXTRACTU(extword, 0, 7) % 8)))
		 && TME_FIELD_EXTRACTU(extword, 10, 3) != TME_M6888X_TYPE_INVALID) {

	  /* if this instruction has an immediate: */
	  if (TME_M68K_OPCODE_HAS_IMM(params)) {

	    /* if the source specifier is for a size that the normal
	       immediate fetching code can handle, let it handle it,
	       otherwise we have to fetch the immediate ourselves: */

	    /* m68k-iset.txt must have specified the EA operand to be
	       operand one: */
	    assert((void *) TME_M68K_OPCODE_OP1_WHICH(ic, params)
		   == (void *) &ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 0));

	    /* dispatch on the source specifier: */
	    src_specifier = TME_FIELD_EXTRACTU(extword, 10, 3);
	    switch (src_specifier) {

	      /* we can let the normal immediate fetching code fetch
		 word integers, long-word integers, and
		 single-precision reals: */
	    default: 
	      assert (func == tme_m68k_illegal);
	      /* FALLTHROUGH */
	    case TME_M6888X_TYPE_WORD:
	      /* we can simply flip TME_M68K_OPCODE_IMM_16 and
		 TME_M68K_OPCODE_IMM_32 to select a 16-bit immediate;
		 we know that only TME_M68K_OPCODE_IMM_32 is set,
		 thanks to the assert we did at the beginning of the
		 fpgen specop handling: */
	      params ^= (TME_M68K_OPCODE_IMM_16 | TME_M68K_OPCODE_IMM_32);
	      /* FALLTHROUGH */
	    case TME_M6888X_TYPE_LONG:
	    case TME_M6888X_TYPE_SINGLE:
	      break;
	    case TME_M6888X_TYPE_EXTENDED80:
	    case TME_M6888X_TYPE_PACKEDDEC:
	    case TME_M6888X_TYPE_DOUBLE:
	      _TME_M68K_EXECUTE_FETCH_U32(imm32);
	      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 0) = imm32;
	      _TME_M68K_EXECUTE_FETCH_U32(imm32);
	      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 1) = imm32;
	      if (src_specifier != TME_M6888X_TYPE_DOUBLE) {
		_TME_M68K_EXECUTE_FETCH_U32(imm32);
		ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 2) = imm32;
	      }
	      /* we only need to clear TME_M68K_OPCODE_IMM_32 here to
		 cancel the later immediate fetching; we know that
		 only TME_M68K_OPCODE_IMM_32 is set, thanks to the
		 assert we did at the beginning of the fpgen specop
		 handling: */
	      params &= ~TME_M68K_OPCODE_IMM_32;
	      break;
	    case TME_M6888X_TYPE_BYTE:	      
	      _TME_M68K_EXECUTE_FETCH_U16(imm32);
	      ic->tme_m68k_ireg_uint32(TME_M68K_IREG_IMM32 + 0) = TME_EXT_S8_U32((tme_int8_t) imm32);
	      /* we only need to clear TME_M68K_OPCODE_IMM_32 here to
		 cancel the later immediate fetching; we know that
		 only TME_M68K_OPCODE_IMM_32 is set, thanks to the
		 assert we did at the beginning of the fpgen specop
		 handling: */
	      params &= ~TME_M68K_OPCODE_IMM_32;
	      break;
	    }
	  }
	}

	/* otherwise, this FPgen instruction does not need any memory
	   EA or immediate.  we'll decide later if this instruction is
	   actually legal: */
	else {
	  /* cancel immediate fetching and all EA work: */
	  params &= ~(TME_M68K_OPCODE_IMM_32
		      | TME_M68K_OPCODE_IMM_16
		      | TME_M68K_OPCODE_EA_SIZE_MASK
		      | TME_M68K_OPCODE_EA_READ
		      | TME_M68K_OPCODE_EA_WRITE);
	}

	/* if this instruction has already been marked as illegal, or
	   this EA must be writable, and this is a PC-relative or
	   immediate EA, this instruction is illegal: */
	if (__tme_predict_false(func == tme_m68k_illegal
				|| (params & TME_M68K_OPCODE_EA_WRITE
				    && (TME_M68K_OPCODE_HAS_IMM(params)
					|| (TME_M68K_OPCODE_EA_MODE_WHICH(params) == 7
					    && (TME_M68K_OPCODE_EA_REG_WHICH(params) & 6) == 2))))) {
	  func = tme_m68k_illegal;
	  /* cancel immediate fetching and all EA work: */
	  params &= ~(TME_M68K_OPCODE_IMM_32
		      | TME_M68K_OPCODE_IMM_16
		      | TME_M68K_OPCODE_EA_SIZE_MASK
		      | TME_M68K_OPCODE_EA_READ
		      | TME_M68K_OPCODE_EA_WRITE);
	}

	/* otherwise, if this instruction has a memory EA: */
	else if (params & TME_M68K_OPCODE_EA_READ) {

	  /* override any m68k-iset.txt guess about the operand
	     size, and cancel all memory cycles.  the instruction
	     itself will do the actual operand reading and any address
	     register postincrement or predecrement: */
	  params = 
	    ((params
	      & ~(TME_M68K_OPCODE_EA_READ
		  | TME_M68K_OPCODE_EA_WRITE
		  | TME_M68K_OPCODE_EA_SIZE_MASK))
	     | TME_M68K_OPCODE_EA_UNSIZED);
	}

	/* otherwise, this instruction does not have a memory EA: */
	else {

	  /* cancel all EA work: */
	  params &= ~(TME_M68K_OPCODE_EA_SIZE_MASK
		      | TME_M68K_OPCODE_EA_READ
		      | TME_M68K_OPCODE_EA_WRITE);
	}
      }
      else
#endif /* TME_M68K_M68020 || TME_M68K_M68030 */

	/* if this is not a memory-to-memory move instruction: */
	if ((params & TME_M68K_OPCODE_EA_Y) == 0) {

	  /* many instructions have a single special extension word: */
	  _TME_M68K_EXECUTE_FETCH_U16_FIXED(ic->_tme_m68k_insn_specop, _tme_m68k_insn_specop);
	}
    }

    /* get any immediate operand: */
    if (__tme_predict_false(TME_M68K_OPCODE_HAS_IMM(params))) {
      if (params & TME_M68K_OPCODE_IMM_16) {
	_TME_M68K_EXECUTE_FETCH_S16(imm32);
      }
      else {
	_TME_M68K_EXECUTE_FETCH_U32(imm32);
      }
      ic->tme_m68k_ireg_imm32 = imm32;
    }	
    
    /* loop over up to two effective addresses calculations.  this
       initializes for the normal, single effective address: */
    while (TME_M68K_OPCODE_HAS_EA(params)) {

      /* if this EA is described by the alternate EA mode and reg
	 fields, copy them into the EA mode and reg fields in
	 params: */
      if (__tme_predict_false((params
			       & (TME_M68K_OPCODE_EA_Y | TME_M68K_OPCODE_SPECOP))
			      == TME_M68K_OPCODE_EA_Y)) {
      
	/* reload to write the other memory EA: */
	params
	  = ((params
	      & ~(TME_M68K_OPCODE_EA_MODE_MASK
		  | TME_M68K_OPCODE_EA_REG_MASK
		  | TME_M68K_OPCODE_EA_READ
		  | TME_M68K_OPCODE_EA_Y))
	     | TME_M68K_OPCODE_EA_MODE(TME_FIELD_EXTRACTU(opw, 6, 3))
	     | TME_M68K_OPCODE_EA_REG(TME_FIELD_EXTRACTU(opw, 9, 3))
	     | TME_M68K_OPCODE_EA_WRITE);
      }

      /* get the reg, size, and function code of this EA: */
      ea_reg = TME_M68K_IREG_A0 + TME_M68K_OPCODE_EA_REG_WHICH(params);
      ea_size = TME_M68K_OPCODE_EA_SIZE_WHICH(params);
      ea_function_code = function_code_data;
      
      /* this EA must have either no size, or be exactly one, two, or
	 four bytes: */
      assert(ea_size == TME_M68K_SIZE_UNSIZED
	     || ea_size == TME_M68K_SIZE_8
	     || ea_size == TME_M68K_SIZE_16
	     || ea_size == TME_M68K_SIZE_32);

      /* for the effective address predecrement and postincrement
	 modes, we require that these size macros correspond exactly
	 to the number of bytes, that the %a7 register number be 15,
	 and that the ea reg not be greater than %a7: */
#if TME_M68K_SIZE_UNSIZED != 0
#error "TME_M68K_SIZE_UNSIZED must be 0"
#endif
#if TME_M68K_SIZE_8 != 1
#error "TME_M68K_SIZE_8 must be 1"
#endif
#if TME_M68K_SIZE_16 != 2
#error "TME_M68K_SIZE_16 must be 2"
#endif
#if TME_M68K_SIZE_32 != 4
#error "TME_M68K_SIZE_32 must be 4"
#endif
#if TME_M68K_IREG_A7 != 15
#error "TME_M68K_IREG_A7 must be 15"
#endif
      assert(ea_reg <= TME_M68K_IREG_A7);
#define TME_M68K_AREG_INCREMENT(areg, size) \
  (((((areg) + 1) / (TME_M68K_IREG_A7 + 1)) & (size)) + (size))

      /* initialize ea_address to silence -Wuninitialized: */
      ea_address = 0;

      /* set the EA inner function code: */
      eai_function_code = ea_function_code;

      /* dispatch on the mode: */
      switch (TME_M68K_OPCODE_EA_MODE_WHICH(params)) {
	
	/* address register indirect: */
      case 2:
	ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	break;
	  
	/* address register indirect postincrement: */
      case 3:
	/* if we are not restarting, set the effective address: */
	if (!_TME_M68K_SEQUENCE_RESTARTING) {
	  ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	  ic->tme_m68k_ireg_uint32(ea_reg) += TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
	}
	break;
	  
	/* address register indirect predecrement: */
      case 4:
	/* if we are not restarting, set the effective address: */
	if (!_TME_M68K_SEQUENCE_RESTARTING) {
	  ic->tme_m68k_ireg_uint32(ea_reg) -= TME_M68K_AREG_INCREMENT(ea_reg, ea_size);
	  ea_address = ic->tme_m68k_ireg_uint32(ea_reg);
	}
	break;
	  
	/* address register indirect with 16-bit displacement: */
      case 5:
	_TME_M68K_EXECUTE_FETCH_S16(ea_bd);
	ea_address = ic->tme_m68k_ireg_uint32(ea_reg) + ea_bd;
	break;
	  
	/* miscellaneous modes: */
      case 7:
	  
	/* absolute short addressing: */
	if (ea_reg == TME_M68K_IREG_A0) {
	  _TME_M68K_EXECUTE_FETCH_S16(ea_address);
	  break;
	}	    
	  
	/* absolute long addressing: */
	if (ea_reg == TME_M68K_IREG_A1) {
	  _TME_M68K_EXECUTE_FETCH_S32(ea_address);
	  break;
	}	    
	  
	/* the remaining modes use the PC of the first extension word
	   as a base register: */
#ifdef _TME_M68K_EXECUTE_FAST
	ic->tme_m68k_ireg_pc_next = ic->tme_m68k_ireg_pc + (fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start);
#else  /* !_TME_M68K_EXECUTE_FAST */
	ic->tme_m68k_ireg_pc_next = linear_pc;
#endif /* !_TME_M68K_EXECUTE_FAST */

	/* program counter indirect with 16-bit displacement: */
	if (ea_reg == TME_M68K_IREG_A2) {
	  _TME_M68K_EXECUTE_FETCH_S16(ea_bd);
	  ea_address = ic->tme_m68k_ireg_pc_next + ea_bd;
	  ea_function_code = function_code_program;
	  break;
	}
	  
	/* everything else is just like mode 6 except with the PC of
	   the first extension word as the base register: */
	assert (ea_reg == TME_M68K_IREG_A3);
	ea_reg = TME_M68K_IREG_PC_NEXT;
	eai_function_code = function_code_program;
	/* FALLTHROUGH */
	  
	/* various indexed modes: */
      case 6:
	  
	/* fetch the extension word and take it apart.  the 68000 and
	   68010 ignore the scale field in the extension word and always
	   behave as if it is zero: */
	_TME_M68K_EXECUTE_FETCH_U16(extword);
	ea_pre_index = TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(extword, 12, 4);
	ea_index_long = (extword & TME_BIT(11));
#if (_TME_M68K_EXECUTE_CPU == TME_M68K_M68020) || (_TME_M68K_EXECUTE_CPU == TME_M68K_M68030)
	ea_index_scale = TME_FIELD_EXTRACTU(extword, 9, 2);
#else  /* !TME_M68K_M68020 && !TME_M68K_M68030 */
	ea_index_scale = 0;
#endif /* !TME_M68K_M68020 && !TME_M68K_M68030 */
	  
	/* if this is a full extension word: */
	if (__tme_predict_false(extword & TME_BIT(8))) {
#if (_TME_M68K_EXECUTE_CPU == TME_M68K_M68020) || (_TME_M68K_EXECUTE_CPU == TME_M68K_M68030)

	  ea_i_is = TME_FIELD_EXTRACTU(extword, 0, 3);

	  /* optionally suppress the base register: */
	  if (extword & TME_BIT(7)) {
	    ea_reg = TME_M68K_IREG_ZERO;
	  }
	    
	  /* fetch any base displacement: */
	  ea_bd = 0;
	  switch (TME_FIELD_EXTRACTU(extword, 4, 2)) {
	  case 0: abort();
	  case 1: break;
	  case 2: _TME_M68K_EXECUTE_FETCH_S16(ea_bd); break;
	  case 3: _TME_M68K_EXECUTE_FETCH_S32(ea_bd); break;
	  }
	    
	  /* optionally suppress the index register.  this is also
	     where we check for combined IS-I/IS fields greater than
	     or equal to 0xc, which are reserved: */
	  if (extword & TME_BIT(6)) {
	    ea_pre_index = TME_M68K_IREG_ZERO;
	    if (ea_i_is >= 0x4) {
	      abort();
	    }
	  }

	  /* fetch any outer displacement: */
	  ea_od = 0;
	  switch (ea_i_is & 3) {
	  case 0: case 1: break;
	  case 2: _TME_M68K_EXECUTE_FETCH_S16(ea_od); break;
	  case 3: _TME_M68K_EXECUTE_FETCH_S32(ea_od); break;
	  }

	  /* dispatch on the I/IS fields: */
	  ea_post_index = TME_M68K_IREG_ZERO;
	  switch (ea_i_is) {

	    /* no memory indirect action: */
	  case 0x0:
	    ea_post_index = TME_M68K_IREG_UNDEF;
	    break;
	    
	    /* indirect preindexed with null outer displacement: */
	    /* indirect preindexed with word outer displacement: */
	    /* indirect preindexed with long outer displacement: */
	  case 0x1: case 0x2: case 0x3:
	    break;

	    /* reserved: */
	  case 0x4: default: abort();

	    /* indirect postindexed with null outer displacement: */
	    /* indirect postindexed with word outer displacement: */
	    /* indirect postindexed with long outer displacement: */
	  case 0x5: case 0x6: case 0x7:
	    ea_post_index = ea_pre_index;
	    ea_pre_index = TME_M68K_IREG_ZERO;
	    break;
	  }

	  /* preindex and base-displace the original address register
	     to arrive at the indirect EA: */
	  ea_address = 
	    (ic->tme_m68k_ireg_uint32(ea_reg)
	     + ((ea_index_long
		 ? ic->tme_m68k_ireg_int32(ea_pre_index)
		 : ((tme_int32_t) ic->tme_m68k_ireg_int16(ea_pre_index << 1)))
		<< ea_index_scale)
	     + ea_bd);
	  
	  /* if this is a memory indirect, read the indirect EA.
	     don't disturb the EA in the IC state if we're restarting,
	     for two reasons:

	     first, the value in the IC state may belong to some later
	     part of the instruction handling, in which case we must
	     (continue to) preserve it, and
	     
	     second, if the EA in the IC state *is* from this part of
	     the instruction handling, it's correct, while our EA may
	     *not* be correct, since it was generated from IC state
	     that may have changed since the instruction originally
	     started (i.e., address register changes by the user or by
	     our own postincrement/predecrement, or function code
	     register changes by the user): */
	  if (ea_post_index != TME_M68K_IREG_UNDEF) {
	    if (!_TME_M68K_SEQUENCE_RESTARTING) {
	      ic->_tme_m68k_ea_address = ea_address;
	      ic->_tme_m68k_ea_function_code = eai_function_code;
	    }
	    _TME_M68K_INSN_FETCH_SAVE;
	    tme_m68k_read_mem32(ic, TME_M68K_IREG_MEMY32);
	    ea_address =
	      (ic->tme_m68k_ireg_memy32
	       + ((ea_index_long
		   ? ic->tme_m68k_ireg_int32(ea_post_index)
		   : ((tme_int32_t) ic->tme_m68k_ireg_int16(ea_post_index << 1)))
		  << ea_index_scale)
	       + ea_od);
	  }
	  else {
	    ea_function_code = eai_function_code;
	  }
	  
#else  /* !TME_M68K_M68020 && !TME_M68K_M68030 */
	  /* XXX - illegal instruction */
	  abort();
#endif /* !TME_M68K_M68020 && !TME_M68K_M68030 */
	}
	
	/* otherwise, this is a brief extension word: */
	else {
	  ea_address = 
	    (ic->tme_m68k_ireg_uint32(ea_reg)
	     + ((tme_int32_t) ((tme_int8_t) (extword & 0xff)))
	     + ((ea_index_long
		 ? ic->tme_m68k_ireg_int32(ea_pre_index)
		 : ((tme_int32_t) ic->tme_m68k_ireg_int16(ea_pre_index << 1)))
		<< ea_index_scale));
	  ea_function_code = eai_function_code;
	}
	break;

      default: assert(FALSE);
      }

      /* we have calculated the effective address.  we don't store it
	 if we're restarting, because it may have been calculated
	 using user-visible registers (address registers and even
	 function code registers!) that the user may have changed (or,
	 in the case of pre/postdecrement EAs, that *we* may have
	 changed) between the bus fault and the instruction restart.
	 when we restart an instruction we *always* want to use the
	 same effective address as before: */
      if (!_TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_address = ea_address;
	ic->_tme_m68k_ea_function_code = eai_function_code;
      }
      
      /* XXX XXX XXX - if we detect a store to program space, that's an illegal: */
      /* XXX but maybe not for moves? */
      if (__tme_predict_false(ea_function_code == function_code_program
			      && (params & TME_M68K_OPCODE_EA_WRITE) != 0)) {
	abort();
      }

      /* if we're loading this operand: */
      if (params & TME_M68K_OPCODE_EA_READ) {
	_TME_M68K_INSN_FETCH_SAVE;
	(*_tme_m68k_read_memx[ea_size])(ic);
      }

      /* stop unless this is a memory-to-memory move: */
      if (__tme_predict_true(!(params & TME_M68K_OPCODE_EA_Y)))
	break;

      /* loop to reload for the other memory EA at the same size: */
      params &= ~TME_M68K_OPCODE_SPECOP;
    }

    /* we've fetched all of the instruction words: */
    _TME_M68K_INSN_FETCH_SAVE;

    /* set the next PC: */
#ifdef _TME_M68K_EXECUTE_FAST
    ic->tme_m68k_ireg_pc_next = ic->tme_m68k_ireg_pc + (fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start);
#else  /* !_TME_M68K_EXECUTE_FAST */
    ic->tme_m68k_ireg_pc_next = linear_pc;
#endif /* !_TME_M68K_EXECUTE_FAST */

    /* if we're not restarting, or if this instruction function can
       fault, call the instruction function: */
    if (!_TME_M68K_SEQUENCE_RESTARTING
	|| (ic->_tme_m68k_mode_flags & TME_M68K_EXECUTION_INST_CANFAULT)) {
      transfer_next_before = ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next;
      (*func)(ic, TME_M68K_OPCODE_OP0_WHICH(ic, params), TME_M68K_OPCODE_OP1_WHICH(ic, params));
      assert(!(ic->_tme_m68k_mode_flags & TME_M68K_EXECUTION_INST_CANFAULT)
	     != (ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next
		 != transfer_next_before));
      ic->_tme_m68k_mode_flags &= ~TME_M68K_EXECUTION_INST_CANFAULT;
    }

    /* store up to one EA path: */
    if ((params & TME_M68K_OPCODE_EA_WRITE) != 0) {
      (*_tme_m68k_write_memx[ea_size])(ic);
    }

    /* an instruction has ended: */
    tme_m68k_verify_end(ic, func);

    /* update the PC: */
    ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
    TME_M68K_SEQUENCE_START;

#ifdef _TME_M68K_EXECUTE_FAST
    /* if we haven't finished the instruction burst yet, continue: */
    if (__tme_predict_true(ic->_tme_m68k_instruction_burst_remaining != 0)) {
      continue;
    }
#endif /* _TME_M68K_EXECUTE_FAST */

    /* try to acquire the external mutex and check for external
       resets, halts, or interrupts, and process them along
       with any internal exceptions: */
    rc = tme_mutex_trylock(&ic->tme_m68k_external_mutex);
    if (TME_THREADS_ERRNO(rc) == TME_OK) {
      tme_m68k_external_check(ic, 
#ifdef _TME_M68K_EXECUTE_FAST
			      0
#else  /* !_TME_M68K_EXECUTE_FAST */
			      exceptions
#endif /* !_TME_M68K_EXECUTE_FAST */
			      );

      /* unlock the external mutex: */
      tme_mutex_unlock(&ic->tme_m68k_external_mutex);
    }

#ifndef _TME_M68K_EXECUTE_FAST

    /* otherwise, if we have internal exceptions, process them: */
    else if (exceptions) {
      tme_m68k_exception(ic, exceptions);
    }

    /* if we can go fast now, go fast: */
    if (!tme_m68k_go_slow(ic)) {
      tme_m68k_redispatch(ic);
    }

    /* otherwise, unless we've used up our burst, continue: */
    if (ic->_tme_m68k_instruction_burst_remaining != 0) {
      continue;
    }

#endif /* !_TME_M68K_EXECUTE_FAST */

    /* start a new burst: */
    ic->_tme_m68k_instruction_burst_remaining
      = ic->_tme_m68k_instruction_burst;

    /* if this is a cooperative threading system, yield: */
#if TME_THREADS_COOPERATIVE
#ifdef _TME_M68K_EXECUTE_FAST
    /* unbusy and forget the fast instruction TLB entry: */
    assert (ic->_tme_m68k_insn_fetch_fast_itlb == tlb);
    tme_m68k_tlb_unbusy(tlb);
    ic->_tme_m68k_insn_fetch_fast_itlb = NULL;
#endif /* _TME_M68K_EXECUTE_FAST */
    tme_thread_yield();
#endif /* TME_THREADS_COOPERATIVE */

#ifdef _TME_M68K_EXECUTE_FAST
    /* if this instruction TLB entry has been invalidated, redispatch.
       this can only happen in a multiprocessing (preemptive or true
       multiprocessor) environment, and it means that during the
       previous burst, another thread invalidated this instruction TLB
       entry, but we didn't make any callouts at all (for TLB fills,
       slow bus cycles, etc.) where we would have noticed this
       earlier: */
    if (tme_m68k_tlb_is_invalid(tlb)) {
      tme_m68k_redispatch(ic);
    }
#endif /* _TME_M68K_EXECUTE_FAST */

  }
  /* NOTREACHED */

#ifdef _TME_M68K_EXECUTE_FAST

  /* if we get here, we "faulted" trying to fetch an instruction word
     from host memory.  it's possibly not a "real" fault, since this
     instruction may simply cross a page boundary, but since the fast
     executor can't restart instructions we have to treat this like a
     group 0 fault: */
 _tme_m68k_fast_fetch_failed:

  /* mimic a group 0 exception: */
  _TME_M68K_INSN_FETCH_SAVE;
  ic->_tme_m68k_group0_flags = TME_M68K_BUS_CYCLE_FETCH | TME_M68K_BUS_CYCLE_READ;
  ic->_tme_m68k_group0_function_code = function_code_program;
  ic->_tme_m68k_group0_address = ic->tme_m68k_ireg_pc + (fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start);
  ic->_tme_m68k_group0_sequence = ic->_tme_m68k_sequence;
  ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_faulted_after = 0;
  ic->_tme_m68k_group0_buffer_read_size = 0;
  ic->_tme_m68k_group0_buffer_read_softrr = 0;
  tme_m68k_group0_hook_fast(ic);
  ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_faulted =
    ic->_tme_m68k_group0_sequence._tme_m68k_sequence_transfer_next;

  /* mimic the rte: */
  ic->_tme_m68k_sequence = ic->_tme_m68k_group0_sequence;
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next = 1;
  TME_M68K_SEQUENCE_RESTART;

  tme_m68k_redispatch(ic);
  /* NOTREACHED */
#endif /* _TME_M68K_EXECUTE_FAST */
}
