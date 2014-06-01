/* $Id: m68020.c,v 1.7 2007/02/16 01:40:56 fredette Exp $ */

/* ic/m68k/m68020.c - implementation of Motorola 68020 emulation: */

/*
 * Copyright (c) 2002, 2003, 2004 Matt Fredette
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
_TME_RCSID("$Id: m68020.c,v 1.7 2007/02/16 01:40:56 fredette Exp $");

/* includes: */
#include "m68k-impl.h"

/* macros: */
#define TME_M68K_SSWB_FC	(0x8000)
#define TME_M68K_SSWB_FB	(0x4000)
#define TME_M68K_SSWB_RC	(0x2000)
#define TME_M68K_SSWB_RB	(0x1000)
#define TME_M68K_SSWB_DF	(0x0100)
#define TME_M68K_SSWB_RM	(0x0080)
#define TME_M68K_SSWB_RW	(0x0040)
#define TME_M68K_SSWB_SIZE_MASK	(0x0030)
#define  TME_M68K_SSWB_SIZE_4	(0x0000)
#define  TME_M68K_SSWB_SIZE_1	(0x0010)
#define  TME_M68K_SSWB_SIZE_2	(0x0020)
#define  TME_M68K_SSWB_SIZE_3	(0x0030)

/* structures: */

/* the format 0xB stack frame: */
/* XXX we assume that this will pack: */
struct tme_m68k_fmtB {

  /* an unused word: */
  tme_uint8_t tme_m68k_fmtB_state0[1 * sizeof(tme_uint16_t)];

  /* the special status word: */
  tme_uint16_t tme_m68k_fmtB_ssw;

  /* the instruction pipe stage C: */
  tme_uint16_t tme_m68k_fmtB_ipipe_C;

  /* the instruction pipe stage B: */
  tme_uint16_t tme_m68k_fmtB_ipipe_B;

  /* the data cycle fault address: */
  tme_uint32_t tme_m68k_fmtB_addr;

  /* two unused words: */
  tme_uint8_t tme_m68k_fmtB_state1[2 * sizeof(tme_uint16_t)];

  /* the data output buffer: */
  tme_uint8_t tme_m68k_fmtB_dob[4];

  /* four unused words: */
  tme_uint8_t tme_m68k_fmtB_state2[4 * sizeof(tme_uint16_t)];

  /* the stage B address: */
  tme_uint32_t tme_m68k_fmtB_addr_B;

  /* two unused words: */
  tme_uint8_t tme_m68k_fmtB_state3[2 * sizeof(tme_uint16_t)];

  /* the data input buffer: */
  tme_uint8_t tme_m68k_fmtB_dib[4];

  /* three unused words: */
  tme_uint8_t tme_m68k_fmtB_state4[3 * sizeof(tme_uint16_t)];

  /* the version number and internal information: */
  tme_uint16_t tme_m68k_fmtB_version_info;

  /* eighteen unused words: */
  tme_uint8_t tme_m68k_fmtB_state5[18 * sizeof(tme_uint16_t)];
};

/* both executors are for the 68020: */
#define _TME_M68K_EXECUTE_CPU TME_M68K_M68020
#define _TME_M68K_EXECUTE_OPMAP tme_m68k_opcodes_m68020

/* create the slow executor: */
#define _TME_M68K_EXECUTE_NAME _tme_m68020_execute_slow
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW
#include "m68k-execute.c"
#undef _TME_M68K_EXECUTE_NAME
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW

/* create the fast executor: */
#define _TME_M68K_EXECUTE_NAME _tme_m68020_execute
#define _TME_M68K_EXECUTE_FAST
#define _TME_M68K_EXECUTE_SLOW _tme_m68020_execute_slow
#include "m68k-execute.c"
#undef _TME_M68K_EXECUTE_NAME
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW

#undef _TME_M68K_EXECUTE_CPU

