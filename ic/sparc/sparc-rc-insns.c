/* $Id: sparc-rc-insns.c,v 1.8 2010/06/05 17:00:47 fredette Exp $ */

/* ic/sparc/sparc-rc-insns.c - SPARC recode instruction support: */

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

#if TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32

_TME_RCSID("$Id: sparc-rc-insns.c,v 1.8 2010/06/05 17:00:47 fredette Exp $");

/* macros: */

/* the sparc instruction recoder is mostly driven by the 32-bit
   control value kept in the sparc_recode variable.  this value
   includes various fields and flags, and is initially taken from the
   sparc instruction's entry in _tme_sparc_recode_insn_opmap[].  this
   entry provides only the least significant 16 bits; other
   more-significant bits may be added as recoding proceeds: */

/* the opcode for the recode instruction: */
#define TME_SPARC_RECODE_INSN_OPCODE(x)		((tme_uint8_t) (x))
#define TME_SPARC_RECODE_INSN_OPCODE_MASK	(0xff)

/* if set, the PC and PC_next registers must be valid for this sparc
   instruction, either because the sparc instruction itself needs them
   to be valid, or because the situation calls for them to be valid
   (for example, when TME_SPARC_RECODE_INSN_LAST is also set): */
#define TME_SPARC_RECODE_INSN_UPDATE_PCS	(1 << 8)

/* if set, this sparc instruction will be the last one recoded in this
   instructions thunk: */
#define TME_SPARC_RECODE_INSN_LAST		(1 << 9)

/* when TME_SPARC_RECODE_INSN_UPDATE_INSN is set, these flags are set
   if this sparc instruction doesn't take rs2/simm13 and/or rs1 source
   operands: */
#define TME_SPARC_RECODE_INSN_NO_RS2		(1 << 10)
#define TME_SPARC_RECODE_INSN_NO_RS1		(1 << 11)

/* when the recode opcode is TME_RECODE_OPCODE_GUEST, this flag is set
   if this sparc instruction doesn't write any sparc register visible
   to recode: */
#define TME_SPARC_RECODE_INSN_NO_RD		(1 << 12)

/* if set, the TME_SPARC_IREG_INSN register must be valid for this
   sparc instruction, usually because it uses a generic assist
   function that needs the instruction word: */
#define TME_SPARC_RECODE_INSN_UPDATE_INSN	(1 << 13)

/* if set, this is a shift instruction: */
#define TME_SPARC_RECODE_INSN_SHIFT		(1 << 14)

/* when TME_SPARC_RECODE_INSN_UPDATE_INSN is clear, this is set if
   this sparc instruction needs TME_RECODE_FLAG_CARRY set with the
   %icc.C flag: */
#define TME_SPARC_RECODE_INSN_DEFC		(TME_SPARC_RECODE_INSN_NO_RS2)

/* when TME_SPARC_RECODE_INSN_UPDATE_INSN is clear, this is set if
   this sparc instruction updates the sparc condition codes: */
#define TME_SPARC_RECODE_INSN_CC		(TME_SPARC_RECODE_INSN_NO_RS1)

/* NB: the following sparc_recode flags cannot be used in the 16-bit
   _tme_sparc_recode_insn_opmap[]: */

/* if set, this suppresses the normal recoding of the sparc
   instruction itself (although any TME_SPARC_RECODE_INSN_UPDATE_PCS
   will still be done): */
#define TME_SPARC_RECODE_INSN_NONE		(1 << 16)

/* if set, this is a v9 32-bit right shift: */
#define TME_SPARC_RECODE_INSN_SHIFT_RIGHT	(1 << 17)

/* if set, after the normal recoding of the sparc instruction, a
   recode endif will be emitted: */
#define TME_SPARC_RECODE_INSN_NEED_ENDIF	(1 << 18)

/* when TME_SPARC_RECODE_INSN_LAST is set, this is any recode chain
   information for the instructions thunk: */
#define TME_SPARC_RECODE_CHAIN_INFO(x)		((x) << 19)
#if (TME_RECODE_CHAIN_INFO_UNCONDITIONAL | TME_RECODE_CHAIN_INFO_CONDITIONAL | TME_RECODE_CHAIN_INFO_NEAR | TME_RECODE_CHAIN_INFO_FAR | TME_RECODE_CHAIN_INFO_JUMP | TME_RECODE_CHAIN_INFO_RETURN | TME_RECODE_CHAIN_INFO_CALL) > 0x1f
#error "TME_RECODE_CHAIN_INFO_ values changed"
#endif

/* these are composite values used in
   _tme_sparc_recode_insn_opmap[]: */

/* this sparc instruction needs an assist: */
#define TME_SPARC_RECODE_INSN_ASSIST		\
  (TME_RECODE_OPCODE_GUEST			\
   | TME_SPARC_RECODE_INSN_UPDATE_INSN)

/* this sparc instruction needs an assist, and can trap: */
#define TME_SPARC_RECODE_INSN_ASSIST_CANTRAP	\
  (TME_SPARC_RECODE_INSN_ASSIST			\
   | TME_SPARC_RECODE_INSN_UPDATE_PCS)

/* this sparc instruction needs an assist, can trap, and the assist
   function does not need source operands from the rs1 and rs2/simm13
   fields: */
#define TME_SPARC_RECODE_INSN_ASSIST_FULL	\
  (TME_SPARC_RECODE_INSN_ASSIST_CANTRAP		\
   | TME_SPARC_RECODE_INSN_NO_RS2		\
   | TME_SPARC_RECODE_INSN_NO_RS1)

/* this sparc instruction is undefined: */
#define TME_SPARC_RECODE_INSN_UNDEF		TME_SPARC_RECODE_INSN_ASSIST_FULL

/* this sparc instruction is a load: */
#define TME_SPARC_RECODE_INSN_LD		\
  (TME_SPARC_RECODE_INSN_UPDATE_PCS		\
   | TME_RECODE_OPCODE_RW)

/* this sparc instruction is a store: */
#define TME_SPARC_RECODE_INSN_ST		\
  (TME_SPARC_RECODE_INSN_UPDATE_PCS		\
   | TME_RECODE_OPCODE_RW)

/* this sparc instruction is a load alternate: */
#define TME_SPARC_RECODE_INSN_LDA		\
  (TME_SPARC_VERSION(ic) < 9			\
   ? TME_SPARC_RECODE_INSN_ASSIST_CANTRAP	\
   : TME_SPARC_RECODE_INSN_LD)

/* rename various things by the architecture size: */
#define tme_sparc_idle_type_pc _TME_SPARC_RECODE_SIZE(tme_sparc_idle_type_pc,/**/)
#define _tme_sparc_recode_insn_opmap _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_opmap)
#define _tme_sparc_execute_opmap _TME_SPARC_RECODE_SIZE(_tme_sparc,_execute_opmap)
#define tme_sparc_recode_insn_current _TME_SPARC_RECODE_SIZE(tme_sparc,_recode_insn_current)
#define tme_sparc_recode_insn_assist_redispatch _TME_SPARC_RECODE_SIZE(tme_sparc,_recode_insn_assist_redispatch)
#define _tme_sparc_recode_insn_assist _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_assist)
#define _tme_sparc_recode_insn_assist_full _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_assist_full)
#define _tme_sparc_recode_insn_assist_jmpl _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_assist_jmpl)
#define _tme_sparc_recode_insn_assist_unimpl _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_assist_unimpl)
#define _tme_sparc_recode_insns_total _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insns_total)
#define _tme_sparc_recode_insn_branch_delay_bad _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_insn_branch_delay_bad)
#define _tme_sparc_recode_recode _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_recode)

#endif /* TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32 */

