/* $Id: mb86900.c,v 1.4 2010/02/14 14:00:58 fredette Exp $ */

/* ic/m68k/mb86900.c - implementation of Fujitsu SPARC MB86900 emulation: */

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

#include <tme/common.h>
_TME_RCSID("$Id: mb86900.c,v 1.4 2010/02/14 14:00:58 fredette Exp $");

/* includes: */
#include "sparc-impl.h"

/* the format three opcode map: */
#define _TME_SPARC_EXECUTE_OPMAP tme_sparc_opcodes_mb86900
const _tme_sparc32_format3 _TME_SPARC_EXECUTE_OPMAP[] = {

  /* op=2: arithmetic, logical, shift, and remaining: */

  /* 000000 */ tme_sparc32_add,
  /* 000001 */ tme_sparc32_and,
  /* 000010 */ tme_sparc32_or,
  /* 000011 */ tme_sparc32_xor,
  /* 000100 */ tme_sparc32_sub,
  /* 000101 */ tme_sparc32_andn,
  /* 000110 */ tme_sparc32_orn,
  /* 000111 */ tme_sparc32_xnor,
  /* 001000 */ tme_sparc32_addx,
  /* 001001 */ NULL,
  /* 001010 */ tme_sparc32_illegal, /* the MB86900 does not implement umul */
  /* 001011 */ tme_sparc32_illegal, /* the MB86900 does not implement smul */
  /* 001100 */ tme_sparc32_subx,
  /* 001101 */ NULL,
  /* 001110 */ tme_sparc32_illegal, /* the MB86900 does not implement udiv */
  /* 001111 */ tme_sparc32_illegal, /* the MB86900 does not implement sdiv */
  /* 010000 */ tme_sparc32_addcc,
  /* 010001 */ tme_sparc32_andcc,
  /* 010010 */ tme_sparc32_orcc,
  /* 010011 */ tme_sparc32_xorcc,
  /* 010100 */ tme_sparc32_subcc,
  /* 010101 */ tme_sparc32_andncc,
  /* 010110 */ tme_sparc32_orncc,
  /* 010111 */ tme_sparc32_xnorcc,
  /* 011000 */ tme_sparc32_addxcc,
  /* 011001 */ NULL,
  /* 011010 */ tme_sparc32_illegal, /* the MB86900 does not implement umulcc */
  /* 011011 */ tme_sparc32_illegal, /* the MB86900 does not implement smulcc */
  /* 011100 */ tme_sparc32_subxcc,
  /* 011101 */ NULL,
  /* 011110 */ tme_sparc32_illegal, /* the MB86900 does not implement udivcc */
  /* 011111 */ tme_sparc32_illegal, /* the MB86900 does not implement sdivcc */
  /* 100000 */ tme_sparc32_taddcc,
  /* 100001 */ tme_sparc32_tsubcc,
  /* 100010 */ tme_sparc32_taddcctv,
  /* 100011 */ tme_sparc32_tsubcctv,
  /* 100100 */ tme_sparc32_mulscc,
  /* 100101 */ tme_sparc32_sll,
  /* 100110 */ tme_sparc32_srl,
  /* 100111 */ tme_sparc32_sra,
  /* 101000 */ tme_sparc32_rdasr,
  /* 101001 */ tme_sparc32_rdpsr,
  /* 101010 */ tme_sparc32_rdwim,
  /* 101011 */ tme_sparc32_rdtbr,
  /* 101100 */ NULL,
  /* 101101 */ NULL,
  /* 101110 */ NULL,
  /* 101111 */ NULL,
  /* 110000 */ tme_sparc32_wrasr,
  /* 110001 */ tme_sparc32_wrpsr,
  /* 110010 */ tme_sparc32_wrwim,
  /* 110011 */ tme_sparc32_wrtbr,
  /* 110100 */ tme_sparc32_fpop1,
  /* 110101 */ tme_sparc32_fpop2,
  /* 110110 */ tme_sparc32_cpop1,
  /* 110111 */ tme_sparc32_cpop2,
  /* 111000 */ tme_sparc32_jmpl,
  /* 111001 */ tme_sparc32_rett,
  /* 111010 */ tme_sparc32_ticc,
  /* 111011 */ tme_sparc32_flush,
  /* 111100 */ tme_sparc32_save_restore,
  /* 111101 */ tme_sparc32_save_restore,
  /* 111110 */ NULL,
  /* 111111 */ NULL,

  /* op=3: memory instructions: */

  /* 000000 */ tme_sparc32_ld,
  /* 000001 */ tme_sparc32_ldb,
  /* 000010 */ tme_sparc32_ldh,
  /* 000011 */ tme_sparc32_ldd,
  /* 000100 */ tme_sparc32_st,
  /* 000101 */ tme_sparc32_stb,
  /* 000110 */ tme_sparc32_sth,
  /* 000111 */ tme_sparc32_std,
  /* 001000 */ NULL,
  /* 001001 */ tme_sparc32_ldb,
  /* 001010 */ tme_sparc32_ldh,
  /* 001011 */ NULL,
  /* 001100 */ NULL,
  /* 001101 */ tme_sparc32_ldstub,
  /* 001110 */ NULL,
  /* 001111 */ tme_sparc32_illegal, /* the MB86900 does not implement swap */
  /* 010000 */ tme_sparc32_lda,
  /* 010001 */ tme_sparc32_ldba,
  /* 010010 */ tme_sparc32_ldha,
  /* 010011 */ tme_sparc32_ldda,
  /* 010100 */ tme_sparc32_sta,
  /* 010101 */ tme_sparc32_stba,
  /* 010110 */ tme_sparc32_stha,
  /* 010111 */ tme_sparc32_stda,
  /* 011000 */ NULL,
  /* 011001 */ tme_sparc32_ldba,
  /* 011010 */ tme_sparc32_ldha,
  /* 011011 */ NULL,
  /* 011100 */ NULL,
  /* 011101 */ tme_sparc32_ldstuba,
  /* 011110 */ NULL,
  /* 011111 */ tme_sparc32_illegal, /* the MB86900 does not implement swapa */
  /* 100000 */ tme_sparc32_ldf,
  /* 100001 */ tme_sparc32_ldfsr,
  /* 100010 */ NULL,
  /* 100011 */ tme_sparc32_lddf,
  /* 100100 */ tme_sparc32_stf,
  /* 100101 */ tme_sparc32_stfsr,
  /* 100110 */ tme_sparc32_stdfq,
  /* 100111 */ tme_sparc32_stdf,
  /* 101000 */ NULL,
  /* 101001 */ NULL,
  /* 101010 */ NULL,
  /* 101011 */ NULL,
  /* 101100 */ NULL,
  /* 101101 */ NULL,
  /* 101110 */ NULL,
  /* 101111 */ NULL,
  /* 110000 */ tme_sparc32_ldc,
  /* 110001 */ tme_sparc32_ldcsr,
  /* 110010 */ NULL,
  /* 110011 */ tme_sparc32_lddc,
  /* 110100 */ tme_sparc32_stc,
  /* 110101 */ tme_sparc32_stcsr,
  /* 110110 */ tme_sparc32_stdcq,
  /* 110111 */ tme_sparc32_stdc,
  /* 111000 */ NULL,
  /* 111001 */ NULL,
  /* 111010 */ NULL,
  /* 111011 */ NULL,
  /* 111100 */ NULL,
  /* 111101 */ NULL,
  /* 111110 */ NULL,
  /* 111111 */ NULL,
};

