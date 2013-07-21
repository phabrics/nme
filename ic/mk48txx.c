/* $Id: mk48txx.c,v 1.4 2010/06/05 14:54:10 fredette Exp $ */

/* ic/mk48txx.c - implementation of Mostek 48Txx emulation: */

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
_TME_RCSID("$Id: mk48txx.c,v 1.4 2010/06/05 14:54:10 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/ic/mk48txx.h>
#include <tme/misc.h>
#include <time.h>
#include <sys/time.h>

/* macros: */

/* the different parts: */
#define TME_MK48TXX_PART_02	(02)
#define TME_MK48TXX_PART_59	(59)

/* register addresses: */
#define TME_MK48TXX_REGS_COUNT	(16)		/* maximum number of registers */
#define TME_MK48TXX_REG_YEAR	(TME_MK48TXX_REGS_COUNT - 1)	/* BCD year */
#define TME_MK48TXX_REG_MON	(TME_MK48TXX_REGS_COUNT - 2)	/* BCD month */
#define TME_MK48TXX_REG_DAY	(TME_MK48TXX_REGS_COUNT - 3)	/* BCD day in month */
#define TME_MK48TXX_REG_WDAY	(TME_MK48TXX_REGS_COUNT - 4)	/* weekday */
#define TME_MK48TXX_REG_HOUR	(TME_MK48TXX_REGS_COUNT - 5)	/* BCD hour */
#define TME_MK48TXX_REG_MIN	(TME_MK48TXX_REGS_COUNT - 6)	/* BCD minutes */
#define TME_MK48TXX_REG_SEC	(TME_MK48TXX_REGS_COUNT - 7)	/* BCD seconds */
#define TME_MK48TXX_REG_CSR	(TME_MK48TXX_REGS_COUNT - 8)	/* control register */
#define TME_MK48TXX_REG_WDOG	(TME_MK48TXX_REGS_COUNT - 9)	/* watchdog */
#define TME_MK48TXX_REG_INTR	(TME_MK48TXX_REGS_COUNT - 10)	/* interrupts */
#define TME_MK48TXX_REG_ADAY	(TME_MK48TXX_REGS_COUNT - 11)	/* BCD alarm day */
#define TME_MK48TXX_REG_AHOUR	(TME_MK48TXX_REGS_COUNT - 12)	/* BCD alarm hour */
#define TME_MK48TXX_REG_AMIN	(TME_MK48TXX_REGS_COUNT - 13)	/* BCD alarm minutes */
#define TME_MK48TXX_REG_ASEC	(TME_MK48TXX_REGS_COUNT - 14)	/* BCD alarm seconds */
			     /* (TME_MK48TXX_REGS_COUNT - 15)	   unused */
#define TME_MK48TXX_REG_FLAGS	(TME_MK48TXX_REGS_COUNT - 16)	/* flags */

/* the first register implemented by a part: */
#define TME_MK48TXX_REG_FIRST(part)	\
  ((part) == TME_MK48TXX_PART_59	\
   ? TME_MK48TXX_REG_FLAGS		\
   : TME_MK48TXX_REG_CSR)

/* bits in the CSR: */
#define TME_MK48TXX_CSR_WRITE	TME_BIT(7)	/* start writing */
#define TME_MK48TXX_CSR_READ	TME_BIT(6)	/* start reading */

/* bits in the weekday register: */
#define TME_MK48TXX_WDAY_FTEST	TME_BIT(6)	/* 512Hz frequency test */

/* bits in the seconds register: */
#define TME_MK48TXX_SEC_STOP	TME_BIT(7)	/* all stop */

#define TME_MK48TXX_LOG_HANDLE(am) (&(am)->tme_mk48txx_element->tme_element_log_handle)

/* structures: */
struct tme_mk48txx {

  /* our simple bus device header: */
  struct tme_bus_device tme_mk48txx_device;
#define tme_mk48txx_element tme_mk48txx_device.tme_bus_device_element

  /* our socket: */
  struct tme_mk48txx_socket tme_mk48txx_socket;
#define tme_mk48txx_addr_shift tme_mk48txx_socket.tme_mk48txx_socket_addr_shift
#define tme_mk48txx_port_least_lane tme_mk48txx_socket.tme_mk48txx_socket_port_least_lane
#define tme_mk48txx_year_zero tme_mk48txx_socket.tme_mk48txx_socket_year_zero

  /* our mutex: */
  tme_mutex_t tme_mk48txx_mutex;

  /* the part emulated: */
  unsigned int tme_mk48txx_part;

  /* our timer condition: */
  tme_cond_t tme_mk48txx_cond_timer;

  /* it's easiest to just model the chip as a chunk of memory: */
  tme_uint8_t tme_mk48txx_regs[TME_MK48TXX_REGS_COUNT];

  /* if nonzero, the internal time-of-day needs to be updated: */
  tme_uint8_t tme_mk48txx_tod_update;
};

/* the mk48txx bus router: */
static const tme_bus_lane_t tme_mk48txx_router[TME_BUS_ROUTER_SIZE(TME_BUS8_LOG2)] = {
  
  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
};

/* these convert values to and from BCD: */
static inline tme_uint8_t
_tme_mk48txx_bcd_out(unsigned int value)
{
  return ((value % 10)
	  + ((value / 10) * 16));
}
static inline unsigned int
_tme_mk48txx_bcd_in(tme_uint8_t value)
{
  return ((value % 16)
	  + ((value / 16) * 10));
}

/* this resets an mk48txx: */
static void
_tme_mk48txx_reset(struct tme_mk48txx *mk48txx)
{

  /* clear the CSR: */
  mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_CSR] = 0;

  /* start the clock running normally: */
  mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_WDAY] = 0;
  mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_SEC] = !TME_MK48TXX_SEC_STOP;
}

