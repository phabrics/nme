/* $Id: sparc-insns.c,v 1.7 2010/02/14 14:39:35 fredette Exp $ */

/* ic/sparc/sparc-insns.c - SPARC instruction functions: */

/*
 * Copyright (c) 2005 Matt Fredette
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
#include "sparc-impl.h"
#include <tme/misc.h>

_TME_RCSID("$Id: sparc-insns.c,v 1.7 2010/02/14 14:39:35 fredette Exp $");

#undef TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) (8)

TME_SPARC_FORMAT3(tme_sparc32_illegal, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}

/* the coprocessor instructions are all illegal for now: */
TME_SPARC_FORMAT3(tme_sparc32_cpop1, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_cpop2, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_ldc, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_ldcsr, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_lddc, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_stc, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_stcsr, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_stdc, tme_uint32_t)
{
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}
TME_SPARC_FORMAT3(tme_sparc32_stdcq, tme_uint32_t)
{
  TME_SPARC_INSN_PRIV;
  TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
}

TME_SPARC_FORMAT3(tme_sparc32_rdasr, tme_uint32_t)
{
  unsigned int reg_rs1;
  unsigned int reg_rd;
  tme_uint32_t value;

  reg_rs1 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0x1f << 14));
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);

  /* rdy: */
  if (reg_rs1 == 0) {
    value = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y);
  }

  /* stbar: */
  else if (reg_rs1 == 15 && reg_rd == 0) {
    TME_SPARC_INSN_OK;
  }

  else {

    /* all other rdasr instructions are privileged: */
    TME_SPARC_INSN_PRIV;

    value = 0;
    abort();
  }

  TME_SPARC_FORMAT3_RD = value;
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_rdpsr, tme_uint32_t)
{
  TME_SPARC_INSN_PRIV;
  TME_SPARC_FORMAT3_RD = ic->tme_sparc32_ireg_psr;
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_rdwim, tme_uint32_t)
{
  TME_SPARC_INSN_PRIV;
  TME_SPARC_FORMAT3_RD = ic->tme_sparc32_ireg_wim;
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_rdtbr, tme_uint32_t)
{
  TME_SPARC_INSN_PRIV;
  TME_SPARC_FORMAT3_RD = ic->tme_sparc32_ireg_tbr;
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_wrasr, tme_uint32_t)
{
  unsigned int reg_rd;
  tme_uint32_t value_xor;
  
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);
  value_xor = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* "WRY ... writes r[rs1] xor r[rs2] if the i field is zero, or
     r[rs1] xor sign_ext(simm13) if the i field is one, to the
     writable fields of the specified IU state register. (Note the
     exclusive-or operation.)  Note that WRY is distinguished from
     WRASR only by the rd field. The rd field must be zero and op3 =
     0x30 to write the Y register." */
  if (reg_rd == 0) {
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y) = value_xor;
  }

  else {

    /* all other wrasr instructions are privileged: */
    TME_SPARC_INSN_PRIV;

    abort();
  }

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_wrpsr, tme_uint32_t)
{
  tme_uint32_t value;
  tme_uint32_t mask_writable;
  tme_uint32_t insn;
  unsigned int cwp;
  unsigned int cwp_offset;

  TME_SPARC_INSN_PRIV;

  /* if we haven't detected the idle PC yet: */
  if (__tme_predict_false(TME_SPARC_IDLE_TYPE_PC_STATE(ic->tme_sparc_idle_pcs_32[0]) != 0)) {

    /* "WRPSR ... writes r[rs1] xor r[rs2] if the i field is zero, or
       r[rs1] xor sign_ext(simm13) if the i field is one, to the
       writable fields of the specified IU state register. (Note the
       exclusive-or operation.)" */
    value = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

    /* the sunos32-type-0 idle type detects these four instructions
       in disassembly order:

       swtch():
         [...]
	 rd %psr, %l0
	 andn %l0, 0xf00, %g1
	 or %g1, 0xa00, %g1
	 mov %g1, %psr

       and then these two instructions in disassembly order, when
       %g1 sets PIL to 0x0:

       sw_testq():
         [...]
	 mov  %g1, %psr
	 b,a disp22 ; idle()

       this detects swtch.s:idle() via swtch() and sw_testq()
       in SunOS/sparc 4.1.4: */
    if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0)) {
      if (((tme_sparc_fetch_nearby(ic, -3)
	    & ~ ((tme_uint32_t)
		 ((31 << 14)		/* rs1 (reserved) */
		  + (1 << 13)		/* i (reserved) */
		  + 0x1fff)))		/* imm13 (reserved) */
	   == ((tme_uint32_t)
	       (2 << 30)		/* format */
	       + (16 << 25)		/* rd (%l0) */
	       + (0x29 << 19)))		/* op3 (rdpsr) */
	  && (tme_sparc_fetch_nearby(ic, -2)
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (1 << 25)		/* rd (%g1) */
		  + (0x05 << 19)	/* op3 (andn) */
		  + (16 << 14)		/* rs1 (%l0) */
		  + (1 << 13)		/* i */
		  + TME_SPARC32_PSR_PIL))
	  && (tme_sparc_fetch_nearby(ic, -1)
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (1 << 25)		/* rd (%g1) */
		  + (0x02 << 19)	/* op3 (or) */
		  + (1 << 14)		/* rs1 (%g1) */
		  + (1 << 13)		/* i */
		  + (0xa * _TME_FIELD_MASK_FACTOR(TME_SPARC32_PSR_PIL))))
	  && ((TME_SPARC_INSN
	       & ~ ((tme_uint32_t)
		    (31 << 25)		/* rd (reserved) */
		    + (255 << 5)))	/* unused (zero) */
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (0x31 << 19)	/* op3 (wrpsr) */
		  + (1 << 14)		/* rs1 (%g1) */
		  + (0 << 13)		/* i */
		  + (0 << 0)))) {	/* rs2 (%g0) */

	/* enter state two: */
	ic->tme_sparc_idle_pcs_32[0] = TME_SPARC_IDLE_TYPE_PC_STATE(2);
      }
      else if (ic->tme_sparc_idle_pcs_32[0] == TME_SPARC_IDLE_TYPE_PC_STATE(2)
	       && (value & TME_SPARC32_PSR_PIL) == 0
	       && ((TME_SPARC_INSN
		    & ~ ((tme_uint32_t)
			 (31 << 25)	/* rd (reserved) */
			 + (255 << 5)))	/* unused (zero) */
		   == ((tme_uint32_t)
		       (2 << 30)	/* format */
		       + (0x31 << 19)	/* op3 (wrpsr) */
		       + (1 << 14)	/* rs1 (%g1) */
		       + (0 << 13)	/* i */
		       + (0 << 0)))	/* rs2 (%g0) */
	       && (((insn = tme_sparc_fetch_nearby(ic, 1))
		    & ~ ((tme_uint32_t)
			 0x3fffff))	/* disp22 */
		   == ((0 << 30)	/* format */
		       + (1 << 29)	/* a */
		       + ((TME_SPARC_COND_NOT
			   + TME_SPARC_COND_N) << 25) /* cond (a) */
		       + (2 << 22)))) {	/* op2 (bicc) */

	/* we have detected the idle PC: */
	ic->tme_sparc_idle_pcs_32[0]
	  = (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	     + sizeof(tme_uint32_t)
	     + (TME_FIELD_MASK_EXTRACTS(insn, (tme_uint32_t) 0x003fffff) << 2));
	tme_sparc_recode_invalidate_all(ic);
      }
      
      /* otherwise, reset to state one: */
      else {
	ic->tme_sparc_idle_pcs_32[0] = TME_SPARC_IDLE_TYPE_PC_STATE(1);
      }
    }

    /* the netbsd32-type-0 idle type detects these five instructions
       in disassembly order:

       idle_enter():
         [...]
	 wr %l1, 0, %psr    ; or wr %g1, 0, %psr
	 [any]
	 [any]
	 bnz,a disp22 ; idle_leave()
	  wr %l1, (IPL_SCHED << 8), %psr    ; or wr %g1, PSR_PIL, %psr

       this detects locore.s:idle_enter() and idle_leave() in
       NetBSD/sparc 1.6 through 4.99.19 (until locore.s revision 1.233
       on 20070517): */
    if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0)) {
      if ((value & TME_SPARC32_PSR_PIL) == 0
	  && ((TME_SPARC_INSN
	       & ~((tme_uint32_t)
		   (31 << 25)		/* rd (reserved) */
		   + (16 << 14)))	/* rs1 (mask %ln to %gn) */
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (0x31 << 19)	/* op3 (wrpsr) */
		  + (1 << 14)		/* rs1 (%g1) */
		  + (1 << 13)		/* i */
		  + 0))
	  && (((insn = tme_sparc_fetch_nearby(ic, 3))
	       & ~ ((tme_uint32_t)
		    0x3fffff))		/* disp22 */
	      == ((0 << 30)		/* format */
		  + (1 << 29)		/* a */
		  + ((TME_SPARC_COND_NOT
		      + TME_SPARC_COND_E) << 25) /* cond (nz) */
		  + (2 << 22)))		/* op2 (bicc) */
	  && ((tme_sparc_fetch_nearby(ic, 4)
	       & ~((tme_uint32_t)
		   (31 << 25)		/* rd (reserved) */
		   + (16 << 14)		/* rs1 (mask %ln to %gn) */
		   + (0x4 << 8)))	/* imm13 (mask PSR_PIL to (IPL_SCHED << 8)) */
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (0x31 << 19)	/* op3 (wrpsr) */
		  + (0x01 << 14)	/* rs1 (%g1) */
		  + (1 << 13)		/* i */
		  + (0x0b * _TME_FIELD_MASK_FACTOR(TME_SPARC32_PSR_PIL))))) {

	/* we have detected the range of idle loop PCs: */
	ic->tme_sparc_idle_pcs_32[0]
	  = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC);
	ic->tme_sparc_idle_pcs_32[1]
	  = (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	     + (sizeof(tme_uint32_t) * 3)
	     + (TME_FIELD_MASK_EXTRACTS(insn, (tme_uint32_t) 0x003fffff) << 2));
	tme_sparc_recode_invalidate_all(ic);
      }
    }

    /* the netbsd32-type-1 idle type detects these three instructions
       in disassembly order:

       cpu_switchto():
         [...]
	 wr %g2, IPL_SCHED << 8 ,%psr
	 ret
	  restore %g0, %g1, %o0
       cpu_idle:

       this detects locore.s:cpu_idle() via cpu_switchto() in
       NetBSD/sparc 4.99.20 through this current writing
       (starting with locore.s revision 1.233 on 20070517)
       but only in a non-MULTIPROCESSOR kernel: */
    if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1)) {
      if (((TME_SPARC_INSN
	    & ~ ((tme_uint32_t)
		 (31 << 25)))		/* rd (reserved) */
	   == ((tme_uint32_t)
	       (2 << 30)		/* format */
	       + (0x31 << 19)		/* op3 (wrpsr) */
	       + (2 << 14)		/* rs1 (%g2) */
	       + (1 << 13)		/* i */
	       + (0xb * _TME_FIELD_MASK_FACTOR(TME_SPARC32_PSR_PIL))))
	  && (tme_sparc_fetch_nearby(ic, 1)
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (0 << 25)		/* rd (%g0) */
		  + (0x38 << 19)	/* op3 (jmpl) */
		  + (31 << 14)		/* rs1 (%i7) */
		  + (1 << 13)		/* i */
		  + 8))			/* simm13 */
	  && ((tme_sparc_fetch_nearby(ic, 2)
	       & ~ ((tme_uint32_t)
		    (0xff << 5)))	/* unused (zero) */
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (8 << 25)		/* rd (%o0) */
		  + (0x3d << 19)	/* op3 (restore) */
		  + (0 << 14)		/* rs1 (%g0) */
		  + (0 << 13)		/* i */
		  + (1 << 0)))) {	/* rs2 (%g1) */

	/* we have detected the idle PC: */
	ic->tme_sparc_idle_pcs_32[0]
	  = (ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	     + (sizeof(tme_uint32_t) * 3));
	tme_sparc_recode_invalidate_all(ic);
      }
    }
  }

  /* otherwise, if this is the idle PC: */
  else if (__tme_predict_false(ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
			       == ic->tme_sparc_idle_pcs_32[0])) {

    /* if this idle type has an idle PC range that begins with this
       wrpsr: */
    if (TME_SPARC_IDLE_TYPE_IS(ic,
			       (TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0
				))) {

      /* we don't mark an idle now, to avoid duplicating the idle PC
	 range checking code in sparc-execute.c.  but assuming that
	 the idle conditions exist, to accelerate going idle we zero
	 the remaining instruction burst: */
      ic->_tme_sparc_instruction_burst_remaining = 0;
      ic->_tme_sparc_instruction_burst_other = TRUE;
    }
  }

  /* "WRPSR ... writes r[rs1] xor r[rs2] if the i field is zero, or
     r[rs1] xor sign_ext(simm13) if the i field is one, to the
     writable fields of the specified IU state register. (Note the
     exclusive-or operation.)" */
  value = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* "If the result of a WRPSR instruction would cause the CWP field
     of the PSR to point to an unimplemented window, it causes an
     illegal_instruction trap and does not write the PSR." */
  cwp = TME_FIELD_MASK_EXTRACTU(value, TME_SPARC32_PSR_CWP);
  if (__tme_predict_false(cwp >= ic->tme_sparc_nwindows)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
  }

  /* set the new CWP offset: */
  TME_SPARC32_CWP_UPDATE(ic, cwp, cwp_offset);

  /* set the new PSR value: */
  mask_writable = (TME_SPARC32_PSR_ICC
		   | TME_SPARC32_PSR_EC
		   | TME_SPARC32_PSR_EF
		   | TME_SPARC32_PSR_PIL
		   | TME_SPARC32_PSR_S
		   | TME_SPARC32_PSR_PS
		   | TME_SPARC32_PSR_ET
		   | TME_SPARC32_PSR_CWP);
  value = (value & mask_writable) | (ic->tme_sparc32_ireg_psr & ~mask_writable);
  ic->tme_sparc32_ireg_psr = value;

  /* redispatch, since the executor may have cached information
     derived from the PSR (for example, default data ASI, ITLB, etc.)
     that we need to invalidate: */
  tme_sparc_redispatch(ic);
  /* NOTREACHED */
}

