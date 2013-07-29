/* $Id: am9513.c,v 1.17 2010/06/05 14:36:59 fredette Exp $ */

/* ic/am9513.c - implementation of Am9513 emulation: */

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
_TME_RCSID("$Id: am9513.c,v 1.17 2010/06/05 14:36:59 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/ic/am9513.h>
#include <sys/types.h>
#include <sys/time.h>

/* macros: */
#define TME_AM9513_CMD_SET_BUS_16BIT		(0xffef)
#define TME_AM9513_CMD_RESET			(0xffff)
#define TME_AM9513_CMD_LOAD_COUNTERS		(0xff40)
#define TME_AM9513_CMD_CLEAR_TOGGLE_OUT		(0xffe0)
#define TME_AM9513_CMD_LOAD_ARM_COUNTERS	(0xff60)
#define TME_AM9513_CMD_DISARM_COUNTERS		(0xffc0)
#define TME_AM9513_CMD_ARM_COUNTERS		(0xff20)
#define _TME_AM9513_CMD_TIMERS_MASK	(0x001f)
#define _TME_AM9513_CMD_TIMER_MASK	(0x0007)

/* bits in the Master Mode Register: */
#define TME_AM9513_MMR_BUS_16BIT	TME_BIT(13)
#define TME_AM9513_MMR_NO_INCREMENT	TME_BIT(14)

/* bits in the Status Register: */
#define TME_AM9513_STATUS_BYTE_POINTER	TME_BIT(0)

/* the Counter Mode register: */
#define TME_AM9513_CM_SOURCE_MASK		(0x0f00)
#define  TME_AM9513_CM_SOURCE_F1		(0x0b00)
#define  TME_AM9513_CM_SOURCE_F2		(0x0c00)
#define TME_AM9513_CM_REPEAT_ENA		(0x0020)
#define TME_AM9513_CM_OUTPUT_MASK		(0x0007)
#define  TME_AM9513_CM_OUTPUT_INACTIVE		(0x0000)
#define  TME_AM9513_CM_OUTPUT_TC_TOGGLED	(0x0002)

/* counter flags: */
#define TME_AM9513_COUNTER_FLAG_ARMED		TME_BIT(0)

/* define this to track interrupt rates, reporting once every N
   seconds: */
#if 1
#define TME_AM9513_TRACK_INT_RATE		(10)
#endif

#define TME_AM9513_LOG_HANDLE(am) (&(am)->tme_am9513_element->tme_element_log_handle)

/* structures: */
struct tme_am9513 {

  /* our simple bus device header: */
  struct tme_bus_device tme_am9513_device;
#define tme_am9513_element tme_am9513_device.tme_bus_device_element

  /* our socket: */
  struct tme_am9513_socket tme_am9513_socket;
#define tme_am9513_address_cmd tme_am9513_socket.tme_am9513_socket_address_cmd
#define tme_am9513_address_data tme_am9513_socket.tme_am9513_socket_address_data
#define tme_am9513_port_least_lane tme_am9513_socket.tme_am9513_socket_port_least_lane
#define tme_am9513_basic_clock tme_am9513_socket.tme_am9513_socket_basic_clock
  tme_uint32_t tme_am9513_basic_clock_msec;

  /* our mutex: */
  tme_mutex_t tme_am9513_mutex;

  /* this is nonzero iff callouts are running: */
  int tme_am9513_callouts_running;

  /* the control registers: */
  tme_uint16_t tme_am9513_control_regs[4];
#define tme_am9513_alarm1 tme_am9513_control_regs[0]
#define tme_am9513_alarm2 tme_am9513_control_regs[1]
#define tme_am9513_mmr tme_am9513_control_regs[2]
#define tme_am9513_status tme_am9513_control_regs[3]

  /* our counters: */
  struct tme_am9513_counter {
    tme_uint16_t tme_am9513_counter_regs[4];
#define tme_am9513_counter_mode tme_am9513_counter_regs[0]
#define tme_am9513_counter_load tme_am9513_counter_regs[1]
#define tme_am9513_counter_hold tme_am9513_counter_regs[2]
#define tme_am9513_counter_cntr tme_am9513_counter_regs[3]
    unsigned int tme_am9513_counter_flags;
#ifdef TME_AM9513_TRACK_INT_RATE
    unsigned long tme_am9513_counter_int_sample;
    struct timeval tme_am9513_counter_int_sample_time;
#endif /* TME_AM9513_TRACK_INT_RATE */
  } tme_am9513_counters[5];

  /* the data pointer register: */
  tme_uint8_t tme_am9513_data_pointer;

  /* the current output pins: */
  tme_uint8_t tme_am9513_output_pins;

  /* the last output pins we called out: */
  tme_uint8_t tme_am9513_output_pins_last;

  /* the last time our connection thread ran: */
  struct timeval tme_am9513_conn_last;
};

/* the Am9513 doesn't have any concept of endianness because it has a
   single "address bit" (the C/!D pin), so when we're running cycles
   we always read and write an entire 16-bit port using a data cycle
   buffer that points to a 16-bit variable, routing the least
   significant lane to the least significant part of the variable,
   etc.: */
#ifdef WORDS_BIGENDIAN
#define REG_LO	(1)
#define REG_HI	(0)
#else
#define REG_LO  (0)
#define REG_HI	(1)
#endif

/* the am9513 bus router: */
static const tme_bus_lane_t tme_am9513_router[TME_BUS_ROUTER_SIZE(TME_BUS16_LOG2)] = {
  
  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(REG_LO),
  /* D15-D8 */	TME_BUS_LANE_UNDEF,

  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 1: */
  /* D7-D0 */	TME_BUS_LANE_UNDEF,
  /* D15-D8 */	TME_BUS_LANE_ROUTE(REG_HI),

  /* [gen]  initiator port size: 16 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(REG_LO),
  /* D15-D8 */	TME_BUS_LANE_ROUTE(REG_HI),

  /* [gen]  initiator port size: 16 bits
     [gen]  initiator port least lane: 1 - invalid, array placeholder: */
  /* D7-D0 */	TME_BUS_LANE_ABORT,
  /* D15-D8 */	TME_BUS_LANE_ABORT
};

/* this resets an am9513: */
static void
_tme_am9513_reset(struct tme_am9513 *am9513)
{
  unsigned int counter_i;
  struct tme_am9513_counter *counter;

  /* disarm all counters: */
  /* XXX */

  /* enter 0000 into the Master Mode register: */
  am9513->tme_am9513_mmr = 0;

  /* initialize all of the counters: */
  for (counter_i = 0;
       counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
       counter_i++) {
    counter = &am9513->tme_am9513_counters[counter_i];
    
    counter->tme_am9513_counter_mode = 0x0b00;
    counter->tme_am9513_counter_load = 0x0000;
    counter->tme_am9513_counter_hold = 0x0000;
    counter->tme_am9513_counter_cntr = 0x0000;
    counter->tme_am9513_counter_flags = 0;
  }
}

/* this loads counters: */
static void
_tme_am9513_counters_load(struct tme_am9513 *am9513, tme_uint16_t counters_mask)
{
  unsigned int counter_i;
  struct tme_am9513_counter *counter;

  /* load all selected counters: */
  for (counter_i = 0;
       counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
       counter_i++) {
    if (counters_mask & TME_BIT(counter_i)) {
      counter = &am9513->tme_am9513_counters[counter_i];
      counter->tme_am9513_counter_cntr = counter->tme_am9513_counter_load;
    }
  }
}

/* this arms counters: */
static void
_tme_am9513_counters_arm(struct tme_am9513 *am9513, tme_uint16_t counters_mask)
{
  unsigned int counter_i;
  struct tme_am9513_counter *counter;

  /* load all selected counters: */
  for (counter_i = 0;
       counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
       counter_i++) {
    if (counters_mask & TME_BIT(counter_i)) {
      counter = &am9513->tme_am9513_counters[counter_i];
      counter->tme_am9513_counter_flags |= TME_AM9513_COUNTER_FLAG_ARMED;
    }
  }
}

/* this disarms counters: */
static void
_tme_am9513_counters_disarm(struct tme_am9513 *am9513, tme_uint16_t counters_mask)
{
  unsigned int counter_i;
  struct tme_am9513_counter *counter;

  /* load all selected counters: */
  for (counter_i = 0;
       counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
       counter_i++) {
    if (counters_mask & TME_BIT(counter_i)) {
      counter = &am9513->tme_am9513_counters[counter_i];
      counter->tme_am9513_counter_flags &= ~TME_AM9513_COUNTER_FLAG_ARMED;
    }
  }
}

/* this makes am9513 callouts.  it must be called with the mutex held: */
static void
_tme_am9513_callout(struct tme_am9513 *am9513)
{
  struct tme_bus_connection *conn_bus;
  unsigned int counter_i;
  int again;
  int pin_high;
  unsigned int signal;
  int rc;

  /* if this function is already running in another thread, return
     now.  the other thread will do our work: */
  if (am9513->tme_am9513_callouts_running) {
    return;
  }

  /* callouts are now running: */
  am9513->tme_am9513_callouts_running = TRUE;

  /* get our bus connection: */
  conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					    am9513->tme_am9513_device.tme_bus_device_connection,
					    &am9513->tme_am9513_device.tme_bus_device_connection_rwlock);

  /* loop forever: */
  for (again = TRUE; again;) {
    again = FALSE;

    /* check all of the counter output pins for changes: */
    for (counter_i = 0;
	 counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
	 counter_i++) {
      
      /* skip this output pin if it hasn't changed: */
      if (((am9513->tme_am9513_output_pins
	    ^ am9513->tme_am9513_output_pins_last)
	   & TME_BIT(counter_i)) == 0) {
	continue;
      }
	
      /* see if this pin is edging high or low: */
      pin_high = am9513->tme_am9513_output_pins & TME_BIT(counter_i);
      
      /* get any bus signal this pin maps to: */
      signal = am9513->tme_am9513_socket.tme_am9513_socket_counter_signals[counter_i];
      
      /* if this signal is ignored: */
      if (TME_BUS_SIGNAL_WHICH(signal) == TME_BUS_SIGNAL_IGNORE) {
	rc = TME_OK;
      }
      
      /* call out this signal edge: */
      else {

	/* unlock our mutex: */
	tme_mutex_unlock(&am9513->tme_am9513_mutex);

	rc = (*conn_bus->tme_bus_signal)
	  (conn_bus,
	   signal
	   ^ (pin_high
	      ? TME_BUS_SIGNAL_LEVEL_HIGH
	      : TME_BUS_SIGNAL_LEVEL_LOW));

	/* lock our mutex: */
	tme_mutex_lock(&am9513->tme_am9513_mutex);
      }
      
      /* if this call out succeeded, update the pin: */
      if (rc == TME_OK) {
	am9513->tme_am9513_output_pins_last =
	  ((am9513->tme_am9513_output_pins_last
	    & ~TME_BIT(counter_i))
	   | pin_high);
	again = TRUE;
      }
    }
  }

  /* there are no more callouts to make: */
  am9513->tme_am9513_callouts_running = FALSE;
}

