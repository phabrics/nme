/* $Id: sparc-rc-cc.c,v 1.2 2009/08/28 01:41:45 fredette Exp $ */

/* ic/sparc/sparc-rc-cc.c - SPARC recode condition code support: */

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

_TME_RCSID("$Id: sparc-rc-cc.c,v 1.2 2009/08/28 01:41:45 fredette Exp $");

/* macros: */

/* this converts sparc64 icc flags into the current architecture's icc
   flags: */
#define TME_SPARC_ICC(ccr_icc)					\
  ((ccr_icc)							\
   * (((TME_SPARC_VERSION(ic) < 9)				\
       * (TME_SPARC32_PSR_ICC					\
	  / TME_SPARC64_CCR_ICC))				\
      + (TME_SPARC_VERSION(ic) >= 9)))

#define TME_SPARC_RECODE_CCR_NEED(insn_class, insn_size, size)	\
  ((((insn_class == TME_RECODE_INSN_CLASS_ADDITIVE)		\
     * ((TME_SPARC64_CCR_ICC_C					\
	 * (TME_RECODE_FLAG_NEED(insn_class, insn_size,		\
				 TME_RECODE_COND_C, size)	\
	    != 0))						\
	+ (TME_SPARC64_CCR_ICC_V				\
	   * (TME_RECODE_FLAG_NEED(insn_class, insn_size,	\
				   TME_RECODE_COND_V, size)	\
	      != 0))))						\
    + (TME_SPARC64_CCR_ICC_Z					\
       * (TME_RECODE_FLAG_NEED(insn_class, insn_size,		\
			       TME_RECODE_COND_Z, size)		\
	  != 0))						\
    + (TME_SPARC64_CCR_ICC_N					\
       * (TME_RECODE_FLAG_NEED(insn_class, insn_size,		\
			       TME_RECODE_COND_N, size)		\
	  != 0)))						\
   * ((TME_SPARC64_CCR_XCC					\
       / TME_SPARC64_CCR_ICC)					\
      * (size > TME_RECODE_SIZE_32)))

/* this macro gives the %icc and %xcc N and Z flags: */
#define TME_SPARC_RECODE_CCR_N_Z(dst, ccr_need)			\
  (0								\
   + ((((tme_uint32_t) (dst)) == 0)				\
      * ((ccr_need) & TME_SPARC64_CCR_ICC_Z))			\
   + ((((tme_int32_t) (dst)) < 0)				\
      * ((ccr_need) & TME_SPARC64_CCR_ICC_N))			\
   + ((((tme_sparc_ireg_t) (dst)) == 0)				\
      * ((ccr_need) & TME_SPARC64_CCR_XCC_Z))			\
   + ((((dst) & TME_SPARC_IREG_MSBIT) != 0)			\
      * ((ccr_need) & TME_SPARC64_CCR_XCC_N)))

/* this macro updates %icc and %xcc: */
#define TME_SPARC_RECODE_CCR_UPDATE(ic, ccr, ccr_need)		\
  do {								\
    if (TME_SPARC_VERSION((struct tme_sparc *) (ic)) >= 9) {	\
      ((struct tme_sparc *) (ic))->tme_sparc64_ireg_ccr		\
	= ((((struct tme_sparc *) (ic))->tme_sparc64_ireg_ccr	\
	    & ~ccr_need)					\
	   | ccr);						\
    }								\
    else {							\
      ((struct tme_sparc *) (ic))->tme_sparc32_ireg_psr		\
	= ((((struct tme_sparc *) (ic))->tme_sparc32_ireg_psr	\
	    & ~TME_SPARC_ICC(ccr_need))				\
	   | TME_SPARC_ICC(ccr_need));				\
    }								\
  } while (/* CONSTCOND */ 0)

/* rename various things by the architecture size: */
#define _tme_sparc_flags_additive _TME_SPARC_RECODE_SIZE(_tme_sparc,_flags_additive)
#define _tme_sparc_flags_logical _TME_SPARC_RECODE_SIZE(_tme_sparc,_flags_logical)
#define _tme_sparc_flags_func_logical _TME_SPARC_RECODE_SIZE(_tme_sparc,_flags_func_logical)
#define _tme_sparc_flags_func_add _TME_SPARC_RECODE_SIZE(_tme_sparc,_flags_func_add)
#define _tme_sparc_flags_func_sub _TME_SPARC_RECODE_SIZE(_tme_sparc,_flags_func_sub)
#define _tme_sparc_recode_conds_func_icc _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_conds_func_icc)
#define _tme_sparc_recode_cc_init _TME_SPARC_RECODE_SIZE(_tme_sparc,_recode_cc_init)

