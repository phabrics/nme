/* $Id: m68k-misc.c,v 1.27 2009/08/29 19:47:52 fredette Exp $ */

/* ic/m68k/m68k-misc.c - miscellaneous things for the m68k emulator: */

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

/* includes: */
#include "m68k-impl.h"

_TME_RCSID("$Id: m68k-misc.c,v 1.27 2009/08/29 19:47:52 fredette Exp $");

/* the memory buffer read and write functions: */
#if TME_M68K_SIZE_8 != 1
#error "TME_M68K_SIZE_8 must be 1"
#endif
#if TME_M68K_SIZE_16 != 2
#error "TME_M68K_SIZE_16 must be 2"
#endif
#if TME_M68K_SIZE_32 != 4
#error "TME_M68K_SIZE_32 must be 4"
#endif
const _tme_m68k_xfer_memx _tme_m68k_read_memx[5] = {
  NULL,
  tme_m68k_read_memx8,
  tme_m68k_read_memx16,
  NULL,
  tme_m68k_read_memx32
};
const _tme_m68k_xfer_memx _tme_m68k_write_memx[5] = {
  NULL,
  tme_m68k_write_memx8,
  tme_m68k_write_memx16,
  NULL,
  tme_m68k_write_memx32
};
const _tme_m68k_xfer_mem _tme_m68k_read_mem[5] = {
  NULL,
  tme_m68k_read_mem8,
  tme_m68k_read_mem16,
  NULL,
  tme_m68k_read_mem32
};
const _tme_m68k_xfer_mem _tme_m68k_write_mem[5] = {
  NULL,
  tme_m68k_write_mem8,
  tme_m68k_write_mem16,
  NULL,
  tme_m68k_write_mem32
};

/* our bus signal handler: */
static int
_tme_m68k_bus_signal(struct tme_bus_connection *conn_bus, unsigned int signal)
{
  struct tme_m68k *ic;
  unsigned int level_edge;

  /* recover our IC: */
  ic = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* take out the level and edge: */
  level_edge = signal;
  signal = TME_BUS_SIGNAL_WHICH(signal);
  level_edge ^= signal;

  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_m68k_external_mutex);

  /* on the falling edge of HALT or RESET, halt the processor: */
  if (((level_edge & TME_BUS_SIGNAL_LEVEL_MASK)
       == TME_BUS_SIGNAL_LEVEL_ASSERTED)
      && (signal == TME_BUS_SIGNAL_HALT
	  || signal == TME_BUS_SIGNAL_RESET)) {
    ic->tme_m68k_external_halt = TRUE;
  }

  /* on the rising edge of RESET, reset the processor: */
  else if (signal == TME_BUS_SIGNAL_RESET
	   && ((level_edge & TME_BUS_SIGNAL_LEVEL_MASK)
	       == TME_BUS_SIGNAL_LEVEL_NEGATED)) {
    ic->tme_m68k_external_reset = TRUE;
  }

  /* on any other HALT or RESET, do nothing: */
  else if (signal == TME_BUS_SIGNAL_RESET
	   || signal == TME_BUS_SIGNAL_HALT) {
    /* nothing */
  }

  /* anything else: */
  else {
    abort();
  }

  /* unlock the external mutex: */
  tme_mutex_unlock(&ic->tme_m68k_external_mutex);

  /* notify any threads waiting on the external condition: */
  tme_cond_notify(&ic->tme_m68k_external_cond, TRUE);
  return (TME_OK);
}

/* this enables or disables an m6888x: */
static int
_tme_m6888x_enable(struct tme_m68k_bus_connection *conn_m68k, int enabled)
{
  struct tme_m68k *ic;

  /* recover our IC: */
  ic = conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* NB: we're lazy here and don't bother locking the external mutex: */
  if (ic->tme_m68k_fpu_type == TME_M68K_FPU_NONE) {
    return (ENXIO);
  }
  ic->tme_m68k_fpu_enabled = enabled;
  return (TME_OK);
}

/* our interrupt handler: */
static int
_tme_m68k_bus_interrupt(struct tme_m68k_bus_connection *conn_m68k, unsigned int ipl)
{
  struct tme_m68k *ic;

  /* recover our IC: */
  ic = conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_m68k_external_mutex);

  /* set the interrupt line: */
  ic->tme_m68k_external_ipl = ipl;

  /* if the IPL has dropped below the NMI level, the next transition
     to that level will cause an NMI: */
  if (ipl < TME_M68K_IPL_NMI) {
    ic->tme_m68k_external_ipl_previous_nmi = FALSE;
  }

  /* unlock the external mutex: */
  tme_mutex_unlock(&ic->tme_m68k_external_mutex);

  /* notify any threads waiting on the external condition: */
  tme_cond_notify(&ic->tme_m68k_external_cond, TRUE);
  return (TME_OK);
}

/* this checks for external signals.  this must be called with the
   external mutex held: */
void
tme_m68k_external_check(struct tme_m68k *ic, tme_uint32_t internal_exceptions)
{
  unsigned int ipl;
  int vector;
  int rc;

  /* if an external reset has been requested, start reset exception
     processing: */
  if (ic->tme_m68k_external_reset) {
    ic->tme_m68k_external_reset = FALSE;
    tme_mutex_unlock(&ic->tme_m68k_external_mutex);
    tme_m68k_exception(ic, TME_M68K_EXCEPTION_RESET);
  }

  /* if an external halt has been requested, halt: */
  if (ic->tme_m68k_external_halt) {
    ic->tme_m68k_external_halt = FALSE;
    tme_mutex_unlock(&ic->tme_m68k_external_mutex);
    ic->_tme_m68k_mode = TME_M68K_MODE_HALT;
    TME_M68K_SEQUENCE_START;
    tme_m68k_redispatch(ic);
  }

  /* if we are not halted, and an interrupt can be serviced, start
     interrupt exception processing: */
  ipl = ic->tme_m68k_external_ipl;
  if (ic->_tme_m68k_mode != TME_M68K_MODE_HALT
      && ipl >= TME_M68K_IPL_MIN
      && ipl <= TME_M68K_IPL_MAX
      && ((ipl == TME_M68K_IPL_NMI
	   && !ic->tme_m68k_external_ipl_previous_nmi)
	  || ipl > TME_M68K_FLAG_IPM(ic->tme_m68k_ireg_sr))) {
    
    /* if this is an NMI, prevent it from being repeatedly accepted: */
    if (ipl == TME_M68K_IPL_NMI) {
      ic->tme_m68k_external_ipl_previous_nmi = TRUE;
    }

    tme_mutex_unlock(&ic->tme_m68k_external_mutex);
    
    /* acknowledge the interrupt and get the vector: */
    tme_m68k_callout_unlock(ic);
    rc = (*ic->_tme_m68k_bus_connection->tme_m68k_bus_connection.tme_bus_intack)
      (&ic->_tme_m68k_bus_connection->tme_m68k_bus_connection,
       ipl, &vector);
    tme_m68k_callout_relock(ic);
    if (rc == TME_EDEADLK) {
      abort();
    }

    /* if the interrupt acknowledge failed, this is a spurious interrupt: */
    if (rc == ENOENT) {
      vector = TME_M68K_VECTOR_SPURIOUS;
    }

    /* if no vector is given, use the autovector: */
    else if (vector == TME_BUS_INTERRUPT_VECTOR_UNDEF) {
      vector = TME_M68K_VECTOR_SPURIOUS + ipl;
    }

    /* dispatch the exceptions: */
    tme_m68k_exception(ic, internal_exceptions | TME_M68K_EXCEPTION_INT(ipl, vector));
  }

  /* if there are internal exceptions to process, do so: */
  if (internal_exceptions != 0) {
    tme_mutex_unlock(&ic->tme_m68k_external_mutex);
    tme_m68k_exception(ic, internal_exceptions);
  }

  /* there are no exceptions to process: */
}

/* the idle function, used when the processor is halted or stopped: */
static void
tme_m68k_idle(struct tme_m68k *ic)
{  
  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_m68k_external_mutex);

  /* loop forever: */
  for (;;) {

    /* check for any external signal: */
    tme_m68k_external_check(ic, 0);

    /* await an external condition: */
    tme_cond_wait_yield(&ic->tme_m68k_external_cond, &ic->tme_m68k_external_mutex);
  }
}

/* the m68k thread: */
static void
tme_m68k_thread(struct tme_m68k *ic)
{

  /* we use longjmp to redispatch: */
  do { } while (setjmp(ic->_tme_m68k_dispatcher));

  /* we must not have a busy fast instruction TLB entry: */
  assert (ic->_tme_m68k_insn_fetch_fast_itlb == NULL);

  /* clear the group 0 hook: */
  ic->_tme_m68k_group0_hook = NULL;

  /* dispatch on the current mode: */
  switch (ic->_tme_m68k_mode) {

  case TME_M68K_MODE_EXECUTION:
    (*ic->_tme_m68k_mode_execute)(ic);
    /* NOTREACHED */

  case TME_M68K_MODE_EXCEPTION:
    (*ic->_tme_m68k_mode_exception)(ic);
    /* NOTREACHED */

  case TME_M68K_MODE_RTE:
    (*ic->_tme_m68k_mode_rte)(ic);
    /* NOTREACHED */

  case TME_M68K_MODE_STOP:
  case TME_M68K_MODE_HALT:
    tme_m68k_idle(ic);
    /* NOTREACHED */

  default:
    abort();
  }
  /* NOTREACHED */
}

