/* $Id: scsi-device.c,v 1.8 2009/11/08 17:17:42 fredette Exp $ */

/* scsi/scsi-device.c - implementation of generic SCSI device support: */

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
_TME_RCSID("$Id: scsi-device.c,v 1.8 2009/11/08 17:17:42 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-device.h>
#include <tme/scsi/scsi-msg.h>
#include <tme/scsi/scsi-cdb.h>

/* macros: */

/* the callout flags: */
#define TME_SCSI_DEVICE_CALLOUT_CHECK		(0)
#define TME_SCSI_DEVICE_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SCSI_DEVICE_CALLOUTS_MASK		(-2)
#define  TME_SCSI_DEVICE_CALLOUT_CYCLE		TME_BIT(1)

/* the SCSI device callout function.  it must be called with the mutex locked: */
static void
_tme_scsi_device_callout(struct tme_scsi_device *scsi_device,
			 int new_callouts)
{
  struct tme_scsi_connection *conn_scsi;
  int callouts, later_callouts;
  int rc;
  tme_uint32_t events;
  tme_uint32_t actions;
  const struct tme_scsi_dma *dma;
  struct tme_scsi_dma dma_buffer;
  
  /* add in any new callouts: */
  scsi_device->tme_scsi_device_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (scsi_device->tme_scsi_device_callout_flags
      & TME_SCSI_DEVICE_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  scsi_device->tme_scsi_device_callout_flags
    |= TME_SCSI_DEVICE_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = scsi_device->tme_scsi_device_callout_flags)
	  & TME_SCSI_DEVICE_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    scsi_device->tme_scsi_device_callout_flags
      = (callouts
	 & ~TME_SCSI_DEVICE_CALLOUTS_MASK);
    callouts
      &= TME_SCSI_DEVICE_CALLOUTS_MASK;

    /* get this card's SCSI connection: */
    conn_scsi = scsi_device->tme_scsi_device_connection;

    /* if we need to call out a SCSI bus cycle: */
    if (callouts & TME_SCSI_DEVICE_CALLOUT_CYCLE) {

      /* if the bus is busy: */
      if (scsi_device->tme_scsi_device_control
	  & TME_SCSI_SIGNAL_BSY) {

	/* run a target information transfer phase DMA sequence: */
	events = TME_SCSI_EVENT_NONE;
	actions = TME_SCSI_ACTION_DMA_TARGET;
	dma_buffer = scsi_device->tme_scsi_device_dma;
	dma = &dma_buffer;
      }

      /* otherwise, the bus is not busy: */
      else {

	/* wait to be selected: */
	events = TME_SCSI_EVENT_SELECTED | TME_SCSI_EVENT_IDS_SELF(TME_BIT(scsi_device->tme_scsi_device_id));
	actions = TME_SCSI_ACTION_RESPOND_SELECTED;
	dma = NULL;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&scsi_device->tme_scsi_device_mutex);
      
      /* do the callout: */
      rc = (conn_scsi != NULL
	    ? ((*conn_scsi->tme_scsi_connection_cycle)
	       (conn_scsi,
		scsi_device->tme_scsi_device_control,
		0,
		events,
		actions,
		dma))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&scsi_device->tme_scsi_device_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_SCSI_DEVICE_CALLOUT_CYCLE;
      }
    }

  }
  
  /* put in any later callouts, and clear that callouts are running: */
  scsi_device->tme_scsi_device_callout_flags = later_callouts;
}