/* the sparc64 recode xcc conditions function: */
static int
_tme_sparc64_recode_conds_func_xcc(tme_recode_uguest_t flags, tme_uint32_t cond)
{
  return (_tme_sparc_conds_icc
	  [TME_FIELD_MASK_EXTRACTU(flags, TME_SPARC64_CCR_XCC)]
	  & (1 << cond));
}

#endif /* TME_SPARC_RECODE_SIZE(ic) == TME_RECODE_SIZE_32 */

/* the additive instruction flags: */
static const struct tme_recode_flag _tme_sparc_flags_additive[] = {
#if TME_SPARC_VERSION(ic) >= 9
  {
    TME_RECODE_COND_N,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_N
  },
  {
    TME_RECODE_COND_Z,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_Z
  },
  {
    TME_RECODE_COND_V,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_V
  },
  {
    TME_RECODE_COND_C,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_C
  },
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  { 
    TME_RECODE_COND_N,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_N)
  },
  { 
    TME_RECODE_COND_Z,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_Z)
  },
  { 
    TME_RECODE_COND_V,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_V)
  },
  { 
    TME_RECODE_COND_C,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_C)
  },
  {
    0,
    0,
    0
  }
};

/* the logical instruction flags: */
static const struct tme_recode_flag _tme_sparc_flags_logical[] = {
#if TME_SPARC_VERSION(ic) >= 9
  {
    TME_RECODE_COND_N,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_N
  },
  {
    TME_RECODE_COND_Z,
    TME_RECODE_SIZE_64,
    TME_SPARC64_CCR_XCC_Z
  },
  {
    TME_RECODE_COND_FALSE,
    0,
    TME_SPARC64_CCR_XCC_V
  },
  {
    TME_RECODE_COND_FALSE,
    0,
    TME_SPARC64_CCR_XCC_C
  },
#endif /* TME_SPARC_VERSION(ic) >= 9 */
  { 
    TME_RECODE_COND_N,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_N)
  },
  { 
    TME_RECODE_COND_Z,
    TME_RECODE_SIZE_32,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_Z)
  },
  { 
    TME_RECODE_COND_FALSE,
    0,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_V)
  },
  { 
    TME_RECODE_COND_FALSE,
    0,
    TME_SPARC_ICC(TME_SPARC64_CCR_ICC_C)
  },
  {
    0,
    0,
    0
  }
};

/* the logical instruction flags function: */
static void
_tme_sparc_flags_func_logical(struct tme_ic *ic,
			      tme_recode_uguest_t src1,
			      tme_recode_uguest_t src2,
			      tme_recode_uguest_t dst)
{
  unsigned int ccr_need;

  ccr_need
    = (TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_LOGICAL,
				 TME_SPARC_RECODE_SIZE(ic),
				 TME_RECODE_SIZE_32)
       | TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_LOGICAL,
				   TME_SPARC_RECODE_SIZE(ic),
				   TME_SPARC_RECODE_SIZE(ic)));

  TME_SPARC_RECODE_CCR_UPDATE(ic,
			      TME_SPARC_RECODE_CCR_N_Z(dst, ccr_need),
			      ccr_need);
}