/* m68020 exception processing: */
static void
_tme_m68020_exception(struct tme_m68k *ic)
{
  tme_uint32_t exceptions;
  /* one 16-bit word for each 16-bit internal register word in the format 0xB frame: */
  tme_uint8_t raw_state[(1 + 2 + 4 + 2 + 3 + 18) * sizeof(tme_uint16_t)], *raw;
  unsigned int raw_avail, raw_used;
  struct tme_m68k_fmtB fmtB;
  unsigned int flags;
  tme_uint8_t function_code;
  tme_uint16_t ssw;

  /* get the set of exceptions: */
  exceptions = ic->_tme_m68k_exceptions;

  /* a reset exception: */
  if (exceptions & TME_M68K_EXCEPTION_RESET) {

    /* do the common reset processing: */
    tme_m68k_do_reset(ic);
    /* NOTREACHED */
  }

  /* an address or bus error: */
  else if (exceptions & (TME_M68K_EXCEPTION_AERR
			 | TME_M68K_EXCEPTION_BERR)) {

    /* start the exception processing: */
    tme_m68k_exception_process_start(ic, 0);

    /* start assembling the raw data: */
    raw = raw_state;
    raw_avail = sizeof(raw_state);
#define RAW_PUT(v)			\
do {					\
  assert(raw_avail >= sizeof(v));	\
  memcpy(raw, &v, sizeof(v));		\
  raw += sizeof(v);			\
  raw_avail -= sizeof(v);		\
} while (/* CONSTCOND */ 0)

    /* put in the sequence: */
    raw_used = tme_m68k_sequence_empty(ic, raw, raw_avail);
    raw += raw_used;
    raw_avail -= raw_used;

    /* dispatch on the mode: */
    switch (ic->_tme_m68k_group0_sequence._tme_m68k_sequence_mode) {

    case TME_M68K_MODE_EXECUTION:
      
      /* put in the instruction buffer: */
      raw_used = tme_m68k_insn_buffer_empty(ic, raw, raw_avail);
      raw += raw_used;
      raw_avail -= raw_used;

      /* put in the EA address: */
      function_code = ic->_tme_m68k_ea_function_code;
      RAW_PUT(function_code);
      RAW_PUT(ic->_tme_m68k_ea_address);

      /* put in the memory X, Y, and Z buffers: */
      RAW_PUT(ic->tme_m68k_ireg_memx32);
      RAW_PUT(ic->tme_m68k_ireg_memy32);
      RAW_PUT(ic->tme_m68k_ireg_memz32);
      
      /* done: */
      break;

    case TME_M68K_MODE_EXCEPTION:

      /* put in the exceptions set: */
      RAW_PUT(ic->_tme_m68k_exceptions);

      /* put in the shadow status register: */
      RAW_PUT(ic->tme_m68k_ireg_shadow_sr);

      /* put in the last PC: */
      RAW_PUT(ic->tme_m68k_ireg_pc_last);
      
      /* done: */
      break;

    case TME_M68K_MODE_RTE:
      
      /* put in the shadow status register: */
      RAW_PUT(ic->tme_m68k_ireg_shadow_sr);

      /* put in the next program counter: */
      RAW_PUT(ic->tme_m68k_ireg_pc_next);

      /* put in the format/offset word: */
      RAW_PUT(ic->tme_m68k_ireg_format_offset);

      break;

    default: abort();
    }

    /* put in the internal state: */
    raw_avail = raw - raw_state;
    raw = raw_state;
#undef RAW_PUT
#define RAW_PUT(f)				\
do {						\
    raw_used = TME_MIN(sizeof(f), raw_avail);	\
    memcpy(f, raw, raw_used);			\
    raw += raw_used;				\
    raw_avail -= raw_used;			\
} while (/* CONSTCOND */ 0)

    /* use all of the reserved regions: */
    RAW_PUT(fmtB.tme_m68k_fmtB_state0);
    RAW_PUT(fmtB.tme_m68k_fmtB_state1);
    RAW_PUT(fmtB.tme_m68k_fmtB_state2);
    RAW_PUT(fmtB.tme_m68k_fmtB_state3);
    RAW_PUT(fmtB.tme_m68k_fmtB_state4);
    RAW_PUT(fmtB.tme_m68k_fmtB_state5);
#undef RAW_PUT

    /* put in the special status word and the data output buffer: */
    ssw = 0;
    flags = ic->_tme_m68k_group0_flags;

    /* if this was an instruction fetch: */
    if (flags & TME_M68K_BUS_CYCLE_FETCH) {

      /* we always ask for a rerun of at least the stage C word, so we
	 associate the fault address with the stage C word.  the
	 address of the stage C word is the address of the stage B
	 word minus sizeof(stage C word): */
      ssw |= TME_M68K_SSWB_RC;
      fmtB.tme_m68k_fmtB_addr_B = tme_htobe_u32(ic->_tme_m68k_group0_address + sizeof(fmtB.tme_m68k_fmtB_ipipe_C));

      /* if this is an address fault: */
      if (exceptions & TME_M68K_EXCEPTION_AERR) {

	/* nothing to do */
      }

      /* otherwise, this is a bus fault: */
      else {

	/* mark stage C as faulted: */
	ssw |= TME_M68K_SSWB_FC;

	/* if this is a 32-bit fetch: */
	if (ic->_tme_m68k_group0_buffer_read_size == sizeof(tme_uint32_t)) {

	  /* a 32-bit fetch additionally faults "prefetching" the stage B word: */
	  ssw |= (TME_M68K_SSWB_FB | TME_M68K_SSWB_RB);
	}
      }
    }

    /* otherwise, this was not an instruction fetch: */
    else {

      /* this is a data fault: */
      ssw |= TME_M68K_SSWB_DF;

      /* put in the function code and address: */
      ssw |= ic->_tme_m68k_group0_function_code;
      fmtB.tme_m68k_fmtB_addr = tme_htobe_u32(ic->_tme_m68k_group0_address);

      /* if this was part of a read/modify/write cycle: */
      if (flags & TME_M68K_BUS_CYCLE_RMW) {
	ssw |= TME_M68K_SSWB_RM;
      }

      /* if this was a read: */
      if (flags & TME_M68K_BUS_CYCLE_READ) {
	ssw |= TME_M68K_SSWB_RW;

	/* put in the size indication: */
	switch (ic->_tme_m68k_group0_buffer_read_size) {
	default: assert(FALSE);
	case 1: ssw |= TME_M68K_SSWB_SIZE_1; break;
	case 2: ssw |= TME_M68K_SSWB_SIZE_2; break;
	case 3: ssw |= TME_M68K_SSWB_SIZE_3; break;
	case 4: ssw |= TME_M68K_SSWB_SIZE_4; break;
	}

      }
	
      /* otherwise, this is a write: */
      else {

	/* put in the size indication: */
	switch (ic->_tme_m68k_group0_buffer_write_size) {
	default: assert(FALSE);
	case 1: ssw |= TME_M68K_SSWB_SIZE_1; break;
	case 2: ssw |= TME_M68K_SSWB_SIZE_2; break;
	case 3: ssw |= TME_M68K_SSWB_SIZE_3; break;
	case 4: ssw |= TME_M68K_SSWB_SIZE_4; break;
	}

	/* put in the data output buffer: */
	memcpy(fmtB.tme_m68k_fmtB_dob
	       + sizeof(fmtB.tme_m68k_fmtB_dob)
	       - ic->_tme_m68k_group0_buffer_write_size,
	       ic->_tme_m68k_group0_buffer_write,
	       ic->_tme_m68k_group0_buffer_write_size);
      }
    }

    /* put in the SSW: */
    fmtB.tme_m68k_fmtB_ssw = tme_htobe_u16(ssw);

    /* push the format 0xB contents.  we don't need to worry about
       restarting here, because any fault is a double fault: */
    ic->tme_m68k_ireg_a7 -= sizeof(fmtB);
    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_a7;
    ic->_tme_m68k_ea_function_code = TME_M68K_FC_SD; /* XXX right? */
    tme_m68k_write_mem(ic, (tme_uint8_t *) &fmtB, sizeof(fmtB));

    /* now finish the processing for this exception: */
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_B, 
				      ((exceptions & TME_M68K_EXCEPTION_BERR)
				       ? TME_M68K_VECTOR_BERR
				       : TME_M68K_VECTOR_AERR));
    ic->_tme_m68k_exceptions = (exceptions &= ~ (TME_M68K_EXCEPTION_AERR
						 | TME_M68K_EXCEPTION_BERR));
  }

  /* do normal exception processing: */
  tme_m68020_exception_process(ic);
  /* NOTREACHED */
}

