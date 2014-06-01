/* $Id: scsi-bus.c,v 1.8 2007/02/15 01:30:43 fredette Exp $ */

/* scsi/scsi-bus.c - a generic SCSI bus element: */

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
_TME_RCSID("$Id: scsi-bus.c,v 1.8 2007/02/15 01:30:43 fredette Exp $");

/* includes: */
#include <tme/generic/scsi.h>
#include <tme/threads.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */

/* macros: */

/* the callout flags: */
#define TME_SCSI_BUS_CALLOUT_CHECK		(0)
#define TME_SCSI_BUS_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SCSI_BUS_CALLOUTS_MASK		(-2)
#define  TME_SCSI_BUS_CALLOUT_CYCLE		TME_BIT(1)

/* structures: */

/* a scsi bus: */
struct tme_scsi_bus {

  /* backpointer to our element: */
  struct tme_element *tme_scsi_bus_element;

  /* our mutex: */
  tme_mutex_t tme_scsi_bus_mutex;

  /* our connections: */
  struct tme_connection *tme_scsi_bus_connections;

  /* the callout flags: */
  int tme_scsi_bus_callout_flags;

  /* the current bus state: */
  tme_scsi_control_t tme_scsi_bus_control;
  tme_scsi_control_t tme_scsi_bus_data;
};

/* internal information about a SCSI connection: */
struct tme_scsi_connection_int {

  /* the external SCSI connection: */
  struct tme_scsi_connection tme_scsi_connection_int;

  /* the cycle marker for this connection: */
  tme_uint32_t tme_scsi_connection_int_cycle_marker;

  /* the control and data lines currently asserted by this connection: */
  tme_scsi_control_t tme_scsi_connection_int_control;
  tme_scsi_data_t tme_scsi_connection_int_data;

  /* the SCSI bus state last reported to the connection: */
  tme_scsi_control_t tme_scsi_connection_int_last_control;
  tme_scsi_control_t tme_scsi_connection_int_last_data;

  /* any events that this connection is waiting on, and any that
     triggered: */
  tme_uint32_t tme_scsi_connection_int_events_waiting;
  tme_uint32_t tme_scsi_connection_int_events_triggered;

  /* any actions that this connection is waiting to take, and any that
     were taken: */
  tme_uint32_t tme_scsi_connection_int_actions_waiting;
  tme_uint32_t tme_scsi_connection_int_actions_taken;

  /* any step within the current action sequence: */
  unsigned int tme_scsi_connection_int_sequence_step;

  /* any DMA structure for this connection: */
  struct tme_scsi_dma tme_scsi_connection_int_dma_buffer;
  struct tme_scsi_dma *tme_scsi_connection_int_dma;
  
  /* specific callout flags for this connection: */
  int tme_scsi_connection_int_callout_flags;
};

