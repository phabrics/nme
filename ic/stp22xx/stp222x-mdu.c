/* $Id: stp222x-mdu.c,v 1.5 2010/06/05 18:59:29 fredette Exp $ */

/* ic/stp222x-mdu.c - emulation of the Mondo Dispatch Unit of the UPA
   to SBus interface controller (STP2220) and the UPA to PCI interface
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
_TME_RCSID("$Id: stp222x-mdu.c,v 1.5 2010/06/05 18:59:29 fredette Exp $");

/* includes: */
#include "stp222x-impl.h"

/* macros: */

/* an interrupt mapping register: */
#define TME_STP222X_MDU_IMR_INR			((2 << 10) - (1 << 0))
#define TME_STP222X_MDU_IMR_TID			((2 << 30) - (tme_uint32_t) (1 << 26))
#define TME_STP222X_MDU_IMR_V			TME_BIT(31)

/* interrupt states: */
#define TME_STP222X_MDU_STATE_IDLE		(0)
#define TME_STP222X_MDU_STATE_RECEIVED		(1)
#define TME_STP222X_MDU_STATE_PENDING		(3)

/* the interrupt retry timer register: */
#define TME_STP222X_MDU_RETRY_TIMER_LIMIT	((2 << 19) - (1 << 0))

/* dispatch states: */
#define TME_STP222X_MDU_DISPATCH_NOW		(0)
#define TME_STP222X_MDU_DISPATCH_RETRY(n)	(1 + (n))

/* priorities: */
#define TME_STP222X_MDU_PRIORITY_NULL		(9)

/* this updates an IDI's bit in the received or pending sets: */
#define TME_STP222X_MDU_IDIS_UPDATE(field, op, idi)	\
  do {							\
    field[(idi)						\
	  / (sizeof(tme_stp222x_idis_t) * 8)]		\
      op (((tme_stp222x_idis_t) 1)			\
          << ((idi)					\
	      % (sizeof(tme_stp222x_idis_t) * 8)));	\
  } while (/* CONSTCOND */ 0)

/* this tests an IDI's bit in an IDI set: */
#define TME_STP222X_MDU_IDI_TEST(field, idi)		\
  (field[(idi)						\
	 / (sizeof(tme_stp222x_idis_t) * 8)]		\
   & (((tme_stp222x_idis_t) 1)				\
      << ((idi)						\
	  % (sizeof(tme_stp222x_idis_t) * 8))))

/* globals: */

/* the stp2220 obio interrupt priorities: */
static const tme_uint8_t _tme_stp2220_mdu_idi_obio_priority[] = {
  /* TME_STP222X_IDI_SCSI */		3,
  /* TME_STP222X_IDI_ETHER */		3,
  /* TME_STP222X_IDI_BPP */		2,
  /* TME_STP2220_IDI_AUDIO */		8,
  /* TME_STP2220_IDI_POWER */ 		8,
  /* TME_STP2220_IDI_ZS0_ZS1 */ 	7,
  /* TME_STP2220_IDI_FD */		8,
  /* TME_STP2220_IDI_THERM */		8,
  /* TME_STP2220_IDI_KBD */		4,
  /* TME_STP2220_IDI_MOUSE */		4,
  /* TME_STP2220_IDI_SERIAL */		7,
  /* TME_STP2220_IDI_TIMER(0) */	6,
  /* TME_STP2220_IDI_TIMER(1) */	6,
  /* TME_STP2220_IDI_UE */		8,
  /* TME_STP2220_IDI_CE */		8,
  /* TME_STP2220_IDI_SBUS_ASYNC */	8,
  /* TME_STP2220_IDI_POWER_MANAGE */	1,
  /* TME_STP2220_IDI_UPA */		5,
  /* TME_STP2220_IDI_RESERVED */	5,
};

/* this maps a register group index into an IDI for an obio
   interrupt: */
static tme_uint32_t
_tme_stp222x_reggroup_index_to_obio_idi(struct tme_stp222x *stp222x, 
					tme_uint32_t reggroup_index)
{
  tme_uint32_t idi;

  /* assume that the register group index maps directly to IDI: */
  idi = TME_STP222X_IDI0_OBIO + reggroup_index;

  /* if this is an stp2220: */
  if (TME_STP222X_IS_2220(stp222x)) {

    /* there is a one-register gap before the first timer interrupt: */
    if (idi > TME_STP2220_IDI_TIMER(0)) {
      idi--;
    }
  }

  return (idi);
}

