/* $Id: stp22xx.c,v 1.4 2009/09/07 15:03:19 fredette Exp $ */

/* ic/stp22xx.c - common STP2200, STP2202, STP2220, and STP2222
   emulation: */

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
_TME_RCSID("$Id: stp22xx.c,v 1.4 2009/09/07 15:03:19 fredette Exp $");

/* includes: */
#include "stp22xx-impl.h"

/* this busies a generic bus connection: */
struct tme_bus_connection *
tme_stp22xx_busy_bus(struct tme_stp22xx *stp22xx,
		     tme_uint32_t conn_index)
{
  struct tme_bus_connection *conn_bus;

  /* if the connection index is valid: */
  conn_bus = NULL;
  if (__tme_predict_true(conn_index != stp22xx->tme_stp22xx_conn_index_null)) {

    /* if there is a connection at this index: */
    conn_bus = stp22xx->tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus;
    if (__tme_predict_true(conn_bus != NULL)) {

    }
  }

  return (conn_bus);
}

/* this unbusies a generic bus connection: */
void
tme_stp22xx_unbusy_bus(struct tme_stp22xx *stp22xx,
		       struct tme_bus_connection *conn_bus)
{
  
  /* the connection must be valid: */
  assert (conn_bus != NULL);
}

/* this busies a slave generic bus connection: */
struct tme_bus_connection *
tme_stp22xx_slave_busy_bus(struct tme_stp22xx *stp22xx,
			   tme_uint32_t slave_conn_index)
{
  struct tme_bus_connection *slave_conn_bus;

  /* if the connection index is valid: */
  slave_conn_bus = tme_stp22xx_busy_bus(stp22xx, slave_conn_index);
  if (__tme_predict_true(slave_conn_bus != NULL)) {

    /* this is now the busy slave connection: */
    assert (stp22xx->tme_stp22xx_slave_conn_bus == NULL);
    stp22xx->tme_stp22xx_slave_conn_bus = slave_conn_bus;
  }
  return (slave_conn_bus);
}

/* this unbusies a slave generic bus connection: */
void
tme_stp22xx_slave_unbusy(struct tme_stp22xx *stp22xx)
{

  /* unbusy the slave connection: */
  tme_stp22xx_unbusy_bus(stp22xx, stp22xx->tme_stp22xx_slave_conn_bus);

  /* there is now no busy slave connection: */
  stp22xx->tme_stp22xx_slave_conn_bus = NULL;
}

/* this enters without locking the mutex: */
static struct tme_stp22xx *
_tme_stp22xx_enter_locked(struct tme_stp22xx *stp22xx)
{
  signed long completion_i;
  struct tme_completion *completion;
  _tme_stp22xx_completion_handler_t handler;

  /* loop over the completions: */
  completion_i = TME_STP22XX_COMPLETIONS_MAX - 1;
  completion = &stp22xx->tme_stp22xx_completions[completion_i];
  do {

    /* if this completion is valid: */
    if (tme_completion_is_valid(completion)) {

      /* make a read-before-read barrier: */
      tme_memory_barrier(stp22xx, stp22xx->tme_stp22xx_sizeof, TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);

      /* invalidate the completion: */
      tme_completion_invalidate(completion);

      /* call the completion handler: */
      handler = stp22xx->tme_stp22xx_completion_handlers[completion_i];
      assert (handler != NULL);
      stp22xx->tme_stp22xx_completion_handlers[completion_i] = NULL;
      (*handler)(stp22xx,
		 completion,
		 stp22xx->tme_stp22xx_completion_args[completion_i]);
    }

    completion--;
  } while (--completion_i >= 0);

  return (stp22xx);
}

/* this enters: */
struct tme_stp22xx *
tme_stp22xx_enter(struct tme_stp22xx *stp22xx)
{

  /* lock the mutex: */
  tme_mutex_lock(&stp22xx->tme_stp22xx_mutex);

  /* finish the enter: */
  return (_tme_stp22xx_enter_locked(stp22xx));
}