TME_SPARC_FORMAT3(tme_sparc32_wrwim, tme_uint32_t)
{
  tme_uint32_t value;

  TME_SPARC_INSN_PRIV;

  /* "WRWIM ... writes r[rs1] xor r[rs2] if the i field is zero, or
     r[rs1] xor sign_ext(simm13) if the i field is one, to the
     writable fields of the specified IU state register. (Note the
     exclusive-or operation.)" */
  value = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* "A WRWIM with all bits set to 1, followed by a RDWIM, yields a
     bit vector in which the imple- mented windows (and only the
     implemented windows) are indicated by 1s." */
  value &= (0xffffffff >> (32 - ic->tme_sparc_nwindows));
  
  ic->tme_sparc32_ireg_wim = value;

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_wrtbr, tme_uint32_t)
{
  tme_uint32_t value;

  TME_SPARC_INSN_PRIV;

  /* "WRTBR ... writes r[rs1] xor r[rs2] if the i field is zero, or
     r[rs1] xor sign_ext(simm13) if the i field is one, to the
     writable fields of the specified IU state register. (Note the
     exclusive-or operation.)" */
  value = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* "Bits 11 through 4 comprise the trap type (tt) field.  This 8-bit
     field is written by the hardware when a trap occurs, and retains
     its value until the next trap.  It provides an offset into the
     trap table.  The WRTBR instruction does not affect the tt
     field.  TBR_zero (0) Bits 3 through 0 are zeroes.  The WRTBR
     instruction does not affect this field.  For future compatibility,
     supervisor software should only issue a WRTBR instruction with a
     zero value in this field." */
  value = (value & 0xfffff000) | (ic->tme_sparc32_ireg_tbr & 0x00000ff0);
  ic->tme_sparc32_ireg_tbr = value;

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_flush, tme_uint32_t)
{
  /* nothing to do */
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_rett, tme_uint32_t)
{
  tme_uint32_t psr;
  unsigned int cwp;
  unsigned int cwp_offset;
  tme_uint32_t pc_next_next;

  /* "One of several traps may occur when an RETT is executed.  These
     are described in priority order (highest priority first):

     If traps are enabled (ET=1) and the processor is in user mode
     (S=0), a privileged_instruction trap occurs.

     If traps are enabled (ET=1) and the processor is in supervisor
     mode (S=1), an illegal_instruction trap occurs.

     If traps are disabled (ET=0), and (a) the processor is in user
     mode (S=0), or (b) a window_underflow condition is detected (WIM
     and 2^new_CWP ) = 1, or (c) either of the low-order two bits of
     the target address is nonzero, then the processor indicates a
     trap condition of (a) privileged_instruction, (b)
     window_underflow, or (c) mem_address_not_aligned (respectively)
     in the tt field of the TBR register, and enters the error_mode
     state." */
  psr = ic->tme_sparc32_ireg_psr;
  
  if (__tme_predict_false((psr & TME_SPARC32_PSR_S) == 0)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_privileged_instruction);
  }

  if (__tme_predict_false((psr & TME_SPARC32_PSR_ET) != 0)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_illegal_instruction);
  }
  
  cwp = TME_FIELD_MASK_EXTRACTU(psr, TME_SPARC32_PSR_CWP);
  cwp += 1;
  cwp %= ic->tme_sparc_nwindows;
  if (ic->tme_sparc32_ireg_wim & (((tme_uint32_t) 1) << cwp)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_window_underflow);
  }

  pc_next_next = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;
  if (__tme_predict_false((pc_next_next % sizeof(tme_uint32_t)) != 0)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_mem_address_not_aligned);
  }

  /* set the new PSR: */
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_CWP, cwp);
  psr |= TME_SPARC32_PSR_ET;
  psr &= ~TME_SPARC32_PSR_S;
  psr |= ((psr & TME_SPARC32_PSR_PS) * (TME_SPARC32_PSR_S / TME_SPARC32_PSR_PS));
  ic->tme_sparc32_ireg_psr = psr;

  /* set the new CWP offset: */
  TME_SPARC32_CWP_UPDATE(ic, cwp, cwp_offset);

  /* set the delayed control transfer: */
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

  /* redispatch, since the executor may have cached information
     derived from the PSR (for example, default data ASI, ITLB, etc.)
     that we need to invalidate: */
  tme_sparc_redispatch(ic);
}

