/* $Id: recode.h,v 1.8 2010/03/07 16:29:27 fredette Exp $ */

/* tme/recode.h - public header file for recode support: */

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

#ifndef _TME_RECODE_H
#define _TME_RECODE_H

#include <tme/common.h>
_TME_RCSID("$Id: recode.h,v 1.8 2010/03/07 16:29:27 fredette Exp $");

/* includes: */
#include <tme/recode-host.h>
#include <tme/generic/ic.h>
#include <tme/token.h>

/* macros: */

/* log base two of power-of-two sizes: */
#define TME_RECODE_SIZE_1	(0)
#define TME_RECODE_SIZE_2	(1)
#define TME_RECODE_SIZE_4	(2)
#define TME_RECODE_SIZE_8	(3)
#define TME_RECODE_SIZE_16	(4)
#define TME_RECODE_SIZE_32	(5)
#define TME_RECODE_SIZE_64	(6)
#define TME_RECODE_SIZE_128	(7)

/* configure initially defines TME_RECODE_SIZE_GUEST_MAX to be the log
   base two of the largest natural word size (in bits) of all guests
   with recode support that are selected at configure time.

   however, we don't support guest sizes larger than twice the host's
   size, or guest sizes for which we don't have an integral type, so
   we may have to reduce TME_RECODE_SIZE_GUEST_MAX.
   TME_RECODE_SIZE_GUEST_MAX is always at least five, to support
   32-bit guests: */

/* limit TME_RECODE_SIZE_GUEST_MAX to (TME_RECODE_SIZE_HOST + 1): */
#if TME_RECODE_SIZE_GUEST_MAX > (TME_RECODE_SIZE_HOST + 1)
#undef TME_RECODE_SIZE_GUEST_MAX
#define TME_RECODE_SIZE_GUEST_MAX (TME_RECODE_SIZE_HOST + 1)
#endif /* TME_RECODE_SIZE_GUEST_MAX > (TME_RECODE_SIZE_HOST + 1) */

/* we don't support guests larger than 128 bits: */
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_128
#undef TME_RECODE_SIZE_GUEST_MAX
#define TME_RECODE_SIZE_GUEST_MAX	TME_RECODE_SIZE_128
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_128 */

/* we only support 128-bit guests if we have a 128-bit integral type: */
#if TME_RECODE_SIZE_GUEST_MAX == TME_RECODE_SIZE_128
#ifdef TME_HAVE_INT128_T
typedef tme_int128_t tme_recode_guest_t;
typedef tme_uint128_t tme_recode_uguest_t;
#else  /* !TME_HAVE_INT128_T */
#undef TME_RECODE_SIZE_128
#undef TME_RECODE_SIZE_GUEST_MAX
#define TME_RECODE_SIZE_GUEST_MAX	TME_RECODE_SIZE_64
#endif /* !TME_HAVE_INT128_T */
#endif /* TME_RECODE_SIZE_GUEST_MAX == TME_RECODE_SIZE_128 */

/* we only support 64-bit guests if we have a 64-bit integral type: */
#if TME_RECODE_SIZE_GUEST_MAX == TME_RECODE_SIZE_64
#ifdef TME_HAVE_INT64_T
typedef tme_int64_t tme_recode_guest_t;
typedef tme_uint64_t tme_recode_uguest_t;
#else  /* !TME_HAVE_INT64_T */
#undef TME_RECODE_SIZE_64
#undef TME_RECODE_SIZE_GUEST_MAX
#define TME_RECODE_SIZE_GUEST_MAX	TME_RECODE_SIZE_64
#endif /* !TME_HAVE_INT64_T */
#endif /* TME_RECODE_SIZE_GUEST_MAX == TME_RECODE_SIZE_64 */

/* we always support at least 32-bit guests: */
#if TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_32
typedef tme_int32_t tme_recode_guest_t;
typedef tme_uint32_t tme_recode_uguest_t;
#undef TME_RECODE_SIZE_GUEST_MAX
#define TME_RECODE_SIZE_GUEST_MAX	TME_RECODE_SIZE_32
#endif /* TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_32 */

/* this evaluates to nonzero if the size is double-host-size: */
#define TME_RECODE_SIZE_IS_DOUBLE_HOST(size)		\
  (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST	\
   && (size) == TME_RECODE_SIZE_GUEST_MAX)

/* this evaluates to nonzero if the given unsigned value fits in the
   given size.  the value is only evaluated once, while the size is
   evaluated many times and so should be a constant.  NB that we are
   careful not to generate any compile-time shift counts that equal to
   or greater than the number of bits in value's type: */
