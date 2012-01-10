/* $Id: scsi-msg.c,v 1.3 2007/01/07 23:59:31 fredette Exp $ */

/* scsi/scsi-msg.c - implementation of generic SCSI device message support: */

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
_TME_RCSID("$Id: scsi-msg.c,v 1.3 2007/01/07 23:59:31 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-msg.h>

/* this handles the COMMAND COMPLETE message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_cmd_complete)
{
  abort();
}

/* this handles the SAVE DATA POINTER message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_save_data_pointer)
{
  abort();
}

/* this handles the RESTORE POINTERS message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_restore_pointers)
{
  abort();
}

/* this handles the DISCONNECT message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_disconnect)
{
  abort();
}

/* this handles the INITIATOR ERROR message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_initiator_error)
{
  abort();
}

/* this handles the ABORT message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_abort)
{
  abort();
}

/* this handles the MESSAGE REJECT message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_message_reject)
{
  abort();
}

/* this handles the NOP message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_nop)
{
  abort();
}

/* this handles the MESSAGE PARITY ERROR message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_parity_error)
{
  abort();
}

/* this handles the IDENTIFY message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_identify)
{
  
  /* set the addressed LUN: */
  scsi_device->tme_scsi_device_addressed_lun
    = TME_FIELD_MASK_EXTRACTU(scsi_device->tme_scsi_device_msg[0],
			      TME_SCSI_MSG_IDENTIFY_LUN_MASK);
}

/* this sends a MESSAGE REJECT message: */
_TME_SCSI_DEVICE_MSG_DECL(tme_scsi_device_msg_target_reject)
{

  /* enter the MESSAGE IN phase and send a MESSAGE REJECT message: */
  scsi_device->tme_scsi_device_msg[0] = TME_SCSI_MSG_MESSAGE_REJECT;
  tme_scsi_device_target_phase(scsi_device,
			       (TME_SCSI_SIGNAL_BSY
				| TME_SCSI_PHASE_MESSAGE_IN));

  /* the next phase we will enter will be either a MESSAGE OUT
     or the COMMAND phase: */
  scsi_device->tme_scsi_device_phase = tme_scsi_device_target_mc;
}
