/* $Id: scsi-cdb.c,v 1.5 2007/08/25 22:52:18 fredette Exp $ */

/* scsi/scsi-cdb.c - implementation of generic SCSI device CDB support: */

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
_TME_RCSID("$Id: scsi-cdb.c,v 1.5 2007/08/25 22:52:18 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-cdb.h>
#include <tme/scsi/scsi-msg.h>

/* this handles any illegal request: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_device_cdb_illegal)
{
  struct tme_scsi_device_sense *sense;
  int lun;

  /* get the addressed LUN's sense: */
  lun = scsi_device->tme_scsi_device_addressed_lun;
  sense = &scsi_device->tme_scsi_device_sense[lun];

  /* if this target does not support extended sense: */
  if (scsi_device->tme_scsi_device_sense_no_extended) {

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
  }

  /* otherwise, return an extended sense: */
  else {

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
  }

  /* return the CHECK CONDITION status: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_CHECK_CONDITION,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this handles the TEST UNIT READY command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_device_cdb_tur)
{
  /* finish the command: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_GOOD,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this handles the REQUEST SENSE command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_device_cdb_request_sense)
{
  int lun;
  struct tme_scsi_device_sense *sense;
  unsigned long transfer_length;
  
  /* get the addressed LUN's sense: */
  lun = scsi_device->tme_scsi_device_addressed_lun;
  sense = &scsi_device->tme_scsi_device_sense[lun];

  /* if the sense is not valid: */
  if (!sense->tme_scsi_device_sense_valid) {

    /* if this device doesn't do extended sense: */
    if (scsi_device->tme_scsi_device_sense_no_extended) {

      /* conjure up a no-error nonextended sense: */
      sense->tme_scsi_device_sense_data[0]
	= 0;
      sense->tme_scsi_device_sense_data[1]
	= 0;
      sense->tme_scsi_device_sense_data[2]
	= 0;
      sense->tme_scsi_device_sense_data[3]
	= 0;
      sense->tme_scsi_device_sense_valid = 4;
    }

    /* otherwise, this device does do extended sense: */
    else {
      
      /* the error class and error code: */
      sense->tme_scsi_device_sense_data[0]
	= 0x70;

      /* the NO SENSE sense key: */
      sense->tme_scsi_device_sense_data[2]
	= 0x00;

      /* the additional sense length: */
      sense->tme_scsi_device_sense_data[7]
	= 0x00;
    }
  }

  /* see how much space for sense bytes the initiator
     has allocated.  zero means four: */
  transfer_length
    = scsi_device->tme_scsi_device_cdb[4];
  if (transfer_length == 0) {
    transfer_length = 4;
  }

  /* bound this with the number of sense bytes we have available.  an
     extended sense has a length of 8 plus the additional sense
     length.  any other sense must specify how long it is in the
     tme_scsi_device_sense_valid field (a standard nonextended sense
     always has a length of four): */
  transfer_length
    = TME_MIN(transfer_length,
	      (((sense->tme_scsi_device_sense_data[0]
		 & 0x70)
		== 0x70)
	       ? ((unsigned int) 8
		  + sense->tme_scsi_device_sense_data[7])
	       : sense->tme_scsi_device_sense_valid));

  /* set up to transfer the sense: */
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = &sense->tme_scsi_device_sense_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = transfer_length;

  /* the sense is no longer valid: */
  sense->tme_scsi_device_sense_valid = FALSE;

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this processes the parameter list from a MODE SELECT command: */
void
tme_scsi_device_mode_select_data(struct tme_scsi_device *scsi_device,
				 int (*do_mode_select_blocks) _TME_P((struct tme_scsi_device *,
								      const struct tme_scsi_device_mode_blocks *)),
				 int (*do_mode_select_page) _TME_P((struct tme_scsi_device *,
								    const tme_uint8_t *)))
{
  const tme_uint8_t *data;
  const tme_uint8_t *data_end;
  tme_uint32_t length;
  tme_uint32_t block_descriptor_length;
  struct tme_scsi_device_mode_blocks blocks_buffer;
  int is_group0_cmd;
  
  /* see if this is a Group 0 MODE SELECT: */
  is_group0_cmd
    = ((scsi_device->tme_scsi_device_cdb[0]
	& TME_SCSI_CDB_GROUP_MASK)
       == TME_SCSI_CDB_GROUP_0);

  /* "A parameter list length of zero indicates that no data shall be
     transferred.  This condition shall not be considered as an error." */
  if (scsi_device->tme_scsi_device_cdb[4] == 0) {
    tme_scsi_device_target_do_smf(scsi_device,
				  TME_SCSI_STATUS_GOOD,
				  TME_SCSI_MSG_CMD_COMPLETE);
    return;
  }

  /* get a pointer to the first byte of data, and a pointer past the
     last byte of data: */
  data = &scsi_device->tme_scsi_device_data[0];
  length = scsi_device->tme_scsi_device_cdb[4];
  data_end = (data
	      + TME_MIN(sizeof(scsi_device->tme_scsi_device_data),
			length));

  /* "A parameter list length that results in the truncation of any
     descriptor, header or page of parameters shall cause the target
     to terminate the command with CHECK CONDITION status.  The sense
     key shall be set to ILLEGAL REQUEST, and the additional sense
     code shall be set to PARAMETER LIST LENGTH ERROR. " */

  /* process the mode data: */
  do {

    /* skip a reserved byte (in a MODE SENSE command, this byte would
       be the Mode Data Length): */
    data++;

    /* skip the Medium Type and the Device Specific Parameter: */
    if ((data_end - data) < 2) break;
    data += 2;

    /* if this is not the Group 0 command, skip two reserved bytes: */
    if (!is_group0_cmd) {
      if ((data_end - data) < 2) break;
      data += 2;
    }

    /* get the Block Descriptor Length: */
    if (data == data_end) break;
    block_descriptor_length = *(data++);
    if (!is_group0_cmd) {
      if (data == data_end) break;
      block_descriptor_length = (block_descriptor_length << 8) + *(data++);
    }

    /* check the Block Descriptors: */
    if (((unsigned long) (data_end - data)) < block_descriptor_length
	|| (block_descriptor_length % 8) != 0) {
      break;
    }
    for (;
	 block_descriptor_length >= 8;
	 block_descriptor_length -= 8) {

      /* get the density code: */
      blocks_buffer.tme_scsi_device_mode_blocks_density_code = *(data++);

      /* get the number of blocks: */
      blocks_buffer.tme_scsi_device_mode_blocks_number
	= ((((tme_uint32_t) data[0]) << 16)
	   | (((tme_uint32_t) data[1]) << 8)
	   | ((tme_uint32_t) data[2]));
      data += 3;

      /* skip the reserved byte: */
      data++;

      /* get the block length: */
      blocks_buffer.tme_scsi_device_mode_blocks_length
	= ((((tme_uint32_t) data[0]) << 16)
	   | (((tme_uint32_t) data[1]) << 8)
	   | ((tme_uint32_t) data[2]));
      data += 3;

      /* call back for this blocks descriptor: */
      if ((*do_mode_select_blocks)(scsi_device, &blocks_buffer)) {
	return;
      }
    }

    /* check the Mode Pages: */
    for (;
	 ((data_end - data) >= 2
	  && (data_end - data) >= (2 + data[1]));
	 data += (2 + data[1])) {

      /* call back for this Mode Page: */
      if ((*do_mode_select_page)(scsi_device, data)) {
	return;
      }
    }

    /* terminate the command with GOOD status: */
    tme_scsi_device_target_do_smf(scsi_device,
				  TME_SCSI_STATUS_GOOD,
				  TME_SCSI_MSG_CMD_COMPLETE);
    return;
  
  } while (/* CONSTCOND */ 0);

  /* terminate the command with CHECK CONDITION status: */
  tme_scsi_device_check_condition(scsi_device, 
				  TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				  TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_LIST_LENGTH_ERROR);
}

/* this adds one of the inquiry strings to the data: */
static tme_uint8_t *
_tme_scsi_device_make_inquiry_string(tme_uint8_t *data,
				     const char *string,
				     unsigned int size)
{
  tme_uint8_t c;

  for (; size-- > 0; ) {
    c = *(string++);
    if (c == '\0') {
      c = ' ';
      string--;
    }
    *(data++) = c;
  }
  return (data);
}

/* this creates INQUIRY data: */
tme_uint8_t *
tme_scsi_device_make_inquiry_data(struct tme_scsi_device *scsi_device,
				  const struct tme_scsi_device_inquiry *inquiry)
{
  tme_uint8_t *data;
  
  data = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out = data;
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in = NULL;

  /* byte 0 is the peripheral device type: */
  *(data++)
    = (inquiry->tme_scsi_device_inquiry_type
       | inquiry->tme_scsi_device_inquiry_lun_state);

  /* byte 1 is the device type qualifier: */
  *(data++)
    = (inquiry->tme_scsi_device_inquiry_type_qualifier
       | (inquiry->tme_scsi_device_inquiry_lun_removable
	  ? 0x80
	  : 0x00));

  /* byte 2 is the standards versions: */
  *(data++)
    = ((inquiry->tme_scsi_device_inquiry_std_iso << 6)
       | (inquiry->tme_scsi_device_inquiry_std_ecma << 3)
       | (inquiry->tme_scsi_device_inquiry_std_iso << 0));

  /* byte 3 is the response format: */
  *(data++)
    = inquiry->tme_scsi_device_response_format;

  /* byte 4 is the additional length.  we will come back and
     fill it later: */
  data++;

  /* bytes 5, 6, and 7 are flags, that we initialize to zero: */
  *(data++) = 0x00;
  *(data++) = 0x00;
  *(data++) = 0x00;

  /* the next eight bytes are for the vendor: */
  data
    = _tme_scsi_device_make_inquiry_string(data,
					   scsi_device->tme_scsi_device_vendor,
					   8);
    
  /* the next 16 bytes are for the product: */
  data
    = _tme_scsi_device_make_inquiry_string(data,
					   scsi_device->tme_scsi_device_product,
					   16);

  /* the next four bytes are for the revision: */
  data
    = _tme_scsi_device_make_inquiry_string(data,
					   scsi_device->tme_scsi_device_revision,
					   4);

  /* fill in the additional length byte: */
  scsi_device->tme_scsi_device_data[4]
    = (data
       - &scsi_device->tme_scsi_device_data[5]);

  return (data);
}
