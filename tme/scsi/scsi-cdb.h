/* $Id: scsi-cdb.h,v 1.3 2007/02/15 01:29:37 fredette Exp $ */

/* tme/scsi/scsi-cdb.h - header file describing SCSI CDBs: */

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

#ifndef _TME_SCSI_SCSI_CDB_H
#define _TME_SCSI_SCSI_CDB_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-cdb.h,v 1.3 2007/02/15 01:29:37 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-device.h>

/* macros: */

/* CDB groups: */
#define TME_SCSI_CDB_GROUP_MASK		(0xe0)
#define TME_SCSI_CDB_GROUP_0		(0x00)
#define TME_SCSI_CDB_GROUP_1		(0x20)
#define TME_SCSI_CDB_GROUP_2		(0x40)
#define TME_SCSI_CDB_GROUP_3		(0x60)
#define TME_SCSI_CDB_GROUP_4		(0x80)
#define TME_SCSI_CDB_GROUP_5		(0xa0)
#define TME_SCSI_CDB_GROUP_6		(0xc0)
#define TME_SCSI_CDB_GROUP_7		(0xe0)

/* CDB group lengths: */
#define TME_SCSI_CDB_GROUP_0_LEN	(6)
#define TME_SCSI_CDB_GROUP_1_LEN	(10)
#define TME_SCSI_CDB_GROUP_2_LEN	(10)
#undef  TME_SCSI_CDB_GROUP_3_LEN	/* reserved */
#define TME_SCSI_CDB_GROUP_4_LEN	(16)
#define TME_SCSI_CDB_GROUP_5_LEN	(12)
#undef  TME_SCSI_CDB_GROUP_6_LEN	/* vendor unique */
#undef  TME_SCSI_CDB_GROUP_7_LEN	/* vendor unique */

/* peripheral device types: */
#define TME_SCSI_TYPE_DISK		(0x00)
#define TME_SCSI_TYPE_TAPE		(0x01)
#define TME_SCSI_TYPE_CDROM		(0x05)

/* LUN states: */
#define TME_SCSI_LUN_PRESENT		(0x00)
#define TME_SCSI_LUN_NOT_PRESENT	(0x20)
#define TME_SCSI_LUN_UNSUPPORTED	(0x60)

/* SCSI formats: */
#define TME_SCSI_FORMAT_SCSI1		(0x00)
#define TME_SCSI_FORMAT_CCS		(0x01)
#define TME_SCSI_FORMAT_ISO		(0x02)

/* "Group 0 Common Commands for All Device Types" */
#define TME_SCSI_CDB_TEST_UNIT_READY	(0x00)
					/* 0x01 type specific */
					/* 0x02 vendor unique */
#define TME_SCSI_CDB_REQUEST_SENSE	(0x03)
					/* 0x04 type specific */
					/* 0x05 type specific */
					/* 0x06 vendor unique */
					/* 0x07 type specific */
					/* 0x08 type specific */
					/* 0x09 vendor unique */
					/* 0x0a type specific */
					/* 0x0b type specific */
					/* 0x0c vendor unique */
					/* 0x0d vendor unique */
					/* 0x0e vendor unique */
					/* 0x0f type specific */
					/* 0x10 type specific */
					/* 0x11 type specific */
#define TME_SCSI_CDB_INQUIRY		(0x12)
					/* 0x13 type specific */
					/* 0x14 type specific */
					/* 0x15 type specific */
					/* 0x16 type specific */
					/* 0x17 type specific */
#define TME_SCSI_CDB_COPY		(0x18)
					/* 0x19 type specific */
					/* 0x1a type specific */
					/* 0x1b type specific */
#define TME_SCSI_CDB_DIAG_RESULTS	(0x1c)
#define TME_SCSI_CDB_DIAG_SEND		(0x1d)
					/* 0x1e type specific */
					/* 0x1f reserved */

/* "Group 1 Commands for All Device Types" */
					/* 0x20 vendor unique */
					/* 0x21 vendor unique */
					/* 0x22 vendor unique */
					/* 0x23 vendor unique */
					/* 0x24 vendor unique */
					/* 0x25 type specific */
					/* 0x26 vendor unique */
					/* 0x27 vendor unique */
					/* 0x28 type specific */
					/* 0x29 vendor unique */
					/* 0x2a type specific */
					/* 0x2b type specific */
					/* 0x2c vendor unique */
					/* 0x2d vendor unique */
					/* 0x2e type specific */
					/* 0x2f type specific */
					/* 0x30 type specific */
					/* 0x31 type specific */
					/* 0x32 type specific */
					/* 0x33 type specific */
					/* 0x34 reserved */
					/* 0x35 reserved */
					/* 0x36 reserved */
					/* 0x37 reserved */
					/* 0x38 reserved */