TME_SPARC_FORMAT3(tme_sparc32_save_restore, tme_uint32_t)
{
  int direction;
  tme_uint32_t psr;
  unsigned int cwp;
  unsigned int cwp_offset;
  unsigned int reg_rd;

  /* calculate the window direction: */
  direction = -1 + (((TME_SPARC_INSN & TME_BIT(19)) != 0) * 2);

  /* calculate the new CWP: */
  psr = ic->tme_sparc32_ireg_psr;
  cwp = TME_FIELD_MASK_EXTRACTU(psr, TME_SPARC32_PSR_CWP);
  cwp += direction;
  cwp %= ic->tme_sparc_nwindows;

  /* if the new window is invalid: */
  if (__tme_predict_false((ic->tme_sparc32_ireg_wim & (((tme_uint32_t) 1) << cwp)) != 0)) {
    TME_SPARC_INSN_TRAP((direction < 0)
			? TME_SPARC32_TRAP_window_overflow
			: TME_SPARC32_TRAP_window_underflow);
  }

  /* write the new PSR: */
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_CWP, cwp);
  ic->tme_sparc32_ireg_psr = psr;

  /* set the new CWP offset: */
  TME_SPARC32_CWP_UPDATE(ic, cwp, cwp_offset);

  /* decode rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* do the add: */
  ic->tme_sparc_ireg_uint32(reg_rd) = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc32_ticc, tme_uint32_t)
{
  tme_uint8_t conds_mask_icc;
  tme_uint16_t conds_mask;
  unsigned int cond;

  conds_mask_icc = _tme_sparc_conds_icc[TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_ICC)];

  /* add the not-conditions to the conditions mask: */
  conds_mask = conds_mask_icc ^ 0xff;
  conds_mask = (conds_mask << 8) | conds_mask_icc;

  /* get the condition field: */
  cond = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, (0xf << 25));

  /* if this condition is true: */
  if (conds_mask & TME_BIT(cond)) {
    TME_SPARC_INSN_TRAP(TME_SPARC32_TRAP_trap_instruction((TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2) & 0x7f));
  }

  TME_SPARC_INSN_OK;
}

#ifdef TME_HAVE_INT64_T

#undef TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) (9)

