/* $Id: sun3-mainbus.c,v 1.7 2009/08/30 14:17:53 fredette Exp $ */

/* machine/sun3/sun3-mainbus.c - implementation of Sun 3 emulation: */

/*
 * Copyright (c) 2003, 2004 Matt Fredette
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
_TME_RCSID("$Id: sun3-mainbus.c,v 1.7 2009/08/30 14:17:53 fredette Exp $");

/* includes: */
#include "sun3-impl.h"
#include <tme/ic/z8530.h>
#include <tme/ic/isil7170.h>
#include <stdio.h>

/* macros: */
#define TME_BUS_SIGNAL_INT_CLOCK	TME_BUS_SIGNAL_INT(8)

/* this possibly updates that the interrupt priority level driven to the CPU: */
int
_tme_sun3_ipl_check(struct tme_sun3 *sun3)
{
  tme_uint8_t interreg;
  unsigned int ipl, ipl_index;
  tme_uint8_t ipl_mask;

  /* get the interrupt register: */
  interreg = sun3->tme_sun3_ints;

  /* assume that interrupts are completely masked: */
  ipl = TME_M68K_IPL_NONE;

  /* if interrupts are enabled: */
  if (interreg & TME_SUN3_IREG_INTS_ENAB) {

    /* find the highest ipl now asserted on the buses: */
    for (ipl = TME_M68K_IPL_MAX;
	 ipl > TME_M68K_IPL_NONE;
	 ipl--) {
      ipl_index = ipl >> 3;
      ipl_mask = TME_BIT(ipl & 7);
      if (sun3->tme_sun3_int_signals[ipl_index] & ipl_mask) {
	break;
      }
    }
    
    /* check the soft interrupts: */
    if (interreg & TME_SUN3_IREG_SOFT_INT_3) {
      ipl = TME_MAX(ipl, 3);
    }
    else if (interreg & TME_SUN3_IREG_SOFT_INT_2) {
      ipl = TME_MAX(ipl, 2);
    }
    else if (interreg & TME_SUN3_IREG_SOFT_INT_1) {
      ipl = TME_MAX(ipl, 1);
    }
  }
  
  /* possibly update the CPU: */
  if (ipl != sun3->tme_sun3_int_ipl_last) {
    sun3->tme_sun3_int_ipl_last = ipl;
    return ((*sun3->tme_sun3_m68k->tme_m68k_bus_interrupt)
	    (sun3->tme_sun3_m68k, ipl));
  }
  return (TME_OK);
}

