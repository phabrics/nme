/* $Id: stp22xx-impl.h,v 1.2 2009/08/29 21:17:06 fredette Exp $ */

/* ic/stp22xx/stp22xx-impl.h - implementation header file for STP2200,
   STP2202, STP2220, and STP2222 emulation: */

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

#ifndef _STP22XX_IMPL_H
#define _STP22XX_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: stp22xx-impl.h,v 1.2 2009/08/29 21:17:06 fredette Exp $");

/* includes: */
#include <tme/completion.h>
#include <tme/bus/upa.h>

/* macros: */

/* this macro wraps code used during the transition to the new bus
   interface.  the new bus interface will remove everything from
   struct tme_bus_tlb starting with the tme_bus_tlb_cycles_ok member
   until the end.  this will remove all slow cycle information from a
   TLB, so TLB filling will only give fast read and write information.

   every bus connection will then have a slow cycle function.  when a
   master needs to do a slow cycle, through its bus connection it will
   arbitrate for the bus and then start its cycle.  the bus
   implementation will steer the cycle towards the slave, which may be
   across a bridge and over another bus, etc.  slow bus cycles will
   then follow the actual path from master to slave, with bus
   implementations ordering bus access and cycles as needed.

   this seems better than the old interface, where masters called
   directly into slaves through function pointers in a TLB.  in the
   old interface, buses couldn't enforce any ordering, because they
   weren't involved.  this created the risk that a master in the
   middle of running a bus cycle could be called as a slave over the
   same bus connection.  real devices never have to face that, and
   without eliminating that here, turning on preemptive threading
   seems impossible.

   the new cycle function in a bus connection will take the bus
   connection, the struct tme_bus_cycle *, a struct tme_completion *,
   and a pointer to a mask of cycle types that can be done fast.  as
   the slow bus cycle makes its way from master to slave, if any point
   along that path can't allow fast reads or writes, those bits are
   cleared from the mask.  when the slow cycle completes, if the
   master finds that any cycles can be done fast again, it may choose
   to fill a TLB entry again to get the new fast cycle information.
   this frees slaves from having to track filled TLBs that weren't
   filled with any fast cycle information, just so they can be
   invalidated when addresses become fast-capable to make sure that
   masters refill them.

   eventually, all of the code wrapped by this macro will be
   removed: */
#define TME_STP22XX_BUS_TRANSITION		(TRUE)

/* the maximum number of completions: */
#define TME_STP22XX_COMPLETIONS_MAX		(2)

/* the maximum number of delayed completions: */
#define TME_STP22XX_COMPLETIONS_DELAYED_MAX	(2)

/* condition states: */
#define TME_STP22XX_COND_STATE_IDLE		(0)
#define TME_STP22XX_COND_STATE_RUNNING		(1)
#define TME_STP22XX_COND_STATE_WAITING		(2)
#define TME_STP22XX_COND_STATE_NOTIFIED		(3)

/* types: */
struct tme_stp22xx;

/* a completion handler: */
typedef void (*_tme_stp22xx_completion_handler_t) _TME_P((struct tme_stp22xx *, struct tme_completion *, void *));

/* a condition: */
struct tme_stp22xx_cond {
  int tme_stp22xx_cond_state;
  tme_cond_t tme_stp22xx_cond_cond;
};

/* a connection: */
union tme_stp22xx_conn {
  struct tme_bus_connection *tme_stp22xx_conn_bus;
  struct tme_upa_bus_connection *tme_stp22xx_conn_upa;
};

/* the device: */
struct tme_stp22xx {
  
  /* backpointer to the device's element: */
  struct tme_element *tme_stp22xx_element;

  /* the mutex protecting the device: */
  tme_mutex_t tme_stp22xx_mutex;

  /* the size of the part-specific structure: */
  unsigned long tme_stp22xx_sizeof;

  /* this is nonzero if the run function is running: */
  int tme_stp22xx_running;

  /* the run function: */
  void (*tme_stp22xx_run) _TME_P((struct tme_stp22xx *));

  /* our completions: */
  struct tme_completion tme_stp22xx_completions[TME_STP22XX_COMPLETIONS_MAX];
  _tme_stp22xx_completion_handler_t tme_stp22xx_completion_handlers[TME_STP22XX_COMPLETIONS_MAX];
  void * tme_stp22xx_completion_args[TME_STP22XX_COMPLETIONS_MAX];

  /* any delayed completions: */
  /* NB: this array is always NULL-terminated: */
  struct tme_completion *tme_stp22xx_completions_delayed[TME_STP22XX_COMPLETIONS_DELAYED_MAX + 1];