TME_SPARC_FORMAT3(tme_sparc64_movcc, tme_uint64_t)
{
  tme_uint32_t insn;
  tme_uint32_t cond;
  tme_uint32_t cc;
  tme_uint32_t conds_mask;
  tme_uint32_t cc_i;

  insn = TME_SPARC_INSN;

  /* get the cond field: */
  cond = TME_BIT(TME_FIELD_MASK_EXTRACTU(insn, (0xf << 14)));
  
  /* if cc2 is set, this is an integer instruction: */
  if (insn & TME_BIT(18)) {

    /* if cc0 is set, this is an illegal instruction: */
    if (__tme_predict_false(insn & TME_BIT(11))) {
      TME_SPARC_INSN_ILL(ic);
    }

    /* get %icc or %xcc, depending on cc1: */
    cc = ic->tme_sparc64_ireg_ccr;
    if (insn & TME_BIT(12)) {
      cc /= (TME_SPARC64_CCR_XCC / TME_SPARC64_CCR_ICC);
    }
    cc = TME_FIELD_MASK_EXTRACTU(cc, TME_SPARC64_CCR_ICC);

    /* get the conditions mask: */
    conds_mask = _tme_sparc_conds_icc[cc];
  }

  /* otherwise, this is a floating-point instruction: */
  else {
    TME_SPARC_INSN_FPU;

    /* get the right %fcc: */
    cc_i = TME_FIELD_MASK_EXTRACTU(insn, (0x3 << 11));
    if (cc_i == 0) {
      cc = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc_fpu_fsr, TME_SPARC_FSR_FCC);
    }
    else {
      cc = (ic->tme_sparc_fpu_xfsr >> (2 * (cc_i - 1))) & 0x3;
    }

    /* get the conditions mask: */
    conds_mask = _tme_sparc_conds_fcc[cc];
  }

  /* add the not-conditions to the conditions mask: */
  conds_mask += ((~conds_mask) << 8);

  /* if this condition is true, move the register: */
  if (conds_mask & cond) {
    TME_SPARC_FORMAT3_RD
      = ((insn & TME_BIT(13))
	 ? TME_FIELD_MASK_EXTRACTS(insn, (tme_uint64_t) 0x7ff)
	 : TME_SPARC_FORMAT3_RS2);
  }
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_movr, tme_uint64_t)
{
  tme_int64_t rs1;
  tme_uint32_t conds_mask;
  tme_uint32_t insn;
  unsigned int cond;

  /* make a conditions mask, with the E and LE conditions if the
     register is zero, and with the L and LE conditions if the
     register is less than zero: */
  rs1 = TME_SPARC_FORMAT3_RS1;
  conds_mask
    = (((rs1 == 0)
	* (TME_BIT(TME_SPARC_COND_E)
	   + TME_BIT(TME_SPARC_COND_LE)))
       | ((rs1 < 0)
	  * (TME_BIT(TME_SPARC_COND_L)
	     + TME_BIT(TME_SPARC_COND_LE))));

  /* add the not-conditions to the conditions mask: */
  conds_mask += ((~conds_mask) << 4);

  /* get the instruction: */
  insn = TME_SPARC_INSN;

  /* get the condition: */
  cond = TME_FIELD_MASK_EXTRACTU(insn, (0x7 << 10));
  if (__tme_predict_false((cond & 3) == TME_SPARC_COND_N)) {
    TME_SPARC_INSN_ILL(ic);
  }

  /* if the comparison is true, move the register: */
  if (conds_mask & (1 << cond)) {
    TME_SPARC_FORMAT3_RD
      = ((insn & TME_BIT(13))
	 ? TME_FIELD_MASK_EXTRACTS(insn, (tme_uint64_t) 0x3ff)
	 : TME_SPARC_FORMAT3_RS2);
  }
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_tcc, tme_uint64_t)
{
  tme_uint32_t insn;
  tme_uint32_t cc;
  tme_uint32_t conds_mask;
  tme_uint32_t cond;
  tme_uint32_t sw_trap;

  insn = TME_SPARC_INSN;

  /* if cc0 is set, this is an illegal instruction: */
  if (__tme_predict_false(insn & TME_BIT(11))) {
    TME_SPARC_INSN_ILL(ic);
  }

  /* get %icc or %xcc, depending on cc1: */
  cc = ic->tme_sparc64_ireg_ccr;
  if (insn & TME_BIT(12)) {
    cc /= (TME_SPARC64_CCR_XCC / TME_SPARC64_CCR_ICC);
  }
  cc = TME_FIELD_MASK_EXTRACTU(cc, TME_SPARC64_CCR_ICC);

  /* get the conditions mask: */
  conds_mask = _tme_sparc_conds_icc[cc];

  /* add the not-conditions to the conditions mask: */
  conds_mask += ((~conds_mask) << 8);

  /* get the cond field: */
  cond = TME_FIELD_MASK_EXTRACTU(insn, (0xf << 25));

  /* if this condition is true: */
  if (conds_mask & TME_BIT(cond)) {
    sw_trap
      = ((TME_SPARC_FORMAT3_RS1
	  + ((insn & TME_BIT(13))
	     ? TME_FIELD_MASK_EXTRACTU(insn, 0x7f)
	     : TME_SPARC_FORMAT3_RS2))
	 & 0x7f);
    TME_SPARC_INSN_TRAP(TME_SPARC64_TRAP_trap_instruction(sw_trap));
  }
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_save_restore, tme_uint64_t)
{
  tme_uint32_t winstates;
  tme_uint32_t winstates_addend;
  tme_uint32_t trap;
  tme_uint32_t winstates_mask;
  tme_uint32_t cwp;
  unsigned int cwp_offset;
  unsigned int reg_rd;

  /* log the save or restore: */
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("%s cwp %u canrestore %u cansave %u otherwin %u cleanwin %u"),
		 ((TME_SPARC_INSN & TME_BIT(19))
		  ? "restore"
		  : "save"),
		 ic->tme_sparc64_ireg_cwp,
		 ic->tme_sparc64_ireg_canrestore,
		 ic->tme_sparc64_ireg_cansave,
		 ic->tme_sparc64_ireg_otherwin,
		 ic->tme_sparc64_ireg_cleanwin));

  /* get the common sparc register-window state registers: */
  winstates = ic->tme_sparc64_ireg_winstates;

  /* assume that this is a restore: */
  /* "If the RESTORE instruction does not trap, it decrements the CWP
     (mod NWINDOWS) to restore the register window [...] It also
     updates the state of the register windows by decrementing
     CANRESTORE and incrementing CANSAVE." */
  winstates_addend
    = (TME_SPARC64_WINSTATES_CWP(-1)
       + TME_SPARC64_WINSTATES_CANRESTORE(-1)
       + TME_SPARC64_WINSTATES_CANSAVE(+1));

  /* if this is a save: */
  if ((TME_SPARC_INSN & TME_BIT(19)) == 0) {

    /* "If CANSAVE != 0, the SAVE instruction checks whether the new
       window needs to be cleaned.  It causes a clean_window trap if
       the number of unused clean windows is zero, that is, (CLEANWIN
       - CANRESTORE = 0)" */
#if TME_SPARC64_WINSTATES_CWP(1) != 1
#error "TME_SPARC64_WINSTATES_CWP() changed"
#endif
    if (__tme_predict_false(ic->tme_sparc64_ireg_cleanwin
			    == ((tme_uint8_t)
				(winstates
				 / TME_SPARC64_WINSTATES_CANRESTORE(1))))) {

      /* NB: we still have to check if CANSAVE == 0; a spill trap
	 happens before a clean window trap: */
      /* NB: if this is a spill trap, the exact trap vector will be
	 generated by tme_sparc64_trap(): */
      trap
	= ((winstates & TME_SPARC64_WINSTATES_CANSAVE(-1))
	   ? TME_SPARC64_TRAP_clean_window
	   : TME_SPARC64_TRAP_spill_normal(0));

      /* trap: */
      tme_sparc64_trap(ic, trap);
      TME_SPARC_INSN_OK;
    }

    /* "If the SAVE instruction does not trap, it increments the CWP
       (mod NWINDOWS) to provide a new register window and updates the
       state of the register windows by decrementing CANSAVE and
       incrementing CANRESTORE."  */
    winstates_addend
      = (TME_SPARC64_WINSTATES_CWP(+1)
	 + TME_SPARC64_WINSTATES_CANRESTORE(+1)
	 + TME_SPARC64_WINSTATES_CANSAVE(-1));
  }

  /* update the common sparc register-window state registers: */
  winstates += winstates_addend;

  /* get any mask for the common sparc register-window state registers: */
  winstates_mask = ic->tme_sparc64_ireg_winstates_mask;

  /* iff this instruction traps, one of CANRESTORE and CANSAVE (the
     one that this instruction decrements) must not have carried out
     of its -1 addend: */
  if (__tme_predict_false((winstates
			   & (TME_SPARC64_WINSTATES_CANRESTORE(-1)
			      + TME_SPARC64_WINSTATES_CANRESTORE(1)
			      + TME_SPARC64_WINSTATES_CANSAVE(-1)
			      + TME_SPARC64_WINSTATES_CANSAVE(1))) == 0)) {

    /* NB: the exact spill or fill trap vector will be generated by
       tme_sparc64_trap(): */
    trap
      = ((TME_SPARC_INSN & TME_BIT(19))
	 ? TME_SPARC64_TRAP_fill_normal(0)
	 : TME_SPARC64_TRAP_spill_normal(0));

    /* trap: */
    tme_sparc64_trap(ic, trap);
    TME_SPARC_INSN_OK;
  }

  /* if the common sparc register-window state registers can be
     masked: */
  assert (winstates_mask != 0);
  if (TRUE) {

    /* write the updated common sparc register-window state registers: */
    winstates &= winstates_mask;
    ic->tme_sparc64_ireg_winstates = winstates;

    /* get the updated CWP: */
#if TME_SPARC64_WINSTATES_CWP(1) != 1
#error "TME_SPARC64_WINSTATES_CWP() changed"
#endif
    cwp = (tme_uint8_t) (winstates / TME_SPARC64_WINSTATES_CWP(1));
  }

  /* set the new CWP offset: */
  TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);

  /* decode rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);
  TME_SPARC_REG_INDEX(ic, reg_rd);

  /* do the add: */
  ic->tme_sparc_ireg_uint64(reg_rd) = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_return, tme_uint64_t)
{
  unsigned int reg_i0;
  tme_uint32_t winstates;
  tme_uint64_t pc_next_next;
  tme_uint32_t ls_faults;
  tme_uint32_t winstates_mask;
  tme_uint32_t cwp;
  unsigned int cwp_offset;

  /* log the return: */
  reg_i0 = 24;
  TME_SPARC_REG_INDEX(ic, reg_i0);
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("return 0x%016" TME_PRIx64 " cwp %u canrestore %u cansave %u otherwin %u cleanwin %u %%i0 0x%016" TME_PRIx64" %%g1 0x%016" TME_PRIx64" %%g2 0x%016" TME_PRIx64" %%g3 0x%016" TME_PRIx64" %%g4 0x%016" TME_PRIx64" %%g5 0x%016" TME_PRIx64),
		 (TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2),
		 ic->tme_sparc64_ireg_cwp,
		 ic->tme_sparc64_ireg_canrestore,
		 ic->tme_sparc64_ireg_cansave,
		 ic->tme_sparc64_ireg_otherwin,
		 ic->tme_sparc64_ireg_cleanwin,
		 ic->tme_sparc_ireg_uint64(reg_i0),
		 ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G1),
		 ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G2),
		 ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G3),
		 ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G4),
		 ic->tme_sparc_ireg_uint64(TME_SPARC_G0_OFFSET(ic) + TME_SPARC_IREG_G5)));

  /* get the common sparc register-window state registers: */
  winstates = ic->tme_sparc64_ireg_winstates;

  /* "The RETURN instruction [...] has the window semantics of a
     RESTORE instruction" */
  /* "If the RESTORE instruction does not trap, it decrements the CWP
     (mod NWINDOWS) to restore the register window [...] It also
     updates the state of the register windows by decrementing
     CANRESTORE and incrementing CANSAVE." */
  winstates
    += (TME_SPARC64_WINSTATES_CWP(-1)
	+ TME_SPARC64_WINSTATES_CANRESTORE(-1)
	+ TME_SPARC64_WINSTATES_CANSAVE(+1));

  /* if CANRESTORE didn't carry out of its -1 addend: */
  if (__tme_predict_false((winstates
			   & (TME_SPARC64_WINSTATES_CANRESTORE(-1)
			      + TME_SPARC64_WINSTATES_CANRESTORE(1))) == 0)) {

    /* trap: */
    /* NB: the exact fill trap vector will be generated by
       tme_sparc64_trap(): */
    tme_sparc64_trap(ic, TME_SPARC64_TRAP_fill_normal(0));
    TME_SPARC_INSN_OK;
  }

  /* get the target address: */
  pc_next_next = TME_SPARC_FORMAT3_RS1 + TME_SPARC_FORMAT3_RS2;
  pc_next_next &= ic->tme_sparc_address_mask;

  /* do the delayed control transfer: */
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

  /* check that the target address is aligned and not in the virtual
     address hole: */
  ls_faults = TME_SPARC_LS_FAULT_NONE;
  if (__tme_predict_false((pc_next_next
			   + ic->tme_sparc64_ireg_va_hole_start)
			  > ((ic->tme_sparc64_ireg_va_hole_start * 2) - 1))) {
    ls_faults += TME_SPARC64_LS_FAULT_VA_RANGE_NNPC;
  }
  if (__tme_predict_false(((unsigned int) pc_next_next) % sizeof(tme_uint32_t))) {
    ls_faults += TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED;
  }

  /* if the target address has caused a trap: */
  if (__tme_predict_false(ls_faults != TME_SPARC_LS_FAULT_NONE)) {
    tme_sparc_nnpc_trap(ic, ls_faults);
  }

  /* get any mask for the common sparc register-window state registers: */
  winstates_mask = ic->tme_sparc64_ireg_winstates_mask;

  /* if the common sparc register-window state registers can be
     masked: */
  assert (winstates_mask != 0);
  if (TRUE) {

    /* write the updated common sparc register-window state registers: */
    winstates &= winstates_mask;
    ic->tme_sparc64_ireg_winstates = winstates;

    /* get the updated CWP: */
#if TME_SPARC64_WINSTATES_CWP(1) != 1
#error "TME_SPARC64_WINSTATES_CWP() changed"
#endif
    cwp = (tme_uint8_t) (winstates / TME_SPARC64_WINSTATES_CWP(1));
  }

  /* set the new CWP offset: */
  TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);

  /* redispatch, since if we're running from a recode instructions
     thunk, and we don't redispatch, our pc_next_next will be lost: */
  /* XXX FIXME - return is too complicated to not need an assist, but
     if we could preserve its pc_next_next in the insns thunk, we
     could avoid this expensive redispatch: */
  tme_sparc_redispatch(ic);
}