/* our mainbus signal handler: */
static int
_tme_sun3_bus_signal(struct tme_bus_connection *conn_bus_raiser, unsigned int signal)
{
  struct tme_sun3 *sun3;
  int signal_asserted;
  unsigned int ipl, ipl_index;
  tme_uint8_t ipl_mask;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn_bus_raiser->tme_bus_connection.tme_connection_element->tme_element_private;

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

  /* ipl 8 doesn't really exist - it's the interrupt signal from the
     isil7170.  we must map it into either ipl 7 (NMI) or ipl 5
     (clock) depending on the state of the interrupt register: */
  if (signal == TME_BUS_SIGNAL_INT_CLOCK) {

    /* if the signal is being asserted: */
    if (signal_asserted) {

      /* map the interrupt signal: */
      if (sun3->tme_sun3_ints & TME_SUN3_IREG_CLOCK_ENAB_5) {
	signal = TME_BUS_SIGNAL_INT(5);
      }
      else if (sun3->tme_sun3_ints & TME_SUN3_IREG_CLOCK_ENAB_7) {
	signal = TME_BUS_SIGNAL_INT(7);
      }
      else {
	/* the clock interrupt isn't connected to any ipl: */
	signal = TME_BUS_SIGNAL_INT_UNSPEC;
      }

      /* remember the last clock interrupt signal: */
      sun3->tme_sun3_int_signal_clock_last = signal;
    }

    else {

      /* recover the last clock interrupt signal: */
      signal = sun3->tme_sun3_int_signal_clock_last;
    }

    /* if we're supposed to ignore this signal: */
    if (signal == TME_BUS_SIGNAL_INT_UNSPEC) {
      return (TME_OK);
    }
  }

  /* dispatch on the signal: */

  /* halt: */
  if (signal == TME_BUS_SIGNAL_HALT) {
    abort();
  }

  /* reset: */
  else if (signal == TME_BUS_SIGNAL_RESET) {

    /* if this reset signal is coming from the m68k: */
    if (conn_bus_raiser->tme_bus_connection.tme_connection_other
	== &sun3->tme_sun3_m68k->tme_m68k_bus_connection.tme_bus_connection) {

      /* XXX FIXME - note that the do_reset code is lazy and only
	 sends reset negation edges out to the busses.  we are also
	 lazy (and also worried that sending out assertion edges might
	 break things), so we only send out reset negation edges too: */
      if (signal_asserted) {
	return (TME_OK);
      }

      /* propagate this reset edge to all busses: */
      (*sun3->tme_sun3_obio->tme_bus_signal)
	(sun3->tme_sun3_obio,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
      (*sun3->tme_sun3_obmem->tme_bus_signal)
	(sun3->tme_sun3_obmem,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
      (*sun3->tme_sun3_vmebus->tme_bus_signal)
	(sun3->tme_sun3_vmebus,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
    }
  }

  /* an interrupt signal: */
  else if (TME_BUS_SIGNAL_IS_INT(signal)) {
    ipl = TME_BUS_SIGNAL_INDEX_INT(signal);
    if (ipl >= TME_M68K_IPL_MIN
	&& ipl <= TME_M68K_IPL_MAX) {
      
      /* update this ipl in the byte array: */
      ipl_index = ipl >> 3;
      ipl_mask = TME_BIT(ipl & 7);
      sun3->tme_sun3_int_signals[ipl_index]
	= ((sun3->tme_sun3_int_signals[ipl_index]
	    & ~ipl_mask)
	   | (signal_asserted
	      ? ipl_mask
	      : 0));

      /* possibly update the ipl being driven to the CPU: */
      return (_tme_sun3_ipl_check(sun3));
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
_tme_sun3_bus_intack(struct tme_bus_connection *conn_m68k, unsigned int ipl, int *vector)
{
  struct tme_sun3 *sun3;
  tme_uint8_t interreg;
  unsigned int signal;
  int rc;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn_m68k->tme_bus_connection.tme_connection_element->tme_element_private;

  /* acknowledge any soft interrupt: */
  interreg = sun3->tme_sun3_ints;
  if ((ipl == 3
       && (interreg & TME_SUN3_IREG_SOFT_INT_3))
      || (ipl == 2
	  && (interreg & TME_SUN3_IREG_SOFT_INT_2))
      || (ipl == 1
	  && (interreg & TME_SUN3_IREG_SOFT_INT_1))) {
    *vector = TME_BUS_INTERRUPT_VECTOR_UNDEF;
    return (TME_OK);
  }

  /* turn the ipl into a bus signal number: */
  signal = TME_BUS_SIGNAL_INT(ipl);

  /* try the acknowledge on these buses, in order: */

  /* obio: */
  rc = (*sun3->tme_sun3_obio->tme_bus_intack)
    (sun3->tme_sun3_obio, signal, vector);
  if (rc == ENOENT
      && signal == sun3->tme_sun3_int_signal_clock_last) {
    rc = (*sun3->tme_sun3_obio->tme_bus_intack)
      (sun3->tme_sun3_obio, TME_BUS_SIGNAL_INT_CLOCK, vector);
  }
  if (rc != ENOENT) {
    return (rc);
  }
  /* obmem: */
  rc = (*sun3->tme_sun3_obmem->tme_bus_intack)
    (sun3->tme_sun3_obmem, signal, vector);
  if (rc != ENOENT) {
    return (rc);
  }
  /* VMEbus: */
  rc = (*sun3->tme_sun3_vmebus->tme_bus_intack)
    (sun3->tme_sun3_vmebus, signal, vector);
  if (rc != ENOENT) {
    return (rc);
  }

  /* done: */
  return (rc);
}

/* our command function: */
static int
_tme_sun3_command(struct tme_element *element, const char * const * args, char **_output)
{
  struct tme_sun3 *sun3;
  int do_reset;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) element->tme_element_private;

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
  else if (TME_ARG_IS(args[1], "diag-switch")) {

    if (args[2] == NULL) {
      tme_output_append_error(_output,
			      "diag-switch %s",
			      (sun3->tme_sun3_enable & TME_SUN3_ENA_DIAG
			       ? "true"
			       : "false"));
    }

    else if (TME_ARG_IS(args[2], "true")
	     && args[3] == NULL) {
      sun3->tme_sun3_enable |= TME_SUN3_ENA_DIAG;
    }

    else if (TME_ARG_IS(args[2], "false")
	     && args[3] == NULL) {
      sun3->tme_sun3_enable &= ~TME_SUN3_ENA_DIAG;
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
			    _("available %s commands: %s"),
			    args[0],
			    "power");
    return (EINVAL);
  }

  if (do_reset) {

    /* reset the CPU: */
    (*sun3->tme_sun3_m68k->tme_m68k_bus_connection.tme_bus_signal)
      (&sun3->tme_sun3_m68k->tme_m68k_bus_connection,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);

    /* reset all busses: */
    (*sun3->tme_sun3_obio->tme_bus_signal)
      (sun3->tme_sun3_obio,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
    (*sun3->tme_sun3_obmem->tme_bus_signal)
      (sun3->tme_sun3_obmem,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
    (*sun3->tme_sun3_vmebus->tme_bus_signal)
      (sun3->tme_sun3_vmebus,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
  }

  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_sun3_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun3_bus_connection *conn_sun3;
  struct tme_bus_connection *conn_bus;
  struct tme_sun3 *sun3;
  unsigned int score;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn->tme_connection_element->tme_element_private;

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_m68k = (struct tme_m68k_bus_connection *) conn->tme_connection_other;
  conn_sun3 = (struct tme_sun3_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {

    /* this must be an m68k chip, and not another bus: */
  case TME_CONNECTION_BUS_M68K:
    if (conn_bus->tme_bus_tlb_set_add == NULL
	&& conn_m68k->tme_m68k_bus_tlb_fill == NULL
	&& conn_m68k->tme_m68k_bus_m6888x_enable != NULL) {
      score = 10;
    }
    break;

    /* if this connection is not for an obio master, this must be a bus,
       and not a chip, and vice versa.  if this connection is for a bus,
       the bus must still be free: */
  case TME_CONNECTION_BUS_GENERIC:
    if (((conn_sun3->tme_sun3_bus_connection_which != TME_SUN3_CONN_OBIO_MASTER)
	 == (conn_bus->tme_bus_tlb_set_add != NULL
	     && conn_bus->tme_bus_tlb_fill != NULL))
	&& (conn_sun3->tme_sun3_bus_connection_which >= TME_SUN3_CONN_BUS_COUNT
	    || sun3->tme_sun3_buses[conn_sun3->tme_sun3_bus_connection_which] == NULL)) {
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
_tme_sun3_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun3 *sun3;
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun3_bus_connection *conn_sun3;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn_other;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) conn->tme_connection_element->tme_element_private;

  /* dispatch on the connection type: */
  conn_other = conn->tme_connection_other;
  conn_m68k = (struct tme_m68k_bus_connection *) conn_other;
  conn_sun3 = (struct tme_sun3_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn_other;
  switch (conn->tme_connection_type) {
      
  case TME_CONNECTION_BUS_M68K:
    sun3->tme_sun3_m68k = conn_m68k;
    break;
      
  case TME_CONNECTION_BUS_GENERIC:

    /* if this connection is for a bus: */
    if (conn_sun3->tme_sun3_bus_connection_which < TME_SUN3_CONN_BUS_COUNT) {

      /* remember the connection to this bus: */
      sun3->tme_sun3_buses[conn_sun3->tme_sun3_bus_connection_which] = conn_bus;
    }

    /* otherwise, if this connection is for the memory error register: */
    else if (conn_sun3->tme_sun3_bus_connection_which == TME_SUN3_CONN_REG_MEMERR) {

      /* remember the memory error register's bus connection: */
      sun3->tme_sun3_memerr_bus = conn_bus;
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
_tme_sun3_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes new connection sides: */
static int
_tme_sun3_connections_new(struct tme_element *element, const char * const *args, struct tme_connection **_conns, char **_output)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun3_bus_connection *conn_sun3;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  struct tme_sun3 *sun3;
  char *free_buses;
  int which_conn;

  /* recover our sun3: */
  sun3 = (struct tme_sun3 *) element->tme_element_private;

  /* if we have no arguments, and we don't have a CPU yet, we can take
     an m68k bus connection: */
  if (args[1] == NULL
      && sun3->tme_sun3_m68k == NULL) {

    /* create our side of an m68k bus connection: */
    conn_m68k = tme_new0(struct tme_m68k_bus_connection, 1);
    conn_bus = &conn_m68k->tme_m68k_bus_connection;
    conn = &conn_bus->tme_bus_connection;
    conn->tme_connection_next = *_conns;

    /* fill in the generic connection: */
    conn->tme_connection_type = TME_CONNECTION_BUS_M68K;
    conn->tme_connection_score = _tme_sun3_connection_score;
    conn->tme_connection_make = _tme_sun3_connection_make;
    conn->tme_connection_break = _tme_sun3_connection_break;

    /* fill in the generic bus connection: */
    conn_bus->tme_bus_signal = _tme_sun3_bus_signal;
    conn_bus->tme_bus_intack = _tme_sun3_bus_intack;
    conn_bus->tme_bus_tlb_set_add = _tme_sun3_mmu_tlb_set_add;

    /* full in the m68k bus connection: */
    conn_m68k->tme_m68k_bus_tlb_fill = _tme_sun3_m68k_tlb_fill;

    /* add in this connection side possibility: */
    *_conns = conn;
  }

  /* create our side of a generic bus connection: */
  conn_sun3 = tme_new0(struct tme_sun3_bus_connection, 1);
  conn_bus = &conn_sun3->tme_sun3_bus_connection;
  conn = &conn_bus->tme_bus_connection;
  conn->tme_connection_next = *_conns;

  /* fill in the generic connection: */
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_sun3_connection_score;
  conn->tme_connection_make = _tme_sun3_connection_make;
  conn->tme_connection_break = _tme_sun3_connection_break;
    
  /* fill in the generic bus connection: */
  conn_bus->tme_bus_signal = _tme_sun3_bus_signal;
  conn_bus->tme_bus_intack = NULL;
  conn_bus->tme_bus_tlb_set_add = _tme_sun3_mmu_tlb_set_add;
  conn_bus->tme_bus_tlb_fill = _tme_sun3_bus_tlb_fill;

  /* if we have no argument: */
  if (args[1] == NULL) {

    /* make this connection for an obio master: */
    conn_sun3->tme_sun3_bus_connection_which = TME_SUN3_CONN_OBIO_MASTER;
  }

  /* otherwise, we have at least one argument: */
  else {

    /* we must have no other arguments: */
    if (args[2] != NULL) {
      tme_output_append_error(_output,
			      "%s %s",
			      args[2],
			      _("unexpected"));
      tme_free(conn_sun3);
      return (EINVAL);
    }

    /* start the list of buses that we don't have yet: */
    free_buses = NULL;

    /* poison the which connection: */
    which_conn = -1;

    /* check each bus: */

    /* obio: */
    if (sun3->tme_sun3_obio == NULL) {
      tme_output_append(&free_buses, " obio");
    }
    if (TME_ARG_IS(args[1], "obio")) {
      which_conn = TME_SUN3_CONN_BUS_OBIO;
    }

    /* obmem: */
    if (sun3->tme_sun3_obmem == NULL) {
      tme_output_append(&free_buses, " obmem");
    }
    if (TME_ARG_IS(args[1], "obmem")) {
      which_conn = TME_SUN3_CONN_BUS_OBMEM;
    }

    /* VMEbus: */
    if (sun3->tme_sun3_vmebus == NULL) {
      tme_output_append(&free_buses, " vme");
    }
    if (TME_ARG_IS(args[1], "vme")) {
      which_conn = TME_SUN3_CONN_BUS_VME;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= TME_SUN3_DVMA_SIZE_VME;
    }

    /* random connections: */
    
    if (TME_ARG_IS(args[1], "memerr")) {
      which_conn = TME_SUN3_CONN_REG_MEMERR;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= TME_SUN3_MEMERR_SIZ_REG;
    }
    else if (TME_ARG_IS(args[1], "intreg")) {
      which_conn = TME_SUN3_CONN_REG_INTREG;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= sizeof(sun3->tme_sun3_ints);
    }

    /* if the which connection is still poison, or if this is trying
       to connect a bus that we already have, complain: */
    if (which_conn < 0
	|| (which_conn < TME_SUN3_CONN_BUS_COUNT
	    && sun3->tme_sun3_buses[which_conn] != NULL)) {
      if (which_conn < 0) {
	tme_output_append_error(_output,
				"%s %s",
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
      tme_free(conn_sun3);
      return (EINVAL);
    }

    /* free the remaining bus list: */
    if (free_buses != NULL) {
      tme_free(free_buses);
    }

    /* fill in the sun3 connection: */
    conn_sun3->tme_sun3_bus_connection_which = which_conn;
  }

  /* add in this connection side possibility: */
  *_conns = conn;

  /* done: */
  return (TME_OK);
}

/* this creates a new sun3 element: */
TME_ELEMENT_NEW_DECL(tme_machine_sun3) {
  int usage;
  struct tme_sun3 *sun3;
  const char *idprom_filename;
  FILE *idprom_fp;
  tme_uint8_t idprom[TME_SUN_IDPROM_SIZE];
  int arg_i;

  arg_i = 1;
  usage = FALSE;

  /* our first argument is the filename to load our IDPROM contents from: */
  idprom_filename = args[arg_i++];
  if (idprom_filename == NULL) {
    usage = TRUE;
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
			    "%s %s IDPROM%s",
			    _("usage:"),
			    args[0],
			    _("-FILENAME"));
    return (EINVAL);
  }

  /* try to read in the IDPROM: */
  idprom_fp = fopen(idprom_filename, "r");
  if (idprom_fp == NULL) {
    tme_output_append_error(_output, "%s", idprom_filename);
    return (errno);
  }
  if (fread(idprom, sizeof(tme_uint8_t), sizeof(idprom), idprom_fp) != sizeof(idprom)) {
    tme_output_append_error(_output, "%s", idprom_filename);
    fclose(idprom_fp);
    return (ENOEXEC);
  }
  fclose(idprom_fp);

  /* allocate and initialize the new sun3: */
  sun3 = tme_new0(struct tme_sun3, 1);
  sun3->tme_sun3_element = element;

  /* set the IDPROM: */
  memcpy(sun3->tme_sun3_idprom_contents, idprom, sizeof(idprom));
  
  /* the context register: */
  sun3->tme_sun3_context = 0;

  /* the diagnostics register: */
  sun3->tme_sun3_diag = 0;

  /* the bus error register: */
  sun3->tme_sun3_buserr = 0;

  /* the enable register: */
  sun3->tme_sun3_enable = 0;

  /* the memory error register: */
  sun3->tme_sun3_memerr_csr = 0;
  sun3->tme_sun3_memerr_vaddr = 0;

  /* the interrupt register: */
  sun3->tme_sun3_ints = 0;

  /* the MMU: */
  _tme_sun3_mmu_new(sun3);

  /* the busses: */
  sun3->tme_sun3_obio = NULL;
  sun3->tme_sun3_obmem = NULL;
  sun3->tme_sun3_vmebus = NULL;

  /* the last clock interrupt signal: */
  sun3->tme_sun3_int_signal_clock_last = TME_BUS_SIGNAL_INT_UNSPEC;

  /* fill the element: */
  element->tme_element_private = sun3;
  element->tme_element_connections_new = _tme_sun3_connections_new;
  element->tme_element_command = _tme_sun3_command;

  return (TME_OK);
}

/* this creates a new Sun-3 isil7170: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,clock) {
  struct tme_isil7170_socket socket;
  const char *sub_args[4];
  int arg_i;

  /* create the isil7170 socket: */
  memset (&socket, 0, sizeof(socket));
  socket.tme_isil7170_socket_version = TME_ISIL7170_SOCKET_0;
  socket.tme_isil7170_socket_addr_shift = 0;
  socket.tme_isil7170_socket_port_least_lane = 3; /* D31-D24 */
  socket.tme_isil7170_socket_clock_basic = TME_ISIL7170_FREQ_32K;
  socket.tme_isil7170_socket_int_signal = TME_BUS_SIGNAL_INT_CLOCK;

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

/* this creates a new Sun-3 z8530: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,zs) {
  struct tme_z8530_socket socket = TME_SUN_Z8530_SOCKET_INIT;
  char *sub_args[2];

  /* create the z8530: */
  sub_args[0] = "tme/ic/z8530";
  sub_args[1] = NULL;
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}

/* this creates a new Sun-3 obie: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,obie) {
  return (tme_sun_obie(element, args, _output));
}

/* this creates a new Sun-3 bwtwo: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,bwtwo) {
  return (tme_sun_bwtwo(element, args, _output));
}

/* this creates a new Sun-3 si: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,si) {
  return (tme_sun_si(element, args, _output));
}

/* this creates a new Sun-3 cgtwo: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun3,cgtwo) {
  return (tme_sun_cgtwo(element, args, _output));
}