/* this maps format three opcodes into sparc recode controls: */
static const tme_uint16_t _tme_sparc_recode_insn_opmap[128] = {

  /* op=2: arithmetic, logical, shift, and remaining: */

  /* 000000 */ TME_RECODE_OPCODE_ADD,
  /* 000001 */ TME_RECODE_OPCODE_AND,
  /* 000010 */ TME_RECODE_OPCODE_OR,
  /* 000011 */ TME_RECODE_OPCODE_XOR,
  /* 000100 */ TME_RECODE_OPCODE_SUB,
  /* 000101 */ TME_RECODE_OPCODE_ANDN,
  /* 000110 */ TME_RECODE_OPCODE_ORN,
  /* 000111 */ TME_RECODE_OPCODE_XORN,
  /* 001000 */ TME_RECODE_OPCODE_ADDC | TME_SPARC_RECODE_INSN_DEFC,
#if TME_SPARC_VERSION(ic) < 9
  /* 001001 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101001 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: mulx */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001010 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* umul */
  /* 001011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* smul */
  /* 001100 */ TME_RECODE_OPCODE_SUBC | TME_SPARC_RECODE_INSN_DEFC,
#if TME_SPARC_VERSION(ic) < 9
  /* 001101 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: udivx */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* udiv */
  /* 001111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* sdiv */
  /* 010000 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_ADD,
  /* 010001 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_AND,
  /* 010010 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_OR,
  /* 010011 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_XOR,
  /* 010100 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_SUB,
  /* 010101 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_ANDN,
  /* 010110 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_ORN,
  /* 010111 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_XORN,
  /* 011000 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_ADDC | TME_SPARC_RECODE_INSN_DEFC,
  /* 011001 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 011010 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* umulcc */
  /* 011011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* smulcc */
  /* 011100 */ TME_SPARC_RECODE_INSN_CC | TME_RECODE_OPCODE_SUBC | TME_SPARC_RECODE_INSN_DEFC,
  /* 011101 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 011110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* udivcc */
  /* 011111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* sdivcc */
  /* 100000 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* taddcc */
  /* 100001 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* tsubcc */
  /* 100010 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* taddcctv */
  /* 100011 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* tsubcctv */
  /* 100100 */ TME_SPARC_RECODE_INSN_ASSIST, /* mulscc */
  /* 100101 */ TME_RECODE_OPCODE_SHLL | TME_SPARC_RECODE_INSN_SHIFT,
  /* 100110 */ TME_RECODE_OPCODE_SHRL | TME_SPARC_RECODE_INSN_SHIFT,
  /* 100111 */ TME_RECODE_OPCODE_SHRA | TME_SPARC_RECODE_INSN_SHIFT,
  /* 101000 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* rdasr */
#if TME_SPARC_VERSION(ic) < 9
  /* 101001 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* rdpsr */
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101001 */ TME_SPARC_RECODE_INSN_UNDEF,
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101010 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* rdwim (v9: rdpr) */
  /* 101011 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* rdtbr (v9: flushw) */
#if TME_SPARC_VERSION(ic) < 9
  /* 101100 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101101 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101110 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101111 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101100 */ 0, /* v9: movcc (handled specially) */
  /* 101101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: sdivx */
  /* 101110 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP, /* v9: popc */
  /* 101111 */ 0, /* v9: movr (handled specially) */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110000 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* wrasr */
#if TME_SPARC_VERSION(ic) < 9
  /* 110001 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* wrpsr */
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110001 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: saved/restored */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110010 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* wrwim (v9: wrpr) */
#if TME_SPARC_VERSION(ic) < 9
  /* 110011 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* wrtbr */
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110011 */ TME_SPARC_RECODE_INSN_UNDEF,
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* fpop1 */
  /* 110101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* fpop2 */
  /* 110110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* cpop1 (v9: impdep1) */
  /* 110111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* cpop2 (v9: impdep2) */
  /* 111000 */ 0, /* jmpl (handled specially) */
  /* 111001 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_LAST, /* rett (v9: return) */
  /* 111010 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* ticc */
  /* 111011 */ TME_SPARC_RECODE_INSN_ASSIST | TME_SPARC_RECODE_INSN_NO_RD, /* flush */
  /* 111100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* save */
  /* 111101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* restore */
#if TME_SPARC_VERSION(ic) < 9
  /* 111110 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 111110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_LAST, /* v9: done/retry */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 111111 */ TME_SPARC_RECODE_INSN_UNDEF,

  /* op=3: memory instructions: */

  /* 000000 */ TME_SPARC_RECODE_INSN_LD, /* ld (v9: lduw) */
  /* 000001 */ TME_SPARC_RECODE_INSN_LD, /* ldub */
  /* 000010 */ TME_SPARC_RECODE_INSN_LD, /* lduh */
  /* 000011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL,  /* ldd */
  /* 000100 */ TME_SPARC_RECODE_INSN_ST, /* st (v9: stw) */
  /* 000101 */ TME_SPARC_RECODE_INSN_ST, /* stb */
  /* 000110 */ TME_SPARC_RECODE_INSN_ST, /* sth */
  /* 000111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD,  /* std */
#if TME_SPARC_VERSION(ic) < 9
  /* 001000 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001000 */ TME_SPARC_RECODE_INSN_LD, /* v9: ldsw */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001001 */ TME_SPARC_RECODE_INSN_LD, /* ldsb */
  /* 001010 */ TME_SPARC_RECODE_INSN_LD, /* ldsh */
#if TME_SPARC_VERSION(ic) < 9
  /* 001011 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001011 */ TME_SPARC_RECODE_INSN_LD, /* v9: ldx */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001100 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 001101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldstub */
#if TME_SPARC_VERSION(ic) < 9
  /* 001110 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001110 */ TME_SPARC_RECODE_INSN_ST, /* v9: stx */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* swap */
  /* 010000 */ TME_SPARC_RECODE_INSN_LDA, /* lda (v9: lduwa) */
  /* 010001 */ TME_SPARC_RECODE_INSN_LDA, /* lduba */
  /* 010010 */ TME_SPARC_RECODE_INSN_LDA, /* lduha */
  /* 010011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL,  /* ldda */
  /* 010100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD, /* sta (v9: stwa) */
  /* 010101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD, /* stba */
  /* 010110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD, /* stha */
  /* 010111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD, /* stda */
#if TME_SPARC_VERSION(ic) < 9
  /* 011000 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 011000 */ TME_SPARC_RECODE_INSN_LDA, /* v9: ldswa*/
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 011001 */ TME_SPARC_RECODE_INSN_LDA, /* ldsba */
  /* 011010 */ TME_SPARC_RECODE_INSN_LDA, /* ldsha */
#if TME_SPARC_VERSION(ic) < 9
  /* 011011 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001011 */ TME_SPARC_RECODE_INSN_LDA, /* v9: ldxa*/
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 011100 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 011101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldstuba */
#if TME_SPARC_VERSION(ic) < 9
  /* 011110 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 001110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_NO_RD, /* v9: stxa */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 011111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* swapa */
  /* 100000 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldf */
  /* 100001 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldfsr */
#if TME_SPARC_VERSION(ic) < 9
  /* 100010 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 100010 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: ldqf */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 100011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* lddf */
  /* 100100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stf */
  /* 100101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stfsr */
  /* 100110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stdfq */
  /* 100111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stdf */
  /* 101000 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101001 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101010 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101011 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101100 */ TME_SPARC_RECODE_INSN_UNDEF,
#if TME_SPARC_VERSION(ic) < 9
  /* 101101 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101101 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* v9: prefetch */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 101110 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 101111 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 110000 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldc (v9: ldfa) */
#if TME_SPARC_VERSION(ic) < 9
  /* 110001 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* ldcsr */
  /* 110010 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110001 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 110010 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: ldqfa */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110011 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* lddc (v9: lddfa) */
  /* 110100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stc (v9: stfa) */
#if TME_SPARC_VERSION(ic) < 9
  /* 110101 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stcsr */
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110101 */ TME_SPARC_RECODE_INSN_UNDEF,
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 110110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stdcq (v9: stqfa) */
  /* 110111 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* stdc (v9: stdfa) */
  /* 111000 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 111001 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 111010 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 111011 */ TME_SPARC_RECODE_INSN_UNDEF,
#if TME_SPARC_VERSION(ic) < 9
  /* 111100 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 111101 */ TME_SPARC_RECODE_INSN_UNDEF,
  /* 111110 */ TME_SPARC_RECODE_INSN_UNDEF,
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  /* 111100 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: casa */
  /* 111101 */ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP | TME_SPARC_RECODE_INSN_NO_RD, /* v9: prefetch */
  /* 111110 */ TME_SPARC_RECODE_INSN_ASSIST_FULL, /* v9: casxa */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  /* 111111 */ TME_SPARC_RECODE_INSN_UNDEF,
};

/* this returns the current instruction: */
tme_uint32_t
tme_sparc_recode_insn_current(const struct tme_sparc *ic)
{
  const struct tme_token *itlb_current_token;
  const struct _TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,/**/) *recode_itlb_current;
  const struct tme_sparc_tlb *itlb_current;
  tme_uint32_t page_offset;
  const tme_shared tme_uint32_t *_sparc_insn;
  tme_uint32_t sparc_insn;

  /* in a cooperative threading system, the current ITLB entry must be
     valid.  in a noncooperative threading system, some master may be
     in the process of invalidating the current ITLB entry: */
  itlb_current_token = ic->_tme_sparc_itlb_current_token;
  assert (!TME_THREADS_COOPERATIVE || tme_token_is_valid(itlb_current_token));

  /* get the current recode ITLB entry: */
  recode_itlb_current
    = ((struct _TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,/**/) *)
       (((const char *) itlb_current_token)
	- (((char *) &(((struct _TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,/**/) *) 0)->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_token)))
	   - (char *) 0)));

  /* the current ITLB entry must cover the current context,
     instruction ASI and PC, and allow fast reading: */
  itlb_current = &ic->tme_sparc_tlbs[(recode_itlb_current - &ic->_TME_SPARC_RECODE_SIZE(tme_sparc_recode_tlb,s)[0])];
  assert ((itlb_current->tme_sparc_tlb_context > ic->tme_sparc_memory_context_max
	   || itlb_current->tme_sparc_tlb_context == ic->tme_sparc_memory_context_default)
	  && TME_SPARC_TLB_ASI_MASK_OK(itlb_current, ic->tme_sparc_asi_mask_insn)
	  && itlb_current->tme_sparc_tlb_addr_first <= ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
	  && ic->tme_sparc_ireg(TME_SPARC_IREG_PC) <= itlb_current->tme_sparc_tlb_addr_last
	  && itlb_current->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF);

  /* fetch the instruction: */
  page_offset = recode_itlb_current->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_page);
  page_offset ^= (tme_uint32_t) ic->tme_sparc_ireg(TME_SPARC_IREG_PC);
  assert ((page_offset >> ic->tme_sparc_tlb_page_size_log2) == 0);
  _sparc_insn
    = ((const tme_shared tme_uint32_t *)
       (recode_itlb_current->_TME_SPARC_RECODE_SIZE(tme_recode_tlb_c16_a,_memory)
	+ page_offset));
  sparc_insn
    = tme_memory_bus_read32(_sparc_insn,
			    (tme_rwlock_t *) NULL,
			    sizeof(tme_uint32_t),
			    sizeof(tme_sparc_ireg_t));
  sparc_insn = tme_betoh_u32(sparc_insn);

  return (sparc_insn);
}

/* this checks for conditions that require a redispatch out of recode
   instruction thunks: */
tme_recode_uguest_t
tme_sparc_recode_insn_assist_redispatch(struct tme_sparc *ic)
{
  int redispatch_required;

  /* if the instruction TLB entry for this PC is invalid, or doesn't
     cover the PC and context and ASI, or doesn't allow fast reading,
     or isn't covered by any cacheable, or the page containing this PC
     isn't still cache-valid (i.e., its contents have changed since we
     cached recoded instructions for it), a redispatch is required: */
  /* NB: we check for the current PC, even though it's for an
     instruction that has already completed.  

     if PC and PC_next are on the same page, this does the right
     thing.  if PC and PC_next are on different pages, and the PC page
     requires a redispatch but the PC_next page does not, this will
     cause a needless redispatch.  if PC and PC_next are on different
     pages, and the PC page doesn't require a redispatch but the
     PC_next page does, that will get handled by either
     _TME_SPARC_EXECUTE_NAME()/tme_sparc_recode() or the host chain
     out/chain in, because the current instructions thunk can't
     cover both PC and PC_next.

     so, checking for the current PC never does the wrong thing, and
     is useful for at least _tme_sparc_recode_ls_assist_ld(), which
     must redispatch after executing a ld instruction that might be
     different from the ld instruction at the time the instructions
     thunk was recoded - because if the destination register has been
     changed in the new instruction, if
     _tme_sparc_recode_ls_assist_ld() did return to the instructions
     thunk, it would incorrectly take the return value for the
     destination register in the original instruction: */
  redispatch_required
    = (_tme_sparc_recode_chain_src_key(ic,
				       ic->tme_sparc_ireg(TME_SPARC_IREG_PC))
       == TME_SPARC_RECODE_SRC_KEY_UNDEF);

  /* if the instruction burst has been set to zero by a
     TME_BUS_CYCLE_SYNCHRONOUS_EVENT, a redispatch is required: */
  redispatch_required |= (ic->_tme_sparc_instruction_burst_remaining == 0);

  /* if a redispatch is required: */
  if (__tme_predict_false(redispatch_required)) {

    /* do the redispatch: */
    tme_sparc_redispatch(ic);
  }

  return (0);
}

