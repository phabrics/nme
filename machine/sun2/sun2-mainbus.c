/* $Id: sun2-mainbus.c,v 1.18 2009/08/30 14:32:29 fredette Exp $ */

/* machine/sun2/sun2-mainbus.c - implementation of Sun 2 emulation: */

/*
 * Copyright (c) 2003 Matt Fredette
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
_TME_RCSID("$Id: sun2-mainbus.c,v 1.18 2009/08/30 14:32:29 fredette Exp $");

/* includes: */
#include "sun2-impl.h"
#include <tme/ic/am9513.h>
#include <tme/ic/z8530.h>
#include <tme/ic/mm58167.h>
#include <stdio.h>

/* this possibly updates that the interrupt priority level driven to the CPU: */
int
_tme_sun2_ipl_check(struct tme_sun2 *sun2)
{
  tme_uint16_t ena;
  unsigned int ipl, ipl_index;
  tme_uint8_t ipl_mask;

  /* get the enable register: */
  ena = sun2->tme_sun2_enable;

  /* assume that interrupts are completely masked: */
  ipl = TME_M68K_IPL_NONE;

  /* if interrupts are enabled: */
  if (ena & TME_SUN2_ENA_INTS) {

    /* find the highest ipl now asserted on the buses: */
    for (ipl = TME_M68K_IPL_MAX;
	 ipl > TME_M68K_IPL_NONE;
	 ipl--) {
      ipl_index = ipl >> 3;
      ipl_mask = TME_BIT(ipl & 7);
      if (sun2->tme_sun2_int_signals[ipl_index] & ipl_mask) {
	break;
      }
    }
    
    /* check the soft interrupts: */
    if (ena & TME_SUN2_ENA_SOFT_INT_3) {
      ipl = TME_MAX(ipl, 3);
    }
    else if (ena & TME_SUN2_ENA_SOFT_INT_2) {
      ipl = TME_MAX(ipl, 2);
    }
    else if (ena & TME_SUN2_ENA_SOFT_INT_1) {
      ipl = TME_MAX(ipl, 1);
    }
  }
  
  /* possibly update the CPU: */
  if (ipl != sun2->tme_sun2_int_ipl_last) {
    sun2->tme_sun2_int_ipl_last = ipl;
    return ((*sun2->tme_sun2_m68k->tme_m68k_bus_interrupt)
	    (sun2->tme_sun2_m68k, ipl));
  }
  return (TME_OK);
}