/* the stp222x interrupt retry thread: */
static void
_tme_stp222x_mdu_retry_th(void *_stp222x)
{
  struct tme_stp222x *stp222x;
  struct timeval *sleep;
  signed long buffer_i;
  unsigned int dispatch_state;

  /* recover our data structures: */
  stp222x = (struct tme_stp222x *) _stp222x;

  /* enter: */
  tme_stp22xx_enter(&stp222x->tme_stp222x);

  /* loop forever: */
  for (;;) {

    /* assume that we can block forever: */
    sleep = NULL;

    /* loop over the dispatch buffers: */
    buffer_i = TME_STP222X_MDU_BUFFER_COUNT - 1;
    do {

      /* if this dispatch buffer is valid: */
      if (stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] != !TME_STP222X_MDU_IMR_V) {

	/* if this dispatch buffer is retrying: */
	dispatch_state = stp222x->tme_stp222x_mdu_dispatch_state[buffer_i];
	if (dispatch_state != TME_STP222X_MDU_DISPATCH_NOW) {

	  /* NB: we sleep three times for each buffer before retrying
	     it.  since the sleep time is half of the retry interval,
	     this means that the maximum delay is one a half times the
	     retry interval, which is the expected value of the delay.
	     the minimum delay is exactly the retry interval, and this
	     happens only on the stp2222, when the other buffer needs
	     a retry right at the beginning of one of the sleeps for
	     this buffer.

	     it's impossible for more than one of the three sleeps a
	     buffer does to be interrupted in this way, because there
	     is not more than one other buffer: */

	  /* if this dispatch buffer has slept three times: */
	  if (dispatch_state == TME_STP222X_MDU_DISPATCH_RETRY(3)) {

	    /* it's time to try this buffer again: */
	    dispatch_state = TME_STP222X_MDU_DISPATCH_NOW;
	  }

	  /* otherwise, we need to sleep for this buffer: */
	  else {

	    /* advance the retry state: */
	    dispatch_state++;

	    /* sleep: */
	    sleep = &stp222x->tme_stp222x_mdu_retry_sleep;
	  }

	  /* update this buffer's dispatch state: */
	  stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] = dispatch_state;
	}
      }
    } while (--buffer_i >= 0);

    /* block: */
    tme_stp22xx_cond_sleep_yield(&stp222x->tme_stp222x,
				 &stp222x->tme_stp222x_mdu_retry_cond,
				 sleep);
  }
  /* NOTREACHED */
}

/* this decodes and arbitrates interrupts: */
static void
_tme_stp222x_mdu_decode_arbitrate(struct tme_stp222x *stp222x)
{
  signed long buffer_i;
  tme_uint32_t chosen_idi[2];
  tme_uint32_t chosen_imr[2];
  unsigned int chosen_priority[2];
  signed long idis_i;
  tme_stp222x_idis_t idis_arbitrate;
  tme_uint32_t idi;
  tme_uint32_t imr;
  unsigned int priority;

  /* start with no chosen interrupts for any CPU buffer, but poison
     any full CPU buffer with an impossibly high priority, to avoid
     overwriting a dispatching interrupt: */
  buffer_i = TME_STP222X_MDU_BUFFER_COUNT - 1;
  do {
    chosen_idi[buffer_i] = TME_STP222X_IDI_NULL;
    chosen_imr[buffer_i] = !TME_STP222X_MDU_IMR_V;
    chosen_priority[buffer_i]
      = (stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] == !TME_STP222X_MDU_IMR_V
	 ? 0
	 : TME_STP222X_MDU_PRIORITY_NULL);
  } while (--buffer_i >= 0);

  /* loop over sets of IDIs: */
  idis_i = TME_ARRAY_ELS(stp222x->tme_stp222x_mdu_idis_received) - 1;
  do {

    /* get another set of IDIs that have been received, but are not
       pending: */
    idis_arbitrate
      = (stp222x->tme_stp222x_mdu_idis_received[idis_i]
	 & ~stp222x->tme_stp222x_mdu_idis_pending[idis_i]);
    if (idis_arbitrate != 0) {

      /* loop over the IDIs in this set: */
      idi = idis_i * (sizeof(idis_arbitrate) * 8);
      do {
	for (; (idis_arbitrate & 1) == 0; idi++, idis_arbitrate >>=1);

	/* get the IMR for this IDI: */
	imr = stp222x->tme_stp222x_mdu_imrs[idi];

	/* if this IMR is valid: */
	if (imr & TME_STP222X_MDU_IMR_V) {

	  /* if this is an stp2220: */
	  if (TME_STP222X_IS_2220(stp222x)) {

	    /* if this is a card IDI: */
	    if (idi < TME_STP222X_IDI0_OBIO) {

	      /* the SBus priority is our priority: */
	      priority = idi % TME_SBUS_SLOT_INTS;
	    }

	    /* otherwise, this is an obio IDI: */
	    else {

	      /* get the priority for this obio IDI: */
	      assert ((idi - TME_STP222X_IDI0_OBIO)
		      < TME_ARRAY_ELS(_tme_stp2220_mdu_idi_obio_priority));
	      priority = _tme_stp2220_mdu_idi_obio_priority[idi - TME_STP222X_IDI0_OBIO];
	    }

	    /* all stp2220 interrupts use buffer zero: */
	    buffer_i = 0;
	  }

	  /* otherwise, this is an stp2222: */
	  else {
	    abort();
	  }

	  /* update the chosen IDI: */
	  if (priority > chosen_priority[buffer_i]) {
	    chosen_idi[buffer_i] = idi;
	    chosen_imr[buffer_i] = imr;
	    chosen_priority[buffer_i] = priority;
	  }
	}

	/* otherwise, this IMR is not valid: */
	else {

	  /* clear this IDI's received bit: */
	  /* XXX FIXME - is this right? shouldn't it stay received,
	     but just never be pending? */
	  TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_received, &= ~, idi);
	}

	/* advance: */
	idi++;
	idis_arbitrate >>= 1;
      } while (idis_arbitrate != 0);
    }
  } while (--idis_i >= 0);

  /* update the dispatching interrupts: */
  buffer_i = TME_STP222X_MDU_BUFFER_COUNT - 1;
  do {
    imr = chosen_imr[buffer_i];
    if (imr != !TME_STP222X_MDU_IMR_V) {
      stp222x->tme_stp222x_mdu_dispatch_idi[buffer_i] = chosen_idi[buffer_i];
      stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] = imr;
      assert (stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] == TME_STP222X_MDU_DISPATCH_NOW);
    }
  } while (--buffer_i >= 0);
}