/* the assist functions for format three instructions reference the
   opmap in struct tme_sparc: */

/* the assist function for TME_SPARC_RECODE_INSN_ASSIST and
   TME_SPARC_RECODE_INSN_ASSIST_CANTRAP instructions: */
static tme_recode_uguest_t
_tme_sparc_recode_insn_assist(struct tme_ic *_ic,
			      tme_recode_uguest_t rs1,
			      tme_recode_uguest_t rs2)
{
  struct tme_sparc *ic;
  tme_uint32_t sparc_insn;
  tme_uint32_t sparc_opcode;
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  tme_sparc_ireg_t rs1_buffer;
  tme_sparc_ireg_t rs2_buffer;
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic) */
  tme_sparc_ireg_t *_rs1;
  tme_sparc_ireg_t *_rs2;
  tme_sparc_ireg_t rd;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist);

  /* set PC_next_next from PC_next: */
  /* NB that this sets PC_next_next to garbage unless this instruction
     was a TME_SPARC_RECODE_INSN_ASSIST_CANTRAP: */
  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
       + sizeof(tme_uint32_t));

  /* copy the instruction from the internal register into its normal
     position: */
  sparc_insn = ic->tme_sparc_ireg(TME_SPARC_IREG_INSN);
  ic->_tme_sparc_insn = sparc_insn;

  /* form the opcode index: */
  sparc_opcode = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));
  sparc_opcode += ((sparc_insn >> (30 - 6)) & 0x40);
  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_opcode[sparc_opcode]);

  /* run the instruction: */
#if TME_RECODE_SIZE_GUEST_MAX > TME_SPARC_RECODE_SIZE(ic)
  rs1_buffer = rs1;
  rs2_buffer = rs2;
  _rs1 = &rs1_buffer;
  _rs2 = &rs2_buffer;
#else  /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
  _rs1 = &rs1;
  _rs2 = &rs2;
#endif /* TME_RECODE_SIZE_GUEST_MAX == TME_SPARC_RECODE_SIZE(ic) */
#ifdef _TME_SPARC_RECODE_VERIFY
  rd = 0;
#endif /* _TME_SPARC_RECODE_VERIFY */
  (*ic->_tme_sparc_execute_opmap[sparc_opcode])
    (ic, _rs1, _rs2, &rd);

  /* return the result: */
  return (rd);
}

/* the assist function for TME_SPARC_RECODE_INSN_ASSIST_FULL
   instructions: */
static tme_recode_uguest_t
_tme_sparc_recode_insn_assist_full(struct tme_ic *_ic,
				   tme_recode_uguest_t _rs1,
				   tme_recode_uguest_t _rs2)
{
  struct tme_sparc *ic;
  tme_uint32_t sparc_insn;
  unsigned int reg_rs1;
  unsigned int reg_rs2;
  unsigned int reg_rd;
  tme_uint32_t sparc_opcode;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_full);

  /* set PC_next_next from PC_next: */
  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
       + sizeof(tme_uint32_t));

  /* copy the instruction from the internal register into its normal
     position: */
  sparc_insn = ic->tme_sparc_ireg(TME_SPARC_IREG_INSN);
  ic->_tme_sparc_insn = sparc_insn;

  /* if the i bit is zero: */
  if (__tme_predict_true((sparc_insn & TME_BIT(13)) == 0)) {

    /* decode rs2: */
    reg_rs2 = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2);
    TME_SPARC_REG_INDEX(ic, reg_rs2);
  }

  /* otherwise, the i bit is one: */
  else {

    /* decode simm13: */
    ic->tme_sparc_ireg(TME_SPARC_IREG_TMP(0)) = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x1fff);
    reg_rs2 = TME_SPARC_IREG_TMP(0);
  }

  /* decode rs1: */
  reg_rs1 = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1);
  TME_SPARC_REG_INDEX(ic, reg_rs1);

  /* decode rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* form the opcode index: */
  sparc_opcode = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));
  sparc_opcode += ((sparc_insn >> (30 - 6)) & 0x40);
  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_opcode[sparc_opcode]);

  /* run the instruction: */
  (*ic->_tme_sparc_execute_opmap[sparc_opcode])
    (ic,
     &ic->tme_sparc_ireg(reg_rs1),
     &ic->tme_sparc_ireg(reg_rs2),
     &ic->tme_sparc_ireg(reg_rd));

  /* set %g0 to zero, in case the instruction changed it: */
  ic->tme_sparc_ireg(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G0) = 0;

  /* return no result */
  return (tme_sparc_recode_insn_assist_redispatch(ic));

  /* unused: */
  _rs1 = 0;
  _rs2 = 0;
}

/* the assist function for jmpl instructions: */
static tme_recode_uguest_t
_tme_sparc_recode_insn_assist_jmpl(struct tme_ic *_ic,
				   tme_recode_uguest_t rs1,
				   tme_recode_uguest_t rs2)
{
  struct tme_sparc *ic;
  tme_sparc_ireg_t pc_next_next;
  tme_uint32_t ls_faults;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  TME_SPARC_STAT(ic, tme_sparc_stats_recode_assist_opcode[0x38 /* jmpl */]);

  /* get the target address: */
  pc_next_next = ((tme_sparc_ireg_t) rs1) + (tme_sparc_ireg_t) rs2;

  /* assume that the target address will not fault: */
  ls_faults = TME_SPARC_LS_FAULT_NONE;

#if TME_SPARC_VERSION(ic) >= 9

  /* mask the target address: */
  pc_next_next &= ic->tme_sparc_address_mask;

  /* if the target address is in a virtual address hole: */
  if (__tme_predict_false((pc_next_next
                           + ic->tme_sparc64_ireg_va_hole_start)
                          > ((ic->tme_sparc64_ireg_va_hole_start * 2) - 1))) {

    /* the target address is out of range: */
    ls_faults += TME_SPARC64_LS_FAULT_VA_RANGE_NNPC;
  }

#endif /* TME_SPARC_VERSION(ic) >= 9 */

  /* if the target address is not 32-bit aligned: */
  if (__tme_predict_false((((tme_uint32_t) pc_next_next) % sizeof(tme_uint32_t)) != 0)) {

    /* the target address is misaligned: */
    ls_faults += TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;
  }

  /* if the target address has faulted: */
  if (__tme_predict_false(ls_faults != TME_SPARC_LS_FAULT_NONE)) {

    /* set PC_next from PC: */
    /* NB: we don't need to set PC_next_next, because we're
       faulting: */
    ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
      = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC)
	 + sizeof(tme_uint32_t));

    /* in case a CPU-specific trap function needs it, store a dummy
       jmpl instruction: */
    ic->_tme_sparc_insn
      = ((((tme_uint32_t) 2) << 30)
	 + (0x38 << 19)	/* jmpl */
	 );

    /* fault: */
    tme_sparc_nnpc_trap(ic, ls_faults);
  }

  /* return the target address: */
  return (pc_next_next);
}

/* the assist function for unimplemented instructions: */
static tme_recode_uguest_t
_tme_sparc_recode_insn_assist_unimpl(struct tme_ic *_ic,
				     tme_recode_uguest_t _rs1,
				     tme_recode_uguest_t _rs2)
{
  struct tme_sparc *ic;

  /* recover our ic: */
  ic = (struct tme_sparc *) _ic;

  /* set PC_next_next from PC_next: */
  ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT_NEXT)
    = (ic->tme_sparc_ireg(TME_SPARC_IREG_PC_NEXT)
       + sizeof(tme_uint32_t));

  /* trap: */
#if TME_SPARC_VERSION(ic) < 9
  tme_sparc32_trap(ic, TME_SPARC32_TRAP_illegal_instruction);
#else  /* TME_SPARC_VERSION(ic) >= 9 */
  tme_sparc64_trap(ic, TME_SPARC64_TRAP_illegal_instruction);
#endif /* TME_SPARC_VERSION(ic) >= 9 */

  /* return no result */
  return (0);

  /* unused: */
  _rs1 = 0;
  _rs2 = 0;
}

#ifdef _TME_SPARC_STATS
static tme_recode_uguest_t
_tme_sparc_recode_insns_total(struct tme_ic *_ic,
			      tme_recode_uguest_t _rs1,
			      tme_recode_uguest_t _rs2)
{
  TME_SPARC_STAT_N(((struct tme_sparc *) _ic),
		   tme_sparc_stats_recode_insns_total, 
		   (tme_uint32_t) _rs2);
  return (0);
}
#endif /* _TME_SPARC_STATS */

/* this returns nonzero if the given instruction can't be recoded in a
   branch delay slot.  in general, an instruction can't be recoded in
   a branch delay slot if it may set PC_next_next without doing a
   redispatch, or if it may use a recode if/endif: */
static int
_tme_sparc_recode_insn_branch_delay_bad(tme_uint32_t sparc_insn)
{
  unsigned int sparc_opcode;

  /* if this is a format three instruction, and op is three: */
  if (sparc_insn >= 0xc0000000) {

    /* all of the instructions with op equal to three can be recoded
       in a branch delay slot: */
    return (FALSE);
  }

  /* otherwise, if this is a format three instruction, and op is
     two: */
  else if (sparc_insn >= 0x80000000) {

    /* form the opcode index: */
    sparc_opcode = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));

    /* instructions that may set PC_next_next without doing a
       redispatch, and instructions that may use a recode if/endif
       can't be recoded in a branch delay slot: */
    return (sparc_opcode == 0x38		/* jmpl */
	    || (TME_SPARC_VERSION(ic) >= 9
		&& (sparc_opcode == 0x2c	/* v9: movcc */
		    || sparc_opcode == 0x2f)));	/* v9: movr */
  }

  /* otherwise, if this is a format two instruction: */
  else if (__tme_predict_true(sparc_insn < 0x40000000)) {

    /* except for sethi instructions, format two instructions can't be
       recoded in a branch delay slot - the other implemented format
       two instructions are all branches that set PC_next_next: */
    return ((sparc_insn & (0x7 << 22)) != (0x4 << 22));
  }

  /* otherwise, this is a format three instruction: */
  else {

    /* we can't recode call instructions in branch delay slots, since
       they set PC_next_next: */
    return (TRUE);
  }
}