/* the TLB filler for when we are on a generic bus: */
static int
_tme_m68k_generic_tlb_fill(struct tme_m68k_bus_connection *conn_m68k, 
			   struct tme_m68k_tlb *tlb,
			   unsigned int function_code, 
			   tme_uint32_t external_address, 
			   unsigned int cycles)
{
  struct tme_m68k *ic;

  /* recover our IC: */
  ic = conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* call the generic bus TLB filler: */
  (ic->_tme_m68k_bus_generic->tme_bus_tlb_fill)
    (ic->_tme_m68k_bus_generic,
     &tlb->tme_m68k_tlb_bus_tlb,
     external_address,
     cycles);
  
  /* when we're on a generic bus a TLB entry is valid for all function codes: */
  tlb->tme_m68k_tlb_function_codes_mask = -1;

  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_m68k_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_bus_connection *conn_bus;
  unsigned int score;

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_m68k = (struct tme_m68k_bus_connection *) conn->tme_connection_other;
  conn_bus = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {

    /* this must be a bus, and not another m68k chip: */
  case TME_CONNECTION_BUS_M68K:
    if (conn_bus->tme_bus_tlb_set_add != NULL
	&& conn_m68k->tme_m68k_bus_tlb_fill != NULL
	&& conn_m68k->tme_m68k_bus_m6888x_enable == NULL) {
      score = 10;
    }
    break;

    /* this must be a bus, and not another chip: */
  case TME_CONNECTION_BUS_GENERIC:
    if (conn_bus->tme_bus_tlb_set_add != NULL
	&& conn_bus->tme_bus_tlb_fill != NULL) {
      score = 1;
    }
    break;

  default: abort();
  }

  *_score = score;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_m68k_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_m68k *ic;
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn_other;
  struct tme_bus_tlb_set_info tlb_set_info;
  unsigned long tlb_i;
  struct tme_m68k_tlb *tlb;
  int rc;

  /* since the CPU is halted, it won't be making any connection calls,
     so we only have to do work when the connection is fully made: */
  if (state == TME_CONNECTION_FULL) {

    /* recover our IC: */
    ic = conn->tme_connection_element->tme_element_private;
    
    /* dispatch on the connection type: */
    conn_other = conn->tme_connection_other;
    conn_m68k = (struct tme_m68k_bus_connection *) conn_other;
    conn_bus = (struct tme_bus_connection *) conn_other;
    switch (conn->tme_connection_type) {
      
    case TME_CONNECTION_BUS_M68K:
      ic->_tme_m68k_bus_connection = conn_m68k;
      break;
      
      /* we need an adaptation layer: */
    case TME_CONNECTION_BUS_GENERIC:
      conn_m68k = tme_new0(struct tme_m68k_bus_connection, 1);
      conn_m68k->tme_m68k_bus_connection.tme_bus_connection.tme_connection_element = conn->tme_connection_element;
      conn_m68k->tme_m68k_bus_tlb_fill = _tme_m68k_generic_tlb_fill;
      ic->_tme_m68k_bus_connection = conn_m68k;
      ic->_tme_m68k_bus_generic = conn_bus;
      break;
      
    default: abort();
    }

    /* make the TLB set information: */
    memset(&tlb_set_info, 0, sizeof(tlb_set_info));
    tlb_set_info.tme_bus_tlb_set_info_token0 = &ic->_tme_m68k_tlb_array[0].tme_m68k_tlb_token;
    tlb_set_info.tme_bus_tlb_set_info_token_stride = sizeof(struct tme_m68k_tlb);
    tlb_set_info.tme_bus_tlb_set_info_token_count = TME_ARRAY_ELS(ic->_tme_m68k_tlb_array);
    tlb_set_info.tme_bus_tlb_set_info_bus_context = &ic->_tme_m68k_bus_context;

    /* initialize the TLBs in the set: */
    for (tlb_i = 0; tlb_i < TME_ARRAY_ELS(ic->_tme_m68k_tlb_array); tlb_i++) {
      tlb = &ic->_tme_m68k_tlb_array[tlb_i];

      /* initialize this token: */
      tme_token_init(&tlb->tme_m68k_tlb_token);

      /* connect this token with this TLB: */
      tlb->tme_m68k_tlb_bus_tlb.tme_bus_tlb_token = &tlb->tme_m68k_tlb_token;
    }

    /* add the TLB set: */
    rc = ((*ic->_tme_m68k_bus_connection->tme_m68k_bus_connection.tme_bus_tlb_set_add)
	  (&ic->_tme_m68k_bus_connection->tme_m68k_bus_connection,
	   &tlb_set_info));
    assert (rc == TME_OK);
  }

  /* NB: the machine needs to issue a reset to bring the CPU out of halt. */
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_m68k_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
  return (0);
}

/* this makes new connection sides: */
static int
_tme_m68k_connections_new(struct tme_element *element, const char * const *args, struct tme_connection **_conns, char **_output)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;

  /* if we already have a bus connection, we can take no more connections: */
  if (((struct tme_m68k *) element->tme_element_private)->_tme_m68k_bus_connection != NULL) {
    return (TME_OK);
  }

  /* create our side of an m68k bus connection: */
  conn_m68k = tme_new0(struct tme_m68k_bus_connection, 1);
  conn_bus = &conn_m68k->tme_m68k_bus_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_M68K;
  conn->tme_connection_score = _tme_m68k_connection_score;
  conn->tme_connection_make = _tme_m68k_connection_make;
  conn->tme_connection_break = _tme_m68k_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_signal = _tme_m68k_bus_signal;
  conn_bus->tme_bus_tlb_set_add = NULL;

  /* full in the m68k bus connection: */
  conn_m68k->tme_m68k_bus_interrupt = _tme_m68k_bus_interrupt;
  conn_m68k->tme_m68k_bus_tlb_fill = NULL;
  conn_m68k->tme_m68k_bus_m6888x_enable = _tme_m6888x_enable;

  /* add this connection to the set of possibilities: */
  *_conns = conn;

  /* create our side of a generic bus connection: */
  conn_bus = tme_new0(struct tme_bus_connection, 1);
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_m68k_connection_score;
  conn->tme_connection_make = _tme_m68k_connection_make;
  conn->tme_connection_break = _tme_m68k_connection_break;

  /* fill in the generic bus connection: */
  conn_bus->tme_bus_signal = _tme_m68k_bus_signal;
  conn_bus->tme_bus_tlb_set_add = NULL;
  conn_bus->tme_bus_tlb_fill = NULL;

  /* add this connection to the set of possibilities: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* the common m68k new function: */
int
tme_m68k_new(struct tme_m68k *ic, const char * const *args, const void *extra, char **_output)
{
  struct tme_element *element;
  int arg_i;
  int usage;

  /* check our arguments: */
  arg_i = 1;
  usage = FALSE;
  for (;;) {
    
    if (0) {

    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* this is either a bad argument or an FPU argument: */
    else {

      /* if this is not an FPU argument: */
      if (!tme_m68k_fpu_new(ic, args, &arg_i, &usage, _output)) {
	tme_output_append_error(_output,
				"%s %s, ",
				args[arg_i],
				_("unexpected"));
	usage = TRUE;
      }
      
      if (usage) {
	break;
      }
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s",
			    _("usage:"),
			    args[0]);
    tme_m68k_fpu_usage(_output);
    tme_free(ic);
    return (EINVAL);
  }

  /* initialize the verifier: */
  tme_m68k_verify_init();

  /* dispatch on the type: */
  switch (ic->tme_m68k_type) {
  case TME_M68K_M68000:
    ic->_tme_m68k_bus_16bit = 1;
    break;
  case TME_M68K_M68010:
    ic->_tme_m68k_bus_16bit = 1;
    break;
  case TME_M68K_M68020:
    ic->_tme_m68k_bus_16bit = 0;
    break;
  default:
    abort();
  }

  /* we have no bus connection yet: */
  ic->_tme_m68k_bus_connection = NULL;

  /* fill the element: */
  element = ic->tme_m68k_element;
  element->tme_element_private = ic;
  element->tme_element_connections_new = _tme_m68k_connections_new;

  /* calculate the instruction burst size: */
  /* XXX TBD: */
  ic->_tme_m68k_instruction_burst = 200;
  ic->_tme_m68k_instruction_burst_remaining
    = ic->_tme_m68k_instruction_burst;

  /* set the status register T bits mask: */
  ic->_tme_m68k_sr_mask_t
    = (TME_M68K_FLAG_T1
       | ((ic->tme_m68k_type >= TME_M68K_M68020)
	  * TME_M68K_FLAG_T0));

  /* initialize the small immediates: */
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_ZERO) = 0;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_ONE) = 1;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_TWO) = 2;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_THREE) = 3;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_FOUR) = 4;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_FIVE) = 5;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_SIX) = 6;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_SEVEN) = 7;
  ic->tme_m68k_ireg_uint32(TME_M68K_IREG_EIGHT) = 8;

  /* force the processor to be halted: */
  ic->_tme_m68k_mode = TME_M68K_MODE_HALT;
  TME_M68K_SEQUENCE_START;

  /* start the m68k thread: */
  tme_thread_create((tme_thread_t) tme_m68k_thread, ic);

  return (TME_OK);
}  