/* this completes dispatch of an interrupt: */
static void
_tme_stp222x_mdu_dispatch_complete(struct tme_stp22xx *stp22xx,
				   struct tme_completion *completion,
				   void *arg)
{
  struct tme_stp222x *stp222x;
  signed long buffer_i;
  int error;
  tme_uint32_t idi;

  /* recover our data structure: */
  stp222x = (struct tme_stp222x *) stp22xx;

  /* get the interrupt buffer that was dispatched: */
  buffer_i = stp222x->tme_stp222x_mdu_dispatch_buffer;
  assert (stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] != !TME_STP222X_MDU_IMR_V);
  assert (stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] == TME_STP222X_MDU_DISPATCH_NOW);

  /* get any error code: */
  error = completion->tme_completion_error;

  /* if this interrupt was ACKed: */
  if (error == TME_OK) {

    /* clear the dispatched interrupt: */
    stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] = !TME_STP222X_MDU_IMR_V;

    /* get the IDI that was dispatched: */
    idi = stp222x->tme_stp222x_mdu_dispatch_idi[buffer_i];

    /* if the dispatched interrupt is pulse-driven: */
    /* XXX FIXME - we assume that TME_STP2220_IDI_RESERVED is pulse-driven: */
    if (TME_STP222X_IS_2220(stp222x)
	? (idi == TME_STP2220_IDI_UPA
	   || idi == TME_STP2220_IDI_RESERVED)
	: (idi == TME_STP2222_IDI_FFB0
	   || idi == TME_STP2222_IDI_FFB1)) {

      /* nothing to do */
    }

    /* otherwise, the dispatched interrupt is level-driven: */
    else {

      /* set this IDI's pending bit: */
      TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_pending, |= , idi);
    }

    /* decode and arbitrate again: */
    _tme_stp222x_mdu_decode_arbitrate(stp222x);
  }

  /* otherwise, if interrupt was NACKed: */
  else if (error == EAGAIN) {

    /* we need to retry this interrupt buffer later: */
    stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] = TME_STP222X_MDU_DISPATCH_RETRY(0);

    /* wake up the retry thread: */
    tme_stp22xx_cond_notify(&stp222x->tme_stp222x_mdu_retry_cond);
  }

  /* the interrupt target must not exist: */
  else {
    assert (error == ENOENT);

    /* XXX FIXME - what happens in this case? */
    abort();
  }

  /* round-robin the interrupt buffer to dispatch: */
  buffer_i = (buffer_i + 1) % TME_STP222X_MDU_BUFFER_COUNT;
  stp222x->tme_stp222x_mdu_dispatch_buffer = buffer_i;

  /* unused: */
  arg = 0;
}