/* this enters as the bus master: */
struct tme_stp22xx *
tme_stp22xx_enter_master(struct tme_bus_connection *master_conn_bus)
{
  struct tme_stp22xx *stp22xx;

#if TME_STP22XX_BUS_TRANSITION
  signed long completion_i;
  struct tme_completion *completion;

  /* if the bus master was making a callout through the bus: */
#if !TME_THREADS_COOPERATIVE
#error "preemptive threads not supported yet"
#endif /* !TME_THREADS_COOPERATIVE */
  stp22xx = master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  if (stp22xx->tme_stp22xx_master_completion != NULL) {

    /* find the completion for that callout and force it to be valid.
       this is necessary since we may be reentered by a new master
       before the original callout returns: */
    completion_i = TME_STP22XX_COMPLETIONS_MAX - 1;
    for (;;) {
      if (stp22xx->tme_stp22xx_completion_handlers[completion_i]
	  == tme_stp22xx_complete_master) {
	break;
      }
      --completion_i;
      assert (completion_i >= 0);
    }
    assert (stp22xx->tme_stp22xx_completion_args[completion_i]
	    == stp22xx->tme_stp22xx_master_completion);
    completion = &stp22xx->tme_stp22xx_completions[completion_i];
    if (!tme_completion_is_valid(completion)) {
      tme_completion_validate(completion);
    }
  }

#endif /* TME_STP22XX_BUS_TRANSITION */
  
  /* enter: */
  stp22xx = tme_stp22xx_enter(master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private);

  /* only the bus master can cause a callout through the bus, and the
     bus master can only cause one such callout at a time: */
  assert (stp22xx->tme_stp22xx_master_completion == NULL);

  return (stp22xx);
}

/* this runs: */
static void
_tme_stp22xx_run(struct tme_stp22xx *stp22xx)
{

  /* if the run function is not running: */
  if (!stp22xx->tme_stp22xx_running) {

    /* the run function is now running: */
    stp22xx->tme_stp22xx_running = TRUE;

    /* call the run function: */
    (*stp22xx->tme_stp22xx_run)(stp22xx);

    /* the run function is not running: */
    stp22xx->tme_stp22xx_running = FALSE;
  }
}

/* this leaves: */
void
tme_stp22xx_leave(struct tme_stp22xx *stp22xx)
{
  signed long completion_i;
  struct tme_completion *completion;
  struct tme_completion *completions_delayed[TME_STP22XX_COMPLETIONS_DELAYED_MAX];

  /* run: */
  _tme_stp22xx_run(stp22xx);

  /* get any completions whose validations were delayed: */
  for (completion_i = 0;; completion_i++) {
    completion = stp22xx->tme_stp22xx_completions_delayed[completion_i];
    if (completion == NULL) {
      break;
    }
    stp22xx->tme_stp22xx_completions_delayed[completion_i] = NULL;
    completions_delayed[completion_i] = completion;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&stp22xx->tme_stp22xx_mutex);

  /* validate any completions: */
  for (; --completion_i >= 0; ) {
    tme_completion_validate(completions_delayed[completion_i]);
  }
}

