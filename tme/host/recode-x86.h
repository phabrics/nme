/* $Id: recode-x86.h,v 1.5 2010/02/07 17:16:56 fredette Exp $ */

/* libtme/host/recode-x86.h - recode header for x86 hosts: */

/*
 * Copyright (c) 2007 Matt Fredette
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

_TME_RCSID("$Id: recode-x86.h,v 1.5 2010/02/07 17:16:56 fredette Exp $");

/* macros: */

/* TME_RECODE_SIZE_HOST is the log base two of the natural size of the
   host in bits: */
#ifdef __x86_64__
#define TME_RECODE_SIZE_HOST	(6)
#else  /* !__x86_64__ */
#define TME_RECODE_SIZE_HOST	(5)
#endif /* !__x86_64__ */

/* TME_RECODE_FLAG_NEED() evaluates to nonzero if the host can't
   provide a condition at a certain size, after a recode instruction
   of the given class and size.  in this case, the guest
   implementation needs to compute it.

   for additive, logical, shift, and extension instructions, x86
   provides N and Z codes at all sizes starting at 8 bits, and an even
   parity code at the 8-bit size.  for additive instructions only,
   it also provides C, V, BE, and LE, and L codes at all sizes
   starting at 8 bits: */
#define TME_RECODE_FLAG_NEED(insn_class, insn_size, cond, size) \
  (!((((insn_class) == TME_RECODE_INSN_CLASS_ADDITIVE		\
       || (insn_class) == TME_RECODE_INSN_CLASS_LOGICAL		\
       || (insn_class) == TME_RECODE_INSN_CLASS_SHIFT		\
       || (insn_class) == TME_RECODE_INSN_CLASS_EXT)		\
      && (((size) >= TME_RECODE_SIZE_8				\
	   && ((cond) == TME_RECODE_COND_N			\
	       || (cond) == TME_RECODE_COND_Z))			\
	  || ((size) == TME_RECODE_SIZE_8			\
	      && (cond) == TME_RECODE_COND_PE)))		\
     || ((insn_class) == TME_RECODE_INSN_CLASS_ADDITIVE		\
	 && ((size) >= TME_RECODE_SIZE_8			\
	     && ((cond) == TME_RECODE_COND_C			\
		 || (cond) == TME_RECODE_COND_V			\
		 || (cond) == TME_RECODE_COND_BE		\
		 || (cond) == TME_RECODE_COND_LE		\
		 || (cond) == TME_RECODE_COND_L)))))

/* TME_RECODE_FLAGS_COMPAT() evaluates to nonzero if two flags from
   two different flags groups are compatible.  this is used to
   determine if two flags groups are compatible (and can use the same
   code thunk), even though their instruction sizes are different.
   this can happen when two groups have the same flags relative to the
   instruction size.
   
   for x86, the ratio between instruction size and condition code size
   must be the same in both groups.  and because double-host-size
   condition codes need different thunk code than all others, either
   both condition code sizes must be the double-host size, or neither
   can be: */
#define TME_RECODE_FLAGS_COMPAT(flags_group0, flag0, flags_group1, flag1)\
  ((((flags_group0)->tme_recode_flags_group_insn_size		\
     - (flag0)->tme_recode_flag_size)				\
    == ((flags_group1)->tme_recode_flags_group_insn_size	\
        - (flag1)->tme_recode_flag_size))			\
   && (((flag0)->tme_recode_flag_size				\
	<= TME_RECODE_SIZE_HOST)				\
       == ((flag1)->tme_recode_flag_size			\
	   <= TME_RECODE_SIZE_HOST)))

/* the undefined host register number.  this is also the number of
   host registers, so this must match the number of elements in
   tme_recode_x86_reg_from_host[]: */
#if TME_RECODE_SIZE_HOST > 5
#define TME_RECODE_REG_HOST_UNDEF	(13)
#else  /* TME_RECODE_SIZE_HOST == 5 */
#define TME_RECODE_REG_HOST_UNDEF	(5)
#endif /* TME_RECODE_SIZE_HOST == 5 */

/* the alignment of a thunk, in bytes: */
#define TME_RECODE_HOST_THUNK_ALIGN			(16)

/* the overhead of a instructions thunk, in bytes: */
/* on x86, this is the maximum size of the chain in plus chain out: */
#define TME_RECODE_X86_CHAIN_IN_SIZE_MAX		(48)
#define TME_RECODE_X86_CHAIN_OUT_SIZE_MAX		(32)
#define TME_RECODE_HOST_INSN_THUNK_OVERHEAD		\
  (TME_RECODE_X86_CHAIN_IN_SIZE_MAX			\
   + TME_RECODE_X86_CHAIN_OUT_SIZE_MAX)

/* the maximum size of a single recoded insn, in bytes: */
#if TME_RECODE_SIZE_HOST > 5
#define TME_RECODE_HOST_INSN_SIZE_MAX			(384)
#else  /* TME_RECODE_SIZE_HOST == 5 */
#define TME_RECODE_HOST_INSN_SIZE_MAX			(256)
#endif /* TME_RECODE_SIZE_HOST == 5 */

/* these are the host-specific members added to struct
   tme_recode_flags_thunk: */