/* this dispatches an interrupt: */
int
tme_stp222x_mdu_dispatch(struct tme_stp222x *stp222x)
{
  unsigned long buffer_i;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_completion *completion;
  tme_uint32_t imr;
  tme_uint32_t mid;
  tme_uint64_t interrupt_data[8];
  struct tme_upa_bus_connection *conn_upa_other;

  /* find a full interrupt buffer: */
  buffer_i = stp222x->tme_stp222x_mdu_dispatch_buffer;
  for (;;) {

    /* if this interrupt buffer is full, and can be dispatched now: */
    if (stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i] != !TME_STP222X_MDU_IMR_V
	&& stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] == TME_STP222X_MDU_DISPATCH_NOW) {
      break;
    }

    /* round-robin to the next interrupt buffer: */
    buffer_i = (buffer_i + 1) % TME_STP222X_MDU_BUFFER_COUNT;

    /* if all interrupt buffers are empty: */
    if (buffer_i == stp222x->tme_stp222x_mdu_dispatch_buffer) {

      /* we didn't dispatch an interrupt: */
      return (FALSE);
    }
  }
  stp222x->tme_stp222x_mdu_dispatch_buffer = buffer_i;

  /* busy the UPA bus connection: */
  conn_upa = tme_stp222x_busy_upa(stp222x);

  /* allocate a completion: */
  completion
    = tme_stp222x_completion_alloc(stp222x,
				   _tme_stp222x_mdu_dispatch_complete,
				   (void *) NULL);

  /* get the interrupt information: */
  imr = stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i];
  mid = TME_FIELD_MASK_EXTRACTU(imr, TME_STP222X_MDU_IMR_TID);
  memset(interrupt_data, 0, sizeof(interrupt_data));
  interrupt_data[0] = tme_htobe_u64(TME_FIELD_MASK_EXTRACTU(imr, TME_STP222X_MDU_IMR_INR));

  /* leave: */
  tme_stp222x_leave(stp222x);

  /* call out the interrupt: */
  conn_upa_other = (struct tme_upa_bus_connection *) conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_other;
  (*conn_upa_other->tme_upa_bus_interrupt)
    (conn_upa_other,
     mid,
     interrupt_data,
     completion);

  /* reenter: */
  stp222x = tme_stp222x_enter_bus(&conn_upa->tme_upa_bus_connection);

  /* unbusy the UPA bus connection: */
  tme_stp222x_unbusy_bus(stp222x, &conn_upa->tme_upa_bus_connection);

  /* we dispatched an interrupt: */
  return (TRUE);
}

/* this receives an interrupt: */
void
tme_stp222x_mdu_receive(struct tme_stp222x *stp222x,
			tme_uint32_t idi)
{

  /* set this IDI's received bit: */
  TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_received, |= , idi);

  /* decode and arbitrate again: */
  _tme_stp222x_mdu_decode_arbitrate(stp222x);
}

/* this updates the interrupt concentrator: */
void
tme_stp222x_mdu_intcon(struct tme_stp222x *stp222x,
		       tme_uint32_t idi,
		       tme_uint32_t level)
{

  /* NB: the interrupt concentrator is really in the RIC chip
     (STP2210): */

  /* if this is an stp2220, and this is the zs0/zs1 IDI: */
  if (TME_STP222X_IS_2220(stp222x)
      && idi == TME_STP2220_IDI_ZS0_ZS1) {

    /* the interrupt signals from zs0 and zs1 are wired together
       somewhere in a real system, probably inside the sym89c105
       (which is apparently not identical to an ncr89c105, because the
       latter doesn't even have output pins for its individual
       functions' interrupts).  we have to mimic this wiring here: */

    /* if the interrupt signal is being asserted, increase the active
       count, otherwise decrease the active count.  the active count
       (which is unsigned) can never be greater than two: */
    assert (level == TME_BUS_SIGNAL_LEVEL_ASSERTED
	    || level == TME_BUS_SIGNAL_LEVEL_NEGATED);
    stp222x->tme_stp2220_mdu_idi_zs0_zs1_active
      += (level == TME_BUS_SIGNAL_LEVEL_ASSERTED
	  ? 1
	  : -1);
    assert (stp222x->tme_stp2220_mdu_idi_zs0_zs1_active <= 2);

    /* if the interrupt signal is being asserted, but the active count
       was already one, or if the interrupt signal is being negated,
       but the active count is still one: */
    if ((level == TME_BUS_SIGNAL_LEVEL_ASSERTED)
	!= stp222x->tme_stp2220_mdu_idi_zs0_zs1_active) {

      /* the state of the interrupt signal at the RIC chip is
	 unchanged: */
      return;
    }
  }

  /* if this interrupt signal is being asserted: */
  if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {

    /* set this IDI's active bit: */
    TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_active, |= , idi);

    /* receive this interrupt: */
    tme_stp222x_mdu_receive(stp222x, idi);
  }

  /* otherwise, this interrupt signal must be being negated: */
  else {
    assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED);

    /* clear this IDI's active bit: */
    TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_active, &= ~, idi);
  }
}

/* this recalculates the interrupt retry period: */
static void
_tme_stp222x_mdu_retry_set(struct tme_stp222x *stp222x)
{
  tme_uint64_t sleep_usec;

  /* the retry timer has a period of 15.7ms at the maximum limit of
     2^20 ticks.  calculate half of the period of the retry timer for
     _tme_stp222x_mdu_retry_th(): */
  sleep_usec = TME_FIELD_MASK_EXTRACTU(stp222x->tme_stp222x_mdu_retry, TME_STP222X_MDU_RETRY_TIMER_LIMIT) + 1;
  sleep_usec *= 15700;
#if (TME_STP222X_MDU_RETRY_TIMER_LIMIT & 1) == 0
#error "TME_STP222X_MDU_RETRY_TIMER_LIMIT changed"
#endif
  sleep_usec = (sleep_usec + TME_STP222X_MDU_RETRY_TIMER_LIMIT) / (TME_STP222X_MDU_RETRY_TIMER_LIMIT + 1);
  stp222x->tme_stp222x_mdu_retry_sleep.tv_sec = 0;
  stp222x->tme_stp222x_mdu_retry_sleep.tv_usec = sleep_usec;
}