  /* the undefined connection index: */
  tme_uint32_t tme_stp22xx_conn_index_null;

  /* any pending master connection index: */
  tme_uint32_t tme_stp22xx_master_conn_index_pending;

  /* any current master connection index and its completion: */
  tme_uint32_t tme_stp22xx_master_conn_index;
  struct tme_completion **tme_stp22xx_master_completion;

#if TME_STP22XX_BUS_TRANSITION
  /* the token for filling TLB entries for slave cycles: */
  struct tme_token tme_stp22xx_slave_cycle_tlb_token;
#endif /* TME_STP22XX_BUS_TRANSITION */

  /* any current slave connection: */
  struct tme_bus_connection *tme_stp22xx_slave_conn_bus;

  /* the connections: */
  /* NB: this must be the last member of this structure; the
     part-specific structure allocates enough space for its real
     size: */
  union tme_stp22xx_conn tme_stp22xx_conns[1];
};

/* prototypes: */

/* this busies a generic bus connection: */
struct tme_bus_connection *tme_stp22xx_busy_bus _TME_P((struct tme_stp22xx *, tme_uint32_t));

/* this unbusies a generic bus connection: */
void tme_stp22xx_unbusy_bus _TME_P((struct tme_stp22xx *, struct tme_bus_connection *));

/* this busies a slave generic bus connection: */
struct tme_bus_connection *tme_stp22xx_slave_busy_bus _TME_P((struct tme_stp22xx *, tme_uint32_t));

/* this unbusies a slave generic bus connection: */
void tme_stp22xx_slave_unbusy _TME_P((struct tme_stp22xx *));

/* this enters: */
struct tme_stp22xx *tme_stp22xx_enter _TME_P((struct tme_stp22xx *));

/* this enters as the bus master: */
struct tme_stp22xx *tme_stp22xx_enter_master _TME_P((struct tme_bus_connection *));

/* this leaves: */
void tme_stp22xx_leave _TME_P((struct tme_stp22xx *));

/* this waits on a condition, with an optional sleep time: */
void tme_stp22xx_cond_sleep_yield _TME_P((struct tme_stp22xx *, struct tme_stp22xx_cond *, const struct timeval *));

/* this validates a completion: */
void tme_stp22xx_completion_validate _TME_P((struct tme_stp22xx *, struct tme_completion *));

/* this allocates a completion: */
struct tme_completion *tme_stp22xx_completion_alloc _TME_P((struct tme_stp22xx *, _tme_stp22xx_completion_handler_t, void *));

/* this calls out a bus signal to a connection: */
void tme_stp22xx_callout_signal _TME_P((struct tme_stp22xx *, tme_uint32_t, unsigned int, _tme_stp22xx_completion_handler_t));

/* this completes a bus grant: */
void tme_stp22xx_complete_bg _TME_P((struct tme_stp22xx *, struct tme_completion *, void *));

/* this completes a bus operation between master and slave: */
void tme_stp22xx_complete_master _TME_P((struct tme_stp22xx *, struct tme_completion *, void *));

/* this is a no-op completion: */
void tme_stp22xx_complete_nop _TME_P((struct tme_stp22xx *, struct tme_completion *, void *));

/* this runs a slave bus cycle: */
void tme_stp22xx_slave_cycle _TME_P((struct tme_bus_connection *, tme_uint32_t, struct tme_bus_cycle *,	tme_uint32_t *,	struct tme_completion **));

/* this fills a TLB entry: */
void tme_stp22xx_tlb_fill _TME_P((struct tme_bus_connection *, struct tme_bus_tlb *, tme_uint32_t, tme_bus_addr64_t, unsigned int));

/* this adds a TLB set: */
void tme_stp22xx_tlb_set_add _TME_P((struct tme_bus_connection *, struct tme_bus_tlb_set_info *, struct tme_completion *));

#if TME_STP22XX_BUS_TRANSITION

/* this is the bus TLB set add transition glue: */
int tme_stp22xx_tlb_set_add_transition _TME_P((struct tme_bus_connection *, struct tme_bus_tlb_set_info *));
#define tme_stp22xx_tlb_set_add tme_stp22xx_tlb_set_add_transition

#endif /* TME_STP22XX_BUS_TRANSITION */

/* this notifies a condition: */
void tme_stp22xx_cond_notify _TME_P((struct tme_stp22xx_cond *));

/* this initializes a condition: */
void tme_stp22xx_cond_init _TME_P((struct tme_stp22xx_cond *));

/* this initializes an stp22xx: */
void tme_stp22xx_init _TME_P((struct tme_stp22xx *, unsigned long, tme_uint32_t));

#endif /* !_STP22XX_IMPL_H */