/* the common m68k reset function: */
void
tme_m68k_do_reset(struct tme_m68k *ic)
{
  
  /* force the VBR to zero: */
  ic->tme_m68k_ireg_vbr = 0;

  /* clear the E and F bits in the CACR: */
  ic->tme_m68k_ireg_cacr = 0;

  /* force supervisor mode, interrupts disabled: */
  tme_m68k_change_sr(ic, TME_M68K_FLAG_S | (7 << 8));

  /* load the initial SSP and PC: */
  ic->_tme_m68k_ea_function_code = TME_M68K_FC_SP;
  ic->_tme_m68k_ea_address = 0;
  tme_m68k_read_mem32(ic, TME_M68K_IREG_A7);
  ic->_tme_m68k_ea_address += sizeof(ic->tme_m68k_ireg_a7);
  tme_m68k_read_mem32(ic, TME_M68K_IREG_PC);

  /* clear all exceptions: */
  ic->_tme_m68k_exceptions = 0;

  /* reset the FPU: */
  tme_m68k_fpu_reset(ic);

  /* start execution: */
  ic->_tme_m68k_mode = TME_M68K_MODE_EXECUTION;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* this returns nonzero iff the slow instruction executor must be
   used: */
int
tme_m68k_go_slow(const struct tme_m68k *ic)
{
  tme_bus_context_t bus_context;
  const struct tme_m68k_tlb *tlb;
  tme_uint32_t linear_pc;
  const tme_shared tme_uint8_t *emulator_load;
  const tme_shared tme_uint8_t *emulator_load_last;

  bus_context = ic->_tme_m68k_bus_context;
  tlb = &ic->_tme_m68k_itlb;
  emulator_load = tlb->tme_m68k_tlb_emulator_off_read;
  emulator_load_last = emulator_load;
  if (emulator_load != TME_EMULATOR_OFF_UNDEF) {
    emulator_load += (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first;
    emulator_load_last += (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last;
    assert (emulator_load <= emulator_load_last);
  }
  linear_pc = ic->tme_m68k_ireg_pc;
  return (
	  
	  /* the ITLB entry must support reads from emulator memory: */
	  tme_m68k_tlb_is_invalid(tlb)
	  || tlb->tme_m68k_tlb_bus_context != bus_context
	  || (tlb->tme_m68k_tlb_function_codes_mask
	      & TME_BIT(TME_M68K_FUNCTION_CODE_PROGRAM(ic))) == 0
	  || linear_pc < (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first
	  || linear_pc > (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last
	  || tlb->tme_m68k_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF

	  /* the ITLB emulator memory must be 32-bit aligned for the
	     benefit of the fast instruction word fetch macros, so
	     that emulator address alignment goes with linear address
	     alignment: */
	  || (((unsigned long) tlb->tme_m68k_tlb_emulator_off_read)
	      & (sizeof(tme_uint32_t) - 1))

	  /* the ITLB emulator memory must not be so low that the
	     first valid pointer minus one, or the last valid pointer
	     minus (sizeof(tme_uint32_t) - 1), wraps around, nor so
	     high that the last valid pointer, plus one, wraps around: */
	  /* NB: this enables the fast instruction word fetch macros
	     to simply fetch 16 and 32 bit values until fetch_fast_next
	     is greater than ic->_tme_m68k_insn_fetch_fast_last, and 
	     not have to do any pointer math or ever check for pointer
	     wrapping: */
	  || ((emulator_load
	       - 1)
	      >= emulator_load)
	  || ((emulator_load_last
	       - (sizeof(tme_uint32_t) - 1))
	      >= emulator_load_last)
	  || ((emulator_load_last
	       + 1)
	      <= emulator_load_last)

	  /* the linear PC must be 16-bit aligned: */
	  || (linear_pc & 1)

	  /* there must be no tracing: */
	  || (ic->tme_m68k_ireg_sr & ic->_tme_m68k_sr_mask_t) != 0);
}

/* this redispatches: */
void
tme_m68k_redispatch(struct tme_m68k *ic)
{
  struct tme_m68k_tlb *tlb;

  /* if we have a busy fast instruction TLB entry: */
  tlb = ic->_tme_m68k_insn_fetch_fast_itlb;
  if (__tme_predict_true(tlb != NULL)) {

    /* unbusy and forget the fast instruction TLB entry: */
    tme_m68k_tlb_unbusy(tlb);
    ic->_tme_m68k_insn_fetch_fast_itlb = NULL;
  }

  /* do the redispatch: */
#ifdef _TME_M68K_STATS
  ic->tme_m68k_stats.tme_m68k_stats_redispatches++;
#endif /* _TME_M68K_STATS */
  longjmp(ic->_tme_m68k_dispatcher, 1);
}

/* this fills a TLB entry: */
void
tme_m68k_tlb_fill(struct tme_m68k *ic, struct tme_m68k_tlb *tlb, 
		  unsigned int function_code, 
		  tme_uint32_t linear_address, 
		  unsigned int cycles)
{
  tme_uint32_t external_address;
  struct tme_bus_tlb tlb_internal;
  
#ifdef _TME_M68K_STATS
  if (function_code == TME_M68K_FC_UP
      || function_code == TME_M68K_FC_SP) {
    ic->tme_m68k_stats.tme_m68k_stats_itlb_fill++;
  }
  else {
    ic->tme_m68k_stats.tme_m68k_stats_dtlb_fill++;
  }
#endif /* _TME_M68K_STATS */

  /* when emulating a CPU with a 16-bit bus, only 24 bits of address
     are external: */
  external_address = linear_address;
  if (ic->_tme_m68k_bus_16bit) {
    external_address &= 0x00ffffff;
  }

  /* unbusy the TLB entry: */
  tme_m68k_tlb_unbusy(tlb);

  /* clear any invalid token: */
  tme_token_invalid_clear(&tlb->tme_m68k_tlb_token);

  /* unlock for the callout: */
  tme_m68k_callout_unlock(ic);

  /* fill the TLB entry: */
  (*ic->_tme_m68k_bus_connection->tme_m68k_bus_tlb_fill)
    (ic->_tme_m68k_bus_connection, tlb,
     function_code,
     external_address,
     cycles);

  /* relock after the callout: */
  tme_m68k_callout_relock(ic);

  /* set the context on the TLB entry: */
  tlb->tme_m68k_tlb_bus_context = ic->_tme_m68k_bus_context;

  /* rebusy the TLB entry: */
  tme_m68k_tlb_busy(tlb);

  /* if this code isn't 32-bit clean, we have to deal: */
  if (external_address != linear_address) {
    tlb_internal.tme_bus_tlb_addr_first
      = (((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first)
	 | (linear_address ^ external_address));
    tlb_internal.tme_bus_tlb_addr_last
      = (((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last)
	 | (linear_address ^ external_address));
    tlb_internal.tme_bus_tlb_cycles_ok = tlb->tme_m68k_tlb_bus_tlb.tme_bus_tlb_cycles_ok;
    tme_bus_tlb_map(&tlb->tme_m68k_tlb_bus_tlb, external_address,
		    &tlb_internal, linear_address);
  }
}

/* this triggers exception processing: */
void
tme_m68k_exception(struct tme_m68k *ic, tme_uint32_t new_exceptions)
{
  assert(new_exceptions != 0);

  /* if the set of new exceptions includes a group zero exception: */
  if (new_exceptions & 
      (TME_M68K_EXCEPTION_RESET
       | TME_M68K_EXCEPTION_AERR
       | TME_M68K_EXCEPTION_BERR)) {
    
    /* there must be only one exception - you cannot trigger a group 0
       exception simultaneously with any other group 0, 1, or 2
       exception: */
    assert((new_exceptions & (new_exceptions - 1)) == 0);
    
    /* if this is a reset exception, it clears all other exceptions: */
    if (new_exceptions == TME_M68K_EXCEPTION_RESET) {
      ic->_tme_m68k_exceptions = 0;
    }

    /* otherwise, this is an address error or a bus error.  if we were
       already processing a group 0 exception, this is a
       double fault, and the processor enters the halted state: */
    else if (ic->_tme_m68k_exceptions &
	     (TME_M68K_EXCEPTION_RESET
	      | TME_M68K_EXCEPTION_AERR
	      | TME_M68K_EXCEPTION_BERR)) {
      tme_log(TME_M68K_LOG_HANDLE(ic), 0, TME_OK,
	      (TME_M68K_LOG_HANDLE(ic),
	       _("double fault, processor halted")));
      ic->_tme_m68k_mode = TME_M68K_MODE_HALT;
      TME_M68K_SEQUENCE_START;
      tme_m68k_redispatch(ic);
    }
  }

  /* otherwise, exception processing must not already be happening: */
  else {
    assert(ic->_tme_m68k_exceptions == 0);
  }

  /* begin exception processing: */
  ic->_tme_m68k_exceptions |= new_exceptions;
  ic->_tme_m68k_mode = TME_M68K_MODE_EXCEPTION;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* this changes SR, and swaps %a7 as needed: */
void
tme_m68k_change_sr(struct tme_m68k *ic, tme_uint16_t sr)
{
  tme_uint16_t flags_mode;

  /* only recognize the M bit on a 68020 or better: */
  flags_mode = (TME_M68K_FLAG_S
		| ((ic->tme_m68k_type >= TME_M68K_M68020)
		   * TME_M68K_FLAG_M));
  
  /* save %a7 in the proper stack pointer control register: */
  switch (ic->tme_m68k_ireg_sr & flags_mode) {
  case 0:
  case TME_M68K_FLAG_M:
    ic->tme_m68k_ireg_usp = ic->tme_m68k_ireg_a7;
    break;
  case TME_M68K_FLAG_S:
    ic->tme_m68k_ireg_isp = ic->tme_m68k_ireg_a7;
    break;
  case (TME_M68K_FLAG_S | TME_M68K_FLAG_M):
    ic->tme_m68k_ireg_msp = ic->tme_m68k_ireg_a7;
    break;
  }

  /* load %a7 from the proper stack pointer control register: */
  ic->tme_m68k_ireg_sr = sr;
  switch (ic->tme_m68k_ireg_sr & flags_mode) {
  case 0:
  case TME_M68K_FLAG_M:
    ic->tme_m68k_ireg_a7 = ic->tme_m68k_ireg_usp;
    break;
  case TME_M68K_FLAG_S:
    ic->tme_m68k_ireg_a7 = ic->tme_m68k_ireg_isp;
    break;
  case (TME_M68K_FLAG_S | TME_M68K_FLAG_M):
    ic->tme_m68k_ireg_a7 = ic->tme_m68k_ireg_msp;
    break;
  }
}

/* this starts processing an m68k exception: */
void
tme_m68k_exception_process_start(struct tme_m68k *ic, unsigned int ipl)
{
  tme_uint16_t sr;

  /* make an internal copy of the status register, then set S, clear
     T, and update I: */
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->tme_m68k_ireg_shadow_sr = ic->tme_m68k_ireg_sr;
    sr = (ic->tme_m68k_ireg_sr | TME_M68K_FLAG_S) & ~ic->_tme_m68k_sr_mask_t;
    if (ipl > TME_M68K_IPL_NONE) {
      assert(ipl == TME_M68K_IPL_NMI
	     || ipl > TME_M68K_FLAG_IPM(sr));
      sr = (sr & ~(TME_M68K_IPL_MAX << 8)) | (ipl << 8);
    }
    tme_m68k_change_sr(ic, sr);
  }
}

/* this finishes processing an m68k exception: */
void
tme_m68k_exception_process_finish(struct tme_m68k *ic, tme_uint8_t format, tme_uint8_t vector)
{
  tme_uint16_t vector_offset;

  /* stack the frame format and vector offset, unless this is a 68000: */
  vector_offset = ((tme_uint16_t) vector) << 2;
  if (ic->tme_m68k_type != TME_M68K_M68000) {
    tme_m68k_push16(ic, (((tme_uint16_t) format) << 12) | vector_offset);
  }

  /* stack the program counter: */
  tme_m68k_push32(ic, ic->tme_m68k_ireg_pc);
  
  /* stack the internal copy of the status register: */
  tme_m68k_push16(ic, ic->tme_m68k_ireg_shadow_sr);

  /* do a bus cycle to read the vector into the program counter: */
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->_tme_m68k_ea_function_code = TME_M68K_FC_SD;
    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_vbr + vector_offset;
  }
  tme_m68k_read_mem32(ic, TME_M68K_IREG_PC);
}

/* common m68000 and m68010 exception processing: */
void
tme_m68000_exception_process(struct tme_m68k *ic)
{
  tme_uint32_t exceptions;
  tme_uint8_t vector;

  /* get the set of exceptions.  we must have no group 0 exceptions: */
  exceptions = ic->_tme_m68k_exceptions;
  assert((exceptions & (TME_M68K_EXCEPTION_RESET
			| TME_M68K_EXCEPTION_AERR
			| TME_M68K_EXCEPTION_BERR)) == 0);

  /* these if statements are ordered to implement the priority
     relationship between the different exceptions as outlined in 
     the 68000 user's manual (pp 93 in my copy): */
  
  if (TME_M68K_EXCEPTION_IS_INST(exceptions)) {
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_EXCEPTION_IS_INST(exceptions));
  }
  
  if (exceptions & TME_M68K_EXCEPTION_TRACE) {
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_VECTOR_TRACE);
  }
  
  if (TME_M68K_EXCEPTION_IS_INT(exceptions)) {
    tme_m68k_exception_process_start(ic, TME_M68K_EXCEPTION_IS_INT(exceptions));
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_EXCEPTION_INT_VEC(exceptions));
  }
  
  if (exceptions & TME_M68K_EXCEPTION_ILL) {
    if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 12, 4) == 0xa) {
      vector = TME_M68K_VECTOR_LINE_A;
    }
    else if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 12, 4) == 0xf) {
      vector = TME_M68K_VECTOR_LINE_F;
    }
    else {
      vector = TME_M68K_VECTOR_ILL;
    }
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, vector);
  }
  
  if (exceptions & TME_M68K_EXCEPTION_PRIV) {
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_VECTOR_TRACE);
  }
  
  /* we have processed all exceptions - resume execution: */
  ic->_tme_m68k_exceptions = 0;
  ic->_tme_m68k_mode = TME_M68K_MODE_EXECUTION;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* common m68020 and later exception processing: */