TME_SPARC_FORMAT3(tme_sparc64_saved_restored, tme_uint64_t)
{
  tme_uint32_t insn;
  tme_uint32_t winstates_addend_insn;
  tme_uint32_t winstates_addend_otherwin;
  tme_uint32_t cleanwin;
  tme_uint32_t winstates;
  tme_uint32_t winstates_mask;

  TME_SPARC_INSN_PRIV;

  /* log the saved or restored: */
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("%s cwp %u canrestore %u cansave %u otherwin %u cleanwin %u"),
		 ((TME_SPARC_INSN & TME_BIT(25))
		  ? "restored"
		  : "saved"),
		 ic->tme_sparc64_ireg_cwp,
		 ic->tme_sparc64_ireg_canrestore,
		 ic->tme_sparc64_ireg_cansave,
		 ic->tme_sparc64_ireg_otherwin,
		 ic->tme_sparc64_ireg_cleanwin));

  insn = TME_SPARC_INSN;

  /* if fcn is greater than one (if any of bits 26 through 29 are
     set), this is an illegal instruction: */
  if (__tme_predict_false(insn & (TME_BIT(30) - TME_BIT(26)))) {
    TME_SPARC_INSN_ILL(ic);
  }

  /* assume that this is a saved: */
  /* "SAVED increments CANSAVE.  If OTHERWIN = 0, it decrements
     CANRESTORE.  If OTHERWIN != 0, it decrements OTHERWIN.  */
  winstates_addend_insn = TME_SPARC64_WINSTATES_CANSAVE(+1);
  winstates_addend_otherwin = TME_SPARC64_WINSTATES_CANRESTORE(-1);

  /* if this is a restored: */
  if (insn & TME_BIT(25)) {

    /* "RESTORED increments CANRESTORE.  If CLEANWIN < (NWINDOWS - 1),
       RESTORED increments CLEANWIN.  If OTHERWIN = 0, it decrements
       CANSAVE.  If OTHERWIN != 0, it decrements OTHERWIN" */
    cleanwin = ic->tme_sparc64_ireg_cleanwin + 1;
    cleanwin -= (cleanwin >= ic->tme_sparc_nwindows);
    ic->tme_sparc64_ireg_cleanwin = cleanwin;
    winstates_addend_insn = TME_SPARC64_WINSTATES_CANRESTORE(+1);
    winstates_addend_otherwin = TME_SPARC64_WINSTATES_CANSAVE(-1);
  }
  
  /* get the common sparc register-window state registers and
     increment either CANSAVE or CANRESTORE: */
  winstates = winstates_addend_insn;
  winstates += ic->tme_sparc64_ireg_winstates;

  /* if OTHERWIN is nonzero, decrement OTHERWIN instead of the
     other of CANSAVE or CANRESTORE: */
  if (winstates & TME_SPARC64_WINSTATES_OTHERWIN(-1)) {
    winstates_addend_otherwin = TME_SPARC64_WINSTATES_OTHERWIN(-1);
  }
  winstates += winstates_addend_otherwin;

  /* get any mask for the common sparc register-window state registers: */
  winstates_mask = ic->tme_sparc64_ireg_winstates_mask;

  /* if the common sparc register-window state registers can be
     masked: */
  assert (winstates_mask != 0);
  if (TRUE) {

    /* write the updated common sparc register-window state registers: */
    winstates &= winstates_mask;
    ic->tme_sparc64_ireg_winstates = winstates;
  }

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_flushw, tme_uint64_t)
{
  /* "FLUSHW acts as a NOP if CANSAVE = NWINDOWS - 2.
     Otherwise, there is more than one active window, so
     FLUSHW causes a spill exception." */
  if ((ic->tme_sparc64_ireg_cansave + 2)
      != TME_SPARC_NWINDOWS(ic)) {

    /* NB: the exact spill vector will be generated by
       tme_sparc64_trap(): */
    tme_sparc64_trap(ic, TME_SPARC64_TRAP_spill_normal(0));
  }

  /* if we haven't detected the idle PC yet: */
  if (__tme_predict_false(TME_SPARC_IDLE_TYPE_PC_STATE(ic->tme_sparc_idle_pcs_64[0]) != 0)) {

    /* the netbsd64-type-1 idle type detects these five instructions
       in disassembly order:

       cpu_idle():
         retl
          nop
       cpu_switchto():
         save  %sp, simm13, %sp
	 flushw
	 wrpr %g0, PSTATE_KERN, %pstate

       this detects locore.s:cpu_idle() via cpu_switchto() in
       NetBSD/sparc64 4.99.21 through this current writing
       (starting with locore.s revision 1.250 on 20070528): */
    /* NB: in locore.s revisions 1.244 through 1.249, there was an
       additional flushw at the beginning of cpu_switchto().  we
       assume that nobody cares about -current kernels from this
       period not idling, and don't detect this: */
    if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1)) {
      if ((tme_sparc_fetch_nearby(ic, -3)
	   == ((tme_uint32_t)
	       (2 << 30)		/* format */
	       + (0 << 25)		/* rd (%g0) */
	       + (0x38 << 19)		/* op3 (jmpl) */
	       + (15 << 14)		/* rs1 (%o7) */
	       + (1 << 13)		/* i */
	       + 8))			/* simm13 */
	  && (tme_sparc_fetch_nearby(ic, -2)
	      == ((0 << 30)		/* format */
		  + (0 << 25)		/* rd (%g0) */
		  + (4 << 22)
		  + 0))			/* imm22 */
	  && ((tme_sparc_fetch_nearby(ic, -1)
	       & (0 - (tme_uint32_t) (1 << 13)))
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (14 << 25)		/* rd (%o6) */
		  + (0x3c << 19)	/* op3 (save) */
		  + (14 << 14)		/* rs1 (%o6) */
		  + (1 << 13)))
	  && ((tme_sparc_fetch_nearby(ic, 1)
	       & ~ (tme_uint32_t) TME_SPARC64_PSTATE_AM)
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (6 << 25)		/* %pstate */
		  + (0x32 << 19)	/* op3 (wrpr) */
		  + (0 << 14)		/* rs1 (%g0) */
		  + (1 << 13)		/* i */
		  + TME_SPARC64_PSTATE_PRIV))) {

	/* we have detected the idle PC: */
	ic->tme_sparc_idle_pcs_64[0]
	  = (ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
	     - (sizeof(tme_uint32_t) * 3));
	tme_sparc_recode_invalidate_all(ic);
      }
    }
  }

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_prefetch, tme_uint64_t)
{
  tme_uint32_t fcn;

  /* "In nonprivileged code, a prefetch instruction has the same
     observable effect as a NOP; its execution is nonblocking and
     cannot cause an observable trap.  In particular, a prefetch
     instruction shall not trap if it is applied to an illegal or
     nonexistent memory address." */
  /* we interpret this to mean that there are no ASI and address
     combinations that can cause a privileged_action trap, which means
     we can ignore them altogether: */
  /* NB: we assume that all sparc64 implementations treat prefetch
     the same for privileged and nonprivileged code: */

  fcn = TME_SPARC_INSN & (0x1f << 25);
  fcn -= (5 << 25);
  if (__tme_predict_false(fcn <= ((15 - 5) << 25))) {
    TME_SPARC_INSN_ILL(ic);
  }

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_rdpr, tme_uint64_t)
{
  unsigned int reg_rs1;
  unsigned int tl;
  tme_uint64_t value;
  tme_uint64_t va_hole_start;

  TME_SPARC_INSN_PRIV;

  /* assume that we aren't reading a virtual address that can't be in
     any virtual address hole: */
  va_hole_start = 0;

  /* dispatch on rs1: */
  reg_rs1 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RS1);
  switch (reg_rs1) {

    /* TPC, TNPC, TSTATE, TT: */
  case 0:
  case 1:
  case 2:
  case 3:
    tl = ic->tme_sparc64_ireg_tl;
    if (tl > 0) {
      switch (reg_rs1) {
      case 0:
	value = ic->tme_sparc64_ireg_tpc(tl);
	va_hole_start = ic->tme_sparc64_ireg_va_hole_start;
	break;
      case 1:
	value = ic->tme_sparc64_ireg_tnpc(tl);
	va_hole_start = ic->tme_sparc64_ireg_va_hole_start;
	break;
      case 2: value = ic->tme_sparc64_ireg_tstate(tl); break;
      default: value = ic->tme_sparc64_ireg_tt(tl); break;
      }
      break;
    }
    /* FALLTHROUGH */

  default:
    TME_SPARC_INSN_ILL(ic);
    TME_SPARC_INSN_OK;

    /* TICK: */
  case 4:
    tme_sparc_recode_verify_reg_tick(ic, &TME_SPARC_FORMAT3_RD);
    if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {
      TME_SPARC_INSN_OK;
    }
    value = tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint;
    value += ic->tme_sparc64_ireg_tick_offset;
    value &= TME_SPARC64_TICK_COUNTER;
    if (__tme_predict_false(ic->tme_sparc64_ireg_tick_npt)) {
      value |= TME_SPARC64_TICK_NPT;
    }
    tme_sparc_recode_verify_reg_tick_now(ic, &value);
    break;

  case 5:
    value = ic->tme_sparc64_ireg_tba;
    va_hole_start = ic->tme_sparc64_ireg_va_hole_start;
    break;
  case 6: value = ic->tme_sparc64_ireg_pstate; break;
  case 7: value = ic->tme_sparc64_ireg_tl; break;
  case 8: value = ic->tme_sparc64_ireg_pil; break;
  case 9: value = ic->tme_sparc64_ireg_cwp; break;
  case 10: value = ic->tme_sparc64_ireg_cansave; break;
  case 11: value = ic->tme_sparc64_ireg_canrestore; break;
  case 12: value = ic->tme_sparc64_ireg_cleanwin; break;
  case 13: value = ic->tme_sparc64_ireg_otherwin; break;
  case 14: value = ic->tme_sparc64_ireg_wstate; break;

    /* XXX FIXME - add support for implementations with an FQ: */
#if 0
    /* FQ: */
  case 15:
    break;
#endif /* 0 */

  case 31: value = ic->tme_sparc64_ireg_ver; break;
  }

  /* get any virtual address hole form value: */
  value = value | (0 - (va_hole_start * 2));
  value = (value ^ va_hole_start) + va_hole_start;

  TME_SPARC_FORMAT3_RD = value;
  TME_SPARC_INSN_OK;
}

