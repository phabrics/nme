/* $Id: m68010.c,v 1.9 2007/02/16 01:40:44 fredette Exp $ */

/* ic/m68k/m68010.c - implementation of Motorola 68010 emulation: */

/*
 * Copyright (c) 2002, 2003 Matt Fredette
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
_TME_RCSID("$Id: m68010.c,v 1.9 2007/02/16 01:40:44 fredette Exp $");

/* includes: */
#include "m68k-impl.h"

/* macros: */
#define TME_M68K_SSW8_RR         0x8000  
#define TME_M68K_SSW8_IF         0x2000
#define TME_M68K_SSW8_DF         0x1000
#define TME_M68K_SSW8_RM         0x0800  
#define TME_M68K_SSW8_HI         0x0400
#define TME_M68K_SSW8_BX         0x0200  
#define TME_M68K_SSW8_RW         0x0100

/* structures: */

/* the format 8 stack frame: */
/* XXX we assume that this will pack: */
struct tme_m68k_fmt8 {

  /* the special status word: */
  tme_uint16_t tme_m68k_fmt8_ssw;

  /* the faulting address: */
  tme_uint16_t tme_m68k_fmt8_addr_high;
  tme_uint16_t tme_m68k_fmt8_addr_low;

  /* the first unused word: */
  tme_uint8_t tme_m68k_fmt8_state0[2];

  /* the data output buffer: */
  tme_uint8_t tme_m68k_fmt8_dob[2];

  /* the second unused word: */
  tme_uint8_t tme_m68k_fmt8_state1[2];

  /* the data input buffer: */
  tme_uint8_t tme_m68k_fmt8_dib[2];

  /* the third unused word: */
  tme_uint8_t tme_m68k_fmt8_state2[2];

  /* the instruction input buffer: */
  tme_uint8_t tme_m68k_fmt8_iib[2];

  /* the internal information: */
  tme_uint8_t tme_m68k_fmt8_state3[32];
};

/* both executors are for the 68010: */
#define _TME_M68K_EXECUTE_CPU TME_M68K_M68010
#define _TME_M68K_EXECUTE_OPMAP tme_m68k_opcodes_m68010

/* create the slow executor: */
#define _TME_M68K_EXECUTE_NAME _tme_m68010_execute_slow
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW
#include "m68k-execute.c"
#undef _TME_M68K_EXECUTE_NAME
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW

/* create the fast executor: */
#define _TME_M68K_EXECUTE_NAME _tme_m68010_execute
#define _TME_M68K_EXECUTE_FAST
#define _TME_M68K_EXECUTE_SLOW _tme_m68010_execute_slow
#include "m68k-execute.c"
#undef _TME_M68K_EXECUTE_NAME
#undef _TME_M68K_EXECUTE_FAST
#undef _TME_M68K_EXECUTE_SLOW

#undef _TME_M68K_EXECUTE_CPU

