/* $Id: isil7170.c,v 1.6 2010/06/05 14:37:27 fredette Exp $ */

/* ic/isil7170.c - implementation of Intersil 7170 emulation: */

/*
 * Copyright (c) 2004 Matt Fredette
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
_TME_RCSID("$Id: isil7170.c,v 1.6 2010/06/05 14:37:27 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/ic/isil7170.h>
#include <tme/misc.h>
#include <time.h>
#include <sys/time.h>

/* macros: */

/* register addresses: */
#define TME_ISIL7170_REG_CSEC		(0)
#define TME_ISIL7170_REG_HOUR		(1)
#define TME_ISIL7170_REG_MIN		(2)
#define TME_ISIL7170_REG_SEC		(3)
#define TME_ISIL7170_REG_MON		(4)
#define TME_ISIL7170_REG_DAY		(5)
#define TME_ISIL7170_REG_YEAR		(6)
#define TME_ISIL7170_REG_DOW		(7)
#define TME_ISIL7170_REG_CMP_CSEC	(8)
#define TME_ISIL7170_REG_CMP_HOUR	(9)
#define TME_ISIL7170_REG_CMP_MIN	(10)
#define TME_ISIL7170_REG_CMP_SEC	(11)
#define TME_ISIL7170_REG_CMP_MON	(12)
#define TME_ISIL7170_REG_CMP_DAY	(13)
#define TME_ISIL7170_REG_CMP_YEAR	(14)
#define TME_ISIL7170_REG_CMP_DOW	(15)
#define TME_ISIL7170_REG_INT		(16)
#define TME_ISIL7170_REG_CMD		(17)
#define TME_ISIL7170_REGS_COUNT		(32)

/* bits in the Interrupt register: */
#define	TME_ISIL7170_INT_PENDING	TME_BIT(7)	/* interrupt pending */
#define	TME_ISIL7170_INT_DAY		TME_BIT(6)	/* day periodic interrupt */
#define	TME_ISIL7170_INT_HOUR		TME_BIT(5)	/* hour periodic interrupt */
#define	TME_ISIL7170_INT_MIN		TME_BIT(4)	/* minute periodic interrupt */
#define	TME_ISIL7170_INT_SEC		TME_BIT(3)	/* second periodic interrupt */
#define	TME_ISIL7170_INT_TSEC		TME_BIT(2)	/* 1/10 second periodic interrupt */
#define	TME_ISIL7170_INT_HSEC		TME_BIT(1)	/* 1/100 periodic second interrupt */
#define	TME_ISIL7170_INT_ALARM		TME_BIT(0)	/* time match interrupt */

/* bits in the Command register: */
#define	TME_ISIL7170_CMD_TEST		TME_BIT(5)	/* test mode */
#define	TME_ISIL7170_CMD_INTENA		TME_BIT(4)	/* interrupt enable */
#define	TME_ISIL7170_CMD_RUN		TME_BIT(3)	/* running */
#define	TME_ISIL7170_CMD_FMT24		TME_BIT(2)	/* 24-hour format (instead of 12-hour) */
#define TME_ISIL7170_CMD_FREQ_MASK	(0x3)		/* frequency mask */
#define  TME_ISIL7170_CMD_FREQ_4M	(0x3)		/* frequency 4.194304MHz */
#define  TME_ISIL7170_CMD_FREQ_2M	(0x2)		/* frequency 2.097152MHz */
#define  TME_ISIL7170_CMD_FREQ_1M	(0x1)		/* frequency 1.048576MHz */
#define  TME_ISIL7170_CMD_FREQ_32K	(0x0)		/* frequency 32.768KHz */

/* year zero in the chip corresponds to 1968: */
#define TME_ISIL7170_REG_YEAR_0		(1968)

/* define this to track interrupt rates, reporting once every N
   seconds: */
#if 1
#define TME_ISIL7170_TRACK_INT_RATE		(10)
#endif

#define TME_ISIL7170_LOG_HANDLE(am) (&(am)->tme_isil7170_element->tme_element_log_handle)

/* structures: */
struct tme_isil7170 {

  /* our simple bus device header: */
  struct tme_bus_device tme_isil7170_device;
#define tme_isil7170_element tme_isil7170_device.tme_bus_device_element

