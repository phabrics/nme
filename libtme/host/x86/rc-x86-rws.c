/* $Id: rc-x86-rws.c,v 1.5 2010/06/05 19:14:40 fredette Exp $ */

/* libtme/host/x86/rc-x86-rws.c - x86 host recode reads and writes support: */

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

_TME_RCSID("$Id: rc-x86-rws.c,v 1.5 2010/06/05 19:14:40 fredette Exp $");

/* this emits instructions to do a byte swap in a host register: */
static tme_uint8_t *
_tme_recode_x86_rw_bswap(tme_uint8_t *thunk_bytes,
			 unsigned int size,
			 unsigned int reg_x86)
{
  unsigned int rex;

  /* NB: we don't have to worry about zero-truncation on an x86-64
     host; if this register is supposed to be sign-extended, we do
     that after all byte swapping: */

  /* if this is an eight-bit byte swap: */
  if (size == TME_RECODE_SIZE_8) {
    /* nothing to do */
  }

  /* otherwise, if this is a 16-bit byte swap: */
  else if (size == TME_RECODE_SIZE_16) {

    /* if this register has a high 8-bit encoding: */
    if (reg_x86 < TME_RECODE_X86_REG_SP) {

      /* emit an xchgb %regh, %regl: */
      thunk_bytes[0]
	= (TME_RECODE_X86_OPCODE_BINOP_XCHG
	   + TME_RECODE_X86_OPCODE_BINOP_Gb_Eb);
      thunk_bytes[1]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_REG((TME_RECODE_X86_REG_SP + reg_x86)));
      thunk_bytes += 2;
    }

    /* otherwise, this register doesn't have a high 8-bit encoding: */
    else {

      /* emit a rorw $8, %reg: */
      thunk_bytes[0] = TME_RECODE_X86_PREFIX_OPSIZ;
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[2]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_OPCODE_GRP2_ROR);
      thunk_bytes[3] = 8;
      thunk_bytes += 4;
    }
  }

  /* otherwise, this is a 32-bit swap, and/or a host-sized byte
     swap: */
  else {

    /* if the bswap instruction is available: */
    if (1) {

      /* emit a bswap %reg: */
      rex = TME_RECODE_X86_REX_R(size, reg_x86);
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_ESC_0F;
      thunk_bytes[1] = TME_RECODE_X86_OPCODE0F_BSWAP(reg_x86);
      thunk_bytes += 2;
    }

    /* otherwise, the bswap instruction is not available: */
    else {

      /* this must be a 32-bit swap: */
      assert (size == TME_RECODE_SIZE_32);

      /* emit:
	 rorw $8, %reg
	 rorl $16, %reg
	 rorw $8, %reg
      */
      thunk_bytes[0] = TME_RECODE_X86_PREFIX_OPSIZ;
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[2]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_OPCODE_GRP2_ROR);
      thunk_bytes[3] = 8;
      thunk_bytes += 4;
      thunk_bytes[0] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[1]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_OPCODE_GRP2_ROR);
      thunk_bytes[2] = 16;
      thunk_bytes += 3;
      thunk_bytes[0] = TME_RECODE_X86_PREFIX_OPSIZ;
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_GRP2_Ib_Ev;
      thunk_bytes[2]
	= TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(reg_x86),
				      TME_RECODE_X86_OPCODE_GRP2_ROR);
      thunk_bytes[3] = 8;
      thunk_bytes += 4;
    }
  }

  return (thunk_bytes);
}

