/* $Id: ad184x.c,v 1.1 2009/06/17 22:41:30 fredette Exp $ */

/* ic/ad184x.c - Analog Devices 184x (and Crystal Semiconductors 4248
   and 423x) implementation: */

/*
 * Copyright (c) 2009 Matt Fredette
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
_TME_RCSID("$Id: ad184x.c,v 1.1 2009/06/17 22:41:30 fredette Exp $");

/* includes: */
#include <tme/completion.h>
#include <tme/generic/bus-device.h>

/* macros: */

/* the different parts: */
#define TME_AD184X_PART_AD1848			(0)
#define TME_AD184X_PART_CS4231			(10)
#define TME_AD184X_PART_CS4231A			(11)

/* direct registers: */
#define TME_AD184X_REG_IADDR			(0)
#define TME_AD184X_REG_IDATA			(1)
#define TME_AD184X_REG_STATUS			(2)
#define TME_AD184X_REG_PIO			(3)
#define TME_AD184X_SIZ				(4)

/* the IADDR register: */
#define TME_AD184X_IADDR_INIT			(1 << 7)
#define TME_AD184X_IADDR_MCE			(1 << 6)
#define TME_AD184X_IADDR_TRD			(1 << 5)
#define TME_AD184X_IADDR_IXA			(0x0f)
#define TME_CS423X_IADDR_IXA			(0x1f)

/* inputs: */
#define TME_AD184X_INPUT_MAIN			(0)
#define TME_AD184X_INPUT_AUX1			(1)
#define TME_AD184X_INPUT_AUX2			(2)

/* channels: */
#define TME_AD184X_CHANNEL_L			(0)
#define TME_AD184X_CHANNEL_R			(1)
#define TME_AD184X_CHANNEL_COUNT		(2)

/* the indirect registers: */
#define TME_AD184X_IREG_INPUT(input, channel)	(((input) * 2) + (channel))
#define TME_AD184X_IREG_OUTPUT(channel)		(6 + (channel))
#define TME_AD184X_IREG_CLOCK_DATA		(8)
#define TME_AD184X_IREG_INTERFACE		(9)
#define TME_AD184X_IREG_PINCONTROL		(10)
#define TME_AD184X_IREG_TEST_INIT		(11)
#define TME_AD184X_IREG_MISC			(12)
#define TME_AD184X_IREG_DIGITALMIX		(13)
#define TME_AD184X_IREG_BASECOUNT_UPPER		(14)
#define TME_AD184X_IREG_BASECOUNT_LOWER		(15)
#define TME_CS423X_IREG_VERSION_ID		(25)
#define TME_CS423X_SIZ_IREG			(32)

/* the MISC register: */
#define TME_AD184X_MISC_CS423X_MODE2		(1 << 6)

#define TME_AD184X_LOG_HANDLE(ad184x) (&(ad184x)->tme_ad184x_element->tme_element_log_handle)

#define TME_AD184X_BUS_TRANSITION		(1)

/* structures: */

/* the device: */
struct tme_ad184x {

  /* our simple bus device header: */
  struct tme_bus_device tme_ad184x_device;
#define tme_ad184x_element tme_ad184x_device.tme_bus_device_element

  /* the mutex: */
  tme_mutex_t tme_ad184x_mutex;

  /* the part: */
  unsigned int tme_ad184x_part;
#define TME_AD184X_IS_AD184X(ad184x)	((ad184x)->tme_ad184x_part < TME_AD184X_PART_CS4231)
#define TME_AD184X_IS_CS423X(ad184x)	((ad184x)->tme_ad184x_part >= TME_AD184X_PART_CS4231)
#define TME_AD184X_IS_CS4231A(ad184x)	((ad184x)->tme_ad184x_part == TME_AD184X_PART_CS4231A)

  /* the IADDR register: */
  tme_uint8_t tme_ad184x_iaddr;

  /* the STATUS register: */
  tme_uint8_t tme_ad184x_status;

  /* the indirect registers: */
  tme_uint8_t tme_ad184x_ireg[TME_CS423X_SIZ_IREG];
};

/* this resets: */
static void
_tme_ad184x_reset(struct tme_ad184x *ad184x)
{
}