  /* our socket: */
  struct tme_isil7170_socket tme_isil7170_socket;
#define tme_isil7170_addr_shift tme_isil7170_socket.tme_isil7170_socket_addr_shift
#define tme_isil7170_port_least_lane tme_isil7170_socket.tme_isil7170_socket_port_least_lane
#define tme_isil7170_clock_basic tme_isil7170_socket.tme_isil7170_socket_clock_basic
#define tme_isil7170_int_signal tme_isil7170_socket.tme_isil7170_socket_int_signal

  /* our mutex: */
  tme_mutex_t tme_isil7170_mutex;

  /* our timer condition: */
  tme_cond_t tme_isil7170_cond_timer;

  /* this is nonzero iff callouts are running: */
  int tme_isil7170_callouts_running;

  /* it's easiest to just model the chip as a chunk of memory: */
  tme_uint8_t tme_isil7170_regs[TME_ISIL7170_REGS_COUNT];

  /* the real-time durations, in microseconds, of a hundredth of a
     second and a tenth of a second: */
  unsigned long tme_isil7170_clock_hsec_usec;
  unsigned long tme_isil7170_clock_tsec_usec;

  /* if nonzero, the internal time-of-day needs to be updated: */
  tme_uint8_t tme_isil7170_tod_update;

  /* if nonzero, the interrupt the timer thread is sleeping for: */
  tme_uint8_t tme_isil7170_timer_sleeping;

  /* the interrupt mask: */
  tme_uint8_t tme_isil7170_int_mask;

  /* nonzero if the interrupt signal is asserted: */
  int tme_isil7170_int_asserted;

  /* any scaling factor: */
  unsigned long tme_isil7170_clock_scale;

#ifdef TME_ISIL7170_TRACK_INT_RATE

  /* the end time of this sample: */
  struct timeval tme_isil7170_int_sample_time;

  /* the number of distinct interrupts that have been delivered during
     this sample: */
  unsigned long tme_isil7170_int_sample;
#endif /* TME_ISIL7170_TRACK_INT_RATE */
};

/* the isil7170 bus router: */
static const tme_bus_lane_t tme_isil7170_router[TME_BUS_ROUTER_SIZE(TME_BUS8_LOG2)] = {
  
  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
};

/* this sets the frequency on an isil7170: */
static void
_tme_isil7170_freq(struct tme_isil7170 *isil7170)
{
  tme_uint32_t clock_user, clock_basic;
  unsigned long hsec_usec, tsec_usec;

  /* get the user clock frequency: */
  switch (isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD] & TME_ISIL7170_CMD_FREQ_MASK) {
  case TME_ISIL7170_CMD_FREQ_4M:  clock_user = TME_ISIL7170_FREQ_4M; break;
  case TME_ISIL7170_CMD_FREQ_2M:  clock_user = TME_ISIL7170_FREQ_2M; break;
  case TME_ISIL7170_CMD_FREQ_1M:  clock_user = TME_ISIL7170_FREQ_1M; break;
  default:
  case TME_ISIL7170_CMD_FREQ_32K: clock_user = TME_ISIL7170_FREQ_32K; break;
  }

  /* get the hardware basic clock frequency: */
  clock_basic = isil7170->tme_isil7170_clock_basic;

  /* calculate the real-time durations, in microseconds, of a tenth
     and hundredth of a second, given the actual basic clock into the
     chip, and what the user claims is the basic clock into the chip.
     we have to be careful to avoid overflow here: */
  if (clock_user == clock_basic) {
    hsec_usec = 10000;
    tsec_usec = 100000;
  }
  else {
    hsec_usec = ((1000 * clock_user) / (clock_basic / 10));
    tsec_usec = ((1000 * clock_user) / (clock_basic / 100));
  }
  
  /* scale the result: */
  hsec_usec *= isil7170->tme_isil7170_clock_scale;
  tsec_usec *= isil7170->tme_isil7170_clock_scale;

  isil7170->tme_isil7170_clock_hsec_usec = hsec_usec;
  isil7170->tme_isil7170_clock_tsec_usec = tsec_usec;
}