/* our mainbus signal handler: */
static int
_tme_sun2_bus_signal(struct tme_bus_connection *conn_bus_raiser, unsigned int signal)
{
  struct tme_sun2 *sun2;
  int signal_asserted;
  unsigned int ipl, ipl_index;
  tme_uint8_t ipl_mask;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn_bus_raiser->tme_bus_connection.tme_connection_element->tme_element_private;

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
    if (ipl >= TME_M68K_IPL_MIN
	&& ipl <= TME_M68K_IPL_MAX) {
      
      /* update this ipl in the byte array: */
      ipl_index = ipl >> 3;
      ipl_mask = TME_BIT(ipl & 7);
      sun2->tme_sun2_int_signals[ipl_index]
	= ((sun2->tme_sun2_int_signals[ipl_index]
	    & ~ipl_mask)
	   | (signal_asserted
	      ? ipl_mask
	      : 0));

      /* possibly update the ipl being driven to the CPU: */
      return (_tme_sun2_ipl_check(sun2));
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
_tme_sun2_bus_intack(struct tme_bus_connection *conn_m68k, unsigned int ipl, int *vector)
{
  struct tme_sun2 *sun2;
  tme_uint16_t ena;
  int signal;
  int rc;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn_m68k->tme_bus_connection.tme_connection_element->tme_element_private;

  /* acknowledge any soft interrupt: */
  ena = sun2->tme_sun2_enable;
  if ((ipl == 3
       && (ena & TME_SUN2_ENA_SOFT_INT_3))
      || (ipl == 2
	  && (ena & TME_SUN2_ENA_SOFT_INT_2))
      || (ipl == 1
	  && (ena & TME_SUN2_ENA_SOFT_INT_1))) {
    *vector = TME_BUS_INTERRUPT_VECTOR_UNDEF;
    return (TME_OK);
  }

  /* turn the ipl into a bus signal number: */
  signal = TME_BUS_SIGNAL_INT(ipl);

  /* try the acknowledge on these buses, in order: */

  /* obio: */
  rc = (*sun2->tme_sun2_obio->tme_bus_intack)
    (sun2->tme_sun2_obio, signal, vector);
  if (rc != ENOENT) {
    return (rc);
  }
  /* obmem: */
  rc = (*sun2->tme_sun2_obmem->tme_bus_intack)
    (sun2->tme_sun2_obmem, signal, vector);
  if (rc != ENOENT) {
    return (rc);
  }
  if (sun2->tme_sun2_has_vme) {
    /* VME: */
    rc = (*sun2->tme_sun2_vmebus->tme_bus_intack)
      (sun2->tme_sun2_vmebus, signal, vector);
    if (rc != ENOENT) {
      return (rc);
    }
  }
  else {
    /* mbio: */
    rc = (*sun2->tme_sun2_mbio->tme_bus_intack)
      (sun2->tme_sun2_mbio, signal, vector);
    if (rc != ENOENT) {
      return (rc);
    }
    /* mbmem: */
    rc = (*sun2->tme_sun2_mbmem->tme_bus_intack)
      (sun2->tme_sun2_mbmem, signal, vector);
    if (rc != ENOENT) {
      return (rc);
    }
  }

  /* done: */
  return (rc);
}

/* our command function: */
static int
_tme_sun2_command(struct tme_element *element, const char * const * args, char **_output)
{
  struct tme_sun2 *sun2;
  int do_reset;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) element->tme_element_private;

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
    (*sun2->tme_sun2_m68k->tme_m68k_bus_connection.tme_bus_signal)
      (&sun2->tme_sun2_m68k->tme_m68k_bus_connection,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);

    /* reset all busses: */
    (*sun2->tme_sun2_obio->tme_bus_signal)
      (sun2->tme_sun2_obio,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
    (*sun2->tme_sun2_obmem->tme_bus_signal)
      (sun2->tme_sun2_obmem,
       TME_BUS_SIGNAL_RESET
       | TME_BUS_SIGNAL_LEVEL_NEGATED
       | TME_BUS_SIGNAL_EDGE);
    if (sun2->tme_sun2_has_vme) {
      (*sun2->tme_sun2_vmebus->tme_bus_signal)
	(sun2->tme_sun2_obmem,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
    }
    else {
      (*sun2->tme_sun2_mbio->tme_bus_signal)
	(sun2->tme_sun2_mbio,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
      (*sun2->tme_sun2_mbmem->tme_bus_signal)
	(sun2->tme_sun2_mbmem,
	 TME_BUS_SIGNAL_RESET
	 | TME_BUS_SIGNAL_LEVEL_NEGATED
	 | TME_BUS_SIGNAL_EDGE);
    }
  }

  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_sun2_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun2_bus_connection *conn_sun2;
  struct tme_bus_connection *conn_bus;
  struct tme_sun2 *sun2;
  unsigned int score;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn->tme_connection_element->tme_element_private;

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_m68k = (struct tme_m68k_bus_connection *) conn->tme_connection_other;
  conn_sun2 = (struct tme_sun2_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {

    /* this must be an m68k chip, and not another bus: */
  case TME_CONNECTION_BUS_M68K:
    if (conn_bus->tme_bus_tlb_set_add == NULL
	&& conn_m68k->tme_m68k_bus_tlb_fill == NULL) {
      score = 10;
    }
    break;

    /* this must be a bus, and not a chip: */
  case TME_CONNECTION_BUS_GENERIC:
    if (conn_bus->tme_bus_tlb_set_add != NULL
	&& conn_bus->tme_bus_tlb_fill != NULL
	&& sun2->tme_sun2_buses[conn_sun2->tme_sun2_bus_connection_which] == NULL) {
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
_tme_sun2_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sun2 *sun2;
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun2_bus_connection *conn_sun2;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn_other;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) conn->tme_connection_element->tme_element_private;

  /* dispatch on the connection type: */
  conn_other = conn->tme_connection_other;
  conn_m68k = (struct tme_m68k_bus_connection *) conn_other;
  conn_sun2 = (struct tme_sun2_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn_other;
  switch (conn->tme_connection_type) {
      
  case TME_CONNECTION_BUS_M68K:
    sun2->tme_sun2_m68k = conn_m68k;
    break;
      
  case TME_CONNECTION_BUS_GENERIC:
    sun2->tme_sun2_buses[conn_sun2->tme_sun2_bus_connection_which] = conn_bus;
    break;

  default: assert(FALSE);
  }
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_sun2_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes new connection sides: */
static int
_tme_sun2_connections_new(struct tme_element *element, const char * const *args, struct tme_connection **_conns, char **_output)
{
  struct tme_m68k_bus_connection *conn_m68k;
  struct tme_sun2_bus_connection *conn_sun2;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  struct tme_sun2 *sun2;
  char *free_buses;
  int which_bus;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) element->tme_element_private;

  /* if we have no arguments and don't have a CPU yet, we can take an m68k connection: */
  if (args[1] == NULL
      && sun2->tme_sun2_m68k == NULL) {

    /* create our side of an m68k bus connection: */
    conn_m68k = tme_new0(struct tme_m68k_bus_connection, 1);
    conn_bus = &conn_m68k->tme_m68k_bus_connection;
    conn = &conn_bus->tme_bus_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_BUS_M68K;
    conn->tme_connection_score = _tme_sun2_connection_score;
    conn->tme_connection_make = _tme_sun2_connection_make;
    conn->tme_connection_break = _tme_sun2_connection_break;

    /* fill in the generic bus connection: */
    conn_bus->tme_bus_signal = _tme_sun2_bus_signal;
    conn_bus->tme_bus_intack = _tme_sun2_bus_intack;
    conn_bus->tme_bus_tlb_set_add = _tme_sun2_mmu_tlb_set_add;

    /* full in the m68k bus connection: */
    conn_m68k->tme_m68k_bus_tlb_fill = _tme_sun2_m68k_tlb_fill;

    /* add in this connection side possibility: */
    *_conns = conn;
  }

  /* otherwise, we must have an argument and it must be for a bus that
     we don't have yet: */
  else {

    free_buses = NULL;
    which_bus = -1;

    if (sun2->tme_sun2_obio == NULL) {
      tme_output_append(&free_buses, " obio");
    }
    if (TME_ARG_IS(args[1], "obio")) {
      which_bus = TME_SUN2_BUS_OBIO;
    }

    if (sun2->tme_sun2_obmem == NULL) {
      tme_output_append(&free_buses, " obmem");
    }
    if (TME_ARG_IS(args[1], "obmem")) {
      which_bus = TME_SUN2_BUS_OBMEM;
    }

    if (sun2->tme_sun2_has_vme) {

      if (sun2->tme_sun2_vmebus == NULL) {
	tme_output_append(&free_buses, " vme");
      }
      if (TME_ARG_IS(args[1], "vme")) {
	which_bus = TME_SUN2_BUS_VME;
      }

    }

    else {

      if (sun2->tme_sun2_mbio == NULL) {
	tme_output_append(&free_buses, " mbio");
      }
      if (TME_ARG_IS(args[1], "mbio")) {
	which_bus = TME_SUN2_BUS_MBIO;
      }
      
      if (sun2->tme_sun2_mbmem == NULL) {
	tme_output_append(&free_buses, " mbmem");
      }
      if (TME_ARG_IS(args[1], "mbmem")) {
	which_bus = TME_SUN2_BUS_MBMEM;
      }
    }

    if (args[1] == NULL
	|| which_bus < 0
	|| sun2->tme_sun2_buses[which_bus] != NULL) {
      if (free_buses != NULL) {
	tme_output_append_error(_output, 
				"%s%s",
				_("remaining buses:"),
				free_buses);
	tme_free(free_buses);
      }
      else {
	tme_output_append_error(_output, _("all buses present"));
      }
      return (EINVAL);
    }
    if (free_buses != NULL) {
      tme_free(free_buses);
    }

    if (args[2] != NULL) {
      tme_output_append_error(_output,
			      "%s %s",
			      args[2],
			      _("unexpected"));
      return (EINVAL);
    }

    /* create our side of a generic bus connection: */
    conn_sun2 = tme_new0(struct tme_sun2_bus_connection, 1);
    conn_bus = &conn_sun2->tme_sun2_bus_connection;
    conn = &conn_bus->tme_bus_connection;
    conn->tme_connection_next = *_conns;

    /* fill in the generic connection: */
    conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
    conn->tme_connection_score = _tme_sun2_connection_score;
    conn->tme_connection_make = _tme_sun2_connection_make;
    conn->tme_connection_break = _tme_sun2_connection_break;
    
    /* fill in the generic bus connection: */
    if (which_bus == TME_SUN2_BUS_MBMEM) {
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= TME_SUN2_DVMA_SIZE_MBMEM;
    }
    else if (which_bus == TME_SUN2_BUS_VME) {
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last
	= TME_SUN2_DVMA_SIZE_VME;
    }
    conn_bus->tme_bus_signal = _tme_sun2_bus_signal;
    conn_bus->tme_bus_intack = NULL;
    conn_bus->tme_bus_tlb_set_add = _tme_sun2_mmu_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_sun2_bus_tlb_fill;

    /* fill in the sun2 connection: */
    conn_sun2->tme_sun2_bus_connection_which = which_bus;

    /* add in this connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* this creates a new sun2 element: */
TME_ELEMENT_NEW_DECL(tme_machine_sun2) {
  int usage;
  struct tme_sun2 *sun2;
  int sun2_has_vme;
  const char *idprom_filename;
  FILE *idprom_fp;
  tme_uint8_t idprom[TME_SUN_IDPROM_SIZE];
  int arg_i;

  arg_i = 1;
  usage = FALSE;

  /* our first argument must be either "multibus" or "vme": */
  sun2_has_vme = FALSE;
  if (TME_ARG_IS(args[arg_i], "multibus")) {
    arg_i++;
  }
  else if (TME_ARG_IS(args[arg_i], "vme")) {
    arg_i++;
    sun2_has_vme = TRUE;
  }
  else {
    usage = TRUE;
  }

  /* our second argument is the filename to load our IDPROM contents from: */
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
			    "%s %s [ multibus | vme ] IDPROM%s",
			    _("usage:"),
			    args[0],
			    _("-FILENAME"));
    return (EINVAL);
  }

  /* try to read in the IDPROM: */
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

  /* allocate and initialize the new sun2: */
  sun2 = tme_new0(struct tme_sun2, 1);
  sun2->tme_sun2_element = element;

  /* set the VME flag: */
  sun2->tme_sun2_has_vme = sun2_has_vme;

  /* set the IDPROM: */
  memcpy(sun2->tme_sun2_idprom_contents, idprom, sizeof(idprom));
  
  /* the system and user context registers: */
  sun2->tme_sun2_context_system = 0;
  sun2->tme_sun2_context_user = 0;

  /* the diagnostics register: */
  sun2->tme_sun2_diag = 0;

  /* the bus error register: */
  sun2->tme_sun2_buserr = 0;

  /* the enable register: */
  sun2->tme_sun2_enable = 0;

  /* the MMU: */
  _tme_sun2_mmu_new(sun2);

  /* the busses: */
  sun2->tme_sun2_obio = NULL;
  sun2->tme_sun2_obmem = NULL;
  sun2->tme_sun2_mbio = NULL;
  sun2->tme_sun2_mbmem = NULL;
  sun2->tme_sun2_vmebus = NULL;

  /* we don't have the CPU's bus context register yet: */
  sun2->tme_sun2_m68k_bus_context = NULL;

  /* fill the element: */
  element->tme_element_private = sun2;
  element->tme_element_connections_new = _tme_sun2_connections_new;
  element->tme_element_command = _tme_sun2_command;

  return (TME_OK);
}

/* this creates a new Sun-2 am9513: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun2,clock) {
  struct tme_am9513_socket socket;
  char *sub_args[2];

  /* create the am9513 socket: */
  socket.tme_am9513_socket_version = TME_AM9513_SOCKET_0;
  socket.tme_am9513_socket_address_cmd = 2;
  socket.tme_am9513_socket_address_data = 0;
  socket.tme_am9513_socket_port_least_lane = 0; /* D7-D0 */
  socket.tme_am9513_socket_basic_clock = (19660800 / 4);

  /* Timer 1 is connected to the bus as ipl 7 (inverted): */
  socket.tme_am9513_socket_counter_signals[0] = 
    TME_BUS_SIGNAL_INT(7) | (TME_BUS_SIGNAL_LEVEL_ASSERTED ^ TME_BUS_SIGNAL_LEVEL_HIGH);

  /* Timer 2 is connected to the bus as ipl 5 (inverted): */
  socket.tme_am9513_socket_counter_signals[1] = 
    TME_BUS_SIGNAL_INT(5) | (TME_BUS_SIGNAL_LEVEL_ASSERTED ^ TME_BUS_SIGNAL_LEVEL_HIGH);

  /* Timer 3 is connected to the bus as ???: */
  socket.tme_am9513_socket_counter_signals[2] = TME_BUS_SIGNAL_ABORT;

  /* Timer 4 is connected to the bus as ???: */
  socket.tme_am9513_socket_counter_signals[3] = TME_BUS_SIGNAL_ABORT;

  /* Timer 5 is connected to the bus as ???: */
  socket.tme_am9513_socket_counter_signals[4] = TME_BUS_SIGNAL_ABORT;

  /* create the am9513: */
  sub_args[0] = "tme/ic/am9513";
  sub_args[1] = NULL;
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}

/* this creates a new Sun-2 mm58167: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun2,tod) {
  struct tme_mm58167_socket socket;
  char *sub_args[2];

  /* create the mm58167 socket: */
  socket.tme_mm58167_socket_version = TME_MM58167_SOCKET_0;
  socket.tme_mm58167_socket_addr_shift = 1;
  socket.tme_mm58167_socket_port_least_lane = 1; /* D15-D8 */

  /* create the mm58167: */
  sub_args[0] = "tme/ic/mm58167";
  sub_args[1] = NULL;
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}

/* this creates a new Sun-2 z8530: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun2,zs) {
  struct tme_z8530_socket socket = TME_SUN_Z8530_SOCKET_INIT;
  char *sub_args[2];

  /* override the least lane in the z8530 socket: */
  socket.tme_z8530_socket_port_least_lane = 1; /* D15-D8 */

  /* override the socket flags: */
  socket.tme_z8530_socket_flags |= TME_Z8530_SOCKET_FLAG_IEI_TIED_LOW;

  /* create the z8530: */
  sub_args[0] = "tme/ic/z8530";
  sub_args[1] = NULL;
  return (tme_element_new(element, (const char * const *) sub_args, &socket, _output));
}

/* this creates a new Sun-2 bwtwo: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun2,bwtwo) {
  return (tme_sun_bwtwo(element, args, _output));
}