/* the sparc instruction recoder: */
static tme_recode_thunk_off_t
_tme_sparc_recode_recode(struct tme_sparc *ic,
			 const struct tme_sparc_tlb *itlb)
{
  tme_uint32_t page_size;
  unsigned int reg_guest_pc;
  unsigned int recode_size_address;
  struct tme_recode_insn *recode_insn;
  struct tme_recode_insn *recode_insns_last_start;
  tme_sparc_ireg_t pc_next;
  tme_uint32_t sparc_insn_next;
  tme_uint32_t sparc_insn_count;
  tme_uint32_t pc_advance;
  tme_uint32_t sparc_recode;
  tme_sparc_ireg_t itlb_addr_last;
  tme_recode_uguest_t (*sparc_assist) _TME_P((struct tme_ic *, tme_recode_uguest_t, tme_recode_uguest_t));
  tme_uint32_t sparc_insn;
  tme_uint32_t sparc_opcode;
  int recode_operand;
  tme_sparc_ireg_t imm;
  unsigned int shift_count_mask;
  unsigned int cond;
  const struct tme_recode_conds_thunk *conds_thunk;
  tme_int32_t branch_displacement_raw;
  tme_sparc_ireg_t branch_displacement;
  const struct tme_recode_flags_thunk *flags_thunk;
  int recode_operand_address;
  tme_uint32_t chain_info;

  /* get the page size: */
  page_size = (((tme_uint32_t) 1) << ic->tme_sparc_tlb_page_size_log2);

  /* on entry to this function, PC has already been updated to be the
     first instruction.  on entry to an instructions thunk, PC hasn't
     been updated yet, and PC_next is the first instruction: */
  reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);

  /* assume that addresses are guest size: */
  recode_size_address = TME_SPARC_RECODE_SIZE(ic);

  /* if this is a v9 CPU and PSTATE.AM is set: */
  if (TME_SPARC_VERSION(ic) >= 9
      && (ic->tme_sparc64_ireg_pstate
	  & TME_SPARC64_PSTATE_AM)) {

    /* addresses are only 32 bits wide: */
    recode_size_address = TME_RECODE_SIZE_32;
  }

  /* start at the beginning of the recode instructions buffer: */
  recode_insn = &ic->tme_sparc_recode_insns[0];

  /* calculate the worst-case position in the recode instructions
     buffer, past which we may not be able to recode another sparc
     instruction.  this is actually an upper bound on the worst case,
     found by counting the number of "recode_insn++" statements in
     this function, and multiplying that by two so that branch
     instructions can loop to recode a branch delay slot: */
  recode_insns_last_start = &ic->tme_sparc_recode_insns[TME_ARRAY_ELS(ic->tme_sparc_recode_insns) - (35 * 2)];

  /* fetch the first instruction as the next instruction: */
  pc_next = ic->tme_sparc_ireg(TME_SPARC_IREG_PC);
  sparc_insn_next
    = tme_memory_bus_read32(((const tme_shared tme_uint32_t *)
			     (itlb->tme_sparc_tlb_emulator_off_read
			      + pc_next)),
			    itlb->tme_sparc_tlb_bus_rwlock,
			    sizeof(tme_uint32_t),
			    sizeof(tme_sparc_ireg_t));
  sparc_insn_next = tme_betoh_u32(sparc_insn_next);
  sparc_insn_count = 0;

  /* the next instruction will be the first instruction: */
  pc_advance = 0 - (tme_uint32_t) sizeof(sparc_insn_next);

  /* clear the recode information: */
  sparc_recode = 0;

  /* if PC is at the last instruction address before tme_sparc_ireg_t
     wrapping: */
  /* NB: we don't consider the v9 address mask here, because we only
     need to protect against the pc_next local variable itself
     wrapping its tme_sparc_ireg_t.  if the v9 address mask is set for
     32 bits and PC is 0xfffffffc, pc_next will not wrap to zero -
     instead it will advance to 0x100000000, but we also won't
     continue to recode because that address is on a different
     page: */
  assert (TME_SPARC_VERSION(ic) < 9
	  || (pc_next < ic->tme_sparc_address_mask));
  if (__tme_predict_false(pc_next
			  == (tme_sparc_ireg_t) (0 - (tme_sparc_ireg_t) sizeof(tme_uint32_t)))) {

    /* recode is impossible here: */
    return (0);
  }

  /* stop recoding after the last instruction on the page, or right
     before the last instruction before tme_sparc_ireg_t wrapping,
     whichever comes first: */
  itlb_addr_last
    = ((pc_next
	| (page_size - 1))
       - (sizeof(tme_uint32_t) - 1));
  if (__tme_predict_false(itlb_addr_last
			  == (tme_sparc_ireg_t) (0 - (tme_sparc_ireg_t) sizeof(tme_uint32_t)))) {
    itlb_addr_last -= sizeof(tme_uint32_t);
  }
  assert (itlb_addr_last >= pc_next);
  assert (itlb->tme_sparc_tlb_addr_last >= itlb_addr_last);

  /* loop recoding instructions: */
  for (;;) {

    /* reset the assist function: */
    sparc_assist = _tme_sparc_recode_insn_assist;

    /* make the next instruction the current instruction: */
    pc_advance += sizeof(sparc_insn);
    sparc_insn = sparc_insn_next;
    sparc_insn_count++;

    /* advance the PC of the next instruction: */
    pc_next += sizeof(sparc_insn);

    /* if the PC of the next instruction is still in this
       page: */
    if (__tme_predict_true(pc_next <= itlb_addr_last)) {

      /* fetch the next instruction: */
      sparc_insn_next
	= tme_memory_bus_read32(((const tme_shared tme_uint32_t *)
				 (itlb->tme_sparc_tlb_emulator_off_read
				  + pc_next)),
				itlb->tme_sparc_tlb_bus_rwlock,
				sizeof(tme_uint32_t),
				sizeof(tme_sparc_ireg_t));
      sparc_insn_next = tme_betoh_u32(sparc_insn_next);
    }

    /* otherwise, the instruction TLB entry doesn't cover the PC of
       the next instruction: */
    else {

      /* the current instruction is the last instruction, and we need
	 to update the PC registers: */
      sparc_recode |= TME_SPARC_RECODE_INSN_LAST | TME_SPARC_RECODE_INSN_UPDATE_PCS;

      /* in case the current instruction is a control transfer
	 instruction, poison the next instruction to look like another
	 control transfer instruction.  we use a call instruction: */
      sparc_insn_next = 0x40000000;
    }

    /* if this is a format three instruction (op is two or three): */
    if (__tme_predict_true(sparc_insn >= 0x80000000)) {

      /* form the opcode index: */
      sparc_opcode = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x3f << 19));
      sparc_opcode += ((sparc_insn >> (30 - 6)) & 0x40);

      /* add in the recode information for this instruction: */
      sparc_recode |= _tme_sparc_recode_insn_opmap[sparc_opcode];

      /* if this is a jmpl instruction: */
      if (__tme_predict_false(sparc_opcode == 0x38)) {

	/* if the next instruction can't be recoded when it's in a
	   branch delay slot (usually because it's also a control
	   transfer instruction): */
	if (__tme_predict_false(_tme_sparc_recode_insn_branch_delay_bad(sparc_insn_next))) {

	  /* if we haven't recoded any instructions yet, recode is
	     impossible here: */
	  if (recode_insn == &ic->tme_sparc_recode_insns[0]) {
	    return (0);
	  }
	  
	  /* stop recoding immediately, leaving the updated PC
	     pointing to the instruction before the jmpl: */
	  assert (pc_advance >= sizeof(sparc_insn));
	  pc_advance -= sizeof(sparc_insn);
	  sparc_insn_count--;
	  sparc_recode
	    |= (TME_SPARC_RECODE_INSN_UPDATE_PCS
		| TME_SPARC_RECODE_INSN_NONE
		| TME_SPARC_RECODE_INSN_LAST);
	}

	/* otherwise, the next instruction can be recoded when it's in
	   a branch delay slot: */
	else {

	  /* if PC doesn't already point to the jmpl instruction: */
	  if (pc_advance != 0
	      || reg_guest_pc != TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC)) {

	    /* advance PC to the jmpl instruction: */
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	    recode_insn->tme_recode_insn_size = recode_size_address;
	    recode_insn->tme_recode_insn_operand_src[0] = reg_guest_pc;
	    recode_insn->tme_recode_insn_operand_src[1]
	      = (pc_advance == 0
		 ? TME_RECODE_OPERAND_ZERO
		 : TME_RECODE_OPERAND_IMM);
	    recode_insn->tme_recode_insn_imm_uguest = pc_advance;
	    recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	    recode_insn->tme_recode_insn_flags_thunk = NULL;
	    recode_insn++;
	  }

	  /* if rs1 is %i7 or %o7, assume that this jmpl is doing a
	     return, otherwise assume that this jmpl is doing a
	     call.  either way, the branch delay slot will be the
	     last instruction: */
	  recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1));
	  sparc_recode
	    |= ((((recode_operand
		   - TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0))
		  & 15 /* %o7 */)
		 == 15 /* %o7 */)
		? (TME_SPARC_RECODE_INSN_LAST
		   + TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_RETURN
						 + TME_RECODE_CHAIN_INFO_FAR
						 + TME_RECODE_CHAIN_INFO_UNCONDITIONAL))
		: (TME_SPARC_RECODE_INSN_LAST
		   + TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_CALL
						 + TME_RECODE_CHAIN_INFO_FAR
						 + TME_RECODE_CHAIN_INFO_UNCONDITIONAL)));

	  /* call the jmpl assist function.  if this doesn't trap, it
	     returns the target address for PC_next_next, which we put
	     directly into PC_next: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_GUEST;
	  recode_insn->tme_recode_insn_size = recode_size_address;
	  recode_insn->tme_recode_insn_operand_src[0]
	    = (recode_operand == TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0)
	       ? TME_RECODE_OPERAND_ZERO
	       : recode_operand);
	  if (__tme_predict_true(sparc_insn & TME_BIT(13))) {
	    recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	    recode_insn->tme_recode_insn_imm_uguest
	      = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x1fff);
	  }
	  else {
	    recode_insn->tme_recode_insn_operand_src[1]
	      = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2));
	  }
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	  recode_insn->tme_recode_insn_guest_func = _tme_sparc_recode_insn_assist_jmpl;
	  recode_insn++;

	  /* assume that rd is %g0: */
	  reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);

	  /* decode rd: */
	  recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD));

	  /* if rd is not %g0: */
	  if (recode_operand != TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0)) {

	    /* we will soon be able to find the PC of the jmpl in rd: */
	    reg_guest_pc = recode_operand;

	    /* write the PC of the jmpl into the destination register: */
	    /* NB: we don't use recode_size_address here because we need to
	       update the entire destination register: */
	    recode_insn->tme_recode_insn_operand_dst = recode_operand;
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	    recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	    recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	    recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	    recode_insn->tme_recode_insn_flags_thunk = NULL;
	    recode_insn++;
	  }

	  /* advance PC to the branch delay slot: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = recode_size_address;
	  recode_insn->tme_recode_insn_operand_src[0] = reg_guest_pc;
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	  recode_insn->tme_recode_insn_imm_uguest = sizeof(sparc_insn);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;
	  pc_advance = 0 - (tme_uint32_t) sizeof(sparc_insn);
	  reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);

	  /* loop now to fetch and recode the instruction in the branch
	     delay slot: */
	  continue;
	}
      }

      /* otherwise, if this is a v9 movcc instruction: */
      else if (__tme_predict_false(TME_SPARC_VERSION(ic) >= 9
				   && sparc_opcode == 0x2c)) {

	/* if cc2 is clear, or if cc1 is set, this is not an integer
	   movcc: */
	if (__tme_predict_false((sparc_insn
				 & (TME_BIT(18)
				    | TME_BIT(11)))
				!= TME_BIT(18))) {

	  /* this instruction needs a full assist: */
	  sparc_recode |= TME_SPARC_RECODE_INSN_ASSIST_FULL;
	}

	/* otherwise, this is an integer movcc: */
	else {

	  /* get the condition field: */
	  cond = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0xf << 14));

	  /* get the conditions thunk to test: */
	  conds_thunk
	    = ((sparc_insn & TME_BIT(12))
	       ? ic->tme_sparc_recode_conds_thunk_xcc
	       : ic->tme_sparc_recode_conds_thunk_icc);

	  /* define the first recode flag with the condition: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_DEFC;
	  recode_insn->tme_recode_insn_operand_src[0] = (cond % TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_src[1] = 0 - (int) (cond / TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_FLAG(0);
	  recode_insn->tme_recode_insn_conds_thunk = conds_thunk;
	  recode_insn++;

	  /* emit the if to test the first recode carry flag: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_IF;
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_FLAG(0);
	  recode_insn++;

	  /* move the source operand to the destination register: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	  if (sparc_insn & TME_BIT(13)) {
	    recode_insn->tme_recode_insn_imm_uguest
	      = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x7ff);
	    recode_operand = TME_RECODE_OPERAND_IMM;
	  }
	  else {
	    recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2));
	  }
	  recode_insn->tme_recode_insn_operand_src[1] = recode_operand;
	  recode_insn->tme_recode_insn_operand_dst
	    = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD));
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;

	  /* emit the endif: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ENDIF;
	  recode_insn++;

	  /* suppress any instructions below: */
	  sparc_recode |= TME_SPARC_RECODE_INSN_NONE;
	}
      }

      /* otherwise, if this is a v9 movr instruction: */
      else if (__tme_predict_false(TME_SPARC_VERSION(ic) >= 9
				   && sparc_opcode == 0x2f)) {

	/* get the condition field: */
	cond = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x7 << 10));

	/* this shifts the "not" bit in the condition up from bit
	   two to bit three, to match the other branches: */
	cond = (cond + 4) & (TME_SPARC_COND_NOT | 3);

	/* if this is an unimplemented movr: */
	if (__tme_predict_false(!TME_SPARC_COND_IS_CONDITIONAL(cond))) {

	  /* this instruction needs a full assist: */
	  sparc_recode |= TME_SPARC_RECODE_INSN_ASSIST_FULL;
	}

	/* otherwise, this is an implemented movr: */
	else {

	  /* test the register and set the internal "rcc" flags: */
	  recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1));
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_AND;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	  recode_insn->tme_recode_insn_operand_src[1] = recode_operand;
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_OPERAND_NULL;
	  recode_insn->tme_recode_insn_flags_thunk = ic->tme_sparc_recode_flags_thunk_rcc;
	  recode_insn++;

	  /* define the first recode flag with the condition: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_DEFC;
	  recode_insn->tme_recode_insn_operand_src[0] = (cond % TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_src[1] = 0 - (int) (cond / TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_FLAG(0);
	  recode_insn->tme_recode_insn_conds_thunk = ic->tme_sparc_recode_conds_thunk_rcc;
	  recode_insn++;

	  /* emit the if to test the first recode carry flag: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_IF;
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_FLAG(0);
	  recode_insn++;

	  /* move the source operand to the destination register: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	  if (sparc_insn & TME_BIT(13)) {
	    recode_insn->tme_recode_insn_imm_uguest
	      = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x3ff);
	    recode_operand = TME_RECODE_OPERAND_IMM;
	  }
	  else {
	    recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2));
	  }
	  recode_insn->tme_recode_insn_operand_src[1] = recode_operand;
	  recode_insn->tme_recode_insn_operand_dst
	    = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD));
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;

	  /* emit the endif: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ENDIF;
	  recode_insn++;

	  /* suppress any instructions below: */
	  sparc_recode |= TME_SPARC_RECODE_INSN_NONE;
	}
      }

      /* otherwise, if this is a v9 ld*a instruction: */
      /* NB: all of the simple ld and st instructions that we handle
	 here, have bit 21 (bit 2 in the opcode) set for a st, and
	 clear for a ld: */
      else if (__tme_predict_false(TME_SPARC_VERSION(ic) >= 9
				   && TME_SPARC_RECODE_INSN_OPCODE(sparc_recode) == TME_RECODE_OPCODE_RW
				   && ((sparc_opcode
					& (TME_BIT(4)
					   + TME_BIT(2)))
				       == (TME_BIT(4)
					   + !TME_BIT(2))))) {

	/* if the i bit is clear, and the immediate ASI is not
	   ASI_PRIMARY_NOFAULT or ASI_PRIMARY_NOFAULT_LITTLE: */
	if ((sparc_insn & TME_BIT(13)) == 0
	    && (TME_FIELD_MASK_EXTRACTU(sparc_insn, (0xff << 5))
		!= (TME_SPARC64_ASI_FLAG_UNRESTRICTED
		    + ((ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_CLE)
		       ? TME_SPARC64_ASI_FLAG_LITTLE
		       : !TME_SPARC64_ASI_FLAG_LITTLE)
		    + TME_SPARC64_ASI_FLAG_NO_FAULT))) {

	  /* this instruction can't be a simple ld instruction: */
	  sparc_recode
	    ^= (TME_SPARC_RECODE_INSN_LDA
		^ TME_SPARC_RECODE_INSN_ASSIST_CANTRAP);
	}
      }

      /* if this instruction needs a full assist: */
      if ((sparc_recode
	   & (TME_SPARC_RECODE_INSN_OPCODE_MASK
	      | TME_SPARC_RECODE_INSN_NO_RS2
	      | TME_SPARC_RECODE_INSN_NO_RS1))
	  == (TME_RECODE_OPCODE_GUEST
	      | TME_SPARC_RECODE_INSN_NO_RS2
	      | TME_SPARC_RECODE_INSN_NO_RS1)) {

	/* set the full assist function: */
	sparc_assist = _tme_sparc_recode_insn_assist_full;
      }
    }

    /* otherwise, if this is a format two instruction: */
    else if (__tme_predict_true(sparc_insn < 0x40000000)) {

      /* dispatch on op2: */
      switch (TME_FIELD_MASK_EXTRACTU(sparc_insn, (0x7 << 22))) {

      default:
      case 0: /* UNIMP: */
	sparc_recode |= TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_LAST;
	sparc_assist = _tme_sparc_recode_insn_assist_unimpl;
	break;

#if TME_SPARC_VERSION(ic) >= 9
      case 1: /* BPcc */
      case 3: /* BPr */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
      case 2: /* Bicc: */

	/* if the next instruction can't be recoded when it's in a
	   branch delay slot (usually because it's also a control
	   transfer instruction): */
	if (__tme_predict_false(_tme_sparc_recode_insn_branch_delay_bad(sparc_insn_next))) {

	  /* if we haven't recoded any instructions yet, recode is
	     impossible here: */
	  if (recode_insn == &ic->tme_sparc_recode_insns[0]) {
	    return (0);
	  }
	  
	  /* stop recoding immediately, leaving the updated PC
	     pointing to the instruction before the jmpl: */
	  assert (pc_advance >= sizeof(sparc_insn));
	  pc_advance -= sizeof(sparc_insn);
	  sparc_insn_count--;
	  sparc_recode
	    |= (TME_SPARC_RECODE_INSN_UPDATE_PCS
		| TME_SPARC_RECODE_INSN_NONE
		| TME_SPARC_RECODE_INSN_LAST);
	  break;
	}

	/* get the condition field: */
	cond = TME_FIELD_MASK_EXTRACTU(sparc_insn, (0xf << 25));

	/* assume that this branch instruction tests a condition in
	   the icc flags: */
	conds_thunk = ic->tme_sparc_recode_conds_thunk_icc;

	/* if this is a BPr or a BPcc: */
	if (TME_SPARC_VERSION(ic) >= 9
	    && (sparc_insn & (0x1 << 22))) {

	  /* if this is a BPr: */
	  if (sparc_insn & (0x2 << 22)) {

	    /* if this is an unimplemented BPr instruction: */
	    if (__tme_predict_false((sparc_insn & (1 << 28)) != 0
				    || (sparc_insn & (3 << 25)) == 0)) {

	      /* do a full assist and stop recoding: */
	      sparc_recode |= TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_LAST;
	      sparc_assist = _tme_sparc_recode_insn_assist_unimpl;
	      break;
	    }

	    /* test the register and set the internal "rcc" flags: */
	    recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1));
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_AND;
	    recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	    recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	    recode_insn->tme_recode_insn_operand_src[1] = recode_operand;
	    recode_insn->tme_recode_insn_operand_dst = TME_RECODE_OPERAND_NULL;
	    recode_insn->tme_recode_insn_flags_thunk = ic->tme_sparc_recode_flags_thunk_rcc;
	    recode_insn++;

	    /* this shifts the "not" bit in the condition up from bit
	       two to bit three, to match the other branches: */
	    cond = (cond + 4) & (TME_SPARC_COND_NOT | 3);

	    /* we need to test the condition in the internal "rcc" flags: */
	    conds_thunk = ic->tme_sparc_recode_conds_thunk_rcc;

	    /* get the raw branch displacement: */
	    branch_displacement_raw
	      = ((tme_int16_t)
		 (((sparc_insn & (0x3 << 20)) >> 6)
		  | (sparc_insn & 0x3fff)));
	  }

	  /* otherwise, this is a BPcc: */
	  else {

	    /* if this is an unimplemented BPcc instruction: */
	    if (__tme_predict_false((sparc_insn & (1 << 20)) != 0)) {

	      /* do a full assist and stop recoding: */
	      sparc_recode |= TME_SPARC_RECODE_INSN_ASSIST_FULL | TME_SPARC_RECODE_INSN_LAST;
	      sparc_assist = _tme_sparc_recode_insn_assist_unimpl;
	      break;
	    }

	    /* if this BPcc is testing the xcc flags: */
	    if (sparc_insn & (1 << 21)) {
	      conds_thunk = ic->tme_sparc_recode_conds_thunk_xcc;
	    }

	    /* get the raw branch displacement: */
	    branch_displacement_raw
	      = ((tme_int32_t)
		 TME_FIELD_MASK_EXTRACTS(sparc_insn, 0x0007ffff));
	  }
	}

	/* otherwise, this is a Bicc: */
	else {

	  /* get the raw branch displacement: */
	  branch_displacement_raw
	    = ((tme_int32_t)
	       TME_FIELD_MASK_EXTRACTS(sparc_insn, 0x003fffff));
	}

	/* fully sign-extend the branch displacement: */
	branch_displacement
	  = ((tme_sparc_ireg_t)
	     (tme_recode_guest_t)
	     branch_displacement_raw);

 	/* advance PC to the branch delay slot: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = reg_guest_pc;
	recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	recode_insn->tme_recode_insn_imm_uguest = pc_advance + sizeof(sparc_insn);
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;
	pc_advance = 0 - (tme_uint32_t) sizeof(sparc_insn);
	reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);

	/* if this isn't a branch always: */
	if (cond != (TME_SPARC_COND_NOT | TME_SPARC_COND_N)) {

	  /* assuming that the branch won't be taken, set PC_next to
	     the instruction following the branch delay slot: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = recode_size_address;
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	  recode_insn->tme_recode_insn_imm_uguest = sizeof(sparc_insn);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;
	}

	/* if this is a conditional branch: */
	if (TME_SPARC_COND_IS_CONDITIONAL(cond)) {

	  /* define the recode jump flag with the condition: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_DEFC;
	  recode_insn->tme_recode_insn_operand_src[0] = (cond % TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_src[1] = 0 - (int) (cond / TME_SPARC_COND_NOT);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_FLAG_JUMP;
	  recode_insn->tme_recode_insn_conds_thunk = conds_thunk;
	  recode_insn++;

	  /* emit the if to test the recode jump flag: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_IF;
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_FLAG_JUMP;
	  recode_insn++;
	}

	/* if this isn't a branch never: */
	if (cond != TME_SPARC_COND_N) {

	  /* if this branch target is within this same page, this chain
	     can be near, otherwise it must be far: */
	  if ((((pc_next - sizeof(sparc_insn))
		+ (branch_displacement * sizeof(sparc_insn)))
	       ^ (pc_next - sizeof(sparc_insn)))
	      < page_size) {
	    sparc_recode |= TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_NEAR);
	  }
	  else {
	    sparc_recode |= TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_FAR);
	  }

	  /* if this is a branch always: */
	  if (cond == (TME_SPARC_COND_NOT | TME_SPARC_COND_N)) {

	    /* the branch delay slot will be the last instruction, and
	       after that we need an unconditional jump: */
	    sparc_recode
	      |= (TME_SPARC_RECODE_INSN_LAST
		  | TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_JUMP
						+ TME_RECODE_CHAIN_INFO_UNCONDITIONAL));

	    /* set PC_next to the branch target, using the address of
	       the branch delay slot already in PC (which is one
	       instruction away from the PC of the branch, to which
	       the branch displacement is relative): */
	    recode_operand = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	    branch_displacement -= 1;
	  }

	  /* otherwise, this is a conditional branch: */
	  else {

	    /* the branch delay slot will be the last instruction, and
	       after that we need an conditional jump: */
	    sparc_recode
	      |= (TME_SPARC_RECODE_INSN_LAST
		  | TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_JUMP
						+ TME_RECODE_CHAIN_INFO_CONDITIONAL));

	    /* change PC_next to the branch target, from the
	       instruction following the branch delay slot already in
	       PC_next (which is two instructions away from the PC of
	       the branch, to which the branch displacement is
	       relative): */
	    recode_operand = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	    branch_displacement -= 2;
	  }

	  /* set PC_next to the branch target: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = recode_size_address;
	  recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	  recode_insn->tme_recode_insn_imm_uguest = branch_displacement * sizeof(sparc_insn);
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;
	}

	/* if this is a conditional branch: */
	if (TME_SPARC_COND_IS_CONDITIONAL(cond)) {

	  /* if this conditional branch never annuls the branch delay
	     slot: */
	  if ((sparc_insn & TME_BIT(29)) == 0) {

	    /* emit the endif: */
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ENDIF;
	    recode_insn++;
	  }

	  /* otherwise, this conditional branch annuls the branch
	     delay slot if the branch is not taken: */
	  else {

	    /* we need to emit the endif after we emit the
	       instructions for the branch delay slot: */
	    sparc_recode |= TME_SPARC_RECODE_INSN_NEED_ENDIF;
	  }
	}

	/* otherwise, this is an unconditional branch: */
	else {

	  /* if the branch delay slot is annulled: */
	  if (sparc_insn & TME_BIT(29)) {

	    /* don't bother to recode the instruction in the branch
	       delay slot: */
	    sparc_recode |= TME_SPARC_RECODE_INSN_NONE;
	  }
	}

	/* if this is a branch to . instruction: */
	if (__tme_predict_false(branch_displacement_raw == 0)) {

	  /* if this isn't a branch never, and this isn't an
	     unconditional branch that annuls, and the branch with its
	     delay instruction are a supported timing loop: */
	  if (cond != TME_SPARC_COND_N
	      && (sparc_recode & TME_SPARC_RECODE_INSN_NONE) == 0
	      && tme_sparc_timing_loop_ok(sparc_insn,
					  sparc_insn_next)) {

	    /* store the branch delay instruction in the instruction register: */
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	    recode_insn->tme_recode_insn_size = TME_RECODE_SIZE_32;
	    recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	    recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	    recode_insn->tme_recode_insn_imm_u32 = sparc_insn_next;
	    recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_INSN);
	    recode_insn->tme_recode_insn_flags_thunk = NULL;
	    recode_insn++;

	    /* call the timing loop assist function: */
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_GUEST;
	    recode_insn->tme_recode_insn_size = TME_RECODE_SIZE_32;
	    recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_IMM;
	    recode_insn->tme_recode_insn_imm_u32 = sparc_insn;
	    recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_ZERO;
	    recode_insn->tme_recode_insn_operand_dst = TME_RECODE_OPERAND_NULL;
	    recode_insn->tme_recode_insn_guest_func = tme_sparc_timing_loop_assist;
	    recode_insn++;

	    /* the branch delay instruction must be the last instruction: */
	    assert (sparc_recode & TME_SPARC_RECODE_INSN_LAST);

	    /* advance to the branch delay instruction: */
	    assert (((tme_uint32_t) (pc_advance + sizeof(sparc_insn))) == 0
		    && reg_guest_pc == TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC));
	    pc_advance = 0;

	    /* suppress the branch delay instruction: */
	    sparc_recode |= TME_SPARC_RECODE_INSN_NONE;

	    /* do *not* loop now to fetch and recode the instruction
	       in the branch delay slot: */
	    break;
	  }
	}

	/* loop now to fetch and recode the instruction in the branch
	   delay slot: */
	continue;

      case 4: /* SETHI: */

	/* unless this instruction is suppressed, load the constant
	   into the register: */
	if (!(sparc_recode & TME_SPARC_RECODE_INSN_NONE)) {
	  recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD));
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	  recode_insn->tme_recode_insn_imm_u32 = (tme_uint32_t) (sparc_insn << 10);
	  recode_insn->tme_recode_insn_operand_dst = recode_operand;
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;
	}

	/* suppress any instructions below: */
	sparc_recode |= TME_SPARC_RECODE_INSN_NONE;
	break;