#define TME_RECODE_SIZE_FITS(value, size)		\
  (((sizeof(value) * 8) <= TME_BIT(size))		\
   || (((value)						\
	>> ((((TME_BIT(size)				\
	       > (sizeof(unsigned int) * 8))		\
	      * (TME_BIT(size)				\
		 - (sizeof(unsigned int) * 8)))		\
	     + 1)					\
	    * ((sizeof(value) * 8) > TME_BIT(size))))	\
       < (((unsigned int) 1)				\
	  << (TME_MIN(TME_BIT(size),			\
		      sizeof(unsigned int) * 8)		\
	      - 1))))

/* conditions: */
#define TME_RECODE_COND_MOD_NOT	TME_BIT(0)
#define _TME_RECODE_COND(x)	((x) << 4)
#define TME_RECODE_COND_UNDEF	_TME_RECODE_COND(0)
#define TME_RECODE_COND_FALSE	_TME_RECODE_COND(1)
#define TME_RECODE_COND_N	_TME_RECODE_COND(2)
#define TME_RECODE_COND_C	_TME_RECODE_COND(3)
#define TME_RECODE_COND_V	_TME_RECODE_COND(4)
#define TME_RECODE_COND_Z	_TME_RECODE_COND(5)
#define TME_RECODE_COND_PE	_TME_RECODE_COND(6)
#define TME_RECODE_COND_BE	_TME_RECODE_COND(7)
#define TME_RECODE_COND_LE	_TME_RECODE_COND(8)
#define TME_RECODE_COND_L	_TME_RECODE_COND(9)
#define TME_RECODE_COND_X(n)	_TME_RECODE_COND(10 + (n))

/* bitwise operations.  NB that these are not instruction opcodes: */
#define TME_RECODE_BITWISE_OR	TME_BIT(0)
#define TME_RECODE_BITWISE_NOR	TME_BIT(1)
#define TME_RECODE_BITWISE_AND	TME_BIT(2)
#define TME_RECODE_BITWISE_NAND	TME_BIT(3)

/* instruction classes: */
#define TME_RECODE_INSN_CLASS_ADDITIVE		(0)
#define TME_RECODE_INSN_CLASS_LOGICAL		(1)
#define TME_RECODE_INSN_CLASS_SHIFT		(2)
#define TME_RECODE_INSN_CLASS_EXT		(3)

/* instruction opcodes: */
/* NB: these are in a deliberate order: */
#define TME_RECODE_OPCODE_ANDN			(0)
#define TME_RECODE_OPCODE_ORN			(1)
#define TME_RECODE_OPCODE_XORN			(2)
#define TME_RECODE_OPCODE_SUB			(3)
#define TME_RECODE_OPCODE_SUBC			(4)
#define TME_RECODE_OPCODE_AND			(5)
#define TME_RECODE_OPCODE_OR			(6)
#define TME_RECODE_OPCODE_XOR			(7)
#define TME_RECODE_OPCODE_ADD			(8)
#define TME_RECODE_OPCODE_ADDC			(9)
#define TME_RECODE_OPCODE_SHLL     		(10)
#define TME_RECODE_OPCODE_SHRL     		(11)
#define TME_RECODE_OPCODE_SHRA     		(12)
#define TME_RECODE_OPCODE_EXTZ     		(13)
#define TME_RECODE_OPCODE_EXTS     		(14)
#define  TME_RECODE_OPCODES_INTEGER		 (15)
#define TME_RECODE_OPCODE_GUEST     		(15)
#define TME_RECODE_OPCODE_DEFC			(16)
#define TME_RECODE_OPCODE_IF			(17)
#define TME_RECODE_OPCODE_ELSE			(18)
#define TME_RECODE_OPCODE_ENDIF			(19)
						/* 20 unused */
#define TME_RECODE_OPCODE_RW			(21)

/* this maps a recode logical noncommutative opcode to its plain version: */
#define TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(opcode)	\
  (((opcode) + TME_RECODE_OPCODE_AND) - TME_RECODE_OPCODE_ANDN)
#if TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(TME_RECODE_OPCODE_ANDN) != TME_RECODE_OPCODE_AND
#error "TME_RECODE_OPCODE_ values changed"
#endif
#if TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(TME_RECODE_OPCODE_ORN) != TME_RECODE_OPCODE_OR
#error "TME_RECODE_OPCODE_ values changed"
#endif
#if TME_RECODE_OPCODE_LOGICALN_TO_PLAIN(TME_RECODE_OPCODE_XORN) != TME_RECODE_OPCODE_XOR
#error "TME_RECODE_OPCODE_ values changed"
#endif

