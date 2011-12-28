/* $Id: sun4-mainbus.c,v 1.6 2009/08/30 14:03:11 fredette Exp $ */

/* machine/sun4/sun4-mainbus.c - implementation of Sun 4 emulation: */

/*
 * Copyright (c) 2003, 2004, 2006 Matt Fredette
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
_TME_RCSID("$Id: sun4-mainbus.c,v 1.6 2009/08/30 14:03:11 fredette Exp $");

/* includes: */
#include "sun4-impl.h"
#include <tme/ic/z8530.h>
#include <tme/ic/isil7170.h>
#include <tme/ic/mk48txx.h>
#include <stdio.h>

/* this possibly updates that the interrupt priority level driven to the CPU: */
int
_tme_sun4_ipl_check(struct tme_sun4 *sun4)
{
  unsigned int ipl;
  unsigned int ipl_index;
  tme_uint8_t ipl_mask;
  tme_uint32_t interreg;
  int rc;

  /* find the highest ipl now asserted on the buses, that is enabled
     to the CPU: */
  ipl = TME_SPARC_IPL_MAX;
  ipl_index = ipl / 8;
  ipl_mask = TME_BIT(ipl % 8);
  do {

    /* if this ipl is asserted: */
    if ((sun4->tme_sun4_int_signals[ipl_index] & ipl_mask) != 0) {

      /* if this ipl is not disabled: */
      if (TME_SUN4_IS_SUN44C(sun4)
	  ? ((sun4->tme_sun44c_ints & TME_SUN44C_IREG_INTS_ENAB)
	     && (ipl != 10 || (sun4->tme_sun44c_ints & TME_SUN44C_IREG_COUNTER_L10))
	     && (ipl != 14 || (sun4->tme_sun44c_ints & TME_SUN44C_IREG_COUNTER_L14)))
	  : TRUE) {

	/* stop now: */
	break;
      }
    }

    /* advance to the next ipl: */
    ipl--;
    ipl_mask >>= 1;
    if (ipl_mask == 0) {
      ipl_mask = 0x80;
      ipl_index--;
    }
  } while (ipl != TME_SPARC_IPL_NONE);

  /* if this is a sun4 or sun4c: */
  if (TME_SUN4_IS_SUN44C(sun4)) {
    
    /* get the sun4/4c interrupt register: */
    interreg = sun4->tme_sun44c_ints;

    /* if interrupts are enabled: */
    if (interreg & TME_SUN44C_IREG_INTS_ENAB) {

      /* check the soft interrupts: */
      if (interreg & TME_SUN44C_IREG_SOFT_INT_L6) {
	ipl = TME_MAX(ipl, 6);
      }
      else if (interreg & TME_SUN44C_IREG_SOFT_INT_L4) {
	ipl = TME_MAX(ipl, 4);
      }
      else if (interreg & TME_SUN44C_IREG_SOFT_INT_L1) {
	ipl = TME_MAX(ipl, 1);
      }
    }
  }

  /* possibly update the CPU: */
  if (ipl != sun4->tme_sun4_int_ipl_last) {
    sun4->tme_sun4_int_ipl_last = ipl;
    rc = ((*sun4->tme_sun4_sparc->tme_sparc_bus_interrupt)
	  (sun4->tme_sun4_sparc, ipl));
    assert (rc == TME_OK);
  }

  /* return nonzero if any interrupt is pending: */
  return (ipl != TME_SPARC_IPL_NONE);
}

