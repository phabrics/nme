/* $Id: scsi.h,v 1.4 2007/01/07 23:59:59 fredette Exp $ */

/* tme/generic/scsi.h - header file for generic SCSI support: */

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

#ifndef _TME_GENERIC_SCSI_H
#define _TME_GENERIC_SCSI_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi.h,v 1.4 2007/01/07 23:59:59 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/memory.h>

/* macros: */

/* the SCSI control signals.  the parity signals are really data
   signals, but it's easiest to keep them here: */
#define TME_SCSI_SIGNAL_BSY		TME_BIT(0)
#define TME_SCSI_SIGNAL_SEL		TME_BIT(1)
#define TME_SCSI_SIGNAL_C_D		TME_BIT(2)
#define TME_SCSI_SIGNAL_I_O		TME_BIT(3)
#define TME_SCSI_SIGNAL_MSG		TME_BIT(4)
#define TME_SCSI_SIGNAL_REQ		TME_BIT(5)
#define TME_SCSI_SIGNAL_ACK		TME_BIT(6)
#define TME_SCSI_SIGNAL_ATN		TME_BIT(7)
#define TME_SCSI_SIGNAL_RST		TME_BIT(8)
#define TME_SCSI_SIGNAL_DBP		TME_BIT(9)
#define TME_SCSI_SIGNAL_DBP1		TME_BIT(10)

/* this evaluates to nonzero if only one of the bits in mask is set in data: */
#define _TME_SCSI_ID_SET(ids, data)		\
  (((data) & (ids)) != 0 && (((data) & (ids)) & (((data) & (ids)) - 1)) == 0)

/* "In all systems, the target shall determine that it is selected
   when SEL and its SCSI ID bit are true and BSY and I/O are false for
   at least a bus settle delay." */
#define TME_SCSI_ID_SELECTED(ids, control, data)\
  ((((control)					\
     & (TME_SCSI_SIGNAL_BSY			\
	| TME_SCSI_SIGNAL_SEL			\
	| TME_SCSI_SIGNAL_I_O))			\
    == TME_SCSI_SIGNAL_SEL)			\
   && _TME_SCSI_ID_SET(ids, data))

/* "The initiator shall determine that it is reselected when SEL, I/O,
   and its SCSI ID bit are true and BSY is false for at least a bus
   settle delay." */
#define TME_SCSI_ID_RESELECTED(ids, control, data)\
  ((((control)					\
     & (TME_SCSI_SIGNAL_BSY			\
	| TME_SCSI_SIGNAL_SEL			\
	| TME_SCSI_SIGNAL_I_O))			\
    == (TME_SCSI_SIGNAL_SEL			\
	| TME_SCSI_SIGNAL_I_O))			\
   && _TME_SCSI_ID_SET(ids, data))

/* this gets the current SCSI bus information transfer phase from a
   set of control signals: */
#define TME_SCSI_PHASE(c)		\
  ((c) & (TME_SCSI_SIGNAL_MSG		\
	  | TME_SCSI_SIGNAL_C_D		\
	  | TME_SCSI_SIGNAL_I_O))

/* the SCSI bus information transfer phases.  these are always tied to
   definite configurations of SCSI control signals after selection: */
#define TME_SCSI_PHASE_DATA_OUT		(0)
#define TME_SCSI_PHASE_DATA_IN		(TME_SCSI_SIGNAL_I_O)
#define TME_SCSI_PHASE_COMMAND		(TME_SCSI_SIGNAL_C_D)
#define TME_SCSI_PHASE_STATUS		(TME_SCSI_SIGNAL_C_D	\
					 | TME_SCSI_SIGNAL_I_O)
#define TME_SCSI_PHASE_MESSAGE_OUT	(TME_SCSI_SIGNAL_MSG	\
					 | TME_SCSI_SIGNAL_C_D)
#define TME_SCSI_PHASE_MESSAGE_IN	(TME_SCSI_SIGNAL_MSG	\
					 | TME_SCSI_SIGNAL_C_D	\
					 | TME_SCSI_SIGNAL_I_O)

/* the SCSI DMA flags: */
#define TME_SCSI_DMA_WIDTH		(0x03)
#define  TME_SCSI_DMA_8BIT		(0x00)
#define  TME_SCSI_DMA_16BIT		(0x01)
#define TME_SCSI_DMA_PARITY		(0x04)

