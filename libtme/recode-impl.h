/* $Id: recode-impl.h,v 1.5 2010/06/05 19:05:00 fredette Exp $ */

/* libtme/recode-impl.h - private header file for recode support: */

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

#ifndef _TME_RECODE_IMPL_H
#define _TME_RECODE_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: recode-impl.h,v 1.5 2010/06/05 19:05:00 fredette Exp $");

/* includes: */
#include <tme/recode.h>

/* types: */

/* a recode TLB type: */
struct tme_recode_tlb_type {

  /* the offset of the memory field: */
  unsigned long tme_recode_tlb_type_offset_memory;

  /* the offset of the page field: */
  unsigned long tme_recode_tlb_type_offset_page;

  /* the offset of the flags: */
  unsigned long tme_recode_tlb_type_offset_flags;

  /* the offset of the token: */
  unsigned long tme_recode_tlb_type_offset_token;

  /* the offset of any context: */
  unsigned long tme_recode_tlb_type_offset_context;

  /* the size: */
  unsigned long tme_recode_tlb_type_sizeof;
};

/* prototypes: */

/* this host function starts a new IC: */
void tme_recode_host_ic_new _TME_P((struct tme_recode_ic *));

/* this host function returns a new flags thunk: */
struct tme_recode_flags_thunk *tme_recode_host_flags_thunk_new _TME_P((struct tme_recode_ic *, const struct tme_recode_flags_thunk *, const struct tme_recode_flags_group *));

/* this host function adds another flags group to an existing flags thunk: */
struct tme_recode_flags_thunk *tme_recode_host_flags_thunk_add _TME_P((struct tme_recode_ic *, struct tme_recode_flags_thunk *, const struct tme_recode_flags_group *));
  
/* this generic function returns a mask of flags that are changed in
   the same deterministic way (defined, set, clear) by all of the
   instructions in the group and that are provided by the host,
   optionally matching only a certain condition: */
tme_recode_uguest_t tme_recode_flags_group_flags_defined_host _TME_P((const struct tme_recode_flags_group *, unsigned int));

/* this generic function returns the mask of sizes for the given
   flags: */
tme_uint32_t tme_recode_flags_group_sizes _TME_P((const struct tme_recode_flags_group *, tme_recode_uguest_t));

/* this returns the offset of the first byte of a flags register: */
tme_uint32_t tme_recode_flags_reg_offset _TME_P((unsigned int, tme_uint32_t));

/* this host function returns a new conditions thunk: */
struct tme_recode_conds_thunk *tme_recode_host_conds_thunk_new _TME_P((struct tme_recode_ic *, const struct tme_recode_conds_group *));

/* this generic function returns the maximum index for a conditions
   group's flags: */
tme_uint32_t tme_recode_conds_group_flags_index_max _TME_P((const struct tme_recode_conds_group *));

/* this generic function returns the indexed combination of a
   conditions group's flags: */
tme_recode_uguest_t tme_recode_conds_group_flags_from_index _TME_P((const struct tme_recode_conds_group *,
								    tme_uint32_t));

/* this generic function returns a simple mask of a conditions group's
   flags and one or more bitwise operations that can be used with the
   mask to test the given condition.  it returns zero if the condition
   can't be tested with a simple mask: */
tme_uint32_t tme_recode_conds_simple_mask _TME_P((const struct tme_recode_conds_group *, 
						  tme_uint32_t,
						  tme_recode_uguest_t *));

/* this host function returns a new read/write thunk: */
struct tme_recode_rw_thunk *tme_recode_host_rw_thunk_new _TME_P((struct tme_recode_ic *, const struct tme_recode_rw *));

/* this host function tries to duplicate a read/write thunk: */
struct tme_recode_rw_thunk *tme_recode_host_rw_thunk_dup _TME_P((struct tme_recode_ic *, const struct tme_recode_rw *, const struct tme_recode_rw *));

/* this generic function frees all host registers, starting from the
   given host register: */
void tme_recode_regs_host_free_many _TME_P((struct tme_recode_ic *,
					    unsigned long));

/* this cleans all dirty host registers, freeing those that weren't
   dirty at the full guest register size: */
void tme_recode_regs_host_clean_all _TME_P((struct tme_recode_ic *ic));

/* this generic function returns any best host register with the
   smallest read-uses count less than the given count: */
unsigned long tme_recode_regs_host_best _TME_P((const struct tme_recode_ic *,
						tme_uint32_t));

/* this generic function frees and returns a specific host
   register: */
unsigned long tme_recode_regs_host_free_specific _TME_P((struct tme_recode_ic *,
							 unsigned long));

/* this generic function reserves one host register, and returns the
   host register: */