/* when a device is the target, this enters a new bus phase: */
void
tme_scsi_device_target_phase(struct tme_scsi_device *scsi_device,
			     tme_scsi_control_t control)
{
  const char *info_type;

  /* set the phase: */
  scsi_device->tme_scsi_device_control = control;

  /* if we are not freeing the bus: */
  if (control & TME_SCSI_SIGNAL_BSY) {

    /* assume that we will not dump the information transferred: */
    info_type = NULL;

    /* XXX parity? */

    /* dispatch on the phase: */
    switch (TME_SCSI_PHASE(control)) {

      /* for the DATA_OUT and DATA_IN phases, we assume that
	 the caller has already set up the DMA structure: */
    case TME_SCSI_PHASE_DATA_IN:
      info_type = "DATA_IN";
      /* FALLTHROUGH */
    case TME_SCSI_PHASE_DATA_OUT:
      break;
      
      /* for the COMMAND phase, begin by transferring the
	 first byte of the CDB: */
    case TME_SCSI_PHASE_COMMAND:
      assert ((scsi_device->tme_scsi_device_dma.tme_scsi_dma_flags
	       & TME_SCSI_DMA_WIDTH)
	      == TME_SCSI_DMA_8BIT);
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
	= &scsi_device->tme_scsi_device_cdb[0];
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	= 1;
      break;

      /* for the STATUS phase, transfer the status byte: */
    case TME_SCSI_PHASE_STATUS:
      assert ((scsi_device->tme_scsi_device_dma.tme_scsi_dma_flags
	       & TME_SCSI_DMA_WIDTH)
	      == TME_SCSI_DMA_8BIT);
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
	= &scsi_device->tme_scsi_device_status;
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	= 1;
      info_type = "STATUS";
      break;

      /* for the MESSAGE_OUT phase, begin by transferring the
	 first byte of the message: */
    case TME_SCSI_PHASE_MESSAGE_OUT:
      assert ((scsi_device->tme_scsi_device_dma.tme_scsi_dma_flags
	       & TME_SCSI_DMA_WIDTH)
	      == TME_SCSI_DMA_8BIT);
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
	= &scsi_device->tme_scsi_device_msg[0];
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	= 1;
      break;

      /* for the MESSAGE_IN phase, transfer the entire message: */
    case TME_SCSI_PHASE_MESSAGE_IN:
      assert ((scsi_device->tme_scsi_device_dma.tme_scsi_dma_flags
	       & TME_SCSI_DMA_WIDTH)
	      == TME_SCSI_DMA_8BIT);
      scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
	= &scsi_device->tme_scsi_device_msg[0];

      /* the total message length depends on the message type: */

      /* "The extended message length specifies the length in bytes of
	 the extended message code plus the extended message arguments
	 to follow.  Therefore, the total length of the message is
	 equal to the extended message length plus two.  A value of
	 zero for the extended message length indicates 256 bytes
	 follow." */
      if (scsi_device->tme_scsi_device_msg[0]
	  == TME_SCSI_MSG_EXTENDED) {
	scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	  = ((scsi_device->tme_scsi_device_msg[1]
	      == 0)
	     ? (2 + 256)
	     : (2 + scsi_device->tme_scsi_device_msg[1]));
      }

      else if (TME_SCSI_MSG_IS_2(scsi_device->tme_scsi_device_msg[0])) {
	scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	  = 2;
      }

      else {
	assert (TME_SCSI_MSG_IS_1(scsi_device->tme_scsi_device_msg[0]));
	scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	  = 1;
      }
      info_type = "MESSAGE_IN";
      break;

    default:
      abort();
    }

    /* if we can, dump up to 128 bytes of information: */
    if (info_type != NULL) {
      tme_log_start(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
		    2000, TME_OK) {
	unsigned int byte_i, count;
	count = TME_MIN(scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid,
			128);
	tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
		     "%s:", info_type);
	for (byte_i = 0; byte_i < count ; byte_i++) {
	  tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
		       " 0x%02x",
		       scsi_device->tme_scsi_device_dma.tme_scsi_dma_out[byte_i]);
	}
	if (count < scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid) {
	  tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
		       " ...");
	}
      } tme_log_finish(&scsi_device->tme_scsi_device_element->tme_element_log_handle);
    }
  }
}