void
tme_m68020_exception_process(struct tme_m68k *ic)
{
  tme_uint32_t exceptions;
  tme_uint8_t vector;
  struct {
    tme_uint16_t tme_m68k_fmt1_sr;
    tme_uint16_t tme_m68k_fmt1_pc_hi;
    tme_uint16_t tme_m68k_fmt1_pc_lo;    
    tme_uint16_t tme_m68k_fmt1_vector_offset;
  } fmt1;

  /* get the set of exceptions.  we must have no group 0 or 1
     exceptions: */
  exceptions = ic->_tme_m68k_exceptions;
  assert((exceptions & (TME_M68K_EXCEPTION_RESET
			| TME_M68K_EXCEPTION_AERR
			| TME_M68K_EXCEPTION_BERR)) == 0);

  /* these if statements are ordered to implement the priority
     relationship between the different exceptions as outlined in 
     the 68020 user's manual (pp 144 in my copy): */
  
  /* group 2 exceptions: */
  if (TME_M68K_EXCEPTION_IS_INST(exceptions)) {
    tme_m68k_exception_process_start(ic, 0);

    /* get the vector number: */
    vector = TME_M68K_EXCEPTION_IS_INST(exceptions);

    /* of the group 2 exceptions, only the Format Error and TRAP #N
       exceptions generate a format 0 stack frame.  the RTE mode code
       and the TRAP instruction code are expected to have left
       ic->tme_m68k_ireg_pc as the PC they want stacked: */
    if (vector == TME_M68K_VECTOR_FORMAT
	|| (TME_M68K_VECTOR_TRAP_0 <= vector
	    && vector < (TME_M68K_VECTOR_TRAP_0 + 16))) {
      tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, vector);
    }

    /* all other group 2 exceptions generate a format 2 stack frame.
       all code that can signal this exception is expected to have
       left ic->tme_m68k_ireg_pc *and* ic->tme_m68k_ireg_pc_last as
       the PCs they want stacked: */
    else {
      
      /* stack the program counter of the instruction that caused the exception: */
      tme_m68k_push32(ic, ic->tme_m68k_ireg_pc_last);

      /* finish with a format 2 stack frame: */
      tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_2, vector);
    }
  }

  /* group 3 exceptions: */
  if (exceptions & TME_M68K_EXCEPTION_ILL) {
    if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 12, 4) == 0xa) {
      vector = TME_M68K_VECTOR_LINE_A;
    }
    else if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 12, 4) == 0xf) {
      vector = TME_M68K_VECTOR_LINE_F;
    }
    else {
      vector = TME_M68K_VECTOR_ILL;
    }
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, vector);
  }
  if (exceptions & TME_M68K_EXCEPTION_PRIV) {
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_VECTOR_PRIV);
  }

  /* group 4.1 exceptions: */
  if (exceptions & TME_M68K_EXCEPTION_TRACE) {
    tme_m68k_exception_process_start(ic, 0);
    tme_m68k_push32(ic, ic->tme_m68k_ireg_pc_last);
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_2, TME_M68K_VECTOR_TRACE);
  }
  
  /* group 4.2 exceptions: */
  if (TME_M68K_EXCEPTION_IS_INT(exceptions)) {
    tme_m68k_exception_process_start(ic, TME_M68K_EXCEPTION_IS_INT(exceptions));
    tme_m68k_exception_process_finish(ic, TME_M68K_FORMAT_0, TME_M68K_EXCEPTION_INT_VEC(exceptions));

    /* if the M-bit is set: */
    if (ic->tme_m68k_ireg_sr & TME_M68K_FLAG_M) {

      /* make the throwaway four-word stack frame (format 1): */
      fmt1.tme_m68k_fmt1_vector_offset = tme_htobe_u16((TME_M68K_FORMAT_1 << 12) | (TME_M68K_EXCEPTION_INT_VEC(exceptions) << 2));
      fmt1.tme_m68k_fmt1_pc_lo = tme_htobe_u16((ic->tme_m68k_ireg_pc >>  0) & 0xffff);
      fmt1.tme_m68k_fmt1_pc_hi = tme_htobe_u16((ic->tme_m68k_ireg_pc >> 16) & 0xffff);
      fmt1.tme_m68k_fmt1_sr = tme_htobe_u16(ic->tme_m68k_ireg_sr);

      /* store the throwaway four-word stack frame on the interrupt stack: */
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	ic->_tme_m68k_ea_function_code = TME_M68K_FC_SD;
	ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_isp - sizeof(fmt1);
      }
      tme_m68k_write_mem(ic, (tme_uint8_t *) &fmt1, sizeof(fmt1));

      /* move to the interrupt stack: */
      ic->tme_m68k_ireg_isp -= sizeof(fmt1);
      tme_m68k_change_sr(ic, ic->tme_m68k_ireg_sr & ~TME_M68K_FLAG_M);
    }
  }
  
  /* we have processed all exceptions - resume execution: */
  ic->_tme_m68k_exceptions = 0;
  ic->_tme_m68k_mode = TME_M68K_MODE_EXECUTION;
  TME_M68K_SEQUENCE_START;
  tme_m68k_redispatch(ic);
}

/* this starts an m68k RTE: */
tme_uint16_t
tme_m68k_rte_start(struct tme_m68k *ic)
{

  /* set up to read from the stack frame: */
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->_tme_m68k_ea_function_code = TME_M68K_FC_SD;
    ic->_tme_m68k_ea_address = ic->tme_m68k_ireg_a7;
  }

  /* read the stacked status register: */
  tme_m68k_read_mem16(ic, TME_M68K_IREG_SHADOW_SR);
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->_tme_m68k_ea_address += sizeof(ic->tme_m68k_ireg_shadow_sr);
  }

  /* read the stacked PC: */
  tme_m68k_read_mem32(ic, TME_M68K_IREG_PC_NEXT);
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->_tme_m68k_ea_address += sizeof(ic->tme_m68k_ireg_pc_next);
  }

  /* read the stacked format/offset word, unless this is a 68000: */
  if (ic->tme_m68k_type != TME_M68K_M68000) {
    tme_m68k_read_mem16(ic, TME_M68K_IREG_FORMAT_OFFSET);
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      ic->_tme_m68k_ea_address += sizeof(ic->tme_m68k_ireg_format_offset);
    }
  }
  else {
    ic->tme_m68k_ireg_format_offset = 0;
  }

  /* return the frame format: */
  return (ic->tme_m68k_ireg_format_offset >> 12);
}

/* this finishes an m68k RTE: */
void
tme_m68k_rte_finish(struct tme_m68k *ic, tme_uint32_t format_extra)
{
  tme_uint32_t frame_size;

  /* calculate the total frame size.  the 68000 doesn't have a
     format/status word: */
  frame_size = (sizeof(ic->tme_m68k_ireg_shadow_sr)
		+ sizeof(ic->tme_m68k_ireg_pc_next)
		+ (ic->tme_m68k_type != TME_M68K_M68000
		   ? sizeof(ic->tme_m68k_ireg_format_offset)
		   : 0)
		+ format_extra);
  assert((frame_size & 1) == 0);

  /* adjust the stack: */
  ic->tme_m68k_ireg_a7 += frame_size;
  
  /* set the status register: */
  tme_m68k_change_sr(ic, ic->tme_m68k_ireg_shadow_sr);
		     
  /* set the PC: */
  ic->tme_m68k_ireg_pc = ic->tme_m68k_ireg_pc_next;
  
  /* redispatch: */
  tme_m68k_redispatch(ic);
}

/* this stores the group 0 sequence into a region of host memory.
   this is used when preparing the state information to be stored
   on the stack for a bus or address error: */
unsigned int
tme_m68k_sequence_empty(const struct tme_m68k *ic, tme_uint8_t *raw, unsigned int raw_avail)
{
  const struct _tme_m68k_sequence *sequence;
  unsigned int raw_used;
  
  /* get the group 0 sequence: */
  sequence = &ic->_tme_m68k_group0_sequence;
  raw_used = 0;

  /* we use 8 bits for the mode (2 bits) and flags (6 bits): */
  raw_used += sizeof(tme_uint8_t);
  assert(raw_avail >= raw_used);
  assert(sequence->_tme_m68k_sequence_mode < TME_BIT(2));
  assert(sequence->_tme_m68k_sequence_mode_flags < TME_BIT(6));
  *(raw++) = ((sequence->_tme_m68k_sequence_mode << 6)
	      | sequence->_tme_m68k_sequence_mode_flags);
  

  /* we use 16 bits for the faulted memory transfer ordinal
     (12 bits) and already-transferred byte count (4 bits): */
  raw_used += sizeof(tme_uint16_t);
  assert(raw_avail >= raw_used);
  assert(sequence->_tme_m68k_sequence_transfer_faulted < TME_BIT(12));
  assert(sequence->_tme_m68k_sequence_transfer_faulted_after < TME_BIT(4));
  *(raw++) = sequence->_tme_m68k_sequence_transfer_faulted >> 4;
  *(raw++) = ((sequence->_tme_m68k_sequence_transfer_faulted << 4)
	      | sequence->_tme_m68k_sequence_transfer_faulted_after);

#ifdef _TME_M68K_VERIFY
  /* we use sizeof(_tme_m68k_sequence_uid) bytes for the sequence UID: */
  raw_used += sizeof(sequence->_tme_m68k_sequence_uid);
  assert(raw_avail >= raw_used);
  memcpy(raw,
	 &sequence->_tme_m68k_sequence_uid,
	 sizeof(sequence->_tme_m68k_sequence_uid));
  raw += sizeof(sequence->_tme_m68k_sequence_uid);
#endif /* _TME_M68K_VERIFY */

  /* done: */
  return (raw_used);
}

/* this restores the group 0 sequence from a region of host memory.
   this is used when reading the state information stored on the
   stack for a bus or address error: */
unsigned int
tme_m68k_sequence_fill(struct tme_m68k *ic, const tme_uint8_t *raw, unsigned int raw_avail)
{
  struct _tme_m68k_sequence *sequence;
  unsigned int raw_used;
  
  /* get the group 0 sequence: */
  sequence = &ic->_tme_m68k_group0_sequence;
  raw_used = 0;

  /* we used 8 bits for the mode (2 bits) and flags (6 bits): */
  raw_used += sizeof(tme_uint8_t);
  if (raw_avail < raw_used) {
    return (0);
  }
  sequence->_tme_m68k_sequence_mode = *raw >> 6;
  sequence->_tme_m68k_sequence_mode_flags = (*(raw++) & (TME_BIT(6) - 1));

  /* we used 16 bits for the faulted memory transfer ordinal
     (12 bits) and already-transferred byte count (4 bits): */
  raw_used += sizeof(tme_uint16_t);
  if (raw_avail < raw_used) {
    return (0);
  }
  sequence->_tme_m68k_sequence_transfer_faulted = 
    (((tme_uint16_t) raw[0]) << 4)
    | (raw[1] >> 4);
  sequence->_tme_m68k_sequence_transfer_faulted_after = raw[1] & (TME_BIT(4) - 1);
  raw += sizeof(tme_uint16_t);

#ifdef _TME_M68K_VERIFY
  /* we used sizeof(_tme_m68k_sequence_uid) bytes for the sequence UID: */
  raw_used += sizeof(sequence->_tme_m68k_sequence_uid);
  if (raw_avail < raw_used) {
    return (0);
  }
  memcpy(&sequence->_tme_m68k_sequence_uid,
	 raw,
	 sizeof(sequence->_tme_m68k_sequence_uid));
  raw += sizeof(sequence->_tme_m68k_sequence_uid);
#endif /* _TME_M68K_VERIFY */

  /* initialize this to one: */
  sequence->_tme_m68k_sequence_transfer_next = 1;

  /* done: */
  return (raw_used);
}