/* our mainbus signal handler: */
static int
_tme_sun4_bus_signal(struct tme_bus_connection *conn_bus_raiser, unsigned int signal)
{
  struct tme_sun4 *sun4;
  int signal_asserted;
  unsigned int ipl, ipl_index;
  tme_uint8_t ipl_mask;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_bus_raiser->tme_bus_connection.tme_connection_element->tme_element_private;

  /* see whether the signal is asserted or negated: */
  signal_asserted = TRUE;
  switch (signal & TME_BUS_SIGNAL_LEVEL_MASK) {
  case TME_BUS_SIGNAL_LEVEL_NEGATED:
    signal_asserted = FALSE;
  case TME_BUS_SIGNAL_LEVEL_ASSERTED:
    break;
  default:
    abort();
  }
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* dispatch on the signal: */

  /* halt: */
  if (signal == TME_BUS_SIGNAL_HALT) {
    abort();
  }

  /* reset: */
  else if (signal == TME_BUS_SIGNAL_RESET) {
    /* XXX reset is just ignored for now: */
  }

  /* an interrupt signal: */
  else if (TME_BUS_SIGNAL_IS_INT(signal)) {
    ipl = TME_BUS_SIGNAL_INDEX_INT(signal);
    if (ipl >= TME_SPARC_IPL_MIN
	&& ipl <= TME_SPARC_IPL_MAX) {
      
      /* update this ipl in the byte array: */
      ipl_index = ipl / 8;
      ipl_mask = TME_BIT(ipl % 8);
      sun4->tme_sun4_int_signals[ipl_index]
	= ((sun4->tme_sun4_int_signals[ipl_index]
	    & ~ipl_mask)
	   | (signal_asserted
	      ? ipl_mask
	      : 0));

      /* possibly update the ipl being driven to the CPU: */
      _tme_sun4_ipl_check(sun4);
      return (TME_OK);
    }
  }

  /* an unknown signal: */
  else {
    abort();
  }

  return (TME_OK);
}

/* this handles a CPU interrupt acknowledge: */
static int
_tme_sun4_bus_intack(struct tme_bus_connection *conn_sparc, unsigned int ipl, int *vector)
{
  struct tme_sun4 *sun4;
  tme_uint32_t interreg;
  unsigned int signal;
  int rc;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn_sparc->tme_bus_connection.tme_connection_element->tme_element_private;

  /* acknowledge any soft interrupt: */
  if (TME_SUN4_IS_SUN44C(sun4)) {
    interreg = sun4->tme_sun44c_ints;
    if ((ipl == 6
	 && (interreg & TME_SUN44C_IREG_SOFT_INT_L6))
	|| (ipl == 4
	    && (interreg & TME_SUN44C_IREG_SOFT_INT_L4))
	|| (ipl == 1
	    && (interreg & TME_SUN44C_IREG_SOFT_INT_L1))) {
      *vector = TME_BUS_INTERRUPT_VECTOR_UNDEF;
      return (TME_OK);
    }
  }

  /* turn the ipl into a bus signal number: */
  signal = TME_BUS_SIGNAL_INT(ipl);

  /* try the acknowledge on these buses, in order: */

  /* obio: */
  rc = (*sun4->tme_sun4_32_obio->tme_bus_intack)
    (sun4->tme_sun4_32_obio, signal, vector);
  if (rc != ENOENT) {
    return (rc);
  }
  /* obmem: */
  if (!TME_SUN4_IS_SUN4C(sun4)) {
    rc = (*sun4->tme_sun4_32_obmem->tme_bus_intack)
      (sun4->tme_sun4_32_obmem, signal, vector);
    if (rc != ENOENT) {
      return (rc);
    }
  }
  /* VMEbus: */
  if (TME_SUN4_IS_SUN4(sun4)) {
    rc = (*sun4->tme_sun4_vmebus->tme_bus_intack)
      (sun4->tme_sun4_vmebus, signal, vector);
    if (rc != ENOENT) {
      return (rc);
    }
  }

  /* done: */
  return (rc);
}

