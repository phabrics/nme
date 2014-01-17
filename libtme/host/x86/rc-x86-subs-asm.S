/* $Id: rc-x86-subs-asm.S,v 1.2 2009/09/07 15:25:23 fredette Exp $ */

/* libtme/host/x86/rc-x86-subs-asm.S - hand-coded x86 host recode subs: */

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

	.text

	.ascii "$Id: rc-x86-subs-asm.S,v 1.2 2009/09/07 15:25:23 fredette Exp $"

# concatenation:
#
#if ((defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)) && !defined(UNIXCPP)) || defined(ANSICPP)
#define __TME_CONCAT(a,b) a ## b
#define _TME_CONCAT(a,b) __TME_CONCAT(a,b)
#else
#define _TME_CONCAT(a,b) a/**/b
#endif

#ifdef __x86_64__
#define TME_RECODE_SIZE_HOST			6
#define TME_RECODE_BITS_HOST(x)			_TME_CONCAT(x, 64)
#define TME_RECODE_BITS_DOUBLE_HOST(x)		_TME_CONCAT(x, 128)
#define TME_RECODE_X86_OPN(x)			_TME_CONCAT(x, q)
#define TME_RECODE_X86_REGN(x)			_TME_CONCAT(%r, x)
#define TME_RECODE_X86_REG_HOST_SUBS_DST_N	%r12
#define TME_RECODE_X86_REG_HOST_SUBS_DST_L	%r12d
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_N	%rbp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_L	%ebp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_W	%bp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_B	%bpl
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_P1_N	%rax
#else  /* !__x86_64__ */
#define TME_RECODE_SIZE_HOST			5
#define TME_RECODE_BITS_HOST(x)			_TME_CONCAT(x, 32)
#define TME_RECODE_BITS_DOUBLE_HOST(x)		_TME_CONCAT(x, 64)
#define TME_RECODE_X86_OPN(x)			_TME_CONCAT(x, l)
#define TME_RECODE_X86_REGN(x)			_TME_CONCAT(%e, x)
#define TME_RECODE_X86_REG_HOST_SUBS_DST_N	%edi
#define TME_RECODE_X86_REG_HOST_SUBS_DST_L	%edi
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_N	%ebp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_L	%ebp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_W	%bp
#define TME_RECODE_X86_REG_HOST_SUBS_SRC1_P1_N	%eax
#endif /* !__x86_64__ */

	# this macro does a double-host-size shift:
	#
.macro	tme_recode_x86_double_shift name shift shiftd reg_first reg_second arith=0
	.align	16
	.globl	\name
\name:

	# branch if the most-significant half of the shift count in
	# src1 is nonzero, swapping the least-significant half of the
	# shift count in src1 with the scratch c register in the
	# meantime.  we will swap them back after the shift.
	#
	# NB: we do the swaps with the c register instead of just
	# overwriting it, to cooperate with subs that keep the
	# most-significant half of src0 in the c register:	
	#
	TME_RECODE_X86_OPN(test)	TME_RECODE_X86_REG_HOST_SUBS_SRC1_P1_N, TME_RECODE_X86_REG_HOST_SUBS_SRC1_P1_N
	TME_RECODE_X86_OPN(xchg)	TME_RECODE_X86_REG_HOST_SUBS_SRC1_N, TME_RECODE_X86_REGN(cx)
	jnz	.L_double_shift_2_\@

	# branch if the shift count is greater than the host size:
	#
	TME_RECODE_X86_OPN(cmp)		$(1 << TME_RECODE_SIZE_HOST), TME_RECODE_X86_REGN(cx)
	jae	.L_double_shift_1_\@

	# do the double-precision shift:
	# 
	\shiftd				%cl, \reg_second, \reg_first
	\shift				%cl, \reg_second
	TME_RECODE_X86_OPN(xchg)	TME_RECODE_X86_REG_HOST_SUBS_SRC1_N, TME_RECODE_X86_REGN(cx)
	ret