/* guest registers and guest non-register operands: */
#define TME_RECODE_REG_GUEST(x)			(x)

/* non-register source operands: */
#define TME_RECODE_OPERAND_ZERO			TME_RECODE_REG_GUEST(-1)
#define TME_RECODE_OPERAND_IMM			TME_RECODE_REG_GUEST(-2)
#define TME_RECODE_OPERAND_UNDEF		TME_RECODE_REG_GUEST(-3)

/* non-register destination operands: */
#define TME_RECODE_OPERAND_NULL			TME_RECODE_REG_GUEST(-1)

/* the undefined guest register window: */
#define TME_RECODE_REG_GUEST_WINDOW_UNDEF	(3)

/* host registers: */
#define TME_RECODE_REG_HOST(x)			(x)

/* guest register info: */
/* NB: these are in a deliberate order, to match union
   tme_recode_reginfo and the tme_recode_reginfo_ macros for its
   parts: */
#define TME_RECODE_REGINFO_TYPE_FLAT		(0)
#define TME_RECODE_REGINFO_TYPE_WINDOW(n) 	(((n) + 1) << 30)
#define TME_RECODE_REGINFO_TYPE_IS_WINDOW(x)	((x) & (TME_RECODE_REG_GUEST_WINDOW_UNDEF << 30))
#define TME_RECODE_REGINFO_TYPE_WHICH_WINDOW(x)	(((x) >> 30) - 1)
#define TME_RECODE_REGINFO_TYPE_FIXED		TME_BIT(29)
#define TME_RECODE_REGINFO_TYPE_TEMPORARY	TME_BIT(28)
#define TME_RECODE_REGINFO_RUSES(x)		((tme_uint16_t) (x))
#define TME_RECODE_REGINFO_TAGS_VALID		TME_BIT(15)
#define TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK	(0x1e00)
#define TME_RECODE_REGINFO_TAGS_CLEAN		(1 << 8)
#define TME_RECODE_REGINFO_TAGS_WHICH_VALID_SIZE(x)			\
  TME_FIELD_MASK_EXTRACTU(x, TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK)
#define TME_RECODE_REGINFO_TAGS_ARE_VALID_SIZE(x, size)			\
  (TME_RECODE_REGINFO_TAGS_WHICH_VALID_SIZE(x)				\
   >= (size))
#if ((TME_RECODE_REGINFO_TAGS_CLEAN << 1) & TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK) == 0
#error "TME_RECODE_REGINFO_TAGS_ values changed"
#endif
#define TME_RECODE_REGINFO_TAGS_WHICH_DIRTY_SIZE(x)				\
  TME_FIELD_MASK_EXTRACTU((((TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK		\
			     + TME_RECODE_REGINFO_TAGS_CLEAN)			\
			    + ((x)						\
			       & TME_RECODE_REGINFO_TAGS_CLEAN))		\
			   & (x)),						\
			  TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK)
#define TME_RECODE_REGINFO_TAGS_ARE_DIRTY(x)	(TME_RECODE_REGINFO_TAGS_WHICH_DIRTY_SIZE(x) != 0)
#define TME_RECODE_REGINFO_TAGS_VALID_SIZE(x)	((x) * _TME_FIELD_MASK_FACTOR(TME_RECODE_REGINFO_TAGS_VALID_SIZE_MASK))
#define TME_RECODE_REGINFO_TAGS_REG_HOST(x)	(x)
#define TME_RECODE_REGINFO_TAGS_WHICH_REG_HOST(x) ((tme_uint8_t) (x))

/* special register read-uses values: */
#define TME_RECODE_REG_RUSES_FREE		(0)
#define TME_RECODE_REG_RUSES_RESERVED		(TME_RECODE_REGINFO_TAGS_VALID)

/* special register read-uses record values: */
#define TME_RECODE_REG_RUSES_RECORD_REG_GUEST(x)	(TME_RECODE_REG_RUSES_RESERVED + (x))
#define TME_RECODE_REG_RUSES_RECORD_UNDEF	(TME_RECODE_REG_RUSES_RESERVED + (TME_RECODE_REG_RUSES_RESERVED - 1))

/* recode flags: */
#define TME_RECODE_FLAG(n)			(n)
#define TME_RECODE_FLAG_CARRY			TME_RECODE_FLAG(0)
#define TME_RECODE_FLAG_JUMP			TME_RECODE_FLAG(1)