/* this host function returns a new read/write thunk: */
struct tme_recode_rw_thunk *
tme_recode_host_rw_thunk_new(struct tme_recode_ic *ic,
			     const struct tme_recode_rw *rw)
{
  struct tme_recode_rw_thunk *rw_thunk;
  unsigned int max_boundaries_guest;
  unsigned int max_boundaries_host;
  struct tme_recode_x86_tlb_type x86_tlb_type;
  unsigned int reg_x86_address;
  unsigned int reg_host_value_0;
  unsigned int reg_host_value_1;
  struct tme_recode_insn insn_buffer;
  tme_uint8_t *thunk_bytes;
  unsigned int rex;
  unsigned int reg_host_value_orig;
  int stack_adjust;
  unsigned int reg_size;
  int memory_signed;

  /* start the new read/write thunk: */
  if (!tme_recode_host_thunk_start(ic)) {
    abort();
  }
  rw_thunk = tme_new(struct tme_recode_rw_thunk, 1);
  rw_thunk->tme_recode_x86_rw_thunk_subs
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);

  /* assume that we will always need to assist: */
  x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok = (tme_uint8_t *) NULL;
  x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp = (tme_uint8_t *) NULL;

  /* get the worst-case maximum number of guest bus boundaries this
     read/write could cross: */
  max_boundaries_guest
    = (rw->tme_recode_rw_bus_boundary == 0
       ? 0
       : (((TME_MAX(rw->tme_recode_rw_bus_boundary,
		    rw->tme_recode_rw_address_type.tme_recode_address_type_align_min)
	    - rw->tme_recode_rw_address_type.tme_recode_address_type_align_min)
	   + (TME_BIT(rw->tme_recode_rw_memory_size - TME_RECODE_SIZE_8)
	      - 1))
	  / rw->tme_recode_rw_bus_boundary));

  /* get the worst-case maximum number of host bus boundaries this
     read/write could cross: */
  max_boundaries_host
    = (((TME_MAX(TME_MEMORY_BUS_BOUNDARY,
		 rw->tme_recode_rw_address_type.tme_recode_address_type_align_min)
	 - rw->tme_recode_rw_address_type.tme_recode_address_type_align_min)
	+ (TME_BIT(rw->tme_recode_rw_memory_size - TME_RECODE_SIZE_8)
	   - 1))
       / TME_MEMORY_BUS_BOUNDARY);

  /* NB: as of 20080906, Intel's "Intel 64 Architecture Memory
     Ordering White Paper" (Order number 318147-001, from August 2007)
     only guarantees that size-aligned reads and writes up to 64 bits
     are atomic; we assume that non-Intel processors are similar.
     until this guarantee is extended to cover size-aligned 128-bit
     reads and writes, TME_MEMORY_BUS_BOUNDARY shouldn't be more than
     sizeof(tme_uint64_t).

     this means that we will always assist guest 128-bit reads
     and writes that might be atomic for the guest (since we will
     detect above that we might cross more bus boundaries than the
     guest might): */
      
  /* if threads are cooperative, or if common atomic operations aren't
     being done under software lock and we can't cross more boundaries
     than the guest would: */
  if (TME_THREADS_COOPERATIVE
      || (TME_MEMORY_ALIGNMENT_ATOMIC(TME_MEMORY_TYPE_COMMON) != 0
	  && max_boundaries_host <= max_boundaries_guest)) {

    /* get the TLB type for the address type: */
    tme_recode_address_type_tlb_type(ic,
				     &rw->tme_recode_rw_address_type,
				     &x86_tlb_type.tme_recode_tlb_type);

    /* XXX FIXME - document read/write thunk calling convention, how
       it differs from normal subs */

    /* for a double-host-size guest, the guest address is in the a:bp
       register pair, otherwise it's in the a register.  NB that we
       primarily deal with only a host-sized part: */
    reg_x86_address
      = tme_recode_x86_reg_from_host[_tme_recode_x86_tlb_reg_host_address(ic)];

    /* for a write, the value to write is in the first host register
       (pair).  for a read, the value read is returned in the same
       host register (pair) that was used for the address: */
    if (rw->tme_recode_rw_write) {
      reg_host_value_0 = TME_RECODE_REG_HOST(0);
    }
    else {
      reg_host_value_0
	= _tme_recode_x86_tlb_reg_host_address(ic);
      assert (tme_recode_x86_reg_from_host[reg_host_value_0] == reg_x86_address);
    }
    reg_host_value_1 = reg_host_value_0 + 1;

    /* find, busy, and check a data TLB entry: */
    _tme_recode_x86_tlb_busy(ic,
			     &rw->tme_recode_rw_address_type,
			     &x86_tlb_type);

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* if this is a write, and we need to byte-swap the value to
       write: */
    if (rw->tme_recode_rw_write
	&& rw->tme_recode_rw_memory_size > TME_RECODE_SIZE_8
	&& rw->tme_recode_rw_memory_endian != TME_ENDIAN_NATIVE) {

      /* we will byte-swap the value to write into (at least) the
	 TLB scratch register: */
      reg_host_value_orig = reg_host_value_0;
      assert (reg_host_value_1 == reg_host_value_0 + 1);
      reg_host_value_0 = TME_RECODE_REG_HOST_UNDEF;
      assert (tme_recode_x86_reg_from_host[reg_host_value_0] == TME_RECODE_X86_REG_TLB_SCRATCH);

      /* if this is a double-host-size write: */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_memory_size)) {

	/* we will use the most-significant half of the guest register
	   that held the address for the higher-in-memory (i.e., the
	   guest least-significant) part of the value.  NB that we are
	   swapping guest register halves here, too: */
	reg_host_value_1 = TME_RECODE_X86_REG_HOST_SUBS_SRC1 + 1;
	assert (tme_recode_x86_reg_from_host[reg_host_value_1 - 1] == reg_x86_address);
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[reg_host_value_orig + 0],
				      tme_recode_x86_reg_from_host[reg_host_value_1]);
	thunk_bytes
	  = _tme_recode_x86_rw_bswap(thunk_bytes,
				     TME_RECODE_SIZE_HOST,
				     tme_recode_x86_reg_from_host[reg_host_value_1]);
	reg_host_value_orig += 1;
      }

      /* copy and swap the lower-in-memory (i.e., the guest
	 most-significant) part of the value: */
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    tme_recode_x86_reg_from_host[reg_host_value_orig],
				    tme_recode_x86_reg_from_host[reg_host_value_0]);
      thunk_bytes
	= _tme_recode_x86_rw_bswap(thunk_bytes,
				   TME_MIN(rw->tme_recode_rw_memory_size,
					   TME_RECODE_SIZE_HOST),
				   tme_recode_x86_reg_from_host[reg_host_value_0]);
    }

    /* if threads aren't cooperative, and this is a double-host-size
       access: */
    if (!TME_THREADS_COOPERATIVE
	&& TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_memory_size)) {

      /* NB: in this case, we assume that the host is capable of SSE2
	 instructions.  this seems reasonable: */

      /* the x86-64 ABI requires that the stack pointer be 16-byte
	 aligned immediately before a call instruction.  inside an
	 insn thunk, the stack pointer is 16-byte aligned immediately
	 before a call to a read/write thunk, which means at the
	 beginning of a read/write thunk it is only 8-byte aligned
	 (because of the return address for the read/write thunk).

	 on x86-64, we want to use at least one movdqa, which requires
	 us to align the stack pointer: */
      stack_adjust
	= (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	   ? 0
	   : TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));

      /* if this is a write: */
      if (rw->tme_recode_rw_write) {

	/* do any stack pointer alignment: */
	if (stack_adjust) {
	  thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, -stack_adjust);
	}

	/* push the double-host-size value to write: */
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[reg_host_value_1]);
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[reg_host_value_0]);

	/* after the write, we will need to discard the value: */
	stack_adjust += TME_BIT(rw->tme_recode_rw_memory_size - TME_RECODE_SIZE_8);
      }

      /* otherwise, this is a read: */
      else {

	/* do any stack pointer alignment, and make space for the
	   double-host-size value to read: */
	thunk_bytes
	  = _tme_recode_x86_emit_adjust_sp(thunk_bytes, 
					   -(stack_adjust
					     + TME_BIT(rw->tme_recode_rw_memory_size
						       - TME_RECODE_SIZE_8)));
      }

      /* emit one of: 
	 movq (%esp), %xmm0
	 movdqa (%esp), %xmm0
	 movq (%address), %xmm0
	 movdqa (%address), %xmm0

	 to read the value to read or write into %xmm0:
      */
      thunk_bytes[0]
	= ((TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	    || (!rw->tme_recode_rw_write
		&& (rw->tme_recode_rw_address_type.tme_recode_address_type_align_min
		    < TME_BIT(rw->tme_recode_rw_memory_size - TME_RECODE_SIZE_8))))
	   ? TME_RECODE_X86_PREFIX_REP
	   : TME_RECODE_X86_PREFIX_OPSIZ);
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_ESC_0F;
      thunk_bytes[2]
	= (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	   ? TME_RECODE_X86_OPCODEF30F_MOVQ_Wq_Vq
	   : thunk_bytes[0] == TME_RECODE_X86_PREFIX_REP
	   ? TME_RECODE_X86_OPCODEF30F_MOVDQU_Wdq_Vdq
	   : TME_RECODE_X86_OPCODE660F_MOVDQA_Wdq_Vdq);
      if (rw->tme_recode_rw_write) {
	thunk_bytes[3]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),
					TME_RECODE_X86_REG(TME_RECODE_X86_REG_XMM(0)));
	thunk_bytes[4]
	  = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
	thunk_bytes += 5;
      }
      else {
	thunk_bytes[3]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(reg_x86_address),
					TME_RECODE_X86_REG(TME_RECODE_X86_REG_XMM(0)));
	thunk_bytes += 4;
      }

      /* emit one of:
	 movq %xmm0, (%address)
	 movdqu %xmm0, (%address)
	 movdqa %xmm0, (%address)
	 movq %xmm0, (%esp)
	 movdqa %xmm0, (%esp)

	 to write the value to read or write in %xmm0:
      */
      thunk_bytes[0]
	= ((TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	    || (rw->tme_recode_rw_write
		&& (rw->tme_recode_rw_address_type.tme_recode_address_type_align_min
		    >= TME_BIT(rw->tme_recode_rw_memory_size - TME_RECODE_SIZE_8))))
	   ? TME_RECODE_X86_PREFIX_OPSIZ
	   : TME_RECODE_X86_PREFIX_REP);
      thunk_bytes[1] = TME_RECODE_X86_OPCODE_ESC_0F;
      thunk_bytes[2]
	= (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32
	   ? TME_RECODE_X86_OPCODE660F_MOVQ_Vq_Wq
	   : thunk_bytes[0] == TME_RECODE_X86_PREFIX_REP
	   ? TME_RECODE_X86_OPCODEF30F_MOVDQU_Vdq_Wdq
	   : TME_RECODE_X86_OPCODE660F_MOVDQA_Vdq_Wdq);
      if (rw->tme_recode_rw_write) {
	thunk_bytes[3]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(reg_x86_address),
					TME_RECODE_X86_REG(TME_RECODE_X86_REG_XMM(0)));
	thunk_bytes += 4;
      }
      else {
	thunk_bytes[3]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(TME_RECODE_X86_EA_BASE_SIB),
					TME_RECODE_X86_REG(TME_RECODE_X86_REG_XMM(0)));
	thunk_bytes[4]
	  = TME_RECODE_X86_SIB(TME_RECODE_X86_REG_SP, TME_RECODE_X86_SIB_INDEX_NONE, 1);
	thunk_bytes += 5;
      }

      /* if this was a read: */
      if (!rw->tme_recode_rw_write) {

	/* pop the double-host-size value we read: */
	_tme_recode_x86_emit_reg_pop(thunk_bytes, tme_recode_x86_reg_from_host[reg_host_value_0]);
	_tme_recode_x86_emit_reg_pop(thunk_bytes, tme_recode_x86_reg_from_host[reg_host_value_1]);
      }

      /* discard any double-host-size value we wrote, and any stack
	 pointer alignment: */
      if (stack_adjust) {
	thunk_bytes = _tme_recode_x86_emit_adjust_sp(thunk_bytes, -stack_adjust);
      }
    }

    /* otherwise, either threads are cooperative, or this isn't a
       double-host-size access: */

    /* if this is a write: */
    else if (rw->tme_recode_rw_write) {

      /* if this is an ia32 host, and an 8-bit store of a register
	 that doesn't have an 8-bit encoding: */
      if (TME_RECODE_SIZE_HOST <= TME_RECODE_SIZE_32
	  && rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	  && tme_recode_x86_reg_from_host[reg_host_value_0] >= TME_RECODE_X86_REG_SP) {

	/* we will copy the value to write into the TLB scratch
	   register, which has an 8-bit encoding: */
	reg_host_value_orig = reg_host_value_0;
	reg_host_value_0 = TME_RECODE_REG_HOST_UNDEF;
	assert (tme_recode_x86_reg_from_host[reg_host_value_0] < TME_RECODE_X86_REG_SP);
	assert (tme_recode_x86_reg_from_host[reg_host_value_0] == TME_RECODE_X86_REG_TLB_SCRATCH);
	_tme_recode_x86_emit_reg_copy(thunk_bytes,
				      tme_recode_x86_reg_from_host[reg_host_value_orig],
				      tme_recode_x86_reg_from_host[reg_host_value_0]);
      }

      /* emit one of:
	 movb %reg, (%address)
	 movw %reg, (%address)
	 movl %reg, (%address)
	 movq %reg, (%address)
      */
      if (rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_16) {
	*(thunk_bytes++) = TME_RECODE_X86_PREFIX_OPSIZ;
      }
      rex
	= (TME_RECODE_X86_REX_R(TME_MIN(rw->tme_recode_rw_memory_size,
					TME_RECODE_SIZE_HOST),
				tme_recode_x86_reg_from_host[reg_host_value_0])
	   | TME_RECODE_X86_REX_B(0, reg_x86_address));
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }
      thunk_bytes[0]
	= (rw->tme_recode_rw_memory_size >= TME_RECODE_SIZE_16
	   ? (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev)
	   : (TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gb_Eb));
      /* NB: a disp8 EA must be used when the base register is bp or r13: */
      if (TME_RECODE_X86_REG(reg_x86_address) == TME_RECODE_X86_REG_BP) {
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_0]));
	thunk_bytes[2] = 0;
	thunk_bytes += 3;
      }
      else {
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_0]));
	thunk_bytes += 2;
      }

      /* if this is a double-host-size write: */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_memory_size)) {

	/* emit one of:
	   movl %reg, 4(%address)
	   movq %reg, 8(%address)
	*/
	rex
	  = (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,
				  tme_recode_x86_reg_from_host[reg_host_value_1])
	   | TME_RECODE_X86_REX_B(0, reg_x86_address));
	if (rex != 0) {
	  *(thunk_bytes++) = rex;
	}
	thunk_bytes[0] = TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev;
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_1]));
	thunk_bytes[2] = TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
	thunk_bytes += 3;
      }
    }

    /* otherwise, this is a read: */
    else {

      /* if this is a double-host-size read: */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_memory_size)) {

	/* emit one of:
	   movl 4(%address), %reg
	   movq 8(%address), %reg
	*/
	rex
	  = (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_HOST,
				  tme_recode_x86_reg_from_host[reg_host_value_1])
	     | TME_RECODE_X86_REX_B(0, reg_x86_address));
	if (rex != 0) {
	  *(thunk_bytes++) = rex;
	}
	thunk_bytes[0] = TME_RECODE_X86_OPCODE_BINOP_MOV + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv;
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_1]));
	thunk_bytes[2] = TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
	thunk_bytes += 3;
      }

      /* assume that we we will read into all of a host register: */
      reg_size = TME_MAX(rw->tme_recode_rw_reg_size, TME_RECODE_SIZE_HOST);

      /* by default, in the read instruction itself we zero-extend the
	 value into all of a host register.  the only time we will
	 sign-extend in the read instruction itself is when the
	 register size is greater than the memory size, memory is
	 signed, and we're only reading a byte or the guest's byte
	 order matches the host.  if the first two are true, but we're
	 reading more than a byte and the guest's byte order doesn't
	 match the host, we have to wait to do the sign extension
	 after we've byte swapped the value read: */
      memory_signed
	= (rw->tme_recode_rw_reg_size > rw->tme_recode_rw_memory_size
	   && rw->tme_recode_rw_memory_signed
	   && (rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	       || rw->tme_recode_rw_memory_endian == TME_ENDIAN_NATIVE));

      /* if this is an x86-64 host and a 32-bit sign- or zero-extended load: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
	  && rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_32
	  && rw->tme_recode_rw_reg_size > rw->tme_recode_rw_memory_size) {

	/* if this is a zero-extended load, or if the guest's byte
	   order doesn't match the host: */
	if (!memory_signed) {

	  /* read into only the least-significant 32 bits of the
	     register.  this will zero-extend the read to all 64
	     bits.  this should prevent a rex prefix: */
	  reg_size = TME_RECODE_SIZE_32;
	}
      }

      /* emit any rex prefix: */
      rex
	= (TME_RECODE_X86_REX_B(0, reg_x86_address)
	   | TME_RECODE_X86_REX_R(TME_MIN(reg_size,
					  TME_RECODE_SIZE_HOST),
				  tme_recode_x86_reg_from_host[reg_host_value_0]));
      if (rex != 0) {
	*(thunk_bytes++) = rex;
      }

      /* if this is an x86-64 host and a 32-bit sign-extended load: */
      if (TME_RECODE_SIZE_HOST > TME_RECODE_SIZE_32
	  && rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_32
	  && reg_size > TME_RECODE_SIZE_32) {

	/* emit the opcode part of a movslq (%address), %reg: */
	thunk_bytes[0] = TME_RECODE_X86_OPCODE_MOVS_El_Gv;
      }

      /* otherwise, if this is an 8- or 16-bit load: */
      else if (rw->tme_recode_rw_memory_size <= TME_RECODE_SIZE_16) {

	/* emit the opcode part of one of:
	   movsb (%address), %reg
	   movzb (%address), %reg
	   movsw (%address), %reg
	   movzw (%address), %reg
	*/
	*(thunk_bytes++) = TME_RECODE_X86_OPCODE_ESC_0F;
	thunk_bytes[0]
	  = (rw->tme_recode_rw_memory_size == TME_RECODE_SIZE_8
	     ? (memory_signed
		? TME_RECODE_X86_OPCODE0F_MOVS_Eb_Gv
		: TME_RECODE_X86_OPCODE0F_MOVZ_Eb_Gv)
	     : (memory_signed
		? TME_RECODE_X86_OPCODE0F_MOVS_Ew_Gv
		: TME_RECODE_X86_OPCODE0F_MOVZ_Ew_Gv));
      }

      /* otherwise, this load is double-host-size, or host-size, or a
	 32-bit zero-extended load on an x86-64 host: */
      else {

	/* emit the opcode part of a movl (%address), %reg or a movq (%address), %reg: */
	thunk_bytes[0]
	  = (TME_RECODE_X86_OPCODE_BINOP_MOV
	     + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv);
      }

      /* emit the modR/M byte for this instruction: */
      /* NB: a disp8 EA must be used when the base register is bp or r13: */
      if (TME_RECODE_X86_REG(reg_x86_address) == TME_RECODE_X86_REG_BP) {
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA_DISP8(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_0]));
	thunk_bytes[2] = 0;
	thunk_bytes += 3;
      }
      else {
	thunk_bytes[1]
	  = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_EA(reg_x86_address),
					TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_0]));
	thunk_bytes += 2;
      }
    }

    /* if this is a read: */
    if (!rw->tme_recode_rw_write) {

      /* if we need to byte-swap the value read: */
      if (rw->tme_recode_rw_memory_size > TME_RECODE_SIZE_8
	  && rw->tme_recode_rw_memory_endian != TME_ENDIAN_NATIVE) {

	/* if this is a double-host-size read: */
	if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_memory_size)) {

	  /* swap the guest register halves: */
	  _tme_recode_x86_emit_reg_binop(thunk_bytes,
					 TME_RECODE_X86_OPCODE_BINOP_XCHG,
					 tme_recode_x86_reg_from_host[reg_host_value_0],
					 tme_recode_x86_reg_from_host[reg_host_value_1]);

	  /* byte-swap the most-significant half of the guest register: */
	  thunk_bytes
	    = _tme_recode_x86_rw_bswap(thunk_bytes,
				       TME_RECODE_SIZE_HOST,
				       tme_recode_x86_reg_from_host[reg_host_value_1]);
	}

	/* byte-swap the (least-significant half of the) guest register: */
	thunk_bytes
	  = _tme_recode_x86_rw_bswap(thunk_bytes,
				     TME_MIN(rw->tme_recode_rw_memory_size,
					     TME_RECODE_SIZE_HOST),
				     tme_recode_x86_reg_from_host[reg_host_value_0]);

	/* if the read is sign-extended from smaller than host-sized: */
	if (rw->tme_recode_rw_reg_size > rw->tme_recode_rw_memory_size
	    && rw->tme_recode_rw_memory_signed
	    && rw->tme_recode_rw_memory_size < TME_RECODE_SIZE_HOST) {

	  /* sign-extend the value read to host size: */
	  tme_recode_x86_insns_finish(ic, thunk_bytes);
	  insn_buffer.tme_recode_insn_opcode = TME_RECODE_OPCODE_EXTS;
	  insn_buffer.tme_recode_insn_operand_src[0] = reg_host_value_0;
	  insn_buffer.tme_recode_insn_operand_src[1] = rw->tme_recode_rw_memory_size;
	  insn_buffer.tme_recode_insn_operand_dst = reg_host_value_0;
	  _tme_recode_x86_insn_ext(ic, &insn_buffer);
	  tme_recode_x86_insns_start(ic, thunk_bytes);
	}
      }

      /* if this is a double-host-size read that needs zero- or sign-extension: */
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_reg_size)
	  && rw->tme_recode_rw_reg_size > rw->tme_recode_rw_memory_size) {

	/* if memory is signed: */
	if (rw->tme_recode_rw_memory_signed) {

	  /* sign-extend the value read: */
	  _tme_recode_x86_emit_reg_binop(thunk_bytes,
					 TME_RECODE_X86_OPCODE_BINOP_MOV,
					 tme_recode_x86_reg_from_host[reg_host_value_0],
					 tme_recode_x86_reg_from_host[reg_host_value_1]);
	  _tme_recode_x86_emit_reg_binop(thunk_bytes,
					 TME_RECODE_X86_OPCODE_BINOP_ADD,
					 tme_recode_x86_reg_from_host[reg_host_value_0],
					 tme_recode_x86_reg_from_host[reg_host_value_1]);
	  _tme_recode_x86_emit_reg_binop(thunk_bytes,
					 TME_RECODE_X86_OPCODE_BINOP_SBB,
					 tme_recode_x86_reg_from_host[reg_host_value_1],
					 tme_recode_x86_reg_from_host[reg_host_value_1]);
	}

	/* otherwise, memory is unsigned: */
	else {

	  /* zero-extend the value read: */
	  /* NB: we always make this a 32-bit operation, to try to
	     prevent a rex prefix: */
	  rex = (TME_RECODE_X86_REX_R(TME_RECODE_SIZE_32,
				      tme_recode_x86_reg_from_host[reg_host_value_1])
		 | TME_RECODE_X86_REX_B(TME_RECODE_SIZE_32,
					tme_recode_x86_reg_from_host[reg_host_value_1]));
	  if (rex != 0) {
	    *(thunk_bytes++) = rex;
	  }
	  thunk_bytes[0]
	    = (TME_RECODE_X86_OPCODE_BINOP_XOR
	       + TME_RECODE_X86_OPCODE_BINOP_Gv_Ev);
	  thunk_bytes[1]
	    = TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(tme_recode_x86_reg_from_host[reg_host_value_1]),
					  TME_RECODE_X86_REG(tme_recode_x86_reg_from_host[reg_host_value_1]));
	  thunk_bytes += 2;
	}
      }
    }

    /* finish these instructions: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);

    /* unbusy the TLB entry: */
    _tme_recode_x86_tlb_unbusy(ic,
			       x86_tlb_type.tme_recode_tlb_type.tme_recode_tlb_type_offset_token);

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* return to the instructions thunk: */
    *(thunk_bytes++) = TME_RECODE_X86_OPCODE_RET;

    /* finish these instructions: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  if (x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp != NULL) {

    /* finish the assist conditional jump above, now
       that we are at the target: */
    _tme_recode_x86_fixup_jmp(x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp,
			      ic->tme_recode_ic_thunk_build_next);

    /* start more instructions: */
    tme_recode_x86_insns_start(ic, thunk_bytes);

    /* exclusive-or the TLB entry page offset with the
       (least-significant half of the) TLB entry page, to convert the
       TLB entry page offset back into the (least-significant half of
       the) guest address: */
    thunk_bytes
      = _tme_recode_x86_tlb_ref(thunk_bytes,
				TME_RECODE_SIZE_HOST,
				(TME_RECODE_X86_OPCODE_BINOP_XOR
				 + TME_RECODE_X86_OPCODE_BINOP_Ev_Gv),
				TME_RECODE_X86_REG_TLB,
				x86_tlb_type.tme_recode_tlb_type.tme_recode_tlb_type_offset_page,
				TME_RECODE_X86_REG(reg_x86_address));

    /* finish these instructions: */
    tme_recode_x86_insns_finish(ic, thunk_bytes);
  }

  /* fix up any double-host-size address assist: */
  if (x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok != NULL) {
    _tme_recode_x86_fixup_jmp(x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok,
			      ic->tme_recode_ic_thunk_build_next);
  }

  if (x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp_address_ok != NULL
      || x86_tlb_type.tme_recode_x86_tlb_type_assist_jmp != NULL) {

    /* unbusy the TLB entry: */
    _tme_recode_x86_tlb_unbusy(ic,
			       x86_tlb_type.tme_recode_tlb_type.tme_recode_tlb_type_offset_token);
  }

  /* start more instructions: */
  tme_recode_x86_insns_start(ic, thunk_bytes);

  /* if this is an ia32 host: */
  if (TME_RECODE_SIZE_HOST == TME_RECODE_SIZE_32) {

    /* if this is a write: */
    if (rw->tme_recode_rw_write) {

      /* push the value argument for the guest function.  NB that if
	 double-host-size guests are supported, but this isn't a
	 double-host-size guest, we use a garbage word on the stack as
	 the most-significant half of this argument (which is okay
	 since the guest functions are supposed to truncate their
	 arguments to the expected size): */
      if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
	_tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_REG_HOST(0) + 1]);
      }
      _tme_recode_x86_emit_reg_push(thunk_bytes, tme_recode_x86_reg_from_host[TME_RECODE_REG_HOST(0)]);
    }

    /* push the address argument for the guest function.  NB that if
       double-host-size guests are supported, but this isn't a
       double-host-size guest, we use a garbage word on the stack as
       the most-significant half of this argument (which is okay since
       the guest functions are supposed to truncate their arguments to
       the expected size): */
    if (TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_HOST) {
      _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_A);
    }
    _tme_recode_x86_emit_reg_push(thunk_bytes, reg_x86_address);

    /* emit the instruction to push the struct tme_ic * argument for
       the guest function, and then the call instruction to the guest
       function: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_PUSH_Gv(TME_RECODE_X86_REG_IC)
	 + (TME_RECODE_X86_OPCODE_CALL_RELz << 8));
    thunk_bytes += 2 + sizeof(tme_uint32_t);
    ((tme_int32_t *) thunk_bytes)[-1]
      = (tme_recode_function_to_thunk_off(ic, rw->tme_recode_rw_guest_func_read)
	 - tme_recode_build_to_thunk_off(ic, thunk_bytes));

    /* remove the guest function arguments from the stack: */
    thunk_bytes
      = _tme_recode_x86_emit_adjust_sp(thunk_bytes, 
				       (sizeof(struct tme_ic *)
					+ sizeof(tme_recode_uguest_t)
					+ (sizeof(tme_recode_uguest_t)
					   * !!rw->tme_recode_rw_write)));
  }

  /* otherwise, this is an x86-64 host: */
  else {

    /* push the caller-saved registers that aren't normally destroyed
       by a read/write thunk: */
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(10));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(11));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_DI);
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_SI);
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(8));
    _tme_recode_x86_emit_reg_push(thunk_bytes, TME_RECODE_X86_REG_N(9));

    /* make the struct tme_ic * argument for the guest function: */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_IC, TME_RECODE_X86_REG_DI);

    /* make the address argument for the guest function.  NB that if
       double-host-size guests are supported, but this isn't a
       double-host-size guest, we use a garbage word as the
       most-significant half of this argument (which is okay since the
       guest functions are supposed to truncate their arguments to the
       expected size): */
    _tme_recode_x86_emit_reg_copy(thunk_bytes, reg_x86_address, TME_RECODE_X86_REG_SI);
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
      _tme_recode_x86_emit_reg_copy(thunk_bytes, TME_RECODE_X86_REG_A, TME_RECODE_X86_REG_D);
    }

    /* if this is a write: */
    if (rw->tme_recode_rw_write) {

      /* make the value argument for the guest function.  NB that if
	 double-host-size guests are supported, but this isn't a
	 double-host-size guest, we use a garbage word as the
	 most-significant half of this argument (which is okay since
	 the guest functions are supposed to truncate their arguments
	 to the expected size): */
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    tme_recode_x86_reg_from_host[TME_RECODE_REG_HOST(0)],
				    (TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_HOST
				     ? TME_RECODE_X86_REG_D
				     : TME_RECODE_X86_REG_C));
      if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {
	_tme_recode_x86_emit_reg_copy(thunk_bytes, 
				      tme_recode_x86_reg_from_host[TME_RECODE_REG_HOST(0)],
				      TME_RECODE_X86_REG_N(8));
      }
    }

    /* we must assume that we can't reach the guest function from the
       instruction thunk with a 32-bit displacement.  emit a direct
       call to the guest function using %rax: */
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_REX_B(TME_RECODE_SIZE_HOST, TME_RECODE_X86_REG_A)
	 + (TME_RECODE_X86_OPCODE_MOV_Iv_Gv(TME_RECODE_X86_REG_A)
	    << 8));
    memcpy(thunk_bytes + 2,
	   &rw->tme_recode_rw_guest_func_write,
	   TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8));
    thunk_bytes += 2 + TME_BIT(TME_RECODE_SIZE_HOST - TME_RECODE_SIZE_8);
    *((tme_uint16_t *) thunk_bytes)
      = (TME_RECODE_X86_OPCODE_GRP5
	 + (TME_RECODE_X86_MOD_OPREG_RM(TME_RECODE_X86_MOD_RM_REG(TME_RECODE_X86_REG_A),
					TME_RECODE_X86_OPCODE_GRP5_CALL)
	    << 8));
    thunk_bytes += 2;

    /* pop the caller-saved registers that aren't normally destroyed
       by a read/write thunk: */
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(9));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(8));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_SI);
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_DI);
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(11));
    _tme_recode_x86_emit_reg_pop(thunk_bytes, TME_RECODE_X86_REG_N(10));
  }

  /* if this is a read: */
  if (!rw->tme_recode_rw_write) {

    /* if this is a double-host-size guest: */
    if (TME_RECODE_SIZE_IS_DOUBLE_HOST(ic->tme_recode_ic_reg_size)) {

      /* move the value read into the expected return registers: */
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    TME_RECODE_X86_REG_A,
				    tme_recode_x86_reg_from_host[reg_host_value_0]);
      _tme_recode_x86_emit_reg_copy(thunk_bytes,
				    TME_RECODE_X86_REG_D,
				    tme_recode_x86_reg_from_host[reg_host_value_1]);
    }

    /* otherwise, this is not a double-host-size guest: */
    else {

      /* the value read should already be in the expected register: */
      assert (tme_recode_x86_reg_from_host[reg_host_value_0] == TME_RECODE_X86_REG_A);
    }
  }

  /* return to the instructions thunk: */
  *(thunk_bytes++) = TME_RECODE_X86_OPCODE_RET;

  /* finish these instructions: */
  tme_recode_x86_insns_finish(ic, thunk_bytes);

  /* finish this read/write thunk: */
  tme_recode_host_thunk_finish(ic);

  /* no further extension is needed for this read/write thunk: */
  rw_thunk->tme_recode_x86_rw_thunk_extend_size = 0;

  return (rw_thunk);
}