/* the am9513 timer thread: */
static void
_tme_am9513_th_timer(struct tme_am9513 *am9513)
{
  struct timeval then, elapsed;
  tme_uint16_t counter_mode;
  tme_uint32_t basic_elapsed;
  tme_uint32_t basic_sleep;
  tme_uint32_t divisor;
  unsigned int counter_i;
  struct tme_am9513_counter *counter;
  tme_uint32_t counter_elapsed;

  /* loop forever: */
  for (;;) {

    /* figure out how much time has elapsed since our last run: */
    gettimeofday(&elapsed, NULL);
    then = am9513->tme_am9513_conn_last;
    am9513->tme_am9513_conn_last = elapsed;
    if (elapsed.tv_usec < then.tv_usec) {
      elapsed.tv_sec--;
      elapsed.tv_usec += 1000000;
    }
    elapsed.tv_sec -= then.tv_sec;
    elapsed.tv_usec -= then.tv_usec;

    /* calculate the number of basic ticks that have elapsed: */
    basic_elapsed = am9513->tme_am9513_basic_clock;
    basic_elapsed *= elapsed.tv_sec;
    basic_elapsed += (am9513->tme_am9513_basic_clock_msec * elapsed.tv_usec) / 1000;

    /* assume that we will sleep for one second: */
    basic_sleep = am9513->tme_am9513_basic_clock;

    /* lock our mutex: */
    tme_mutex_lock(&am9513->tme_am9513_mutex);

    /* check all of the counters: */
    for (counter_i = 0;
	 counter_i < TME_ARRAY_ELS(am9513->tme_am9513_counters);
	 counter_i++) {
      counter = &am9513->tme_am9513_counters[counter_i];
      counter_mode = counter->tme_am9513_counter_mode;

      /* dispatch on the counter source: */
      switch (counter_mode & TME_AM9513_CM_SOURCE_MASK) {

      case TME_AM9513_CM_SOURCE_F2:
      case TME_AM9513_CM_SOURCE_F1:
	
	/* if this counter is armed: */
	if (counter->tme_am9513_counter_flags & TME_AM9513_COUNTER_FLAG_ARMED) {

	  /* calculate the divisor: */
	  divisor = (1 << (((counter->tme_am9513_counter_mode & TME_AM9513_CM_SOURCE_MASK)
			    - TME_AM9513_CM_SOURCE_F1) >> 6));
	  
	  /* calculate the number of ticks on this counter that have happened: */
	  counter_elapsed = basic_elapsed / divisor;
	  
	  /* while this counter (repeatedly) reaches zero: */
	  for (; counter->tme_am9513_counter_cntr <= counter_elapsed; ) {
	    counter_elapsed -= counter->tme_am9513_counter_cntr;

	    /* dispatch on this counter's output control: */
	    switch (counter_mode & TME_AM9513_CM_OUTPUT_MASK) {
	    case TME_AM9513_CM_OUTPUT_INACTIVE:
	      break;
	    case TME_AM9513_CM_OUTPUT_TC_TOGGLED:

	      /* as a slightly sun2-specific hack, if this timer
		 output becomes an interrupt signal, we only set it,
		 and never clear it.  if we find it set, that means
		 clock interrupt latency is high: */
	      if (TME_BUS_SIGNAL_IS_INT(am9513->tme_am9513_socket.tme_am9513_socket_counter_signals[counter_i])) {
#ifdef TME_AM9513_TRACK_INT_RATE
		if (!(am9513->tme_am9513_output_pins & TME_BIT(counter_i))) {
		  counter->tme_am9513_counter_int_sample++;
		  am9513->tme_am9513_output_pins |= TME_BIT(counter_i);
		}
#else  /* !TME_AM9513_TRACK_INT_RATE */
		am9513->tme_am9513_output_pins |= TME_BIT(counter_i);
#endif /* !TME_AM9513_TRACK_INT_RATE */
	      }

	      /* otherwise, always toggle it like we're supposed to: */
	      else {
		am9513->tme_am9513_output_pins ^= TME_BIT(counter_i);
	      }
	      break;
	    default:
	      abort();
	    }
	    
	    /* if this counter reloads, reload it, otherwise disarm it: */
	    /* XXX is disarming it the right thing to do? */
	    if (counter_mode & TME_AM9513_CM_REPEAT_ENA) {
	      counter->tme_am9513_counter_cntr = counter->tme_am9513_counter_load;
	    }
	    else {
	      counter_elapsed = counter->tme_am9513_counter_cntr;
	      counter->tme_am9513_counter_flags &= ~TME_AM9513_COUNTER_FLAG_ARMED;
	      break;
	    }
	  }
	  counter->tme_am9513_counter_cntr -= counter_elapsed;
	
	  /* calculate the number of basic ticks until this counter expires: */
	  if (counter->tme_am9513_counter_cntr > 0) {
	    basic_sleep = TME_MIN(basic_sleep, 
				  counter->tme_am9513_counter_cntr * divisor);
	  }
	}

	break;

      default:
	abort();
      }

      /* no other bits can be set in the mode: */
      if (counter_mode & ~(TME_AM9513_CM_SOURCE_MASK
			   | TME_AM9513_CM_REPEAT_ENA
			   | TME_AM9513_CM_OUTPUT_MASK)) {
	abort();
      }

#ifdef TME_AM9513_TRACK_INT_RATE

      /* update the sample time: */
      for (counter->tme_am9513_counter_int_sample_time.tv_usec += elapsed.tv_usec;
	   counter->tme_am9513_counter_int_sample_time.tv_usec >= 1000000;
	   counter->tme_am9513_counter_int_sample_time.tv_usec -= 1000000) {
	counter->tme_am9513_counter_int_sample_time.tv_sec++;
      }
      counter->tme_am9513_counter_int_sample_time.tv_sec += elapsed.tv_sec;

      /* if the sample time has finished, report on the interrupt
         rate: */
      if (counter->tme_am9513_counter_int_sample_time.tv_sec
	  >= TME_AM9513_TRACK_INT_RATE) {
	if (counter->tme_am9513_counter_int_sample > 0) {
	  tme_log(TME_AM9513_LOG_HANDLE(am9513),
		  0, TME_OK,
		  (TME_AM9513_LOG_HANDLE(am9513),
		   "timer %d interrupt rate: %ld/sec",
		   counter_i,
		   (counter->tme_am9513_counter_int_sample
		    / (unsigned long) counter->tme_am9513_counter_int_sample_time.tv_sec)));
	}

	/* reset the sample: */
	counter->tme_am9513_counter_int_sample_time.tv_sec = 0;
	counter->tme_am9513_counter_int_sample_time.tv_usec = 0;
	counter->tme_am9513_counter_int_sample = 0;
      }
#endif /* TME_AM9513_TRACK_INT_RATE */
    }

    /* if we need to, call out for the output pins: */
    if (am9513->tme_am9513_output_pins
	!= am9513->tme_am9513_output_pins_last) {
      _tme_am9513_callout(am9513);
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&am9513->tme_am9513_mutex);

    /* sleep: */
    tme_thread_sleep_yield(0, (basic_sleep * 1000) / am9513->tme_am9513_basic_clock_msec);
  }
  /* NOTREACHED */
}