/* this handles a SCSI bus cycle: */
static int
_tme_scsi_device_cycle(struct tme_scsi_connection *conn_scsi,
		       tme_scsi_control_t control_new,
		       tme_scsi_data_t data,
		       tme_uint32_t events,
		       tme_uint32_t actions,
		       const struct tme_scsi_dma *dma)
{
  struct tme_scsi_device *scsi_device;
  int new_callouts;
  tme_scsi_control_t control_old;
  unsigned long count;
  void (*do_cdb) _TME_P((struct tme_scsi_device *,
			 tme_scsi_control_t,
			 tme_scsi_control_t));

  /* recover our device: */
  scsi_device = conn_scsi->tme_scsi_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&scsi_device->tme_scsi_device_mutex);

  /* get the last bus state: */
  control_old = scsi_device->tme_scsi_device_control;

  /* if we last knew the bus to be free, we must now be selected: */
  if (!(control_old
	& TME_SCSI_SIGNAL_BSY)) {

    /* "SCSI devices indicate their ability to accommodate more than
       the COMMAND COMPLETE message by asserting or responding to the
       ATN signal.  The initiator indicates this in the SELECTION
       phase by asserting ATN prior to the SCSI bus condition of SEL
       true, and BSY false.  The target indicates its ability to
       accommodate more messages by responding to the ATTENTION
       condition with the MESSAGE OUT phase after going through the
       SELECTION phase." */

    /* XXX parity? */
    scsi_device->tme_scsi_device_dma.tme_scsi_dma_flags
      = TME_SCSI_DMA_8BIT;
    tme_scsi_device_target_phase(scsi_device,
				 TME_SCSI_SIGNAL_BSY
				 | ((control_new
				     & TME_SCSI_SIGNAL_ATN)
				    ? TME_SCSI_PHASE_MESSAGE_OUT
				    : TME_SCSI_PHASE_COMMAND));

    /* no LUN has been addressed yet: */
    scsi_device->tme_scsi_device_addressed_lun
      = -1;
  }

  /* otherwise, we had the bus in some phase: */
  else {

    /* if we didn't transfer all of the bytes we wanted, something
       went wrong with the initiator - probably the bus was reset: */
    assert (dma != NULL);
    scsi_device->tme_scsi_device_dma = *dma;
    if (scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid > 0) {

      /* return to the bus-free phase: */
      scsi_device->tme_scsi_device_control
	= 0;

      /* log the forced bus free: */
      tme_log(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
	      1000, TME_OK,
	      (&scsi_device->tme_scsi_device_element->tme_element_log_handle,
	       _("short transfer, control now 0x%04x, forced bus free"),
		 control_new));

      /* XXX callback? */
    }

    /* otherwise, we did transfer all of the bytes we wanted: */
    else {

      /* dispatch on the previous bus phase: */
      switch (TME_SCSI_PHASE(control_old)) {
      case TME_SCSI_PHASE_MESSAGE_OUT:
	
	/* calculate how many message bytes we have transferred: */
	count = (scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
		 - &scsi_device->tme_scsi_device_msg[0]);
	
	/* dispatch on the first byte of the message: */

	/* if this is an extended message: */
	if (scsi_device->tme_scsi_device_msg[0]
	    == TME_SCSI_MSG_EXTENDED) {

	  /* if we have only received the first message byte, receive
	     the next byte, which will give us the total length of the
	     message: */
	  if (count == 1) {
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = 1;
	  }

	  /* if we have received the second message byte, we now know
	     the total length of the message: */
	  else if (count == 2) {
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = ((scsi_device->tme_scsi_device_msg[1]
		  == 0)
		 ? 256
		 : scsi_device->tme_scsi_device_msg[1]);
	  }

	  /* otherwise, we must have received all of the message: */
	}

	/* if this is a two-byte message: */
	else if (TME_SCSI_MSG_IS_2(scsi_device->tme_scsi_device_msg[0])) {

	  /* if we have only received the first message byte,
	     receive the second message byte: */
	  if (count == 1) {
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = 1;
	  }

	  /* otherwise, we must have received all of the message: */
	}

	/* otherwise, this must be a one-byte message, and we have
	   received all of it: */
	else {
	  assert (TME_SCSI_MSG_IS_1(scsi_device->tme_scsi_device_msg[0]));
	}

	/* if we have received all of the message: */
	if (scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid == 0) {

	  /* log the message: */
	  tme_log_start(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
			1000, TME_OK) {
	    unsigned int byte_i;
	    tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
	                 _("MESSAGE_OUT:"));
	    for (byte_i = 0; byte_i < count ; byte_i++) {
	      tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
			   " 0x%02x",
			   scsi_device->tme_scsi_device_msg[byte_i]);
	    }
	  } tme_log_finish(&scsi_device->tme_scsi_device_element->tme_element_log_handle);

	  /* "Normally, the initiator negates ATN while REQ is true
	     and ACK is false during the last REQ/ACK handshake of the
	     MESSAGE OUT phase." */
	  tme_scsi_device_target_phase(scsi_device,
				       TME_SCSI_SIGNAL_BSY
				       | ((control_new
					   & TME_SCSI_SIGNAL_ATN)
					  ? TME_SCSI_PHASE_MESSAGE_OUT
					  : TME_SCSI_PHASE_COMMAND));

	  /* call out for the message: */
	  (*((scsi_device->tme_scsi_device_msg[0]
	      == TME_SCSI_MSG_EXTENDED)
	     ? (scsi_device->tme_scsi_device_do_msg_ext
		[scsi_device->tme_scsi_device_msg[2]])
	     : (scsi_device->tme_scsi_device_do_msg
		[TME_MIN(scsi_device->tme_scsi_device_msg[0],
			 TME_SCSI_MSG_IDENTIFY)])))
	    (scsi_device,
	     control_old,
	     control_new);
	}
	
	break;
	
      case TME_SCSI_PHASE_COMMAND:
	
	/* calculate how many message bytes we have transferred: */
	count = (scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
		 - &scsi_device->tme_scsi_device_cdb[0]);
	
	/* if we have only transferred the first byte: */
	if (count == 1) {

	  /* dispatch on the CDB group: */
	  switch (scsi_device->tme_scsi_device_cdb[0]
		  & TME_SCSI_CDB_GROUP_MASK) {

	  case TME_SCSI_CDB_GROUP_0:
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = TME_SCSI_CDB_GROUP_0_LEN;
	    break;
	  case TME_SCSI_CDB_GROUP_1:
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = TME_SCSI_CDB_GROUP_1_LEN;
	    break;
	  case TME_SCSI_CDB_GROUP_2:
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = TME_SCSI_CDB_GROUP_2_LEN;
	    break;
	  case TME_SCSI_CDB_GROUP_4:
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = TME_SCSI_CDB_GROUP_4_LEN;
	    break;
	  case TME_SCSI_CDB_GROUP_5:
	    scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
	      = TME_SCSI_CDB_GROUP_5_LEN;
	    break;

	  case TME_SCSI_CDB_GROUP_3:
	  case TME_SCSI_CDB_GROUP_6:
	  case TME_SCSI_CDB_GROUP_7:
	  default:
	    abort();
	  }

	  /* we have already transferred the first byte of the CDB: */
	  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid--;
	  break;
	}

	/* otherwise, we must have transferred all of the command: */
	else {

	  /* log the CDB: */
	  tme_log_start(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
			1000, TME_OK) {
	    unsigned int byte_i;
	    tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
	                 _("CDB:"));
	    for (byte_i = 0; byte_i < count ; byte_i++) {
	      tme_log_part(&scsi_device->tme_scsi_device_element->tme_element_log_handle,
			   " 0x%02x",
			   scsi_device->tme_scsi_device_cdb[byte_i]);
	    }
	  } tme_log_finish(&scsi_device->tme_scsi_device_element->tme_element_log_handle);

	  /* if the addressed LUN is valid: */
	  if (((*scsi_device->tme_scsi_device_address_lun)
	       (scsi_device))
	      == TME_OK) {

	    /* if this command is illegal: */
	    do_cdb
	      = (scsi_device->tme_scsi_device_do_cdb
		 [scsi_device->tme_scsi_device_cdb[0]]);
	    if (__tme_predict_false(do_cdb == NULL)) {

	      /* use the illegal command handler: */
	      do_cdb = tme_scsi_device_cdb_illegal;
	    }
	    
	    /* call out for the CDB: */
	    (*do_cdb)
	      (scsi_device,
	       control_old,
	       control_new);
	  }
	}
	
	break;

	/* all of these phases just cause phase callbacks: */
      case TME_SCSI_PHASE_STATUS:
      case TME_SCSI_PHASE_MESSAGE_IN:
      case TME_SCSI_PHASE_DATA_OUT:
      case TME_SCSI_PHASE_DATA_IN:
	(*scsi_device->tme_scsi_device_phase)
	  (scsi_device,
	   control_old,
	   control_new);
	break;

      default: abort();
      }
    }
  }

  /* call out a new SCSI cycle: */
  new_callouts |= TME_SCSI_DEVICE_CALLOUT_CYCLE;

  /* make any needed callouts: */
  _tme_scsi_device_callout(scsi_device, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&scsi_device->tme_scsi_device_mutex);

  return (TME_OK);
}