/* this empties the instruction buffer into an exception frame: */
unsigned int
tme_m68k_insn_buffer_empty(const struct tme_m68k *ic, tme_uint8_t *raw, unsigned int raw_avail)
{
  unsigned int fetch_total;
  
  /* get the total number of bytes in the instruction buffer: */
  fetch_total = ic->_tme_m68k_insn_fetch_slow_count_total;

  /* save the total number of bytes fetched into the instruction
     buffer, the number of bytes in the instruction buffer fetched by
     the fast executor, and then the instruction buffer itself: */
  assert ((fetch_total % sizeof(tme_uint16_t)) == 0
	  && fetch_total <= (TME_M68K_INSN_WORDS_MAX * sizeof(tme_uint16_t)));
  assert ((ic->_tme_m68k_insn_fetch_slow_count_fast % sizeof(tme_uint16_t)) == 0
	  && ic->_tme_m68k_insn_fetch_slow_count_fast <= fetch_total);
  assert (raw_avail >= (sizeof(tme_uint8_t) + sizeof(tme_uint8_t) + fetch_total));
  raw[0] = fetch_total;
  raw[1] = ic->_tme_m68k_insn_fetch_slow_count_fast;
  memcpy(raw + 2,
	 &ic->_tme_m68k_insn_fetch_buffer[0],
	 fetch_total);
  
  /* return the number of bytes we put in an exception frame: */
  return (sizeof(tme_uint8_t) + sizeof(tme_uint8_t) + fetch_total);
}

/* this fills the instruction buffer from an exception frame: */
unsigned int
tme_m68k_insn_buffer_fill(struct tme_m68k *ic, const tme_uint8_t *raw, unsigned int raw_avail)
{
  unsigned int fetch_total;
  unsigned int fetch_fast;

  /* there must be at least two bytes in the exception frame: */
  if (raw_avail >= (sizeof(tme_uint8_t) + sizeof(tme_uint8_t))) {

    /* restore the total number of bytes fetched into the instruction
       buffer, and the number of bytes in the instruction buffer
       fetched by the fast executor: */
    fetch_total = raw[0];
    fetch_fast = raw[1];
    if ((fetch_total % sizeof(tme_uint16_t)) == 0
	&& fetch_total <= (TME_M68K_INSN_WORDS_MAX * sizeof(tme_uint16_t))
	&& (fetch_fast % sizeof(tme_uint16_t)) == 0
	&& fetch_fast <= fetch_total
	&& raw_avail >= (sizeof(tme_uint8_t) + sizeof(tme_uint8_t) + fetch_total)) {

      /* restore the total number of bytes fetched into the instruction
	 buffer, the number of bytes in the instruction buffer fetched by
	 the fast executor, and then the instruction buffer itself: */
      ic->_tme_m68k_insn_fetch_slow_count_total = fetch_total;
      ic->_tme_m68k_insn_fetch_slow_count_fast = fetch_fast;
      memcpy(&ic->_tme_m68k_insn_fetch_buffer[0],
	     raw + 2,
	     fetch_total);

      /* return the number of bytes restored from the exception frame: */
      return ((sizeof(tme_uint8_t) + sizeof(tme_uint8_t) + fetch_total));
    }
  }

  /* this exception frame is invalid: */
  return (0);
}

/* this unlocks data structures before a callout: */
void
tme_m68k_callout_unlock(struct tme_m68k *ic)
{
  struct tme_m68k_tlb *tlb;

  assert ((ic->_tme_m68k_mode == TME_M68K_MODE_EXECUTION)
	  || (ic->_tme_m68k_insn_fetch_fast_itlb == NULL));

  /* if we have a busy fast instruction TLB entry: */
  tlb = ic->_tme_m68k_insn_fetch_fast_itlb;
  if (tlb != NULL) {

    /* unbusy the fast instruction TLB entry: */
    tme_m68k_tlb_unbusy(tlb);
  }
}

/* this relocks data structures after a callout: */
void
tme_m68k_callout_relock(struct tme_m68k *ic)
{
  struct tme_m68k_tlb *tlb;
  tme_bus_context_t bus_context;
  struct tme_m68k_tlb *tlb_now;

  assert ((ic->_tme_m68k_mode == TME_M68K_MODE_EXECUTION)
	  || (ic->_tme_m68k_insn_fetch_fast_itlb == NULL));

  /* if we have a busy fast instruction TLB entry: */
  tlb = ic->_tme_m68k_insn_fetch_fast_itlb;
  if (tlb != NULL) {

    /* rebusy the fast instruction TLB entry: */
    tme_m68k_tlb_busy(tlb);
      
    /* get the bus context: */
    bus_context = ic->_tme_m68k_bus_context;

    /* get what should be our instruction TLB entry now: */
    tlb_now = &ic->_tme_m68k_itlb;

    /* if this instruction TLB entry has changed, is for the wrong
       context, or is invalid: */
    if (__tme_predict_false(tlb_now != tlb
			    || tlb->tme_m68k_tlb_bus_context != bus_context
			    || tme_m68k_tlb_is_invalid(tlb))) {

      /* poison ic->_tme_m68k_insn_fetch_fast_last so the fast
	 instruction executor fetch macros will fail: */
      assert ((ic->_tme_m68k_insn_fetch_fast_next - 1) < ic->_tme_m68k_insn_fetch_fast_next);
      ic->_tme_m68k_insn_fetch_fast_last = ic->_tme_m68k_insn_fetch_fast_next - 1;
    }
  }
}

/* this is the group 0 fault hook for the fast executor: */
void
tme_m68k_group0_hook_fast(struct tme_m68k *ic)
{
  unsigned int fetch_fast;

  /* get the number of bytes in the instruction buffer.  they have all
     been fetched by the fast executor: */
  /* NB: it's possible for this to be zero: */
  fetch_fast = (ic->_tme_m68k_insn_fetch_fast_next - ic->_tme_m68k_insn_fetch_fast_start);
  assert ((fetch_fast % sizeof(tme_uint16_t)) == 0
	  && fetch_fast <= (TME_M68K_INSN_WORDS_MAX * sizeof(tme_uint16_t)));
  ic->_tme_m68k_insn_fetch_slow_count_total = fetch_fast;
  ic->_tme_m68k_insn_fetch_slow_count_fast = fetch_fast;
}

/* this starts a read/modify/write cycle: */
int
tme_m68k_rmw_start(struct tme_m68k *ic,
		   struct tme_m68k_rmw *rmw)
{
  tme_bus_context_t bus_context;
  struct tme_m68k_tlb *tlbs_all[3];
  int tlbs_busy[2];
  struct tme_m68k_tlb *tlb;
  struct tme_m68k_tlb *tlb_use;
  unsigned int tlb_i;
  unsigned int address_i;
  unsigned int address_i_fill;
  tme_uint32_t address;
  unsigned int address_cycles[2];
  unsigned int address_fills[2];
  tme_uint32_t *buffer_reg;
  int supported;

  /* if the user reran the cycle: */
  if (TME_M68K_SEQUENCE_RESTARTING
      && (ic->_tme_m68k_group0_buffer_read_softrr > 0
	  || ic->_tme_m68k_group0_buffer_write_softrr > 0)) {

    /* return failure: */
    return (-1);
  }

  /* we always rerun read/modify/write cycles in their entirety: */
  ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_faulted
    = ic->_tme_m68k_sequence._tme_m68k_sequence_transfer_next - 1;

  /* we only support tas and cas, which have one address, and cas2,
     which has two addresses: */
  assert (rmw->tme_m68k_rmw_address_count == 1
	  || rmw->tme_m68k_rmw_address_count == 2);

  /* get the context that we will use to index TLB entries for this
     instruction.  NB that this may be different from the context in
     which the instruction eventually completes: */
  bus_context = ic->_tme_m68k_bus_context;

  /* assume that we will only consider one TLB entry, for the first
     address: */
  tlbs_all[0] = TME_M68K_DTLB_ENTRY(ic,
				    bus_context,
				    ic->_tme_m68k_ea_function_code,
				    rmw->tme_m68k_rmw_addresses[0]);
  tlbs_all[1] = NULL;

  /* if there are two addresses: */
  if (rmw->tme_m68k_rmw_address_count == 2) {

    /* we will consider another TLB entry for the second address: */
    tlbs_all[1] = TME_M68K_DTLB_ENTRY(ic,
				      bus_context,
				      ic->_tme_m68k_ea_function_code,
				      rmw->tme_m68k_rmw_addresses[1]);

    /* if the TLB entry for the second address collides with the TLB
       entry for the first address: */
    if (tlbs_all[1] == tlbs_all[0]) {

      /* we will instead consider an alternate TLB entry for the
         second address: */
      tlbs_all[1] = TME_M68K_DTLB_ENTRY(ic,
					bus_context,
					ic->_tme_m68k_ea_function_code,
					(rmw->tme_m68k_rmw_addresses[1]
					 + TME_M68K_TLB_ADDRESS_BIAS(1)));
      assert (tlbs_all[1] != tlbs_all[0]);
    }
  }

  /* make sure that the list of TLB entries to consider is terminated: */
  tlbs_all[2] = NULL;

  /* none of the TLB entries to consider are busy: */
  tlbs_busy[0] = FALSE;
  tlbs_busy[1] = FALSE;

  /* the addresses aren't using any TLB entries yet: */
  rmw->tme_m68k_rmw_tlbs[0] = NULL;
  rmw->tme_m68k_rmw_tlbs[1] = NULL;

  /* we haven't done any slow reads for any addresses yet: */
  rmw->tme_m68k_rmw_slow_reads[0] = FALSE;
  rmw->tme_m68k_rmw_slow_reads[1] = FALSE;

