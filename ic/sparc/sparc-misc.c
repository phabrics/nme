/* $Id: sparc-misc.c,v 1.14 2010/06/05 16:17:19 fredette Exp $ */

/* ic/sparc/sparc-misc.c - miscellaneous things for the SPARC emulator: */

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

/* includes: */
#include "sparc-impl.h"

_TME_RCSID("$Id: sparc-misc.c,v 1.14 2010/06/05 16:17:19 fredette Exp $");

#include "sparc-bus-auto.c"

/* our bus signal handler: */
static int
_tme_sparc_bus_signal(struct tme_bus_connection *conn_bus, unsigned int signal)
{
  struct tme_sparc *ic;
  unsigned int level;

  /* recover our IC: */
  ic = conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get the level.  this must be an edge: */
  assert (signal & TME_BUS_SIGNAL_EDGE);
  level = signal - TME_BUS_SIGNAL_EDGE;
  signal = TME_BUS_SIGNAL_WHICH(signal);
  level ^= signal;

  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_sparc_external_mutex);

  /* if the signal is asserted: */
  if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {

    /* update the asserted flags for these signals: */
    if (__tme_predict_true(signal == TME_BUS_SIGNAL_BG)) {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_bg_asserted, TRUE);
    }
    else if (signal == TME_BUS_SIGNAL_RESET) {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_asserted, TRUE);
    }
    else {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_halt_asserted, TRUE);
    }
  }

  /* otherwise, the signal must be negated: */
  else {
    assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED);

    /* update the asserted or negated flags for these signals: */
    if (__tme_predict_true(signal == TME_BUS_SIGNAL_BG)) {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_bg_asserted, FALSE);
    }
    else if (signal == TME_BUS_SIGNAL_RESET) {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_negated, TRUE);
    }
    else {
      tme_memory_atomic_write_flag(&ic->tme_sparc_external_halt_negated, TRUE);
    }
  }

  /* write the external flag before any earlier signal flag write: */
  tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, TRUE);

  /* notify any thread waiting on the external condition: */
  tme_cond_notify(&ic->tme_sparc_external_cond, FALSE);

  /* unlock the external mutex: */
  tme_mutex_unlock(&ic->tme_sparc_external_mutex);
  return (TME_OK);
}

/* our interrupt handler: */
static int
_tme_sparc_bus_interrupt(struct tme_sparc_bus_connection *conn_sparc, unsigned int ipl)
{
  struct tme_sparc *ic;

  /* recover our IC: */
  ic = conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_sparc_external_mutex);

  /* set the interrupt line: */
  tme_memory_atomic_write8(&ic->tme_sparc_external_ipl,
			   ipl,
			   &ic->tme_sparc_external_ipl_rwlock,
			   sizeof(tme_uint8_t));

  /* write the external flag before the earlier ipl write: */
  tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, TRUE);

  /* notify any thread waiting on the external condition: */
  tme_cond_notify(&ic->tme_sparc_external_cond, FALSE);

  /* unlock the external mutex: */
  tme_mutex_unlock(&ic->tme_sparc_external_mutex);
  return (TME_OK);
}

/* the idle function, used when the processor is halted or stopped: */
static void
tme_sparc_idle(struct tme_sparc *ic)
{  
  /* lock the external mutex: */
  tme_mutex_lock(&ic->tme_sparc_external_mutex);

  /* loop forever: */
  for (;;) {

    /* check for any external signal: */
    (*ic->_tme_sparc_external_check)(ic, TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED);

    /* await an external condition: */
    tme_cond_wait_yield(&ic->tme_sparc_external_cond, &ic->tme_sparc_external_mutex);
  }
}

/* this resets idle detection: */
static void
_tme_sparc_idle_reset(struct tme_sparc *ic)
{

  /* reset the main idle PC to state one, and assume that the idle
     type has an idle PC range and zero the idle PC range upper
     bound: */
  if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T
    ic->tme_sparc_idle_pcs_64[0] = TME_SPARC_IDLE_TYPE_PC_STATE(1);
    ic->tme_sparc_idle_pcs_64[1] = 0;
#endif /* TME_HAVE_INT64_T */
  }
  else {
    ic->tme_sparc_idle_pcs_32[0] = TME_SPARC_IDLE_TYPE_PC_STATE(1);
    ic->tme_sparc_idle_pcs_32[1] = 0;
  }
}

/* the sparc thread: */
static void
tme_sparc_thread(struct tme_sparc *ic)
{

  /* we use longjmp to redispatch: */
  do { } while (setjmp(ic->_tme_sparc_dispatcher));

  /* we must not have a busy instruction TLB entry: */
  assert (ic->_tme_sparc_itlb_current_token == NULL);

  /* dispatch on the current mode: */
  switch (ic->_tme_sparc_mode) {

  case TME_SPARC_MODE_EXECUTION:

    /* if we may update the runlength with this instruction burst,
       note its start time: */
    if (ic->tme_sparc_runlength_update_next == 0
	&& (ic->_tme_sparc_instruction_burst_remaining
	    == ic->_tme_sparc_instruction_burst)) {
      ic->tme_sparc_runlength.tme_runlength_cycles_start = tme_misc_cycles();
    }

    (*ic->_tme_sparc_execute)(ic);
    /* NOTREACHED */

  case TME_SPARC_MODE_STOP:
  case TME_SPARC_MODE_HALT:
  case TME_SPARC_MODE_OFF:
    tme_sparc_idle(ic);
    /* NOTREACHED */

  case TME_SPARC_MODE_TIMING_LOOP:
    tme_sparc_timing_loop_finish(ic);
    /* NOTREACHED */

  default:
    abort();
  }
  /* NOTREACHED */
}

/* the TLB filler for when we are on a generic bus: */
static int
_tme_sparc_generic_tlb_fill(struct tme_sparc_bus_connection *conn_sparc, 
			    struct tme_sparc_tlb *tlb,
			    tme_uint32_t asi_mask, 
			    tme_bus_addr_t external_address, 
			    unsigned int cycles)
{
  struct tme_sparc *ic;

  /* recover our IC: */
  ic = conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_element->tme_element_private;

  /* call the generic bus TLB filler: */
  (ic->_tme_sparc_bus_generic->tme_bus_tlb_fill)
    (ic->_tme_sparc_bus_generic,
     &tlb->tme_sparc_tlb_bus_tlb,
     external_address,
     cycles);
  
  return (TME_OK);
}

/* this sets the run length: */
static void
_tme_sparc_runlength(struct tme_sparc *ic,
		     tme_uint32_t instruction_burst_msec)
{
  union tme_value64 runlength_target_cycles;
  unsigned int runlength_update_hz;

  /* set the run length target cycles: */
  runlength_target_cycles.tme_value64_uint32_lo
    = (tme_misc_cycles_per_ms()
       * instruction_burst_msec);
  runlength_target_cycles.tme_value64_uint32_hi = 0;
  tme_runlength_target_cycles(&ic->tme_sparc_runlength, runlength_target_cycles);

  /* set the run length update period: */
  runlength_update_hz = 50;
  ic->tme_sparc_runlength_update_period
    = (((1000
	 + (instruction_burst_msec - 1)
	 / instruction_burst_msec)
	+ (runlength_update_hz - 1))
       / runlength_update_hz);
}

/* the sparc command function: */
static int
_tme_sparc_command(struct tme_element *element, const char * const * args, char **_output)
{
  struct tme_sparc *ic;
  unsigned int idle_type_saved;
  tme_uint32_t instruction_burst_msec;
  int usage;
  tme_uint32_t prom_delay_factor;

  /* recover our IC: */
  ic = (struct tme_sparc *) element->tme_element_private;

  /* the "idle-type" command: */
  if (TME_ARG_IS(args[1], "idle-type")) {

    /* save the current idle type and set it to none: */
    idle_type_saved = ic->tme_sparc_idle_type;
    ic->tme_sparc_idle_type = TME_SPARC_IDLE_TYPE_NULL;

    /* if we're not setting the idle type to none: */
    if (!TME_ARG_IS(args[2], "none")) {

      /* check for a supported idle type: */
#define _TME_SPARC_IDLE_TYPE(x, s)		\
  do {						\
    if (TME_SPARC_IDLE_TYPE_IS_SUPPORTED(ic, x)	\
      && TME_ARG_IS(args[2], s)) {		\
      ic->tme_sparc_idle_type = (x);		\
    }						\
  } while (/* CONSTCOND */ 0)
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0, "netbsd32-type-0");
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0, "sunos32-type-0");
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1, "netbsd32-type-1");
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0, "netbsd64-type-0");
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1, "netbsd64-type-1");
      _TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0, "sunos64-type-0");