/* this waits on a condition, with an optional sleep time: */
void
tme_stp22xx_cond_sleep_yield(struct tme_stp22xx *stp22xx,
			     struct tme_stp22xx_cond *cond,
			     const struct timeval *sleep)
{
  signed long completion_i;
  struct tme_completion *completion;

  /* this condition must be idle: */
  assert (cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_IDLE);

  /* we are now running, before waiting on this condition: */
  cond->tme_stp22xx_cond_state = TME_STP22XX_COND_STATE_RUNNING;

  /* run: */
  _tme_stp22xx_run(stp22xx);

  /* if this condition was notified while we were running: */
  if (cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_NOTIFIED) {

    /* this condition is idle again: */
    cond->tme_stp22xx_cond_state = TME_STP22XX_COND_STATE_IDLE;

    return;
  }

  /* the condition must still be running: */
  assert (cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_RUNNING);

  /* make a total write-after-write barrier, to force writes to the
     completion states to happen before validation: */
  tme_memory_barrier(0, 0, TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);

  /* validate any completions whose validations were delayed: */
  for (completion_i = 0;; completion_i++) {
    completion = stp22xx->tme_stp22xx_completions_delayed[completion_i];
    if (completion == NULL) {
      break;
    }
    stp22xx->tme_stp22xx_completions_delayed[completion_i] = NULL;
    tme_completion_validate(completion);
  }

  /* we are now waiting on this condition, unless threading is
     cooperative, in which case the condition will be idle again the
     next time this thread runs: */
  cond->tme_stp22xx_cond_state
    = (TME_THREADS_COOPERATIVE
       ? TME_STP22XX_COND_STATE_IDLE
       : TME_STP22XX_COND_STATE_WAITING);

  /* sleep or wait on the condition variable: */
  if (sleep != NULL) {
    tme_cond_sleep_yield(&cond->tme_stp22xx_cond_cond,
			 &stp22xx->tme_stp22xx_mutex,
			 sleep);
  }
  else {
    tme_cond_wait_yield(&cond->tme_stp22xx_cond_cond,
			&stp22xx->tme_stp22xx_mutex);
  }

  /* this condition is idle again: */
  cond->tme_stp22xx_cond_state = TME_STP22XX_COND_STATE_IDLE;

  /* reenter: */
  _tme_stp22xx_enter_locked(stp22xx);
}

/* this validates a completion: */
/* NB: completion may be NULL: */
void
tme_stp22xx_completion_validate(struct tme_stp22xx *stp22xx,
				struct tme_completion *completion)
{
  unsigned long completion_i;

  /* we delay validating completions: */
  for (completion_i = 0;; completion_i++) {
    assert (completion_i < TME_STP22XX_COMPLETIONS_DELAYED_MAX);
    if (stp22xx->tme_stp22xx_completions_delayed[completion_i] == NULL) {
      break;
    }
  }
  stp22xx->tme_stp22xx_completions_delayed[completion_i] = completion;
}

/* this allocates a completion: */
struct tme_completion *
tme_stp22xx_completion_alloc(struct tme_stp22xx *stp22xx,
			     _tme_stp22xx_completion_handler_t handler,
			     void *arg)
{
  signed long completion_i;
  struct tme_completion *completion;

  /* find a free completion: */
  completion_i = TME_STP22XX_COMPLETIONS_MAX - 1;
  for (; stp22xx->tme_stp22xx_completion_handlers[completion_i] != NULL; ) {
    completion_i--;
    assert (completion_i >= 0);
  }

  /* allocate this completion: */
  stp22xx->tme_stp22xx_completion_handlers[completion_i] = handler;
  stp22xx->tme_stp22xx_completion_args[completion_i] = arg;

  /* the completion can't still (or already) be valid: */
  completion = &stp22xx->tme_stp22xx_completions[completion_i];
  assert (!tme_completion_is_valid(completion));

  /* return the completion: */
  return (completion);
}