/* the MDU IMR and retry register handler: */
void
tme_stp222x_mdu_regs_imr_retry(struct tme_stp222x *stp222x,
			       struct tme_stp222x_reg *reg)
{
  tme_uint32_t reggroup;
  tme_uint32_t reggroup_index;
  tme_uint32_t imr_partial;
  tme_uint32_t idi;
  const char *name;

  /* get the register: */
  reggroup = TME_STP222X_REGGROUP_WHICH(reg->tme_stp222x_reg_address);
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* assume that this is a write to a partial IMR, and get a partial
     IMR value: */
  imr_partial
    = (reg->tme_stp222x_reg_value
       & (TME_STP222X_MDU_IMR_TID
	  | TME_STP222X_MDU_IMR_V));

  /* assume that this is a register for an obio interrupt: */
  idi = _tme_stp222x_reggroup_index_to_obio_idi(stp222x, reggroup_index);

  /* dispatch on the register: */
  name = NULL;
  switch (reggroup) {

    /* the stp2220 SBus card IMRs and the retry register: */
  case 0x2c:
    if (__tme_predict_false(!TME_STP222X_IS_2220(stp222x))) {
      return;
    }

    /* if this is an SBus card IMR: */
    if (reggroup_index < TME_STP2220_SLOTS_CARD) {

      /* get the IDI for this card's ipl 0 interrupt.  SBus ipl 0
	 doesn't really exist, but the partial IMR for this IDI will
	 have zeros in the ipl position, which is what a read must
	 return: */
      idi = reggroup_index * TME_SBUS_SLOT_INTS;

      /* if this is a write: */
      if (reg->tme_stp222x_reg_write) {

	/* write the partial IMRs for all of this card's ipls, only
	   updating the V and TID fields: */
	do {
	  stp222x->tme_stp222x_mdu_imrs[idi]
	    = ((stp222x->tme_stp222x_mdu_imrs[idi]
		& ~(TME_STP222X_MDU_IMR_TID
		    | TME_STP222X_MDU_IMR_V))
	       | imr_partial);
	} while (++idi % TME_SBUS_SLOT_INTS);
	idi -= TME_SBUS_SLOT_INTS;
      }

      /* otherwise, this is a read: */
      else {

	/* read the partial IMR for this card: */
	reg->tme_stp222x_reg_value = stp222x->tme_stp222x_mdu_imrs[idi];
      }
      break;
    }
    /* FALLTHROUGH */

    /* the STP2222 retry register: */
  case 0x1a:
    if (__tme_predict_false(reg->tme_stp222x_reg_address
			    != (TME_STP222X_IS_2220(stp222x)
				? 0x2c20
				: 0x1a00))) {
      return;
    }
    if (reg->tme_stp222x_reg_write) {
      stp222x->tme_stp222x_mdu_retry = reg->tme_stp222x_reg_value;
      _tme_stp222x_mdu_retry_set(stp222x);
    }
    else {
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_mdu_retry;
    }
    name = "RETRY";
    break;

    /* the STP2222 PCI card IMRs: */
  case 0x0c:
    if (__tme_predict_false(TME_STP222X_IS_2220(stp222x))) {
      return;
    }

    /* if this is not a PCI card IMR: */
    idi = reggroup_index * TME_PCI_SLOT_INTS;
    if (__tme_predict_false((((1 << TME_STP2222_IDI_CARD(0, 0, 0))
			      | (1 << TME_STP2222_IDI_CARD(0, 1, 0))
			      | (1 << TME_STP2222_IDI_CARD(1, 0, 0))
			      | (1 << TME_STP2222_IDI_CARD(1, 1, 0))
			      | (1 << TME_STP2222_IDI_CARD(1, 2, 0))
			      | (1 << TME_STP2222_IDI_CARD(1, 3, 0)))
			     & (1 << idi)) == 0)) {
      return;
    }

    /* if this is a write: */
    if (reg->tme_stp222x_reg_write) {

      /* write the partial IMRs for all of this PCI card's INTx, only
	 updating the V and TID fields: */
      do {
	stp222x->tme_stp222x_mdu_imrs[idi]
	  = ((stp222x->tme_stp222x_mdu_imrs[idi]
	      & ~(TME_STP222X_MDU_IMR_TID
		  | TME_STP222X_MDU_IMR_V))
	     | imr_partial);
      } while (++idi % TME_PCI_SLOT_INTS);
      idi -= TME_PCI_SLOT_INTS;
    }

    /* otherwise, this is a read: */
    else {

      /* read the partial IMR for this PCI card's INTA: */
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_mdu_imrs[idi];
    }
    break;

    /* the STP2222 alternate FFB0 and FFB1 IMRs: */
  case 0x60:
  case 0x80:
    idi = (reggroup == 0x60 ? TME_STP2222_IDI_FFB0 : TME_STP2222_IDI_FFB1);
    reggroup = 0x10;
    /* FALLTHROUGH */

    /* the obio IMRs: */
  default:

    /* if this is the wrong obio IMR register group for this part, or
       if this is not an obio IMR, return failure: */
    if (TME_STP222X_IS_2220(stp222x)) {
      if (__tme_predict_false(reggroup != 0x30
			      || idi > TME_STP2220_IDI_RESERVED)) {
	return;
      }
    }
    else {
      if (__tme_predict_false(reggroup != 0x10
			      || idi > TME_STP2222_IDI_FFB1)) {
	return;
      }
    }

    /* if this is an obio IMR write: */
    if (reg->tme_stp222x_reg_write) {

      /* if this is a obio full IMR write: */
      if (TME_STP222X_IS_2220(stp222x)
	  ? (idi == TME_STP2220_IDI_UPA
	     || idi == TME_STP2220_IDI_RESERVED)
	  : (idi == TME_STP2222_IDI_FFB0
	     || idi == TME_STP2222_IDI_FFB1)) {

	/* do the obio full IMR write: */
	stp222x->tme_stp222x_mdu_imrs[idi]
	  = (reg->tme_stp222x_reg_value
	     & (TME_STP222X_MDU_IMR_INR
		| TME_STP222X_MDU_IMR_TID
		| TME_STP222X_MDU_IMR_V));
      }

      /* otherwise, this is a obio partial IMR write: */
      else {

	/* do the obio partial IMR write: */
	stp222x->tme_stp222x_mdu_imrs[idi]
	  = ((stp222x->tme_stp222x_mdu_imrs[idi]
	      & ~(TME_STP222X_MDU_IMR_TID
		  | TME_STP222X_MDU_IMR_V))
	     | imr_partial);
      }
    }

    /* otherwise, read the obio IMR: */
    else {
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_mdu_imrs[idi];
    }
    break;
  }
  