  /* whenever we need to find a TLB entry to use for an address, we
     always prefer one that allows both reading and writing, because
     we hope that such a TLB entry allows both fast reading and fast
     writing.

     if we can't find such a TLB entry initially, we try to fill a TLB
     entry for writing (you can't fill a TLB entry for both reading
     and writing), in the hopes that this gives us a TLB entry that
     allows both fast reading and fast writing.  filling for writing
     is important with some virtual memory hardware, and may actually
     be required to enable writing.

     if this fill gives us a TLB entry that doesn't allow both fast
     reading and fast writing, it actually might not allow reading at
     all.  to check for this, we then try to fill a TLB entry for
     reading.

     if we still don't have a TLB entry that allows both fast reading
     and fast writing, we must at least have a TLB entry that allows
     slow reading.  at this point we do a slow read to start a locked
     read-modify-write cycle (unless this is a cas2, in which case we
     do a normal slow read).

     we always want to return to the caller with a TLB entry that
     allows writing, so after we do a slow read we do one more TLB
     fill for writing.

     the first TLB fill we do for an address will be for writing, so
     that is how we initialize an address' address_cycles mask: */
  address_cycles[0] = TME_BUS_CYCLE_WRITE;
  address_cycles[1] = TME_BUS_CYCLE_WRITE;

  /* we haven't filled TLBs for any addresses yet: */
  address_fills[0] = 0;
  address_fills[1] = 0;

  /* assume that we can support this instruction on the given memory: */
  supported = TRUE;  

  /* loop forever: */
  for (;;) {

    /* assume that no address needs a TLB fill: */
    address_i_fill = rmw->tme_m68k_rmw_address_count;

    /* get the bus context for this iteration: */
    bus_context = ic->_tme_m68k_bus_context;

    /* walk the addresses: */
    address_i = 0;
    do {
      
      /* get this address: */
      address = rmw->tme_m68k_rmw_addresses[address_i];

      /* this address isn't using a TLB entry yet: */
      tlb_use = NULL;

      /* walk the TLB entries we are considering: */
      for (tlb_i = 0; 
	   (tlb = tlbs_all[tlb_i]) != NULL;
	   tlb_i++) {

	/* if this TLB entry isn't busy, busy it: */
	if (!tlbs_busy[tlb_i]) {
	  tme_m68k_tlb_busy(tlb);
	  tlbs_busy[tlb_i] = TRUE;
	}

	/* if this TLB entry is valid, applies to this context, function code
	   and address, and allows at least the desired cycle(s), and
	   either this address isn't already using a TLB entry, or the
	   TLB entry it's using doesn't cover the entire operand, or
	   this TLB entry allows more cycles or allows both fast
	   reading and fast writing: */
	if (tme_m68k_tlb_is_valid(tlb)
	    && tlb->tme_m68k_tlb_bus_context == bus_context
	    && (tlb->tme_m68k_tlb_function_codes_mask
		& TME_BIT(ic->_tme_m68k_ea_function_code)) != 0
	    && address >= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_first
	    && address <= (tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last
	    && (tlb->tme_m68k_tlb_cycles_ok
		& address_cycles[address_i]) != 0
	    && (tlb_use == NULL
		|| (((tme_bus_addr32_t) tlb_use->tme_m68k_tlb_linear_last) - address) < rmw->tme_m68k_rmw_size
		|| tlb->tme_m68k_tlb_cycles_ok > tlb_use->tme_m68k_tlb_cycles_ok
		|| (tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
		    && tlb->tme_m68k_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF))) {

	  /* update the TLB entry this address is using: */
	  tlb_use = tlb;
	}
      }

      /* set the TLB entry being used by this address: */
      rmw->tme_m68k_rmw_tlbs[address_i] = tlb_use;

      /* if this address is not using any TLB entry: */
      if (tlb_use == NULL) {

	/* we need to fill a TLB entry for this address: */
	address_i_fill = address_i;
      }

    } while (++address_i < rmw->tme_m68k_rmw_address_count);

    /* if we need to fill a TLB entry for an address: */
    address_i = address_i_fill;
    if (address_i < rmw->tme_m68k_rmw_address_count) {

      /* get this address: */
      address = rmw->tme_m68k_rmw_addresses[address_i];

      /* get an unused TLB entry to fill: */
      tlb_i = 0;
      tlb = tlbs_all[0];
      if (tlb == rmw->tme_m68k_rmw_tlbs[!address_i]) {
	tlb_i = 1;
	tlb = tlbs_all[1];
      }
      assert (tlb != NULL
	      && tlb != rmw->tme_m68k_rmw_tlbs[!address_i]);

      /* NB: cas2 can need two TLB entries.  we may find one good TLB
	 entry for one address, but need to call out to fill a TLB for
	 the second address, and unfortunately we have to unbusy the
	 good one while we're doing the fill.  while the good one is
	 unbusy, it can be invalidated, and we'll have to fill it
	 again, unbusying the good one we just filled, possibly
	 leading to a vicious cycle.

	 it's also possible that the TLB entry we fill here could be
	 invalidated after it's been filled and before we've busied it
	 again.  this is also the case for the single-TLB operations:
	 normal memory reads and writes, and tas and cas, and to
	 handle that we simply loop around the fill.  since these
	 operations only use a single TLB entry, we assume that there
	 won't be a vicious cycle - that eventually a single filled
	 TLB entry will stay valid until we can busy it and use it.

	 but we can't really guarantee this for two TLB entries.
	 there's not much we can do about this, except put a limit on
	 the number of times we will fill for each address.  this
	 limit is somewhat arbitrary: */
      /* XXX FIXME - this should be a macro, or a per-m68k argument: */
      if (rmw->tme_m68k_rmw_address_count == 2
	  && address_fills[address_i]++ >= 20) {

	/* we can't support this instruction on this memory: */
	supported = FALSE;
	break;
      }

      /* if the other TLB entry is busy, unbusy it: */
      if (tlbs_busy[!tlb_i]) {
	tme_m68k_tlb_unbusy(tlbs_all[tlb_i]);
	tlbs_busy[!tlb_i] = FALSE;
      }

      /* fill this TLB entry: */
      tme_m68k_tlb_fill(ic,
			tlb,
			ic->_tme_m68k_ea_function_code,
			address,
			address_cycles[address_i]);

      /* restart: */
      continue;
    }

    /* walk the addresses: */
    address_i = 0;
    do {

      /* get this address and its TLB entry: */
      address = rmw->tme_m68k_rmw_addresses[address_i];
      tlb = rmw->tme_m68k_rmw_tlbs[address_i];

      /* if this TLB entry doesn't cover the entire operand: */
      if ((((tme_bus_addr32_t) tlb->tme_m68k_tlb_linear_last) - address) < rmw->tme_m68k_rmw_size) {

	/* we can't support this instruction on this memory, because
	   we can't split an atomic operation across TLB entries.  on
	   a real m68k, the CPU can do repeated bus cycles under one
	   bus lock: */
	supported = FALSE;
	break;
      }

      /* if this TLB entry supports both fast reading and fast
         writing: */
      if (tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
	  && tlb->tme_m68k_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {

	/* if fast reading and fast writing aren't to the same memory: */
	if (tlb->tme_m68k_tlb_emulator_off_read
	    != tlb->tme_m68k_tlb_emulator_off_write) {
	  
	  /* we can't support this instruction on this memory, because
	     we can't split an atomic operation across two memories.
	     on a real m68k, the CPU can do repeated bus cycles under
	     one bus lock: */
	  supported = FALSE;
	  break;
	}
      }

      /* otherwise, this TLB entry does not support both fast reading
	 and fast writing: */

      /* if we have already done a slow read for this address: */
      else if (rmw->tme_m68k_rmw_slow_reads[address_i]) {

	/* this TLB entry must support writing: */
	assert (tlb->tme_m68k_tlb_cycles_ok & TME_BUS_CYCLE_WRITE);

	/* nothing to do: */
      }

      /* otherwise, we have not already done a slow read for this
         address: */

      /* if this TLB entry doesn't support slow reading: */
      else if ((tlb->tme_m68k_tlb_cycles_ok & TME_BUS_CYCLE_READ) == 0) {

	/* we must fill a TLB entry for reading: */
	assert (address_cycles[address_i] == TME_BUS_CYCLE_WRITE);
	address_cycles[address_i] = TME_BUS_CYCLE_READ;

	/* restart: */
	break;
      }

      /* otherwise, this TLB entry does support slow reading: */
      else {

	/* if the other TLB entry is busy, unbusy it: */
	tlb_i = (tlb == tlbs_all[1]);
	if (tlbs_busy[!tlb_i]) {
	  tme_m68k_tlb_unbusy(tlbs_all[tlb_i]);
	  tlbs_busy[!tlb_i] = FALSE;
	}

	/* this instruction can fault: */
	TME_M68K_INSN_CANFAULT;

	/* do a slow read.  if this is the first address, we start a
	   slow read-modify-write cycle, otherwise we do a normal slow
	   read cycle: */
	assert (rmw->tme_m68k_rmw_size <= sizeof(ic->tme_m68k_ireg_memx32));
	tme_m68k_read(ic,
		      tlb,
		      &ic->_tme_m68k_ea_function_code,
		      &rmw->tme_m68k_rmw_addresses[address_i],
		      (((tme_uint8_t *) 
			(address_i == 0
			 ? &ic->tme_m68k_ireg_memx32
			 : &ic->tme_m68k_ireg_memy32))
		       + (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
			  ? (sizeof(ic->tme_m68k_ireg_memx32)
			     - rmw->tme_m68k_rmw_size)
			  : 0)),
		      rmw->tme_m68k_rmw_size,
		      (address_i == 0
		       ? TME_M68K_BUS_CYCLE_RMW
		       : TME_M68K_BUS_CYCLE_NORMAL));

	/* we have done a slow read for this address: */
	rmw->tme_m68k_rmw_slow_reads[address_i] = TRUE;

	/* now we need a TLB entry for this address that supports writing: */
	address_cycles[address_i] = TME_BUS_CYCLE_WRITE;

	/* restart: */
	break;
      }

    } while (++address_i < rmw->tme_m68k_rmw_address_count);

    /* if this instruction is not supported or we've handled all
       addresses, stop now: */
    if (!supported
	|| address_i >= rmw->tme_m68k_rmw_address_count) {
      break;
    }
  }

  /* unbusy any TLB entries that aren't being used: */
  if (tlbs_busy[0]
      && (!supported
	  || (tlbs_all[0] != rmw->tme_m68k_rmw_tlbs[0]
	      && tlbs_all[0] != rmw->tme_m68k_rmw_tlbs[1]))) {
    tme_m68k_tlb_unbusy(tlbs_all[0]);
  }
  if (tlbs_busy[1]
      && (!supported
	  || (tlbs_all[1] != rmw->tme_m68k_rmw_tlbs[0]
	      && tlbs_all[1] != rmw->tme_m68k_rmw_tlbs[1]))) {
    tme_m68k_tlb_unbusy(tlbs_all[1]);
  }

  /* if this instruction is not supported on this memory: */
  if (!supported) {

    /* cause an illegal instruction exception: */
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }

  /* if this is the cas2 instruction: */
  if (rmw->tme_m68k_rmw_address_count == 2) {

    /* cas2 is a difficult instruction to emulate, since it accesses
       two different addresses during one atomic read-modify-write
       cycle.

       most host CPUs can't do this, so when threads are not
       cooperative, we're forced to suspend all other threads when
       running a cas2 instruction: */
    if (!TME_THREADS_COOPERATIVE) {
      tme_thread_suspend_others();
    }

    /* the cas2 functions also assume that we have read all operands
       into the memory buffers, which means we have to fast-read any
       addresses that we haven't already slow-read: */
    address_i = 0;
    do {

      /* skip this address if we really did slow read it: */
      if (rmw->tme_m68k_rmw_slow_reads[address_i]) {
	continue;
      }

      /* get this address and its TLB entry: */
      address = rmw->tme_m68k_rmw_addresses[address_i];
      tlb = rmw->tme_m68k_rmw_tlbs[address_i];

      /* this TLB entry must support fast reading and fast writing: */
      assert (tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
	      && tlb->tme_m68k_tlb_emulator_off_write == tlb->tme_m68k_tlb_emulator_off_read);

      /* do the fast read.  all other threads are suspended here, so
	 we can do a memcpy instead of an atomic read: */
      assert (rmw->tme_m68k_rmw_size <= sizeof(ic->tme_m68k_ireg_memx32));
      buffer_reg
	= (address_i == 0
	   ? &ic->tme_m68k_ireg_memx32
	   : &ic->tme_m68k_ireg_memy32);
      memcpy((((tme_uint8_t *) buffer_reg)
	      + (sizeof(ic->tme_m68k_ireg_memx32)
		 - rmw->tme_m68k_rmw_size)),
	     (((tme_uint8_t *)
	       tlb->tme_m68k_tlb_emulator_off_read)
	      + address),
	     rmw->tme_m68k_rmw_size);

      /* byteswap the value read: */
      *buffer_reg = tme_betoh_u32(*buffer_reg);
    
    } while (++address_i < rmw->tme_m68k_rmw_address_count);
  }

  /* return success: */
  return (0);
}

/* this finishes a read/modify/write cycle: */
void
tme_m68k_rmw_finish(struct tme_m68k *ic, 
		    struct tme_m68k_rmw *rmw,
		    int do_write)
{
  struct tme_m68k_tlb *tlbs_all[2];
  int tlbs_busy[2];
  struct tme_m68k_tlb *tlb;
  unsigned int tlb_i;
  unsigned int address_i;
  tme_uint32_t address;
  int supported;
  tme_uint32_t *buffer_reg;

  /* recover the tlbs_all[] array and tlbs_busy[] information: */
  tlbs_all[0] = rmw->tme_m68k_rmw_tlbs[0];
  tlbs_busy[0] = TRUE;
  if (rmw->tme_m68k_rmw_tlbs[1] != NULL
      && rmw->tme_m68k_rmw_tlbs[1] != rmw->tme_m68k_rmw_tlbs[0]) {
    tlbs_all[1] = rmw->tme_m68k_rmw_tlbs[1];
    tlbs_busy[1] = TRUE;
  }
  else {
    tlbs_all[1] = NULL;
    tlbs_busy[1] = FALSE;
  }

  /* assume that this instruction is supported: */
  supported = TRUE;

  /* loop over the addresses: */
  address_i = 0;
  do {

    /* get this address and TLB entry: */
    address = rmw->tme_m68k_rmw_addresses[address_i];
    tlb = rmw->tme_m68k_rmw_tlbs[address_i];

    /* get the buffer for this address: */
    buffer_reg
      = (address_i == 0
	 ? &ic->tme_m68k_ireg_memx32
	 : &ic->tme_m68k_ireg_memy32);

    /* if we did a slow read for this operand: */
    if (rmw->tme_m68k_rmw_slow_reads[address_i]) {

      /* if the other TLB entry is busy, unbusy it: */
      tlb_i = (tlb == tlbs_all[1]);
      if (tlbs_busy[!tlb_i]) {
	tme_m68k_tlb_unbusy(tlbs_all[tlb_i]);
	tlbs_busy[!tlb_i] = FALSE;
      }

      /* do the slow write for this operand: */
      assert (rmw->tme_m68k_rmw_size <= sizeof(ic->tme_m68k_ireg_memx32));
      tme_m68k_write(ic,
		     tlb,
		     &ic->_tme_m68k_ea_function_code,
		     &rmw->tme_m68k_rmw_addresses[address_i],
		     (((tme_uint8_t *) buffer_reg)
		      + (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
			 ? (sizeof(ic->tme_m68k_ireg_memx32)
			    - rmw->tme_m68k_rmw_size)
			 : 0)),
		     rmw->tme_m68k_rmw_size,
		     (address_i == 0
		      ? TME_M68K_BUS_CYCLE_RMW
		      : TME_M68K_BUS_CYCLE_NORMAL));

      /* if this is the cas2 instruction: */
      if (rmw->tme_m68k_rmw_address_count == 2) {

	/* if a cas2 slow write doesn't fault, it just did a slow
	   write to device memory, which is actually bad because we
	   can't do an atomic cas2 involving any device memory at all
	   (we can't do the dual reads and dual writes all atomically).

	   we tried to do the slow write anyways hoping that the slow
	   write was really to write-protected memory that would
	   fault, and when we would restart this address would point
	   to fast-writable memory.

	   unfortunately, we can't undo the slow write.  we do cause
	   an illegal instruction exception, to make this problem
	   visible: */
	supported = FALSE;
	break;
      }
    }

    /* otherwise, if this is the cas2 instruction, and we're writing: */
    else if (rmw->tme_m68k_rmw_address_count == 2
	     && do_write) {

      /* this TLB entry must support fast reading and fast writing: */
      assert (tlb->tme_m68k_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF
	      && tlb->tme_m68k_tlb_emulator_off_write == tlb->tme_m68k_tlb_emulator_off_read);

      /* byteswap the value to write: */
      *buffer_reg = tme_htobe_u32(*buffer_reg);

      /* do the fast write.  all other threads are suspended here, so
	 we can do a memcpy instead of an atomic write: */
      assert (rmw->tme_m68k_rmw_size <= sizeof(ic->tme_m68k_ireg_memx32));
      memcpy((((tme_uint8_t *)
	       tlb->tme_m68k_tlb_emulator_off_read)
	      + address),
	     (((tme_uint8_t *) buffer_reg)
	      + (sizeof(ic->tme_m68k_ireg_memx32)
		 - rmw->tme_m68k_rmw_size)),
	     rmw->tme_m68k_rmw_size);
    }

  } while (++address_i < rmw->tme_m68k_rmw_address_count);

  /* unbusy all TLB entries: */
  if (tlbs_busy[0]) {
    tme_m68k_tlb_unbusy(tlbs_all[0]);
  }
  if (tlbs_busy[1]) {
    tme_m68k_tlb_unbusy(tlbs_all[1]);
  }

  /* cas2 is a difficult instruction to emulate, since it accesses two
     different addresses during one atomic read-modify-write cycle.
     most host CPUs can't do this, so when threads are not
     cooperative, we're forced to suspend all other threads when
     running a cas2 instruction: */
  if (!TME_THREADS_COOPERATIVE
      && rmw->tme_m68k_rmw_address_count > 1) {
    tme_thread_resume_others();
  }

  /* if this instruction is not supported on this memory: */
  if (!supported) {

    /* cause an illegal instruction exception: */
    TME_M68K_INSN_EXCEPTION(TME_M68K_EXCEPTION_ILL);
  }
}

/* this handles a bitfield offset.  if the bitfield is in memory,
   and it hasn't already been done, this adjusts the effective
   address to point to the beginning of the bitfield.  this always
   returns a nonnegative bitfield offset: */
unsigned int
tme_m68k_bitfield_offset(struct tme_m68k *ic, int adjust)
{
  tme_int16_t specop;
  tme_int32_t bf_offset;
  tme_int32_t bf_ea_offset;
    
  /* get the bitfield offset from a data register or as an immediate: */
  specop = ic->_tme_m68k_insn_specop;
  bf_offset = ((specop & TME_BIT(11))
	       ? ic->tme_m68k_ireg_int32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specop, 6, 3))
	       : (tme_int32_t) TME_FIELD_EXTRACTU(specop, 6, 5));