/* this enters: */
static struct tme_ad184x *
_tme_ad184x_enter(struct tme_ad184x *ad184x)
{

  /* lock the mutex: */
  tme_mutex_lock(&ad184x->tme_ad184x_mutex);

  return (ad184x);
}
#define _tme_ad184x_enter_bus(conn_bus) \
  ((struct tme_ad184x *) _tme_ad184x_enter((struct tme_ad184x *) (conn_bus)->tme_bus_connection.tme_connection_element->tme_element_private))

/* this leaves: */
#define _tme_ad184x_leave(ad184x) tme_mutex_unlock(&(ad184x)->tme_ad184x_mutex)

/* this busies a generic bus connection: */
#define _tme_ad184x_busy_bus(ad184x, conn_index) \
  ((ad184x)->tme_ad184x_conns[conn_index])

/* this unbusies a generic bus connection: */
#define _tme_ad184x_unbusy_bus(ad184x, conn_bus) \
  do { } while (/* CONSTCOND */ 0 && (ad184x)->tme_ad184x_element && (conn_bus)->tme_bus_connection.tme_connection_element)

/* this returns the current indirect register: */
static unsigned int
_tme_ad184x_ireg(const struct tme_ad184x *ad184x)
{
  return (ad184x->tme_ad184x_iaddr
	  & ((TME_AD184X_IS_CS423X(ad184x)
	      && (ad184x->tme_ad184x_ireg[TME_AD184X_IREG_MISC]
		  & TME_AD184X_MISC_CS423X_MODE2))
	     ? TME_CS423X_IADDR_IXA
	     : TME_AD184X_IADDR_IXA));
}

/* the bus cycle handler: */
static void
_tme_ad184x_cycle(struct tme_bus_connection *master_conn_bus,
		  struct tme_bus_cycle *master_cycle,
		  tme_uint32_t *_master_fast_cycle_types,
		  struct tme_completion *master_completion)
{
  struct tme_ad184x *ad184x;
  tme_uint32_t address;
  tme_uint8_t value;
  const char *reg;
  tme_uint32_t ireg;

  /* enter: */
  ad184x = _tme_ad184x_enter_bus(master_conn_bus);

  /* this must be an eight-bit cycle: */
  if (master_cycle->tme_bus_cycle_size != sizeof(tme_uint8_t)) {
    abort();
  }

  /* get the address: */
  address = master_cycle->tme_bus_cycle_address;

  /* if this is a write: */
  if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value,
			   TME_BUS8_LOG2);

    /* dispatch on the register: */
    switch (address) {
    default: assert(FALSE);

    case TME_AD184X_REG_IADDR:
      ad184x->tme_ad184x_iaddr
	= ((ad184x->tme_ad184x_iaddr
	    & TME_AD184X_IADDR_INIT)
	   | (value
	      & ~TME_AD184X_IADDR_INIT));
      reg = "IADDR";
      break;

    case TME_AD184X_REG_IDATA:
      ireg = _tme_ad184x_ireg(ad184x);
      reg = "(IDATA)";

      /* if this is an input or output control: */
      if (ireg <= TME_AD184X_IREG_OUTPUT(TME_AD184X_CHANNEL_COUNT - 1)) {

	/* write the new control: */
	ad184x->tme_ad184x_ireg[ireg] = value;
      }

      /* otherwise, this is another register: */
      else {
	switch (ireg) {

	default: 
	  /* temporarily, just write any unknown registers: */
	  ad184x->tme_ad184x_ireg[ireg] = value;
	  break;
	}
      }
      break;

    case TME_AD184X_REG_STATUS:
      reg = "STATUS";
      break;

    case TME_AD184X_REG_PIO:
      reg = "PIO";
      break;
    }

    tme_log(TME_AD184X_LOG_HANDLE(ad184x), 1000, TME_OK,
	    (TME_AD184X_LOG_HANDLE(ad184x),
	     _("%s <- 0x%02x"),
	     reg,
	     value));
  }

  /* otherwise, this must be a read: */
  else {
    assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register: */
    switch (address) {
    default: assert(FALSE);

    case TME_AD184X_REG_IADDR:
      value = ad184x->tme_ad184x_iaddr;
      reg = "IADDR";
      break;

    case TME_AD184X_REG_IDATA:
      ireg = _tme_ad184x_ireg(ad184x);
      reg = "(IDATA)";

      /* assume that this is a simple read: */
      value = ad184x->tme_ad184x_ireg[ireg];

      switch (ireg) {
      default: 
	break;
      case TME_CS423X_IREG_VERSION_ID:
	value = 0xa0;
	reg = "VERSION_ID";
	break;
      }
      break;

    case TME_AD184X_REG_STATUS:
      value = ad184x->tme_ad184x_status;
      reg = "STATUS";
      break;

    case TME_AD184X_REG_PIO:
      abort();
      reg = "PIO";
      break;
    }

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value,
			   TME_BUS8_LOG2);

    tme_log(TME_AD184X_LOG_HANDLE(ad184x), 1000, TME_OK,
	    (TME_AD184X_LOG_HANDLE(ad184x),
	     _("%s -> 0x%02x"),
	     reg,
	     value));
  }

  /* complete the cycle: */
  master_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(master_completion);
  *_master_fast_cycle_types = 0;

  /* leave: */
  _tme_ad184x_leave(ad184x);
}