/* this calls out a bus signal to a connection: */
void
tme_stp22xx_callout_signal(struct tme_stp22xx *stp22xx,
			   tme_uint32_t conn_index,
			   unsigned int signal,
			   _tme_stp22xx_completion_handler_t handler)
{
  struct tme_bus_connection *conn_bus;
  struct tme_completion completion_buffer;
  struct tme_completion *completion;
  struct tme_bus_connection *conn_bus_other;

  /* if there is a connection at this index: */
  conn_bus = tme_stp22xx_busy_bus(stp22xx, conn_index);
  if (conn_bus != NULL) {

    /* if the connection at this index doesn't care about bus
       signals: */
    conn_bus_other = (struct tme_bus_connection *) conn_bus->tme_bus_connection.tme_connection_other;
    if (conn_bus_other->tme_bus_signal == NULL) {

      /* unbusy the bus connection: */
      tme_stp22xx_unbusy_bus(stp22xx, conn_bus);

      /* behave as if there is no connection at this index: */
      conn_bus = NULL;
    }
  }

  /* if there is no connection at this index: */
  if (conn_bus == NULL) {

    /* call the completion handler directly: */
    completion_buffer.tme_completion_error = TME_OK;
    (*handler)(stp22xx, &completion_buffer, (void *) NULL);
    return;
  }

  /* allocate a completion: */
  completion = tme_stp22xx_completion_alloc(stp22xx, handler, (void *) NULL);

  /* leave: */
  tme_stp22xx_leave(stp22xx);

  /* call out the bus signal: */
  conn_bus_other = (struct tme_bus_connection *) conn_bus->tme_bus_connection.tme_connection_other;
#if TME_STP22XX_BUS_TRANSITION
  completion->tme_completion_error =
  (*conn_bus_other->tme_bus_signal)
    (conn_bus_other,
     signal);
  tme_completion_validate(completion);
#else  /* !TME_STP22XX_BUS_TRANSITION */
#error WRITEME
#endif /* !TME_STP22XX_BUS_TRANSITION */

  /* reenter: */
  tme_stp22xx_enter(stp22xx);

  /* unbusy the bus connection: */
  tme_stp22xx_unbusy_bus(stp22xx, conn_bus);
}

/* this completes a bus operation between master and slave: */
void
tme_stp22xx_complete_master(struct tme_stp22xx *stp22xx,
			    struct tme_completion *completion,
			    void *__master_completion)
{
  struct tme_completion **_master_completion;
  struct tme_completion *master_completion;

  /* unbusy the slave connection: */
  tme_stp22xx_slave_unbusy(stp22xx);

  /* if this bus operation was not aborted: */
  _master_completion = (struct tme_completion **) __master_completion;
  if (stp22xx->tme_stp22xx_master_completion == _master_completion) {

    /* pass any slave error and scalar return value back to the master: */
    master_completion = *_master_completion;
    master_completion->tme_completion_error = completion->tme_completion_error;
    master_completion->tme_completion_scalar = completion->tme_completion_scalar;

    /* this bus operation is completed: */
    stp22xx->tme_stp22xx_master_completion = NULL;

    /* validate the completion: */
    tme_stp22xx_completion_validate(stp22xx, master_completion);
  }
}

/* this completes a bus grant: */
void
tme_stp22xx_complete_bg(struct tme_stp22xx *stp22xx,
			struct tme_completion *completion,
			void *arg)
{
  stp22xx->tme_stp22xx_master_conn_index
    = stp22xx->tme_stp22xx_master_conn_index_pending;
  stp22xx->tme_stp22xx_master_conn_index_pending = stp22xx->tme_stp22xx_conn_index_null;

  /* unused: */
  completion = 0;
  arg = 0;
}

/* this is a no-op completion: */
void
tme_stp22xx_complete_nop(struct tme_stp22xx *stp22xx,
			 struct tme_completion *completion,
			 void *arg)
{
  /* unused: */
  stp22xx = 0;
  completion = 0;
  arg = 0;
}

/* this runs a slave bus cycle: */
void
tme_stp22xx_slave_cycle(struct tme_bus_connection *master_conn_bus,
			tme_uint32_t slave_conn_index,
			struct tme_bus_cycle *master_cycle,
			tme_uint32_t *_master_fast_cycle_types,
			struct tme_completion **_master_completion)
{
  struct tme_stp22xx *stp22xx;
  struct tme_bus_connection *slave_conn_bus;
  int completion_error;
#if TME_STP22XX_BUS_TRANSITION
  struct tme_bus_tlb tlb_local;
  int rc;
  tme_uint8_t *memory;
  tme_bus_addr_t tlb_cycle_address;
  int shift;
#endif /* TME_STP22XX_BUS_TRANSITION */
  struct tme_completion *completion;
  struct tme_bus_connection *slave_conn_bus_other;
  struct tme_completion *master_completion;