/* chain information: */
#define TME_RECODE_CHAIN_INFO_UNCONDITIONAL	(0 << 0)
#define TME_RECODE_CHAIN_INFO_CONDITIONAL	(1 << 0)
#define TME_RECODE_CHAIN_INFO_NEAR		(0 << 1)
#define TME_RECODE_CHAIN_INFO_FAR		(1 << 1)
#define TME_RECODE_CHAIN_INFO_JUMP		(1 << 2)
#define TME_RECODE_CHAIN_INFO_RETURN		(1 << 3)
#define TME_RECODE_CHAIN_INFO_CALL		(1 << 4)
#define TME_RECODE_CHAIN_INFO_ALTERNATE_NEAR	(0 << 5)
#define TME_RECODE_CHAIN_INFO_ALTERNATE_FAR	(1 << 5)

/* the maximum size of a single instructions thunk, in insns: */
#define TME_RECODE_INSNS_THUNK_INSNS_MAX	\
  ((TME_RECODE_HOST_THUNK_SIZE_MAX		\
    - TME_RECODE_HOST_INSN_THUNK_OVERHEAD)	\
   / TME_RECODE_HOST_INSN_SIZE_MAX)

/* types: */

/* one flag: */
struct tme_recode_flag {

  /* the condition that sets the flag: */
  tme_uint16_t tme_recode_flag_cond;

  /* the operand size: */
  tme_uint8_t tme_recode_flag_size;

  /* the flag: */
  tme_recode_uguest_t tme_recode_flag_flag;
};

/* a flags thunk: */
struct tme_recode_flags_thunk {

  /* the flags that are changed in any way (undefined, defined, set,
     clear) by the thunk: */
  tme_recode_uguest_t tme_recode_flags_thunk_flags_changed;

  /* the flags that are changed in a deterministic way (defined, set,
     clear) by the thunk: */
  tme_recode_uguest_t tme_recode_flags_thunk_flags_defined;

  /* the struct tme_ic offset of the flags register: */
  tme_uint32_t tme_recode_flags_thunk_flags_offset;

  /* the host-specific members: */
  TME_RECODE_HOST_FLAGS_THUNK;
};

/* a flags group: */
struct tme_recode_flags_group {

  /* flags groups are kept on a list: */
  _tme_const struct tme_recode_flags_group *tme_recode_flags_group_next;

  /* the class and size of instructions in this flags group: */
  tme_uint8_t tme_recode_flags_group_insn_class;
  tme_uint8_t tme_recode_flags_group_insn_size;

  /* the size and index of the flags register in the struct tme_ic: */
  tme_uint8_t tme_recode_flags_group_flags_reg_size;
  tme_uint32_t tme_recode_flags_group_flags_reg;

  /* the flags for all of the instructions in the group: */
  _tme_const struct tme_recode_flag *tme_recode_flags_group_flags;

  /* the guest flags function: */
  void (*tme_recode_flags_group_guest_func) _TME_P((struct tme_ic *, tme_recode_uguest_t, tme_recode_uguest_t, tme_recode_uguest_t));

  /* the flags thunk: */
  struct tme_recode_flags_thunk *tme_recode_flags_group_thunk;
};

/* a conditions thunk: */
struct tme_recode_conds_thunk {

  /* the flags tested by the thunk: */
  tme_recode_uguest_t tme_recode_conds_thunk_flags;

  /* the struct tme_ic offset of the flags register: */
  tme_uint32_t tme_recode_conds_thunk_flags_offset;

  /* the host-specific members: */
  TME_RECODE_HOST_CONDS_THUNK;
};

/* a conditions group: */
struct tme_recode_conds_group {

  /* conditions groups are kept on a list: */
  _tme_const struct tme_recode_conds_group *tme_recode_conds_group_next;

  /* the size and index of the flags register in the struct tme_ic: */
  tme_uint8_t tme_recode_conds_group_flags_reg_size;
  tme_uint32_t tme_recode_conds_group_flags_reg;

  /* the flags tested by the conditions in the group: */
  tme_recode_uguest_t tme_recode_conds_group_flags;

  /* the number of conditions in the group.  this must be at least
     one, and not more than 32: */
  tme_uint8_t tme_recode_conds_group_cond_count;

  /* the guest condition function.  this returns nonzero if the
     condition is true for the given flags: */
  int (*tme_recode_conds_group_guest_func) _TME_P((tme_recode_uguest_t, tme_uint32_t));

  /* the conditions thunk: */
  struct tme_recode_conds_thunk *tme_recode_conds_group_thunk;
};

#ifdef TME_RECODE_SIZE_128
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128

/* a TLB entry with a 16-bit context and a 128-bit address: */
struct tme_recode_tlb_c16_a128 {

  /* adding an offset in this TLB entry's page to this pointer gives
     the host memory address: */
  _tme_const tme_shared tme_uint8_t *tme_recode_tlb_c16_a128_memory;