#if TME_SPARC_VERSION(ic) >= 9
      case 5: /* FBPfcc */
#endif /* TME_SPARC_VERSION(ic) >= 9 */
      case 6: /* FBfcc: */

	/* if we haven't recoded any instructions yet, recode is
	   impossible here: */
	if (recode_insn == &ic->tme_sparc_recode_insns[0]) {
	  return (0);
	}

	/* stop recoding immediately, leaving the updated PC
	   pointing to the instruction before the jmpl: */
	assert (pc_advance >= sizeof(sparc_insn));
	pc_advance -= sizeof(sparc_insn);
	sparc_insn_count--;
	sparc_recode
	  |= (TME_SPARC_RECODE_INSN_UPDATE_PCS
	      | TME_SPARC_RECODE_INSN_NONE
	      | TME_SPARC_RECODE_INSN_LAST);
	break;
      }
    }

    /* otherwise, this is a format one instruction: */
    else {

      /* if the next instruction can't be recoded when it's in a
	 branch delay slot (usually because it's also a control
	 transfer instruction): */
      if (__tme_predict_false(_tme_sparc_recode_insn_branch_delay_bad(sparc_insn_next))) {

	/* if we haven't recoded any instructions yet, recode is
	   impossible here: */
	if (recode_insn == &ic->tme_sparc_recode_insns[0]) {
	  return (0);
	}
	  
	/* stop recoding immediately, leaving the updated PC
	   pointing to the instruction before the jmpl: */
	assert (pc_advance >= sizeof(sparc_insn));
	pc_advance -= sizeof(sparc_insn);
	sparc_insn_count--;
	sparc_recode
	  |= (TME_SPARC_RECODE_INSN_UPDATE_PCS
	      | TME_SPARC_RECODE_INSN_NONE
	      | TME_SPARC_RECODE_INSN_LAST);
      }

      /* otherwise, the next instruction can be recoded when it's in a
	 branch delay slot: */
      else {

	/* write the PC of the CALL into r[15]: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = reg_guest_pc;
	recode_insn->tme_recode_insn_operand_src[1]
	  = (pc_advance == 0
	     ? TME_RECODE_OPERAND_ZERO
	     : TME_RECODE_OPERAND_IMM);
	recode_insn->tme_recode_insn_imm_uguest = pc_advance;
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(15);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;

	/* if the PC is not guest size: */
	if (recode_size_address < TME_SPARC_RECODE_SIZE(ic)) {

	  /* zero-extend r[15] to guest size: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_EXTZ;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(15);
	  recode_insn->tme_recode_insn_operand_src[1] = recode_size_address;
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(15);
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	  recode_insn++;
	}

	/* advance PC to the branch delay slot: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(15);
	recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	recode_insn->tme_recode_insn_imm_uguest = sizeof(sparc_insn);
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;
	pc_advance = 0 - (tme_uint32_t) sizeof(sparc_insn);
	reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);

	/* write the target of the call into PC_next.  NB that we have
	   to account for the fact that PC already points to the
	   branch delay slot, but the displacement is relative to the
	   PC of the call instruction: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	recode_insn->tme_recode_insn_imm_uguest
	  = ((tme_sparc_ireg_t)
	     (((tme_recode_guest_t)
	       (tme_int32_t)
	       (sparc_insn << 2))
	      - sizeof(sparc_insn)));
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;

	/* the branch delay slot will be the last instruction, and
	   after that we need an unconditional call: */
	sparc_recode
	  |= (TME_SPARC_RECODE_INSN_LAST
	      | TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_CALL
					    + TME_RECODE_CHAIN_INFO_UNCONDITIONAL));

	/* if the call target is within this same page,
	   the call can be near, otherwise the call must be far: */
	if ((((pc_next - sizeof(sparc_insn))
	      + ((tme_sparc_ireg_t)
		 (tme_recode_guest_t)
		 (tme_int32_t)
		 (sparc_insn << 2)))
	     ^ (pc_next - sizeof(sparc_insn)))
	    < page_size) {
	  sparc_recode |= TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_NEAR);
	}
	else {
	  sparc_recode |= TME_SPARC_RECODE_CHAIN_INFO(TME_RECODE_CHAIN_INFO_FAR);
	}

	continue;
      }
    }

    /* if we might not have enough space for recode insns for the next
       instruction: */
    if (__tme_predict_false(recode_insn >= recode_insns_last_start)) {

      /* this will be the last instruction, and we need to update the
	 PCs: */
      sparc_recode |= TME_SPARC_RECODE_INSN_UPDATE_PCS | TME_SPARC_RECODE_INSN_LAST;
    }

    /* if this is the last instruction, we must be updating PC and
       PC_next: */
    assert ((sparc_recode & TME_SPARC_RECODE_INSN_LAST) == 0
	    || (sparc_recode & TME_SPARC_RECODE_INSN_UPDATE_PCS)
	    || (pc_advance == 0
		&& reg_guest_pc == TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC)));

    /* if we need PC and PC_next to be valid for this instruction: */
    if (sparc_recode & TME_SPARC_RECODE_INSN_UPDATE_PCS) {

      /* if we need to update PC and PC_next: */
      if (pc_advance > 0
	  || reg_guest_pc != TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC)) {

	/* advance PC: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = reg_guest_pc;
	recode_insn->tme_recode_insn_operand_src[1]
	  = (pc_advance == 0
	     ? TME_RECODE_OPERAND_ZERO
	     : TME_RECODE_OPERAND_IMM);
	recode_insn->tme_recode_insn_imm_uguest = pc_advance;
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;
	pc_advance = 0;
	reg_guest_pc = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);

	/* set PC_next: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = recode_size_address;
	recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC);
	recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	recode_insn->tme_recode_insn_imm_uguest = sizeof(sparc_insn);
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_PC_NEXT);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;
      }
    }

    /* if this instruction hasn't been suppressed: */
    if (__tme_predict_true((sparc_recode & TME_SPARC_RECODE_INSN_NONE) == 0)) {

      /* if this instruction always or sometimes needs an assist: */
      if (sparc_recode & TME_SPARC_RECODE_INSN_UPDATE_INSN) {

	/* store the instruction in the instruction register: */
	recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	recode_insn->tme_recode_insn_size = TME_RECODE_SIZE_32;
	recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
	recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	recode_insn->tme_recode_insn_imm_u32 = sparc_insn;
	recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_INSN);
	recode_insn->tme_recode_insn_flags_thunk = NULL;
	recode_insn++;

	/* if this instruction doesn't take rs2: */
	if (sparc_recode & TME_SPARC_RECODE_INSN_NO_RS2) {

	  /* clear the i bit and rs2, to force the second source
	     operand to be %g0, or zero: */
	  sparc_insn &= ~(TME_BIT(13) | TME_SPARC_FORMAT3_MASK_RS2);
	}

	/* if this instruction doesn't take rs1: */
	if (sparc_recode & TME_SPARC_RECODE_INSN_NO_RS1) {

	  /* clear rs1, to force the first source operand to be %g0,
	     or zero: */
	  sparc_insn &= ~TME_SPARC_FORMAT3_MASK_RS1;
	}
      }

      /* otherwise, this instruction doesn't need an assist: */
      else {

	/* if this is an addx, addxcc, subx, or subxcc instruction: */
	if (sparc_recode & TME_SPARC_RECODE_INSN_DEFC) {

	  /* define the recode carry flag with the icc C flag: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_DEFC;
	  recode_insn->tme_recode_insn_operand_src[0] = TME_SPARC_COND_CS;
	  recode_insn->tme_recode_insn_operand_src[1] = 0;
	  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_FLAG_CARRY;
	  recode_insn->tme_recode_insn_conds_thunk = ic->tme_sparc_recode_conds_thunk_icc;
	  recode_insn++;
	}

	/* because we set the flags thunk on the recode instruction
	   now, this instruction can't be a shift (because a shift may
	   make more than one recode instruction): */
	assert ((sparc_recode
		 & (TME_SPARC_RECODE_INSN_CC
		    | TME_SPARC_RECODE_INSN_SHIFT))
		!= (TME_SPARC_RECODE_INSN_CC
		    | TME_SPARC_RECODE_INSN_SHIFT));

	/* assume that this instruction doesn't update flags: */
	flags_thunk = NULL;

	/* if this instruction updates flags: */
	if (sparc_recode & TME_SPARC_RECODE_INSN_CC) {

	  /* set the right flags thunk for this instruction.  NB that
	     the least significant two bits of op3 are zero only for
	     the additive instructions, and bit two of op3 is set only
	     for a subtraction instruction: */
	  flags_thunk
	    = ((sparc_insn & (0x3 << 19)) == 0
	       ? ((sparc_insn & (0x4 << 19))
		  ? ic->tme_sparc_recode_flags_thunk_sub
		  : ic->tme_sparc_recode_flags_thunk_add)
	       : ic->tme_sparc_recode_flags_thunk_logical);
	}

	/* set any flags thunk on this instruction: */
	recode_insn->tme_recode_insn_flags_thunk = flags_thunk;
      }

      /* assume that if this is a shift, it's a 32-bit shift: */
      shift_count_mask = 32 - 1;

      /* if this is a v9 shift: */
      if (TME_SPARC_VERSION(ic) >= 9
	  && (sparc_recode & TME_SPARC_RECODE_INSN_SHIFT)) {

	/* if this is a 64-bit shift: */
	if (sparc_insn & TME_BIT(12)) {

	  /* this is a 64-bit shift: */
	  shift_count_mask = 64 - 1;
	}

	/* otherwise, this is a 32-bit shift: */
	else {

	  /* if this is a right shift: */
	  if (sparc_insn & TME_BIT(20)) {

	    /* remember that this is a right shift: */
	    sparc_recode |= TME_SPARC_RECODE_INSN_SHIFT_RIGHT;

	    /* decode rs1: */
	    recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1));

	    /* extend rs1 into the second temporary register.  if this
	       is a shift right arithmetic, sign extend, else zero
	       extend: */
	    recode_insn->tme_recode_insn_opcode
	      = ((sparc_insn & TME_BIT(19))
		 ? TME_RECODE_OPCODE_EXTS
		 : TME_RECODE_OPCODE_EXTZ);
	    recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	    recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	    recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_SIZE_32;
	    recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(1));
	    recode_insn++;

	    /* NB that the flags thunk for the final instruction was
	       already set above (to NULL, because sparc shift
	       instructions do not affect any condition codes), so we
	       have to do the same since we advanced to the next
	       instruction: */
	    recode_insn->tme_recode_insn_flags_thunk = NULL;
	  }
	}
      }

      /* if the i bit is one: */
      if (sparc_insn & TME_BIT(13)) {

	/* decode simm13: */
	imm = TME_FIELD_MASK_EXTRACTS(sparc_insn, (tme_sparc_ireg_t) 0x1fff);

	/* if this is a shift instruction: */
	if (__tme_predict_false(sparc_recode & TME_SPARC_RECODE_INSN_SHIFT)) {

	  /* mask the immediate with the shift count mask: */
	  imm &= shift_count_mask;
	}

	/* if the immediate is zero, the second source operand is zero,
	   otherwise the second source operand is the immediate: */
	recode_insn->tme_recode_insn_imm_uguest = imm;
	recode_operand
	  = (imm == 0
	     ? TME_RECODE_OPERAND_ZERO
	     : TME_RECODE_OPERAND_IMM);
      }

      /* otherwise, the i bit is zero: */
      else {

	/* decode rs2: */
	recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS2));
	if (recode_operand == TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0)) {
	  recode_operand = TME_RECODE_OPERAND_ZERO;
	}

	/* if this is a shift instruction: */
	if (__tme_predict_false(sparc_recode & TME_SPARC_RECODE_INSN_SHIFT)) {

	  /* make a temporary copy of the register with shift count,
	     masked with the shift count mask, and use the temporary
	     register as the second source operand: */
	  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_AND;
	  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
	  recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
	  recode_insn->tme_recode_insn_imm_uguest = shift_count_mask;
	  recode_operand = TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(0));
	  recode_insn->tme_recode_insn_operand_dst = recode_operand;
	  recode_insn++;

	  /* NB that the flags thunk for the final instruction was
	     already set above (to NULL, because sparc shift
	     instructions do not affect any condition codes), so we
	     have to do the same since we advanced to the next
	     instruction: */
	  recode_insn->tme_recode_insn_flags_thunk = NULL;
	}
      }

      /* set the second source operand: */
      recode_insn->tme_recode_insn_operand_src[1] = recode_operand;

      /* decode rs1: */
      recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RS1));
      if (recode_operand == TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0)) {
	recode_operand = TME_RECODE_OPERAND_ZERO;
      }

      /* if this is a v9 32-bit right shift: */
      if (__tme_predict_false(TME_SPARC_VERSION(ic) >= 9
			      && (sparc_recode & TME_SPARC_RECODE_INSN_SHIFT_RIGHT))) {

	/* the first source operand is actually in the second
	   temporary register: */
	recode_operand = TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(1));
      }

      /* set the first source operand: */
      recode_insn->tme_recode_insn_operand_src[0] = recode_operand;
	
      /* decode rd: */
      recode_operand = TME_RECODE_REG_GUEST(TME_FIELD_MASK_EXTRACTU(sparc_insn, TME_SPARC_FORMAT3_MASK_RD));
      if (recode_operand == TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0)) {
	recode_operand = TME_RECODE_OPERAND_NULL;
      }

      /* if this is a simple ld or st instruction: */
      if (TME_SPARC_RECODE_INSN_OPCODE(sparc_recode) == TME_RECODE_OPCODE_RW) {

	/* assume that the first source operand is %g0, which makes
	   the second source operand the guest address: */
	recode_operand_address = recode_insn->tme_recode_insn_operand_src[1];

	/* if the second source operand is %g0: */
	if (recode_operand_address == TME_RECODE_OPERAND_ZERO) {

	  /* nothing to do; the guest address is the first source
	     operand, which is already in
	     recode_insn->tme_recode_insn_operand_src[0]: */
	}

	/* otherwise, the second source operand is not %g0: */
	else {

	  /* if the first source operand is not %g0: */
	  if (recode_insn->tme_recode_insn_operand_src[0] != TME_RECODE_OPERAND_ZERO) {

	    /* add the two parts of the guest address into the
	       temporary register: */
	    recode_operand_address = TME_RECODE_REG_GUEST(TME_SPARC_IREG_TMP(0));
	    recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ADD;
	    recode_insn->tme_recode_insn_size = recode_size_address;
	    recode_insn->tme_recode_insn_operand_dst = recode_operand_address;
	    recode_insn->tme_recode_insn_flags_thunk = NULL;
	    recode_insn++;
	  }

	  /* set the guest address as the first source operand: */
	  recode_insn->tme_recode_insn_operand_src[0] = recode_operand_address;
	}

	/* if this is a st instruction: */
	/* NB: all of the simple ld and st instructions that we handle
	   here, have bit 21 set for a st, and clear for a ld: */
	if (sparc_insn & (4 << 19)) {

	  /* the second source operand is the register being
	     stored: */
	  recode_insn->tme_recode_insn_operand_src[1]
	    = ((TME_RECODE_OPERAND_NULL != TME_RECODE_OPERAND_ZERO
		&& recode_operand == TME_RECODE_OPERAND_NULL)
	       ? TME_RECODE_OPERAND_ZERO
	       : recode_operand);
	  
	  /* the destination operand must be the recode %null: */
	  recode_operand = TME_RECODE_OPERAND_NULL;
	}

	/* otherwise, this is an ld instruction: */
	else {

	  /* the second source operand must be zero: */
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_ZERO;
	}

	/* set the read/write thunk: */
	assert (TME_SPARC_VERSION(ic) >= 9 || (sparc_insn & (16 << 19)) == 0);
	recode_insn->tme_recode_insn_rw_thunk
	  = ic->tme_sparc_recode_rw_thunks
	      [TME_SPARC_RECODE_RW_THUNK_INDEX(ic,
					       (TME_SPARC_RECODE_SIZE(ic)
						- (TME_SPARC_VERSION(ic) >= 9
						   && (ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_AM))),
					       ((TME_SPARC_VERSION(ic) >= 9
						 && (ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_CLE))
						? TME_ENDIAN_LITTLE
						: TME_ENDIAN_BIG),
					       (TME_SPARC_VERSION(ic) >= 9
						&& (sparc_insn & (16 << 19))),
					       TME_FIELD_MASK_EXTRACTU(sparc_insn, (0xf << 19)))];
      }

      /* otherwise, if this instruction needs an assist: */
      else if (TME_SPARC_RECODE_INSN_OPCODE(sparc_recode) == TME_RECODE_OPCODE_GUEST) {

	/* set the assist function: */
	recode_insn->tme_recode_insn_guest_func = sparc_assist;

	/* if this instruction needs a full assist: */
	if ((sparc_recode
	     & (TME_SPARC_RECODE_INSN_NO_RS2
		| TME_SPARC_RECODE_INSN_NO_RS1))
	    == (TME_SPARC_RECODE_INSN_NO_RS2
		| TME_SPARC_RECODE_INSN_NO_RS1)) {

	  /* the source operands can be the recode %undef, since the
	     full assist functions don't use the source operands: */
	  assert (sparc_assist == _tme_sparc_recode_insn_assist_full
		  || sparc_assist == _tme_sparc_recode_insn_assist_unimpl
		  );
	  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_UNDEF;
	  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_UNDEF;

	  /* assume that this instruction is not also marked
	     TME_SPARC_RECODE_INSN_NO_RD, and force the destination
	     operand to be the recode %null, to indicate that the
	     assist can change any of the recode-visible ic state: */
	  recode_operand = TME_RECODE_OPERAND_NULL;
	}

	/* if this instruction's rd field doesn't encode a destination
	   general register: */
	if (sparc_recode & TME_SPARC_RECODE_INSN_NO_RD) {

	  /* force the destination operand to be the fixed %g0: */
	  recode_operand = TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0);
	}
      }

      /* set the destination operand: */
      recode_insn->tme_recode_insn_operand_dst = recode_operand;

      /* set the size: */
      recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);

      /* set the opcode: */
      recode_insn->tme_recode_insn_opcode = TME_SPARC_RECODE_INSN_OPCODE(sparc_recode);

      /* we have finished this instruction: */
      recode_insn++;
    }

    /* if we need to, emit an endif: */
    if (sparc_recode & TME_SPARC_RECODE_INSN_NEED_ENDIF) {
      recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_ENDIF;
      recode_insn++;
#ifndef NDEBUG
      sparc_recode &= ~TME_SPARC_RECODE_INSN_NEED_ENDIF;
#endif
    }

    /* if this was the last instruction: */
    if (sparc_recode & TME_SPARC_RECODE_INSN_LAST) {
      break;
    }

    /* clear sparc_recode: */
    sparc_recode = 0;
  }

  /* make sure we didn't forget an endif: */
  assert ((sparc_recode & TME_SPARC_RECODE_INSN_NEED_ENDIF) == 0);