#undef _TME_SPARC_IDLE_TYPE

      /* if the idle type isn't supported: */
      if (ic->tme_sparc_idle_type == TME_SPARC_IDLE_TYPE_NULL) {

	/* restore the idle type and return a usage: */
	ic->tme_sparc_idle_type = idle_type_saved;
    
	tme_output_append_error(_output,
				"%s %s idle-type { none",
				_("usage:"),
				args[0]);

      /* add in the supported idle types: */
#define _TME_SPARC_IDLE_TYPE(x, s)		\
  do {						\
    if (TME_SPARC_IDLE_TYPE_IS_SUPPORTED(ic, x)) {\
      tme_output_append_error(_output, " | %s",	\
			      s);		\
    }						\
  } while (/* CONSTCOND */ 0)
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_0, "netbsd32-type-0");
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_SUNOS32_TYPE_0, "sunos32-type-0");
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD32_TYPE_1, "netbsd32-type-1");
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_0, "netbsd64-type-0");
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_NETBSD64_TYPE_1, "netbsd64-type-1");
	_TME_SPARC_IDLE_TYPE(TME_SPARC_IDLE_TYPE_SUNOS64_TYPE_0, "sunos64-type-0");
#undef _TME_SPARC_IDLE_TYPE

	tme_output_append_error(_output, " }");
	return (EINVAL);
      }
    }

    /* poison all idle type state: */
    _tme_sparc_idle_reset(ic);
  }

  /* the run-length command: */
  else if (TME_ARG_IS(args[1], "run-length")) {

    /* get the run length, in milliseconds: */
    instruction_burst_msec = tme_misc_unumber_parse(args[2], 0);

    /* if this command is bad: */
    if (instruction_burst_msec == 0
	|| args[3] != NULL) {
      tme_output_append_error(_output,
			      "%s run-length %s",
			      _("usage:"),
			      _("MILLISECONDS"));
    }

    /* otherwise, set the run length: */
    else {
      _tme_sparc_runlength(ic,
			   instruction_burst_msec);
    }
  }

  /* the prom-delay-factor command: */
  else if (TME_ARG_IS(args[1], "prom-delay-factor")) {

    /* get the PROM delay factor: */
    usage = FALSE;
    if (TME_ARG_IS(args[2], "best")) {
      prom_delay_factor = TME_SPARC_PROM_DELAY_FACTOR_BEST;
    }
    else if (TME_ARG_IS(args[2], "uncorrected")) {
      prom_delay_factor = TME_SPARC_PROM_DELAY_FACTOR_UNCORRECTED;
    }
    else if (TME_ARG_IS(args[2], "min")) {
      prom_delay_factor = TME_SPARC_PROM_DELAY_FACTOR_MIN;
    }
    else {
      prom_delay_factor = tme_misc_unumber_parse_any(args[2], &usage);
    }
    if (usage) {
      tme_output_append_error(_output,
			      "%s prom-delay-factor { best | uncorrected | min | %s }",
			      _("usage:"),
			      _("FACTOR"));
    }
    else {
      ic->tme_sparc_prom_delay_factor = prom_delay_factor;
    }
  }

  /* any other command: */
  else {
    if (args[1] != NULL) {
      tme_output_append_error(_output,
			      "%s '%s', ",
			      _("unknown command"),
			      args[1]);
    }
    tme_output_append_error(_output,
			    _("available %s commands:%s run-length"),
			    args[0],
			    (TME_SPARC_IDLE_TYPE_IS_SUPPORTED(ic, (0 - (unsigned int) 1))
			     ? " idle-type"
			     : ""));
    return (EINVAL);
  }

  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_sparc_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_bus_connection *conn_bus;
  unsigned int score;

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  switch (conn->tme_connection_type) {

    /* this must be a bus, and not another sparc chip: */
  case TME_CONNECTION_BUS_SPARC:
    conn_sparc = (struct tme_sparc_bus_connection *) conn->tme_connection_other;
    conn_bus = &conn_sparc->tme_sparc_bus_connection;
    if (conn_bus->tme_bus_tlb_set_add != NULL
	&& conn_sparc->tme_sparc_bus_tlb_fill != NULL
	&& conn_sparc->tme_sparc_bus_fpu_strict == NULL) {
      score = 10;
    }
    break;

    /* this must be a controller, and not another agent: */
  case TME_CONNECTION_BUS_UPA:
    conn_upa = (struct tme_upa_bus_connection *) conn->tme_connection_other;
    conn_bus = &conn_upa->tme_upa_bus_connection;
    if (conn_upa->tme_upa_bus_interrupt != NULL
	&& conn_bus->tme_bus_tlb_set_add != NULL
	&& conn_bus->tme_bus_tlb_fill != NULL) {
      score = 10;
    }
    break;

    /* this must be a bus, and not another chip: */
  case TME_CONNECTION_BUS_GENERIC:
    conn_bus = (struct tme_bus_connection *) conn->tme_connection_other;
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
_tme_sparc_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sparc *ic;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn_other;
  struct tme_bus_tlb_set_info tlb_set_info;
  struct tme_sparc_tlb *tlb;
  struct tme_token *token;
  int rc;

  /* since the CPU is halted, it won't be making any connection calls,
     so we only have to do work when the connection is fully made: */
  if (state == TME_CONNECTION_FULL) {

    /* recover our IC: */
    ic = conn->tme_connection_element->tme_element_private;
    
    /* dispatch on the connection type: */
    conn_other = conn->tme_connection_other;
    switch (conn->tme_connection_type) {
      
    case TME_CONNECTION_BUS_SPARC:
      conn_sparc = (struct tme_sparc_bus_connection *) conn_other;
      ic->_tme_sparc_bus_connection = conn_sparc;
      conn_bus = &conn_sparc->tme_sparc_bus_connection;
      break;
      
    case TME_CONNECTION_BUS_UPA:
      conn_upa = (struct tme_upa_bus_connection *) conn_other;
      ic->_tme_upa_bus_connection = conn_upa;
      assert (&conn_upa->tme_upa_bus_connection == (struct tme_bus_connection *) conn_other);
      /* FALLTHROUGH */
      
      /* we need an adaptation layer: */
    case TME_CONNECTION_BUS_GENERIC:
      conn_bus = (struct tme_bus_connection *) conn_other;
      conn_sparc = tme_new0(struct tme_sparc_bus_connection, 1);
      conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_element = conn->tme_connection_element;
      conn_sparc->tme_sparc_bus_tlb_fill = _tme_sparc_generic_tlb_fill;
      ic->_tme_sparc_bus_connection = conn_sparc;
      ic->_tme_sparc_bus_generic = conn_bus;
      break;
      
    default: abort();
    }

    /* make the TLB set information: */
    memset(&tlb_set_info, 0, sizeof(tlb_set_info));
    tlb_set_info.tme_bus_tlb_set_info_token0 = &ic->tme_sparc_tlb_tokens[0];
    tlb_set_info.tme_bus_tlb_set_info_token_stride = sizeof(struct tme_token);
    tlb_set_info.tme_bus_tlb_set_info_token_count = TME_ARRAY_ELS(ic->tme_sparc_tlbs);
    tlb_set_info.tme_bus_tlb_set_info_bus_context = &ic->tme_sparc_memory_context_default;

#if TME_HAVE_RECODE

    /* if this is a v9 CPU, and we have 64-bit recode support: */
    if (TME_SPARC_VERSION(ic) >= 9) {
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32

      /* we will use the tokens in the 64-bit recode TLBs: */
      tlb_set_info.tme_bus_tlb_set_info_token0 = &ic->tme_sparc_recode_tlb64s[0].tme_recode_tlb_c16_a64_token;
      tlb_set_info.tme_bus_tlb_set_info_token_stride = sizeof(ic->tme_sparc_recode_tlb64s[0]);
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
    }

    /* otherwise, this is a v7 or v8 CPU: */
    else {

      /* we will use the tokens in the 32-bit recode TLBs: */
      tlb_set_info.tme_bus_tlb_set_info_token0 = &ic->tme_sparc_recode_tlb32s[0].tme_recode_tlb_c16_a32_token;
      tlb_set_info.tme_bus_tlb_set_info_token_stride = sizeof(ic->tme_sparc_recode_tlb32s[0]);
    }

#endif /* TME_HAVE_RECODE */

    /* initialize the TLBs in the set: */
    tlb = &ic->tme_sparc_tlbs[0];
    token = tlb_set_info.tme_bus_tlb_set_info_token0;
    do {

      /* initialize this token: */
      tme_token_init(token);

      /* connect this token with this TLB: */
      tlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token = token;

      /* advance: */
      token = (struct tme_token *) (tlb_set_info.tme_bus_tlb_set_info_token_stride + (tme_uint8_t *) token);
    } while (++tlb <= &ic->tme_sparc_tlbs[TME_ARRAY_ELS(ic->tme_sparc_tlbs) - 1]);

    /* add the TLB set: */
    rc = ((*conn_bus->tme_bus_tlb_set_add)
	  (conn_bus,
	   &tlb_set_info));
    assert (rc == TME_OK);

    /* if this is a v7 cpu: */
    if (TME_SPARC_VERSION(ic) == 7) {

      /* get the maximum bus context from the bus: */
      ic->tme_sparc_memory_context_max = tlb_set_info.tme_bus_tlb_set_info_bus_context_max;
    }

#ifdef TME_HAVE_RECODE

    /* the maximum bus context must fit in 16 bits: */
    assert (ic->tme_sparc_memory_context_max <= 0xffff);

#endif /* TME_HAVE_RECODE */
  }

  /* NB: the machine needs to issue a reset to bring the CPU out of halt. */
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_sparc_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
  return (0);
}