#define TME_SCSI_CDB_COMPARE		(0x39)
#define TME_SCSI_CDB_COPY_VERIFY	(0x3a)
					/* 0x3b reserved */
					/* 0x3c reserved */
					/* 0x3d reserved */
					/* 0x3e reserved */
					/* 0x3f reserved */

/* status values: */
#define TME_SCSI_STATUS_GOOD		(0x00)
#define TME_SCSI_STATUS_CHECK_CONDITION	(0x02)
#define TME_SCSI_STATUS_COND_MET	(0x04)
#define TME_SCSI_STATUS_BUSY		(0x08)
#define TME_SCSI_STATUS_INT_GOOD	(0x10)
#define TME_SCSI_STATUS_INT_COND_MET	(0x14)
#define TME_SCSI_STATUS_RSRV_CONFLICT	(0x18)

/* extended sense keys: */
#define TME_SCSI_SENSE_EXT_KEY_NO_SENSE		(0x0)
#define TME_SCSI_SENSE_EXT_KEY_RECOVERED_ERROR	(0x1)
#define TME_SCSI_SENSE_EXT_KEY_NOT_READY	(0x2)
#define TME_SCSI_SENSE_EXT_KEY_MEDIUM_ERROR	(0x3)
#define TME_SCSI_SENSE_EXT_KEY_HARDWARE_ERROR	(0x4)
#define TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST	(0x5)
#define TME_SCSI_SENSE_EXT_KEY_UNIT_ATTENTION	(0x6)
#define TME_SCSI_SENSE_EXT_KEY_DATA_PROTECT	(0x7)
#define TME_SCSI_SENSE_EXT_KEY_BLANK_CHECK	(0x8)
						/* 0x9 vendor specific */
#define TME_SCSI_SENSE_EXT_KEY_COPY_ABORTED	(0xa)
#define TME_SCSI_SENSE_EXT_KEY_ABORTED_COMMAND	(0xb)
#define TME_SCSI_SENSE_EXT_KEY_EQUAL		(0xc)
#define TME_SCSI_SENSE_EXT_KEY_VOLUME_OVERFLOW	(0xd)
#define TME_SCSI_SENSE_EXT_KEY_MISCOMPARE	(0xe)

/* extended sense ASC and ASCQ values: */
/* NB: this is a partial list: */
#define TME_SCSI_SENSE_EXT_ASC_ASCQ_NONE		(0x0000)
#define TME_SCSI_SENSE_EXT_ASC_ASCQ_INVALID_FIELD_CDB	(0x2400)
#define TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_LIST_LENGTH_ERROR (0x1a00) 
#define TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_VALUE_INVALID	(0x2602)

/* types: */

/* SCSI inquiry data: */
struct tme_scsi_device_inquiry {

  /* the device type: */
  tme_uint8_t tme_scsi_device_inquiry_type;

  /* the LUN state: */
  tme_uint8_t tme_scsi_device_inquiry_lun_state;

  /* the device type qualifier: */
  tme_uint8_t tme_scsi_device_inquiry_type_qualifier;

  /* nonzero iff the LUN is removable: */
  tme_uint8_t tme_scsi_device_inquiry_lun_removable;

  /* the various standards versions: */
  tme_uint8_t tme_scsi_device_inquiry_std_ansi;
  tme_uint8_t tme_scsi_device_inquiry_std_ecma;
  tme_uint8_t tme_scsi_device_inquiry_std_iso;

  /* the response format: */
  tme_uint8_t tme_scsi_device_response_format;
};

/* a SCSI MODE SELECT and MODE SENSE block descriptor: */
struct tme_scsi_device_mode_blocks {

  /* the density code: */
  tme_uint8_t tme_scsi_device_mode_blocks_density_code;

  /* the number of blocks: */
  tme_uint32_t tme_scsi_device_mode_blocks_number;

  /* the length of the blocks: */
  tme_uint32_t tme_scsi_device_mode_blocks_length;
};

/* prototypes: */
_TME_SCSI_DEVICE_CDB_P(tme_scsi_device_cdb_illegal);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_device_cdb_tur);
_TME_SCSI_DEVICE_CDB_P(tme_scsi_device_cdb_request_sense);

void tme_scsi_device_mode_select_data _TME_P((struct tme_scsi_device *,
					      int (*) _TME_P((struct tme_scsi_device *,
							      _tme_const struct tme_scsi_device_mode_blocks *)),
					      int (*) _TME_P((struct tme_scsi_device *,
							      const tme_uint8_t *))));
tme_uint8_t *tme_scsi_device_make_inquiry_data _TME_P((struct tme_scsi_device *,
						       _tme_const struct tme_scsi_device_inquiry *));

#endif /* !_TME_SCSI_SCSI_CDB_H */