  /* the page frame of this TLB entry's page: */
  tme_uint128_t tme_recode_tlb_c16_a128_page;

  /* the flags for this TLB entry: */
  tme_uint32_t tme_recode_tlb_c16_a128_flags;

  /* the context for this TLB entry: */
  tme_uint16_t tme_recode_tlb_c16_a128_context;

  /* the token for this TLB entry: */
  struct tme_token tme_recode_tlb_c16_a128_token;
};

#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128 */
#endif /* TME_RECODE_SIZE_128 */

#ifdef TME_RECODE_SIZE_64
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64

/* a TLB entry with a 16-bit context and a 64-bit address: */
struct tme_recode_tlb_c16_a64 {

  /* adding an offset in this TLB entry's page to this pointer gives
     the host memory address: */
  _tme_const tme_shared tme_uint8_t *tme_recode_tlb_c16_a64_memory;

  /* the page frame of this TLB entry's page: */
  tme_uint64_t tme_recode_tlb_c16_a64_page;

  /* the flags for this TLB entry: */
  tme_uint32_t tme_recode_tlb_c16_a64_flags;

  /* the context for this TLB entry: */
  tme_uint16_t tme_recode_tlb_c16_a64_context;

  /* the token for this TLB entry: */
  struct tme_token tme_recode_tlb_c16_a64_token;
};

#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64 */
#endif /* TME_RECODE_SIZE_64 */

/* a TLB entry with a 16-bit context and a 32-bit address: */
struct tme_recode_tlb_c16_a32 {

  /* adding an offset in this TLB entry's page to this pointer gives
     the host memory address: */
  _tme_const tme_shared tme_uint8_t *tme_recode_tlb_c16_a32_memory;

  /* the page frame of this TLB entry's page: */
  tme_uint32_t tme_recode_tlb_c16_a32_page;

  /* the flags for this TLB entry: */
  tme_uint32_t tme_recode_tlb_c16_a32_flags;

  /* the context for this TLB entry: */
  tme_uint16_t tme_recode_tlb_c16_a32_context;

  /* the token for this TLB entry: */
  struct tme_token tme_recode_tlb_c16_a32_token;
};

/* an address type: */
struct tme_recode_address_type {

  /* the struct tme_ic_offset of the tme_bus_context_t context
     register, or less than zero if this address type doesn't include
     a context: */
  tme_int32_t tme_recode_address_type_context_ic_offset;

  /* if this address type includes a context, this is the size of a
     context value: */
  tme_uint8_t tme_recode_address_type_context_size;

  /* the address size and signedness: */
  unsigned int tme_recode_address_type_size;
  int tme_recode_address_type_signed;

  /* the minimum alignment for an address to not always cause an
     assist: */
  /* NB: this is a power of two value in bytes, not a TME_RECODE_SIZE_: */
  tme_uint32_t tme_recode_address_type_align_min;

  /* the TLB flags for this address type.  any TLB flag for the
     address type that isn't cleared by the mask in the TLB entry or
     any mask in the struct tme_ic causes an assist: */
  tme_uint32_t tme_recode_address_type_tlb_flags;

  /* the struct tme_ic offset of any tme_uint32_t TLB flags: */
  tme_int32_t tme_recode_address_type_tlb_flags_ic_offset;

  /* a mask of consecutive address bits that select a TLB entry: */
  tme_uint32_t tme_recode_address_type_mask_tlb_index;

  /* the struct tme_ic offset of the first recode TLB: */
  tme_int32_t tme_recode_address_type_tlb0_ic_offset;
};

/* a read or a write thunk: */
struct tme_recode_rw_thunk {

  /* the address size and register size: */
  tme_uint8_t tme_recode_rw_thunk_address_size;
  tme_uint8_t tme_recode_rw_thunk_reg_size;

  /* this is nonzero if this is a write: */
  tme_uint8_t tme_recode_rw_thunk_write;

  /* the host-specific members: */
  TME_RECODE_HOST_RW_THUNK;
};

/* a read or a write: */
struct tme_recode_rw {

  /* reads and writes are kept on a list: */
  _tme_const struct tme_recode_rw *tme_recode_rw_next;

  /* the address type: */
  struct tme_recode_address_type tme_recode_rw_address_type;

  /* this is nonzero if this is a write: */
  int tme_recode_rw_write;

  /* the register size: */
  unsigned int tme_recode_rw_reg_size;

  /* the memory size, byte order, and signedness: */
  unsigned int tme_recode_rw_memory_size;
  unsigned int tme_recode_rw_memory_endian;
  int tme_recode_rw_memory_signed;