/* this makes new connection sides: */
static int
_tme_sparc_connections_new(struct tme_element *element, const char * const *args, struct tme_connection **_conns, char **_output)
{
  struct tme_sparc *ic;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;

  /* recover our data structure: */
  ic = element->tme_element_private;

  /* if we already have a bus connection, we can take no more connections: */
  if (ic->_tme_sparc_bus_connection != NULL) {
    return (TME_OK);
  }

  /* if this is a v9 CPU: */
  if (TME_SPARC_VERSION(ic) >= 9) {

    /* create our side of a UPA bus connection: */
    conn_upa = tme_new0(struct tme_upa_bus_connection, 1);
    conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_UPA;
#ifdef TME_HAVE_INT64_T
    conn_upa->tme_upa_bus_interrupt = ic->_tme_sparc_upa_interrupt;
#endif /* TME_HAVE_INT64_T */
    conn_bus = &conn_upa->tme_upa_bus_connection;
    conn_bus->tme_bus_tlb_fill = ic->_tme_sparc_tlb_fill;
  }

  /* otherwise, this is a v7 or v8 CPU: */
  else {

    /* create our side of a generic bus connection: */
    conn_bus = tme_new0(struct tme_bus_connection, 1);
    conn_bus->tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_GENERIC;
    conn_bus->tme_bus_signal = _tme_sparc_bus_signal;
    conn_bus->tme_bus_tlb_set_add = NULL;
    conn_bus->tme_bus_tlb_fill = NULL;
    conn = &conn_bus->tme_bus_connection;
    conn->tme_connection_next = *_conns;
    conn->tme_connection_score = _tme_sparc_connection_score;
    conn->tme_connection_make = _tme_sparc_connection_make;
    conn->tme_connection_break = _tme_sparc_connection_break;

    /* add this connection to the set of possibilities: */
    *_conns = conn;

    /* create our side of a sparc bus connection: */
    conn_sparc = tme_new0(struct tme_sparc_bus_connection, 1);
    conn_sparc->tme_sparc_bus_connection.tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_SPARC;
    conn_sparc->tme_sparc_bus_interrupt = _tme_sparc_bus_interrupt;
    conn_sparc->tme_sparc_bus_tlb_fill = NULL;
    conn_sparc->tme_sparc_bus_fpu_strict = tme_sparc_fpu_strict;
    conn_bus = &conn_sparc->tme_sparc_bus_connection;
    conn_bus->tme_bus_tlb_fill = NULL;
  }

  /* finish the preferred bus connection: */
  conn_bus->tme_bus_signal = _tme_sparc_bus_signal;
  conn_bus->tme_bus_tlb_set_add = NULL;
  conn = &conn_bus->tme_bus_connection;
  conn->tme_connection_next = *_conns;
  conn->tme_connection_score = _tme_sparc_connection_score;
  conn->tme_connection_make = _tme_sparc_connection_make;
  conn->tme_connection_break = _tme_sparc_connection_break;

  /* add this connection to the set of possibilities: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* the common sparc synchronization initialization: */
void
tme_sparc_sync_init(struct tme_sparc *ic)
{

  /* initialize the external mutex: */
  tme_mutex_init(&ic->tme_sparc_external_mutex);

  /* initialize the external condition: */
  tme_cond_init(&ic->tme_sparc_external_cond);
}

/* the common sparc new function: */
int
tme_sparc_new(struct tme_sparc *ic, const char * const *args, const void *extra, char **_output)
{
  struct tme_element *element;
  int arg_i;
  int usage;
  tme_uint32_t cycles_per_ms;
  tme_uint32_t cycles_scaled_per_ms;
  const char *cycles_scaled_per_ms_arg;
  unsigned int cwp;
  unsigned int cwp_offset;
  tme_uint32_t asi;

  /* assume that we have no FPU: */
  ic->tme_sparc_fpu_fsr = TME_SPARC_FSR_VER_missing;

  /* if we don't have a tlb page size: */
  if (ic->tme_sparc_tlb_page_size_log2 == 0) {

    /* assume that we are in a machine with a 4K page size: */
    /* XXX FIXME - we never attempt to discover the machine's actual
       page size.  however, using the wrong page size doesn't affect
       correctness, only performance.  using a smaller page size means
       that accesses to different parts of the same true page can be
       spread over multiple DTLB entries.  using a larger page size
       means that accesses to adjacent true pages can collide in one
       DTLB entry.  we assume that using a smaller page size hurts
       performance less than using a larger page size: */
    ic->tme_sparc_tlb_page_size_log2 = 12; /* log2(4096) */
  }

  /* check our arguments: */
  arg_i = 1;
  usage = FALSE;
  cycles_per_ms = tme_misc_cycles_per_ms();
  cycles_scaled_per_ms = cycles_per_ms;
  cycles_scaled_per_ms_arg = NULL;
  ic->tme_sparc_prom_delay_factor = TME_SPARC_PROM_DELAY_FACTOR_BEST;
  for (;;) {

    /* if this is a cycles scaling argument: */
    if (TME_ARG_IS(args[arg_i + 0], "tick-frequency")) {
      cycles_scaled_per_ms_arg = args[arg_i + 0];
      cycles_scaled_per_ms = tme_misc_unumber_parse_any(args[arg_i + 1], &usage) / 1000;
      if (usage) {
	break;
      }
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* this is either a bad argument or an FPU argument: */
    else {

      /* if this is not an FPU argument: */
      if (!tme_sparc_fpu_new(ic, args, &arg_i, &usage, _output)) {
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

  /* set the cycles scaling: */
  if (cycles_scaled_per_ms == 0) {
    if (!usage) {
      tme_output_append_error(_output,
			      "tick-frequency %s %s, ",
			      cycles_scaled_per_ms_arg,
			      _("too small"));
      usage = TRUE;
    }
  }
  else {
    tme_misc_cycles_scaling(&ic->tme_sparc_cycles_scaling,
			    cycles_scaled_per_ms,
			    cycles_per_ms);
    tme_misc_cycles_scaling(&ic->tme_sparc_cycles_unscaling,
			    cycles_per_ms,
			    cycles_scaled_per_ms);
    ic->tme_sparc_cycles_scaled_per_usec = (cycles_scaled_per_ms + 999) / 1000;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s [ tick-frequency %s ]",
			    _("usage:"),
			    args[0],
			    _("TICK-FREQUENCY"));
    tme_sparc_fpu_usage(ic, _output);
    tme_free(ic);
    return (EINVAL);
  }

  /* we have no bus connection yet: */
  ic->_tme_sparc_bus_connection = NULL;

  /* fill the element: */
  element = ic->tme_sparc_element;
  element->tme_element_private = ic;
  element->tme_element_connections_new = _tme_sparc_connections_new;
  element->tme_element_command = _tme_sparc_command;

  /* initialize the instruction burst runlength state: */
  ic->_tme_sparc_instruction_burst = 800;
  ic->tme_sparc_runlength.tme_runlength_history_count = 64;
  tme_runlength_init(&ic->tme_sparc_runlength);
  _tme_sparc_runlength(ic, 2);
  ic->tme_sparc_runlength_update_next = ic->tme_sparc_runlength_update_period;

  /* set the idle instruction burst size: */
  ic->_tme_sparc_instruction_burst_idle = 10;

  /* start the first instruction burst: */
  ic->_tme_sparc_instruction_burst_remaining
    = ic->_tme_sparc_instruction_burst;
  ic->_tme_sparc_instruction_burst_other = TRUE;

  /* force the processor to be off: */
  ic->_tme_sparc_mode = TME_SPARC_MODE_OFF;

  /* initialize the external state: */
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_flag, FALSE);
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_reset_asserted, TRUE);
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_reset_negated, FALSE);
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_halt_asserted, FALSE);
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_halt_negated, FALSE);
  tme_memory_atomic_init_flag(&ic->tme_sparc_external_bg_asserted, FALSE);
  ic->tme_sparc_external_ipl = TME_SPARC_IPL_NONE;
  tme_rwlock_init(&ic->tme_sparc_external_ipl_rwlock);

  /* update the CWP offset: */
  if (TME_SPARC_VERSION(ic) >= 9) {
    cwp = ic->tme_sparc64_ireg_cwp;
    TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);
  }
  else {
    cwp = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP);
    TME_SPARC32_CWP_UPDATE(ic, cwp, cwp_offset);
  }

  /* if the specific CPU doesn't provide any ASI handlers: */
  if (ic->_tme_sparc_ls_asi_handlers == NULL) {

    if (TME_SPARC_VERSION(ic) >= 9) {

      /* this shouldn't happen: */
      abort();
    }

    else {

      /* by default, all sparc32 ASIs are special, except for the
	 required ASIs: */
      for (asi = 0; asi < TME_ARRAY_ELS(ic->tme_sparc_asis); asi++) {
	ic->tme_sparc_asis[asi].tme_sparc_asi_mask_flags = TME_SPARC32_ASI_MASK_FLAG_SPECIAL;
      }
      ic->tme_sparc_asis[TME_SPARC32_ASI_UI].tme_sparc_asi_mask_flags = !TME_SPARC32_ASI_MASK_FLAG_SPECIAL;
      ic->tme_sparc_asis[TME_SPARC32_ASI_SI].tme_sparc_asi_mask_flags = !TME_SPARC32_ASI_MASK_FLAG_SPECIAL;
      ic->tme_sparc_asis[TME_SPARC32_ASI_UD].tme_sparc_asi_mask_flags = !TME_SPARC32_ASI_MASK_FLAG_SPECIAL;
      ic->tme_sparc_asis[TME_SPARC32_ASI_SD].tme_sparc_asi_mask_flags = !TME_SPARC32_ASI_MASK_FLAG_SPECIAL;
    }
  }

  /* poison all idle type state: */
  _tme_sparc_idle_reset(ic);

  /* initialize recoding: */
  tme_sparc_recode_init(ic);

  /* start the sparc thread: */
  tme_thread_create((tme_thread_t) tme_sparc_thread, ic);

  return (TME_OK);
}  