/* make the executor for the MB86900: */
#undef  TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic)	(7)
#undef  TME_SPARC_NWINDOWS
#define TME_SPARC_NWINDOWS(ic)	(7)
#define _TME_SPARC_EXECUTE_NAME _tme_sparc_execute_mb86900
#include "sparc-execute.c"

/* this returns the version of an FPU for an mb86900: */
static tme_uint32_t
_tme_sparc_fpu_ver_mb86900(struct tme_sparc *ic, const char *fpu_name, char **_output)
{
  tme_uint32_t ver;

  /* if we're returning a usage: */
  if (_output != NULL) {
    tme_output_append_error(_output, 
			    "{ mb86910/wtl116x | mb86911/wtl116x | l64802/act8847 | wtl317x | l64804 }");
    return (TME_SPARC_FSR_VER_missing);
  }

  if (TME_ARG_IS(fpu_name, "mb86910/wtl116x")) {
    ver = 0;
  }
  else if (TME_ARG_IS(fpu_name, "mb86911/wtl116x")) {
    ver = 1;
  }
  else if (TME_ARG_IS(fpu_name, "l64802/act8847")) {
    ver = 2;
  }
  else if (TME_ARG_IS(fpu_name, "wtl317x")) {
    ver = 3;
  }
  else if (TME_ARG_IS(fpu_name, "l64804")) {
    ver = 4;
  }
  else {
    return (TME_SPARC_FSR_VER_missing);
  }

  ic->tme_sparc_fpu_flags
    = (TME_SPARC_FPU_FLAG_NO_QUAD
       | TME_SPARC_FPU_FLAG_NO_FSQRT
       | TME_SPARC_FPU_FLAG_NO_FMUL_WIDER);
  return (ver * _TME_FIELD_MASK_FACTOR(TME_SPARC_FSR_VER));
}