/* when a device is the target, this frees the bus: */
_TME_SCSI_DEVICE_PHASE_DECL(tme_scsi_device_target_f)
{
  /* enter the BUS FREE phase: */
  tme_scsi_device_target_phase(scsi_device,
			       0);

  /* there is no next phase we will enter: */
  scsi_device->tme_scsi_device_phase
    = NULL;
}

/* when a device is the target, this runs the MESSAGE IN phase,
   followed by the BUS FREE phase.  the transmitted message must be
   preset: */
_TME_SCSI_DEVICE_PHASE_DECL(tme_scsi_device_target_mf)
{
  /* enter the MESSAGE IN phase: */
  tme_scsi_device_target_phase(scsi_device,
			       TME_SCSI_SIGNAL_BSY
			       | TME_SCSI_PHASE_MESSAGE_IN);

  /* the next phase we enter will be the BUS FREE phase: */
  scsi_device->tme_scsi_device_phase
    = tme_scsi_device_target_f;
}

/* when a device is the target, this runs the STATUS phase, followed
   by the MESSAGE IN phase, followed by the BUS FREE phase.  the
   transmitted status and message must be preset: */
_TME_SCSI_DEVICE_PHASE_DECL(tme_scsi_device_target_smf)
{
  /* enter the STATUS phase: */
  tme_scsi_device_target_phase(scsi_device,
			       TME_SCSI_SIGNAL_BSY
			       | TME_SCSI_PHASE_STATUS);

  /* the next phase we enter will be the MESSAGE IN phase: */
  scsi_device->tme_scsi_device_phase
    = tme_scsi_device_target_mf;
}