/* this redispatches: */
void
tme_sparc_redispatch(struct tme_sparc *ic)
{
  struct tme_token *token;

  /* end any recode verifying: */
  tme_sparc_recode_verify_end(ic, TME_SPARC_TRAP_none);

  /* if we have a busy instruction TLB entry: */
  token = ic->_tme_sparc_itlb_current_token;
  if (__tme_predict_true(token != NULL)) {

    /* unbusy and forget the instruction TLB entry: */
    tme_token_unbusy(token);
    ic->_tme_sparc_itlb_current_token = NULL;
  }

  /* do the redispatch: */
#ifdef _TME_SPARC_STATS
  ic->tme_sparc_stats.tme_sparc_stats_redispatches++;
#endif /* _TME_SPARC_STATS */
  longjmp(ic->_tme_sparc_dispatcher, 1);
}

/* our global verify hook function: */
#undef tme_sparc_verify_hook
void
tme_sparc_verify_hook(void)
{
}

/* the common sparc reset function: */
void
tme_sparc_do_reset(struct tme_sparc *ic)
{

  /* if this is a v7 or v8 CPU: */
  if (ic->tme_sparc_version < 9) {

    /* set the initial PCs: */
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT) = 0;
    ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT) = sizeof(tme_uint32_t);
    
    /* force supervisor mode, traps disabled: */
    ic->tme_sparc32_ireg_psr
      = ((ic->tme_sparc32_ireg_psr
	  & ~TME_SPARC32_PSR_ET)
	 | TME_SPARC32_PSR_S);
  }

  /* otherwise, this is a v9 CPU: */
  else {

    /* XXX WRITEME */
    abort();
  }

  /* reset the FPU: */
  tme_sparc_fpu_reset(ic);

  /* poison all idle type state, to force the idle type to retrain: */
  _tme_sparc_idle_reset(ic);

  /* start execution: */
  ic->_tme_sparc_mode = TME_SPARC_MODE_EXECUTION;
  tme_sparc_redispatch(ic);
}

/* the common sparc idle function: */
void
tme_sparc_do_idle(struct tme_sparc *ic)
{

  /* NB: since the interrupt that causes us to leave stop mode will
     call tme_sparc32_trap_preinstruction(), this function can only be
     called on a preinstruction boundary (i.e., while PC still points
     to the (completed!) instruction that triggered the idle
     condition): */

  /* this will not be a full instruction burst: */
  ic->_tme_sparc_instruction_burst_other = TRUE;

  /* redispatch into stop mode: */
  ic->_tme_sparc_mode = TME_SPARC_MODE_STOP;
  tme_sparc_redispatch(ic);
}

/* this checks for external signals: */
void
tme_sparc32_external_check(struct tme_sparc *ic,
			   int flags)
{
  unsigned int ipl;

  /* if RESET has been negated since the last check: */
  if (__tme_predict_false(tme_memory_atomic_read_flag(&ic->tme_sparc_external_reset_negated))) {

    /* clear the RESET asserted flag, then clear the RESET negated
       flag: */
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_asserted, FALSE);
    tme_memory_barrier(ic, sizeof(*ic), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_reset_negated, FALSE);

    /* start reset trap processing: */
    if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
      tme_mutex_unlock(&ic->tme_sparc_external_mutex);
    }
    tme_sparc32_trap_preinstruction(ic, TME_SPARC32_TRAP_reset);
  }

  /* if RESET is asserted: */
  if (__tme_predict_false(tme_memory_atomic_read_flag(&ic->tme_sparc_external_reset_asserted))) {

    /* halt: */
    if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
      tme_mutex_unlock(&ic->tme_sparc_external_mutex);
    }
    ic->_tme_sparc_mode = TME_SPARC_MODE_HALT;
    tme_sparc_redispatch(ic);
  }

  /* if an interrupt needs service: */
  ipl = tme_memory_atomic_read8(&ic->tme_sparc_external_ipl,
				&ic->tme_sparc_external_ipl_rwlock,
				sizeof(tme_uint8_t));
  assert (ipl <= TME_SPARC_IPL_MAX);
  if (ipl >= TME_SPARC_IPL_MIN) {

    /* if we can't service this interrupt now, we need to set the
       external flag again so we keep coming back to try again.

       even if we do service this interrupt now, we still need to set
       the external flag again - because we may not service all of the
       devices interrupting at this level, and the bus won't bother to
       make another interrupt level callout if the level isn't
       actually changing.  we need to set the external flag again so
       we keep coming back to try again: */
    tme_memory_atomic_write_flag(&ic->tme_sparc_external_flag, TRUE);

    /* if we are not halted and an interrupt can be serviced, start
       interrupt trap processing: */
    if (ic->_tme_sparc_mode != TME_SPARC_MODE_HALT
	&& (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ET)
	&& (ipl == TME_SPARC_IPL_NMI
	    || ipl > TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_PIL))) {

      if (flags & TME_SPARC_EXTERNAL_CHECK_MUTEX_LOCKED) {
	tme_mutex_unlock(&ic->tme_sparc_external_mutex);
      }

      /* dispatch the trap: */
      tme_sparc32_trap_preinstruction(ic, TME_SPARC32_TRAP_interrupt_level(ipl));
    }
  }

  /* there are no traps to process: */
}

/* this triggers sparc32 trap processing on a preinstruction boundary: */
void
tme_sparc32_trap_preinstruction(struct tme_sparc *ic, tme_uint32_t trap)
{

  /* end any recode verifying: */
  tme_sparc_recode_verify_end(ic, TME_SPARC_TRAP_none);

  /* shift the next instruction's PC and next-next PC up: */
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC) = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT);
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT) = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT);

  /* do the rest of the sparc32 trap processing: */
  tme_sparc32_trap(ic, trap);
}

