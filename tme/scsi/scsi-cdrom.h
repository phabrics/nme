/* $Id: scsi-cdrom.h,v 1.2 2009/09/26 13:06:47 fredette Exp $ */

/* tme/scsi/scsi-cdrom.h - header file for generic SCSI CD-ROM emulation: */

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

#ifndef _TME_SCSI_SCSI_CDROM_H
#define _TME_SCSI_SCSI_CDROM_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-cdrom.h,v 1.2 2009/09/26 13:06:47 fredette Exp $");

/* includes: */
/* NB: for now, CDROMs reuse the disk implementation directly: */
#include <tme/generic/disk.h>
#include <tme/scsi/scsi-disk.h>
#include <tme/scsi/scsi-device.h>
#include <tme/scsi/scsi-cdb.h>
#include <tme/scsi/scsi-msg.h>

/* macros: */

/* "Group 0 Commands for CD-ROMs" */
					/* 0x00 common */
#define TME_SCSI_CDB_CDROM_REZERO_UNIT	(0x01)
					/* 0x02 vendor unique */
					/* 0x03 common */
					/* 0x04 reserved */
					/* 0x05 vendor unique */
					/* 0x06 vendor unique */
					/* 0x07 reserved */
#define TME_SCSI_CDB_CDROM_READ0	(0x08)
					/* 0x09 vendor unique */
					/* 0x0a reserved */
#define TME_SCSI_CDB_CDROM_SEEK0	(0x0b)
					/* 0x0c vendor unique */
					/* 0x0d vendor unique */
					/* 0x0e vendor unique */
					/* 0x0f vendor unique */
					/* 0x10 vendor unique */
					/* 0x11 vendor unique */
					/* 0x12 common */
					/* 0x13 vendor unique */
					/* 0x14 vendor unique */
#define TME_SCSI_CDB_CDROM_MODE_SELECT0	(0x15)
#define TME_SCSI_CDB_CDROM_RESERVE	(0x16)
#define TME_SCSI_CDB_CDROM_RELEASE	(0x17)
					/* 0x18 common */
					/* 0x19 vendor unique */
#define TME_SCSI_CDB_CDROM_MODE_SENSE0	(0x1a)
#define TME_SCSI_CDB_CDROM_START_STOP	(0x1b)
					/* 0x1c common */
					/* 0x1d common */
#define TME_SCSI_CDB_CDROM_PREVENT_ALLOW	(0x1e)
					/* 0x1f common */

/* "Group 1 Commands for CD-ROMs" */
					/* 0x20 vendor unique */
					/* 0x21 vendor unique */
					/* 0x22 vendor unique */
					/* 0x23 vendor unique */
					/* 0x24 vendor unique */
#define TME_SCSI_CDB_CDROM_READ_CAPACITY	(0x25)
					/* 0x26 vendor unique */
					/* 0x27 vendor unique */
#define TME_SCSI_CDB_CDROM_READ1	(0x28)
					/* 0x29 vendor unique */
					/* 0x2a reserved */
#define TME_SCSI_CDB_CDROM_SEEK1	(0x2b)
					/* 0x2c vendor unique */
					/* 0x2d vendor unique */
					/* 0x2e reserved */
#define TME_SCSI_CDB_CDROM_VERIFY1	(0x2f)
#define TME_SCSI_CDB_CDROM_SEARCH_HIGH1	(0x30)
#define TME_SCSI_CDB_CDROM_SEARCH_EQUAL1	(0x31)
#define TME_SCSI_CDB_CDROM_SEARCH_LOW1	(0x32)
#define TME_SCSI_CDB_CDROM_SET_LIMITS1	(0x33)
#define TME_SCSI_CDB_CDROM_PRE_FETCH	(0x34)
#define TME_SCSI_CDB_CDROM_CACHE_SYNC	(0x35)
#define TME_SCSI_CDB_CDROM_CACHE_LOCKING	(0x36)
					/* 0x37 reserved */
					/* 0x38 reserved */
#define TME_SCSI_CDB_CDROM_COMPARE	(0x39)
#define TME_SCSI_CDB_CDROM_COPY_AND_VERIFY	(0x3a)
#define TME_SCSI_CDB_CDROM_WRITE_BUFFER	(0x3b)
#define TME_SCSI_CDB_CDROM_READ_BUFFER	(0x3c)
					/* 0x3d reserved */
#define TME_SCSI_CDB_CDROM_READ_LONG	(0x3e)
					/* 0x3f reserved */

/* "Group 2 Commands for CD-ROMs" */
#define TME_SCSI_CDB_CDROM_CHANGE_DEFINITION	(0x40)
					/* 0x41 reserved */
#define TME_SCSI_CDB_CDROM_READ_SUBCHANNEL	(0x42)
#define TME_SCSI_CDB_CDROM_READ_TOC	(0x43)
#define TME_SCSI_CDB_CDROM_READ_HEADER	(0x44)
#define TME_SCSI_CDB_CDROM_PLAY_AUDIO2	(0x45)
					/* 0x46 reserved */
