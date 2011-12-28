/* $Id: am7930.c,v 1.2 2007/08/24 00:58:08 fredette Exp $ */

/* ic/am7930.c - implementation of Am7930 emulation: */

/*
 * Copyright (c) 2006 Matt Fredette
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
_TME_RCSID("$Id: am7930.c,v 1.2 2007/08/24 00:58:08 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>

/* macros: */

/* most registers are not read-write: */
#define TME_AM7930_REG_RO(x)		(x)
#define TME_AM7930_REG_WO(x)		((x) + TME_AM7930_SIZ_REGS)
#define TME_AM7930_REG_RW(x)		TME_AM7930_REG_RO(x)
#define TME_AM7930_REG_INDEX(x)		((x) % TME_AM7930_SIZ_REGS)

/* registers: */
#define TME_AM7930_REG_CMD		TME_AM7930_REG_WO(0)
#define TME_AM7930_REG_INT		TME_AM7930_REG_RO(0)
#define TME_AM7930_REG_DATA		TME_AM7930_REG_RW(1)
#define TME_AM7930_REG_DSR1		TME_AM7930_REG_RO(2)
#define TME_AM7930_REG_DER		TME_AM7930_REG_RO(3)
#define TME_AM7930_REG_DCTB		TME_AM7930_REG_WO(4)
#define TME_AM7930_REG_DCRB		TME_AM7930_REG_RO(4)
#define TME_AM7930_REG_BBTB		TME_AM7930_REG_WO(5)
#define TME_AM7930_REG_BBRB		TME_AM7930_REG_RO(5)
#define TME_AM7930_REG_BCTB		TME_AM7930_REG_WO(6)
#define TME_AM7930_REG_BCRB		TME_AM7930_REG_RO(6)
#define TME_AM7930_REG_DSR2		TME_AM7930_REG_RO(7)
#define TME_AM7930_SIZ_REGS		(8)

/* the read/write registers: */
#define TME_AM7930_REGS_RW					\
  (TME_BIT(TME_AM7930_REG_INDEX(TME_AM7930_REG_DATA)))

/* the callout flags: */
#define TME_AM7930_CALLOUTS_RUNNING		TME_BIT(0)
#define TME_AM7930_CALLOUTS_MASK		(-2)

/* structures: */

/* the chip: */
struct tme_am7930 {

  /* our simple bus device header: */
  struct tme_bus_device tme_am7930_device;
#define tme_am7930_element tme_am7930_device.tme_bus_device_element

  /* the mutex protecting the chip: */
  tme_mutex_t tme_am7930_mutex;

  /* the callout flags: */
  int tme_am7930_callout_flags;

  /* this is nonzero if the interrupt is asserted: */
  int tme_am7930_int_asserted;

  /* it's easiest to just model the registers as a chunk of memory.
     we have twice as much memory as address space, because many
     addresses read one register, and write another: */
  tme_uint8_t tme_am7930_regs[TME_AM7930_SIZ_REGS * 2];
};

/* this resets the am7930: */
static void
_tme_am7930_reset(struct tme_am7930 *am7930)
{

  tme_log(&am7930->tme_am7930_element->tme_element_log_handle,
	  100, TME_OK,
	  (&am7930->tme_am7930_element->tme_element_log_handle,
	   "reset"));

  /* clear all pending callouts: */
  am7930->tme_am7930_callout_flags &= TME_AM7930_CALLOUTS_MASK;
}

/* the am7930 callout function.  it must be called with the mutex locked: */
static void
_tme_am7930_callout(struct tme_am7930 *am7930)
{
  struct tme_bus_connection *conn_bus;
  int again;
  int rc;
  int int_asserted;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (am7930->tme_am7930_callout_flags & TME_AM7930_CALLOUTS_RUNNING) {
    return;
  }

  /* callouts are now running: */
  am7930->tme_am7930_callout_flags |= TME_AM7930_CALLOUTS_RUNNING;

  /* loop while we have work to do: */
  do {
    again = FALSE;

    /* if we need to call out an interrupt: */
    int_asserted = FALSE;
    if (!!am7930->tme_am7930_int_asserted != int_asserted) {
      again = TRUE;

      /* note the new state of the interrupt signal: */
      am7930->tme_am7930_int_asserted = int_asserted;

      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						am7930->tme_am7930_device.tme_bus_device_connection,
						&am7930->tme_am7930_device.tme_bus_device_connection_rwlock);

      /* unlock our mutex: */
      tme_mutex_unlock(&am7930->tme_am7930_mutex);
      
      /* call out the bus interrupt signal edge: */
      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 TME_BUS_SIGNAL_INT_UNSPEC
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));
      assert (rc == TME_OK);
      
      /* lock our mutex: */
      tme_mutex_lock(&am7930->tme_am7930_mutex);
    }
  } while (again);

  /* clear that callouts are running: */
  am7930->tme_am7930_callout_flags &= ~TME_AM7930_CALLOUTS_RUNNING;
}