/* the add instruction flags function: */
static void
_tme_sparc_flags_func_add(struct tme_ic *ic,
			  tme_recode_uguest_t src1,
			  tme_recode_uguest_t src2,
			  tme_recode_uguest_t dst)
{
  unsigned int ccr_need;
  unsigned int ccr;

  ccr_need
    = (TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_ADDITIVE,
				 TME_SPARC_RECODE_SIZE(ic),
				 TME_RECODE_SIZE_32)
       | TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_ADDITIVE,
				   TME_SPARC_RECODE_SIZE(ic),
				   TME_SPARC_RECODE_SIZE(ic)));

  /* make the %icc and %xcc N and Z flags: */
  ccr = TME_SPARC_RECODE_CCR_N_Z(dst, ccr_need);

  /* if src1 and src2 both have bit 31 set, or if dst does not have
     bit 31 set and either src1 or src2 does, set %icc.C: */
  if (((tme_int32_t) ((src1 & src2) | (~dst & (src1 | src2)))) < 0) {
    ccr += (ccr_need & TME_SPARC64_CCR_ICC_C);
  }

  /* if src1 and src2 have the same bit 31, and dst has a different
     bit 31, set %icc.V: */
  if (((tme_int32_t) ((src2 ^ dst) & (src1 ^ ~src2))) < 0) {
    ccr += (ccr_need & TME_SPARC64_CCR_ICC_V);
  }

  /* if src1 and src2 both have the most-significant bit set, or if
     dst does not have the most-significant bit set either src1 or
     src2 does, set %xcc.C: */
  if (((src1 & src2) | (~dst & (src1 | src2))) & TME_SPARC_IREG_MSBIT) {
    ccr += (ccr_need & TME_SPARC64_CCR_XCC_C);
  }

  /* if src1 and src2 have the same most-significant bit, and dst has
     a different most-significant bit, set %xcc.V: */
  if (((src2 ^ dst) & (src1 ^ ~src2)) & TME_SPARC_IREG_MSBIT) {
    ccr += (ccr_need & TME_SPARC64_CCR_XCC_V);
  }

  TME_SPARC_RECODE_CCR_UPDATE(ic, ccr, ccr_need);
}

/* the sub instruction flags function: */
static void
_tme_sparc_flags_func_sub(struct tme_ic *ic,
			  tme_recode_uguest_t src1,
			  tme_recode_uguest_t src2,
			  tme_recode_uguest_t dst)
{
  unsigned int ccr_need;
  unsigned int ccr;

  ccr_need
    = (TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_ADDITIVE,
				 TME_SPARC_RECODE_SIZE(ic),
				 TME_RECODE_SIZE_32)
       | TME_SPARC_RECODE_CCR_NEED(TME_RECODE_INSN_CLASS_ADDITIVE,
				   TME_SPARC_RECODE_SIZE(ic),
				   TME_SPARC_RECODE_SIZE(ic)));

  /* make the %icc and %xcc N and Z flags: */
  ccr = TME_SPARC_RECODE_CCR_N_Z(dst, ccr_need);

  /* if src2 is greater than src1 in 32 bits, set %icc.C: */
  if (((tme_uint32_t) src2) > ((tme_uint32_t) src1)) {
    ccr += (ccr_need & TME_SPARC64_CCR_ICC_C);
  }

  /* if src1 and src2 have different bit 31s, and dst has a different
     bit 31 than src1, set %icc.V: */
  if (((tme_int32_t) ((src1 ^ src2) & (src1 ^ dst))) < 0) {
    ccr += (ccr_need & TME_SPARC64_CCR_ICC_V);
  }

  /* if src2 is greater than src1, set %xcc.C: */
  if (((tme_sparc_ireg_t) src2) > ((tme_sparc_ireg_t) src1)) {
    ccr += (ccr_need & TME_SPARC64_CCR_XCC_C);
  }

  /* if src1 and src2 have different most-significant bits, and dst
     has a different most-significant bit than src1, set %xcc.V: */
  if (((src1 ^ src2) & (src1 ^ dst)) & TME_SPARC_IREG_MSBIT) {
    ccr += (ccr_need & TME_SPARC64_CCR_XCC_V);
  }

  TME_SPARC_RECODE_CCR_UPDATE(ic, ccr, ccr_need);
}

/* the sparc icc conditions function: */
static int
_tme_sparc_recode_conds_func_icc(tme_recode_uguest_t flags, tme_uint32_t cond)
{
  return (_tme_sparc_conds_icc
	  [TME_FIELD_MASK_EXTRACTU(flags, TME_SPARC_ICC(TME_SPARC64_CCR_ICC))]
	  & (1 << cond));
}