/* this host function tries to duplicate a read/write thunk: */
struct tme_recode_rw_thunk *
tme_recode_host_rw_thunk_dup(struct tme_recode_ic *ic,
			     const struct tme_recode_rw *rw,
			     const struct tme_recode_rw *rw_other)
{
  tme_uint8_t *thunk_bytes_0;
  struct tme_recode_insn insn_buffer;
  tme_uint8_t *thunk_bytes_1;
  struct tme_recode_rw_thunk *rw_thunk;

  /* start more instructions, so if we use _tme_recode_x86_insn_ext()
     to emit an extension instruction, we can discard it from the
     thunk build memory: */
  tme_recode_x86_insns_start(ic, thunk_bytes_0);

  /* if our register size is the same as the memory size, or
     if the existing read/write thunk will do the extension that
     we need to at least our register size: */
  /* NB: a read/write thunk always extends to at least host size, and
     when no particular extension is explicitly required,
     zero-extension is the default: */
  if ((rw->tme_recode_rw_reg_size
       == rw_other->tme_recode_rw_memory_size)
      || ((((rw_other->tme_recode_rw_reg_size
	     > rw_other->tme_recode_rw_memory_size)
	    && rw_other->tme_recode_rw_memory_signed)
	   == !!rw->tme_recode_rw_memory_signed)
	  && (TME_MAX(rw_other->tme_recode_rw_reg_size, TME_RECODE_SIZE_HOST)
	      >= rw->tme_recode_rw_reg_size))) {

    /* we can reuse the existing read/write thunk, and we don't need
       to do any extension: */
    /* nothing to do */
  }

  /* otherwise, if this is a double-host-size read: */
  else if (TME_RECODE_SIZE_IS_DOUBLE_HOST(rw->tme_recode_rw_reg_size)) {

    /* we won't reuse the existing read/write thunk: */
    return (NULL);
  }

  /* otherwise, we will reuse this read/write thunk, with a single
     zero- or sign-extension instruction after each call: */
  else {

    /* use _tme_recode_x86_insn_ext() to emit the extension
       instruction: */
    insn_buffer.tme_recode_insn_opcode
      = (rw->tme_recode_rw_memory_signed
	 ? TME_RECODE_OPCODE_EXTS
	 : TME_RECODE_OPCODE_EXTZ);
    insn_buffer.tme_recode_insn_operand_src[0] = TME_RECODE_X86_REG_HOST_FREE_CALL;
    insn_buffer.tme_recode_insn_operand_src[1] = rw->tme_recode_rw_memory_size;
    insn_buffer.tme_recode_insn_operand_dst = insn_buffer.tme_recode_insn_operand_src[0];
    _tme_recode_x86_insn_ext(ic, &insn_buffer);
  }

  /* duplicate the read/write thunk: */
  rw_thunk = tme_dup(struct tme_recode_rw_thunk, rw_other->tme_recode_rw_thunk, 1);

  /* get any extension instruction from the thunk build memory, and
     then discard it: */
  tme_recode_x86_insns_start(ic, thunk_bytes_1);
  rw_thunk->tme_recode_x86_rw_thunk_extend = *((tme_uint32_t *) thunk_bytes_0);
  rw_thunk->tme_recode_x86_rw_thunk_extend_size = (thunk_bytes_1 - thunk_bytes_0);
  assert (rw_thunk->tme_recode_x86_rw_thunk_extend_size <= sizeof(tme_uint32_t));
  tme_recode_x86_insns_finish(ic, thunk_bytes_0);

  /* return the duplicated read/write thunk: */
  return (rw_thunk);
}