/* this triggers sparc32 trap processing by an instruction: */
void
tme_sparc32_trap(struct tme_sparc *ic, tme_uint32_t trap)
{
  unsigned int cwp;
  unsigned int cwp_offset;
  unsigned int reg_17;

  /* end any recode verifying: */
  tme_sparc_recode_verify_end(ic, trap);

  /* stop idling: */
  TME_SPARC_IDLE_STOP(ic);

  /* reset traps are handled specially: */
  if (__tme_predict_false(trap == TME_SPARC32_TRAP_reset)) {
    tme_sparc_do_reset(ic);
    /* NOTREACHED */
  }  

  /* "The processor enters error_mode state when a trap occurs while
     ET = 0. An implementation should preserve as much processor state
     as possible when this happens. Standard trap actions (such as
     decrementing CWP and saving state information in locals) should
     not occur when entering error_mode. In particular, the tt field
     of the TBR is only written during a transition into error_mode
     state in the singular case of a RETT instruction that traps while
     ET = 0. In this case, tt is written to indicate the type of
     exception that was induced by the RETT instruction.

     What occurs after error_mode is entered is
     implementation-dependent; typically the processor triggers an
     external reset, causing a reset trap (see below). */
  if (__tme_predict_false((ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ET) == 0)) {

    /* if we were executing a RETT instruction: */
    assert (ic->_tme_sparc_mode == TME_SPARC_MODE_EXECUTION);
    if ((ic->_tme_sparc_insn
	 & ((3 << 30) | (0x3f << 19)))
	== ((tme_uint32_t) (2 << 30) | (0x39 << 19))) {
      
      /* update the TBR register: */
      TME_FIELD_MASK_DEPOSITU(ic->tme_sparc32_ireg_tbr, 0xff, trap);
    }

    /* reset the processor: */
    tme_log(TME_SPARC_LOG_HANDLE(ic), 0, EPERM,
	    (TME_SPARC_LOG_HANDLE(ic),
	     _("took a trap while traps disabled, processor reset")));
    tme_sparc32_trap(ic, TME_SPARC32_TRAP_reset);
  }

  /* "Traps are disabled: ET <- 0.
     The existing user/supervisor mode is preserved: PS <- S.
     The user/supervisor mode is changed to supervisor: S <- 1." */
  ic->tme_sparc32_ireg_psr
    = ((ic->tme_sparc32_ireg_psr
	& ~(TME_SPARC32_PSR_ET
	    | TME_SPARC32_PSR_PS))
       | ((ic->tme_sparc32_ireg_psr
	   & TME_SPARC32_PSR_S)
	  / (TME_SPARC32_PSR_S
	     / TME_SPARC32_PSR_PS))
       | TME_SPARC32_PSR_S);

  /* "The register window is advanced to a new window: 
     CWP <- ((CWP - 1) modulo NWINDOWS) 
     [note: without test for window overflow]." */
  cwp = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP);
  cwp -= 1;
  cwp %= ic->tme_sparc_nwindows;
  TME_FIELD_MASK_DEPOSITU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP, cwp);
  TME_SPARC32_CWP_UPDATE(ic, cwp, cwp_offset);
  reg_17 = 17;
  TME_SPARC_REG_INDEX(ic, reg_17);

  /* "The trapped program counters are saved in local registers 1 and
     2 of the new window: r[17] <- PC, r[18] <- nPC." */
  ic->tme_sparc_ireg_uint32(reg_17 + 0) = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC);
  ic->tme_sparc_ireg_uint32(reg_17 + 1) = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT);

  /* "The tt field is written to the particular value that identifies
     the exception or interrupt request, except as defined for `Reset
     Trap' and `Error Mode' above." */
  TME_FIELD_MASK_DEPOSITU(ic->tme_sparc32_ireg_tbr, 0x00000ff0, trap);

  /* "If the trap is not a reset trap, control is transferred into the
     trap table: PC <- TBR, nPC <- TBR + 4." */
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT) = ic->tme_sparc32_ireg_tbr;
  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT) = ic->tme_sparc32_ireg_tbr + sizeof(tme_uint32_t);

  /* log the trap: */
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("trap tt 0x%03" TME_PRIx32 " handler-%%pc 0x%08" TME_PRIx32),
		 TME_SPARC_TRAP_TT(trap),
		 ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT)));

  /* redispatch: */
  ic->_tme_sparc_mode = TME_SPARC_MODE_EXECUTION;
  tme_sparc_redispatch(ic);
}

/* the default sparc32 load/store bus cycle functions: */
void
tme_sparc32_ls_bus_cycle(const struct tme_sparc *ic,
			 struct tme_sparc_ls *ls)
{
  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS32_LOG2);
  ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_lane_routing
    = &(tme_sparc32_router
	[TME_SPARC_BUS_ROUTER_INDEX(TME_BUS32_LOG2,
				    ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size,
				    (tme_uint32_t) ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_address)]);
}

/* the default sparc32 load/store direct address map function: */
void
tme_sparc32_ls_address_map(struct tme_sparc *ic,
			   struct tme_sparc_ls *ls)
{
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_first = 0;
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_last = 0 - (tme_bus_addr_t) 1;
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
  ls->tme_sparc_ls_tlb_map.tme_bus_tlb_addr_offset = 0;
}

/* the default sparc32 load/store trap function: */
void
tme_sparc32_ls_trap(struct tme_sparc *ic,
		    struct tme_sparc_ls *ls)
{
  tme_uint32_t lsinfo;
  tme_uint32_t ls_faults;
  tme_uint32_t trap;
  tme_uint32_t fault_trap;

  /* get the information about this load/store: */
  lsinfo = ls->tme_sparc_ls_lsinfo;

  /* get the list of faults from this load/store: */
  ls_faults = ls->tme_sparc_ls_faults;

  /* we only support the sparc32 load/store faults: */
  assert ((ls_faults
	   & ~(TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED
	       | TME_SPARC_LS_FAULT_LDD_STD_RD_ODD
	       | TME_SPARC_LS_FAULT_BUS_FAULT
	       | TME_SPARC_LS_FAULT_BUS_ERROR)) == 0);

  /* start with no fault: */
  trap = TME_SPARC_TRAP_none;

  /* convert the faults into the highest-priority trap: */
  if (ls_faults & TME_SPARC_LS_FAULT_ADDRESS_NOT_ALIGNED) {
    trap = TME_MIN(trap, TME_SPARC32_TRAP_mem_address_not_aligned);
  }
  if (ls_faults & TME_SPARC_LS_FAULT_LDD_STD_RD_ODD) {
    trap = TME_MIN(trap, TME_SPARC32_TRAP_illegal_instruction);
  }
  if (ls_faults
      & (TME_SPARC_LS_FAULT_BUS_FAULT
	 | TME_SPARC_LS_FAULT_BUS_ERROR)) {
    fault_trap
      = ((lsinfo & TME_SPARC_LSINFO_OP_FETCH)
	 ? TME_SPARC32_TRAP_instruction_access_exception
	 : TME_SPARC32_TRAP_data_access_exception);
    trap = TME_MIN(trap, fault_trap);
  }

  /* there must be some fault: */
  assert (trap != TME_SPARC_TRAP_none);

  /* trap: */
  tme_sparc32_trap(ic, trap);
}

/* the default sparc nnPC trap function: */
void
tme_sparc_nnpc_trap(struct tme_sparc *ic,
		     tme_uint32_t ls_faults)
{
  struct tme_sparc_ls ls;
  struct tme_sparc_tlb tlb_dummy;

  /* make a limited load/store structure: */
  ls.tme_sparc_ls_faults = ls_faults;
  ls.tme_sparc_ls_lsinfo = TME_SPARC_LSINFO_OP_FETCH;
  ls.tme_sparc_ls_asi_mask = ic->tme_sparc_asi_mask_insn;
  ls.tme_sparc_ls_tlb = &tlb_dummy;
  tlb_dummy.tme_sparc_tlb_asi_mask
    = (!TME_SPARC64_ASI_MASK_FLAG_TLB_SIDE_EFFECTS
       );
  if (sizeof(tme_sparc_ireg_umax_t) > sizeof(tme_uint32_t)
      && TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T
    ls.tme_sparc_ls_context = ic->tme_sparc_memory_context_primary;
    ls.tme_sparc_ls_address64 = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT);
#endif /* TME_HAVE_INT64_T */
  }
  else {
    ls.tme_sparc_ls_context = ic->tme_sparc_memory_context_default;
    ls.tme_sparc_ls_address32 = ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT);
  }

  /* trap: */
  (*ic->_tme_sparc_ls_trap)(ic, &ls);
}

/* the default load/store bus fault function: */
void
tme_sparc_ls_bus_fault(struct tme_sparc *ic,
		       struct tme_sparc_ls *ls,
		       int err)
{
  tme_uint32_t lsinfo;
  tme_uint32_t cycle_size;
  tme_uint32_t ls_fault;

  /* get the information about this load/store: */
  lsinfo = ls->tme_sparc_ls_lsinfo;

  /* if this load/store ignores all bus faults: */
  if (lsinfo & TME_SPARC_LSINFO_NO_FAULT) {

    /* update the load/store to get past the fault: */
    cycle_size = ls->tme_sparc_ls_bus_cycle.tme_bus_cycle_size;
    if (TME_SPARC_VERSION(ic) >= 9) {
#ifdef TME_HAVE_INT64_T
      ls->tme_sparc_ls_address64 += cycle_size;
#endif /* TME_HAVE_INT64_T */
    }
    else {
      ls->tme_sparc_ls_address32 += cycle_size;
    }
    ls->tme_sparc_ls_buffer_offset += cycle_size;
    ls->tme_sparc_ls_size -= cycle_size;
    return;
  }

  /* convert the bus error code into a fault: */
  switch (err) {
  case EFAULT:
    ls_fault = TME_SPARC_LS_FAULT_BUS_FAULT;
    break;
  case ENOENT:
  case EIO: 
    ls_fault = TME_SPARC_LS_FAULT_BUS_ERROR;
    break;
  default: abort();
  }

  /* add in this fault: */
  ls->tme_sparc_ls_faults |= ls_fault;
}