/* the mk48txx bus cycle handler: */
static int
_tme_mk48txx_bus_cycle(void *_mk48txx, struct tme_bus_cycle *cycle_init)
{
  struct tme_mk48txx *mk48txx;
  tme_bus_addr32_t address, mk48txx_address_last;
  tme_uint8_t buffer, value;
  struct tme_bus_cycle cycle_resp;
  unsigned int reg;
  struct timeval now;
  time_t _now;
  struct tm *now_tm, now_tm_buffer;

  /* recover our data structure: */
  mk48txx = (struct tme_mk48txx *) _mk48txx;

  /* the requested cycle must be within range: */
  mk48txx_address_last = mk48txx->tme_mk48txx_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= mk48txx_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (mk48txx_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* get the register being accessed: */
  address = cycle_init->tme_bus_cycle_address;
  reg = (address >> mk48txx->tme_mk48txx_addr_shift) + TME_MK48TXX_REG_FIRST(mk48txx->tme_mk48txx_part);

  /* lock the mutex: */
  tme_mutex_lock(&mk48txx->tme_mk48txx_mutex);

  /* if the clock is not being read or written: */
  if ((mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_CSR]
       & (TME_MK48TXX_CSR_READ
	  | TME_MK48TXX_CSR_WRITE)) == 0) {

    /* sample the time of day: */
    gettimeofday(&now, NULL);
    _now = now.tv_sec;
    now_tm = gmtime_r(&_now, &now_tm_buffer);

    /* put the time-of-day into the registers: */
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_HOUR] = _tme_mk48txx_bcd_out(now_tm->tm_hour);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_MIN] = _tme_mk48txx_bcd_out(now_tm->tm_min);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_SEC] = _tme_mk48txx_bcd_out(now_tm->tm_sec);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_MON] = _tme_mk48txx_bcd_out(now_tm->tm_mon + 1);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_DAY] = _tme_mk48txx_bcd_out(now_tm->tm_mday);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_YEAR]
      = _tme_mk48txx_bcd_out((1900 + now_tm->tm_year) - mk48txx->tme_mk48txx_year_zero);
    mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_WDAY] = now_tm->tm_wday;
  }

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_mk48txx_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_READ;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(mk48txx->tme_mk48txx_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
    value = buffer;
    
    /* log this write: */
    tme_log(TME_MK48TXX_LOG_HANDLE(mk48txx), 100000, TME_OK,
	    (TME_MK48TXX_LOG_HANDLE(mk48txx),
	     "reg %d write %02x",
	     reg, value));

    /* dispatch on the register: */
    switch (reg) {

    case TME_MK48TXX_REG_WDAY:
      
      /* flag that the time-of-day needs to be updated: */
      mk48txx->tme_mk48txx_tod_update = TRUE;

      /* update the register: */
      mk48txx->tme_mk48txx_regs[reg] = value;
      break;

    case TME_MK48TXX_REG_HOUR:
    case TME_MK48TXX_REG_MIN:
    case TME_MK48TXX_REG_SEC:
    case TME_MK48TXX_REG_MON:
    case TME_MK48TXX_REG_DAY:
    case TME_MK48TXX_REG_YEAR:

      /* flag that the time-of-day needs to be updated: */
      mk48txx->tme_mk48txx_tod_update = TRUE;

      /* update the register: */
      mk48txx->tme_mk48txx_regs[reg] = value;
      break;

    case TME_MK48TXX_REG_CSR:

      /* update the CSR: */
      mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_CSR] = value;

      break;

    case TME_MK48TXX_REG_ADAY:
    case TME_MK48TXX_REG_AHOUR:
    case TME_MK48TXX_REG_AMIN:
    case TME_MK48TXX_REG_ASEC:
    case TME_MK48TXX_REG_WDOG:
    case TME_MK48TXX_REG_INTR:
    case TME_MK48TXX_REG_FLAGS:

      /* update the register: */
      mk48txx->tme_mk48txx_regs[reg] = value;
      break;

    default:
      /* ignore */
      break;
    }
  }

  /* otherwise, this is a read: */
  else {
    assert(cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);
    
    /* dispatch on the register: */
    switch (reg) {

    case TME_MK48TXX_REG_HOUR:
    case TME_MK48TXX_REG_MIN:
    case TME_MK48TXX_REG_SEC:
    case TME_MK48TXX_REG_MON:
    case TME_MK48TXX_REG_DAY:
    case TME_MK48TXX_REG_YEAR:
    case TME_MK48TXX_REG_WDAY:
    case TME_MK48TXX_REG_ADAY:
    case TME_MK48TXX_REG_AHOUR:
    case TME_MK48TXX_REG_AMIN:
    case TME_MK48TXX_REG_ASEC:
    case TME_MK48TXX_REG_WDOG:
    case TME_MK48TXX_REG_INTR:
    case TME_MK48TXX_REG_FLAGS:

      /* read the register: */
      value = mk48txx->tme_mk48txx_regs[reg];
      break;

    case TME_MK48TXX_REG_CSR:

      /* read the register: */
      value = mk48txx->tme_mk48txx_regs[reg];
      break;

      /* undefined registers return garbage when read: */
    default:
      value = 0xff;
      break;
    }

    /* log this read: */
    tme_log(TME_MK48TXX_LOG_HANDLE(mk48txx), 100000, TME_OK,
	    (TME_MK48TXX_LOG_HANDLE(mk48txx),
	     "reg %d read %02x",
	     reg, value));

    /* run the bus cycle: */
    buffer = value;
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_mk48txx_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_WRITE;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(mk48txx->tme_mk48txx_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
  }

  /* if the time-of-day registers have been updated, and the clock
     is running: */
  if (mk48txx->tme_mk48txx_tod_update
      && ((mk48txx->tme_mk48txx_regs[TME_MK48TXX_REG_CSR]
	   & (TME_MK48TXX_CSR_READ
	      | TME_MK48TXX_CSR_WRITE)) == 0)) {
    
    /* XXX update the host's time-of-day clock? */
    mk48txx->tme_mk48txx_tod_update = FALSE;
  }
    
  /* unlock the mutex: */
  tme_mutex_unlock(&mk48txx->tme_mk48txx_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the mk48txx TLB filler: */
static int
_tme_mk48txx_tlb_fill(void *_mk48txx, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_mk48txx *mk48txx;
  tme_bus_addr32_t mk48txx_address_last;

  /* recover our data structure: */
  mk48txx = (struct tme_mk48txx *) _mk48txx;

  /* the address must be within range: */
  mk48txx_address_last = mk48txx->tme_mk48txx_device.tme_bus_device_address_last;
  assert(address <= mk48txx_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = mk48txx_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = mk48txx;
  tlb->tme_bus_tlb_cycle = _tme_mk48txx_bus_cycle;

  return (TME_OK);
}

/* the new mk48txx element function: */
static int
_tme_mk48txx_new(struct tme_element *element,
		 const char * const *args,
		 const void *extra,
		 char **_output,
		 unsigned int part)
{
  const struct tme_mk48txx_socket *socket;
  struct tme_mk48txx *mk48txx;
  struct tme_mk48txx_socket socket_real;
  tme_bus_addr_t address_mask;
  int arg_i;
  int usage;

  /* dispatch on our socket version: */
  socket = (const struct tme_mk48txx_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_mk48txx_socket_version) {
  case TME_MK48TXX_SOCKET_0:
    socket_real = *socket;
    break;
  default: 
    tme_output_append_error(_output, _("socket type"));
    return (EOPNOTSUPP);
  }
    
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

  /* start the mk48txx structure: */
  mk48txx = tme_new0(struct tme_mk48txx, 1);
  tme_mutex_init(&mk48txx->tme_mk48txx_mutex);
  mk48txx->tme_mk48txx_part = part;
  mk48txx->tme_mk48txx_socket = socket_real;
  mk48txx->tme_mk48txx_element = element;
  _tme_mk48txx_reset(mk48txx);

  /* figure our address mask, up to the nearest power of two: */
  address_mask = (TME_MK48TXX_REGS_COUNT - TME_MK48TXX_REG_FIRST(part)) << mk48txx->tme_mk48txx_addr_shift;
  if (address_mask & (address_mask - 1)) {
    for (; address_mask & (address_mask - 1); address_mask &= (address_mask - 1));
    address_mask <<= 1;
  }
  address_mask -= 1;

  /* initialize our simple bus device descriptor: */
  mk48txx->tme_mk48txx_device.tme_bus_device_tlb_fill = _tme_mk48txx_tlb_fill;
  mk48txx->tme_mk48txx_device.tme_bus_device_address_last = address_mask;

  /* fill the element: */
  element->tme_element_private = mk48txx;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}

TME_ELEMENT_X_NEW_DECL(tme_ic_,mk48txx,mk48t02) {
  return (_tme_mk48txx_new(element, args, extra, _output, TME_MK48TXX_PART_02));
}

TME_ELEMENT_X_NEW_DECL(tme_ic_,mk48txx,mk48t59) {
  return (_tme_mk48txx_new(element, args, extra, _output, TME_MK48TXX_PART_59));
}
