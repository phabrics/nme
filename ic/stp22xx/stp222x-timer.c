/* $Id: stp222x-timer.c,v 1.3 2010/06/05 14:38:00 fredette Exp $ */

/* ic/stp22xx/stp222x-timer.c - emulation of the timers of the UPA to
   SBus interface controller (STP2220) and the UPA to PCI interface
   controller (STP2222): */

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
_TME_RCSID("$Id: stp222x-timer.c,v 1.3 2010/06/05 14:38:00 fredette Exp $");

/* includes: */
#include "stp222x-impl.h"

/* macros: */

/* the counter register: */
#define TME_STP222X_TIMER_COUNT_COUNT		((2 << 28) - (1 << 0))

/* the limit register: */
#define TME_STP222X_TIMER_LIMIT_INT_EN		(((tme_uint32_t) 1) << 31)
#define TME_STP222X_TIMER_LIMIT_RELOAD		TME_BIT(30)
#define TME_STP222X_TIMER_LIMIT_PERIODIC	TME_BIT(29)
#define TME_STP222X_TIMER_LIMIT_LIMIT		TME_STP222X_TIMER_COUNT_COUNT

/* define this to track interrupt rates, reporting once every N
   seconds: */
#if 1
#define TME_STP222X_TIMER_TRACK_INT_RATE		TME_TIME_SET_SEC(10)
#endif

/* this updates a timer: */
static tme_time_t
_tme_stp222x_timer_update(struct tme_stp222x_timer *timer)
{
  tme_time_t now;

  /* get the current time: */
  now = tme_thread_get_time();

#ifdef TME_STP222X_TIMER_TRACK_INT_RATE

  /* if the sample time has finished: */
  if (now > timer->tme_stp222x_timer_track_sample) {
    /* if the timer has made any interrupts during the sample time: */
    if (timer->tme_stp222x_timer_track_ints > 0) {

      /* log the interrupt rate: */
      tme_log(TME_STP222X_LOG_HANDLE(timer->tme_stp222x_timer_stp222x),
	      0, TME_OK,
	      (TME_STP222X_LOG_HANDLE(timer->tme_stp222x_timer_stp222x),
	       "timer %d timer interrupt rate: %llu/sec",
	       (timer == &timer->tme_stp222x_timer_stp222x->tme_stp222x_timers[1]),
	       (TME_TIME_SET_SEC(timer->tme_stp222x_timer_track_ints)
		/ (now - (timer->tme_stp222x_timer_track_sample - TME_STP222X_TIMER_TRACK_INT_RATE)))));
    }
    
    /* reset the sampling: */
    timer->tme_stp222x_timer_track_ints = 0;
    timer->tme_stp222x_timer_track_sample = now;
    timer->tme_stp222x_timer_track_sample += TME_STP222X_TIMER_TRACK_INT_RATE;
  }
  
#endif /* TME_STP222X_TIMER_TRACK_INT_RATE */
  
  /* if this timer has reached its next limit: */
  if (__tme_predict_true(now > timer->tme_stp222x_timer_limit_next)) {
    /* if this timer is not in periodic mode: */
    if ((timer->tme_stp222x_timer_limit & TME_STP222X_TIMER_LIMIT_PERIODIC) == 0) {
      
      /* this timer's period is now the maximum: */
      timer->tme_stp222x_timer_period = TME_TIME_SET_USEC(TME_STP222X_TIMER_COUNT_COUNT + 1);
    }
    
    /* set this timer's next limit time: */
    do {
      timer->tme_stp222x_timer_limit_next += timer->tme_stp222x_timer_period;
    } while (now > timer->tme_stp222x_timer_limit_next);
    
    /* if this timer's interrupt is enabled: */
    if (timer->tme_stp222x_timer_limit & TME_STP222X_TIMER_LIMIT_INT_EN) {
      
      /* receive an interrupt: */
#ifdef TME_STP222X_TIMER_TRACK_INT_RATE
      timer->tme_stp222x_timer_track_ints++;
#endif /* TME_STP222X_TIMER_TRACK_INT_RATE */
      tme_stp222x_mdu_receive(timer->tme_stp222x_timer_stp222x,
			      timer->tme_stp222x_timer_idi);
    }
  }
  
  /* sleep until this timer reaches its next limit: */
  return timer->tme_stp222x_timer_limit_next - now;
}

/* the stp222x timer thread: */
static _tme_thret
_tme_stp222x_timer_th(void *_timer)
{
  struct tme_stp222x_timer *timer;
  struct tme_stp222x *stp222x;
  tme_time_t sleep;

  /* recover our data structures: */
  timer = (struct tme_stp222x_timer *) _timer;
  stp222x = timer->tme_stp222x_timer_stp222x;

  /* enter: */
  tme_thread_enter(&timer->tme_stp222x_timer_mutex);

  tme_stp22xx_enter(&stp222x->tme_stp222x);

  /* loop forever: */
  for (;;) {

    /* update this timer: */
    sleep = _tme_stp222x_timer_update(timer);
    
    /* sleep, but wake up if our timer configuration changes: */
    tme_stp22xx_cond_sleep_yield(&stp222x->tme_stp222x,
				 &timer->tme_stp222x_timer_cond,
				 &sleep);
  }
  /* NOTREACHED */
  tme_thread_exit(&timer->tme_stp222x_timer_mutex);
}