#ifdef TME_HAVE_INT64_T

/* this triggers sparc64 trap processing on a preinstruction boundary: */
void
tme_sparc64_trap_preinstruction(struct tme_sparc *ic, tme_uint32_t trap)
{

  /* end any recode verifying: */
  tme_sparc_recode_verify_end(ic, TME_SPARC_TRAP_none);

  /* shift the next instruction's PC and next-next PC up: */
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT);
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT);

  /* do the rest of the sparc64 trap processing: */
  tme_sparc64_trap(ic, trap);
}

/* this triggers sparc64 trap processing by an instruction: */
void
tme_sparc64_trap(struct tme_sparc *ic, tme_uint32_t trap)
{
  tme_uint32_t tt;
  unsigned int tl;
  tme_uint32_t pstate;
  tme_uint32_t tstate_0_31;
  tme_int32_t cwp_addend;
  tme_uint32_t cwp;
  unsigned int cwp_offset;
  unsigned int wstate;
  tme_uint64_t pc;

  /* end any recode verifying: */
  tme_sparc_recode_verify_end(ic, trap);

  /* stop idling: */
  TME_SPARC_IDLE_STOP(ic);

  /* get this trap's tt value: */
  tt = TME_SPARC_TRAP_TT(trap);

  /* get the current TL: */
  tl = ic->tme_sparc64_ireg_tl;

  /* if this is some kind of reset: */
#if (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_power_on_reset) + 1) != TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_watchdog_reset)
#error "TME_SPARC64_TRAP_ values changed"
#endif
#if (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_power_on_reset) + 2) != TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_externally_initiated_reset)
#error "TME_SPARC64_TRAP_ values changed"
#endif
#if (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_power_on_reset) + 3) != TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_software_initiated_reset)
#error "TME_SPARC64_TRAP_ values changed"
#endif
  if (__tme_predict_false((tt >= TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_power_on_reset)
			   && tt <= TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_software_initiated_reset))
			  || (trap & TME_SPARC_TRAP_IMPDEP_RESET))) {

    /* if this is an SIR at TL == MAXTL: */
    if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_software_initiated_reset)
	&& tl == ic->tme_sparc64_maxtl) {

      /* enter error_state: */
      tme_sparc64_trap_error_state(ic);
      /* NOTREACHED */
    }

    /* enter RED_state, if we're not there already, at min(TL + 1,
       MAXTL): */
    pstate = ic->tme_sparc64_ireg_pstate;
    pstate |= TME_SPARC64_PSTATE_RED;
    tl = tl + 1;
    tl = TME_MIN(tl, ic->tme_sparc64_maxtl);

    /* if this is a POR: */
    if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_power_on_reset)) {

      /* reset the FPU: */
      tme_sparc_fpu_reset(ic);

      /* poison all idle type state, to force the idle type to retrain: */
      _tme_sparc_idle_reset(ic);

      /* clear PSTATE.TLE, which will be copied into PSTATE.CLE: */
      pstate &= ~TME_SPARC64_PSTATE_TLE;

      /* set TICK.NPT: */
      ic->tme_sparc64_ireg_tick_npt = TRUE;

      /* zero TICK.counter: */
      ic->tme_sparc64_ireg_tick_offset
	= (0 - tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint);

      /* enter RED_state at MAXTL: */
      tl = ic->tme_sparc64_maxtl;
    }

    /* if this is an XIR: */
    else if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_externally_initiated_reset)) {

      /* zero TICK.counter: */
      ic->tme_sparc64_ireg_tick_offset
	= (0 - tme_misc_cycles_scaled(&ic->tme_sparc_cycles_scaling, 0).tme_value64_uint);
    }
  }

  /* otherwise, this is a normal trap or interrupt: */
  else {

    /* increment TL: */
    tl = tl + 1;

    /* if we were already at MAXTL: */
    if (__tme_predict_false(tl > ic->tme_sparc64_maxtl)) {

      /* enter error_state: */
      tme_sparc64_trap_error_state(ic);
      /* NOTREACHED */
    }

    /* get PSTATE: */
    pstate = ic->tme_sparc64_ireg_pstate;

    /* if we are now at MAXTL: */
    if (tl == ic->tme_sparc64_maxtl) {

      /* enter RED_state, if we're not there already: */
      pstate |= TME_SPARC64_PSTATE_RED;
    }
  }

  /* save ASI: */
  tstate_0_31 = ic->tme_sparc64_ireg_asi;
  tstate_0_31 
    *= (_TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_ASI)
	/ _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE));

  /* save PSTATE: */
  assert (ic->tme_sparc64_ireg_pstate
	  <= (TME_SPARC64_TSTATE_MASK_PSTATE
	      / _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE)));
  tstate_0_31 += ic->tme_sparc64_ireg_pstate;
  tstate_0_31
    *= (_TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_PSTATE)
	/ _TME_FIELD_MASK_FACTOR(TME_SPARC64_TSTATE_MASK_CWP));

  /* save CWP: */
  tstate_0_31 += ic->tme_sparc64_ireg_cwp;

  /* store the least-significant 32 bits of TSTATE[TL]: */
  ic->tme_sparc64_ireg_tstate(tl) = tstate_0_31;

  /* save CCR directly into TSTATE[TL]: */
  ic->tme_sparc64_ireg_tstate_ccr(tl) = ic->tme_sparc64_ireg_ccr;

  /* save TPC[TL] and TNPC[TL]: */
  ic->tme_sparc64_ireg_tpc(tl) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC);
  ic->tme_sparc64_ireg_tnpc(tl) = ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT);

  /* finish the normal PSTATE update: */
  pstate
    &= ~(TME_SPARC64_PSTATE_AM
	 + TME_SPARC64_PSTATE_IE
	 + TME_SPARC64_PSTATE_CLE);
  if (__tme_predict_false(pstate & TME_SPARC64_PSTATE_RED)) {
    pstate &= ~TME_SPARC64_PSTATE_MM;
  }
  pstate
    |= (TME_SPARC64_PSTATE_PEF
	+ TME_SPARC64_PSTATE_PRIV
	+ TME_SPARC64_PSTATE_AG
	+ ((pstate
	    & TME_SPARC64_PSTATE_TLE)
	   * (TME_SPARC64_PSTATE_CLE
	      / TME_SPARC64_PSTATE_TLE)));

  /* call the implementation-specific PSTATE update function to set
     the final value for PSTATE: */
  (*ic->_tme_sparc64_update_pstate)(ic, pstate, trap);

  /* if this is a clean_window trap: */
  if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_clean_window)) {
    cwp_addend = 1;
  }

  /* otherwise, if this is a window spill trap: */
  else if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_spill_normal(0))) {
    cwp_addend = ic->tme_sparc64_ireg_cansave + 2;
  }

  /* otherwise, if this is a window fill trap: */
  else if (tt == TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_fill_normal(0))) {
    cwp_addend = -1;
  }

  /* otherwise, this trap does not update CWP: */
  else {
    cwp_addend = 0;
  }

  /* if this trap updates CWP: */
  if (cwp_addend != 0) {

    /* update CWP: */
    cwp = ic->tme_sparc64_ireg_cwp;
    cwp += cwp_addend;
    assert (ic->tme_sparc64_ireg_winstates_mask != 0);
    cwp &= ic->tme_sparc64_ireg_winstates_mask;
    cwp = (tme_uint8_t) cwp;
    ic->tme_sparc64_ireg_cwp = cwp;
    TME_SPARC64_CWP_UPDATE(ic, cwp, cwp_offset);

    /* if this is a window spill or fill trap: */
    if (tt != TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_clean_window)) {

      /* make the exact trap vector: */
      wstate = ic->tme_sparc64_ireg_wstate;
      if (ic->tme_sparc64_ireg_otherwin) {
#if (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_spill_other(0)) - TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_spill_normal(0))) != (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_fill_other(0)) - TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_fill_normal(0)))
#error "TME_SPARC64_TRAP_ values changed"
#endif
	tt += (TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_spill_other(0))
	       - TME_SPARC_TRAP_TT(TME_SPARC64_TRAP_spill_normal(0)));
	wstate /= (TME_SPARC64_WSTATE_OTHER / TME_SPARC64_WSTATE_NORMAL);
      }
      tt += (4 * (wstate & TME_SPARC64_WSTATE_NORMAL));
    }
  }

  /* if we are in RED_state: */
  if (__tme_predict_false(ic->tme_sparc64_ireg_pstate & TME_SPARC64_PSTATE_RED)) {

    /* transfer control into the RED_state_trap_vector table: */
    pc = ic->tme_sparc64_rstvaddr;
  }

  /* otherwise, we are not in RED_state: */
  else {

    /* transfer control into the normal trap vector table: */
    pc = (ic->tme_sparc64_ireg_tl == 0 ? 0 : TME_BIT(14));
    pc |= ic->tme_sparc64_ireg_tba;
  }

  /* save the trap type: */
  ic->tme_sparc64_ireg_tt(tl) = tt;

  /* update TL: */
  ic->tme_sparc64_ireg_tl = tl;

  /* transfer control to the trap vector table: */
  pc += (tt * 0x20);
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT) = pc;
  ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC_NEXT_NEXT) = pc | sizeof(tme_uint32_t);

  /* log the trap: */
  tme_sparc_log(ic, 250, TME_OK,
		(TME_SPARC_LOG_HANDLE(ic),
		 _("trap tl %u tt 0x%03" TME_PRIx32 " handler-%%pc 0x%016" TME_PRIx64),
		 tl,
		 tt,
		 pc));

  /* redispatch: */
  ic->_tme_sparc_mode = TME_SPARC_MODE_EXECUTION;
  tme_sparc_redispatch(ic);
}