.L_double_shift_1_\@:

	# branch if the shift count is greater than or equal to the
	# double-host size:
	#
	TME_RECODE_X86_OPN(cmp)		$2*(1 << TME_RECODE_SIZE_HOST), TME_RECODE_X86_REGN(cx)
	jae	.L_double_shift_2_\@
	
	# first do a host-size shift by doing a register move and
	# then a clear for a logical shift, or a copy of the most
	# significant bit of the second register down into all other
	# bits:
	#
	TME_RECODE_X86_OPN(mov)		\reg_second, \reg_first
.if \arith
	TME_RECODE_X86_OPN(shl)		$1, \reg_second
	TME_RECODE_X86_OPN(sbb)		\reg_second, \reg_second
.else
	TME_RECODE_X86_OPN(xor)		\reg_second, \reg_second
.endif

	# do the remainder of the shift as a host-size shift (which
	# masks the count in %cl to the host size):
	#
	\shift				%cl, \reg_first
	TME_RECODE_X86_OPN(xchg)	TME_RECODE_X86_REG_HOST_SUBS_SRC1_N, TME_RECODE_X86_REGN(cx)
	ret

	# the shift count is greater than the double-host-size.  for a
	# logical shift, clear both registers.  for an arithmetic shift,
	# copy the most significant bit of the second register down into
	# all bits in both registers:
	#
.L_double_shift_2_\@:
.if \arith
	TME_RECODE_X86_OPN(shl)		$1, \reg_second
	TME_RECODE_X86_OPN(sbb)		\reg_second, \reg_second
	TME_RECODE_X86_OPN(mov)		\reg_second, \reg_first
.else
	TME_RECODE_X86_OPN(xor)		\reg_first, \reg_first
	TME_RECODE_X86_OPN(xor)		\reg_second, \reg_second
.endif
	TME_RECODE_X86_OPN(xchg)	TME_RECODE_X86_REG_HOST_SUBS_SRC1_N, TME_RECODE_X86_REGN(cx)
	ret
.endm

	# this macro does a host-size or smaller shift:
	#
.macro	tme_recode_x86_shift insn size shift
	.align	16
	.globl	tme_recode_x86_\insn\size
tme_recode_x86_\insn\size:

	# _if this is a right shift smaller than host size, first
	# zero-extend or sign-extend the destination to host size,
	# so we can do a 32-bit shift, or branch to
	# _tme_recode_x86_shift_arithmetic_all (which assumes that
	# the destination is host size):
	#
.ifnc \insn,shll
.if \size < TME_RECODE_BITS_HOST(/**/)

	# _if this is an 8-bit shift on an ia32 host, we can't encode
	# %dil for a movzbl or movsbl, so we do an and for a movzbl and
	# a movsbl through the c register:
	#
.ifeq (TME_RECODE_BITS_HOST(/**/) - 32) | (\size - 8)
.ifc \insn,shra
	movl	%edi, %ecx
	movsbl	%cl, %edi
.else
	andl	$0xff, %edi
.endif
.else

	# otherwise, emit a movz or movs instruction to extend
	# the destination in TME_RECODE_X86_REG_HOST_SUBS_DST:
	#
.ifeq \size - 32
	# the x86_64 32-bit extensions are different:
	#
.ifc \insn,shra
	movslq	TME_RECODE_X86_REG_HOST_SUBS_DST_L, TME_RECODE_X86_REG_HOST_SUBS_DST_N
.else
	movl	TME_RECODE_X86_REG_HOST_SUBS_DST_L, TME_RECODE_X86_REG_HOST_SUBS_DST_L
.endif
.else
#ifdef __x86_64__
	.byte	0x48 + (1 << 0) + (1 << 2) # TME_RECODE_X86_REX_R(TME_RECODE_SIZE_64, %r12) + TME_RECODE_X86_REX_B(0, %r12)
#endif /* __x86_64__ */
	.byte	0x0f			# TME_RECODE_X86_OPCODE_ESC_0F
.ifc \insn,shra
	.byte	0xbf - ((\size / 8) & 1)	# TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv or TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv
.else
	.byte	0xb7 - ((\size / 8) & 1)	# TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv or TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv
.endif
#ifdef __x86_64__
	.byte	(0xc0 + (12 % 8)) + ((12 % 8) << 3) # %r12, %r12