/* the SCSI events: */
#define TME_SCSI_EVENT_NONE				(0)
#define  TME_SCSI_EVENT_IDS_SELF(ids)			(ids)
#define  TME_SCSI_EVENT_IDS_WHICH(event)		((event) & 0xffff)
#define TME_SCSI_EVENT_SELECTED				(TME_BIT(16))
#define TME_SCSI_EVENT_RESELECTED			(TME_BIT(17))
#define TME_SCSI_EVENT_BUS_FREE				(TME_BIT(18))
#define TME_SCSI_EVENT_BUS_CHANGE			(TME_BIT(19))
#define TME_SCSI_EVENT_BUS_RESET			(TME_BIT(20))

/* the SCSI actions.  these are deliberately ordered such that actions
   that have to be taken earlier have a larger value than those that
   have to be taken later: */
#define TME_SCSI_ACTION_NONE				(0)
#define  TME_SCSI_ACTION_ID_SELF(id)			((id) << 0)
#define  TME_SCSI_ACTION_ID_SELF_WHICH(actions)		(((actions) >> 0) & 0xf)
#define  TME_SCSI_ACTION_ID_OTHER(id)			((id) << 4)
#define  TME_SCSI_ACTION_ID_OTHER_WHICH(actions)	(((actions) >> 4) & 0xf)
#define TME_SCSI_ACTION_CYCLE_MARKER			(TME_BIT(8))
#define TME_SCSI_ACTION_DMA_INITIATOR			(TME_BIT(13))
#define TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK		(TME_BIT(14) | TME_SCSI_ACTION_DMA_INITIATOR)
#define TME_SCSI_ACTION_DMA_TARGET			(TME_BIT(15))
#define TME_SCSI_ACTION_RESPOND_SELECTED		(TME_BIT(16))
#define TME_SCSI_ACTION_RESPOND_RESELECTED		(TME_BIT(17))
#define TME_SCSI_ACTION_SELECT				(TME_BIT(18))
#define TME_SCSI_ACTION_SELECT_WITH_ATN			(TME_BIT(19) | TME_SCSI_ACTION_SELECT)
#define TME_SCSI_ACTION_SELECT_SINGLE_INITIATOR		(TME_BIT(20) | TME_SCSI_ACTION_SELECT)
#define TME_SCSI_ACTION_RESELECT			(TME_BIT(21))
#define TME_SCSI_ACTION_ARBITRATE_HALF			(TME_BIT(22))
#define TME_SCSI_ACTION_ARBITRATE_FULL			(TME_BIT(23) | TME_SCSI_ACTION_ARBITRATE_HALF)

/* this evaluates to nonzero if any of the actions in the mask are
   selected, and no more significant actions are also selected: */
#define TME_SCSI_ACTIONS_SELECTED(actions, actions_mask)	\
  (((actions) & (actions_mask)) != 0 && (((actions) & ~(actions_mask)) < (actions_mask)))

/* types: */

/* the SCSI control signals: */
typedef tme_uint32_t tme_scsi_control_t;

/* the SCSI data signals: */
typedef tme_uint32_t tme_scsi_data_t;

/* a SCSI DMA buffer: */
struct tme_scsi_dma {

  /* the flags: */
  unsigned char tme_scsi_dma_flags;

  /* how many bytes remain in the buffer: */
  unsigned long tme_scsi_dma_resid;

  /* the input buffer.  this is used for an initiator when I/O is
     true, and for a target when I/O is false: */
  tme_uint8_t *tme_scsi_dma_in;

  /* the output buffer.  this is used for an initiator when I/O is
     false, and for a target when I/O is true: */
  _tme_const tme_uint8_t *tme_scsi_dma_out;

  /* any synchronous transfer REQ/ACK offset.  zero implies 
     asynchronous transfer: */
  unsigned short tme_scsi_dma_sync_offset;

  /* any synchronous transfer period, in nanoseconds: */
  unsigned short tme_scsi_dma_sync_period;
};

/* a SCSI connection: */
struct tme_scsi_connection {

  /* the generic connection side: */
  struct tme_connection tme_scsi_connection;

  /* this is called for a SCSI bus cycle: */
  int (*tme_scsi_connection_cycle) _TME_P((struct tme_scsi_connection *,
					   tme_scsi_control_t,
					   tme_scsi_data_t,
					   tme_uint32_t,
					   tme_uint32_t,
					   const struct tme_scsi_dma *));
};

/* prototypes: */
int tme_scsi_connection_score _TME_P((struct tme_connection *, unsigned int *));
int tme_scsi_id_parse _TME_P((const char *));
#define tme_scsi_lun_parse tme_scsi_id_parse

/* this implements state machines that determine the residual in a
   SCSI command or message phase: */
tme_uint32_t tme_scsi_phase_resid _TME_P((tme_scsi_control_t, 
					  tme_uint32_t *,
					  const tme_shared tme_uint8_t *bytes,
					  unsigned long));

#endif /* !_TME_GENERIC_SCSI_H */