/* this detects the sunos64-type-0 idle type during an splhigh()
   call: */
static void
_tme_sparc64_idle_type_sunos64_type_0_splhigh(struct tme_sparc *ic)
{
  tme_uint64_t address_splhigh;
  unsigned int reg_index;
  tme_uint64_t address_call_splhigh;
  tme_uint32_t insn_ld;
  tme_uint64_t address_t_startpc;
  tme_uint32_t dtlb_hash;
  const struct tme_sparc_tlb *dtlb;
  const tme_shared tme_uint8_t *memory;
  tme_uint64_t address_idle;
  int size_idle;

  /* at this point, we have already detected that we are in splhigh():

     splhigh():
       rdpr %pil, %o1
       [any]
       [any]
       [any]
       wrpr 0xa, %pil

     PC is at the wrpr instruction.  get the address of splhigh(): */
  address_splhigh
    = (ic->tme_sparc_ireg_int64(TME_SPARC_IREG_PC)
       - (sizeof(tme_uint32_t)
	  * 4));

  /* we want to detect when splhigh() has been called as part of these
     instructions in disp.c:disp(), in disassembly order:

     disp():
       [...]
       call disp_getwork
       mov  %i3, %o0
       orcc  %g0, %o0, %l0
       bne,a  disp+any
       [any]
       call splhigh
       ld [ %i3 + any ], %l0

     when disp() has no thread to run on this CPU, it calls
     disp.c:disp_getwork() to see if it can get a thread to run from
     another CPU.  if it can't get a thread to run, it calls splhigh()
     before setting the idle thread to run.

     the delay slot after the splhigh() call loads the kthread_t * for
     the idle thread into %l0.

     the third address-sized member of a kthread_t is the initial PC
     for the thread, which for the idle thread will be disp.c:idle().

     since splhigh() doesn't establish its own register window, as
     long as we can detect the specific splhigh() call above, will be
     able to detect the address of idle() through it's caller's %l0.

     the challenge is to detect this specific splhigh() call.  we take
     advantage of the fact that disp_getwork() contains its own call
     to splhigh(), and since splhigh() doesn't establish its own
     register window, the return address that its caller will use
     should be in %i7: */

  /* assume that splhigh()'s caller is disp_getwork(), and that
     disp_getwork()'s caller is disp(), and get what should be the
     address of the disp() call to splhigh(): */
  reg_index = 31; /* %i7 */
  TME_SPARC_REG_INDEX(ic, reg_index);
  address_call_splhigh
    = (ic->tme_sparc_ireg_uint64(reg_index)
       + (sizeof(tme_uint32_t)
	  * 5));

  /* if this is a call to splhigh(): */
  if (tme_sparc_insn_peek(ic,
			  address_call_splhigh)
      == ((((tme_uint32_t) 0x1) << 30)	/* format */
	  + (((address_splhigh
	       - address_call_splhigh)
	      / sizeof(tme_uint32_t))
	     & ((1 << 30) - 1)))) {	/* disp30 */

    /* if the next instruction is a ld or ldx of %l0: */
    insn_ld
      = tme_sparc_insn_peek(ic,
			    (address_call_splhigh
			     + sizeof(tme_uint32_t)));
    if (((insn_ld
	  & ((tme_uint32_t)
	     (0x3 << 30)
	     + TME_SPARC_FORMAT3_MASK_RD))
	 == ((tme_uint32_t)
	     (0x3 << 30)		/* format */
	     + (16 << 25)))		/* rd (%l0) */
	&& (((insn_ld & (0x3f << 19))
	     == (0x00 << 19))		/* op3 (ld) */
	    || ((insn_ld & (0x3f << 19))
		== (0x0b << 19)))) {	/* op3 (ldx) */

      /* we have detected the disp() call of splhigh().  enter state
	 two: */
      ic->tme_sparc_idle_pcs_64[0]
	= (address_call_splhigh
	   + TME_SPARC_IDLE_TYPE_PC_STATE(2));
    }
  }

  /* if this is the splhigh() call from disp(): */
  reg_index = 15; /* %o7 */
  TME_SPARC_REG_INDEX(ic, reg_index);
  if (ic->tme_sparc_ireg_uint64(reg_index)
      == (ic->tme_sparc_idle_pcs_64[0]
	  - TME_SPARC_IDLE_TYPE_PC_STATE(2))) {

    /* get the address of the t_startpc member in the kthread_t * in
       %l0.  the t_startpc member is the third address-sized
       member: */
    reg_index = 16; /* %l0 */
    TME_SPARC_REG_INDEX(ic, reg_index);
    address_t_startpc = ic->tme_sparc_ireg_uint64(reg_index);
    address_t_startpc
      += ((ic->tme_sparc64_ireg_pstate
	   & TME_SPARC64_PSTATE_AM)
	  ? (sizeof(tme_uint32_t) * 2)
	  : (sizeof(tme_uint64_t) * 2));
    address_t_startpc &= ic->tme_sparc_address_mask;

    /* get the DTLB entry for the t_startpc member: */
    dtlb_hash
      = TME_SPARC_TLB_HASH(ic,
			   ic->tme_sparc_memory_context_default,
			   address_t_startpc);
    dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, dtlb_hash)];

    /* if the t_startpc member hits the DTLB: */
    memory = dtlb->tme_sparc_tlb_emulator_off_read;
    if (tme_bus_tlb_is_valid(&dtlb->tme_sparc_tlb_bus_tlb)
	&& dtlb->tme_sparc_tlb_context == ic->tme_sparc_memory_context_default
	&& address_t_startpc >= (tme_bus_addr64_t) dtlb->tme_sparc_tlb_addr_first
	&& (address_t_startpc + sizeof(tme_uint64_t) - 1) <= (tme_bus_addr64_t) dtlb->tme_sparc_tlb_addr_last
	&& TME_SPARC_TLB_ASI_MASK_OK(dtlb, ic->tme_sparc_asi_mask_data)
	&& (dtlb->tme_sparc_tlb_asi_mask & TME_SPARC64_ASI_FLAG_NO_FAULT) == 0
	&& memory != TME_EMULATOR_OFF_UNDEF) {

      /* read the t_startpc member: */
      if (ic->tme_sparc64_ireg_pstate
	  & TME_SPARC64_PSTATE_AM) {
	address_idle
	  = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (memory + address_t_startpc),
				  dtlb->tme_sparc_tlb_bus_rwlock,
				  sizeof(tme_uint32_t),
				  sizeof(tme_uint64_t));
	address_idle = tme_betoh_u32(address_idle);
      }
      else {
	address_idle
	  = tme_memory_bus_read64((const tme_shared tme_uint64_t *) (memory + address_t_startpc),
				  dtlb->tme_sparc_tlb_bus_rwlock,
				  sizeof(tme_uint64_t),
				  sizeof(tme_uint64_t));
	address_idle = tme_betoh_u64(address_idle);
      }

      /* get the size of idle(): */
      for (size_idle = 1; size_idle < 150; size_idle++) {

	/* if this is a save instruction: */
	if ((tme_sparc_insn_peek(ic,
				 (address_idle
				  + (size_idle
				     * sizeof(tme_uint32_t))))
	     & ((tme_uint32_t)
		(0x3 << 30)		/* format */
		+ (0x3f << 19)))	/* op3 */
	    == ((tme_uint32_t)
		(2 << 30)		/* format */
		+ (0x3c << 19))) {	/* op3 (save) */
	  break;
	}
      }

      /* we have detected idle(): */
      ic->tme_sparc_idle_pcs_64[0] = address_idle;
      ic->tme_sparc_idle_pcs_64[1] = address_idle + (size_idle * sizeof(tme_uint32_t));
      tme_sparc_recode_invalidate_all(ic);
    }
  }
}