/* this enters the sparc64 error_state: */
void
tme_sparc64_trap_error_state(struct tme_sparc *ic)
{
  abort();
}

#endif /* TME_HAVE_INT64_T */

/* this returns the current instruction TLB entry: */
struct tme_sparc_tlb *
tme_sparc_itlb_current(struct tme_sparc *ic)
{
  struct tme_token *token;
  tme_uint32_t tlb_i;
  struct tme_sparc_tlb *itlb_current;

  /* there must be a current instruction TLB entry: */
  token = ic->_tme_sparc_itlb_current_token;
  assert (token != NULL);

  /* recover the index of the instruction TLB entry: */
  tlb_i
    = (
#ifdef TME_HAVE_INT64_T
       TME_SPARC_VERSION(ic) >= 9
       ? 
#if TME_HAVE_RECODE && TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
       (((struct tme_recode_tlb_c16_a64 *) 
	 (((char *) token)
	  - (((char *) &(((struct tme_recode_tlb_c16_a64 *) 0)->tme_recode_tlb_c16_a64_token))
	     - (char *) 0)))
	- &ic->tme_sparc_recode_tlb64s[0])
#else  /* !TME_HAVE_RECODE || TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_32 */
       (token - &ic->tme_sparc_tlb_tokens[0])
#endif /* !TME_HAVE_RECODE || TME_RECODE_SIZE_GUEST_MAX <= TME_RECODE_SIZE_32 */
       :
#endif /* TME_HAVE_INT64_T */
#if TME_HAVE_RECODE
       (((struct tme_recode_tlb_c16_a32 *) 
	 (((char *) token)
	  - (((char *) &(((struct tme_recode_tlb_c16_a32 *) 0)->tme_recode_tlb_c16_a32_token))
	     - (char *) 0)))
	- &ic->tme_sparc_recode_tlb32s[0])
#else  /* !TME_HAVE_RECODE */
       (token - &ic->tme_sparc_tlb_tokens[0])
#endif /* !TME_HAVE_RECODE */
       );
#if TME_HAVE_RECODE
  if (TME_SPARC_VERSION(ic) >= 9) {
#if TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32
    assert (token == &ic->tme_sparc_recode_tlb64s[tlb_i].tme_recode_tlb_c16_a64_token);
#endif /* TME_RECODE_SIZE_GUEST_MAX > TME_RECODE_SIZE_32 */
  }
  else {
    assert (token == &ic->tme_sparc_recode_tlb32s[tlb_i].tme_recode_tlb_c16_a32_token);
  }
#endif /* TME_HAVE_RECODE */

  /* get the current instruction TLB entry: */
  itlb_current = &ic->tme_sparc_tlbs[tlb_i];
  assert (itlb_current->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token == token);

  return (itlb_current);
}

/* this peeks at an instruction.  it returns all-bits-one if there is
   no valid instruction TLB entry that allows fast reading and applies
   to the address: */
tme_uint32_t
tme_sparc_insn_peek(struct tme_sparc *ic,
		    tme_sparc_ireg_umax_t pc_unmasked)
{
  tme_sparc_ireg_umax_t pc;
  tme_uint32_t tlb_hash;
  const struct tme_sparc_tlb *itlb;
  tme_uint32_t insn;
  const struct tme_sparc_tlb *itlb_current;

  /* mask the address: */
  pc = pc_unmasked;
#ifdef TME_HAVE_INT64_T
  if (TME_SPARC_VERSION(ic) >= 9) {
    pc &= ic->tme_sparc_address_mask;
  }
#endif /* TME_HAVE_INT64_T */

  /* the address must be 32-bit aligned: */
  assert ((pc % sizeof(tme_uint32_t)) == 0);

  /* NB: we don't have to check if the PC is in any virtual address
     hole, because we never make valid instruction TLB entries for
     addresses in a hole: */

  /* hash the instruction TLB entry: */
  tlb_hash = TME_SPARC_TLB_HASH(ic, ic->tme_sparc_memory_context_default, pc);
  itlb = &ic->tme_sparc_tlbs[TME_SPARC_ITLB_ENTRY(ic, tlb_hash)];

  /* if this instruction TLB entry is valid, covers this ASI and
     address, and allows fast reading: */
  if (tme_bus_tlb_is_valid(&itlb->tme_sparc_tlb_bus_tlb)
      && TME_SPARC_TLB_ASI_MASK_OK(itlb, ic->tme_sparc_asi_mask_insn)
      && itlb->tme_sparc_tlb_addr_first <= pc
      && pc <= itlb->tme_sparc_tlb_addr_last
      && itlb->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {

    /* fetch the instruction: */
    insn = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (itlb->tme_sparc_tlb_emulator_off_read + pc),
				 itlb->tme_sparc_tlb_bus_rwlock,
				 sizeof(tme_uint32_t),
				 (TME_SPARC_VERSION(ic) < 9
				  ? sizeof(tme_uint32_t)
				  : sizeof(tme_uint32_t) * 2));
    insn = tme_betoh_u32(insn);
    return (insn);
  }

  /* if there is recode support: */
  if (TME_SPARC_HAVE_RECODE(ic)) {

    /* return failure. if the PC doesn't hash to the current
       instruction TLB entry, the current instruction TLB entry won't
       cover it (because tme_sparc_recode_chain_tlb_update() limits
       instruction TLB entries to covering just one page).  if the PC
       does hash to the current instruction TLB entry, we just checked
       that above: */
    return (0xffffffff);
  }

  /* assume that we can't fetch the nearby instruction: */
  insn = 0xffffffff;

  /* if the current instruction TLB entry is valid and covers the address: */
  itlb_current = tme_sparc_itlb_current(ic);
  if (tme_bus_tlb_is_valid(&itlb_current->tme_sparc_tlb_bus_tlb)
      && itlb_current->tme_sparc_tlb_addr_first <= pc
      && pc <= itlb_current->tme_sparc_tlb_addr_last) {
    
    /* the current instruction TLB entry must cover this ASI and allow
       fast reading: */
    assert (TME_SPARC_TLB_ASI_MASK_OK(itlb_current, ic->tme_sparc_asi_mask_insn));
    assert (itlb_current->tme_sparc_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF);

    /* fetch the nearby instruction: */
    insn = tme_memory_bus_read32((const tme_shared tme_uint32_t *) (itlb_current->tme_sparc_tlb_emulator_off_read + pc),
				 itlb_current->tme_sparc_tlb_bus_rwlock,
				 sizeof(tme_uint32_t),
				 (TME_SPARC_VERSION(ic) < 9
				  ? sizeof(tme_uint32_t)
				  : sizeof(tme_uint32_t) * 2));
    insn = tme_betoh_u32(insn);
  }

  return (insn);
}

/* this peeks at an instruction at some offset from the current PC.
   it returns all-bits-one if there is no valid instruction TLB entry
   that allows fast reading and applies to the address: */
tme_uint32_t
tme_sparc_fetch_nearby(struct tme_sparc *ic, long offset_in_insns)
{
  tme_sparc_ireg_umax_t pc_unmasked;

  /* get the PC: */
  pc_unmasked
    = (
#ifdef TME_HAVE_INT64_T
       TME_SPARC_VERSION(ic) >= 9
       ? ((tme_uint64_t)
	  (ic->tme_sparc_ireg_uint64(TME_SPARC_IREG_PC)
	   + (tme_int64_t) (offset_in_insns * (long) sizeof(tme_uint32_t))))
       :
#endif /* TME_HAVE_INT64_T */
       ((tme_uint32_t)
	(ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC)
	 + (tme_int32_t) (offset_in_insns * (long) sizeof(tme_uint32_t)))));

  /* peek at the instruction: */
  return (tme_sparc_insn_peek(ic, pc_unmasked));
}