#ifdef _TME_SPARC_STATS
  /* update the recode instructions total: */
  recode_insn->tme_recode_insn_opcode = TME_RECODE_OPCODE_GUEST;
  recode_insn->tme_recode_insn_size = TME_SPARC_RECODE_SIZE(ic);
  recode_insn->tme_recode_insn_operand_src[0] = TME_RECODE_OPERAND_ZERO;
  recode_insn->tme_recode_insn_operand_src[1] = TME_RECODE_OPERAND_IMM;
  recode_insn->tme_recode_insn_imm_u32 = sparc_insn_count;
  recode_insn->tme_recode_insn_operand_dst = TME_RECODE_REG_GUEST(TME_SPARC_IREG_G0);
  recode_insn->tme_recode_insn_guest_func = _tme_sparc_recode_insns_total;
  recode_insn++;
#endif /* _TME_SPARC_STATS */

  /* set the end of the instructions: */
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_insns_end = recode_insn;

  /* get the chain information: */
  chain_info
    = ((sparc_recode
	/ TME_SPARC_RECODE_CHAIN_INFO(1))
       & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	  | TME_RECODE_CHAIN_INFO_CONDITIONAL
	  | TME_RECODE_CHAIN_INFO_NEAR
	  | TME_RECODE_CHAIN_INFO_FAR
	  | TME_RECODE_CHAIN_INFO_JUMP
	  | TME_RECODE_CHAIN_INFO_RETURN
	  | TME_RECODE_CHAIN_INFO_CALL
	  ));

  /* if there is no chain information: */
  if (chain_info == 0) {

    /* if the next PC is on the same page, make a chain jump near
       unconditional, otherwise make a chain jump far
       unconditional: */
    if (pc_next <= itlb_addr_last) {
      chain_info += TME_RECODE_CHAIN_INFO_NEAR;
    }
    else {
      chain_info += TME_RECODE_CHAIN_INFO_FAR;
    }
    chain_info
      += (TME_RECODE_CHAIN_INFO_JUMP
	  + TME_RECODE_CHAIN_INFO_UNCONDITIONAL);
  }

  /* otherwise, there is chain information.  if the chain is
     conditional: */
  else if ((chain_info
	    & (TME_RECODE_CHAIN_INFO_UNCONDITIONAL
	       | TME_RECODE_CHAIN_INFO_CONDITIONAL))
	   != TME_RECODE_CHAIN_INFO_UNCONDITIONAL) {

    /* if the alternate PC is on the same page, the alternate target
       can be near, otherwise it must be far: */
    if (pc_next <= itlb_addr_last) {
      chain_info += TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR;
    }
    else {
      chain_info += TME_RECODE_CHAIN_INFO_ALTERNATE_FAR;
    }
  }

  /* set the chain information: */
  ic->tme_sparc_recode_insns_group.tme_recode_insns_group_chain_info = chain_info;

  /* make sure we didn't overflow the instructions buffer: */
  assert (recode_insn <= &ic->tme_sparc_recode_insns[TME_ARRAY_ELS(ic->tme_sparc_recode_insns)]);

  /* recode these instructions: */
  return (tme_recode_insns_thunk(ic->tme_sparc_recode_ic,
				 &ic->tme_sparc_recode_insns_group));
}