TME_SPARC_FORMAT3(tme_sparc64_wrpr, tme_uint64_t)
{
  unsigned int reg_rd;
  tme_uint64_t value_xor;
  tme_uint64_t va_hole_start;
  tme_uint64_t value_xor_va_hole;
  unsigned int tl;
  unsigned int pil;
  signed int pil_change;
  unsigned int cwp;
  unsigned int cwp_offset;

  TME_SPARC_INSN_PRIV;

  /* get the xor form value to write: */
  value_xor = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* get the xor virtual address hole form value to write: */
  va_hole_start = ic->tme_sparc64_ireg_va_hole_start;
  value_xor_va_hole = value_xor | (0 - (va_hole_start * 2));
  value_xor_va_hole = (value_xor_va_hole ^ va_hole_start) + va_hole_start;

  /* dispatch on rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);
  switch (reg_rd) {

    /* TPC, TNPC, TSTATE, TT: */
  case 0:
  case 1:
  case 2:
  case 3:
    tl = ic->tme_sparc64_ireg_tl;
    if (tl > 0) {
      switch (reg_rd) {
      case 0: ic->tme_sparc64_ireg_tpc(tl) = value_xor_va_hole; break;
      case 1: ic->tme_sparc64_ireg_tnpc(tl) = value_xor_va_hole; break;
      case 2: ic->tme_sparc64_ireg_tstate(tl) = value_xor; break;
      default: ic->tme_sparc64_ireg_tt(tl) = value_xor; break;
      }
      break;
    }
    /* FALLTHROUGH */

  default:
    TME_SPARC_INSN_ILL(ic);
    TME_SPARC_INSN_OK;

    /* TICK: */
  case 4:
    ic->tme_sparc64_ireg_tick_npt = (value_xor & TME_SPARC64_TICK_NPT) != 0;
    ic->tme_sparc64_ireg_tick_offset
      = ((value_xor & TME_SPARC64_TICK_COUNTER)
	 - tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint);
    break;

  case 5: ic->tme_sparc64_ireg_tba = value_xor_va_hole & ~ (tme_uint64_t) ((2 << 14) - (1 << 0)); break;

    /* a write to the PSTATE register causes a redispatch because we may
       need to update cached state derived from it: */
  case 6:
    (*ic->_tme_sparc64_update_pstate)(ic, value_xor, TME_SPARC_TRAP_none);
    tme_sparc_redispatch(ic);
    TME_SPARC_INSN_OK;

    /* a write to the TL register causes a redispatch because we may
       need to update cached state derived from it: */
  case 7:
    ic->tme_sparc64_ireg_tl = value_xor & 0x7;
    tme_sparc_redispatch(ic);
    TME_SPARC_INSN_OK;

  case 8:

    /* get the next value of PIL: */
    pil = value_xor & 0xf;

    /* see how PIL is changing: */
    pil_change = ic->tme_sparc64_ireg_pil;
    pil_change = pil - pil_change;

    /* if we are about to raise PIL: */
    if (pil_change > 0) {

      /* do an interrupt check: */
      (*ic->_tme_sparc_external_check)(ic, TME_SPARC_EXTERNAL_CHECK_PCS_UPDATED);
    }

    /* update PIL: */
    ic->tme_sparc64_ireg_pil = pil;

    /* if we have lowered PIL: */
    if (pil_change < 0) {

      /* do an interrupt check: */
      (*ic->_tme_sparc_external_check)(ic, TME_SPARC_EXTERNAL_CHECK_NULL);
    }

    /* if we haven't detected the idle PC yet: */
    if (__tme_predict_false(TME_SPARC_IDLE_TYPE_PC_STATE(ic->tme_sparc_idle_pcs_64[0]) != 0)) {

      /* if this is a "wrpr 0, %pil" instruction: */
      if (TME_SPARC_INSN
	  == ((tme_uint32_t)
	      (2 << 30)		/* format */
	      + (8 << 25)	/* rd (%pil) */
	      + (0x32 << 19)	/* op3 (wrpr) */
	      + (0 << 14)	/* rs1 (%g0) */
	      + (1 << 13)	/* i */
	      + 0)) {		/* simm13 */

	/* the netbsd64-type-0 idle type detects these two instructions
	   in disassembly order:

	   idle():
	     [...]
	     wrpr %g0, PSTATE_INTR, %pstate
	     wrpr %g0, 0, %pil

	   this detects locore.s:idle() in NetBSD/sparc64 1.6
	   through 4.99.19 (until revision 1.244 of locore.s on
	   20070517): */
	if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0)) {
	  if ((tme_sparc_fetch_nearby(ic, -1)
	       & ~ (tme_uint32_t) TME_SPARC64_PSTATE_AM)
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (6 << 25)		/* rd (%pstate) */
		  + (0x32 << 19)	/* op3 (wrpr) */
		  + (0 << 14)		/* rs1 (%g0) */
		  + (1 << 13)		/* i */
		  + (TME_SPARC64_PSTATE_PRIV
		     + TME_SPARC64_PSTATE_IE))) { /* PSTATE_INTR */

	    /* we have detected the idle PC: */
	    ic->tme_sparc_idle_pcs_64[0]
	      = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC);
	    tme_sparc_recode_invalidate_all(ic);
	  }
	}
      }

      /* if this is a "wrpr 0xa, %pil" instruction: */
      if (TME_SPARC_INSN
	  == ((tme_uint32_t)
	      (2 << 30)		/* format */
	      + (8 << 25)	/* rd (%pil) */
	      + (0x32 << 19)	/* op3 (wrpr) */
	      + (0 << 14)	/* rs1 (%g0) */
	      + (1 << 13)	/* i */
	      + 0xa)) {		/* simm13 */

	/* the sunos64-type-0 idle type begins by detecting these
	   instructions in disassembly order:

	   splhigh():
	     rdpr %pil, %o1
	     [any]
	     [any]
	     [any]
	     wrpr 0xa, %pil

	   this detects sparcv9_subr.s:splhigh() in Solaris 8 and
	   probably others: */
	if (TME_SPARC_IDLE_TYPE_IS(ic, TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0)) {
	  if ((tme_sparc_fetch_nearby(ic, -4)
	       & (0xffffffff
		  - 0x3fff))		/* bits 0..13 reserved */
	      == ((tme_uint32_t)
		  (2 << 30)		/* format */
		  + (0x9 << 25)		/* rd (%o1) */
		  + (0x2a << 19)	/* op3 (rdpr) */
		  + (8 << 14))) {	/* rs1 (pil) */

	    /* try to detect the idle loop: */
	    _tme_sparc64_idle_type_sunos64_type_0_splhigh(ic);
	  }
	}
      }
    }

    /* otherwise, if this is the idle PC, and the idle type marks the
       idle at this wrpr to %pil: */
    else if (__tme_predict_false(ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
				 == ic->tme_sparc_idle_pcs_64[0])) {
      if (TME_SPARC_IDLE_TYPE_IS(ic,
				 (TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0
				  ))) {

	/* mark the idle: */
	TME_SPARC_IDLE_MARK(ic);
      }
    }

    break;

  case 9:
    /* XXX FIXME - the V9 architecture manual says the effect of
       writing a CWP greater than or equal to NWINDOWS is
       implementation-dependent.  we assume that all of our
       implementations will always write it mod NWINDOWS: */       
    cwp = value_xor;
    cwp %= ic->tme_sparc_nwindows;
    ic->tme_sparc64_ireg_cwp = cwp;
    TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);
    break;
  case 10: ic->tme_sparc64_ireg_cansave = value_xor; break;
  case 11: ic->tme_sparc64_ireg_canrestore = value_xor; break;
  case 12: ic->tme_sparc64_ireg_cleanwin = value_xor; break;
  case 13: ic->tme_sparc64_ireg_otherwin = value_xor; break;
  case 14: ic->tme_sparc64_ireg_wstate = value_xor; break;

    /* XXX FIXME - add support for implementations with an FQ: */