  /* recover our data structure: */
  stp22xx = master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* this cycle must not have been aborted: */
  assert (stp22xx->tme_stp22xx_master_completion == _master_completion);

  /* busy the connection to any slave: */
  slave_conn_bus = tme_stp22xx_slave_busy_bus(stp22xx, slave_conn_index);

  /* if the connection index is invalid, or if there is no slave at
     this connection index: */
  if (__tme_predict_false(slave_conn_bus == NULL)) {

    /* complete with an error: */
    completion_error = ENOENT;
  }

  /* otherwise, if the master is trying a cycle to itself: */
  else if (__tme_predict_false(slave_conn_bus == master_conn_bus)) {

    /* unbusy the connection to the slave: */
    tme_stp22xx_slave_unbusy(stp22xx);

    /* complete with an error: */
    completion_error = EIO;
  }

  /* otherwise, we can run this cycle: */
  else {

#if TME_STP22XX_BUS_TRANSITION

    /* fill a TLB entry for this address: */
    tlb_local.tme_bus_tlb_token = &stp22xx->tme_stp22xx_slave_cycle_tlb_token;
    slave_conn_bus_other = (struct tme_bus_connection *) slave_conn_bus->tme_bus_connection.tme_connection_other;
    rc = ((*slave_conn_bus_other->tme_bus_tlb_fill)
	  (slave_conn_bus_other,
	   &tlb_local,
	   master_cycle->tme_bus_cycle_address,
	   master_cycle->tme_bus_cycle_type));
    assert (rc == TME_OK);

    /* the master can't do any fast transfers that this TLB entry
       doesn't allow: */
    if (tlb_local.tme_bus_tlb_emulator_off_read == TME_EMULATOR_OFF_UNDEF) {
      *_master_fast_cycle_types &= ~TME_BUS_CYCLE_READ;
    }
    if (tlb_local.tme_bus_tlb_emulator_off_write == TME_EMULATOR_OFF_UNDEF) {
      *_master_fast_cycle_types &= ~TME_BUS_CYCLE_WRITE;
    }

    /* if this cycle can be done fast: */
    /* NB: this breaks tme_shared and const: */
    memory
      = (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ
	 ? (tme_uint8_t *) tlb_local.tme_bus_tlb_emulator_off_read
	 : (tme_uint8_t *) tlb_local.tme_bus_tlb_emulator_off_write);
    if (memory != TME_EMULATOR_OFF_UNDEF) {

      /* do the fast transfer: */
      tme_bus_cycle_xfer_memory(master_cycle, memory, tlb_local.tme_bus_tlb_addr_last);

      /* unbusy the connection to the slave: */
      tme_stp22xx_slave_unbusy(stp22xx);

      /* complete with success: */
      master_completion = *_master_completion;
      master_completion->tme_completion_error = TME_OK;
      tme_stp22xx_completion_validate(stp22xx, master_completion);
      stp22xx->tme_stp22xx_master_completion = NULL;
      return;
    }

#endif /* TME_STP22XX_BUS_TRANSITION */

    /* when we complete, we will will complete for the master: */
    completion
      = tme_stp22xx_completion_alloc(stp22xx,
				     tme_stp22xx_complete_master,
				     _master_completion);

    /* leave: */
    tme_stp22xx_leave(stp22xx);

    /* run this cycle: */
#if TME_STP22XX_BUS_TRANSITION
    tlb_cycle_address = tlb_local.tme_bus_tlb_addr_offset + master_cycle->tme_bus_cycle_address;
    shift = tlb_local.tme_bus_tlb_addr_shift;
    if (shift < 0) {
      tlb_cycle_address <<= (0 - shift);
    }
    else if (shift > 0) {
      tlb_cycle_address >>= shift;
    }
    master_cycle->tme_bus_cycle_address = tlb_cycle_address;
    /* NB: we may be reentered before this cycle callout returns,
       usually by this slave device immediately turning around to
       become a master.  unfortunately, this means that
       tme_stp22xx_enter_master() will have to validate the first
       master's completion itself, before we would normally do it
       here.  this means that we also have to predict how the cycle
       will complete - and we assume that if the slave does turn
       around to become a master, that the cycle completed
       successfully: */
    completion->tme_completion_error = TME_OK;
    rc
      = ((*tlb_local.tme_bus_tlb_cycle)
	 (tlb_local.tme_bus_tlb_cycle_private,
	  master_cycle));
    if (stp22xx->tme_stp22xx_master_completion != _master_completion) {
      assert (rc == TME_OK);
    }
    else {
      completion->tme_completion_error = rc;
      tme_completion_validate(completion);
    }
#else  /* !TME_STP22XX_BUS_TRANSITION */
#error WRITEME
#endif /* !TME_STP22XX_BUS_TRANSITION */

    /* reenter: */
    tme_stp22xx_enter(stp22xx);

    return;
  }