unsigned long tme_recode_regs_host_reserve _TME_P((struct tme_recode_ic *,
						   unsigned long));

/* this generic function unreserves all host registers: */
void tme_recode_regs_host_unreserve_all _TME_P((struct tme_recode_ic *));

/* this notifies about a source operand before a call to
   tme_recode_regs_src_any() or tme_recode_regs_src_specific().  calls
   to this function aren't needed for correctness, but they may enable
   optimizations.  if called, the next call to
   tme_recode_regs_src_any() or tme_recode_regs_src_specific() *must*
   be for the same source operand: */
void tme_recode_regs_src_notify _TME_P((struct tme_recode_ic *,
					const struct tme_recode_insn *,
					signed long));

/* this generic function loads an instruction source operand into any
   host register, and reserves and returns the host register: */
unsigned long tme_recode_regs_src_any _TME_P((struct tme_recode_ic *,
					      const struct tme_recode_insn *,
					      signed long));

/* this generic function loads an instruction source operand into a
   specific host register, and reserves and returns that host
   register: */
unsigned long tme_recode_regs_src_specific _TME_P((struct tme_recode_ic *,
						   const struct tme_recode_insn *,
						   signed long,
						   unsigned long));

/* this generic function allocates any host register for the
   instruction destination operand, and returns the host register: */
unsigned long tme_recode_regs_dst_any _TME_P((struct tme_recode_ic *,
					      const struct tme_recode_insn *));

/* this generic function allocates a specific host register for the
   instruction destination operand and returns the host register: */
unsigned long tme_recode_regs_dst_specific _TME_P((struct tme_recode_ic *,
						   const struct tme_recode_insn *,
						   unsigned long));

/* this host function loads or stores a host register: */
void tme_recode_host_reg_move _TME_P((struct tme_recode_ic *,
				      tme_uint32_t,
				      tme_uint32_t));

/* this host function copies a host register into another host register: */
void tme_recode_host_reg_copy _TME_P((struct tme_recode_ic *,
				      unsigned long,
				      unsigned long));

/* this host function zeroes a host register, and reserves and returns
   the host register: */
unsigned long tme_recode_host_reg_zero _TME_P((struct tme_recode_ic *,
					       const struct tme_recode_insn *,
					       unsigned long));

/* this host function loads a host register with an immediate, and
   reserves and returns the host register: */
unsigned long tme_recode_host_reg_imm _TME_P((struct tme_recode_ic *,
					      const struct tme_recode_insn *,
					      unsigned long));

/* this host function returns a new instructions thunk: */
tme_recode_thunk_off_t tme_recode_host_insns_thunk_new _TME_P((struct tme_recode_ic *,
							       const struct tme_recode_insns_group *));

/* this host function allocates memory for building and running thunks: */
void tme_recode_host_thunks_alloc _TME_P((struct tme_recode_ic *, tme_recode_thunk_off_t));

/* this host function starts building a new thunk.  it returns nonzero
   if a thunk of TME_RECODE_HOST_THUNK_SIZE_MAX bytes can be built: */
int tme_recode_host_thunk_start _TME_P((struct tme_recode_ic *));

/* this host function finishes building a new thunk: */
void tme_recode_host_thunk_finish _TME_P((struct tme_recode_ic *));

/* this host function invalidates all thunks starting from the given
   thunk offset: */
void tme_recode_host_thunk_invalidate_all _TME_P((struct tme_recode_ic *, tme_recode_thunk_off_t));

/* this generic function checks an address information structure: */
void tme_recode_address_type_check _TME_P((const struct tme_recode_ic *,
					   const struct tme_recode_address_type *));

/* this generic function returns zero if two address information
   structures are the same: */
int tme_recode_address_type_compare _TME_P((const struct tme_recode_ic *,
					    const struct tme_recode_address_type *,
					    const struct tme_recode_address_type *));

/* this returns the recode TLB type for an address type: */
void tme_recode_address_type_tlb_type _TME_P((const struct tme_recode_ic *,
					      const struct tme_recode_address_type *,
					      struct tme_recode_tlb_type *));

#ifdef TME_RECODE_DEBUG

/* this host function dumps a flags thunk: */
void tme_recode_host_flags_thunk_dump _TME_P((_tme_const struct tme_recode_ic *,
					      _tme_const struct tme_recode_flags_thunk *));

/* this host function dumps a conditions thunk: */
void tme_recode_host_conds_thunk_dump _TME_P((_tme_const struct tme_recode_ic *,
					      _tme_const struct tme_recode_conds_thunk *,
					      _tme_const char * _tme_const *));

#endif /* TME_RECODE_DEBUG */

#endif /* _TME_RECODE_IMPL_H */