#define TME_RECODE_HOST_FLAGS_THUNK					\
									\
  /* the size-specific flags subs: */					\
  struct {								\
									\
    /* the integer-opcode-specific flags subs: */			\
    tme_recode_thunk_off_t tme_recode_x86_flags_thunk_size_subs[TME_RECODE_OPCODES_INTEGER];\
    									\
    /* the zero- and sign-extension subs are kept in this separate	\
       list, because their different source and destination operand	\
       sizes require multiple subs: */					\
    tme_recode_thunk_off_t tme_recode_x86_flags_thunk_size_subs_ext	\
      [TME_RECODE_SIZE_GUEST_MAX - TME_RECODE_SIZE_8][2];		\
									\
    /* the test flags subs, chained to by the flags subs for the	\
       logical, shift, and extension opcodes: */			\
    tme_recode_thunk_off_t tme_recode_x86_flags_thunk_size_subs_test;	\
									\
  } tme_recode_x86_flags_thunk_sizes[TME_RECODE_SIZE_GUEST_MAX + 1 - TME_RECODE_SIZE_8];\
									\
  /* this is the main flags subs, eventually chained to by all of the	\
     above subs: */							\
  tme_recode_thunk_off_t tme_recode_x86_flags_thunk_subs_main;		\
									\
  /* this is nonzero if the flags thunk calls a guest function: */	\
  unsigned int tme_recode_x86_flags_thunk_has_guest_func;		\
									\
  /* if there is a guest function, this is any amount of stack padding	\
     the subs need to do to satisfy the host ABI: */			\
  int tme_recode_x86_flags_thunk_stack_padding

/* these are the host-specific members added to struct
   tme_recode_conds_thunk: */
#define TME_RECODE_HOST_CONDS_THUNK					\
									\
  /* the conditions subs: */						\
  tme_recode_thunk_off_t tme_recode_x86_conds_thunk_subs;		\
									\
  /* the size of the part of the guest flags register tested by this	\
     thunk: */								\
  tme_uint8_t tme_recode_x86_conds_thunk_flags_size;			\
									\
  /* the struct tme_ic offset of the part of the guest flags register	\
     tested by this thunk: */						\
  tme_uint32_t tme_recode_x86_conds_thunk_flags_offset;			\
									\
  /* the simple condition information.  the exact size of this array	\
     is determined by the guest: */					\
  tme_uint32_t tme_recode_x86_conds_thunk_simple[1]

/* these are the host-specific members added to struct
   tme_recode_rw_thunk: */
#define TME_RECODE_HOST_RW_THUNK					\
									\
  /* the read or write subs: */						\
  tme_recode_thunk_off_t tme_recode_x86_rw_thunk_subs;			\
									\
  /* any sign- or zero-extension instruction and its size: */		\
  tme_uint32_t tme_recode_x86_rw_thunk_extend;				\
  tme_uint32_t tme_recode_x86_rw_thunk_extend_size

/* these are the host-specific members added to struct
   tme_recode_chain_thunk: */
#define TME_RECODE_HOST_CHAIN_THUNK					\
									\
  /* the chain subs: */							\
  tme_recode_thunk_off_t tme_recode_x86_chain_thunk_subs[8 + 4];	\
									\
  /* the chain prologue: */						\
  void (*tme_recode_x86_chain_thunk_prologue) _TME_P((struct tme_ic *, tme_recode_thunk_off_t))

/* these are the x86-specific members added to struct tme_recode_ic.
   this macro is added to TME_RECODE_HOST_IC at configure time: */
#define TME_RECODE_X86_IC						\
									\
  /* the thunk offset of the chain epilogue: */				\
  tme_recode_thunk_off_t tme_recode_x86_ic_chain_epilogue;		\
									\
  /* the chain fixup targets: */					\
  tme_recode_thunk_off_t tme_recode_x86_ic_chain_fixup_target[4 + 2];	\
									\
  /* the thunk offsets of the shift insn subs: */			\
  tme_recode_thunk_off_t tme_recode_x86_ic_subs_shift			\
    [TME_RECODE_SIZE_GUEST_MAX + 1 - TME_RECODE_SIZE_8][3];		\
									\
  /* if not TME_RECODE_REG_GUEST_WINDOW_UNDEF, the c register		\
     still holds the base offset of that guest window: */		\
  unsigned int tme_recode_x86_ic_thunks_reg_guest_window_c;		\
									\
  /* if not TME_RECODE_REG_GUEST_WINDOW_UNDEF, the c register still	\
     held the base offset of that guest window at the time of the	\
     previous if jump: */						\
  unsigned int tme_recode_x86_ic_thunks_reg_guest_window_c_if_jmp;	\
									\
  /* the position of an active if in the thunks build memory: */	\
  tme_recode_host_insn_t *tme_recode_x86_ic_thunks_build_if;		\
									\
  /* the position of an active else in the thunks build memory: */	\
  tme_recode_host_insn_t *tme_recode_x86_ic_thunks_build_else

/* this runs an instruction thunk: */
#define tme_recode_insns_thunk_run(ic, chain_thunk, insns_thunk)	\
  ((*((chain_thunk)->tme_recode_x86_chain_thunk_prologue))((ic), (insns_thunk)))

/* TLB flags: */
#define TME_RECODE_TLB_FLAG_CONTEXT_MISMATCH(ic) (((tme_uint32_t) 1) << (31 + (0 && ic)))
#define TME_RECODE_TLB_FLAG(ic, x)	(((tme_uint32_t) 1) << (30 - (x) + (0 && (ic))))
#define TME_RECODE_X86_TLB_FLAG_INVALID(ic)	((ic)->tme_recode_ic_tlb_page_size)
#define TME_RECODE_TLB_FLAGS_MASK(ic)	(0 - (tme_uint32_t) (TME_RECODE_X86_TLB_FLAG_INVALID(ic) * 3))

/* types: */

/* a host instruction: */
typedef tme_uint8_t tme_recode_host_insn_t;

/* a thunk offset: */
typedef tme_int32_t tme_recode_thunk_off_t;

/* a return address stack entry: */
typedef tme_uint32_t tme_recode_ras_entry_t;
