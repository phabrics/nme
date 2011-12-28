/* $Id: scsi-tape.h,v 1.3 2006/11/15 23:10:33 fredette Exp $ */

/* tme/scsi/scsi-tape.h - header file for generic SCSI tape emulation: */

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

#ifndef _TME_SCSI_SCSI_TAPE_H
#define _TME_SCSI_SCSI_TAPE_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-tape.h,v 1.3 2006/11/15 23:10:33 fredette Exp $");

/* includes: */
#include <tme/generic/tape.h>
#include <tme/scsi/scsi-device.h>
#include <tme/scsi/scsi-cdb.h>
#include <tme/scsi/scsi-msg.h>

/* macros: */

/* "Group 0 Commands for Sequential-Access Devices" */
					/* 0x00 common */
#define TME_SCSI_CDB_TAPE_REWIND	(0x01)
					/* 0x02 vendor unique */
					/* 0x03 common */
					/* 0x04 reserved */
#define	TME_SCSI_CDB_TAPE_BLOCK_LIMITS	(0x05)
					/* 0x06 vendor unique */
					/* 0x07 vendor unique */
#define TME_SCSI_CDB_TAPE_READ0		(0x08)
					/* 0x09 vendor unique */
#define TME_SCSI_CDB_TAPE_WRITE0	(0x0a)
#define TME_SCSI_CDB_TAPE_TRACK_SELECT	(0x0b)
					/* 0x0c vendor unique */
					/* 0x0d vendor unique */
					/* 0x0e vendor unique */
#define TME_SCSI_CDB_TAPE_READ_REVERSE	(0x0f)
#define TME_SCSI_CDB_TAPE_WRITE_MARKS	(0x10)
#define TME_SCSI_CDB_TAPE_SPACE		(0x11)
					/* 0x12 common */
#define TME_SCSI_CDB_TAPE_VERIFY	(0x13)
#define TME_SCSI_CDB_TAPE_RECOVER_BUF	(0x14)
#define TME_SCSI_CDB_TAPE_MODE_SELECT	(0x15)
#define TME_SCSI_CDB_TAPE_RESERVE	(0x16)
#define TME_SCSI_CDB_TAPE_RELEASE	(0x17)
					/* 0x18 common */
#define TME_SCSI_CDB_TAPE_ERASE		(0x19)
#define TME_SCSI_CDB_TAPE_MODE_SENSE	(0x1a)
#define TME_SCSI_CDB_TAPE_LOAD_UNLOAD	(0x1b)
					/* 0x1c common */
					/* 0x1d common */
#define TME_SCSI_CDB_TAPE_PREVENT_ALLOW	(0x1e)
					/* 0x1f common */

/* flags: */
#define TME_SCSI_TAPE_FLAG_LOADED	TME_BIT(0)
#define TME_SCSI_TAPE_FLAG_ATTENTION	TME_BIT(1)

/* types: */

/* a SCSI tape device tape connection: */
struct tme_scsi_tape_connection {

  /* the regular tape connection: */
  struct tme_tape_connection tme_scsi_tape_connection;

  /* the LUN for this tape: */
  int tme_scsi_tape_connection_lun;

  /* the flags for this tape: */
  unsigned int tme_scsi_tape_connection_flags;
};

/* a SCSI tape: */
struct tme_scsi_tape {

  /* our SCSI device structure: */
  struct tme_scsi_device tme_scsi_tape_device;
#define tme_scsi_tape_element tme_scsi_tape_device.tme_scsi_device_element
#define tme_scsi_tape_mutex tme_scsi_tape_device.tme_scsi_device_mutex

  /* our type name: */
  char *tme_scsi_tape_type;

  /* our tape connections: */
  struct tme_scsi_tape_connection *tme_scsi_tape_connections[TME_SCSI_DEVICE_LUN_COUNT];

  /* a type-specific connected function: */
  void (*tme_scsi_tape_connected) _TME_P((struct tme_scsi_tape *, int));

  /* a type-specific transfer status function: */
  tme_uint8_t (*tme_scsi_tape_xfer_status) _TME_P((struct tme_scsi_tape *,
						   int flags,
						   unsigned long));

  /* the minimum and maximum block sizes, in bytes: */
  tme_uint32_t tme_scsi_tape_block_size_min;
  tme_uint32_t tme_scsi_tape_block_size_max;

  /* the current block size, in bytes: */
  tme_uint32_t tme_scsi_tape_block_size_current;
};

/* prototypes: */
int tme_scsi_tape_tme_init _TME_P((struct tme_scsi_tape *));
int tme_scsi_tape_emulexmt02_init _TME_P((struct tme_scsi_tape *));

_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_rewind);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_block_limits);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_read0);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_write0);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_inquiry);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_write_marks);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_space);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_mode_select);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_mode_sense);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_load_unload);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_tape_cdb_prevent_allow);

tme_uint8_t tme_scsi_tape_xfer_status _TME_P((struct tme_scsi_tape *,
					      int,
					      unsigned long));
void tme_scsi_tape_cdb_xfer0 _TME_P((struct tme_scsi_device *,
				     int));

#endif /* !_TME_SCSI_SCSI_TAPE_H */