/* this resets the board: */
int
_tme_sun4_reset(struct tme_sun4 *sun4, int soft)
{

  /* clear the sun4/4c enable register: */
  sun4->tme_sun44c_enable = 0;
  
  /* clear the sun4/4c interrupt register: */
  sun4->tme_sun44c_ints = 0;

  /* clear any NMI: */
  sun4->tme_sun4_int_signals[TME_SPARC_IPL_NMI / 8] &= ~TME_BIT(TME_SPARC_IPL_NMI % 8);

  /* reset the CPU: */
  (*sun4->tme_sun4_sparc->tme_sparc_bus_connection.tme_bus_signal)
    (&sun4->tme_sun4_sparc->tme_sparc_bus_connection,
     TME_BUS_SIGNAL_RESET
     | TME_BUS_SIGNAL_LEVEL_NEGATED
     | TME_BUS_SIGNAL_EDGE);

  /* reset all busses: */
  (*sun4->tme_sun4_32_obio->tme_bus_signal)
    (sun4->tme_sun4_32_obio,
     TME_BUS_SIGNAL_RESET
     | TME_BUS_SIGNAL_LEVEL_NEGATED
     | TME_BUS_SIGNAL_EDGE);
  if (!TME_SUN4_IS_SUN4C(sun4)) {
    (*sun4->tme_sun4_32_obmem->tme_bus_signal)
      (sun4->tme_sun4_32_obmem,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
  }
  if (TME_SUN4_IS_SUN4(sun4)) {
    (*sun4->tme_sun4_vmebus->tme_bus_signal)
      (sun4->tme_sun4_vmebus,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
  }

  /* in case this reset was caused by a bus cycle, be sure to tell the
     initiator to check for a synchronous event: */
  return (TME_BUS_CYCLE_SYNCHRONOUS_EVENT);
}

/* our command function: */
static int
_tme_sun4_command(struct tme_element *element, const char * const * args, char **_output)
{
  struct tme_sun4 *sun4;
  int do_reset;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) element->tme_element_private;

  /* assume no reset: */
  do_reset = FALSE;

  /* the "power" command: */
  if (TME_ARG_IS(args[1], "power")) {

    if (TME_ARG_IS(args[2], "up")
	&& args[3] == NULL) {
      do_reset = TRUE;
    }

    else if (TME_ARG_IS(args[2], "down")
	     && args[3] == NULL) {
      /* nothing */
    }

    /* return an error: */
    else {
      tme_output_append_error(_output,
			      "%s %s power [ up | down ]",
			      _("usage:"),
			      args[0]);
      return (EINVAL);
    }
  }

  /* the "diag-switch" command: */
  else if (TME_SUN4_IS_SUN4(sun4)
	   && TME_ARG_IS(args[1], "diag-switch")) {

    if (args[2] == NULL) {
      tme_output_append_error(_output,
			      "diag-switch %s",
			      (sun4->tme_sun44c_enable & TME_SUN4_ENA_DIAG
			       ? "true"
			       : "false"));
    }

    else if (TME_ARG_IS(args[2], "true")
	     && args[3] == NULL) {
      sun4->tme_sun44c_enable |= TME_SUN4_ENA_DIAG;
    }

    else if (TME_ARG_IS(args[2], "false")
	     && args[3] == NULL) {
      sun4->tme_sun44c_enable &= ~TME_SUN4_ENA_DIAG;
    }

    /* return an error: */
    else {
      tme_output_append_error(_output,
			      "%s %s diag-switch [ true | false ]",
			      _("usage:"),
			      args[0]);
      return (EINVAL);
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
			    _("available %s commands: %s%s"),
			    args[0],
			    "power",
			    (TME_SUN4_IS_SUN4(sun4)
			     ? "diag-switch"
			     : ""));
    return (EINVAL);
  }

  if (do_reset) {
    _tme_sun4_reset(sun4, FALSE);
  }

  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_sun4_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_sun4_bus_connection *conn_sun4;
  struct tme_bus_connection *conn_bus;
  struct tme_sun4 *sun4;
  unsigned int score;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn->tme_connection_element->tme_element_private;

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_sparc = (struct tme_sparc_bus_connection *) conn->tme_connection_other;
  conn_sun4 = (struct tme_sun4_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {

    /* this must be an SPARC chip, and not another bus: */
  case TME_CONNECTION_BUS_SPARC:
    if (conn_bus->tme_bus_tlb_set_add == NULL
	&& conn_sparc->tme_sparc_bus_tlb_fill == NULL
	&& conn_sparc->tme_sparc_bus_fpu_strict != NULL) {
      score = 10;
    }
    break;

    /* this must be a bus, and not a chip, and the bus must still be
       free: */
  case TME_CONNECTION_BUS_GENERIC:
    if ((conn_bus->tme_bus_tlb_set_add != NULL
	 && conn_bus->tme_bus_tlb_fill != NULL)
	&& (conn_sun4->tme_sun4_bus_connection_which >= TME_SUN4_32_CONN_BUS_COUNT
	    || sun4->tme_sun4_buses[conn_sun4->tme_sun4_bus_connection_which] == NULL)) {
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
_tme_sun4_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun4 *sun4;
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_sun4_bus_connection *conn_sun4;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn_other;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) conn->tme_connection_element->tme_element_private;

  /* dispatch on the connection type: */
  conn_other = conn->tme_connection_other;
  conn_sparc = (struct tme_sparc_bus_connection *) conn_other;
  conn_sun4 = (struct tme_sun4_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn_other;
  switch (conn->tme_connection_type) {
      
  case TME_CONNECTION_BUS_SPARC:
    sun4->tme_sun4_sparc = conn_sparc;
    break;
      
  case TME_CONNECTION_BUS_GENERIC:

    /* remember the connection to this bus: */
    if (state == TME_CONNECTION_FULL) {
      assert (sun4->tme_sun4_buses[conn_sun4->tme_sun4_bus_connection_which] == NULL);
      sun4->tme_sun4_buses[conn_sun4->tme_sun4_bus_connection_which] = conn_bus;
    }
    break;

  default: 
    assert(FALSE);
    break;
  }
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_sun4_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes new connection sides: */
static int
_tme_sun4_connections_new(struct tme_element *element, const char * const *args, struct tme_connection **_conns, char **_output)
{
  struct tme_sparc_bus_connection *conn_sparc;
  struct tme_sun4_bus_connection *conn_sun4;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  struct tme_sun4 *sun4;
  char *free_buses;
  int which_conn;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) element->tme_element_private;

  /* if we have no arguments, and we don't have a CPU yet, we can take
     a SPARC bus connection: */
  if (args[1] == NULL
      && sun4->tme_sun4_sparc == NULL) {

    /* create our side of a SPARC bus connection: */
    conn_sparc = tme_new0(struct tme_sparc_bus_connection, 1);
    conn_bus = &conn_sparc->tme_sparc_bus_connection;
    conn = &conn_bus->tme_bus_connection;
    conn->tme_connection_next = *_conns;

    /* fill in the generic connection: */
    conn->tme_connection_type = TME_CONNECTION_BUS_SPARC;
    conn->tme_connection_score = _tme_sun4_connection_score;
    conn->tme_connection_make = _tme_sun4_connection_make;
    conn->tme_connection_break = _tme_sun4_connection_break;

    /* fill in the generic bus connection: */
    conn_bus->tme_bus_signal = _tme_sun4_bus_signal;
    conn_bus->tme_bus_intack = _tme_sun4_bus_intack;
    conn_bus->tme_bus_tlb_set_add
      = (TME_SUN4_IS_SUN44C(sun4)
	 ? _tme_sun44c_mmu_tlb_set_add
	 : NULL);

    /* full in the SPARC bus connection: */
    conn_sparc->tme_sparc_bus_tlb_fill
      = (TME_SUN4_IS_SUN44C(sun4)
	 ? _tme_sun44c_tlb_fill_sparc
	 : NULL);

    /* add in this connection side possibility: */
    *_conns = conn;
  }

  /* create our side of a generic bus connection: */
  conn_sun4 = tme_new0(struct tme_sun4_bus_connection, 1);
  conn_bus = &conn_sun4->tme_sun4_bus_connection;
  conn = &conn_bus->tme_bus_connection;
  conn->tme_connection_next = *_conns;

  /* fill in the generic connection: */
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_sun4_connection_score;
  conn->tme_connection_make = _tme_sun4_connection_make;
  conn->tme_connection_break = _tme_sun4_connection_break;
    
  /* fill in the generic bus connection: */
  conn_bus->tme_bus_signal = _tme_sun4_bus_signal;
  conn_bus->tme_bus_intack = NULL;
  if (TME_SUN4_IS_SUN44C(sun4)) {
    conn_bus->tme_bus_tlb_fill = _tme_sun44c_tlb_fill_bus;
  }
  else {
    abort();
  }

  /* if we have no argument: */
  if (args[1] == NULL) {

    /* return no more connections: */
    tme_free(conn_sun4);
    return (TME_OK);
  }

  /* otherwise, we have at least one argument: */
  else {

    /* we must have no other arguments: */
    if (args[2] != NULL) {
      tme_output_append_error(_output,
			      "%s %s",
			      args[2],
			      _("unexpected"));
      tme_free(conn_sun4);
      return (EINVAL);
    }

    /* start the list of buses that we don't have yet: */
    free_buses = NULL;

    /* poison the which connection: */
    which_conn = -1;

    /* check each bus: */

    /* obio: */
    if (sun4->tme_sun4_32_obio == NULL) {
      tme_output_append(&free_buses, 
			(TME_SUN4_IS_SUN4C(sun4)
			 ? " sbus"
			 : " obio"));
    }
    if (TME_ARG_IS(args[1],
		   (TME_SUN4_IS_SUN4C(sun4)
		    ? "sbus"
		    : "obio"))) {
      which_conn = TME_SUN4_32_CONN_BUS_OBIO;
    }

    /* obmem: */
    if (!TME_SUN4_IS_SUN4C(sun4)) {
      if (sun4->tme_sun4_32_obmem == NULL) {
	tme_output_append(&free_buses, " obmem");
      }
      if (TME_ARG_IS(args[1], "obmem")) {
	which_conn = TME_SUN4_32_CONN_BUS_OBMEM;
      }
    }

    /* VMEbus: */
    if (TME_SUN4_IS_SUN4(sun4)) {
      if (sun4->tme_sun4_vmebus == NULL) {
	tme_output_append(&free_buses, " vme");
      }
      if (TME_ARG_IS(args[1], "vme")) {
	which_conn = TME_SUN4_CONN_BUS_VME;
	conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	  = TME_SUN4_DVMA_SIZE_VME;
      }
    }

    /* random connections: */
    
    if (TME_ARG_IS(args[1], "timer")) {
      which_conn = TME_SUN4_32_CONN_REG_TIMER;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= TME_SUN44C_TIMER_SIZ_REG * 2;
    }
    else if (TME_ARG_IS(args[1], "memerr")) {
      which_conn = TME_SUN4_32_CONN_REG_MEMERR;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= (TME_SUN4_IS_MODEL(sun4, TME_SUN_IDPROM_TYPE_CODE_CALVIN)
	   ? (TME_SUN44C_MEMERR_SIZ_REG * 2)
	   : TME_SUN44C_MEMERR_SIZ_REG);
    }
    else if (TME_ARG_IS(args[1], "intreg")) {
      which_conn = TME_SUN4_32_CONN_REG_INTREG;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= sizeof(sun4->tme_sun44c_ints);
    }
    else if (TME_SUN4_IS_SUN4C4M(sun4)
	     && TME_ARG_IS(args[1], "auxreg")) {
      which_conn = TME_SUN4C4M_CONN_REG_AUXREG;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= sizeof(sun4->tme_sun4c4m_aux);
    }

    /* if the which connection is still poison, or if this is trying
       to connect a bus that we already have, complain: */
    if (which_conn < 0
	|| (which_conn < TME_SUN4_32_CONN_BUS_COUNT
	    && sun4->tme_sun4_buses[which_conn] != NULL)) {
      if (which_conn < 0) {
	tme_output_append_error(_output,
				"%s %s ",
				_("unknown bus or register:"),
				args[1]);
      }
      if (free_buses != NULL) {
	tme_output_append_error(_output, 
				"%s %s",
				_("remaining buses:"),
				free_buses);
	tme_free(free_buses);
      }
      else {
	tme_output_append_error(_output, _("all buses present"));
      }
      tme_free(conn_sun4);
      return (EINVAL);
    }

    /* free the remaining bus list: */
    if (free_buses != NULL) {
      tme_free(free_buses);
    }

    /* fill in the sun4 connection: */
    conn_sun4->tme_sun4_bus_connection_which = which_conn;
    
    /* fill in the generic bus connection for a bus: */
    if (which_conn < TME_SUN4_32_CONN_BUS_COUNT) {
      conn_bus->tme_bus_tlb_set_add = _tme_sun44c_mmu_tlb_set_add;
    }
  }

  /* add in this connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* this creates a new sun4 element: */
TME_ELEMENT_NEW_DECL(tme_machine_sun4) {
  int usage;
  struct tme_sun4 *sun4;
  const char *idprom_filename;
  FILE *idprom_fp;
  tme_uint8_t idprom[TME_SUN_IDPROM_SIZE];
  int arg_i;

  arg_i = 1;
  usage = FALSE;

  /* if we've been given an IDPROM filename: */
  idprom_filename = NULL;
  if (TME_ARG_IS(args[arg_i], "idprom")) {
    arg_i++;
    idprom_filename = args[arg_i];
    if (idprom_filename == NULL) {
      usage = TRUE;
    }
    else {
      arg_i++;
    }
  }

  /* otherwise, if we've been given a board name: */
  else if (TME_ARG_IS(args[arg_i], "name")) {
    arg_i++;
    if (TME_ARG_IS(args[arg_i], "Calvin")) {
      idprom[TME_SUN_IDPROM_OFF_MACHTYPE] = TME_SUN_IDPROM_TYPE_CODE_CALVIN;
      arg_i++;
    }
    else if (args[arg_i] != NULL) {
      tme_output_append_error(_output,
			      "%s %s, ",
			      _("unknown name"),
			      args[arg_i]);
      usage = TRUE;
    }
    else {
      tme_output_append_error(_output,
			      "%s, ",
			      _("missing name"));
      usage = TRUE;
    }
  }

  /* we must have no more arguments: */
  if (args[arg_i] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[arg_i],
			    _("unexpected"));
    usage = TRUE;
  }

  /* if our usage was bad: */
  if (usage) {
    tme_output_append_error(_output,
			    "%s %s { idprom IDPROM%s | name { Calvin } } ",
			    _("usage:"),
			    args[0],
			    _("-FILENAME"));
    return (EINVAL);
  }

  /* if we've been given an IDPROM filename, try to read it in: */
  if (idprom_filename != NULL) {
    idprom_fp = fopen(idprom_filename, "r");
    if (idprom_fp == NULL) {
      tme_output_append_error(_output, idprom_filename);
      return (errno);
    }
    if (fread(idprom, sizeof(tme_uint8_t), sizeof(idprom), idprom_fp) != sizeof(idprom)) {
      tme_output_append_error(_output, idprom_filename);
      fclose(idprom_fp);
      return (ENOEXEC);
    }
    fclose(idprom_fp);
  }

  /* allocate and initialize the new sun4: */
  sun4 = tme_new0(struct tme_sun4, 1);
  sun4->tme_sun4_element = element;
  tme_mutex_init(&sun4->tme_sun4_mutex);

  /* set the IDPROM: */
  memcpy(sun4->tme_sun4_idprom_contents, idprom, sizeof(idprom));
  
  /* the MMU: */
  if (TME_SUN4_IS_SUN44C(sun4)) {
    _tme_sun44c_mmu_new(sun4);
    sun4->tme_sun4_tlb_fill = _tme_sun44c_tlb_fill_mmu;
  }
  else {
    abort();
  }

  /* the cache: */
  if (TME_SUN4_IS_SUN44C(sun4)) {
    _tme_sun44c_cache_new(sun4);
  }
  else {
    abort();
  }

  /* the control ASIs: */
  sun4->tme_sun4_asis[TME_SUN4_32_ASI_CONTROL].tme_sun4_asi_sun4 = sun4;
  if (TME_SUN4_IS_SUN44C(sun4)) {
    sun4->tme_sun4_asis[TME_SUN44C_ASI_SEGMAP].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN44C_ASI_PGMAP].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN4C_ASI_HW_FLUSH_SEG /* TME_SUN4_ASI_COPY */].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN4C_ASI_HW_FLUSH_PG /* TME_SUN4_ASI_REGMAP */].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN4C_ASI_HW_FLUSH_CONTEXT /* TME_SUN4_ASI_FLUSH_REG */].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN44C_ASI_FLUSH_SEG].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN44C_ASI_FLUSH_PG].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN44C_ASI_FLUSH_CONTEXT].tme_sun4_asi_sun4 = sun4;
    sun4->tme_sun4_asis[TME_SUN4C_ASI_HW_FLUSH_ALL /* TME_SUN4_ASI_FLUSH_USER */].tme_sun4_asi_sun4 = sun4;
  }
  else {
    abort();
  }

  /* the timers: */
  _tme_sun4_timer_new(sun4);

  /* fill the element: */
  element->tme_element_private = sun4;
  element->tme_element_connections_new = _tme_sun4_connections_new;
  element->tme_element_command = _tme_sun4_command;

  return (TME_OK);
}