/* this makes isil7170 callouts.  it must be called with the mutex held: */
static void
_tme_isil7170_callout(struct tme_isil7170 *isil7170)
{
  struct tme_bus_connection *conn_bus;
  int again;
  int int_asserted;
  int rc;

  /* if this function is already running in another thread, return
     now.  the other thread will do our work: */
  if (isil7170->tme_isil7170_callouts_running) {
    return;
  }

  /* callouts are now running: */
  isil7170->tme_isil7170_callouts_running = TRUE;

  /* get our bus connection: */
  conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					    isil7170->tme_isil7170_device.tme_bus_device_connection,
					    &isil7170->tme_isil7170_device.tme_bus_device_connection_rwlock);

  /* loop forever: */
  for (again = TRUE; again;) {
    again = FALSE;

    /* see if there are any pending, unmasked interrupts: */
    int_asserted = (isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT]
		    & isil7170->tme_isil7170_int_mask);

    /* update our interrupt-pending flag: */
    if (int_asserted) {
      isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT]
	= ((isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT]
	    & ~TME_ISIL7170_INT_PENDING)
	   | (int_asserted
	      ? TME_ISIL7170_INT_PENDING
	      : 0));
    }

    /* see if our interrupt signal should be asserted: */
    int_asserted
      = (int_asserted
	 && (isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD]
	     & TME_ISIL7170_CMD_INTENA));

    /* if our interrupt signal has changed: */
    if (!int_asserted != !isil7170->tme_isil7170_int_asserted) {

      /* unlock our mutex: */
      tme_mutex_unlock(&isil7170->tme_isil7170_mutex);

      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 isil7170->tme_isil7170_int_signal
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));

      /* lock our mutex: */
      tme_mutex_lock(&isil7170->tme_isil7170_mutex);
      
      /* if this call out succeeded, update the interrupt-asserted flag: */
      if (rc == TME_OK) {
	isil7170->tme_isil7170_int_asserted = int_asserted;
	again = TRUE;
      }
    }
  }

  /* there are no more callouts to make: */
  isil7170->tme_isil7170_callouts_running = FALSE;
}

/* this resets an isil7170: */
static void
_tme_isil7170_reset(struct tme_isil7170 *isil7170)
{

  /* clear the interrupt mask: */
  isil7170->tme_isil7170_int_mask = 0;

  /* clear the command register: */
  isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD] = 0;

  /* update the frequency: */
  _tme_isil7170_freq(isil7170);

  /* callout to update our interrupt signal: */
  _tme_isil7170_callout(isil7170);
}