/* this creates and returns a new mb86900: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,sparc,mb86900) {
  struct tme_sparc *ic;
  tme_uint32_t psr;

  /* allocate the sparc structure: */
  ic = tme_new0(struct tme_sparc, 1);
  ic->tme_sparc_element = element;

  /* initialize the synchronization parts of the structure: */
  tme_sparc_sync_init(ic);

  /* fill in the mb86900-specific parts of the structure: */
  psr = 0;
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_IMPL, 0);
  TME_FIELD_MASK_DEPOSITU(psr, TME_SPARC32_PSR_VER, 0);
  ic->tme_sparc32_ireg_psr = psr;
  ic->tme_sparc_version = TME_SPARC_VERSION(ic);
  ic->tme_sparc_nwindows = TME_SPARC_NWINDOWS(ic);
  ic->_tme_sparc32_execute_opmap = _TME_SPARC_EXECUTE_OPMAP;
  ic->_tme_sparc_execute = _tme_sparc_execute_mb86900;
  ic->_tme_sparc_fpu_ver = _tme_sparc_fpu_ver_mb86900;
  ic->_tme_sparc_external_check = tme_sparc32_external_check;
  ic->_tme_sparc_ls_address_map = tme_sparc32_ls_address_map;
  ic->_tme_sparc_ls_bus_cycle = tme_sparc32_ls_bus_cycle;
  ic->_tme_sparc_ls_bus_fault = tme_sparc_ls_bus_fault;
  ic->_tme_sparc_ls_trap = tme_sparc32_ls_trap;
  ic->tme_sparc_timing_loop_cycles_each = (1 + 1);
#ifdef _TME_SPARC_RECODE_VERIFY
  ic->tme_sparc_recode_verify_ic_size = sizeof(struct tme_sparc);
  ic->tme_sparc_recode_verify_ic_size_total = sizeof(struct tme_sparc);
#endif /* _TME_SPARC_RECODE_VERIFY */

  /* call the common sparc new function: */
  return (tme_sparc_new(ic, args, extra, _output));
}

#undef  TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) _TME_SPARC_VERSION(ic)
#undef  TME_SPARC_NWINDOWS
#define TME_SPARC_NWINDOWS(ic) _TME_SPARC_NWINDOWS(ic)
#undef _TME_SPARC_EXECUTE_NAME
#undef _TME_SPARC_EXECUTE_OPMAP