/* when a device is the target, this runs the DATA IN or DATA OUT
   phase, followed by the STATUS phase, followed by the MESSAGE IN
   phase, followed by the BUS FREE phase.  the transmitted data,
   status and message must be preset: */
_TME_SCSI_DEVICE_PHASE_DECL(tme_scsi_device_target_dsmf)
{
  /* enter the DATA IN or DATA OUT phase, depending on how
     DMA was set up: */
  tme_scsi_device_target_phase(scsi_device,
			       TME_SCSI_SIGNAL_BSY
			       | ((scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
				   != NULL)
				  ? TME_SCSI_PHASE_DATA_OUT
				  : TME_SCSI_PHASE_DATA_IN));

  /* the next phase we enter will be the STATUS phase: */
  scsi_device->tme_scsi_device_phase
    = tme_scsi_device_target_smf;
}

/* when a device is the target, this runs a MESSAGE IN phase, followed
   usually by a COMMAND phase (but possibly a MESSAGE OUT phase): */
_TME_SCSI_DEVICE_PHASE_DECL(tme_scsi_device_target_mc)
{
  
  /* enter either the MESSAGE OUT phase or the COMMAND phase: */
  tme_scsi_device_target_phase(scsi_device,
			       TME_SCSI_SIGNAL_BSY
			       | ((control_new
				   & TME_SCSI_SIGNAL_ATN)
				  ? TME_SCSI_PHASE_MESSAGE_OUT
				  : TME_SCSI_PHASE_COMMAND));

#ifndef NDEBUG
  /* both the MESSAGE OUT and COMMAND phases have specific
     dispatchers: */
  scsi_device->tme_scsi_device_phase = NULL;
#endif /* !NDEBUG */
}

/* when a device is the target, this builds a simple extended sense
   and returns an immediate CHECK CONDITION status to the initiator: */