/* the am7930 bus signal handler: */
static int
_tme_am7930_signal(void *_am7930, 
		   unsigned int signal)
{
  struct tme_am7930 *am7930;
  unsigned int level;

  /* recover our data structure: */
  am7930 = (struct tme_am7930 *) _am7930;

  /* lock the mutex: */
  tme_mutex_lock(&am7930->tme_am7930_mutex);

  /* take out the signal level: */
  level = signal & TME_BUS_SIGNAL_LEVEL_MASK;
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* dispatch on the generic bus signals: */
  switch (signal) {
  case TME_BUS_SIGNAL_RESET:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      _tme_am7930_reset(am7930);
    }
    break;
  default:
    signal = TME_BUS_SIGNAL_IGNORE;
    break;
  }

  /* if we didn't ignore this bus signal: */
  if (signal != TME_BUS_SIGNAL_IGNORE) {

    /* make any new callouts: */
    _tme_am7930_callout(am7930);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&am7930->tme_am7930_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the am7930 bus cycle handler: */
static int
_tme_am7930_bus_cycle(void *_am7930, struct tme_bus_cycle *cycle_init)
{
  struct tme_am7930 *am7930;
  unsigned int reg;
  tme_uint8_t value;

  /* recover our data structure: */
  am7930 = (struct tme_am7930 *) _am7930;

  /* the address must be within range: */
  assert(cycle_init->tme_bus_cycle_address < TME_AM7930_SIZ_REGS);
  assert(cycle_init->tme_bus_cycle_size <= (TME_AM7930_SIZ_REGS - cycle_init->tme_bus_cycle_address));

  /* get the register being accessed: */
  reg = cycle_init->tme_bus_cycle_address;

  /* lock the mutex: */
  tme_mutex_lock(&am7930->tme_am7930_mutex);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* turn the address into a proper register number for writing: */
    if ((TME_AM7930_REGS_RW & TME_BIT(reg)) != 0) {
      reg = TME_AM7930_REG_RW(reg);
    }
    else {
      reg = TME_AM7930_REG_WO(reg);
    }

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS8_LOG2);

    /* dispatch on the register: */
    switch (reg) {

    default:
      break;
    }
  }
	    
  /* otherwise, this is a read: */
  else {
    assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* get the value to read: */
    value = 0;

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS8_LOG2);
  }

  /* make any new callouts: */
  _tme_am7930_callout(am7930);

  /* unlock the mutex: */
  tme_mutex_unlock(&am7930->tme_am7930_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the am7930 TLB filler: */
static int
_tme_am7930_tlb_fill(void *_am7930,
		     struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address,
		     unsigned int cycles)
{
  struct tme_am7930 *am7930;

  /* recover our data structure: */
  am7930 = (struct tme_am7930 *) _am7930;

  /* the address must be within range: */
  assert(address < TME_AM7930_SIZ_REGS);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = TME_AM7930_SIZ_REGS - 1;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = am7930;
  tlb->tme_bus_tlb_cycle = _tme_am7930_bus_cycle;

  return (TME_OK);
}

/* this makes a new connection side for a am7930: */
static int
_tme_am7930_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_am7930 *am7930;
  int rc;

  /* recover our data structure: */
  am7930 = (struct tme_am7930 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* done: */
  return (TME_OK);
}

/* the new am7930 function: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,am7930,am7930) {
  struct tme_am7930 *am7930;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  for (;;) {

    if (0) {
    }

    /* if we ran out of arguments: */
    else if (args[arg_i] == NULL) {

      break;
    }

    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s, ",
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the am7930 structure: */
  am7930 = tme_new0(struct tme_am7930, 1);
  am7930->tme_am7930_element = element;
  tme_mutex_init(&am7930->tme_am7930_mutex);

  /* initialize our simple bus device descriptor: */
  am7930->tme_am7930_device.tme_bus_device_element = element;
  am7930->tme_am7930_device.tme_bus_device_tlb_fill = _tme_am7930_tlb_fill;
  am7930->tme_am7930_device.tme_bus_device_address_last = TME_AM7930_SIZ_REGS - 1;
  am7930->tme_am7930_device.tme_bus_device_signal = _tme_am7930_signal;

  /* fill the element: */
  element->tme_element_private = am7930;
  element->tme_element_connections_new = _tme_am7930_connections_new;

  /* reset the am7930: */
  _tme_am7930_reset(am7930);

  return (TME_OK);
}