#else  /* !__x86_64__ */
	.byte	(0xc0 + 7) + (7 << 3) # %edi, %edi
#endif /* __x86_64__ */
.endif
.endif
.endif
.endif

	# compare the shift count in TME_RECODE_X86_REG_HOST_SUBS_SRC1
	# to the size:
	#
.ifeq \size - 64
	cmpq	$\size, TME_RECODE_X86_REG_HOST_SUBS_SRC1_N
.else
.ifeq \size - 32
	cmpl	$\size, TME_RECODE_X86_REG_HOST_SUBS_SRC1_L
.else
.ifeq \size - 16
	cmpw	$\size, TME_RECODE_X86_REG_HOST_SUBS_SRC1_W
.else
.ifeq \size - 8
#ifdef TME_RECODE_X86_REG_HOST_SUBS_SRC1_B
	cmpb	$\size, TME_RECODE_X86_REG_HOST_SUBS_SRC1_B
#else
	movl	TME_RECODE_X86_REG_HOST_SUBS_SRC1_L, %ecx
	cmpb	$\size, %cl
#endif
.endif
.endif
.endif
.endif

	# put the shift count into the c register.  this has already
	# been done if this is an 8-bit shift on an ia32 host:
	#
.ifne (TME_RECODE_BITS_HOST(/**/) - 32) | (\size - 8)
	movl	TME_RECODE_X86_REG_HOST_SUBS_SRC1_L, %ecx
.endif

	# _if the shift count is greater than or equal to the size,
	# for an arithmetic shift copy the most-significant bit down
	# into all other bits, otherwise do a clear:
	#
.ifc \insn,shra
	jae	_tme_recode_x86_shift_arithmetic_all
.else
	jae	_tme_recode_x86_shift_logical_all
.endif

	# otherwise, do the shift:
	#
.ifeq (\size - 64)
	\shift	%cl, TME_RECODE_X86_REG_HOST_SUBS_DST_N
.else
	\shift	%cl, TME_RECODE_X86_REG_HOST_SUBS_DST_L
.endif
	ret
.endm

	# the shifts:
	#
#ifdef __x86_64__
tme_recode_x86_double_shift tme_recode_x86_shll128 shlq shldq %r13 %r12
tme_recode_x86_double_shift tme_recode_x86_shrl128 shrq shrdq %r12 %r13
tme_recode_x86_double_shift tme_recode_x86_shra128 sarq shrdq %r12 %r13 1
tme_recode_x86_shift shll 64 shlq
tme_recode_x86_shift shrl 64 shrq
tme_recode_x86_shift shra 64 sarq
#else  /* !__x86_64__ */
tme_recode_x86_double_shift tme_recode_x86_shll64 shll shldl %esi %edi
tme_recode_x86_double_shift tme_recode_x86_shrl64 shrl shrdl %edi %esi
tme_recode_x86_double_shift tme_recode_x86_shra64 sarl shrdl %edi %esi 1
#endif /* !__x86_64__ */
tme_recode_x86_shift shll 32 shll
tme_recode_x86_shift shrl 32 shrl
tme_recode_x86_shift shra 32 sarl
tme_recode_x86_shift shll 16 shll
tme_recode_x86_shift shrl 16 shrl
tme_recode_x86_shift shra 16 sarl
tme_recode_x86_shift shll 8 shll
tme_recode_x86_shift shrl 8 shrl
tme_recode_x86_shift shra 8 sarl

_tme_recode_x86_shift_arithmetic_all:
	TME_RECODE_X86_OPN(add)		TME_RECODE_X86_REG_HOST_SUBS_DST_N, TME_RECODE_X86_REG_HOST_SUBS_DST_N
	TME_RECODE_X86_OPN(sbb)		TME_RECODE_X86_REG_HOST_SUBS_DST_N, TME_RECODE_X86_REG_HOST_SUBS_DST_N
	ret

_tme_recode_x86_shift_logical_all:
	TME_RECODE_X86_OPN(xor)		TME_RECODE_X86_REG_HOST_SUBS_DST_N, TME_RECODE_X86_REG_HOST_SUBS_DST_N
	ret
