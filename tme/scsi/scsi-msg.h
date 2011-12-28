/* $Id: scsi-msg.h,v 1.3 2007/01/08 00:03:25 fredette Exp $ */

/* tme/scsi/scsi-msg.h - header file describing SCSI messages: */

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

#ifndef _TME_SCSI_SCSI_MSG_H
#define _TME_SCSI_SCSI_MSG_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-msg.h,v 1.3 2007/01/08 00:03:25 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-device.h>

/* macros: */

/* one-byte messages: */
#define TME_SCSI_MSG_CMD_COMPLETE		(0x00)
#define TME_SCSI_MSG_EXTENDED			(0x01)
#define TME_SCSI_MSG_SAVE_DATA_POINTER		(0x02)
#define TME_SCSI_MSG_RESTORE_POINTERS		(0x03)
#define TME_SCSI_MSG_DISCONNECT			(0x04)
#define TME_SCSI_MSG_INITIATOR_ERROR		(0x05)
#define TME_SCSI_MSG_ABORT			(0x06)
#define TME_SCSI_MSG_MESSAGE_REJECT		(0x07)
#define TME_SCSI_MSG_NOP			(0x08)
#define TME_SCSI_MSG_PARITY_ERROR		(0x09)
#define TME_SCSI_MSG_IDENTIFY			(0x80)
#define  TME_SCSI_MSG_IDENTIFY_DISCONNECT	(0x40)
#define  TME_SCSI_MSG_IDENTIFY_LUN_MASK		(0x07)

/* extended messages: */
#define TME_SCSI_MSG_EXT_SDTR			(0x01)

/* this evaluates to nonzero iff this is a one-byte message: */
#define TME_SCSI_MSG_IS_1(msg)			\
  (((msg)					\
    & TME_SCSI_MSG_IDENTIFY)			\
   || ((msg) < 0x20				\
       && (msg) != TME_SCSI_MSG_EXTENDED))

/* this evaluates to nonzero iff this is a two-byte message: */
#define TME_SCSI_MSG_IS_2(msg)			\
  (((msg) & 0xf0) == 0x20)

/* prototypes: */
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_cmd_complete);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_save_data_pointer);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_restore_pointers);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_disconnect);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_initiator_error);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_abort);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_message_reject);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_nop);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_parity_error);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_identify);
_TME_SCSI_DEVICE_MSG_P(tme_scsi_device_msg_target_reject);

#endif /* !_TME_SCSI_SCSI_MSG_H */