/* this unlocks data structures before a callout: */
void
tme_sparc_callout_unlock(struct tme_sparc *ic)
{
  struct tme_token *token;

  assert ((ic->_tme_sparc_mode == TME_SPARC_MODE_EXECUTION)
	  || (ic->_tme_sparc_itlb_current_token == NULL));

  /* if we have a busy instruction TLB entry: */
  token = ic->_tme_sparc_itlb_current_token;
  if (__tme_predict_true(token != NULL)) {

    /* unbusy the instruction TLB entry: */
    tme_token_unbusy(token);
  }
}

/* this relocks data structures after a callout: */
void
tme_sparc_callout_relock(struct tme_sparc *ic)
{
  struct tme_token *token;
  struct tme_sparc_tlb *tlb;

  assert ((ic->_tme_sparc_mode == TME_SPARC_MODE_EXECUTION)
	  || (ic->_tme_sparc_itlb_current_token == NULL));

  /* if we have a busy instruction TLB entry: */
  token = ic->_tme_sparc_itlb_current_token;
  if (__tme_predict_true(token != NULL)) {

    /* rebusy the instruction TLB entry: */
    tme_token_busy(token);
    tlb = tme_sparc_itlb_current(ic);
      
    /* if this instruction TLB entry is invalid, or isn't
       for the current context: */
    if (tme_bus_tlb_is_invalid(&tlb->tme_sparc_tlb_bus_tlb)
	|| (tlb->tme_sparc_tlb_context <= ic->tme_sparc_memory_context_max
	    && tlb->tme_sparc_tlb_context != ic->tme_sparc_memory_context_default)) {

      /* poison this instruction TLB entry, so we won't try to do any
	 fast fetches with it: */
      tlb->tme_sparc_tlb_addr_first = 1;
      tlb->tme_sparc_tlb_addr_last = 0;
    }
  }

  /* if we need to do an external check: */
  if (tme_memory_atomic_read_flag(&ic->tme_sparc_external_flag)) {

    /* after the currently executing instruction finishes, check for
       external resets, halts, or interrupts: */
    ic->_tme_sparc_instruction_burst_remaining = 0;
    ic->_tme_sparc_instruction_burst_other = TRUE;
  }
}

#if 0
#include <stdio.h>

/* this dumps out the sparc state: */
void
tme_sparc32_dump(const struct tme_sparc *ic)
{
  unsigned int cwp_first;
  unsigned int cwp;
  unsigned int reg_i;
  unsigned int reg_base;
  unsigned int ireg;

  /* dump out the windowed integer registers, finishing with the
     current window: */
  cwp_first = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP);
  cwp_first += TME_SPARC_NWINDOWS(ic) - 1;
  cwp_first %= TME_SPARC_NWINDOWS(ic);
  cwp = cwp_first;
  do {
    for (reg_i = 0; reg_i < 8; reg_i++) {
      for (reg_base = 24; reg_base > 8; reg_base -= 8) {
	
	ireg = reg_base + reg_i + (cwp * 16);
	if (ireg > ((TME_SPARC_NWINDOWS(ic) * 16) + 7)) {
	  ireg -= (TME_SPARC_NWINDOWS(ic) * 16);
	}

	fprintf(stderr,
		"w%u.%%%c%u[%p] = 0x%08x ",
		cwp,
		(reg_base == 24
		 ? 'i'
		 : 'l'),
		reg_i,
		&ic->tme_sparc_ireg_uint32(ireg),
		ic->tme_sparc_ireg_uint32(ireg));
      }
      fprintf(stderr, "\n");
    }
    cwp--;
    cwp %= TME_SPARC_NWINDOWS(ic);
  } while (cwp != cwp_first);

  /* dump out the global registers and the current window's output
     registers: */
  cwp = TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP);
  for (reg_i = 0; reg_i < 8; reg_i++) {

    ireg = reg_i;
    fprintf(stderr,
	    "   %%g%u[%p] = 0x%08x ",
	    ireg,
	    &ic->tme_sparc_ireg_uint32(ireg),
	    ic->tme_sparc_ireg_uint32(ireg));

    ireg = 8 + reg_i + (cwp * 16);
    if (ireg > ((TME_SPARC_NWINDOWS(ic) * 16) + 7)) {
      ireg -= (TME_SPARC_NWINDOWS(ic) * 16);
    }

    fprintf(stderr,
	    "w%u.%%o%u[%p] = 0x%08x ",
	    cwp,
	    reg_i,
	    &ic->tme_sparc_ireg_uint32(ireg),
	    ic->tme_sparc_ireg_uint32(ireg));
    fprintf(stderr, "\n");
  }

  /* dump out the PCs: */
  fprintf(stderr, "%%pc = 0x%08x  %%pc_next = 0x%08x  %%pc_next_next = 0x%08x\n",
	  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC),
	  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT),
	  ic->tme_sparc_ireg_uint32(TME_SPARC_IREG_PC_NEXT_NEXT));

  /* dump out the PSR: */
  fprintf(stderr, "%%psr = 0x%08x", ic->tme_sparc32_ireg_psr);
  fprintf(stderr, "  cwp = %u",
	  TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_CWP));
  fprintf(stderr, "  pil = 0x%x",
	  TME_FIELD_MASK_EXTRACTU(ic->tme_sparc32_ireg_psr, TME_SPARC32_PSR_PIL));
  if (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ET) {
    fprintf(stderr, " ET");
  }
  fprintf(stderr, "  %c",
	  (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_S
	   ? 'S'
	   : 'U'));
  fprintf(stderr, "  flags:");
  if (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ICC_N) {
    fprintf(stderr, " N");
  }
  if (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ICC_Z) {
    fprintf(stderr, " Z");
  }
  if (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ICC_V) {
    fprintf(stderr, " V");
  }
  if (ic->tme_sparc32_ireg_psr & TME_SPARC32_PSR_ICC_C) {
    fprintf(stderr, " C");
  }
  fprintf(stderr, "\n");

  /* dump out the instruction and the WIM: */
  fprintf(stderr, "insn = 0x%08x %%wim = 0x%08x\n",
	  ic->_tme_sparc_insn,
	  ic->tme_sparc32_ireg_wim);
}

void
tme_sparc64_dump_memory(struct tme_sparc *ic, tme_uint64_t address, tme_uint32_t resid)
{
  tme_uint64_t address_display;
  tme_uint32_t tlb_hash;
  struct tme_sparc_tlb *dtlb;
  tme_memory_atomic_flag_t tlb_busy_old;
  const tme_shared tme_uint8_t *memory;
  tme_uint32_t count;
  tme_uint32_t byte_i;

  /* we always display aligned rows: */
  address_display = address & (((tme_uint32_t) 0) - (sizeof(tme_uint32_t) * 2));
  resid += (address - address_display);

  /* while we have memory to dump: */
  for (; resid > 0; ) {

    /* get the DTLB entry, and busy it if it isn't already: */
    tlb_hash = TME_SPARC_TLB_HASH(ic, ic->tme_sparc_memory_context_default, address_display);
    dtlb = &ic->tme_sparc_tlbs[TME_SPARC_DTLB_ENTRY(ic, tlb_hash)];
    tlb_busy_old = dtlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token->tme_token_busy;
    dtlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token->tme_token_busy = TRUE;

    /* read more data: */
    count = TME_MIN(resid, sizeof(tme_uint32_t) * 2);
    memory
      = (TME_SPARC_VERSION(ic) < 9
	 ? tme_sparc32_ls(ic,
			  address_display,
			  (tme_uint32_t *) NULL,
			  (TME_SPARC_LSINFO_SIZE(sizeof(tme_uint32_t) * 2)
			   + TME_SPARC_LSINFO_OP_LD
			   + TME_SPARC_LSINFO_NO_FAULT))
	 : tme_sparc64_ls(ic,
			  address_display,
			  (tme_uint64_t *) NULL,
			  (TME_SPARC_LSINFO_SIZE(sizeof(tme_uint32_t) * 2)
			   + TME_SPARC_LSINFO_OP_LD
			   + TME_SPARC_LSINFO_NO_FAULT)));

    /* restore the DTLB busy flag: */
    dtlb->tme_sparc_tlb_bus_tlb.tme_bus_tlb_token->tme_token_busy = tlb_busy_old;

    /* display the row: */
    fprintf(stderr, "0x%0*" TME_PRIx64 " ", (8 << (TME_SPARC_VERSION(ic) >= 9)), address_display);
    for (byte_i = 0;
	 byte_i < count;
	 byte_i++, address_display++) {
      if (address_display < address) {
	fprintf(stderr, "   ");
      }
      else {
	fprintf(stderr, " %02x",
		memory[address_display]);
	address++;
      }
      resid--;
    }
    fputc('\n', stderr);
  }
}

#undef TME_SPARC_VERSION
#define TME_SPARC_VERSION(ic) (9)
#include "sparc-kgdb.c"
#undef TME_SPARC_VERSION

#endif /* 1 */
