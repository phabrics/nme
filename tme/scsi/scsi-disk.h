/* $Id: scsi-disk.h,v 1.2 2003/08/07 22:14:31 fredette Exp $ */

/* tme/scsi/scsi-disk.h - header file for generic SCSI disk emulation: */

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

#ifndef _TME_SCSI_SCSI_DISK_H
#define _TME_SCSI_SCSI_DISK_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-disk.h,v 1.2 2003/08/07 22:14:31 fredette Exp $");

/* includes: */
#include <tme/generic/disk.h>
#include <tme/scsi/scsi-device.h>
#include <tme/scsi/scsi-cdb.h>
#include <tme/scsi/scsi-msg.h>

/* macros: */

/* "Group 0 Commands for Direct-Access Devices" */
					/* 0x00 common */
#define TME_SCSI_CDB_DISK_REZERO_UNIT	(0x01)
					/* 0x02 vendor unique */
					/* 0x03 common */
#define	TME_SCSI_CDB_DISK_FORMAT_UNIT	(0x04)
					/* 0x05 vendor unique */
					/* 0x06 vendor unique */
#define TME_SCSI_CDB_DISK_REASSIGN_BLKS	(0x07)
#define TME_SCSI_CDB_DISK_READ0		(0x08)
					/* 0x09 vendor unique */
#define TME_SCSI_CDB_DISK_WRITE0	(0x0a)
#define TME_SCSI_CDB_DISK_SEEK0		(0x0b)
					/* 0x0c vendor unique */
					/* 0x0d vendor unique */
					/* 0x0e vendor unique */
					/* 0x0f vendor unique */
					/* 0x10 vendor unique */
					/* 0x11 vendor unique */
					/* 0x12 common */
					/* 0x13 vendor unique */
					/* 0x14 vendor unique */
#define TME_SCSI_CDB_DISK_MODE_SELECT	(0x15)
#define TME_SCSI_CDB_DISK_RESERVE	(0x16)
#define TME_SCSI_CDB_DISK_RELEASE	(0x17)
					/* 0x18 common */
					/* 0x19 vendor unique */
#define TME_SCSI_CDB_DISK_MODE_SENSE	(0x1a)
#define TME_SCSI_CDB_DISK_START_STOP	(0x1b)
					/* 0x1c common */
					/* 0x1d common */
#define TME_SCSI_CDB_DISK_PREVENT_ALLOW	(0x1e)
					/* 0x1f common */

/* "Group 1 Commands for Direct-Access Devices" */
					/* 0x20 vendor unique */
					/* 0x21 vendor unique */
					/* 0x22 vendor unique */
					/* 0x23 vendor unique */
					/* 0x24 vendor unique */
#define TME_SCSI_CDB_DISK_READ_CAPACITY	(0x25)
					/* 0x26 vendor unique */
					/* 0x27 vendor unique */
#define TME_SCSI_CDB_DISK_READ1		(0x28)
					/* 0x29 vendor unique */
#define TME_SCSI_CDB_DISK_WRITE1	(0x2a)
#define TME_SCSI_CDB_DISK_SEEK1		(0x2b)
					/* 0x2c vendor unique */
					/* 0x2d vendor unique */
#define TME_SCSI_CDB_DISK_WRITE_VERIFY	(0x2e)
#define TME_SCSI_CDB_DISK_VERIFY	(0x2f)
#define TME_SCSI_CDB_DISK_SEARCH_HIGH	(0x30)
#define TME_SCSI_CDB_DISK_SEARCH_EQUAL	(0x31)
#define TME_SCSI_CDB_DISK_SEARCH_LOW	(0x32)
#define TME_SCSI_CDB_DISK_SET_LIMITS	(0x33)
					/* 0x34 reserved */
					/* 0x35 reserved */
					/* 0x36 reserved */
					/* 0x37 reserved */
					/* 0x38 reserved */
					/* 0x39 common */
					/* 0x3a common */
					/* 0x3b reserved */
					/* 0x3c reserved */
					/* 0x3d reserved */
					/* 0x3e reserved */
					/* 0x3f reserved */

/* types: */

/* a SCSI disk device disk connection: */
struct tme_scsi_disk_connection {

  /* the regular disk connection: */
  struct tme_disk_connection tme_scsi_disk_connection;

  /* the LUN for this disk: */
  int tme_scsi_disk_connection_lun;

  /* the block size for this disk: */
  union tme_value64 tme_scsi_disk_connection_block_size;
};

/* a SCSI disk: */
struct tme_scsi_disk {

  /* our SCSI device structure: */
  struct tme_scsi_device tme_scsi_disk_device;
#define tme_scsi_disk_element tme_scsi_disk_device.tme_scsi_device_element
#define tme_scsi_disk_mutex tme_scsi_disk_device.tme_scsi_device_mutex

  /* our type name: */
  char *tme_scsi_disk_type;

  /* our disk connections: */
  struct tme_scsi_disk_connection *tme_scsi_disk_connections[TME_SCSI_DEVICE_LUN_COUNT];
};

/* prototypes: */
int tme_scsi_disk_connections_new _TME_P((struct tme_element *,
					  const char * const *,
					  struct tme_connection **,
					  char **));
int tme_scsi_disk_connection_make _TME_P((struct tme_connection *,
					  unsigned int));
int tme_scsi_disk_connection_break _TME_P((struct tme_connection *,
					   unsigned int));
int tme_scsi_disk_control _TME_P((struct tme_disk_connection *,
				  unsigned int, ...));

int tme_scsi_disk_tme_init _TME_P((struct tme_scsi_disk *));
int tme_scsi_disk_acb4000_init _TME_P((struct tme_scsi_disk *));

_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_format_unit);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_read0);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_write0);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_mode_select);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_mode_sense);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_start_stop);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_prevent_allow);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_read_capacity);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_read1);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_write1);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_disk_cdb_inquiry);

#endif /* !_TME_SCSI_SCSI_DISK_H */