/* the isil7170 timer thread: */
static void
_tme_isil7170_th_timer(struct tme_isil7170 *isil7170)
{
  tme_uint8_t int_mask;
  tme_uint32_t sleep_usec;
#ifdef TME_ISIL7170_TRACK_INT_RATE
  struct timeval now;
#endif /* TME_ISIL7170_TRACK_INT_RATE */

  /* lock the mutex: */
  tme_mutex_lock(&isil7170->tme_isil7170_mutex);

  /* loop forever: */
  for (;;) {

    /* if we were sleeping: */
    int_mask = isil7170->tme_isil7170_timer_sleeping;
    isil7170->tme_isil7170_timer_sleeping = 0;
    if (int_mask) {

#ifdef TME_ISIL7170_TRACK_INT_RATE

      /* if no interrupt is pending, and this interrupt is not masked,
	 we will deliver another interrupt: */
      if (!(isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT]
	    & TME_ISIL7170_INT_PENDING)
	  && (int_mask
	      & isil7170->tme_isil7170_int_mask)) {
	isil7170->tme_isil7170_int_sample++;
      }

      /* if the sample time has finished, report on the interrupt rate: */
      gettimeofday(&now, NULL);
      if (now.tv_sec > isil7170->tme_isil7170_int_sample_time.tv_sec
	  || (now.tv_sec == isil7170->tme_isil7170_int_sample_time.tv_sec
	      && now.tv_usec > isil7170->tme_isil7170_int_sample_time.tv_usec)) {
	if (isil7170->tme_isil7170_int_sample > 0) {
	  tme_log(TME_ISIL7170_LOG_HANDLE(isil7170),
		  0, TME_OK,
		  (TME_ISIL7170_LOG_HANDLE(isil7170),
		   "timer interrupt rate: %ld/sec",
		   (isil7170->tme_isil7170_int_sample
		    / (TME_ISIL7170_TRACK_INT_RATE
		       + (unsigned long) (now.tv_sec
					  - isil7170->tme_isil7170_int_sample_time.tv_sec)))));
	}

	/* reset the sample: */
	isil7170->tme_isil7170_int_sample_time.tv_sec = now.tv_sec + TME_ISIL7170_TRACK_INT_RATE;
	isil7170->tme_isil7170_int_sample_time.tv_usec = now.tv_usec;
	isil7170->tme_isil7170_int_sample = 0;
      }

#endif /* TME_ISIL7170_TRACK_INT_RATE */

      /* update the interrupt register: */
      isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT] |= int_mask;

      /* callout to update our interrupt signal: */
      _tme_isil7170_callout(isil7170);
    }

    /* get the interrupt mask: */
    int_mask = isil7170->tme_isil7170_int_mask;

    /* if the 1/100 second periodic interrupt is unmasked: */
    if (int_mask & TME_ISIL7170_INT_HSEC) {
      int_mask = TME_ISIL7170_INT_HSEC;
      sleep_usec = isil7170->tme_isil7170_clock_hsec_usec;
    }

    /* if the 1/10 second periodic interrupt is unmasked: */
    else if (int_mask & TME_ISIL7170_INT_TSEC) {
      int_mask = TME_ISIL7170_INT_TSEC;
      sleep_usec = isil7170->tme_isil7170_clock_tsec_usec;
    }

    /* otherwise, all periodic interrupts are masked.  wait until one
       of them is not: */
    else {
      tme_cond_wait_yield(&isil7170->tme_isil7170_cond_timer,
			  &isil7170->tme_isil7170_mutex);
      continue;
    }

    /* we are sleeping: */
    isil7170->tme_isil7170_timer_sleeping = int_mask;

    /* unlock our mutex: */
    tme_mutex_unlock(&isil7170->tme_isil7170_mutex);

    /* sleep: */
    tme_thread_sleep_yield(0, sleep_usec);

    /* lock our mutex: */
    tme_mutex_unlock(&isil7170->tme_isil7170_mutex);
  }
  /* NOTREACHED */
}