/* this creates a new Sun-4 isil7170: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun4,oclock) {
  struct tme_isil7170_socket socket;
  const char *sub_args[4];
  int arg_i;

  /* create the isil7170 socket: */
  memset (&socket, 0, sizeof(socket));
  socket.tme_isil7170_socket_version = TME_ISIL7170_SOCKET_0;
  socket.tme_isil7170_socket_addr_shift = 0;
  socket.tme_isil7170_socket_port_least_lane = 3; /* D31-D24 */
  socket.tme_isil7170_socket_clock_basic = TME_ISIL7170_FREQ_32K;
  socket.tme_isil7170_socket_int_signal = TME_BUS_SIGNAL_INT_UNSPEC;

  /* create the isil7170.  we allow at most two arguments to pass
     through, which is an awful hack: */
  sub_args[0] = "tme/ic/isil7170";
  for (arg_i = 1;; arg_i++) {
    if (arg_i == TME_ARRAY_ELS(sub_args)) {
      abort();
    }
    sub_args[arg_i] = args[arg_i];
    if (args[arg_i] == NULL) {
      break;
    }
  }
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}

/* this creates a new Sun-4 mk48txx: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun4,clock) {
  struct tme_mk48txx_socket socket;

  /* create the mk48txx socket: */
  memset (&socket, 0, sizeof(socket));
  socket.tme_mk48txx_socket_version = TME_MK48TXX_SOCKET_0;
  socket.tme_mk48txx_socket_addr_shift = 0;
  socket.tme_mk48txx_socket_port_least_lane = 3; /* D31-D24 */
  socket.tme_mk48txx_socket_year_zero = 1968;

  if (!TME_ARG_IS(args[1], "type")
      || args[2] == NULL) {
    tme_output_append_error(_output,
			    "%s %s type CLOCK-%s",
			    _("usage:"),
			    args[0],
			    _("TYPE"));
    return (EINVAL);
  }

  return (tme_element_new(element, args + 2, &socket, _output));
}

/* this creates a new Sun-4 z8530: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun4,zs) {
  struct tme_z8530_socket socket = TME_SUN_Z8530_SOCKET_INIT;
  char *sub_args[2];

  /* create the z8530: */
  sub_args[0] = "tme/ic/z8530";
  sub_args[1] = NULL;
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}