/* this initializes for condition codes: */
static void
_tme_sparc_recode_cc_init(struct tme_sparc *ic)
{
  struct tme_recode_ic *recode_ic;
  struct tme_recode_flags_group flags_group;
  struct tme_recode_conds_group conds_group;

  /* recover the recode ic: */
  recode_ic = ic->tme_sparc_recode_ic;

  /* make the common parts of a flags group: */
  memset((char *) &flags_group, 0, sizeof(flags_group));
  flags_group.tme_recode_flags_group_insn_size = TME_SPARC_RECODE_SIZE(ic);
  if (TME_SPARC_VERSION(ic) <= 8) {
    flags_group.tme_recode_flags_group_flags_reg_size = TME_RECODE_SIZE_32;
    flags_group.tme_recode_flags_group_flags_reg = TME_SPARC32_IREG_PSR;
  }
  else {
    flags_group.tme_recode_flags_group_flags_reg_size = TME_RECODE_SIZE_8;
    flags_group.tme_recode_flags_group_flags_reg = TME_SPARC64_IREG_CCR << 3;
  }

  /* make the flags thunk for addcc and addxcc: */
  flags_group.tme_recode_flags_group_insn_class = TME_RECODE_INSN_CLASS_ADDITIVE;
  flags_group.tme_recode_flags_group_flags = _tme_sparc_flags_additive;
  flags_group.tme_recode_flags_group_guest_func = _tme_sparc_flags_func_add;
  ic->tme_sparc_recode_flags_thunk_add
    = tme_recode_flags_thunk(recode_ic,
			     &flags_group);

  /* make the flags thunk for subcc and subxcc: */
  flags_group.tme_recode_flags_group_insn_class = TME_RECODE_INSN_CLASS_ADDITIVE;
  flags_group.tme_recode_flags_group_flags = _tme_sparc_flags_additive;
  flags_group.tme_recode_flags_group_guest_func = _tme_sparc_flags_func_sub;
  ic->tme_sparc_recode_flags_thunk_sub
    = tme_recode_flags_thunk(recode_ic,
			     &flags_group);

  /* make the flags thunk for the logical instructions: */
  flags_group.tme_recode_flags_group_insn_class = TME_RECODE_INSN_CLASS_LOGICAL;
  flags_group.tme_recode_flags_group_flags = _tme_sparc_flags_logical;
  flags_group.tme_recode_flags_group_guest_func = _tme_sparc_flags_func_logical;
  ic->tme_sparc_recode_flags_thunk_logical
    = tme_recode_flags_thunk(recode_ic,
			     &flags_group);

  /* make the common parts of a conditions group: */
  memset((char *) &conds_group, 0, sizeof(conds_group));
  conds_group.tme_recode_conds_group_flags_reg_size = flags_group.tme_recode_flags_group_flags_reg_size;
  conds_group.tme_recode_conds_group_flags_reg = flags_group.tme_recode_flags_group_flags_reg;
  conds_group.tme_recode_conds_group_cond_count = TME_SPARC_COND_NOT;

  /* make the conditions thunk for %icc: */
  conds_group.tme_recode_conds_group_flags = TME_SPARC_ICC(TME_SPARC64_CCR_ICC);
  conds_group.tme_recode_conds_group_guest_func = _tme_sparc_recode_conds_func_icc;
  ic->tme_sparc_recode_conds_thunk_icc
    = tme_recode_conds_thunk(recode_ic,
			     &conds_group);

  /* if this is a sparc64: */
  if (TME_SPARC_VERSION(ic) >= 9) {

    /* make the flags thunk for the sparc64 internal "rcc" flags.
       this is identical to the flags thunk for the logical
       instructions, except it uses the internal "rcc" register: */
    flags_group.tme_recode_flags_group_flags_reg = TME_SPARC64_IREG_RCC << 3;
    ic->tme_sparc_recode_flags_thunk_rcc
      = tme_recode_flags_thunk(recode_ic,
			       &flags_group);

    /* make the conditions thunk for %xcc: */
    conds_group.tme_recode_conds_group_flags = TME_SPARC64_CCR_XCC;
    conds_group.tme_recode_conds_group_guest_func = _tme_sparc64_recode_conds_func_xcc;
    ic->tme_sparc_recode_conds_thunk_xcc
      = tme_recode_conds_thunk(recode_ic,
			       &conds_group);

    /* make the conditions thunk for the internal "rcc" flags.  this
       is identical to the conditions thunk for %xcc, except it uses
       the internal "rcc" register: */
    conds_group.tme_recode_conds_group_flags_reg = flags_group.tme_recode_flags_group_flags_reg;
    ic->tme_sparc_recode_conds_thunk_rcc
      = tme_recode_conds_thunk(recode_ic,
			       &conds_group);
  }
}