  /* the bus boundary.  reads and writes are split into atomic parts
     at bus boundaries: */
  /* NB: this is a power of two value in bytes, not a TME_RECODE_SIZE_: */
  tme_uint32_t tme_recode_rw_bus_boundary;

  /* the assist functions: */
  union {

    /* the read assist function: */
    tme_recode_uguest_t (*_tme_recode_rw_guest_func_u_read) _TME_P((struct tme_ic *, tme_recode_uguest_t));
#define tme_recode_rw_guest_func_read _tme_recode_rw_guest_func_u._tme_recode_rw_guest_func_u_read

    /* the write assist function: */
    void (*_tme_recode_rw_guest_func_u_write) _TME_P((struct tme_ic *, tme_recode_uguest_t, tme_recode_uguest_t));
#define tme_recode_rw_guest_func_write _tme_recode_rw_guest_func_u._tme_recode_rw_guest_func_u_write
  } _tme_recode_rw_guest_func_u;

  /* the read/write thunk: */
  _tme_const struct tme_recode_rw_thunk *tme_recode_rw_thunk;
};

/* a chain thunk: */
struct tme_recode_chain_thunk {

  /* the host-specific members: */
  TME_RECODE_HOST_CHAIN_THUNK;
};

/* a chain: */
struct tme_recode_chain {

  /* the guest register with the next PC: */
  tme_int16_t tme_recode_chain_reg_guest;

  /* the address type: */
  struct tme_recode_address_type tme_recode_chain_address_type;
};

/* one instruction: */
struct tme_recode_insn {

  /* the opcode and size: */
  tme_uint8_t tme_recode_insn_opcode;
  tme_uint8_t tme_recode_insn_size;

  /* two source operands and the destination operand: */
  tme_int16_t tme_recode_insn_operands[3];
#define tme_recode_insn_operand_src tme_recode_insn_operands
#define tme_recode_insn_operand_dst tme_recode_insn_operands[2]

  /* any immediate for the instruction: */
  union {
    tme_recode_guest_t _tme_recode_insn_imm_u_guest;
    tme_recode_uguest_t _tme_recode_insn_imm_u_uguest;
  } _tme_recode_insn_imm_u;
#define tme_recode_insn_imm_guest _tme_recode_insn_imm_u._tme_recode_insn_imm_u_guest
#define tme_recode_insn_imm_uguest _tme_recode_insn_imm_u._tme_recode_insn_imm_u_uguest
#ifdef TME_RECODE_SIZE_128
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128
#define tme_recode_insn_imm_128 tme_recode_insn_imm_guest
#define tme_recode_insn_imm_u128 tme_recode_insn_imm_uguest
#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_128 */
#endif /* TME_RECODE_SIZE_128 */
#ifdef TME_RECODE_SIZE_64
#if TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64
#define tme_recode_insn_imm_64 tme_recode_insn_imm_guest
#define tme_recode_insn_imm_u64 tme_recode_insn_imm_uguest
#endif /* TME_RECODE_SIZE_GUEST_MAX >= TME_RECODE_SIZE_64 */
#endif /* TME_RECODE_SIZE_64 */
#define tme_recode_insn_imm_32 tme_recode_insn_imm_guest
#define tme_recode_insn_imm_u32 tme_recode_insn_imm_uguest
#define tme_recode_insn_imm_16 tme_recode_insn_imm_guest
#define tme_recode_insn_imm_u16 tme_recode_insn_imm_uguest
#define tme_recode_insn_imm_8 tme_recode_insn_imm_guest
#define tme_recode_insn_imm_u8 tme_recode_insn_imm_uguest

  /* any extra information needed by the instruction: */
  union {

    /* a generic pointer: */
    void *_tme_recode_insn_extra_u_pointer;

    /* any thunk offset: */
    tme_recode_thunk_off_t _tme_recode_insn_extra_u_thunk_off;
#define tme_recode_insn_thunk_off _tme_recode_insn_extra_u._tme_recode_insn_extra_u_thunk_off

    /* any flags thunk: */
    _tme_const struct tme_recode_flags_thunk *_tme_recode_insn_extra_u_flags_thunk;
#define tme_recode_insn_flags_thunk _tme_recode_insn_extra_u._tme_recode_insn_extra_u_flags_thunk

    /* any conditions thunk: */
    _tme_const struct tme_recode_conds_thunk *_tme_recode_insn_extra_u_conds_thunk;
#define tme_recode_insn_conds_thunk _tme_recode_insn_extra_u._tme_recode_insn_extra_u_conds_thunk

