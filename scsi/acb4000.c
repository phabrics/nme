/* $Id: acb4000.c,v 1.5 2006/09/30 12:34:21 fredette Exp $ */

/* scsi/acb4000.c - ACB4000 SCSI disk emulation: */

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
_TME_RCSID("$Id: acb4000.c,v 1.5 2006/09/30 12:34:21 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-disk.h>
#include <errno.h>

/* macros: */

/* types: */

/* this is the ACB4000 LUN addresser for LUN-aware devices: */
static int
_tme_acb4000_address_lun(struct tme_scsi_device *scsi_device)
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
     command: */
  if (!(scsi_device->tme_scsi_device_luns
	& TME_BIT(lun))
      && (scsi_device->tme_scsi_device_cdb[0]
	  != TME_SCSI_CDB_REQUEST_SENSE)) {

    /* the ACB4000 returns a nonextended sense code of 0x25
       for an undefined LUN: */
    sense = &scsi_device->tme_scsi_device_sense[lun];
    
    /* the error class and error code: */
    sense->tme_scsi_device_sense_data[0]
      = 0x25;
    sense->tme_scsi_device_sense_data[1]
      = 0;
    sense->tme_scsi_device_sense_data[2]
      = 0;
    sense->tme_scsi_device_sense_data[3]
      = 0;
    sense->tme_scsi_device_sense_valid = 4;

    /* return the CHECK CONDITION status: */
    tme_scsi_device_target_do_smf(scsi_device,
				  TME_SCSI_STATUS_CHECK_CONDITION,
				  TME_SCSI_MSG_CMD_COMPLETE);
    return (EINVAL);
  }

  return (TME_OK);
}

/* the ACB4000 doesn't support various commands: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_acb4000_cdb_bad)
{
  int lun;
  struct tme_scsi_device_sense *sense;
  
  /* get the addressed LUN's sense: */
  lun = scsi_device->tme_scsi_device_addressed_lun;
  sense = &scsi_device->tme_scsi_device_sense[lun];

  /* the ACB4000 returns a nonextended sense code of 0x20
     for an invalid command: */
    
  /* the error class and error code: */
  sense->tme_scsi_device_sense_data[0]
    = 0x20;
  sense->tme_scsi_device_sense_data[1]
    = 0;
  sense->tme_scsi_device_sense_data[2]
    = 0;
  sense->tme_scsi_device_sense_data[3]
    = 0;
  sense->tme_scsi_device_sense_valid = 4;
  
  /* return the CHECK CONDITION status: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_CHECK_CONDITION,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this initializes ACB4000 SCSI disk emulation: */
int
tme_scsi_disk_acb4000_init(struct tme_scsi_disk *scsi_disk)
{
  struct tme_scsi_device *scsi_device;
  
  scsi_device = &scsi_disk->tme_scsi_disk_device;

  /* ACB4000 boards don't support the INQUIRY command: */
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_INQUIRY,
			 _tme_acb4000_cdb_bad);

  /* ACB4000 boards don't support extended sense: */
  scsi_device->tme_scsi_device_sense_no_extended
    = TRUE;

  /* ACB4000 boards have a particular LUN addressing behavior: */
  scsi_device->tme_scsi_device_address_lun
    = _tme_acb4000_address_lun;

  return (TME_OK);
}