  /* if this bitfield is in a register (EA mode field is zero): */
  if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 3, 3) == 0) {

    /* adjust the bitfield offset to be nonnegative: */
    bf_offset &= 31;
  }

  /* otherwise, this bitfield is in memory: */
  else {

    /* calculate the effective address offset and adjust the bitfield
       offset to be nonnegative: */
    bf_ea_offset = ((bf_offset < 0
		     ? (bf_offset - 7)
		     : bf_offset)
		    / 8);
    bf_offset &= 7;

    /* if this is our first call to this function for this instruction
       and we're not restarting, adjust the effective address: */
    if (adjust
	&& !TME_M68K_SEQUENCE_RESTARTING) {
      ic->_tme_m68k_ea_address += bf_ea_offset;
    }
  }

  /* return the nonnegative bitfield offset: */
  return ((unsigned int) bf_offset);
}

/* this returns a bitfield width: */
unsigned int
tme_m68k_bitfield_width(struct tme_m68k *ic)
{
  unsigned int bf_width;
  tme_int16_t specop;

  /* get the bitfield width from a register or as an immediate: */
  specop = ic->_tme_m68k_insn_specop;
  if (specop & TME_BIT(5)) {
    bf_width = ic->tme_m68k_ireg_uint32(TME_M68K_IREG_D0 + TME_FIELD_EXTRACTU(specop, 0, 3));
    bf_width &= 31;
  }
  else {
    bf_width = TME_FIELD_EXTRACTU(specop, 0, 5);
  }
  if (bf_width == 0) bf_width = 32;
  return (bf_width);
}