  /* complete with the error: */
  master_completion = *_master_completion;
  master_completion->tme_completion_error = completion_error;
  tme_stp22xx_completion_validate(stp22xx, master_completion);
  stp22xx->tme_stp22xx_master_completion = NULL;
  *_master_fast_cycle_types = 0;
}

/* this fills a TLB entry: */
void
tme_stp22xx_tlb_fill(struct tme_bus_connection *agent_conn_bus,
		     struct tme_bus_tlb *tlb,
		     tme_uint32_t slave_conn_index,
		     tme_bus_addr64_t slave_address,
		     unsigned int cycle_type)
{
  struct tme_stp22xx *stp22xx;
  struct tme_bus_connection *slave_conn_bus;
  struct tme_bus_connection *slave_conn_bus_other;
#if TME_STP22XX_BUS_TRANSITION
  int rc;
#endif /* TME_STP22XX_BUS_TRANSITION */

  /* recover our data structure: */
  stp22xx = agent_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* busy the connection to any slave: */
  slave_conn_bus = tme_stp22xx_busy_bus(stp22xx, slave_conn_index);

  /* if the connection index is invalid, or if there is no slave at
     this connection index, or if the agent is filling a TLB for an
     address in itself: */
  if (__tme_predict_false(slave_conn_bus == NULL
			  || slave_conn_bus == agent_conn_bus)) {

    /* unbusy any connection to the slave: */
    if (slave_conn_bus != NULL) {
      tme_stp22xx_unbusy_bus(stp22xx, slave_conn_bus);
    }

    /* initialize the TLB entry: */
    tme_bus_tlb_initialize(tlb);

    /* our caller will map this TLB entry from covering all addresses
       to only covering a region's addresses: */
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = 0 - (tme_bus_addr_t) 1;
  }

  /* otherwise, the slave can fill this TLB entry: */
  else {

    /* leave: */
    tme_stp22xx_leave(stp22xx);

    /* fill this TLB entry: */
    slave_conn_bus_other = (struct tme_bus_connection *) slave_conn_bus->tme_bus_connection.tme_connection_other;
#if TME_STP22XX_BUS_TRANSITION
    rc = 
#endif /* TME_STP22XX_BUS_TRANSITION */
    (*slave_conn_bus_other->tme_bus_tlb_fill)
      (slave_conn_bus_other,
       tlb,
       slave_address,
       cycle_type);
#if TME_STP22XX_BUS_TRANSITION
    assert (rc == TME_OK);
#endif /* TME_STP22XX_BUS_TRANSITION */

    /* reenter: */
    tme_stp22xx_enter(stp22xx);

    /* unbusy the connection to the slave: */
    tme_stp22xx_unbusy_bus(stp22xx, slave_conn_bus);
  }
}

#if TME_STP22XX_BUS_TRANSITION

