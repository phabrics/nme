/* $Id: mm58167.c,v 1.8 2009/08/29 21:22:47 fredette Exp $ */

/* ic/mm58167.c - implementation of National Semiconductor MM58167 emulation: */

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
_TME_RCSID("$Id: mm58167.c,v 1.8 2009/08/29 21:22:47 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/ic/mm58167.h>
#include <time.h>
#include <sys/time.h>

/* macros: */

/* register addresses: */
#define TME_MM58167_REG_MSEC_XXX	(0)
#define TME_MM58167_REG_CSEC		(1)
#define TME_MM58167_REG_SEC		(2)
#define TME_MM58167_REG_MIN		(3)
#define TME_MM58167_REG_HOUR		(4)
#define TME_MM58167_REG_WDAY		(5)
#define TME_MM58167_REG_DAY		(6)
#define TME_MM58167_REG_MON		(7)
#define TME_MM58167_REG_STATUS		(20)
#define TME_MM58167_REG_GO		(21)
#define TME_MM58167_REG_BANK_SZ		(24)

/* bits in the status register: */
#define TME_MM58167_STATUS_RIPPLING	TME_BIT(0)	

#define TME_MM58167_LOG_HANDLE(mm) (&(mm)->tme_mm58167_element->tme_element_log_handle)

/* structures: */

struct tme_mm58167 {

  /* our simple bus device header: */
  struct tme_bus_device tme_mm58167_device;
#define tme_mm58167_element tme_mm58167_device.tme_bus_device_element

  /* our socket: */
  struct tme_mm58167_socket tme_mm58167_socket;
#define tme_mm58167_addr_shift tme_mm58167_socket.tme_mm58167_socket_addr_shift
#define tme_mm58167_port_least_lane tme_mm58167_socket.tme_mm58167_socket_port_least_lane

  /* the mutex protecting the chip: */
  tme_mutex_t tme_mm58167_mutex;

  /* the last time sampled.  the tv_usec field is actually a value in
     milliseconds, since we divide it by 1000 right after
     gettimeofday() returns: */
  struct timeval tme_mm58167_sampled_time;

  /* the struct tm for the last time sampled: */
  struct tm tme_mm58167_sampled_tm;

  /* the status register: */
  tme_uint8_t tme_mm58167_status;
};

/* the mm58167 bus router: */
static const tme_bus_lane_t tme_mm58167_router[TME_BUS_ROUTER_SIZE(TME_BUS8_LOG2)] = {
  
  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
};

/* the mm58167 bus cycle handler: */
static int
_tme_mm58167_bus_cycle(void *_mm58167, struct tme_bus_cycle *cycle_init)
{
  struct tme_mm58167 *mm58167;
  tme_bus_addr32_t address, mm58167_address_last;
  tme_uint8_t buffer, value;
  struct tme_bus_cycle cycle_resp;
  unsigned int reg;
  struct timeval now;
  time_t _now;
  struct tm *now_tm;
  int reg_is_bcd;

  /* recover our data structure: */
  mm58167 = (struct tme_mm58167 *) _mm58167;

  /* the requested cycle must be within range: */
  mm58167_address_last = mm58167->tme_mm58167_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= mm58167_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (mm58167_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* get the register being accessed: */
  address = cycle_init->tme_bus_cycle_address;
  reg = address >> mm58167->tme_mm58167_addr_shift;
  reg_is_bcd = (reg <= TME_MM58167_REG_MON);

  /* lock the mutex: */
  tme_mutex_lock(&mm58167->tme_mm58167_mutex);

  /* sample the time, and drop from microsecond accuracy to
     millisecond accuracy: */
  gettimeofday(&now, NULL);
  now.tv_usec /= 1000;

  /* if the seconds value has changed, convert it, and an update is
     rippling through the system: */
  if (now.tv_sec
      != mm58167->tme_mm58167_sampled_time.tv_sec) {
    mm58167->tme_mm58167_status |= TME_MM58167_STATUS_RIPPLING;
    _now = now.tv_sec;
    now_tm = gmtime_r(&_now, &mm58167->tme_mm58167_sampled_tm);
    if (now_tm != &mm58167->tme_mm58167_sampled_tm) {
      mm58167->tme_mm58167_sampled_tm = *now_tm;
    }
  }

  /* otherwise, if the centiseconds value has changed, an update
     is also rippling through the system: */
  else if ((now.tv_usec / 10)
	   != (mm58167->tme_mm58167_sampled_time.tv_usec / 10)) {
    mm58167->tme_mm58167_status |= TME_MM58167_STATUS_RIPPLING;
  }

  /* save the sampled time: */
  mm58167->tme_mm58167_sampled_time = now;

  /* get the converted time: */
  now_tm = &mm58167->tme_mm58167_sampled_tm;
    
  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_mm58167_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_READ;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(mm58167->tme_mm58167_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
    value = buffer;
    
    /* log this write: */
    tme_log(TME_MM58167_LOG_HANDLE(mm58167), 100000, TME_OK,
	    (TME_MM58167_LOG_HANDLE(mm58167),
	     "reg %d write %02x",
	     reg, value));

    /* otherwise, ignore writes for now: */
  }


  /* otherwise, this is a read: */
  else {
    assert(cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);
    
    /* dispatch on the register: */
    switch (reg) {
    case TME_MM58167_REG_MSEC_XXX:
      value = (now.tv_usec % 10) * 10;
      break;
    case TME_MM58167_REG_CSEC:
      value = (now.tv_usec / 10);
      break;
    case TME_MM58167_REG_SEC:
      value = now_tm->tm_sec;
      break;
    case TME_MM58167_REG_MIN:
      value = now_tm->tm_min;
      break;
    case TME_MM58167_REG_HOUR:
      value = now_tm->tm_hour;
      break;
    case TME_MM58167_REG_WDAY:
      value = now_tm->tm_wday;
      break;
    case TME_MM58167_REG_DAY:
      value = now_tm->tm_mday;
      break;
    case TME_MM58167_REG_MON:
      value = now_tm->tm_mon + 1;
      break;
    case TME_MM58167_REG_STATUS:
      value = mm58167->tme_mm58167_status;
      mm58167->tme_mm58167_status = 0;
      break;
    default:
      abort();
    }

    /* if needed, convert this value to BCD: */
    if (reg_is_bcd) {
      value = (value % 10) + ((value / 10) << 4);
    }

    /* log this read: */
    tme_log(TME_MM58167_LOG_HANDLE(mm58167), 100000, TME_OK,
	    (TME_MM58167_LOG_HANDLE(mm58167),
	     "reg %d read %02x",
	     reg, value));

    /* run the bus cycle: */
    buffer = value;
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_mm58167_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_WRITE;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(mm58167->tme_mm58167_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
  }
    
  /* unlock the mutex: */
  tme_mutex_unlock(&mm58167->tme_mm58167_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the mm58167 TLB filler: */
static int
_tme_mm58167_tlb_fill(void *_mm58167, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_mm58167 *mm58167;
  tme_bus_addr32_t mm58167_address_last;

  /* recover our data structure: */
  mm58167 = (struct tme_mm58167 *) _mm58167;

  /* the address must be within range: */
  mm58167_address_last = mm58167->tme_mm58167_device.tme_bus_device_address_last;
  assert(address <= mm58167_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = mm58167_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = mm58167;
  tlb->tme_bus_tlb_cycle = _tme_mm58167_bus_cycle;

  return (TME_OK);
}

/* the new mm58167 function: */
TME_ELEMENT_NEW_DECL(tme_ic_mm58167) {
  const struct tme_mm58167_socket *socket;
  struct tme_mm58167 *mm58167;
  struct tme_mm58167_socket socket_real;
  tme_bus_addr_t address_mask;

  /* dispatch on our socket version: */
  socket = (const struct tme_mm58167_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_mm58167_socket_version) {
  case TME_MM58167_SOCKET_0:
    socket_real = *socket;
    break;
  default: 
    tme_output_append_error(_output, _("socket type"));
    return (EOPNOTSUPP);
  }
    
  /* we take no arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, %s %s",
			    args[1],
			    _("unexpected"),
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the mm58167 structure: */
  mm58167 = tme_new0(struct tme_mm58167, 1);
  tme_mutex_init(&mm58167->tme_mm58167_mutex);
  mm58167->tme_mm58167_socket = socket_real;

  /* figure our address mask, up to the nearest power of two: */
  address_mask = TME_MM58167_REG_BANK_SZ << mm58167->tme_mm58167_addr_shift;
  if (address_mask & (address_mask - 1)) {
    for (; address_mask & (address_mask - 1); address_mask &= (address_mask - 1));
    address_mask <<= 1;
  }
  address_mask -= 1;

  /* initialize our simple bus device descriptor: */
  mm58167->tme_mm58167_device.tme_bus_device_element = element;
  mm58167->tme_mm58167_device.tme_bus_device_tlb_fill = _tme_mm58167_tlb_fill;
  mm58167->tme_mm58167_device.tme_bus_device_address_last = address_mask;

  /* fill the element: */
  element->tme_element_private = mm58167;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}