/* this reads a bitfield: */
tme_uint32_t
_tme_m68k_bitfield_read(struct tme_m68k *ic, int is_signed)
{
  unsigned int bf_offset, bf_width;
  unsigned int shift;
  tme_uint8_t *bf_bytes;
  tme_uint32_t bf_value;
  int ireg;

  /* get the bitfield offset and width: */
  bf_offset = tme_m68k_bitfield_offset(ic, TRUE);
  bf_width = tme_m68k_bitfield_width(ic);

  /* if this expression is > 32, in a register this means the bitfield
     wraps, and in memory this means the bitfield covers 5 bytes: */
  shift = (bf_offset + bf_width);

  /* if this bitfield is in a register (EA mode field is zero): */
  if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 3, 3) == 0) {
    ireg = (TME_M68K_IREG_D0
	    + TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 0, 3));

    /* get the raw 32-bit word containing the bitfield: */
    bf_value = ic->tme_m68k_ireg_uint32(ireg);

    /* if this bitfield wraps the register, shift in the wrapped part
       on the right: */
    if (shift > 32) {
      shift -= 32;
      bf_value = (bf_value << shift) | (bf_value >> (32 - shift));
      bf_offset -= shift;
    }
  }

  /* otherwise, this bitfield is in memory: */
  else {

    /* this instruction can fault: */
    ic->_tme_m68k_mode_flags |= TME_M68K_EXECUTION_INST_CANFAULT;

    /* read in the bytes covering the bitfield: */
    bf_bytes = (tme_uint8_t *) &ic->tme_m68k_ireg_memx32;
    tme_m68k_read_mem(ic, bf_bytes, (bf_offset + bf_width + 7) / 8);

    /* get the raw 32-bit word containing the bitfield: */
    bf_value = tme_betoh_u32(ic->tme_m68k_ireg_memx32);

    /* if this bitfield covers 5 bytes, shift in the part from the fifth byte
       (actually in memy32!) on the right: */
    if (shift > 32) {
      shift -= 32;
      bf_value = (bf_value << shift) | (bf_bytes[4] >> (8 - shift));
      bf_offset -= shift;
    }
  }
  
  /* shift the value: */
  shift = (32 - (bf_offset + bf_width));
  bf_value >>= shift;

  /* mask the value: */
  bf_value &= (0xffffffffUL >> (32 - bf_width));

  /* if this is a signed value, sign-extend it: */
  if (is_signed
      && (bf_value & TME_BIT(bf_width - 1))) {
    bf_value |= (0xffffffffUL << (bf_width - 1));
  }

  /* all bitfield instructions that read the bitfield set the flags: */
  if (!TME_M68K_SEQUENCE_RESTARTING) {
    ic->tme_m68k_ireg_ccr = ((ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X)
			     | ((bf_value & TME_BIT(bf_width - 1))
				? TME_M68K_FLAG_N
				: 0)
			     | (bf_value
				? 0
				: TME_M68K_FLAG_Z));
  }

  /* return the bitfield value: */
  return (bf_value);
}

/* this writes a bitfield to memory: */
void
tme_m68k_bitfield_write_unsigned(struct tme_m68k *ic, tme_uint32_t bf_value, int set_flags)
{
  unsigned int bf_offset, bf_width;
  unsigned int shift;
  tme_uint8_t *bf_bytes;
  unsigned int count;
  int ireg;

  /* for bitfields in memory, we want to know if the memory covering
     the bitfield is already in our memory buffer, so we can avoid
     reading that memory again.  all bitfield instructions set flags
     based on a bitfield value; if set_flags is FALSE our caller
     must have tested the old bitfield value, and so the bitfield
     memory must be in our buffer, otherwise assume that this is our
     first access to the bitfield memory: */
#define first_memory set_flags
  
  /* get the bitfield offset and width: */
  bf_offset = tme_m68k_bitfield_offset(ic, first_memory);
  bf_width = tme_m68k_bitfield_width(ic);

  /* if this expression is > 32, in a register this means the bitfield
     wraps, and in memory this means the bitfield covers 5 bytes: */
  shift = (bf_offset + bf_width);

  /* mask the value: */
  bf_value &= (0xffffffffUL >> (32 - bf_width));

  /* if we're supposed to, set the flags: */
  if (set_flags
      && !TME_M68K_SEQUENCE_RESTARTING) {
    ic->tme_m68k_ireg_ccr = ((ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X)
			     | ((bf_value & TME_BIT(bf_width - 1))
				? TME_M68K_FLAG_N
				: 0)
			     | (bf_value
				? 0
				: TME_M68K_FLAG_Z));
  }

  /* if this bitfield is in a register (EA mode field is zero): */
  if (TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 3, 3) == 0) {
    ireg = (TME_M68K_IREG_D0
	    + TME_FIELD_EXTRACTU(ic->_tme_m68k_insn_opcode, 0, 3));
    
    /* if this bitfield wraps the register, put the wrapped
       part in the left: */
    if (shift > 32) {
      shift -= 32;
      ic->tme_m68k_ireg_uint32(ireg) = ((ic->tme_m68k_ireg_uint32(ireg)
					 & (0xffffffffUL >> shift))
					| (bf_value << (32 - shift)));
      bf_value >>= shift;
      bf_width -= shift;
    }
    
    /* update the register: */
    shift = (32 - (bf_offset + bf_width));
    ic->tme_m68k_ireg_uint32(ireg) = ((ic->tme_m68k_ireg_uint32(ireg)
				       & ~((0xffffffffUL >> (32 - bf_width)) << shift))
				      | (bf_value << shift));
  }

  /* otherwise, this bitfield is in memory: */
  else {

    /* this instruction can fault: */
    ic->_tme_m68k_mode_flags |= TME_M68K_EXECUTION_INST_CANFAULT;

    /* read in the bytes covering the bitfield if we haven't yet: */
    bf_bytes = (tme_uint8_t *) &ic->tme_m68k_ireg_memx32;
    count = (bf_offset + bf_width + 7) / 8;
    if (first_memory) {
      tme_m68k_read_mem(ic, bf_bytes, count);
    }

    /* if this bitfield covers 5 bytes, put the part for the fifth
       byte (actually in memy32!) in on the left: */
    if (shift > 32) {
      shift -= 32;
      if (!TME_M68K_SEQUENCE_RESTARTING) {
	bf_bytes[4] = ((bf_bytes[4]
			& (0xff >> shift))
		       | ((bf_value & 0xff) << (8 - shift)));
      }
      bf_value >>= shift;
      bf_width -= shift;
    }

    /* update the memory buffer: */
    if (!TME_M68K_SEQUENCE_RESTARTING) {
      shift = (32 - (bf_offset + bf_width));
      ic->tme_m68k_ireg_memx32 =
	tme_htobe_u32((tme_betoh_u32(ic->tme_m68k_ireg_memx32)
		       & ~((0xffffffffUL >> (32 - bf_width)) << shift))
		      | (bf_value << shift));
    }

    /* write out the bytes covering bitfield to memory: */
    tme_m68k_write_mem(ic, bf_bytes, count);
  }
#undef first_memory
}

/* our global verify hook function: */
#undef tme_m68k_verify_hook
void
tme_m68k_verify_hook(void)
{
}

#if 1
#include <stdio.h>

/* this dumps out the m68k state: */
void
tme_m68k_dump(struct tme_m68k *ic)
{
  int ireg;
  int count;

  /* dump out the integer registers: */
  count = 0;
  for (ireg = TME_M68K_IREG_D0;
       ireg <= TME_M68K_IREG_A7;
       ireg++) {
    fprintf(stderr,
	    "%%%c%d[%p] = 0x%08x",
	    (ireg < TME_M68K_IREG_A0
	     ? 'd'
	     : 'a'),
	    ireg - (ireg < TME_M68K_IREG_A0
		    ? TME_M68K_IREG_D0
		    : TME_M68K_IREG_A0),
	    &ic->tme_m68k_ireg_uint32(ireg),
	    ic->tme_m68k_ireg_uint32(ireg));
    if (++count == 2) {
      fprintf(stderr, "\n");
      count = 0;
    }
    else {
      fprintf(stderr, "  ");
    }
  }

  /* dump out the PC and next PC: */
  fprintf(stderr, "%%pc = 0x%08x  %%pc_next = 0x%08x\n",
	  ic->tme_m68k_ireg_pc,
	  ic->tme_m68k_ireg_pc_next);

  /* dump out the status register: */
  fprintf(stderr, "%%sr = 0x%04x", ic->tme_m68k_ireg_sr);
  fprintf(stderr, "  flags:");
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_X) {
    fprintf(stderr, " X");
  }
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_N) {
    fprintf(stderr, " N");
  }
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_Z) {
    fprintf(stderr, " Z");
  }
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_V) {
    fprintf(stderr, " V");
  }
  if (ic->tme_m68k_ireg_ccr & TME_M68K_FLAG_C) {
    fprintf(stderr, " C");
  }
  fprintf(stderr, "\n");

  /* dump out the effective address and memory buffers: */
  fprintf(stderr, "\n");
  fprintf(stderr, "EA = %d:0x%08x\n",
	  ic->_tme_m68k_ea_function_code,
	  ic->_tme_m68k_ea_address);
  fprintf(stderr, "%%memx[%p] = 0x%08x  %%memy[%p] = 0x%08x\n",
	  &ic->tme_m68k_ireg_memx32,
	  ic->tme_m68k_ireg_memx32,
	  &ic->tme_m68k_ireg_memy32,
	  ic->tme_m68k_ireg_memy32);

  /* dump out the control registers: */
  fprintf(stderr, "\n");
  fprintf(stderr, "%%usp = 0x%08x\n", ic->tme_m68k_ireg_usp);
  fprintf(stderr, "%%isp = 0x%08x\n", ic->tme_m68k_ireg_isp);
  fprintf(stderr, "%%msp = 0x%08x\n", ic->tme_m68k_ireg_msp);
  fprintf(stderr, "%%sfc = 0x%08x\n", ic->tme_m68k_ireg_sfc);
  fprintf(stderr, "%%dfc = 0x%08x\n", ic->tme_m68k_ireg_dfc);
  fprintf(stderr, "%%vbr = 0x%08x\n", ic->tme_m68k_ireg_vbr);
  
  /* dump out instruction decoding information: */
  fprintf(stderr, "\n");
  fprintf(stderr, "opcode = 0x%04x  specop = 0x%04x\n",
	  ic->_tme_m68k_insn_opcode,
	  ic->_tme_m68k_insn_specop);
}

void
tme_m68k_dump_memory(struct tme_m68k *ic, tme_uint32_t address, tme_uint32_t resid)
{
  unsigned int saved_ea_function_code;
  tme_uint32_t saved_ea_address;
  tme_uint32_t address_display;
  tme_uint8_t buffer[16];
  tme_uint32_t count;
  tme_uint32_t byte_i;

  /* save any EA function code and address: */
  saved_ea_function_code = ic->_tme_m68k_ea_function_code;
  saved_ea_address = ic->_tme_m68k_ea_address;

  /* we always display aligned rows: */
  address_display = address & (((tme_uint32_t) 0) - sizeof(buffer));

  /* while we have memory to dump: */
  for (; resid > 0; ) {

    /* read more data: */
    byte_i = address % sizeof(buffer);
    count = TME_MIN(resid, sizeof(buffer) - byte_i);
    ic->_tme_m68k_ea_function_code = TME_M68K_FUNCTION_CODE_DATA(ic);
    ic->_tme_m68k_ea_address = address;
    tme_m68k_read_mem(ic, &buffer[byte_i], count);
    count += byte_i;

    /* display the row: */
    fprintf(stderr, "0x%08x ", address_display);
    for (byte_i = 0;
	 byte_i < count;
	 byte_i++, address_display++) {
      if (address_display < address) {
	fprintf(stderr, "   ");
      }
      else {
	fprintf(stderr, " %02x",
		buffer[byte_i]);
	address++;
	resid--;
      }
    }
    fputc('\n', stderr);
  }

  /* restore any EA function code and address: */
  ic->_tme_m68k_ea_function_code = saved_ea_function_code;
  ic->_tme_m68k_ea_address = saved_ea_address;
}
#endif /* 1 */