/* the SCSI bus callout function.  it must be called with the mutex locked: */
static void
_tme_scsi_bus_callout(struct tme_scsi_bus *scsi_bus, int new_callouts)
{
  struct tme_scsi_connection_int *conn_int;
  struct tme_scsi_connection *conn_scsi;
  int callouts, later_callouts;
  tme_scsi_control_t control;
  tme_scsi_data_t data;
  tme_uint32_t events_triggered;
  tme_uint32_t actions_taken;
  struct tme_scsi_dma dma_buffer;
  const struct tme_scsi_dma *dma;
  int rc;
  
  /* add in any new callouts: */
  scsi_bus->tme_scsi_bus_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (scsi_bus->tme_scsi_bus_callout_flags
      & TME_SCSI_BUS_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  scsi_bus->tme_scsi_bus_callout_flags
    |= TME_SCSI_BUS_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = scsi_bus->tme_scsi_bus_callout_flags)
	  & TME_SCSI_BUS_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    scsi_bus->tme_scsi_bus_callout_flags
      = (callouts
	 & ~TME_SCSI_BUS_CALLOUTS_MASK);
    callouts &= TME_SCSI_BUS_CALLOUTS_MASK;

    /* if we need to call out SCSI bus cycles: */
    if (callouts & TME_SCSI_BUS_CALLOUT_CYCLE) {

      /* loop over all devices on the bus: */
      for (conn_int
	     = ((struct tme_scsi_connection_int *)
		scsi_bus->tme_scsi_bus_connections);
	   conn_int != NULL;
	   conn_int
	     = ((struct tme_scsi_connection_int *)
		conn_int->tme_scsi_connection_int.tme_scsi_connection.tme_connection_next)) {
	
	/* if this device doesn't need a callout, continue: */
	if (!(conn_int->tme_scsi_connection_int_callout_flags
	      & TME_SCSI_BUS_CALLOUT_CYCLE)) {
	  continue;
	}

	/* clear the callout flag on this device: */
	conn_int->tme_scsi_connection_int_callout_flags
	  &= ~TME_SCSI_BUS_CALLOUT_CYCLE;

	/* get the current state of the bus: */
	control = scsi_bus->tme_scsi_bus_control;
	data = scsi_bus->tme_scsi_bus_data;

	/* remember this last bus state called out to this connection: */
	conn_int->tme_scsi_connection_int_last_control
	  = control;
	conn_int->tme_scsi_connection_int_last_data
	  = data;

	/* get and clear the events triggered, actions taken, and any
           DMA structure: */
	events_triggered = conn_int->tme_scsi_connection_int_events_triggered;
	actions_taken = conn_int->tme_scsi_connection_int_actions_taken;
	dma = conn_int->tme_scsi_connection_int_dma;
	if (dma != NULL) {
	  dma_buffer = *dma;
	  dma = &dma_buffer;
	}
	conn_int->tme_scsi_connection_int_events_triggered = 0;
	conn_int->tme_scsi_connection_int_actions_taken = 0;
	conn_int->tme_scsi_connection_int_dma = NULL;

	/* add the cycle marker to the actions taken: */
	actions_taken |= conn_int->tme_scsi_connection_int_cycle_marker;

	/* unlock the mutex: */
	tme_mutex_unlock(&scsi_bus->tme_scsi_bus_mutex);
	
	/* do the callout: */
	conn_scsi
	  = ((struct tme_scsi_connection *)
	     conn_int->tme_scsi_connection_int.tme_scsi_connection.tme_connection_other);
	rc = ((*conn_scsi->tme_scsi_connection_cycle)
	      (conn_scsi,
	       control,
	       data,
	       events_triggered,
	       actions_taken,
	       dma));
	
	/* lock the mutex: */
	tme_mutex_lock(&scsi_bus->tme_scsi_bus_mutex);
      
	/* if the callout was unsuccessful, remember that at some later
	   time this callout should be attempted again: */
	if (rc != TME_OK) {
	  conn_int->tme_scsi_connection_int_callout_flags
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  later_callouts
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  scsi_bus->tme_scsi_bus_callout_flags = later_callouts;
}

/* this handles a SCSI bus cycle: */
static int
_tme_scsi_bus_cycle(struct tme_scsi_connection *conn_scsi,
		    tme_scsi_control_t control,
		    tme_scsi_data_t data,
		    tme_uint32_t events_asker,
		    tme_uint32_t actions_asker,
		    const struct tme_scsi_dma *dma_asker)
{
  struct tme_scsi_bus *scsi_bus;
  struct tme_scsi_connection_int *conn_int_asker, *conn_int;
  tme_uint32_t events;
  tme_uint32_t actions;
  struct tme_scsi_dma *dma;
  struct tme_scsi_connection_int *dma_initiator;
  struct tme_scsi_connection_int *dma_target;
  struct tme_scsi_dma *dma_in, *dma_out;
  unsigned long count;
  int bus_changed;
  int new_callouts;
  int again;

  /* recover our bus and internal connection: */
  scsi_bus = conn_scsi->tme_scsi_connection.tme_connection_element->tme_element_private;
  conn_int_asker = (struct tme_scsi_connection_int *) conn_scsi;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&scsi_bus->tme_scsi_bus_mutex);

  /* update the cycle marker for this device: */
  conn_int_asker->tme_scsi_connection_int_cycle_marker = (actions_asker & TME_SCSI_ACTION_CYCLE_MARKER);
  actions_asker &= ~TME_SCSI_ACTION_CYCLE_MARKER;

  /* update the signals that this device is asserting: */
  conn_int_asker->tme_scsi_connection_int_control = control;
  conn_int_asker->tme_scsi_connection_int_data = data;

  /* if you're waiting on TME_SCSI_EVENT_BUS_CHANGE, you can't wait on
     anything else, and you can't have any actions: */
  /* you can't wait on TME_SCSI_EVENT_BUS_CHANGE and anything else: */
  assert(!(events_asker & TME_SCSI_EVENT_BUS_CHANGE)
	 || (events_asker == TME_SCSI_EVENT_BUS_CHANGE
	     && actions_asker == TME_SCSI_ACTION_NONE));

  /* you can do at most one of: select, reselect, DMA initiator, DMA target: */
  assert (((events_asker
	    & (TME_SCSI_ACTION_SELECT
	       | TME_SCSI_ACTION_RESELECT
	       | TME_SCSI_ACTION_DMA_INITIATOR
	       | TME_SCSI_ACTION_DMA_TARGET))
	   & ((events_asker
	       & (TME_SCSI_ACTION_SELECT
		  | TME_SCSI_ACTION_RESELECT
		  | TME_SCSI_ACTION_DMA_INITIATOR
		  | TME_SCSI_ACTION_DMA_TARGET)) - 1)) == 0);

  /* you can't provide a DMA structure without a DMA action, or with any events: */
  assert ((dma_asker != NULL)
	  == ((actions_asker
	       & (TME_SCSI_ACTION_DMA_INITIATOR
		  | TME_SCSI_ACTION_DMA_TARGET)) != 0));
  assert ((dma_asker == NULL) || (events_asker == TME_SCSI_EVENT_NONE));

  /* update the events and actions for this device: */
  conn_int_asker->tme_scsi_connection_int_events_waiting = events_asker;
  conn_int_asker->tme_scsi_connection_int_events_triggered = 0;
  conn_int_asker->tme_scsi_connection_int_actions_waiting = actions_asker;
  conn_int_asker->tme_scsi_connection_int_actions_taken = 0;
  conn_int_asker->tme_scsi_connection_int_sequence_step = 0;

  /* if this is a DMA sequence: */
  conn_int_asker->tme_scsi_connection_int_dma = NULL;
  if (dma_asker != NULL) {

    /* XXX is the 8-bit DMA restriction necessary?  how does wide SCSI
       handle transfers of odd numbers of bytes? */
    if ((dma_asker->tme_scsi_dma_flags
	 & TME_SCSI_DMA_WIDTH)
	!= TME_SCSI_DMA_8BIT) {
      abort();
    }

    /* copy in the DMA structure: */
    conn_int_asker->tme_scsi_connection_int_dma_buffer = *dma_asker;
    conn_int_asker->tme_scsi_connection_int_dma = &conn_int_asker->tme_scsi_connection_int_dma_buffer;

    /* if the caller passed us a DMA structure with zero bytes left to
       transfer, call out a cycle now: */
    if (dma_asker->tme_scsi_dma_resid == 0) {
      conn_int_asker->tme_scsi_connection_int_callout_flags
	|= TME_SCSI_BUS_CALLOUT_CYCLE;
      conn_int_asker->tme_scsi_connection_int_events_waiting = TME_SCSI_EVENT_NONE;
      conn_int_asker->tme_scsi_connection_int_actions_waiting = actions_asker;
      new_callouts
	|= TME_SCSI_BUS_CALLOUT_CYCLE;
    }
  }

  /* if during any iteration of the below loop, we see or cause a
     change on the bus, we want to call out cycles to all devices
     waiting on a simple change: */
  bus_changed = FALSE;

  /* loop until things settle down: */
  for (again = TRUE; again; ) {
    again = FALSE;

    /* get the current state of the bus: */
    control = 0;
    data = 0;
    for (conn_int
	   = ((struct tme_scsi_connection_int *)
	      scsi_bus->tme_scsi_bus_connections);
	 conn_int != NULL;
	 conn_int
	   = ((struct tme_scsi_connection_int *)
	      conn_int->tme_scsi_connection_int.tme_scsi_connection.tme_connection_next)) {
      control |= conn_int->tme_scsi_connection_int_control;
      data |= conn_int->tme_scsi_connection_int_data;
    }
    if ((control != scsi_bus->tme_scsi_bus_control)
	|| (data != scsi_bus->tme_scsi_bus_data)) {
      bus_changed = TRUE;
    }
    scsi_bus->tme_scsi_bus_control = control;
    scsi_bus->tme_scsi_bus_data = data;

    /* loop over all devices on the bus: */
    dma_initiator = NULL;
    dma_target = NULL;
    for (conn_int
	   = ((struct tme_scsi_connection_int *)
	      scsi_bus->tme_scsi_bus_connections);
	 conn_int != NULL;
	 conn_int
	   = ((struct tme_scsi_connection_int *)
	      conn_int->tme_scsi_connection_int.tme_scsi_connection.tme_connection_next)) {

      /* get any waiting events and actions: */
      events = conn_int->tme_scsi_connection_int_events_waiting;
      actions = conn_int->tme_scsi_connection_int_actions_waiting;

      /* if this device isn't waiting for any events and has no
         actions to take, continue now: */
      if (events == TME_SCSI_EVENT_NONE
	  && actions == TME_SCSI_ACTION_NONE) {
	continue;
      }

      /* if this device is waiting on any change to the bus state, and
         the bus has changed: */
      if ((events & TME_SCSI_EVENT_BUS_CHANGE)
	  && (bus_changed
	      || (control !=
		  conn_int->tme_scsi_connection_int_last_control)
	      || (data
		  != conn_int->tme_scsi_connection_int_last_data))) {

	/* this event has triggered.  we never take any action: */
	conn_int->tme_scsi_connection_int_events_triggered = TME_SCSI_EVENT_BUS_CHANGE;
	events = TME_SCSI_EVENT_NONE;
	actions = TME_SCSI_ACTION_NONE;
      }

      /* if this device is waiting to be selected, and the device is
         now being selected: */
      if ((events & TME_SCSI_EVENT_SELECTED)

	  /* "In all systems, the target shall determine that it is
	     selected when SEL and its SCSI ID bit are true and BSY and
	     I/O are false for at least a bus settle delay." */
	  && TME_SCSI_ID_SELECTED(TME_SCSI_EVENT_IDS_WHICH(events), control, data)) {

	/* this event has triggered.  the only action we can take is
	   to respond to the selection: */
	conn_int->tme_scsi_connection_int_events_triggered = TME_SCSI_EVENT_SELECTED | TME_SCSI_EVENT_IDS_SELF(data);
	events = TME_SCSI_EVENT_NONE;
	actions &= TME_SCSI_ACTION_RESPOND_SELECTED;
      }

      /* if this device is waiting to be reselected, and the device is
         now being reselected: */
      if ((events & TME_SCSI_EVENT_RESELECTED)

	  /* "The initiator shall determine that it is reselected when
	     SEL, I/O, and its SCSI ID bit are true and BSY is false
	     for at least a bus settle delay." */
	  && TME_SCSI_ID_RESELECTED(TME_SCSI_EVENT_IDS_WHICH(events), control, data)) {

	/* this event has triggered.  the only action we can take is
	   to respond to the reselection: */
	conn_int->tme_scsi_connection_int_events_triggered = TME_SCSI_EVENT_RESELECTED | TME_SCSI_EVENT_IDS_SELF(data);
	events = TME_SCSI_EVENT_NONE;
	actions &= TME_SCSI_ACTION_RESPOND_RESELECTED;
      }

      /* if this device is waiting for the bus to be free, and the bus
         is now free: */
      if ((events & TME_SCSI_EVENT_BUS_FREE)

	  /* "SCSI devices shall detect the BUS FREE phase after SEL
	     and BSY are both false for at least a bus settle delay."  */
	  && (control
	      & (TME_SCSI_SIGNAL_SEL
		 | TME_SCSI_SIGNAL_BSY)) == 0) {

	/* this event has triggered.  we won't respond to selection or
	   reselection, since we won't be selected or reselected: */
	conn_int->tme_scsi_connection_int_events_triggered = TME_SCSI_EVENT_BUS_FREE;
	events = TME_SCSI_EVENT_NONE;
	actions &= ~(TME_SCSI_ACTION_RESPOND_SELECTED | TME_SCSI_ACTION_RESPOND_RESELECTED);
      }

      /* put back this device's waiting events.  if this device is
         still waiting for one or more events, continue now: */
      conn_int->tme_scsi_connection_int_events_waiting = events;
      if (events != TME_SCSI_EVENT_NONE) {
	continue;
      }

      /* if this device is arbitrating for the bus: */
      if (TME_SCSI_ACTIONS_SELECTED(actions, TME_SCSI_ACTION_ARBITRATE_FULL)) {
	
	/* assert BSY and the device ID on this device's behalf.  if
	   we're doing full arbitration, also assert SEL on this
	   device's behalf: */
	conn_int->tme_scsi_connection_int_control
	  = (TME_SCSI_SIGNAL_BSY
	     | (((actions & TME_SCSI_ACTION_ARBITRATE_FULL)
		 == TME_SCSI_ACTION_ARBITRATE_FULL)
		? TME_SCSI_SIGNAL_SEL
		: 0));
	conn_int->tme_scsi_connection_int_data
	  = TME_BIT(TME_SCSI_ACTION_ID_SELF_WHICH(actions));
	again = TRUE;
	bus_changed = TRUE;

	/* this action has been taken: */
	conn_int->tme_scsi_connection_int_actions_taken
	  |= ((actions & TME_SCSI_ACTION_ARBITRATE_FULL)
	      | TME_SCSI_ACTION_ID_SELF(TME_SCSI_ACTION_ID_SELF_WHICH(actions)));
	actions &= ~TME_SCSI_ACTION_ARBITRATE_FULL;
      }

      /* if this device is selecting or reselecting another device: */
      if (TME_SCSI_ACTIONS_SELECTED(actions,
				    (TME_SCSI_ACTION_SELECT
				     | TME_SCSI_ACTION_SELECT_WITH_ATN
				     | TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR
				     | TME_SCSI_ACTION_RESELECT))) {

	/* "Except in certain single initiator environments with
	   initiators employing the single initiator option, the
	   initiator shall set the DATA BUS to a value which is the OR
	   of its SCSI ID bit and the target's SCSI ID bit.  The
	   initiator shall then wait at least two deskew delays and
	   release BSY."

	   "Initiators that do not implement the RESELECTION phase and
	   do not operate in the multiple initiator environment are
	   allowed to set only the target's SCSI ID bit during the
	   SELECTION phase.  This makes it impossible for the target
	   to determine the initiator's SCSI ID."

	   "The winning SCSI device becomes a target by asserting the
	   I/O signal.  The winning SCSI device shall also set the
	   DATA BUS to a value that is the OR of its SCSI ID bit and
	   the initiator's SCSI ID bit.  The target shall wait at
	   least two deskew delays and release BSY." */

	/* set the sequence's selection/reselection SCSI IDs on the
	   bus, then set I/O if this is a reselection, and release
	   BSY: */
	conn_int->tme_scsi_connection_int_data
	  = (TME_BIT(TME_SCSI_ACTION_ID_OTHER_WHICH(actions))
	     | (((actions & TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR)
		 == TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR)
		? 0
		: TME_BIT(TME_SCSI_ACTION_ID_SELF_WHICH(actions))));
	conn_int->tme_scsi_connection_int_control
	  = (TME_SCSI_SIGNAL_SEL
	     | ((actions & TME_SCSI_ACTION_RESELECT)
		? TME_SCSI_SIGNAL_I_O
		: 0)
	     | (((actions & TME_SCSI_ACTION_SELECT_WITH_ATN)
		 == TME_SCSI_ACTION_SELECT_WITH_ATN)
		? TME_SCSI_SIGNAL_ATN
		: 0));
	again = TRUE;
	bus_changed = TRUE;

	/* this action has been taken: */
	conn_int->tme_scsi_connection_int_actions_taken
	  |= ((actions
	       & (TME_SCSI_ACTION_SELECT
		  | TME_SCSI_ACTION_SELECT_WITH_ATN
		  | TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR
		  | TME_SCSI_ACTION_RESELECT))
	      | TME_SCSI_ACTION_ID_SELF(TME_SCSI_ACTION_ID_SELF_WHICH(actions))
	      | TME_SCSI_ACTION_ID_OTHER(TME_SCSI_ACTION_ID_OTHER_WHICH(actions)));
	actions &= ~(TME_SCSI_ACTION_SELECT
		     | TME_SCSI_ACTION_SELECT_WITH_ATN
		     | TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR
		     | TME_SCSI_ACTION_RESELECT);
      }

      /* if this device is responding to selection or reselection: */
      if (TME_SCSI_ACTIONS_SELECTED(actions, 
				    (TME_SCSI_ACTION_RESPOND_SELECTED
				     | TME_SCSI_ACTION_RESPOND_RESELECTED))) {

	/* dispatch on the step: */
	switch (conn_int->tme_scsi_connection_int_sequence_step) {
	    
	case 0:
	  
	  /* assert BSY on the device's behalf: */
	  conn_int->tme_scsi_connection_int_control
	    = TME_SCSI_SIGNAL_BSY;
	  again = TRUE;
	  bus_changed = TRUE;

	  /* advance to the next step: */
	  conn_int->tme_scsi_connection_int_sequence_step++;
	    
	  /* FALLTHROUGH */

	case 1:

	  /* "At least two deskew delays after the initiator detects
	     BSY is true, it shall release SEL and may change the DATA
	     BUS."

	     "After the reselected initiator detects SEL false, it
	     shall release BSY." */
	  if (!(control
		& TME_SCSI_SIGNAL_SEL)) {

	    /* if this was a reselection: */
	    if (actions & TME_SCSI_ACTION_RESPOND_RESELECTED) {

	      /* negate BSY on the device's behalf: */
	      conn_int->tme_scsi_connection_int_control
		= 0;
	      again = TRUE;
	      bus_changed = TRUE;
	    }

	    /* this action has been taken: */
	    conn_int->tme_scsi_connection_int_actions_taken
	      |= (actions
		  & (TME_SCSI_ACTION_RESPOND_SELECTED
		     | TME_SCSI_ACTION_RESPOND_RESELECTED));
	    actions
	      &= ~(TME_SCSI_ACTION_RESPOND_SELECTED
		   | TME_SCSI_ACTION_RESPOND_RESELECTED);
	    conn_int->tme_scsi_connection_int_sequence_step = 0;
	  }
	  break;
	}
      }

      /* there can be at most one device in an initiator or target
	 information transfer phase DMA sequence: */
      else if (TME_SCSI_ACTIONS_SELECTED(actions, 
					 (TME_SCSI_ACTION_DMA_INITIATOR
					  | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK))) {
	assert (dma_initiator == NULL);
	dma_initiator = conn_int;
      }
      else if (TME_SCSI_ACTIONS_SELECTED(actions, TME_SCSI_ACTION_DMA_TARGET)) {
	assert (dma_target == NULL);
	dma_target = conn_int;
      }

      /* put back this device's waiting actions.  if this device is
         not waiting for any more actions: */
      conn_int->tme_scsi_connection_int_actions_waiting = actions;
      if ((actions
	   & (TME_SCSI_ACTION_DMA_INITIATOR
	      | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK
	      | TME_SCSI_ACTION_DMA_TARGET
	      | TME_SCSI_ACTION_RESPOND_SELECTED
	      | TME_SCSI_ACTION_RESPOND_RESELECTED
	      | TME_SCSI_ACTION_SELECT
	      | TME_SCSI_ACTION_SELECT_WITH_ATN
	      | TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR
	      | TME_SCSI_ACTION_RESELECT
	      | TME_SCSI_ACTION_ARBITRATE_HALF
	      | TME_SCSI_ACTION_ARBITRATE_FULL)) == 0) {

	/* call out a cycle on this device: */
	conn_int->tme_scsi_connection_int_callout_flags
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
	new_callouts
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
      }
    }

    /* if we need to loop again, do so immediately: */
    if (again) {
      continue;
    }

    /* if a device is in the target information transfer phase DMA
       sequence: */
    if (dma_target != NULL) {

      /* get this device's DMA structure: */
      dma = dma_target->tme_scsi_connection_int_dma;
      assert (dma != NULL);

      /* dispatch on the sequence step: */
      switch (dma_target->tme_scsi_connection_int_sequence_step) {

	/* "If I/O is true (transfer to the initiator)... [after] ACK
	   is false the target may continue the transfer by driving
	   DB(7-0,P) and asserting REQ, as described above."

	   "If I/O is false (transfer to the target)... [after ACK is
	   false the target] may continue the transfer by asserting
	   REQ, as described above." */
      case 2:
	if (control & TME_SCSI_SIGNAL_ACK) {
	  break;
	}

	/* if the DMA has been exhausted, callout a cycle on this
	   device: */
	if (control & TME_SCSI_SIGNAL_I_O) {
	  dma->tme_scsi_dma_out++;
	}
	else {
	  dma->tme_scsi_dma_in++;
	}
	if (--dma->tme_scsi_dma_resid == 0) {
	  dma_target->tme_scsi_connection_int_callout_flags
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  dma_target->tme_scsi_connection_int_actions_waiting
	    = TME_SCSI_ACTION_NONE;
	  dma_target->tme_scsi_connection_int_actions_taken
	    |= TME_SCSI_ACTION_DMA_TARGET;
	  new_callouts
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  break;
	}

	/* FALLTHROUGH */

      /* "If I/O is true (transfer to the initiator), the target shall
	 first drive DB(7-0,P) to their desired values, delay at least
	 one deskew delay plus a cable skew delay, then assert REQ."

	 "If I/O is false (transfer to the target) the target shall request 
	 information by asserting REQ. " */
      case 0:

	/* assert REQ on the target's behalf: */
	dma_target->tme_scsi_connection_int_control
	  |= TME_SCSI_SIGNAL_REQ;
	again = TRUE;
	bus_changed = TRUE;

	/* if I/O is asserted, assert the output data: */
	if (control & TME_SCSI_SIGNAL_I_O) {
	  dma_target->tme_scsi_connection_int_data
	    = *(dma->tme_scsi_dma_out);
	}

	/* advance to step one: */
	dma_target->tme_scsi_connection_int_sequence_step = 1;
	break;

	/* "If I/O is true (transfer to the initiator)... [when] ACK
	   becomes true at the target, the target may change or
	   release DB(7-0,P) and shall negate REQ."

	   "If I/O is false (transfer to the target)... [when] ACK
	   becomes true at the target, the target shall read
	   DB(7-0,P), then negate REQ." */
      case 1:
	if (control & TME_SCSI_SIGNAL_ACK) {

	  /* if I/O is negated, read the input data: */
	  if (!(control & TME_SCSI_SIGNAL_I_O)) {
	    *(dma->tme_scsi_dma_in) = data;
	  }

	  /* negate REQ on the target's behalf: */
	  dma_target->tme_scsi_connection_int_control
	    &= ~TME_SCSI_SIGNAL_REQ;
	  again = TRUE;
	  bus_changed = TRUE;
	
	  /* advance to step two: */
	  dma_target->tme_scsi_connection_int_sequence_step = 2;
	}
	break;

      default: assert (FALSE);
      }

      /* if the bus is being reset: */
      if (control & TME_SCSI_SIGNAL_RST) {

	/* negate all signals on the target's behalf: */
	dma_target->tme_scsi_connection_int_control = 0;
	dma_target->tme_scsi_connection_int_data = 0;
	again = TRUE;
	bus_changed = TRUE;
	
	/* callout a cycle on this device: */
	dma_target->tme_scsi_connection_int_callout_flags
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
	dma_target->tme_scsi_connection_int_actions_waiting
	  = TME_SCSI_ACTION_NONE;
	dma_target->tme_scsi_connection_int_actions_taken
	  = TME_SCSI_ACTION_NONE;
	dma_target->tme_scsi_connection_int_events_triggered
	  = TME_SCSI_EVENT_BUS_RESET;
	new_callouts
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
      }
    }

    /* if we need to loop again, do so immediately: */
    if (again) {
      continue;
    }

    /* if a device is in the initiator information transfer phase DMA
       sequence: */
    if (dma_initiator != NULL) {

      /* get this device's DMA structure: */
      dma = dma_initiator->tme_scsi_connection_int_dma;
      assert (dma != NULL);
      
      /* dispatch on the sequence step: */
      switch (dma_initiator->tme_scsi_connection_int_sequence_step) {
	
	/* "If I/O is true (transfer to the initiator)... [the]
	   initiator shall read DB(7-0,P) after REQ is true, then
	   signal its acceptance of the data by asserting ACK."
	   
	   "If I/O is false (transfer to the target)... [the]
	   initiator shall drive DB(7-0,P) to their desired values
	   [after REQ is true], delay at least one deskew delay plus a
	   cable skew delay and assert ACK." */
      case 0:
	if (!(control & TME_SCSI_SIGNAL_REQ)) {
	  break;
	}
	  
	/* if the information transfer phase has changed, callout a
	   cycle on this device: */
	if (TME_SCSI_PHASE(control)
	    != TME_SCSI_PHASE(dma_initiator->tme_scsi_connection_int_last_control)) {
	  dma_initiator->tme_scsi_connection_int_callout_flags
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  dma_initiator->tme_scsi_connection_int_actions_taken
	    |= dma_initiator->tme_scsi_connection_int_actions_waiting;
	  dma_initiator->tme_scsi_connection_int_actions_waiting
	    = TME_SCSI_ACTION_NONE;
	  new_callouts
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  break;
	}

	/* if another device is in the target information transfer
	   phase DMA sequence, we may do a bulk copy between them: */
	if (dma_target != NULL) {
	  assert (dma_target->tme_scsi_connection_int_sequence_step == 1);

	  /* sort the devices' DMA structures into input and output: */
	  if (control & TME_SCSI_SIGNAL_I_O) {
	    dma_in = dma_initiator->tme_scsi_connection_int_dma;
	    dma_out = dma_target->tme_scsi_connection_int_dma;
	  }
	  else {
	    dma_out = dma_initiator->tme_scsi_connection_int_dma;
	    dma_in = dma_target->tme_scsi_connection_int_dma;
	  }
	  assert (dma_out != NULL && dma_in != NULL);

	  /* get the size of the bulk copy.  we won't copy all
	     possible bytes, since this code doesn't know how to
	     assert control lines or make callouts.  instead, we bulk
	     copy one less than all possible bytes, and let the
	     normal, non-bulk code handle the final byte: */
	  count = TME_MIN(dma_out->tme_scsi_dma_resid,
			  dma_in->tme_scsi_dma_resid);
	  assert (count > 0);

	  /* if we have bytes to bulk copy: */
	  count--;
	  if (count > 0) {
	  
	    /* do the bulk copy: */
	    memcpy(dma_in->tme_scsi_dma_in,
		   dma_out->tme_scsi_dma_out,
		   count);

	    /* advance: */
	    dma_in->tme_scsi_dma_in += count;
	    dma_in->tme_scsi_dma_resid -= count;
	    dma_out->tme_scsi_dma_out += count;
	    dma_out->tme_scsi_dma_resid -= count;

	    /* if I/O is asserted, assert the new data on the target's
               behalf: */
	    if (control & TME_SCSI_SIGNAL_I_O) {
	      data = *(dma_out->tme_scsi_dma_out);
	      dma_target->tme_scsi_connection_int_data = data;
	      again = TRUE;
	      bus_changed = TRUE;
	    }
	  }
	}

	/* if I/O is true, read the data, else
	   write the data: */
	if (control & TME_SCSI_SIGNAL_I_O) {
	  *dma->tme_scsi_dma_in = data;
	}
	else {
	  dma_initiator->tme_scsi_connection_int_data
	    = *(dma->tme_scsi_dma_out);
	}
	
	/* assert ACK on the initiator's behalf: */
	dma_initiator->tme_scsi_connection_int_control
	  |= TME_SCSI_SIGNAL_ACK;
	again = TRUE;
	bus_changed = TRUE;
	
	/* advance to step one: */
	dma_initiator->tme_scsi_connection_int_sequence_step = 1;
	break;
	
	/* "If I/O is true (transfer to the initiator)... [after]
	   REQ is false the initiator shall then negate ACK."
	   
	   "If I/O is false (transfer to the target)... [when]
	   REQ becomes false at the initiator, the initiator may 
	   change or release DB(7-0,P) and shall negate ACK." */
      case 1:
	if (control & TME_SCSI_SIGNAL_REQ) {
	  break;
	}
	
	/* unless the initiator wants ACK held: */
	if ((dma_initiator->tme_scsi_connection_int_actions_waiting
	     & TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK) 
	    != TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK) {

	  /* negate ACK on the initiator's behalf: */
	  dma_initiator->tme_scsi_connection_int_control
	    &= ~TME_SCSI_SIGNAL_ACK;
	  again = TRUE;
	  bus_changed = TRUE;
	}
	
	/* if the DMA has been exhausted, callout a cycle on this
	   device: */
	if (control & TME_SCSI_SIGNAL_I_O) {
	  dma->tme_scsi_dma_in++;
	}
	else {
	  dma->tme_scsi_dma_out++;
	}
	if (--dma->tme_scsi_dma_resid == 0) {
	  dma_initiator->tme_scsi_connection_int_callout_flags
	    |= TME_SCSI_BUS_CALLOUT_CYCLE;
	  dma_initiator->tme_scsi_connection_int_actions_taken
	    |= dma_initiator->tme_scsi_connection_int_actions_waiting;
	  dma_initiator->tme_scsi_connection_int_actions_waiting
	    = TME_SCSI_ACTION_NONE;
	  new_callouts |= TME_SCSI_BUS_CALLOUT_CYCLE;
	}
	
	/* otherwise, advance to step zero: */
	else {
	  dma_initiator->tme_scsi_connection_int_sequence_step = 0;
	}
	break;
	
      default:
	assert (FALSE);
      }

      /* if the bus is being reset: */
      if (control & TME_SCSI_SIGNAL_RST) {

	/* negate all signals on the initiator's behalf: */
	dma_initiator->tme_scsi_connection_int_control = 0;
	dma_initiator->tme_scsi_connection_int_data = 0;
	again = TRUE;
	bus_changed = TRUE;
	
	/* callout a cycle on this device: */
	dma_initiator->tme_scsi_connection_int_callout_flags
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
	dma_initiator->tme_scsi_connection_int_actions_waiting
	  = TME_SCSI_ACTION_NONE;
	dma_initiator->tme_scsi_connection_int_actions_taken
	  = TME_SCSI_ACTION_NONE;
	dma_initiator->tme_scsi_connection_int_events_triggered
	  = TME_SCSI_EVENT_BUS_RESET;
	new_callouts
	  |= TME_SCSI_BUS_CALLOUT_CYCLE;
      }
    }
  }

  /* make any needed callouts: */
  _tme_scsi_bus_callout(scsi_bus, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&scsi_bus->tme_scsi_bus_mutex);

  return (TME_OK);
}

/* this scores a new connection: */
static int
_tme_scsi_bus_connection_score(struct tme_connection *conn,
			       unsigned int *_score)
{
  struct tme_scsi_bus *scsi_bus;
  struct tme_scsi_connection *conn_other;

  /* both sides must be SCSI connections: */
  assert (conn->tme_connection_type == TME_CONNECTION_SCSI);
  assert (conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SCSI);

  /* recover our bus and the other internal connection side: */
  scsi_bus = conn->tme_connection_element->tme_element_private;
  conn_other = (struct tme_scsi_connection *) conn->tme_connection_other;

  /* you cannot connect a bus to a bus: */
  /* XXX we need a way to distinguish a bus from a device: */
  *_score
    = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_scsi_bus_connection_make(struct tme_connection *conn,
			      unsigned int state)
{
  struct tme_scsi_bus *scsi_bus;
  struct tme_scsi_connection_int *conn_int;

  /* both sides must be SCSI connections: */
  assert (conn->tme_connection_type == TME_CONNECTION_SCSI);
  assert (conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SCSI);

  /* recover our bus and our internal connection side: */
  scsi_bus = conn->tme_connection_element->tme_element_private;
  conn_int = (struct tme_scsi_connection_int *) conn;
  
  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock the mutex: */
    tme_mutex_lock(&scsi_bus->tme_scsi_bus_mutex);

    /* add this connection to our list of connections: */
    conn->tme_connection_next = scsi_bus->tme_scsi_bus_connections;
    scsi_bus->tme_scsi_bus_connections = conn;

    /* unlock the mutex: */
    tme_mutex_unlock(&scsi_bus->tme_scsi_bus_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_scsi_bus_connection_break(struct tme_connection *conn,
			       unsigned int state)
{
  abort();
}

/* this returns the new connections possible: */
static int
_tme_scsi_bus_connections_new(struct tme_element *element,
			      const char * const *args,
			      struct tme_connection **_conns,
			      char **_output)
{
  struct tme_scsi_connection_int *conn_int;
  struct tme_scsi_connection *conn_scsi;
  struct tme_connection *conn;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* create our side of a SCSI connection: */
  conn_int = tme_new0(struct tme_scsi_connection_int, 1);
  conn_scsi = &conn_int->tme_scsi_connection_int;
  conn = &conn_scsi->tme_scsi_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_SCSI;
  conn->tme_connection_score = _tme_scsi_bus_connection_score;
  conn->tme_connection_make = _tme_scsi_bus_connection_make;
  conn->tme_connection_break = _tme_scsi_bus_connection_break;

  /* fill in the SCSI connection: */
  conn_scsi->tme_scsi_connection_cycle = _tme_scsi_bus_cycle;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* this creates a new SCSI bus element: */
TME_ELEMENT_SUB_NEW_DECL(tme_scsi,bus) {
  struct tme_scsi_bus *scsi_bus;
  int usage;
  int arg_i;

  /* check our arguments: */
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    if (0) {
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      break;
    }

    /* this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
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

  /* allocate and initialize the new SCSI bus: */
  scsi_bus = tme_new0(struct tme_scsi_bus, 1);
  tme_mutex_init(&scsi_bus->tme_scsi_bus_mutex);

  /* fill the element: */
  element->tme_element_private = scsi_bus;
  element->tme_element_connections_new = _tme_scsi_bus_connections_new;

  return (TME_OK);
}