/* this is the bus TLB set add transition glue: */
#undef tme_stp22xx_tlb_set_add
int
tme_stp22xx_tlb_set_add_transition(struct tme_bus_connection *agent_conn_bus,
				   struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_completion completion_buffer;
  tme_completion_init(&completion_buffer);
  tme_stp22xx_tlb_set_add(agent_conn_bus,
			  tlb_set_info,
			  &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

#endif /* TME_STP22XX_BUS_TRANSITION */

/* this adds a TLB set: */
void
tme_stp22xx_tlb_set_add(struct tme_bus_connection *agent_conn_bus,
			struct tme_bus_tlb_set_info *tlb_set_info,
			struct tme_completion *agent_completion)
{
  struct tme_stp22xx *stp22xx;

  /* enter: */
  stp22xx = tme_stp22xx_enter((struct tme_stp22xx *) agent_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private);

  /* if this TLB set provides a bus context register: */
  if (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL) {

    /* this bus only has one context: */
    *tlb_set_info->tme_bus_tlb_set_info_bus_context = 0;
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max = 0;
  }

  /* leave: */
  agent_completion->tme_completion_error = TME_OK;
  tme_stp22xx_completion_validate(stp22xx, agent_completion);
  tme_stp22xx_leave(stp22xx);
}

/* this notifies a condition: */
void
tme_stp22xx_cond_notify(struct tme_stp22xx_cond *cond)
{

  /* if threading is cooperative, and this condition is idle: */
  if (TME_THREADS_COOPERATIVE
      && cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_IDLE) {

    /* we have to assume that a thread has yielded waiting on this
       condition, but because threading is cooperative, has already
       set the condition back to idle.  we can't mark the condition as
       notified, since it would never get set back to idle: */
  }

  /* otherwise, threading is not cooperative, or the condition is not
     idle: */
  else {

    /* this condition must be either running before waiting, or
       waiting: */
    assert (cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_RUNNING
	    || cond->tme_stp22xx_cond_state == TME_STP22XX_COND_STATE_WAITING);

    /* this condition has been notified: */
    cond->tme_stp22xx_cond_state = TME_STP22XX_COND_STATE_NOTIFIED;
  }

  /* notify the real condition: */
  tme_cond_notify(&cond->tme_stp22xx_cond_cond, FALSE);
}

/* this initializes a condition: */
void
tme_stp22xx_cond_init(struct tme_stp22xx_cond *cond)
{
  /* this condition is idle: */
  cond->tme_stp22xx_cond_state = TME_STP22XX_COND_STATE_IDLE;
  tme_cond_init(&cond->tme_stp22xx_cond_cond);
}

/* this initializes an stp22xx: */
void
tme_stp22xx_init(struct tme_stp22xx *stp22xx,
		 unsigned long _sizeof,
		 tme_uint32_t conn_index_null)
{
  unsigned long completion_i;

  /* initialize the mutex: */
  tme_mutex_init(&stp22xx->tme_stp22xx_mutex);

  /* set the size of the structure: */
  stp22xx->tme_stp22xx_sizeof = _sizeof;

  /* set the undefined connection index: */
  stp22xx->tme_stp22xx_conn_index_null = conn_index_null;

  /* initialize the completions: */
  for (completion_i = 0; completion_i < TME_STP22XX_COMPLETIONS_MAX; completion_i++) {
    tme_completion_init(&stp22xx->tme_stp22xx_completions[completion_i]);
    stp22xx->tme_stp22xx_completion_handlers[completion_i] = NULL;
  }

  /* initialize the delayed completions: */
  for (completion_i = 0; completion_i < TME_STP22XX_COMPLETIONS_DELAYED_MAX; completion_i++) {
    stp22xx->tme_stp22xx_completions_delayed[completion_i] = NULL;
  }

  /* there is no current bus master: */
  stp22xx->tme_stp22xx_master_conn_index = conn_index_null;

#if TME_STP22XX_BUS_TRANSITION
  /* initialize the token for filling TLB entries for slave cycles: */
  tme_token_init(&stp22xx->tme_stp22xx_slave_cycle_tlb_token);
#endif /* TME_STP22XX_BUS_TRANSITION */
}