/* m68010 exception processing: */
static void
_tme_m68010_exception(struct tme_m68k *ic)
{
  tme_uint32_t exceptions;
  tme_uint8_t raw_state[19 * sizeof(tme_uint16_t)], *raw;
  unsigned int raw_avail, raw_used;
  struct tme_m68k_fmt8 fmt8;
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

      /* put in the memory X and Y buffers: */
      RAW_PUT(ic->tme_m68k_ireg_memx32);
      RAW_PUT(ic->tme_m68k_ireg_memy32);
      
      /* done: */
      break;

    case TME_M68K_MODE_EXCEPTION:

      /* put in the exceptions set: */
      RAW_PUT(ic->_tme_m68k_exceptions);

      /* put in the shadow status register: */
      RAW_PUT(ic->tme_m68k_ireg_shadow_sr);
      
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
    RAW_PUT(fmt8.tme_m68k_fmt8_state0);
    RAW_PUT(fmt8.tme_m68k_fmt8_state1);
    RAW_PUT(fmt8.tme_m68k_fmt8_state2);
    RAW_PUT(fmt8.tme_m68k_fmt8_state3);
#undef RAW_PUT

    /* put in the special status word and the data output buffer: */
    ssw = 0;
    flags = ic->_tme_m68k_group0_flags;
    if (flags & TME_M68K_BUS_CYCLE_READ) {
      ssw |= (TME_M68K_SSW8_RW
	      | ((flags & TME_M68K_BUS_CYCLE_FETCH)
		 ? TME_M68K_SSW8_IF
		 : TME_M68K_SSW8_DF));
      assert(ic->_tme_m68k_group0_buffer_read_size <= sizeof(tme_uint16_t));
      if (ic->_tme_m68k_group0_buffer_read_size == sizeof(tme_uint8_t)) {
	ssw |= TME_M68K_SSW8_BX;
      }
    }
    else {
      assert(ic->_tme_m68k_group0_buffer_write_size <= sizeof(tme_uint16_t));
      if (ic->_tme_m68k_group0_buffer_write_size == sizeof(tme_uint8_t)) {
	ssw |= TME_M68K_SSW8_BX;
      }
      memcpy(fmt8.tme_m68k_fmt8_dob
	     + sizeof(fmt8.tme_m68k_fmt8_dob)
	     - ic->_tme_m68k_group0_buffer_write_size,
	     ic->_tme_m68k_group0_buffer_write,
	     ic->_tme_m68k_group0_buffer_write_size);
    }
    if (flags & TME_M68K_BUS_CYCLE_RMW) {
      ssw |= TME_M68K_SSW8_RM;
    }
    ssw |= ic->_tme_m68k_group0_function_code;
    fmt8.tme_m68k_fmt8_ssw = tme_htobe_u16(ssw);

    /* put in the fault address: */
    fmt8.tme_m68k_fmt8_addr_high = tme_htobe_u16(ic->_tme_m68k_group0_address >> 16);
    fmt8.tme_m68k_fmt8_addr_low = tme_htobe_u16(ic->_tme_m68k_group0_address & 0xffff);

    /* push the format 8 contents.  we don't need to worry about
       restarting here, because any fault is a double fault: */
    ic->tme_m68k_ireg_a7 -= sizeof(fmt8);
    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_a7;
    ic->_tme_m68k_ea_function_code = TME_M68K_FC_SD; /* XXX right? */
    tme_m68k_write_mem(ic, (tme_uint8_t *) &fmt8, sizeof(fmt8));

    /* now finish the processing for this exception: */
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_8, 
				      ((exceptions & TME_M68K_EXCEPTION_BERR)
				       ? TME_M68K_VECTOR_BERR
				       : TME_M68K_VECTOR_AERR));
    ic->_tme_m68k_exceptions = (exceptions &= ~ (TME_M68K_EXCEPTION_AERR
						 | TME_M68K_EXCEPTION_BERR));
  }

  /* do normal exception processing: */
  tme_m68000_exception_process(ic);
  /* NOTREACHED */
}