/* the am9513 bus cycle handler: */
static int
_tme_am9513_bus_cycle(void *_am9513, struct tme_bus_cycle *cycle_init)
{
  struct tme_am9513 *am9513;
  tme_bus_addr32_t am9513_address_last;
  tme_uint8_t data_pointer, group_pointer, element_pointer, data_pointer_next;
  tme_uint16_t byte_pointer;
  int is_cmd;
  tme_uint16_t *value, buffer, cmd;
  struct tme_am9513_counter *counter;
  struct tme_bus_cycle cycle_resp;
  int need_callout;

  /* recover our data structure: */
  am9513 = (struct tme_am9513 *) _am9513;

  /* the requested cycle must be within range: */
  am9513_address_last = am9513->tme_am9513_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= am9513_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (am9513_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* see if this is a command or data access: */
  is_cmd = ((am9513->tme_am9513_address_cmd
	     > am9513->tme_am9513_address_data)
	    == (cycle_init->tme_bus_cycle_address
		>= TME_MAX(am9513->tme_am9513_address_cmd,
			   am9513->tme_am9513_address_data)));

  /* assume that we won't need to call out: */
  need_callout = FALSE;

  /* lock the mutex: */
  tme_mutex_lock(&am9513->tme_am9513_mutex);

  /* assume there is no counter involved in this cycle, get out
     the data pointer, and assume that the data pointer will be
     unchanged: */
  counter = NULL;
  data_pointer = am9513->tme_am9513_data_pointer;
  data_pointer_next = data_pointer;
  byte_pointer = (am9513->tme_am9513_status & TME_AM9513_STATUS_BYTE_POINTER);

  /* initialize value to silence -Wuninitialized: */
  value = NULL;

  /* reads of the command register get the status, and writes to the
     command register are just processed: */
  if (is_cmd) {
    value = (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ
	     ? &am9513->tme_am9513_status
	     : NULL);
    byte_pointer = TME_AM9513_STATUS_BYTE_POINTER;
  }

  /* transfers to or from the data register involve whatever the Data
     Pointer Register is pointing to: */
  else {

    /* take out the Group Pointer and Element Pointer fields: */
    group_pointer = TME_FIELD_EXTRACTU(data_pointer, 0, 3);
    element_pointer = TME_FIELD_EXTRACTU(data_pointer, 3, 2);
    
    /* dispatch on the Group Pointer: */
    switch (group_pointer) {
      
      /* Groups 000 and 110 are illegal: */
    case 0:
    case 6:
      value = &buffer;
      break;
      
      /* Groups 001 through 101 are Counters: */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      counter = &am9513->tme_am9513_counters[group_pointer - 1];
      
      /* dispatch on the Element Pointer: */
      switch (element_pointer) {
	
	/* 00 = Mode Register, Element Cycle Increment: */
	/* 01 = Load Register, Element Cycle Increment: */
	/* 10 = Hold Register, Element Cycle Increment: */
      case 0:
      case 1:
      case 2:
	value = &counter->tme_am9513_counter_regs[element_pointer];
	element_pointer++;
	if (element_pointer == TME_ARRAY_ELS(counter->tme_am9513_counter_regs)) {
	  element_pointer = 0;
	  group_pointer++;
	  if (group_pointer - 1 == TME_ARRAY_ELS(am9513->tme_am9513_counters)) {
	    group_pointer = 1;
	  }
	}
	break;
	
	/* 11 = Hold Register, Hold Cycle Increment: */
      case 3:
	value = &counter->tme_am9513_counter_hold;
	group_pointer++;
	if (group_pointer - 1 == TME_ARRAY_ELS(am9513->tme_am9513_counters)) {
	  group_pointer = 1;
	}
	break;
      }

      /* if this is a read, log it.  remember that the element pointer
	 has been incremented: */
      if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
	tme_log(TME_AM9513_LOG_HANDLE(am9513),
		100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513),
		 "read Timer %d %s Register (data pointer 0x%02x) = 0x%04x",
		 group_pointer,
		 (element_pointer == 1
		  ? "Mode"
		  : element_pointer == 2
		  ? "Load"
		  : "Hold"),
		 data_pointer,
		 *value));
      }

      break;
      
      /* Group 111 is the Control Group: */
    case 7:
      
      /* dispatch on the Element Pointer: */
      switch (element_pointer) {
	
	/* 00 = Alarm Register 1, Control Cycle Increment: */
	/* 01 = Alarm Register 2, Control Cycle Increment: */
	/* 10 = Master Mode Register, Control Cycle Increment: */
      case 0:
      case 1:
      case 2:
	value = &am9513->tme_am9513_control_regs[element_pointer];
	element_pointer++;
	if (element_pointer == TME_ARRAY_ELS(am9513->tme_am9513_control_regs)) {
	  element_pointer = 0;
	}
	break;
	
	/* 11 = Status Register, No Increment: */
      case 3:
	value = &am9513->tme_am9513_status;
	break;
      }

      /* if this is a read, log it.  remember that the element pointer
	 has been incremented: */
      if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
	tme_log(TME_AM9513_LOG_HANDLE(am9513),
		100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513),
		 "read %s (data pointer 0x%02x) = 0x%04x",
		 (element_pointer == 1
		  ? "Alarm Register 1"
		  : element_pointer == 2
		  ? "Alarm Register 2"
		  : element_pointer == 0
		  ? "Master Mode Register"
		  : "Status Register"),
		 data_pointer,
		 *value));
      }

      break;
    }

    /* set what the next data pointer might be: */
    data_pointer_next = (element_pointer << 3) | group_pointer;
  }

  /* get the value to route on the bus.  if we're in 8-bit bus mode,
     route in D7-D0 either the low byte of the value or the high byte,
     depending on the Byte Pointer bit, with D15-D8 as all-bits-one: */
  buffer = (value == NULL ? 0xffff : *value);
  if ((am9513->tme_am9513_mmr & TME_AM9513_MMR_BUS_16BIT) == 0) {
    if (byte_pointer == 0) {
      buffer >>= 8;
    }
    buffer |= 0xff00;
  }

  /* run the bus cycle: */
  cycle_resp.tme_bus_cycle_buffer = (tme_uint8_t *) &buffer;
  cycle_resp.tme_bus_cycle_lane_routing = tme_am9513_router;
  cycle_resp.tme_bus_cycle_address = 0;
  cycle_resp.tme_bus_cycle_buffer_increment = 1;
  cycle_resp.tme_bus_cycle_type =
    (cycle_init->tme_bus_cycle_type
     ^ (TME_BUS_CYCLE_READ
	| TME_BUS_CYCLE_WRITE));
  cycle_resp.tme_bus_cycle_size = sizeof(tme_uint16_t);
  cycle_resp.tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT(am9513->tme_am9513_port_least_lane,
		       TME_BUS16_LOG2);
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {
    
    /* if this is a write to the command register: */
    if (is_cmd) {

      /* dispatch on the command: */
      cmd = buffer;

      /* set 16-bit mode: */
      if (cmd == TME_AM9513_CMD_SET_BUS_16BIT) {
	am9513->tme_am9513_mmr |= TME_AM9513_MMR_BUS_16BIT;
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), "set 16-bit bus mode"));
      }
      
      /* load data pointer: */
      else if ((cmd & 0xe0) == 0x00) {
	am9513->tme_am9513_data_pointer = (cmd & 0x1f);
	am9513->tme_am9513_status |= TME_AM9513_STATUS_BYTE_POINTER;
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513),
		 "set data pointer to %02x",
		 am9513->tme_am9513_data_pointer));
      }
      
      /* master reset: */
      else if (cmd == TME_AM9513_CMD_RESET) {
	_tme_am9513_reset(am9513);
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), "master reset"));
      }

      /* load counters: */
      else if ((cmd & ~_TME_AM9513_CMD_TIMERS_MASK)
	       == TME_AM9513_CMD_LOAD_COUNTERS) {
	_tme_am9513_counters_load(am9513, cmd & _TME_AM9513_CMD_TIMERS_MASK);
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), 
		 "load counters, timer mask 0x%02x",
		 (cmd & _TME_AM9513_CMD_TIMERS_MASK)));
      }

      /* clear toggle out: */
      else if ((cmd & ~_TME_AM9513_CMD_TIMER_MASK)
	       == TME_AM9513_CMD_CLEAR_TOGGLE_OUT) {

	/* clear the output pin for the given timer: */
	group_pointer = cmd & _TME_AM9513_CMD_TIMER_MASK;
	am9513->tme_am9513_output_pins &= ~TME_BIT(group_pointer - 1);
	need_callout = TRUE;

	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), 
		 "Timer %d clear toggle out",
		 (cmd & _TME_AM9513_CMD_TIMER_MASK)));
      }

      /* load and arm counters: */
      else if ((cmd & ~_TME_AM9513_CMD_TIMERS_MASK)
	       == TME_AM9513_CMD_LOAD_ARM_COUNTERS) {
	_tme_am9513_counters_load(am9513, cmd & _TME_AM9513_CMD_TIMERS_MASK);
	_tme_am9513_counters_arm(am9513, cmd & _TME_AM9513_CMD_TIMERS_MASK);
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), 
		 "load and arm counters, timer mask 0x%02x",
		 (cmd & _TME_AM9513_CMD_TIMERS_MASK)));
      }

      /* disarm counters: */
      else if ((cmd & ~_TME_AM9513_CMD_TIMERS_MASK)
	       == TME_AM9513_CMD_DISARM_COUNTERS) {
	_tme_am9513_counters_disarm(am9513, cmd & _TME_AM9513_CMD_TIMERS_MASK);
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), 
		 "disarm counters, timer mask 0x%02x",
		 (cmd & _TME_AM9513_CMD_TIMERS_MASK)));
      }

      /* arm counters: */
      else if ((cmd & ~_TME_AM9513_CMD_TIMERS_MASK)
	       == TME_AM9513_CMD_ARM_COUNTERS) {
	_tme_am9513_counters_arm(am9513, cmd & _TME_AM9513_CMD_TIMERS_MASK);
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513), 
		 "arm counters, timer mask 0x%02x",
		 (cmd & _TME_AM9513_CMD_TIMERS_MASK)));
      }

      else {
	abort();
      }
    }

    /* otherwise this is a write to a data register: */
    else {

      /* take out the Group Pointer and Element Pointer fields: */
      group_pointer = TME_FIELD_EXTRACTU(data_pointer, 0, 3);
      element_pointer = TME_FIELD_EXTRACTU(data_pointer, 3, 2);
    
      /* update the register: */
      if ((am9513->tme_am9513_mmr & TME_AM9513_MMR_BUS_16BIT) == 0) {      
	if (byte_pointer) {
	  *value = (*value & 0xff00) | (buffer & 0xff);
	}
	else {
	  *value = (*value & 0x00ff) | ((buffer & 0xff) << 8);
	}
      }
      else {
	*value = buffer;
      }

      /* if this changed a counter register: */
      if (counter != NULL) {

	/* log the write: */
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513),
		 "write Timer %d %s Register (data pointer 0x%02x) = 0x%04x",
		 group_pointer,
		 (element_pointer == 0
		  ? "Mode"
		  : element_pointer == 1
		  ? "Load"
		  : "Hold"),
		 data_pointer,
		 *value));
	
	/* dispatch on the Element Pointer: */
	switch (element_pointer) {
	
	  /* 00 = Mode Register: */
	case 0:
	  /* XXX TBD */
	  break;
	  
	  /* 01 = Load Register: */
	case 1:
	  /* XXX TBD */
	  break;
	  
	  /* 10 = Hold Register: */
	  /* 11 = Hold Register: */
	case 2:
	case 3:
	  /* XXX TBD */
	  break;
	}
      }

      /* if this changed a control register: */
      else {

	/* log the write: */
	tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
		(TME_AM9513_LOG_HANDLE(am9513),
		 "write %s (data pointer 0x%02x) = 0x%04x",
		 (element_pointer == 0
		  ? "Alarm Register 1"
		  : element_pointer == 1
		  ? "Alarm Register 2"
		  : element_pointer == 2
		  ? "Master Mode Register"
		  : "Status Register"),
		 data_pointer,
		 *value));

	/* dispatch on the Element Pointer: */
	switch (element_pointer) {
	  
	  /* 00 = Alarm Register 1: */
	case 0:
	  /* XXX TBD */
	  break;
	  
	  /* 01 = Alarm Register 2: */
	case 1:
	  /* XXX TBD */
	  break;
	  
	  /* 10 = Master Mode Register: */
	case 2:
	  /* XXX TBD */
	  break;
	  
	  /* 11 = Status Register: */
	case 3:
	  /* XXX TBD */
	  break;
	}

      }
    }
  }

  /* update the byte pointer and data pointer register as needed: */
  if (!is_cmd) {
    if ((am9513->tme_am9513_mmr & TME_AM9513_MMR_BUS_16BIT) == 0) {
      byte_pointer ^= TME_AM9513_STATUS_BYTE_POINTER;
    }
    am9513->tme_am9513_status = ((am9513->tme_am9513_status
				  & ~TME_AM9513_STATUS_BYTE_POINTER)
				 | byte_pointer);
    if (byte_pointer != 0
	&& !(am9513->tme_am9513_mmr & TME_AM9513_MMR_NO_INCREMENT)) {
      am9513->tme_am9513_data_pointer = data_pointer_next;
    }
    tme_log(TME_AM9513_LOG_HANDLE(am9513), 100000, TME_OK,
	    (TME_AM9513_LOG_HANDLE(am9513),
	     "data pointer now 0x%02x (byte pointer %d)",
	     am9513->tme_am9513_data_pointer,
	     byte_pointer / TME_AM9513_STATUS_BYTE_POINTER));
  }

  /* if we need callouts, make them: */
  if (need_callout) {
    _tme_am9513_callout(am9513);
  }
    
  /* unlock the mutex: */
  tme_mutex_unlock(&am9513->tme_am9513_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the am9513 TLB filler: */
static int
_tme_am9513_tlb_fill(void *_am9513, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_am9513 *am9513;
  tme_bus_addr32_t am9513_address_last;

  /* recover our data structure: */
  am9513 = (struct tme_am9513 *) _am9513;

  /* the address must be within range: */
  am9513_address_last = am9513->tme_am9513_device.tme_bus_device_address_last;
  assert(address <= am9513_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = am9513_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = am9513;
  tlb->tme_bus_tlb_cycle = _tme_am9513_bus_cycle;

  return (TME_OK);
}

/* the new am9513 element function: */
TME_ELEMENT_NEW_DECL(tme_ic_am9513) {
  const struct tme_am9513_socket *socket;
  struct tme_am9513 *am9513;
  struct tme_am9513_socket socket_real;
  tme_bus_addr_t address_mask;

  /* dispatch on our socket version: */
  socket = (const struct tme_am9513_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_am9513_socket_version) {
  case TME_AM9513_SOCKET_0:
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

  /* start the am9513 structure: */
  am9513 = tme_new0(struct tme_am9513, 1);
  am9513->tme_am9513_socket = socket_real;
  am9513->tme_am9513_basic_clock_msec = am9513->tme_am9513_basic_clock / 1000;
  am9513->tme_am9513_element = element;
  _tme_am9513_reset(am9513);

  /* figure our address mask, up to the nearest power of two: */
  address_mask = TME_MAX(am9513->tme_am9513_address_cmd,
			 am9513->tme_am9513_address_data);
  address_mask += sizeof(tme_uint16_t);
  if (address_mask & (address_mask - 1)) {
    for (; address_mask & (address_mask - 1); address_mask &= (address_mask - 1));
    address_mask <<= 1;
  }
  address_mask -= 1;

  /* initialize our simple bus device descriptor: */
  am9513->tme_am9513_device.tme_bus_device_tlb_fill = _tme_am9513_tlb_fill;
  am9513->tme_am9513_device.tme_bus_device_address_last = address_mask;

  /* start the timer thread: */
  tme_mutex_init(&am9513->tme_am9513_mutex);
  tme_thread_create((tme_thread_t) _tme_am9513_th_timer, am9513);

  /* fill the element: */
  element->tme_element_private = am9513;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}