    /* any read/write thunk: */
    _tme_const struct tme_recode_rw_thunk *_tme_recode_insn_extra_u_rw_thunk;
#define tme_recode_insn_rw_thunk _tme_recode_insn_extra_u._tme_recode_insn_extra_u_rw_thunk

    /* the guest function: */
    tme_recode_uguest_t (*_tme_recode_insn_extra_u_guest_func) _TME_P((struct tme_ic *, tme_recode_uguest_t, tme_recode_uguest_t));
#define tme_recode_insn_guest_func _tme_recode_insn_extra_u._tme_recode_insn_extra_u_guest_func
  } _tme_recode_insn_extra_u;
};

/* an instructions group: */
struct tme_recode_insns_group {

  /* the instructions in the group: */
  struct tme_recode_insn *tme_recode_insns_group_insns;

  /* one past the last instruction in the group: */
  struct tme_recode_insn *tme_recode_insns_group_insns_end;

  /* the source address for the corresponding guest instructions: */
  _tme_const tme_shared tme_uint8_t *tme_recode_insns_group_src;

  /* any validity flag for the group: */
  _tme_const tme_shared tme_uint8_t *tme_recode_insns_group_valid_byte;
  tme_uint8_t tme_recode_insns_group_valid_mask;

  /* the chain thunk for the group: */
  _tme_const struct tme_recode_chain_thunk *tme_recode_insns_group_chain_thunk;

  /* the chain information for the group: */
  tme_uint32_t tme_recode_insns_group_chain_info;
};

/* the guest register info union: */
/* NB that this union, and the tme_recode_reginfo_ macros for its
   parts, must match the TME_RECODE_REGINFO_ values: */
union tme_recode_reginfo {
  tme_uint8_t _tme_recode_reginfo_uint8s[4];
  tme_uint16_t _tme_recode_reginfo_uint16s[2];
  tme_uint32_t tme_recode_reginfo_all;
};
#ifndef WORDS_BIGENDIAN
#define tme_recode_reginfo_type	_tme_recode_reginfo_uint8s[3]
#define tme_recode_reginfo_tags_ruses _tme_recode_reginfo_uint16s[0]
#else  /* WORDS_BIGENDIAN */
#define tme_recode_reginfo_type	_tme_recode_reginfo_uint8s[0]
#define tme_recode_reginfo_tags_ruses _tme_recode_reginfo_uint16s[1]
#endif /* WORDS_BIGENDIAN */

/* a recode ic: */
struct tme_recode_ic {

  /* this maps from host register to ruses: */
  tme_uint16_t tme_recode_ic_reg_host_to_ruses[TME_RECODE_REG_HOST_UNDEF];

  /* this maps from host register to guest register: */
  tme_uint16_t tme_recode_ic_reg_host_to_reg_guest[TME_RECODE_REG_HOST_UNDEF];

  /* the register size: */
  tme_uint8_t tme_recode_ic_reg_size;

  /* the count of guest registers: */
  tme_uint32_t tme_recode_ic_reg_count;
  
  /* the next position in the thunk build memory: */
  tme_recode_host_insn_t *tme_recode_ic_thunk_build_next;

  /* one past the last position in the thunk build memory: */
  _tme_const tme_recode_host_insn_t *tme_recode_ic_thunk_build_end;

  /* the thunk offset of the first variable thunk: */
  tme_recode_thunk_off_t tme_recode_ic_thunk_off_variable;

  /* the host-specific members: */
  TME_RECODE_HOST_IC;

  /* up to four reserved registers: */
  struct {
    tme_uint16_t tme_reg_host_reserve_reg_host;
    tme_uint16_t tme_reg_host_reserve_ruses;
  } tme_recode_ic_reg_host_reserve[4];
  unsigned int tme_recode_ic_reg_host_reserve_next;

  /* the read-uses records: */
  tme_uint16_t *tme_recode_ic_reg_guest_ruses_records;
  tme_uint32_t tme_recode_ic_reg_guest_ruses_record_count;
  tme_uint32_t tme_recode_ic_reg_guest_ruses_record_next;

  /* the flags groups for this ic: */
  _tme_const struct tme_recode_flags_group *tme_recode_ic_flags_groups;

  /* the conditions groups for this ic: */
  _tme_const struct tme_recode_conds_group *tme_recode_ic_conds_groups;

  /* the reads and writes for this ic: */
  _tme_const struct tme_recode_rw *tme_recode_ic_rws;

  /* the TLB page size for this ic: */
  tme_uint32_t tme_recode_ic_tlb_page_size;