/* m68010 RTE processing: */
static void
_tme_m68010_rte(struct tme_m68k *ic)
{
  tme_uint16_t format;
  struct tme_m68k_fmt8 fmt8;
  tme_uint32_t fmt8_address;
  tme_uint8_t raw_state[19 * sizeof(tme_uint16_t)], *raw;
  unsigned int raw_avail, raw_used;
  tme_uint8_t function_code;
  tme_uint16_t ssw;
  unsigned int flags;
  tme_uint8_t *ib;
  unsigned int buffer_size;

  /* start the RTE: */
  format = tme_m68k_rte_start(ic);

  /* if this is a format 0 stack frame, finish it now: */
  if (format == TME_M68K_FORMAT_0) {
    ic->_tme_m68k_mode = TME_M68K_MODE_EXECUTION;
    TME_M68K_SEQUENCE_START;
    tme_m68k_rte_finish(ic, 0);
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

  /* this frame must be a format 8 frame: */
  FORMAT_ERROR_IF(format != TME_M68K_FORMAT_8);

  /* do a read of the last word in the stack frame to determine
     accessibility.  we rely on tme_m68k_rte_start leaving
     ic->_tme_m68k_ea_address pointing after the format/offset word: */
  fmt8_address = ic->_tme_m68k_ea_address;
  ic->_tme_m68k_ea_address += sizeof(fmt8) - sizeof(tme_uint16_t);
  tme_m68k_read_memx16(ic);

  /* read in the format 8 information.  if we get a bus error
     during this operation it's a double fault: */
  assert(ic->_tme_m68k_exceptions == 0);
  ic->_tme_m68k_exceptions = TME_M68K_EXCEPTION_BERR;
  ic->_tme_m68k_ea_address = fmt8_address;
  tme_m68k_read_mem(ic, (tme_uint8_t *) &fmt8, sizeof(fmt8));
  ic->_tme_m68k_exceptions = 0;

  /* get out the fault address: */
  ic->_tme_m68k_group0_address = ((((tme_uint32_t) tme_betoh_u16(fmt8.tme_m68k_fmt8_addr_high)) << 16)
				  | tme_betoh_u16(fmt8.tme_m68k_fmt8_addr_low));
  
  /* get out the special status word and either the data/instruction
     input buffer or the data output buffer: */
  ssw = tme_betoh_u16(fmt8.tme_m68k_fmt8_ssw);
  FORMAT_ERROR_IF(ssw & TME_M68K_SSW8_HI);
  flags = 0;
  ic->_tme_m68k_group0_buffer_read_size = 0;
  ic->_tme_m68k_group0_buffer_read_softrr = 0;
  ic->_tme_m68k_group0_buffer_write_size = 0;
  ic->_tme_m68k_group0_buffer_write_softrr = 0;
  buffer_size = ((ssw & TME_M68K_SSW8_BX)
		 ? sizeof(tme_uint8_t)
		 : sizeof(tme_uint16_t));

  /* if this was a read cycle: */
  if (ssw & TME_M68K_SSW8_RW) {
    flags |= TME_M68K_BUS_CYCLE_READ;

    /* if this was an instruction fetch cycle: */
    if (ssw & TME_M68K_SSW8_IF) {
      flags |= TME_M68K_BUS_CYCLE_FETCH;
      FORMAT_ERROR_IF(ssw & TME_M68K_SSW8_DF);
      ib = fmt8.tme_m68k_fmt8_iib;
    }

    /* otherwise, this was a data fetch cycle: */
    else {
      FORMAT_ERROR_IF((ssw & (TME_M68K_SSW8_IF
			      | TME_M68K_SSW8_DF))
		      != TME_M68K_SSW8_DF);
      ib = fmt8.tme_m68k_fmt8_dib;
    }

    /* if the user reran this cycle, read in the appropriate
       input buffer: */
    if (ssw & TME_M68K_SSW8_RR) {
      memcpy(ic->_tme_m68k_group0_buffer_read,
	     ib + sizeof(tme_uint16_t) - buffer_size,
	     buffer_size);
      ic->_tme_m68k_group0_buffer_read_softrr = buffer_size;
      ic->_tme_m68k_group0_buffer_read_size = buffer_size;
    }
  }

  /* otherwise, this was a write cycle: */
  else {

    /* if the user reran this cycle, note that, otherwise
       copy in the data output buffer: */
    if (ssw & TME_M68K_SSW8_RR) {
      ic->_tme_m68k_group0_buffer_write_softrr = buffer_size;
    }
    else {
      memcpy(ic->_tme_m68k_group0_buffer_write,
	     fmt8.tme_m68k_fmt8_dob + sizeof(tme_uint16_t) - buffer_size,
	     buffer_size);
      ic->_tme_m68k_group0_buffer_write_size = buffer_size;
    }
  }

  /* get the fault function code and address: */
  ic->_tme_m68k_group0_function_code = ssw & TME_M68K_FC_7;
  ic->_tme_m68k_group0_address =
    ((((tme_uint32_t) tme_betoh_u16(fmt8.tme_m68k_fmt8_addr_high)) << 16)
     | tme_betoh_u16(fmt8.tme_m68k_fmt8_addr_low));

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
  RAW_GET(fmt8.tme_m68k_fmt8_state0);
  RAW_GET(fmt8.tme_m68k_fmt8_state1);
  RAW_GET(fmt8.tme_m68k_fmt8_state2);
  RAW_GET(fmt8.tme_m68k_fmt8_state3);
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
    
    /* get out the memory X and Y buffers: */
    RAW_GET(ic->tme_m68k_ireg_memx32);
    RAW_GET(ic->tme_m68k_ireg_memy32);
    
    /* done: */
    break;
    
  case TME_M68K_MODE_EXCEPTION:
    
    /* get out the exceptions set: */
    RAW_GET(ic->_tme_m68k_exceptions);
    
    /* get out the shadow status register: */
    RAW_GET(ic->tme_m68k_ireg_shadow_sr);
    
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
  tme_m68k_rte_finish(ic, sizeof(fmt8));
  /* NOTREACHED */
}

/* this creates and returns a new m68010: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,m68k,m68010) {
  struct tme_m68k *ic;

  /* allocate the m68k structure: */
  ic = tme_new0(struct tme_m68k, 1);
  ic->tme_m68k_element = element;

  /* fill in the m68010-specific parts of the structure: */
  ic->tme_m68k_type = TME_M68K_M68010;
  ic->_tme_m68k_mode_execute = _tme_m68010_execute;
  ic->_tme_m68k_mode_exception = _tme_m68010_exception;
  ic->_tme_m68k_mode_rte = _tme_m68010_rte;
  tme_m68k_opcodes_init_m68010(tme_m68k_opcodes_m68010);

  /* call the common m68k new function: */
  return (tme_m68k_new(ic, args, extra, _output));
}