void
tme_scsi_device_check_condition(struct tme_scsi_device *scsi_device,
				tme_uint8_t sense_key,
				tme_uint16_t sense_asc_ascq)
{
  struct tme_scsi_device_sense *sense;
  int lun;

  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* this target must support extended sense: */
  assert (!scsi_device->tme_scsi_device_sense_no_extended);

  /* form the sense: */
  sense = &scsi_device->tme_scsi_device_sense[lun];
    
  /* the error class and error code: */
  sense->tme_scsi_device_sense_data[0]
    = 0x70;

  /* the sense key: */
  sense->tme_scsi_device_sense_data[2]
    = sense_key;

  /* if there is no additional sense: */
  if (sense_asc_ascq == TME_SCSI_SENSE_EXT_ASC_ASCQ_NONE) {

    /* there is no additional sense length: */
    sense->tme_scsi_device_sense_data[7] = 0x00;
  }

  /* otherwise, there is additional sense: */
  else {

    /* the additional sense length: */
    sense->tme_scsi_device_sense_data[7]
      = 0x06;

    /* the additional sense code and additional sense code qualifier: */
    sense->tme_scsi_device_sense_data[12] = (sense_asc_ascq >> 8);
    sense->tme_scsi_device_sense_data[13] = sense_asc_ascq;
  }

  /* this sense is valid: */
  sense->tme_scsi_device_sense_valid
    = TRUE;

  /* return the CHECK CONDITION status: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_CHECK_CONDITION,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this is the LUN addresser for LUN-aware devices: */
int
tme_scsi_device_address_lun_aware(struct tme_scsi_device *scsi_device)
{
  struct tme_scsi_device_sense *sense;
  int lun;

  /* if an IDENTIFY message was sent, use that LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* otherwise, get the LUN from bits 5-7 of the second
     CDB byte: */
  if (lun < 0) {
    lun = (scsi_device->tme_scsi_device_cdb[1] >> 5);
    scsi_device->tme_scsi_device_addressed_lun = lun;
  }

  /* if this LUN is not defined, and this isn't a REQUEST SENSE
     or INQUIRY command: */
  if (!(scsi_device->tme_scsi_device_luns
	& TME_BIT(lun))
      && (scsi_device->tme_scsi_device_cdb[0]
	  != TME_SCSI_CDB_REQUEST_SENSE)
      && (scsi_device->tme_scsi_device_cdb[0]
	  != TME_SCSI_CDB_INQUIRY)) {

    /* this target must support extended sense: */
    assert (!scsi_device->tme_scsi_device_sense_no_extended);

    /* form the ILLEGAL REQUEST sense: */
    sense = &scsi_device->tme_scsi_device_sense[lun];
    
    /* the error class and error code: */
    sense->tme_scsi_device_sense_data[0]
      = 0x70;

    /* the ILLEGAL REQUEST sense key: */
    sense->tme_scsi_device_sense_data[2]
	= 0x05;

    /* the additional sense length: */
    sense->tme_scsi_device_sense_data[7]
      = 0x00;

    sense->tme_scsi_device_sense_valid
      = TRUE;

    /* return the CHECK CONDITION status: */
    tme_scsi_device_target_do_smf(scsi_device,
				  TME_SCSI_STATUS_CHECK_CONDITION,
				  TME_SCSI_MSG_CMD_COMPLETE);
    return (EINVAL);
  }

  return (TME_OK);
}

/* this is the LUN addresser for LUN-unaware devices: */
int
tme_scsi_device_address_lun_unaware(struct tme_scsi_device *scsi_device)
{

  /* we always force a LUN of zero: */
  scsi_device->tme_scsi_device_addressed_lun = 0;

  return (tme_scsi_device_address_lun_aware(scsi_device));
}

/* this makes a new connection: */
int
tme_scsi_device_connection_make(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_scsi_device *scsi_device;
  struct tme_scsi_connection *conn_scsi;

  /* recover our device: */
  scsi_device = conn->tme_connection_element->tme_element_private;

  /* both sides must be SCSI connections: */
  assert (conn->tme_connection_type
	  == TME_CONNECTION_SCSI);
  assert (conn->tme_connection_other->tme_connection_type
	  == TME_CONNECTION_SCSI);

  /* simple SCSI devices are always set up to answer calls across the
     connection, so we only have to do work when the connection has
     gone full, namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock the mutex: */
    tme_mutex_lock(&scsi_device->tme_scsi_device_mutex);

    /* save our connection: */
    conn_scsi
      = ((struct tme_scsi_connection *)
	 conn->tme_connection_other);
    scsi_device->tme_scsi_device_connection
      = conn_scsi;

    /* call out a SCSI bus cycle: */
    scsi_device->tme_scsi_device_control = 0;
    _tme_scsi_device_callout(scsi_device,
			     TME_SCSI_DEVICE_CALLOUT_CYCLE);

    /* unlock the mutex: */
    tme_mutex_unlock(&scsi_device->tme_scsi_device_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
int
tme_scsi_device_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a SCSI device: */
int
tme_scsi_device_connections_new(struct tme_element *element,
				const char * const *args,
				struct tme_connection **_conns,
				char **_output)
{
  struct tme_scsi_device *scsi_device;
  struct tme_scsi_connection *conn_scsi;
  struct tme_connection *conn;

  /* recover our device: */
  scsi_device = (struct tme_scsi_device *) element->tme_element_private;

  /* if this device isn't connected to a SCSI bus, make the new
     connection side: */
  if (scsi_device->tme_scsi_device_connection == NULL) {

    /* create our side of a SCSI connection: */
    conn_scsi = tme_new0(struct tme_scsi_connection, 1);
    conn = &conn_scsi->tme_scsi_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SCSI;
    conn->tme_connection_score = tme_scsi_connection_score;
    conn->tme_connection_make = tme_scsi_device_connection_make;
    conn->tme_connection_break = tme_scsi_device_connection_break;
    
    /* fill in the SCSI connection: */
    conn_scsi->tme_scsi_connection_cycle
      = _tme_scsi_device_cycle;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  return (TME_OK);
}

/* this initializes a new SCSI device: */
int
tme_scsi_device_new(struct tme_scsi_device *scsi_device,
		    int id)
{
  int i;

  /* initialize the mutex: */
  tme_mutex_init(&scsi_device->tme_scsi_device_mutex);

  /* set the SCSI ID: */
  scsi_device->tme_scsi_device_id = id;

  /* initialize all of the message and CDB handlers to NULL: */
  for (i = TME_ARRAY_ELS(scsi_device->tme_scsi_device_do_msg);
       i-- > 0; ) {
    scsi_device->tme_scsi_device_do_msg[i] = NULL;
  }
  for (i = TME_ARRAY_ELS(scsi_device->tme_scsi_device_do_cdb);
       i-- > 0; ) {
    scsi_device->tme_scsi_device_do_cdb[i] = NULL;
  }

  /* set the message handlers: */
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_CMD_COMPLETE,
			 tme_scsi_device_msg_cmd_complete);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_SAVE_DATA_POINTER,
			 tme_scsi_device_msg_save_data_pointer);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_RESTORE_POINTERS,
			 tme_scsi_device_msg_restore_pointers);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_DISCONNECT,
			 tme_scsi_device_msg_disconnect);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_INITIATOR_ERROR,
			 tme_scsi_device_msg_initiator_error);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_ABORT,
			 tme_scsi_device_msg_abort);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_MESSAGE_REJECT,
			 tme_scsi_device_msg_message_reject);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_NOP,
			 tme_scsi_device_msg_nop);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_PARITY_ERROR,
			 tme_scsi_device_msg_parity_error);
  TME_SCSI_DEVICE_DO_MSG(scsi_device,
			 TME_SCSI_MSG_IDENTIFY,
			 tme_scsi_device_msg_identify);
  TME_SCSI_DEVICE_DO_MSG_EXT(scsi_device,
			     TME_SCSI_MSG_EXT_SDTR,
			     tme_scsi_device_msg_target_reject);

  /* set the "Group 0 Common Commands for All Device Types": */
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TEST_UNIT_READY,
			 tme_scsi_device_cdb_tur);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_REQUEST_SENSE,
			 tme_scsi_device_cdb_request_sense);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_INQUIRY,
			 NULL);

  /* assume that this device is LUN-aware: */
  scsi_device->tme_scsi_device_address_lun
    = tme_scsi_device_address_lun_aware;

  return (TME_OK);
}