/* the isil7170 bus cycle handler: */
static int
_tme_isil7170_bus_cycle(void *_isil7170, struct tme_bus_cycle *cycle_init)
{
  struct tme_isil7170 *isil7170;
  tme_bus_addr32_t address, isil7170_address_last;
  tme_uint8_t buffer, value, value_old;
  struct tme_bus_cycle cycle_resp;
  unsigned int reg;
  struct timeval now;
  time_t _now;
  struct tm *now_tm, now_tm_buffer;

  /* recover our data structure: */
  isil7170 = (struct tme_isil7170 *) _isil7170;

  /* the requested cycle must be within range: */
  isil7170_address_last = isil7170->tme_isil7170_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= isil7170_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (isil7170_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* get the register being accessed: */
  address = cycle_init->tme_bus_cycle_address;
  reg = address >> isil7170->tme_isil7170_addr_shift;

  /* lock the mutex: */
  tme_mutex_lock(&isil7170->tme_isil7170_mutex);

  /* if the clock is running and this address is in the time-of-day
     registers, or if this is a write to the command register: */
  if (((isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD] & TME_ISIL7170_CMD_RUN)
       && reg <= TME_ISIL7170_REG_DOW)
      || (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	  && reg == TME_ISIL7170_REG_CMD)) {

    /* sample the time of day: */
    gettimeofday(&now, NULL);
    _now = now.tv_sec;
    now_tm = gmtime_r(&_now, &now_tm_buffer);

    /* put the time-of-day into the registers: */
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CSEC] = now.tv_usec / 10000;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_HOUR] = now_tm->tm_hour;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_MIN] = now_tm->tm_min;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_SEC] = now_tm->tm_sec;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_MON] = now_tm->tm_mon + 1;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_DAY] = now_tm->tm_mday;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_YEAR] = (1900 + now_tm->tm_year) - TME_ISIL7170_REG_YEAR_0;
    isil7170->tme_isil7170_regs[TME_ISIL7170_REG_DOW] = now_tm->tm_wday;
  }

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_isil7170_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_READ;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(isil7170->tme_isil7170_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
    value = buffer;
    
    /* log this write: */
    tme_log(TME_ISIL7170_LOG_HANDLE(isil7170), 100000, TME_OK,
	    (TME_ISIL7170_LOG_HANDLE(isil7170),
	     "reg %d write %02x",
	     reg, value));

    /* dispatch on the register: */
    switch (reg) {

    case TME_ISIL7170_REG_CSEC:
    case TME_ISIL7170_REG_HOUR:
    case TME_ISIL7170_REG_MIN:
    case TME_ISIL7170_REG_SEC:
    case TME_ISIL7170_REG_MON:
    case TME_ISIL7170_REG_DAY:
    case TME_ISIL7170_REG_YEAR:
    case TME_ISIL7170_REG_DOW:

      /* flag that the time-of-day needs to be updated: */
      isil7170->tme_isil7170_tod_update = TRUE;

      /* FALLTHROUGH */

    case TME_ISIL7170_REG_CMP_CSEC:
    case TME_ISIL7170_REG_CMP_HOUR:
    case TME_ISIL7170_REG_CMP_MIN:
    case TME_ISIL7170_REG_CMP_SEC:
    case TME_ISIL7170_REG_CMP_MON:
    case TME_ISIL7170_REG_CMP_DAY:
    case TME_ISIL7170_REG_CMP_YEAR:
    case TME_ISIL7170_REG_CMP_DOW:

      /* update the register: */
      isil7170->tme_isil7170_regs[reg] = value;
      break;

    case TME_ISIL7170_REG_INT:

      /* we don't support the alarm interrupt, or any of the daily,
	 hourly, etc., interrupts: */
      if (value & (TME_ISIL7170_INT_DAY
		   | TME_ISIL7170_INT_HOUR
		   | TME_ISIL7170_INT_MIN
		   | TME_ISIL7170_INT_SEC
		   | TME_ISIL7170_INT_ALARM)) {
	abort();
      }

      /* update the interrupt mask: */
      isil7170->tme_isil7170_int_mask = (value & ~TME_ISIL7170_INT_PENDING);

      /* callout to update our interrupt signal: */
      _tme_isil7170_callout(isil7170);

      /* notify the timer thread: */
      tme_cond_notify(&isil7170->tme_isil7170_cond_timer, FALSE);

      break;

    case TME_ISIL7170_REG_CMD:

      /* we don't support the test mode: */
      if (value & TME_ISIL7170_CMD_TEST) {
	abort();
      }

      /* update the command register: */
      value_old = isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD];
      isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD] = value;

      /* if the frequency changed, update our periodic intervals: */
      if ((value_old ^ value) & TME_ISIL7170_CMD_FREQ_MASK) {
	_tme_isil7170_freq(isil7170);
      }

      /* if the interrupt enable changed, callout to update our
         interrupt signal: */
      if ((value_old ^ value) & TME_ISIL7170_CMD_INTENA) {
	_tme_isil7170_callout(isil7170);
      }

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

    case TME_ISIL7170_REG_CSEC:
    case TME_ISIL7170_REG_HOUR:
    case TME_ISIL7170_REG_MIN:
    case TME_ISIL7170_REG_SEC:
    case TME_ISIL7170_REG_MON:
    case TME_ISIL7170_REG_DAY:
    case TME_ISIL7170_REG_YEAR:
    case TME_ISIL7170_REG_DOW:
    case TME_ISIL7170_REG_CMP_CSEC:
    case TME_ISIL7170_REG_CMP_HOUR:
    case TME_ISIL7170_REG_CMP_MIN:
    case TME_ISIL7170_REG_CMP_SEC:
    case TME_ISIL7170_REG_CMP_MON:
    case TME_ISIL7170_REG_CMP_DAY:
    case TME_ISIL7170_REG_CMP_YEAR:
    case TME_ISIL7170_REG_CMP_DOW:

      /* read the register: */
      value = isil7170->tme_isil7170_regs[reg];
      break;

    case TME_ISIL7170_REG_INT:

      /* reading the Interrupt register clears the interrupt: */
      value = isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT];
      isil7170->tme_isil7170_regs[TME_ISIL7170_REG_INT] = 0;

      /* callout to update our interrupt signal: */
      _tme_isil7170_callout(isil7170);

      break;

      /* the Command register, and all undefined registers, return
         garbage when read: */
    case TME_ISIL7170_REG_CMD:
    default:
      value = 0xff;
      break;
    }

    /* log this read: */
    tme_log(TME_ISIL7170_LOG_HANDLE(isil7170), 100000, TME_OK,
	    (TME_ISIL7170_LOG_HANDLE(isil7170),
	     "reg %d read %02x",
	     reg, value));

    /* run the bus cycle: */
    buffer = value;
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_isil7170_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_WRITE;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(isil7170->tme_isil7170_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
  }

  /* if the time-of-day registers have been updated, and the clock
     is running: */
  if (isil7170->tme_isil7170_tod_update
      && (isil7170->tme_isil7170_regs[TME_ISIL7170_REG_CMD]
	  & TME_ISIL7170_CMD_RUN)) {
    
    /* XXX update the host's time-of-day clock? */
    isil7170->tme_isil7170_tod_update = FALSE;
  }
    
  /* unlock the mutex: */
  tme_mutex_unlock(&isil7170->tme_isil7170_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the isil7170 TLB filler: */
static int
_tme_isil7170_tlb_fill(void *_isil7170, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_isil7170 *isil7170;
  tme_bus_addr32_t isil7170_address_last;

  /* recover our data structure: */
  isil7170 = (struct tme_isil7170 *) _isil7170;

  /* the address must be within range: */
  isil7170_address_last = isil7170->tme_isil7170_device.tme_bus_device_address_last;
  assert(address <= isil7170_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = isil7170_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = isil7170;
  tlb->tme_bus_tlb_cycle = _tme_isil7170_bus_cycle;

  return (TME_OK);
}

/* the new isil7170 element function: */
TME_ELEMENT_NEW_DECL(tme_ic_isil7170) {
  const struct tme_isil7170_socket *socket;
  struct tme_isil7170 *isil7170;
  struct tme_isil7170_socket socket_real;
  tme_bus_addr_t address_mask;
  unsigned long scale;
  int arg_i;
  int usage;

  /* dispatch on our socket version: */
  socket = (const struct tme_isil7170_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_isil7170_socket_version) {
  case TME_ISIL7170_SOCKET_0:
    socket_real = *socket;
    break;
  default: 
    tme_output_append_error(_output, _("socket type"));
    return (EOPNOTSUPP);
  }
    
  /* check our arguments: */
  usage = 0;
  scale = 1;
  arg_i = 1;
  for (;;) {

    /* a scale factor: */
    if (TME_ARG_IS(args[arg_i + 0], "scale")) {
      scale = tme_misc_unumber_parse(args[arg_i + 1], 0);
      if (scale == 0) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
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
			    "%s %s [ scale %s ]",
			    _("usage:"),
			    args[0],
			    _("SCALE"));
    return (EINVAL);
  }

  /* start the isil7170 structure: */
  isil7170 = tme_new0(struct tme_isil7170, 1);
  isil7170->tme_isil7170_socket = socket_real;
  isil7170->tme_isil7170_element = element;
  isil7170->tme_isil7170_clock_scale = scale;
  _tme_isil7170_reset(isil7170);

  /* figure our address mask, up to the nearest power of two: */
  address_mask = TME_ISIL7170_REGS_COUNT << isil7170->tme_isil7170_addr_shift;
  if (address_mask & (address_mask - 1)) {
    for (; address_mask & (address_mask - 1); address_mask &= (address_mask - 1));
    address_mask <<= 1;
  }
  address_mask -= 1;

  /* initialize our simple bus device descriptor: */
  isil7170->tme_isil7170_device.tme_bus_device_tlb_fill = _tme_isil7170_tlb_fill;
  isil7170->tme_isil7170_device.tme_bus_device_address_last = address_mask;

  /* start the timer thread: */
  tme_mutex_init(&isil7170->tme_isil7170_mutex);
  tme_cond_init(&isil7170->tme_isil7170_cond_reader);
  tme_thread_create((tme_thread_t) _tme_isil7170_th_timer, isil7170);

  /* fill the element: */
  element->tme_element_private = isil7170;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}