/* m68020 RTE processing: */
static void
_tme_m68020_rte(struct tme_m68k *ic)
{
  tme_uint16_t format;
  struct tme_m68k_fmtB fmtB;
  tme_uint32_t fmtB_address;
  /* one 16-bit word for each 16-bit internal register word in the format 0xB frame: */
  tme_uint8_t raw_state[(1 + 2 + 4 + 2 + 3 + 18) * sizeof(tme_uint16_t)], *raw;
  unsigned int raw_avail, raw_used;
  tme_uint8_t function_code;
  tme_uint16_t ssw;
  unsigned int flags;
  const tme_uint8_t *ib;
  unsigned int buffer_size;

  /* start the RTE: */
  format = tme_m68k_rte_start(ic);

  /* if this is a format 0 or format two stack frame, finish it now: */
  if (format == TME_M68K_FORMAT_0
      || format == TME_M68K_FORMAT_2) {
    ic->_tme_m68k_mode = TME_M68K_MODE_EXECUTION;
    TME_M68K_SEQUENCE_START;
    tme_m68k_rte_finish(ic, (format == TME_M68K_FORMAT_2
			     ? sizeof(ic->tme_m68k_ireg_pc_last)
			     : 0));
    /* NOTREACHED */
  }

  /* if this is a format one stack frame: */
  if (format == TME_M68K_FORMAT_1) {
    
    /* "For the throwaway four-word frame, the processor reads the SR
       value from the frame, increments the active stack pointer by 8,
       updates the SR with the value read from the stack, and then
       begins RTE processing again" */
    ic->tme_m68k_ireg_a7 += 8;
    tme_m68k_change_sr(ic, ic->tme_m68k_ireg_shadow_sr);
    TME_M68K_SEQUENCE_START;
    tme_m68k_redispatch(ic);
    /* NOTREACHED */
  }

  /* whenever we detect a format error, begin exception processing
     for a format error.  we have to back the PC up by the size of the
     RTE instruction so the PC points to it: */
#define FORMAT_ERROR_IF(e)					\
do {								\
  if (e) {							\
    ic->tme_m68k_ireg_pc -= sizeof(tme_uint16_t);		\
    tme_m68k_exception(ic, TME_M68K_EXCEPTION_INST(TME_M68K_VECTOR_FORMAT));\
    /* NOTREACHED */						\
  }								\
} while (/* CONSTCOND */ 0)

  /* this frame must be a format 0xB frame: */
  FORMAT_ERROR_IF(format != TME_M68K_FORMAT_B);

  /* do a read of the last word in the stack frame to determine
     accessibility.  we rely on tme_m68k_rte_start leaving
     ic->_tme_m68k_ea_address pointing after the format/offset word: */
  fmtB_address = ic->_tme_m68k_ea_address;
  ic->_tme_m68k_ea_address += sizeof(fmtB) - sizeof(tme_uint32_t);
  tme_m68k_read_memx32(ic);

  /* read in the format 0xB information.  if we get a bus error
     during this operation it's a double fault: */
  assert(ic->_tme_m68k_exceptions == 0);
  ic->_tme_m68k_exceptions = TME_M68K_EXCEPTION_BERR;
  ic->_tme_m68k_ea_address = fmtB_address;
  tme_m68k_read_mem(ic, (tme_uint8_t *) &fmtB, sizeof(fmtB));
  ic->_tme_m68k_exceptions = 0;

  /* reset the input and output buffers: */
  ic->_tme_m68k_group0_buffer_read_size = 0;
  ic->_tme_m68k_group0_buffer_read_softrr = 0;
  ic->_tme_m68k_group0_buffer_write_size = 0;
  ic->_tme_m68k_group0_buffer_write_softrr = 0;

  /* get out the SSW: */
  ssw = tme_betoh_u16(fmtB.tme_m68k_fmtB_ssw);

  /* initialize the flags: */
  flags = 0;

  /* initialize the buffer size: */
  buffer_size = 0;

  /* initialize the input buffer: */
  ib = NULL;

  /* if this was an instruction fetch: */
  if (ssw & (TME_M68K_SSWB_FC | TME_M68K_SSWB_FB)) {

    /* we don't generate bus errors that only fault on stage B: */
    if ((ssw & (TME_M68K_SSWB_FC | TME_M68K_SSWB_FB)) == TME_M68K_SSWB_FB) {
      abort();
    }

    /* if we faulted on both stages, we must be rerunning both or not
       rerunning both.  since we already know we either faulted only on
       stage C, or we faulted on both, if we faulted on stage B check
       that both RB and RC have the same value: */
    if ((ssw & TME_M68K_SSWB_FB)
	&& !(ssw & TME_M68K_SSWB_RC) != !(ssw & TME_M68K_SSWB_RB)) {
      abort();
    }

    /* note that this was an instruction fetch: */
    flags |= TME_M68K_BUS_CYCLE_FETCH;

    /* get out the function code: */
    ic->_tme_m68k_group0_function_code
      = ((ic->tme_m68k_ireg_shadow_sr & TME_M68K_FLAG_S)
	 ? TME_M68K_FC_SP
	 : TME_M68K_FC_UP);

    /* get out the fault address: */
    ic->_tme_m68k_group0_address = tme_betoh_u32(fmtB.tme_m68k_fmtB_addr_B) - sizeof(fmtB.tme_m68k_fmtB_ipipe_C);

    /* if the user reran this cycle: */
    if (!(ssw & TME_M68K_SSWB_RC)) {

      /* set the input buffer pointer: */
      /* NB that for 32-bit fetches, this relies on the fact that the
	 stage B word comes immediately after the stage C word in the
	 frame: */
      ib = (const tme_uint8_t *) &fmtB.tme_m68k_fmtB_ipipe_C;
    }

    /* if this is a fault only on stage C: */
    if (!(ssw & TME_M68K_SSWB_FB)) {

      /* set the input buffer size: */
      buffer_size = sizeof(tme_uint16_t);
    }

    /* otherwise, if this is a fault on stages C and B: */
    else {
      
      /* set the input buffer size: */
      buffer_size = sizeof(tme_uint32_t);
    }
  }

  /* otherwise, this was a data access: */
  else {

    /* get out the function code: */
    ic->_tme_m68k_group0_function_code = ssw & TME_M68K_FC_7;

    /* get out the fault address: */
    ic->_tme_m68k_group0_address = tme_betoh_u32(fmtB.tme_m68k_fmtB_addr);

    /* get out the size: */
    switch (ssw & TME_M68K_SSWB_SIZE_MASK) {
    case TME_M68K_SSWB_SIZE_1: buffer_size = 1; break;
    case TME_M68K_SSWB_SIZE_2: buffer_size = 2; break;
    case TME_M68K_SSWB_SIZE_3: buffer_size = 3; break;
    case TME_M68K_SSWB_SIZE_4: buffer_size = 4; break;
    }

    /* if this was a read cycle: */
    if (ssw & TME_M68K_SSWB_RW) {

      /* mark that this was a read cycle: */
      flags |= TME_M68K_BUS_CYCLE_READ;

      /* if the user reran this cycle: */
      if (!(ssw & TME_M68K_SSWB_DF)) {

	/* set the input buffer pointer: */
	ib = &fmtB.tme_m68k_fmtB_dib[0];
      }
    }

    /* otherwise, this was a write cycle: */
    else {

      /* if the user reran this cycle, note that, otherwise
	 copy in the data output buffer: */
      if (!(ssw & TME_M68K_SSWB_DF)) {
	ic->_tme_m68k_group0_buffer_write_softrr = buffer_size;
      }
      else {
	memcpy(ic->_tme_m68k_group0_buffer_write,
	       fmtB.tme_m68k_fmtB_dob + sizeof(fmtB.tme_m68k_fmtB_dob) - buffer_size,
	       buffer_size);
	ic->_tme_m68k_group0_buffer_write_size = buffer_size;
      }
    }
  }

  /* if the user reran this instruction fetch or data read cycle, read
     in the appropriate input buffer: */
  if (ib != NULL) {
    memcpy(ic->_tme_m68k_group0_buffer_read,
	   ib + sizeof(fmtB.tme_m68k_fmtB_dib) - buffer_size,
	   buffer_size);
    ic->_tme_m68k_group0_buffer_read_softrr = buffer_size;
    ic->_tme_m68k_group0_buffer_read_size = buffer_size;
  }

  /* get out our internal state: */
  raw = raw_state;
  raw_avail = sizeof(raw_state);
#define RAW_GET(f)				\
do {						\
    raw_used = TME_MIN(sizeof(f), raw_avail);	\
    memcpy(raw, f, raw_used);			\
    raw += raw_used;				\
    raw_avail -= raw_used;			\
} while (/* CONSTCOND */ 0)

  /* use all of the reserved regions: */
  RAW_GET(fmtB.tme_m68k_fmtB_state0);
  RAW_GET(fmtB.tme_m68k_fmtB_state1);
  RAW_GET(fmtB.tme_m68k_fmtB_state2);
  RAW_GET(fmtB.tme_m68k_fmtB_state3);
  RAW_GET(fmtB.tme_m68k_fmtB_state4);
  RAW_GET(fmtB.tme_m68k_fmtB_state5);
#undef RAW_GET

  /* take apart the internal state: */
  raw = raw_state;
  raw_avail = sizeof(raw_state) - raw_avail;
#define RAW_GET(v)				\
do {						\
  FORMAT_ERROR_IF(raw_avail < sizeof(v));	\
  memcpy(&v, raw, sizeof(v));			\
  raw += sizeof(v);				\
  raw_avail -= sizeof(v);			\
} while (/* CONSTCOND */ 0)

  /* get out the sequence: */
  raw_used = tme_m68k_sequence_fill(ic, raw, raw_avail);
  FORMAT_ERROR_IF(raw_used <= 0);
  raw += raw_used;
  raw_avail -= raw_used;
  
  /* dispatch on the mode: */
  switch (ic->_tme_m68k_group0_sequence._tme_m68k_sequence_mode) {
    
  case TME_M68K_MODE_EXECUTION:
    
    /* get out the instruction buffer: */
    raw_used = tme_m68k_insn_buffer_fill(ic, raw, raw_avail);
    FORMAT_ERROR_IF(raw_used <= 0);
    raw += raw_used;
    raw_avail -= raw_used;
    
    /* get out the EA address: */
    RAW_GET(function_code);
    ic->_tme_m68k_ea_function_code = function_code;
    RAW_GET(ic->_tme_m68k_ea_address);
    
    /* get out the memory X, Y, and Z buffers: */
    RAW_GET(ic->tme_m68k_ireg_memx32);
    RAW_GET(ic->tme_m68k_ireg_memy32);
    RAW_GET(ic->tme_m68k_ireg_memz32);
    
    /* done: */
    break;
    
  case TME_M68K_MODE_EXCEPTION:
    
    /* get out the exceptions set: */
    RAW_GET(ic->_tme_m68k_exceptions);
    
    /* get out the shadow status register: */
    RAW_GET(ic->tme_m68k_ireg_shadow_sr);
    
    /* get out the last PC: */
    RAW_GET(ic->tme_m68k_ireg_pc_last);
      
    /* done: */
    break;
    
  case TME_M68K_MODE_RTE:
    
    /* get out the shadow status register: */
    RAW_GET(ic->tme_m68k_ireg_shadow_sr);
    
    /* get out the next program counter: */
    RAW_GET(ic->tme_m68k_ireg_pc_next);
    
    /* get out the format/offset word: */
    RAW_GET(ic->tme_m68k_ireg_format_offset);
    
    break;
    
  default: FORMAT_ERROR_IF(TRUE);
  }

  /* finish the RTE: */
  ic->_tme_m68k_sequence = ic->_tme_m68k_group0_sequence;
  TME_M68K_SEQUENCE_RESTART;
  tme_m68k_rte_finish(ic, sizeof(fmtB));
  /* NOTREACHED */
}

/* this creates and returns a new m68020: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,m68k,m68020) {
  struct tme_m68k *ic;

  /* allocate the m68k structure: */
  ic = tme_new0(struct tme_m68k, 1);
  ic->tme_m68k_element = element;

  /* fill in the m68020-specific parts of the structure: */
  ic->tme_m68k_type = TME_M68K_M68020;
  ic->_tme_m68k_mode_execute = _tme_m68020_execute;
  ic->_tme_m68k_mode_exception = _tme_m68020_exception;
  ic->_tme_m68k_mode_rte = _tme_m68020_rte;
  tme_m68k_opcodes_init_m68020(tme_m68k_opcodes_m68020);

  /* call the common m68k new function: */
  return (tme_m68k_new(ic, args, extra, _output));
}
