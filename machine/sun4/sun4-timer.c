/* $Id: sun4-timer.c,v 1.3 2010/06/05 14:38:23 fredette Exp $ */

/* machine/sun4/sun4-timer.c - implementation of Sun 4 timer emulation: */

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
_TME_RCSID("$Id: sun4-timer.c,v 1.3 2010/06/05 14:38:23 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include "sun4-impl.h"

/* macros: */

/* real sun4/4c timer bits: */
#define TME_SUN4_32_TIMER_LIMIT		TME_BIT(31)
#define	TME_SUN44C_TIMER_MASK		(0x7ffffc00)
#define	TME_SUN4M_TIMER_MASK		(0x7ffffe00)

/* define this to track interrupt rates, reporting once every N
   seconds: */
#if 1
#define TME_SUN4_TIMER_TRACK_INT_RATE		(10)
#endif

/* this makes timer callouts.  it must be called with the mutex held: */
static void
_tme_sun4_timer_callout(struct tme_sun4 *sun4)
{
  struct tme_bus_connection *conn_bus;
  struct tme_sun4_timer *timer;
  unsigned int int_asserted;
  int again;
  int rc;

  /* if this function is already running in another thread, return
     now.  the other thread will do our work: */
  if (sun4->tme_sun4_timer_callouts_running) {
    return;
  }

  /* callouts are now running: */
  sun4->tme_sun4_timer_callouts_running = TRUE;

  /* get our bus connection: */
  conn_bus = sun4->tme_sun4_buses[TME_SUN4_32_CONN_REG_TIMER];

  /* loop forever: */
  for (again = TRUE; again;) {
    again = FALSE;

    /* check all of the timers for changes: */
    timer = &sun4->tme_sun4_timers[0];
    do {
      
      /* if this timer needs an interrupt callout: */
      int_asserted = (timer->tme_sun4_timer_counter & TME_SUN4_32_TIMER_LIMIT) != 0;
      if (!int_asserted
	  != !timer->tme_sun4_timer_int_asserted) {

	/* unlock our mutex: */
	tme_mutex_unlock(&sun4->tme_sun4_mutex);

	/* call out the bus interrupt signal edge: */
	rc = (*conn_bus->tme_bus_signal)
	  (conn_bus,
	   ((timer == &sun4->tme_sun4_timers[0]
	     ? TME_BUS_SIGNAL_INT(10)
	     : TME_BUS_SIGNAL_INT(14))
	    | (int_asserted
	       ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	       : TME_BUS_SIGNAL_LEVEL_NEGATED)));

	/* lock our mutex: */
	tme_mutex_lock(&sun4->tme_sun4_mutex);

	/* if this callout was successful, note the new state of the
	   interrupt signal: */
	if (rc == TME_OK) {
	  timer->tme_sun4_timer_int_asserted = int_asserted;
	  again = TRUE;
	}

	/* otherwise, abort: */
	else {
	  abort();
	}
      }
    } while (++timer != (&sun4->tme_sun4_timers[0] + TME_ARRAY_ELS(sun4->tme_sun4_timers)));

  }

  /* there are no more callouts to make: */
  sun4->tme_sun4_timer_callouts_running = FALSE;
}

/* this can be called to force an immediate timer interrupt: */
void
_tme_sun4_timer_int_force(struct tme_sun4 *sun4,
			  struct tme_sun4_timer *timer)
{

  /* lock our mutex: */
  tme_mutex_lock(&sun4->tme_sun4_mutex);

  /* force an immediate timer interrupt: */
  timer->tme_sun4_timer_counter = TME_SUN4_32_TIMER_LIMIT;
  timer->tme_sun4_timer_limit |= TME_SUN4_32_TIMER_LIMIT;

  /* call out any interrupts: */
  _tme_sun4_timer_callout(sun4);

  /* unlock our mutex: */
  tme_mutex_unlock(&sun4->tme_sun4_mutex);
}

/* the sun4 timer update function.  it must be called with the mutex
   locked: */
static void
_tme_sun4_timer_update(struct tme_sun4_timer *timer, struct timeval *now, struct timeval *sleep)
{

  /* get the current time: */
  gettimeofday(now, NULL);

#ifdef TME_SUN4_TIMER_TRACK_INT_RATE

  /* if the sample time has finished: */
  if (timer->tme_sun4_timer_track_sample.tv_sec < now->tv_sec
      || (timer->tme_sun4_timer_track_sample.tv_sec == now->tv_sec
	  && timer->tme_sun4_timer_track_sample.tv_usec <= now->tv_usec)) {

    /* if the timer has made any interrupts during the sample time: */
    if (timer->tme_sun4_timer_track_ints > 0) {

      /* log the interrupt rate: */
      tme_log(TME_SUN4_LOG_HANDLE(timer->tme_sun4_timer_sun4),
	      0, TME_OK,
	      (TME_SUN4_LOG_HANDLE(timer->tme_sun4_timer_sun4),
	       "level %d timer interrupt rate: %ld/sec",
	       (timer == &timer->tme_sun4_timer_sun4->tme_sun4_timers[0]
		? 10
		: 14),
	       (timer->tme_sun4_timer_track_ints
		/ (unsigned long) (now->tv_sec
				   - (timer->tme_sun4_timer_track_sample.tv_sec
				      - TME_SUN4_TIMER_TRACK_INT_RATE)))));
    }

    /* reset the sampling: */
    timer->tme_sun4_timer_track_ints = 0;
    timer->tme_sun4_timer_track_sample = *now;
    timer->tme_sun4_timer_track_sample.tv_sec += TME_SUN4_TIMER_TRACK_INT_RATE;
  }

#endif /* TME_SUN4_TIMER_TRACK_INT_RATE */

  /* if this timer has not reached its next limit: */
  if (__tme_predict_false(timer->tme_sun4_timer_limit_next.tv_sec > now->tv_sec
			  || (timer->tme_sun4_timer_limit_next.tv_sec == now->tv_sec
			      && timer->tme_sun4_timer_limit_next.tv_usec > now->tv_usec))) {

    /* sleep until this timer reaches its next limit: */
    sleep->tv_sec = timer->tme_sun4_timer_limit_next.tv_sec - now->tv_sec;
    sleep->tv_usec = timer->tme_sun4_timer_limit_next.tv_usec - now->tv_usec;
    if (timer->tme_sun4_timer_limit_next.tv_usec < now->tv_usec) {
      sleep->tv_sec--;
      sleep->tv_usec += 1000000;
    }
    return;
  }

  /* set this timer's next limit time: */
  do {
    timer->tme_sun4_timer_limit_next.tv_sec += timer->tme_sun4_timer_period.tv_sec;
    timer->tme_sun4_timer_limit_next.tv_usec += timer->tme_sun4_timer_period.tv_usec;
    if (__tme_predict_false(timer->tme_sun4_timer_limit_next.tv_usec >= 1000000)) {
      timer->tme_sun4_timer_limit_next.tv_usec -= 1000000;
      timer->tme_sun4_timer_limit_next.tv_sec += 1;
    }
  } while (timer->tme_sun4_timer_limit_next.tv_sec < now->tv_sec
	   || (timer->tme_sun4_timer_limit_next.tv_sec == now->tv_sec
	       && timer->tme_sun4_timer_limit_next.tv_usec <= now->tv_usec));

  /* mark this timer as having reached its limit: */
#ifdef TME_SUN4_TIMER_TRACK_INT_RATE
  if (!(timer->tme_sun4_timer_counter
	& TME_SUN4_32_TIMER_LIMIT)) {
    timer->tme_sun4_timer_track_ints++;
  }
#endif /* TME_SUN4_TIMER_TRACK_INT_RATE */
  timer->tme_sun4_timer_counter = TME_SUN4_32_TIMER_LIMIT;
  timer->tme_sun4_timer_limit |= TME_SUN4_32_TIMER_LIMIT;

  /* sleep for the normal period: */
  *sleep = timer->tme_sun4_timer_period;
}

/* this resets a timer: */
static void
_tme_sun4_timer_reset(struct tme_sun4_timer *timer)
{
  tme_uint32_t counter_one;
  tme_uint32_t ticks;
  tme_uint32_t ticks_max;
  tme_uint32_t usecs;

  /* to keep things simpler, we always use the sun4m 500ns tick: */
  counter_one = TME_SUN4_IS_SUN44C(timer->tme_sun4_timer_sun4) ? 2 : 1;
  ticks_max = (TME_SUN4M_TIMER_MASK / _TME_FIELD_MASK_FACTOR(TME_SUN4M_TIMER_MASK)) + 1;

  /* get this timer's period, in 500ns ticks.  NB that we account for
     the fact that timers count from [1..limit), and not [0..limit): */
  /* XXX FIXME - we assume that the limit value of one gives the
     longest possible period.  is this right? */
  ticks = TME_FIELD_MASK_EXTRACTU(timer->tme_sun4_timer_limit, TME_SUN4M_TIMER_MASK);
  ticks = (ticks - counter_one) & (ticks_max - counter_one);
  if (__tme_predict_false(ticks == 0)) {
    ticks = ticks_max;
  }

  /* convert the timer's period from 500ns ticks to a struct timeval
     and save it: */
  usecs = ticks / 2;
  timer->tme_sun4_timer_period.tv_sec = 0;
  if (__tme_predict_false(usecs >= 1000000)) {
    timer->tme_sun4_timer_period.tv_sec = usecs / 1000000;
    usecs %= 1000000;
  }
  timer->tme_sun4_timer_period.tv_usec = usecs;

  /* set the next limit time for this timer: */
  gettimeofday(&timer->tme_sun4_timer_limit_next, NULL);
  timer->tme_sun4_timer_limit_next.tv_sec += timer->tme_sun4_timer_period.tv_sec;
  timer->tme_sun4_timer_limit_next.tv_usec += timer->tme_sun4_timer_period.tv_usec;
  if (timer->tme_sun4_timer_limit_next.tv_usec >= 1000000) {
    timer->tme_sun4_timer_limit_next.tv_usec -= 1000000;
    timer->tme_sun4_timer_limit_next.tv_sec += 1;
  }
}

/* the sun4 timer thread: */
static void
_tme_sun4_timer_th(struct tme_sun4_timer *timer)
{
  struct tme_sun4 *sun4;
  struct timeval now;
  struct timeval sleep;

  /* recover our sun4: */
  sun4 = timer->tme_sun4_timer_sun4;

  /* lock our mutex: */
  tme_mutex_lock(&sun4->tme_sun4_mutex);

  /* loop forever: */
  for (;;) {

    /* update this timer: */
    _tme_sun4_timer_update(timer, &now, &sleep);

    /* call out any interrupts: */
    _tme_sun4_timer_callout(sun4);

    /* sleep, but wake up if our timer configuration changes: */
    tme_cond_sleep_yield(&timer->tme_sun4_timer_cond,
			 &sun4->tme_sun4_mutex,
			 &sleep);
  }
  /* NOTREACHED */
}

/* the sun4 timer control bus cycle handler: */
int
_tme_sun4_timer_cycle_control(void *_sun4, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun4 *sun4;
  unsigned int timer_i;
  struct tme_sun4_timer *timer;
  tme_uint32_t reg;
  tme_uint32_t value32;
  struct tme_bus_cycle cycle_resp;
  struct timeval now;
  struct timeval last_reset;
  tme_uint32_t counter_one;
  tme_uint32_t usecs;
  tme_uint32_t ticks;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) _sun4;

  /* this must be a full 32-bit register access: */
  if ((cycle_init->tme_bus_cycle_address % sizeof(tme_uint32_t)) != 0
      || cycle_init->tme_bus_cycle_size != sizeof(tme_uint32_t)) {
    abort();
  }

  /* get the timer and register accessed: */
  if (TME_SUN4_IS_SUN44C(sun4)) {
    timer_i = cycle_init->tme_bus_cycle_address / TME_SUN44C_TIMER_SIZ_REG;
    reg = cycle_init->tme_bus_cycle_address & TME_SUN4_32_TIMER_SIZ_COUNTER;
  }
  else {
    abort();
  }
  timer = &sun4->tme_sun4_timers[timer_i];

  /* lock our mutex: */
  tme_mutex_lock(&sun4->tme_sun4_mutex);

  /* if this is a read: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {

    /* dispatch on the register: */
    switch (reg) {
    default: assert(FALSE);
    case TME_SUN4_32_TIMER_REG_COUNTER:

      /* update the timers: */
      _tme_sun4_timer_update(timer, &now, &last_reset);

      /* get the time of the last reset: */
      last_reset = timer->tme_sun4_timer_limit_next;
      if (last_reset.tv_usec < timer->tme_sun4_timer_period.tv_usec) {
	last_reset.tv_sec -= 1;
	last_reset.tv_usec += 1000000;
      }
      last_reset.tv_sec -= timer->tme_sun4_timer_period.tv_sec;
      last_reset.tv_usec -= timer->tme_sun4_timer_period.tv_usec;

      /* get the number of microseconds since the last reset: */
      usecs = now.tv_sec - last_reset.tv_sec;
      usecs *= 1000000;
      usecs
	+= (((tme_int32_t) now.tv_usec)
	    - ((tme_int32_t) last_reset.tv_usec));

      /* to keep things simpler, we always use the sun4m 500ns tick: */
      counter_one = TME_SUN4_IS_SUN44C(sun4) ? 2 : 1;

      /* convert the number of microseconds until this timer resets again,
	 to the 500ns tick counter value for the timer.  NB that we
	 account for the fact that timers count from [1..limit), and not
	 [0..limit): */
      ticks = (usecs * 2) + counter_one;
      TME_FIELD_MASK_DEPOSITU(timer->tme_sun4_timer_counter, 
			      TME_SUN4M_TIMER_MASK,
			      ticks);

      /* read this timer's counter register: */
      value32 = timer->tme_sun4_timer_counter;
      break;

    case TME_SUN4_32_TIMER_REG_LIMIT:

      /* read this timer's limit register: */
      value32 = timer->tme_sun4_timer_limit;	

      /* a read of the limit register is used to acknowledge an
	 interrupt, which probably means clearing the limit bit on the
	 counter register: */
      timer->tme_sun4_timer_counter = 0;
      timer->tme_sun4_timer_limit &= ~TME_SUN4_32_TIMER_LIMIT;
      break;
    }

    tme_log(TME_SUN4_LOG_HANDLE(sun4), 2000, TME_OK,
	    (TME_SUN4_LOG_HANDLE(sun4),
	     _("timer #%d %s -> 0x%08x"),
	     timer_i,
	     (reg == TME_SUN4_32_TIMER_REG_COUNTER
	      ? "counter"
	      : "limit"),
	     value32));

    /* byteswap the register value: */
    value32 = tme_htobe_u32(value32);
  }

  /* run the bus cycle: */
  cycle_resp.tme_bus_cycle_buffer = (tme_uint8_t *) &value32;
  cycle_resp.tme_bus_cycle_buffer_increment = 1;
  cycle_resp.tme_bus_cycle_lane_routing = cycle_init->tme_bus_cycle_lane_routing;
  cycle_resp.tme_bus_cycle_address = 0;
  cycle_resp.tme_bus_cycle_type = (cycle_init->tme_bus_cycle_type
				   ^ (TME_BUS_CYCLE_WRITE
				      | TME_BUS_CYCLE_READ));
  cycle_resp.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS32_LOG2);
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* byteswap the register value: */
    value32 = tme_htobe_u32(value32);

    tme_log(TME_SUN4_LOG_HANDLE(sun4), 2000, TME_OK,
	    (TME_SUN4_LOG_HANDLE(sun4),
	     _("timer #%d %s <- 0x%08x"),
	     timer_i,
	     (reg == TME_SUN4_32_TIMER_REG_COUNTER
	      ? "counter"
	      : "limit"),
	     value32));

    /* dispatch on the register: */
    switch (reg) {
    default: assert(FALSE);
    case TME_SUN4_32_TIMER_REG_COUNTER:
      abort();

    case TME_SUN4_32_TIMER_REG_LIMIT:

      /* write the timer's limit register: */
      timer->tme_sun4_timer_limit = value32;

      /* reset this timer: */
      _tme_sun4_timer_reset(timer);

      /* wake up the thread for this timer: */
      tme_cond_notify(&timer->tme_sun4_timer_cond, FALSE);
      break;
    }
  }

  /* make any callouts: */
  _tme_sun4_timer_callout(sun4);
    
  /* unlock the mutex: */
  tme_mutex_unlock(&sun4->tme_sun4_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this creates the sun4 timers: */
void
_tme_sun4_timer_new(struct tme_sun4 *sun4)
{
  struct tme_sun4_timer *timer;

  /* loop over the timers: */
  timer = &sun4->tme_sun4_timers[0];
  do {

    /* initialize and reset the timer: */
    timer->tme_sun4_timer_sun4 = sun4;
    tme_cond_init(&timer->tme_sun4_timer_cond);
    _tme_sun4_timer_reset(timer);

    /* start the thread for this timer: */
    tme_thread_create((tme_thread_t) _tme_sun4_timer_th, timer);

  } while (++timer != (&sun4->tme_sun4_timers[0] + TME_ARRAY_ELS(sun4->tme_sun4_timers)));
}