/* this resets a timer: */
static void
_tme_stp222x_timer_reset(struct tme_stp222x_timer *timer,
			 tme_uint32_t count)
{
  tme_uint32_t limit;
  tme_uint32_t period;

  /* get the timer's initial period, until the count register next
     reaches the limit.  NB that we assume that if count and limit are
     already equal now, that does not count as reaching the limit, and
     that a complete timer period is needed: */
  limit = TME_FIELD_MASK_EXTRACTU(timer->tme_stp222x_timer_limit, TME_STP222X_TIMER_LIMIT_LIMIT);
  period = ((limit - (count + 1)) & TME_STP222X_TIMER_COUNT_COUNT) + 1;

  /* save this timer's initial period: */
  timer->tme_stp222x_timer_period = TME_TIME_SET_USEC(period);

  /* set the next limit time for this timer: */
  timer->tme_stp222x_timer_limit_next = tme_thread_get_time();
  timer->tme_stp222x_timer_limit_next += timer->tme_stp222x_timer_period;
}

/* this reads a timer's count register: */
static tme_uint32_t
_tme_stp222x_timer_count(struct tme_stp222x_timer *timer)
{
  tme_time_t sleep;
  tme_uint32_t count;

  /* update the timers: */
  sleep = _tme_stp222x_timer_update(timer);

  /* get the absolute count until the next limit: */
  count = sleep;

  /* the count register value is that distance to the limit
     register: */
  return ((timer->tme_stp222x_timer_limit
	   - count)
	  & TME_STP222X_TIMER_COUNT_COUNT);
}

/* the timer register handler: */
void
tme_stp222x_timer_regs(struct tme_stp222x *stp222x,
		       struct tme_stp222x_reg *reg)
{
  unsigned int timer_i;
  struct tme_stp222x_timer *timer;
  tme_uint32_t reg_address;
  tme_uint32_t count;
  tme_uint32_t limit;

  /* get the timer and register accessed: */
  reg_address = reg->tme_stp222x_reg_address;
  timer_i
    = ((reg_address
	/ TME_STP222X_TIMER_SIZE)
       % TME_ARRAY_ELS(stp222x->tme_stp222x_timers));
  timer = &stp222x->tme_stp222x_timers[timer_i];
  reg_address %= TME_STP222X_TIMER_SIZE;

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("timer #%d %s <- 0x%08" TME_PRIx64),
	     timer_i,
	     (reg_address == TME_STP222X_TIMER_COUNT
	      ? "count"
	      : "limit"),
	     reg->tme_stp222x_reg_value));

    /* dispatch on the register: */
    switch (reg_address) {
    case TME_STP222X_TIMER_COUNT:

      /* get the new count register value: */
      count
	= (((tme_uint32_t) reg->tme_stp222x_reg_value)
	   & TME_STP222X_TIMER_COUNT_COUNT);
      break;

    case TME_STP222X_TIMER_LIMIT:

      /* get the new limit register value: */
      limit
	= (((tme_uint32_t) reg->tme_stp222x_reg_value)
	   & (TME_STP222X_TIMER_LIMIT_LIMIT
	      | TME_STP222X_TIMER_LIMIT_INT_EN
	      | TME_STP222X_TIMER_LIMIT_RELOAD
	      | TME_STP222X_TIMER_LIMIT_PERIODIC));

      /* get the new count register value: */
      count
	= ((limit & TME_STP222X_TIMER_LIMIT_RELOAD)
	   ? 0
	   :  _tme_stp222x_timer_count(timer));

      /* write the timer's limit register: */
      timer->tme_stp222x_timer_limit = limit & ~TME_STP222X_TIMER_LIMIT_RELOAD;
      break;

    default: return;
    }

    /* reset this timer: */
    _tme_stp222x_timer_reset(timer, count);

    /* wake up the thread for this timer: */
    tme_stp22xx_cond_notify(&timer->tme_stp222x_timer_cond);
  }

  /* otherwise, this must be a read: */
  else {

    /* dispatch on the register: */
    switch (reg_address) {
    case TME_STP222X_TIMER_COUNT:

      /* read this timer's count register: */
      reg->tme_stp222x_reg_value = _tme_stp222x_timer_count(timer);
      break;

    case TME_STP222X_TIMER_LIMIT:

      /* read this timer's limit register: */
      reg->tme_stp222x_reg_value = timer->tme_stp222x_timer_limit;	
      break;

    default: return;
    }

    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("timer #%d %s -> 0x%08" TME_PRIx64),
	     timer_i,
	     (reg_address == TME_STP222X_TIMER_COUNT
	      ? "count"
	      : "limit"),
	     reg->tme_stp222x_reg_value));
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* this initializes an stp222x timer: */
void
tme_stp222x_timer_init(struct tme_stp222x *stp222x,
		       struct tme_stp222x_timer *timer)
{
  struct tme_element *element = stp222x->tme_stp222x.tme_stp22xx_element;

  /* initialize and reset the timer: */
  timer->tme_stp222x_timer_stp222x = stp222x;
  tme_stp22xx_cond_init(&timer->tme_stp222x_timer_cond);
  tme_mutex_init(&timer->tme_stp222x_timer_mutex);
  _tme_stp222x_timer_reset(timer, 0);

  /* start the thread for this timer: */
  tme_thread_create(&timer->tme_stp222x_timer_thread, _tme_stp222x_timer_th, timer);

  /* set the IDI for this timer: */
  timer->tme_stp222x_timer_idi
    = (TME_STP222X_IS_2220(stp222x)
       ? TME_STP2220_IDI_TIMER(timer
			       == &timer->tme_stp222x_timer_stp222x->tme_stp222x_timers[1])
       : TME_STP2222_IDI_TIMER(timer
			       == &timer->tme_stp222x_timer_stp222x->tme_stp222x_timers[1]));;
}