  if (name == NULL) {
    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("MDU IMR[0x%x] %s 0x%" TME_PRIx64),
	     idi,
	     (reg->tme_stp222x_reg_write
	      ? "<-"
	      : "->"),
	     reg->tme_stp222x_reg_value));
  }
  else {
    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("MDU %s %s 0x%" TME_PRIx64),
	     name,
	     (reg->tme_stp222x_reg_write
	      ? "<-"
	      : "->"),
	     reg->tme_stp222x_reg_value));
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* the MDU clear register handler: */
void
tme_stp222x_mdu_regs_clear(struct tme_stp222x *stp222x,
			   struct tme_stp222x_reg *reg)
{
  tme_uint32_t reggroup;
  tme_uint32_t reggroup_index;
  tme_uint32_t idi;
  tme_uint32_t mdu_state;
  tme_uint32_t imr;
  unsigned long buffer_i;

  /* get the register: */
  reggroup = TME_STP222X_REGGROUP_WHICH(reg->tme_stp222x_reg_address);
  reggroup_index = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);

  /* assume that this is a register for an obio interrupt: */
  idi = _tme_stp222x_reggroup_index_to_obio_idi(stp222x, reggroup_index);

  /* dispatch on the register: */
  switch (reggroup) {

    /* the STP2220 SBus and obio card clears: */
  case 0x34:
    idi = reggroup_index;
    /* FALLTHROUGH */
  case 0x38:
    if (__tme_predict_false(!TME_STP222X_IS_2220(stp222x))) {
      return;
    }
    if (__tme_predict_false(idi > TME_STP2220_IDI_POWER_MANAGE)) {
      return;
    }
    break;

    /* the STP2222 PCI card clears: */
  case 0x14:
    if (__tme_predict_false(TME_STP222X_IS_2220(stp222x))) {
      return;
    }

    /* if this is not a PCI card clear register: */
    idi = reggroup_index;
    if (idi >= TME_STP2222_IDI_CARD(0, 2, 0)
	&& idi < TME_STP2222_IDI_CARD(1, 0, 0)) {
      return;
    }
    break;

    /* the STP2222 obio clears: */
  default:
    assert (reggroup == 0x10);
    if (__tme_predict_false(TME_STP222X_IS_2220(stp222x))) {
      return;
    }
    if (__tme_predict_false(idi > TME_STP2222_IDI_POWER_MANAGE)) {
      return;
    }
    break;
  }

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {

    /* update the MDU state for this IDI: */
    mdu_state = reg->tme_stp222x_reg_value;
    if ((mdu_state & TME_STP222X_MDU_STATE_RECEIVED)
	|| TME_STP222X_MDU_IDI_TEST(stp222x->tme_stp222x_mdu_idis_active, idi)) {
      TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_received, |= , idi);
    }
    else {
      TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_received, &= ~, idi);
    }
    if (mdu_state == TME_STP222X_MDU_STATE_PENDING) {
      TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_pending, |= , idi);
    }
    else {
      TME_STP222X_MDU_IDIS_UPDATE(stp222x->tme_stp222x_mdu_idis_pending, &= ~, idi);

      /* get the IMR for this IDI: */
      imr = stp222x->tme_stp222x_mdu_imrs[idi];

      /* loop over the interrupt dispatch buffers: */
      for (buffer_i = 0; buffer_i < TME_STP222X_MDU_BUFFER_COUNT; buffer_i++) {

	/* if this interrupt dispatch buffer has the same V and TID
	   values as the IMR for this IDI: */
	if (((stp222x->tme_stp222x_mdu_dispatch_imr[buffer_i]
	      ^ imr)
	     & (TME_STP222X_MDU_IMR_V
		+ TME_STP222X_MDU_IMR_TID)) == 0) {

	  /* assume that this interrupt buffer has its V bit set, and
	     force it to dispatch now (if its V bit is clear, this
	     should not change its dispatch state): */
	  /* NB: this is a hack to improve interrupt latency when the
	     retry thread has high latency (due to poor
	     tme_cond_sleep_yield() sleep resolution): whenever a CPU
	     writes an MDU state other than pending for an IDI, assume
	     that the CPU can accept another interrupt and force any
	     interrupt waiting to dispatch to that CPU to dispatch
	     immediately: */
	  assert ((imr & TME_STP222X_MDU_IMR_V)
		  || (stp222x->tme_stp222x_mdu_dispatch_state[buffer_i]
		      == TME_STP222X_MDU_DISPATCH_NOW));
	  stp222x->tme_stp222x_mdu_dispatch_state[buffer_i] = TME_STP222X_MDU_DISPATCH_NOW;
	}
      }
    }

    /* decode and arbitrate again: */
    _tme_stp222x_mdu_decode_arbitrate(stp222x);

    tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	    (TME_STP222X_LOG_HANDLE(stp222x),
	     _("MDU clear[0x%x] <- 0x%" TME_PRIx32),
	     idi,
	     mdu_state));
  }

  /* otherwise, this is a read: */
  else {
    reg->tme_stp222x_reg_value = 0;
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* the MDU diagnostic register handler: */
void
tme_stp222x_mdu_regs_diag(struct tme_stp222x *stp222x,
			  struct tme_stp222x_reg *reg)
{
  unsigned long idis_i;
  tme_stp222x_idis_t idis_received;
  tme_stp222x_idis_t idis_pending;
  tme_uint32_t bit;
  tme_uint32_t value_32_63;
  tme_uint32_t value_0_31;

  /* check the register and cycle type: */
  assert (sizeof(tme_stp222x_idis_t) == sizeof(tme_uint32_t));
  idis_i = TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address);
  if (__tme_predict_false(idis_i > TME_ARRAY_ELS(stp222x->tme_stp222x_mdu_idis_received))) {
    return;
  }
  if (__tme_predict_false(reg->tme_stp222x_reg_write)) {
    return;
  }

  /* get the selected internal received and pending registers: */
  idis_received = stp222x->tme_stp222x_mdu_idis_received[idis_i];
  idis_pending = stp222x->tme_stp222x_mdu_idis_pending[idis_i];

  /* if this diagnostic register covers the obio interrupts: */
  if (idis_i == 1) {

    /* both parts have two pulse-driven interrupts at the end of the
       diagnostic register, using only one bit each instead of the two
       bits each that the level-driven interrupts use.  we move the
       received bit of the last pulse-driven interrupt into the
       pending bit for the next to last pulse-driven interrupt, to
       make the two pulse-driven interrupts look like a single
       level-driven interrupt: */
    if (TME_STP222X_IS_2220(stp222x)) {
      idis_pending
	|= ((idis_received & (1 << (TME_STP2220_IDI_RESERVED - TME_STP222X_IDI0_OBIO)))
	    >> (TME_STP2220_IDI_RESERVED - TME_STP2220_IDI_UPA));
      idis_received &= ~ (tme_uint32_t) (1 << (TME_STP2220_IDI_RESERVED - TME_STP222X_IDI0_OBIO));
    }
    else {
      idis_pending
	|= ((idis_received & (1 << (TME_STP2222_IDI_FFB1 - TME_STP222X_IDI0_OBIO)))
	    >> (TME_STP2222_IDI_FFB1 - TME_STP2222_IDI_FFB0));
      idis_received &= ~ (tme_uint32_t) (1 << (TME_STP2222_IDI_FFB1 - TME_STP222X_IDI0_OBIO));
    }
  }

  /* make bits 32..63 of the value: */
  bit = ((tme_uint32_t) 1) << 31;
  value_32_63 = 0;
  do {
    if (idis_pending & (((tme_uint32_t) 1) << 31)) {
      value_32_63 += bit;
    }
    idis_pending <<= 1;
    bit >>= 1;
    if (idis_received & (((tme_uint32_t) 1) << 31)) {
      value_32_63 += bit;
    }
    idis_received <<= 1;
    bit >>= 1;
  } while (bit != 0);

  /* make bits 0..31 of the value: */
  value_0_31 = 0;
  bit = ((tme_uint32_t) 1) << 31;
  do {
    if (idis_pending & (((tme_uint32_t) 1) << 31)) {
      value_0_31 += bit;
    }
    idis_pending <<= 1;
    bit >>= 1;
    if (idis_received & (((tme_uint32_t) 1) << 31)) {
      value_0_31 += bit;
    }
    idis_received <<= 1;
    bit >>= 1;
  } while (bit != 0);

  reg->tme_stp222x_reg_value
    = ((((tme_uint64_t) value_32_63) << 32)
       | value_0_31);

  tme_log(TME_STP222X_LOG_HANDLE(stp222x), 2000, TME_OK,
	  (TME_STP222X_LOG_HANDLE(stp222x),
	   _("MDU DIAG -> 0x%" TME_PRIx64),
	   reg->tme_stp222x_reg_value));

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* this updates the IGN for the partial IMRs: */
void
tme_stp222x_mdu_ign_update(struct tme_stp222x *stp222x,
			   tme_uint32_t ign)
{
  tme_uint32_t idi;
  tme_uint32_t ino;

  /* loop over all IDIs: */
  for (idi = 0; idi < TME_STP222X_IDI_NULL; idi++) {

    /* if this IDI has a partial IMR: */
    if (TME_STP222X_IS_2220(stp222x)
	? (idi != TME_STP2220_IDI_UPA
	   && idi != TME_STP2220_IDI_RESERVED)
	: (idi != TME_STP2222_IDI_FFB0
	   && idi != TME_STP2222_IDI_FFB1)) {

      /* assume that the IDI is also the INO: */
      ino = idi;

      /* if this is an stp2220 obio interrupt: */
      if (TME_STP222X_IS_2220(stp222x)
	  && idi >= TME_STP222X_IDI0_OBIO) {

	/* the stp2220 obio interrupt to INO mapping is not regular: */
	switch (idi) {
	case TME_STP222X_IDI_SCSI: ino = 0x20; break;
	case TME_STP222X_IDI_ETHER: ino = 0x21; break;
	case TME_STP222X_IDI_BPP: ino = 0x22; break;
	case TME_STP2220_IDI_AUDIO: ino = 0x24; break;
	case TME_STP2220_IDI_POWER: ino = 0x25; break;
	case TME_STP2220_IDI_ZS0_ZS1: ino = 0x28; break;
	case TME_STP2220_IDI_FD: ino = 0x29; break;
	case TME_STP2220_IDI_THERM: ino = 0x2a; break;
	case TME_STP2220_IDI_KBD: ino = 0x2b; break;
	case TME_STP2220_IDI_MOUSE: ino = 0x2c; break;
	case TME_STP2220_IDI_SERIAL: ino = 0x2d; break;
	case TME_STP2220_IDI_TIMER(0): ino = 0x30; break;
	case TME_STP2220_IDI_TIMER(1): ino = 0x31; break;
	case TME_STP2220_IDI_UE: ino = 0x34; break;
	case TME_STP2220_IDI_CE: ino = 0x35; break;
	case TME_STP2220_IDI_SBUS_ASYNC: ino = 0x36; break;
	case TME_STP2220_IDI_POWER_MANAGE: ino = 0x37; break;
	case TME_STP2220_IDI_UPA: ino = 0x38; break;
	case TME_STP2220_IDI_RESERVED: ino = 0x39; break;
	default: break;
	}
      }

      /* update the INR in the IDI's IMR: */
      stp222x->tme_stp222x_mdu_imrs[idi]
	= ((stp222x->tme_stp222x_mdu_imrs[idi]
	    & ~TME_STP222X_MDU_IMR_INR)
	   + (ign * TME_STP222X_IDI_NULL)
	   + ino);
    }
  }
}

/* this initializes the MDU: */
void
tme_stp222x_mdu_init(struct tme_stp222x *stp222x)
{

  /* initialize the IMRs: */
  memset(stp222x->tme_stp222x_mdu_imrs, 0, sizeof(stp222x->tme_stp222x_mdu_imrs));
  tme_stp222x_mdu_ign_update(stp222x, 0);

  /* initialize the retry timer: */
  stp222x->tme_stp222x_mdu_retry = 0;
  _tme_stp222x_mdu_retry_set(stp222x);

  /* initialize the retry condition: */
  tme_stp22xx_cond_init(&stp222x->tme_stp222x_mdu_retry_cond);

  /* start the retry thread: */
  tme_thread_create(_tme_stp222x_mdu_retry_th, stp222x);
}