#define TME_SCSI_CDB_CDROM_PLAY_AUDIO_MSF	(0x47)
#define TME_SCSI_CDB_CDROM_PLAY_AUDIO_TRACK_INDEX	(0x48)
#define TME_SCSI_CDB_CDROM_PLAY_TRACK_RELATIVE2	(0x49)
					/* 0x4a reserved */
#define TME_SCSI_CDB_CDROM_PAUSE_RESUME	(0x4b)
#define TME_SCSI_CDB_CDROM_LOG_SELECT	(0x4c)
#define TME_SCSI_CDB_CDROM_LOG_SENSE	(0x4d)
					/* 0x4e reserved */
					/* 0x4f reserved */
					/* 0x50 reserved */
#define TME_SCSI_CDB_CDROM_READ_DISC_INFORMATION (0x51)
					/* 0x52 reserved */
					/* 0x53 reserved */
					/* 0x54 reserved */
#define TME_SCSI_CDB_CDROM_MODE_SELECT2	(0x55)
					/* 0x56 reserved */
					/* 0x57 reserved */
					/* 0x58 reserved */
					/* 0x59 reserved */
#define TME_SCSI_CDB_CDROM_MODE_SENSE2	(0x5a)
					/* 0x5b reserved */
					/* 0x5c reserved */
					/* 0x5d reserved */
					/* 0x5e reserved */
					/* 0x5f reserved */

/* "Group 5 Commands for CD-ROMs" */
					/* 0xa0 reserved */
					/* 0xa1 reserved */
					/* 0xa2 reserved */
					/* 0xa3 reserved */
					/* 0xa4 reserved */
#define TME_SCSI_CDB_CDROM_PLAY_AUDIO5	(0xa5)
					/* 0xa6 reserved */
					/* 0xa7 reserved */
#define TME_SCSI_CDB_CDROM_READ5	(0xa8)
#define TME_SCSI_CDB_CDROM_PLAY_TRACK_RELATIVE5	(0xa9)
					/* 0xaa reserved */
					/* 0xab reserved */
					/* 0xac reserved */
					/* 0xad reserved */
					/* 0xae reserved */
#define TME_SCSI_CDB_CDROM_VERIFY5	(0xaf)
#define TME_SCSI_CDB_CDROM_SEARCH_DATA_HIGH5	(0xb0)
#define TME_SCSI_CDB_CDROM_SEARCH_DATA_EQUAL5	(0xb1)
#define TME_SCSI_CDB_CDROM_SEARCH_DATA_LOW5	(0xb2)
#define TME_SCSI_CDB_CDROM_SET_LIMITS5	(0xb3)
					/* 0xb4 reserved */
					/* 0xb5 reserved */
					/* 0xb6 reserved */
					/* 0xb7 reserved */
					/* 0xb8 reserved */
					/* 0xb9 reserved */
					/* 0xba reserved */
					/* 0xbb reserved */
					/* 0xbc reserved */
					/* 0xbd reserved */
					/* 0xbe reserved */
					/* 0xbf reserved */

/* types: */

#define tme_scsi_cdrom_connection tme_scsi_disk_connection
#define tme_scsi_cdrom_connection_lun tme_scsi_disk_connection_lun

#define tme_scsi_cdrom tme_scsi_disk
#define tme_scsi_cdrom_device tme_scsi_disk_device
#define tme_scsi_cdrom_element tme_scsi_disk_element
#define tme_scsi_cdrom_mutex tme_scsi_disk_mutex
#define tme_scsi_cdrom_type tme_scsi_disk_type
#define tme_scsi_cdrom_connections tme_scsi_disk_connections

/* prototypes: */
#define tme_scsi_cdrom_connections_new tme_scsi_disk_connections_new
#define tme_scsi_cdrom_connection_make tme_scsi_disk_connection_make
#define tme_scsi_cdrom_connection_break tme_scsi_disk_connection_break
#define tme_scsi_cdrom_control tme_scsi_disk_control
int tme_scsi_cdrom_tme_init _TME_P((struct tme_scsi_cdrom *));

_TME_SCSI_DEVICE_CDB_P(tme_scsi_cdrom_cdb_inquiry);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_cdrom_cdb_read_toc);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_cdrom_cdb_mode_select);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_cdrom_cdb_mode_sense);
#define tme_scsi_cdrom_cdb_read0 tme_scsi_disk_cdb_read0
#define tme_scsi_cdrom_cdb_start_stop tme_scsi_disk_cdb_start_stop
#define tme_scsi_cdrom_cdb_prevent_allow tme_scsi_disk_cdb_prevent_allow
#define tme_scsi_cdrom_cdb_read_capacity tme_scsi_disk_cdb_read_capacity
#define tme_scsi_cdrom_cdb_read1 tme_scsi_disk_cdb_read1

#endif /* !_TME_SCSI_SCSI_CDROM_H */