/* this fills a TLB entry: */
static void
_tme_ad184x_tlb_fill(struct tme_bus_connection *agent_conn_bus,
		     struct tme_bus_tlb *tlb,
		     tme_bus_addr_t address_wider,
		     unsigned int cycle_type)
{

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers only this address: */
  tlb->tme_bus_tlb_addr_first = address_wider;
  tlb->tme_bus_tlb_addr_last = address_wider;
}

#if TME_AD184X_BUS_TRANSITION

/* this is the bus cycle transition glue: */
static int
_tme_ad184x_cycle_transition(void *_master_conn_bus,
			     struct tme_bus_cycle *master_cycle)
{
  struct tme_completion completion_buffer;
  struct tme_ad184x *ad184x;
  struct tme_bus_connection *master_conn_bus;
  tme_uint32_t master_fast_cycle_types;

  tme_completion_init(&completion_buffer);

  master_conn_bus = (struct tme_bus_connection *) _master_conn_bus;
  ad184x = (struct tme_ad184x *) master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  _tme_ad184x_cycle(master_conn_bus,
		    master_cycle,
		    &master_fast_cycle_types,
		    &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

/* this is the TLB fill transition glue: */
static int
_tme_ad184x_tlb_fill_transition(void *_ad184x,
				struct tme_bus_tlb *tlb,
				tme_bus_addr_t address_wider,
				unsigned int cycle_type)
{
  struct tme_ad184x *ad184x;
  struct tme_bus_connection *agent_conn_bus;

  ad184x = (struct tme_ad184x *) _ad184x;
  agent_conn_bus = (struct tme_bus_connection *) ad184x->tme_ad184x_device.tme_bus_device_connection->tme_bus_connection.tme_connection_other;

  _tme_ad184x_tlb_fill(agent_conn_bus,
		       tlb,
		       address_wider,
		       cycle_type);

  /* we always handle any slow cycles: */
  tlb->tme_bus_tlb_cycles_ok |= cycle_type;
  tlb->tme_bus_tlb_addr_offset = 0;
  tlb->tme_bus_tlb_addr_shift = 0;
  tlb->tme_bus_tlb_cycle = _tme_ad184x_cycle_transition;
  tlb->tme_bus_tlb_cycle_private = agent_conn_bus;
  assert (tlb->tme_bus_tlb_fault_handler_count == 0);

  return (TME_OK);
}
#define _tme_ad184x_tlb_fill _tme_ad184x_tlb_fill_transition

#endif /* TME_AD184X_BUS_TRANSITION */

/* the new ad184x function: */
static int
_tme_ad184x_new(struct tme_element *element,
		const char * const *args,
		const void *extra,
		char **_output,
		unsigned int part)
{
  struct tme_ad184x *ad184x;
  int arg_i;
  int usage;
  const char *type;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  type = NULL;
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

  /* start the ad184x structure: */
  ad184x = tme_new0(struct tme_ad184x, 1);
  tme_mutex_init(&ad184x->tme_ad184x_mutex);
  ad184x->tme_ad184x_part = part;
  ad184x->tme_ad184x_element = element;
  _tme_ad184x_reset(ad184x);

  /* initialize our simple bus device descriptor: */
  ad184x->tme_ad184x_device.tme_bus_device_tlb_fill = _tme_ad184x_tlb_fill;
  ad184x->tme_ad184x_device.tme_bus_device_address_last = TME_AD184X_SIZ - 1;

  /* fill the element: */
  element->tme_element_private = ad184x;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}

TME_ELEMENT_X_NEW_DECL(tme_ic_,ad184x,cs4231A) {
  return (_tme_ad184x_new(element, args, extra, _output, TME_AD184X_PART_CS4231A));
}