  /* the struct tme_ic offset of the pointer to the token for the
     current instruction TLB entry: */
  tme_int32_t tme_recode_ic_itlb_current_token_offset;

  /* the chain fixup function: */
  tme_recode_thunk_off_t (*tme_recode_ic_chain_fixup) _TME_P((struct tme_ic *, tme_recode_thunk_off_t, tme_uint32_t));

  /* the struct tme_ic offset of the tme_uint32_t chain counter: */
  tme_int32_t tme_recode_ic_chain_counter_offset;

  /* the number of entries in the chain return address stack: */
  tme_uint32_t tme_recode_ic_chain_ras_size;

  /* the struct tme_ic offset of the tme_recode_ras_entry_t chain
     return address stack: */
  tme_int32_t tme_recode_ic_chain_ras_offset;

  /* the struct tme_ic offset of the tme_uint32_t chain return address
     stack pointer: */
  tme_int32_t tme_recode_ic_chain_ras_pointer_offset;

  /* the struct tme_ic offsets of window base registers, which are
     also tme_int32_t struct tme_ic offsets: */
  tme_int32_t tme_recode_ic_window_base_offsets[TME_RECODE_REG_GUEST_WINDOW_UNDEF];

  /* space for the dummy guest register information for the
     non-register source and destination operands.  this dummy
     information isn't actually accessed using this array; it's still
     accessed using tme_recode_ic_reginfo[] below: */
  union tme_recode_reginfo tme_recode_ic_reginfo_dummy[2];

  /* the guest register information.  the exact size of this array is
     determined by the guest: */
  union tme_recode_reginfo tme_recode_ic_reginfo[1];
};

/* prototypes: */

/* this initializes a new recode ic: */
void tme_recode_ic_new _TME_P((struct tme_recode_ic *, tme_recode_thunk_off_t));

/* this returns a flags thunk for a flags group: */
_tme_const struct tme_recode_flags_thunk *tme_recode_flags_thunk _TME_P((struct tme_recode_ic *, _tme_const struct tme_recode_flags_group *));

/* this returns a conditions thunk for a conditions group: */
_tme_const struct tme_recode_conds_thunk *tme_recode_conds_thunk _TME_P((struct tme_recode_ic *, _tme_const struct tme_recode_conds_group *));

/* this returns a read/write thunk for a read or a write: */
_tme_const struct tme_recode_rw_thunk *tme_recode_rw_thunk _TME_P((struct tme_recode_ic *, _tme_const struct tme_recode_rw *));

/* this clears the return address stack: */
void tme_recode_chain_ras_clear _TME_P((const struct tme_recode_ic *, struct tme_ic *));

/* this returns a chain thunk: */
_tme_const struct tme_recode_chain_thunk *tme_recode_chain_thunk _TME_P((struct tme_recode_ic *, _tme_const struct tme_recode_chain *));

/* this fixes up a chain: */
tme_recode_thunk_off_t tme_recode_chain_fixup _TME_P((struct tme_recode_ic *, tme_recode_thunk_off_t, tme_uint32_t, tme_recode_thunk_off_t, tme_recode_thunk_off_t));

/* this returns an instructions thunk for an instructions group: */
tme_recode_thunk_off_t tme_recode_insns_thunk _TME_P((struct tme_recode_ic *, _tme_const struct tme_recode_insns_group *));

/* this invalidates an instructions thunk: */
void tme_recode_insns_thunk_invalidate _TME_P((struct tme_recode_ic *, tme_recode_thunk_off_t));

/* this invalidates all instructions thunks: */
void tme_recode_insns_thunk_invalidate_all _TME_P((struct tme_recode_ic *));

#ifdef TME_RECODE_DEBUG

/* this returns an opcode name: */
_tme_const char *tme_recode_opcode_dump _TME_P((unsigned int));

/* this dumps a tme_recode_uguest_t: */
void tme_recode_uguest_dump _TME_P((tme_recode_uguest_t));

/* this dumps a flags thunk: */
void tme_recode_flags_thunk_dump _TME_P((_tme_const struct tme_recode_ic *,
					 _tme_const struct tme_recode_flags_thunk *));

/* this dumps a conditions thunk: */
void tme_recode_conds_thunk_dump _TME_P((_tme_const struct tme_recode_ic *,
					 _tme_const struct tme_recode_conds_thunk *,
					 _tme_const char * _tme_const *));

/* this dumps a chain thunk: */
void tme_recode_chain_thunk_dump _TME_P((_tme_const struct tme_recode_ic *,
					 _tme_const struct tme_recode_chain_thunk *));

#endif /* TME_RECODE_DEBUG */

#endif /* _TME_RECODE_H */