#if 0
    /* FQ: */
  case 15:
    break;
#endif /* 0 */
  }

  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_rdasr, tme_uint64_t)
{
  unsigned int reg_rs1;
  tme_uint64_t value;

  /* dispatch on rs1: */
  reg_rs1 = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RS1);
  switch (reg_rs1) {

  case 0: value = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y << 1); break;
  case 2: value = ic->tme_sparc64_ireg_ccr; break;
  case 3: value = ic->tme_sparc64_ireg_asi; break;
  case 4:
    tme_sparc_recode_verify_reg_tick(ic, &TME_SPARC_FORMAT3_RD);
    if (tme_sparc_recode_verify_replay_last_pc(ic) != 0) {
      TME_SPARC_INSN_OK;
    }
    value = tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint;
    value += ic->tme_sparc64_ireg_tick_offset;
    value &= TME_SPARC64_TICK_COUNTER;
    if (__tme_predict_false(ic->tme_sparc64_ireg_tick_npt)) {
      if (__tme_predict_false(!TME_SPARC_PRIV(ic))) {
	tme_sparc64_trap(ic, TME_SPARC64_TRAP_privileged_action);
      }
      value |= TME_SPARC64_TICK_NPT;
    }
    tme_sparc_recode_verify_reg_tick_now(ic, &value);
    break;
  case 5: value = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC); break;
  case 6: value = ic->tme_sparc64_ireg_fprs; break;

  case 15:
    if (__tme_predict_true(TME_SPARC_INSN & TME_SPARC_FORMAT3_MASK_RD) == 0) {
      /* XXX FIXME - we need to implement barriers: */
      TME_SPARC_INSN_OK;
    }

    /* FALLTHROUGH */
  default:
    TME_SPARC_INSN_ILL(ic);
    TME_SPARC_INSN_OK;
  }

  TME_SPARC_FORMAT3_RD = value;
  TME_SPARC_INSN_OK;
}

TME_SPARC_FORMAT3(tme_sparc64_wrasr, tme_uint64_t)
{
  tme_uint64_t value_xor;
  unsigned int reg_rd;

  /* get the value to write: */
  value_xor = TME_SPARC_FORMAT3_RS1 ^ TME_SPARC_FORMAT3_RS2;

  /* dispatch on rd: */
  reg_rd = TME_FIELD_MASK_EXTRACTU(TME_SPARC_INSN, TME_SPARC_FORMAT3_MASK_RD);
  switch (reg_rd) {

  case 0: ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_Y << 1) = value_xor; TME_SPARC_INSN_OK;
  case 2: ic->tme_sparc64_ireg_ccr = value_xor; TME_SPARC_INSN_OK;

    /* a write to the ASI register causes a redispatch because we may
       need to update cached state derived from it: */
  case 3: 
    ic->tme_sparc64_ireg_asi = value_xor;
    tme_sparc_redispatch(ic);
    TME_SPARC_INSN_OK;

  case 6: ic->tme_sparc64_ireg_fprs = value_xor; TME_SPARC_INSN_OK;

    /* a valid sir must be handled in the CPU-specific wrasr function: */
  default:
    assert (reg_rd != 15
	    || ((TME_SPARC_INSN
		 & (TME_SPARC_FORMAT3_MASK_RS1
		    | TME_BIT(13)))
		!= TME_BIT(13)));
    TME_SPARC_INSN_ILL(ic);
    TME_SPARC_INSN_OK;
  }
}

TME_SPARC_FORMAT3(tme_sparc64_done_retry, tme_uint64_t)
{
  tme_uint32_t fcn;
  unsigned long tl;
  tme_uint64_t pc_next;
  tme_uint64_t pc_next_next;
  tme_uint32_t tstate_0_31;
  tme_uint32_t cwp;
  unsigned int cwp_offset;
  tme_uint32_t pstate;

  /* get the (still-shifted) fcn, and TL: */
  fcn = TME_SPARC_INSN & (0x1f << 25);
  tl = ic->tme_sparc64_ireg_tl;

  /* fcn and TL must be legal: */
  if (__tme_predict_false(fcn > (1 << 25)
			  || tl == 0)) {
    TME_SPARC_INSN_ILL(ic);
  }

  TME_SPARC_INSN_PRIV;

  /* assume that this is a done: */
  pc_next = ic->tme_sparc64_ireg_tnpc(tl);
  pc_next_next = pc_next + sizeof(tme_uint32_t);

  /* if this is a retry: */
  if (fcn == (1 << 25)) {
    pc_next_next = pc_next;
    pc_next = ic->tme_sparc64_ireg_tpc(tl);
  }

  /* update the PCs: */
  pc_next &= ic->tme_sparc_address_mask;
  pc_next_next &= ic->tme_sparc_address_mask;
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT) = pc_next;
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = pc_next_next;

  /* update CCR directly from TSTATE: */
  ic->tme_sparc64_ireg_ccr = ic->tme_sparc64_ireg_tstate_ccr(tl);

  /* get the least-significant 32 bits of TSTATE: */
  tstate_0_31 = ic->tme_sparc64_ireg_tstate(tl);

  /* log the done or retry: */
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("%s tl %u next-%%pc 0x%016" TME_PRIx64 " tstate_0_31 0x%08" TME_PRIx32),
		 (fcn == (1 << 25)
		  ? "retry"
		  : "done"),
		 (unsigned int) tl,
		 pc_next,
		 tstate_0_31));

  /* update TL: */
  ic->tme_sparc64_ireg_tl = tl - 1;

  /* update CWP: */
#if (TME_SPARC64_TSTATE_MASK_CWP & 1) == 0
#error "TME_SPARC64_TSTATE_MASK_CWP changed"
#endif
  cwp = (tme_uint8_t) tstate_0_31;
  assert (ic->tme_sparc64_ireg_winstates_mask != 0);
  cwp &= ic->tme_sparc64_ireg_winstates_mask;
  ic->tme_sparc64_ireg_cwp = cwp;
  TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);

  /* get the value for PSTATE: */
  tstate_0_31
    /= (_TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE)
	/ _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_CWP));
  pstate
    = (tstate_0_31
       & (TME_SPARC64_TSTATE_MASK_PSTATE
	  / _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE)));

  /* update ASI: */
  tstate_0_31
    /= (_TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_ASI)
	/ _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE));
  ic->tme_sparc64_ireg_asi
    = (tstate_0_31
       & (TME_SPARC64_TSTATE_MASK_ASI
	  / _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_ASI)));

  /* update PSTATE: */
  (*ic->_tme_sparc64_update_pstate)(ic, pstate, TME_SPARC_TRAP_none);

  /* redispatch, since the executor may have cached information
     derived from PSTATE, ASI, etc., that we need to invalidate: */
  tme_sparc_redispatch(ic);
}

TME_SPARC_FORMAT3(tme_sparc64_illegal_instruction, tme_uint64_t)
{
  TME_SPARC_INSN_ILL(ic);
}

#endif /* TME_HAVE_INT64_T */

#undef TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)

#include "sparc-insns-auto.c"
