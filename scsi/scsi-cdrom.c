/* $Id: scsi-cdrom.c,v 1.3 2009/09/26 13:07:39 fredette Exp $ */

/* scsi/scsi-cdrom.c - implementation of SCSI CD-ROM emulation: */

/*
 * Copyright (c) 2007 Matt Fredette
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
_TME_RCSID("$Id: scsi-cdrom.c,v 1.3 2009/09/26 13:07:39 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-cdrom.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */

/* macros: */

/* globals: */

/* the list of cdroms that we emulate: */
const struct {

  /* the type name: */
  const char *_tme_scsi_cdrom_list_type;

  /* the initialization function: */
  int (*_tme_scsi_cdrom_list_init) _TME_P((struct tme_scsi_cdrom *));
} _tme_scsi_cdrom_list[] = {
  
  /* the generic TME SCSI-1 cdrom: */
  { "tme-scsi-1", tme_scsi_cdrom_tme_init },
};

/* this implements the CD-ROM INQUIRY command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_cdrom_cdb_inquiry)
{
  int lun;
  struct tme_scsi_device_inquiry inquiry;
  tme_uint8_t *data;
  
  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* this is a CD-ROM: */
  inquiry.tme_scsi_device_inquiry_type = TME_SCSI_TYPE_CDROM;

  /* if this LUN is defined: */
  inquiry.tme_scsi_device_inquiry_lun_state
    = ((scsi_device->tme_scsi_device_luns
	& TME_BIT(lun))
       ? TME_SCSI_LUN_PRESENT
       : TME_SCSI_LUN_UNSUPPORTED);

  /* the device type qualifier: */
  inquiry.tme_scsi_device_inquiry_type_qualifier = 0x00;

  /* nonzero iff the LUN is removable: */
  inquiry.tme_scsi_device_inquiry_lun_removable = TRUE;

  /* the various standards versions: */
  inquiry.tme_scsi_device_inquiry_std_ansi = 1;
  inquiry.tme_scsi_device_inquiry_std_ecma = 1;
  inquiry.tme_scsi_device_inquiry_std_iso = 1;

  /* the response format: */
  inquiry.tme_scsi_device_response_format = TME_SCSI_FORMAT_CCS;

  /* make the inquiry data: */
  data
    = tme_scsi_device_make_inquiry_data(scsi_device,
					&inquiry);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN((data
	       - scsi_device->tme_scsi_device_dma.tme_scsi_dma_out),
	      scsi_device->tme_scsi_device_cdb[4]);
  
  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the CD-ROM READ TOC command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_cdrom_cdb_read_toc)
{
  struct tme_scsi_cdrom *scsi_cdrom;
  struct tme_scsi_cdrom_connection *conn_scsi_cdrom;
  struct tme_disk_connection *conn_disk;
  tme_uint8_t *data;
  tme_uint32_t space;
  tme_uint32_t length;
  int lun;
  
  /* recover our CD-ROM: */
  scsi_cdrom = (struct tme_scsi_cdrom *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the CD-ROM connection: */
  conn_scsi_cdrom
    = scsi_cdrom->tme_scsi_cdrom_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_cdrom->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  /* we require that the MSF bit be zero: */
  if (scsi_device->tme_scsi_device_cdb[1] & 0x02) {
    tme_scsi_device_check_condition(scsi_device, 
				    TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				    TME_SCSI_SENSE_EXT_ASC_ASCQ_INVALID_FIELD_CDB);
    return;
  }

  /* "If the starting track field is not valid for the currently
     installed medium, the command shall be terminated with CHECK
     CONDITION status.  The sense key shall be set to ILLEGAL REQUEST
     and the additional sense code set to INVALID FIELD IN CDB." */
  /* NB: for now, we support only one track: */
  if (scsi_device->tme_scsi_device_cdb[6] > 1) {
    tme_scsi_device_check_condition(scsi_device, 
				    TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				    TME_SCSI_SENSE_EXT_ASC_ASCQ_INVALID_FIELD_CDB);
    return;
  }

  /* start returning the data: */
  data = &scsi_device->tme_scsi_device_data[0];

  /* bytes 0 and 1 are the TOC data length.  we will fill this in later: */
  data += 2;

  /* the first and last track numbers: */
  /* NB: for now, we support only one track: */
  *(data++) = 1;
  *(data++) = 1;

  /* one TOC track descriptor: */

  /* a reserved byte: */
  data++;
  
  /* the ADR and control byte: */
  /* NB: this specifies no sub-channel Q mode, and a data track: */
  *(data++) = (0x0 << 4) | (1 << 2);

  /* the track number: */
  *(data++) = 1;

  /* this track begins at block zero: */
  *(data++) = 0;
  *(data++) = 0;
  *(data++) = 0;
  *(data++) = 0;

  /* fill in the TOC data length, which does not include the TOC data
     length field itself: */
  length = (data - (&scsi_device->tme_scsi_device_data[0] + sizeof(tme_uint16_t)));
  scsi_device->tme_scsi_device_data[0] = (length >> 8);
  scsi_device->tme_scsi_device_data[1] = length;

  /* set the DMA pointer and length: */
  space = scsi_device->tme_scsi_device_cdb[7];
  space = (space << 8) + scsi_device->tme_scsi_device_cdb[8];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN((unsigned long) (data - &scsi_device->tme_scsi_device_data[0]),
	      space);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the CD-ROM MODE SENSE command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_cdrom_cdb_mode_sense)
{
  struct tme_scsi_cdrom *scsi_cdrom;
  struct tme_scsi_cdrom_connection *conn_scsi_cdrom;
  tme_uint8_t *data;
  tme_uint32_t block_size;
  tme_uint32_t length;
  int is_group0_cmd;
  int lun;
  
  /* see if this is a Group 0 MODE SELECT: */
  is_group0_cmd
    = ((scsi_device->tme_scsi_device_cdb[0]
	& TME_SCSI_CDB_GROUP_MASK)
       == TME_SCSI_CDB_GROUP_0);
  
  /* recover our CD-ROM: */
  scsi_cdrom = (struct tme_scsi_cdrom *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the CD-ROM connection: */
  conn_scsi_cdrom
    = scsi_cdrom->tme_scsi_cdrom_connections[lun];

  data = &scsi_device->tme_scsi_device_data[0];

  /* the Mode Data Length is one byte for the Group 0 command and two
     bytes otherwise.  we will fill this in later: */
  data += (is_group0_cmd ? 1 : 2);

  /* the Medium Type is one byte: */
  *(data++) = 0x00; /* default (only one medium type supported) */

  /* the Device Specific Parameter is one byte: */
  *(data++) = 0x00; /* no DPO/FUA support */

  /* if this is not the Group 0 command, skip two reserved bytes: */
  if (!is_group0_cmd) {
    data += 2;
  }

  /* the Block Descriptor Length is one byte for the Group 0 command
     and two bytes otherwise.  we will fill this in later: */
  data += (is_group0_cmd ? 1 : 2);

  /* the first Block Descriptor: */
  
  /* the Block Descriptor density code: */
  *(data++) = 0x01; /* User Data only - 2048 bytes per physical sector */

  /* the Number of Blocks: */
  /* "A value of zero indicates that all of the remaining logical
     blocks of the logical unit shall have the medium characteristics
     specified." */
  *(data++) = 0x00;
  *(data++) = 0x00;
  *(data++) = 0x00;

  /* a reserved byte: */
  data++;

  /* the Block Length: */
  block_size
    = conn_scsi_cdrom->tme_scsi_disk_connection_block_size.tme_value64_uint32_lo;
  *(data++) = (block_size >> 16) & 0xff;
  *(data++) = (block_size >>  8) & 0xff;
  *(data++) = (block_size >>  0) & 0xff;

  /* fill in the Block Descriptor Length: */
  if (is_group0_cmd) {
    length = (data - &scsi_device->tme_scsi_device_data[4]);
    scsi_device->tme_scsi_device_data[3] = length;
  }
  else {
    length = (data - &scsi_device->tme_scsi_device_data[8]);
    scsi_device->tme_scsi_device_data[6] = (length >> 8);
    scsi_device->tme_scsi_device_data[7] = length;
  }

  /* there are no vendor-unique bytes: */

  /* fill in the Mode Data Length: */
  if (is_group0_cmd) {
    length = (data - &scsi_device->tme_scsi_device_data[1]);
    scsi_device->tme_scsi_device_data[0] = length;
  }
  else {
    length = (data - &scsi_device->tme_scsi_device_data[2]);
    scsi_device->tme_scsi_device_data[0] = (length >> 8);
    scsi_device->tme_scsi_device_data[1] = length;
  }

  /* set the DMA pointer and length: */
  if (is_group0_cmd) {
    length = scsi_device->tme_scsi_device_cdb[4];
  }
  else {
    length = scsi_device->tme_scsi_device_cdb[7];
    length = (length << 8) + scsi_device->tme_scsi_device_cdb[8];
  }
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN((unsigned long)
	      (data
	       - &scsi_device->tme_scsi_device_data[0]),
	      length);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this handles block descriptors from a MODE SELECT command: */
static int
_tme_scsi_cdrom_do_mode_select_blocks(struct tme_scsi_device *scsi_device,
				      const struct tme_scsi_device_mode_blocks *blocks)
{
  struct tme_scsi_cdrom *scsi_cdrom;
  struct tme_scsi_cdrom_connection *conn_scsi_cdrom;
  int lun;

  /* recover our CD-ROM: */
  scsi_cdrom = (struct tme_scsi_cdrom *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the CD-ROM connection: */
  conn_scsi_cdrom
    = scsi_cdrom->tme_scsi_cdrom_connections[lun];

  /* XXX FIXME - this needs to be implemented correctly: */

  /* if this block descriptor doesn't describe the entire CD-ROM: */
  if (blocks->tme_scsi_device_mode_blocks_number != 0) {
    tme_scsi_device_check_condition(scsi_device, 
				    TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				    TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_VALUE_INVALID);
    return (-1);
  }

  /* dispatch on the density code: */
  switch (blocks->tme_scsi_device_mode_blocks_density_code) {
  case 0x00:	/* Default density code */
  case 0x01:	/* User Data Only, 2048 bytes per physical sector */
    break;
  default:
    tme_scsi_device_check_condition(scsi_device, 
				    TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				    TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_VALUE_INVALID);
    return (-1);
  }

  /* the logical block size must be an exact divisor or an integral
     multiple of 2048: */
  if (blocks->tme_scsi_device_mode_blocks_length == 0
      || ((blocks->tme_scsi_device_mode_blocks_length
	   < 2048)
	  ? (2048 % blocks->tme_scsi_device_mode_blocks_length)
	  : (blocks->tme_scsi_device_mode_blocks_length % 2048))) {
    tme_scsi_device_check_condition(scsi_device, 
				    TME_SCSI_SENSE_EXT_KEY_ILLEGAL_REQUEST,
				    TME_SCSI_SENSE_EXT_ASC_ASCQ_PARAMETER_VALUE_INVALID);
    return (-1);
  }

  /* update the logical block size: */
  conn_scsi_cdrom->tme_scsi_disk_connection_block_size.tme_value64_uint32_lo
    = blocks->tme_scsi_device_mode_blocks_length;

  return (0);
}

/* this handles a mode page from a CD-ROM MODE SELECT command: */
static int
_tme_scsi_cdrom_do_mode_select_page(struct tme_scsi_device *scsi_device,
				    const tme_uint8_t *mode_page)
{
  return (0);
}

/* this processes the parameter list from a CD-ROM MODE SELECT command: */
static
_TME_SCSI_DEVICE_PHASE_DECL(_tme_scsi_cdrom_mode_select_data)
{
  tme_scsi_device_mode_select_data(scsi_device,
				   _tme_scsi_cdrom_do_mode_select_blocks,
				   _tme_scsi_cdrom_do_mode_select_page);
}

/* this implements the CD-ROM MODE SELECT command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_cdrom_cdb_mode_select)
{
  tme_uint32_t length;
  int is_group0_cmd;
  
  /* see if this is a Group 0 MODE SELECT: */
  is_group0_cmd
    = ((scsi_device->tme_scsi_device_cdb[0]
	& TME_SCSI_CDB_GROUP_MASK)
       == TME_SCSI_CDB_GROUP_0);
  
  /* read in the parameter list: */
  if (is_group0_cmd) {
    length = scsi_device->tme_scsi_device_cdb[4];
  }
  else {
    length = scsi_device->tme_scsi_device_cdb[7];
    length = (length << 8) + scsi_device->tme_scsi_device_cdb[8];
  }
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN(sizeof(scsi_device->tme_scsi_device_data),
	      length);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = NULL;

  /* transfer the parameter list: */
  tme_scsi_device_target_phase(scsi_device,
			       (TME_SCSI_SIGNAL_BSY
				| TME_SCSI_PHASE_DATA_OUT));
  scsi_device->tme_scsi_device_phase
    = _tme_scsi_cdrom_mode_select_data;
}

/* the new SCSI CD-ROM function: */
TME_ELEMENT_SUB_NEW_DECL(tme_scsi,cdrom) {
  int id;
  const char *cdrom_type;
  const char *vendor;
  const char *product;
  const char *revision;
  struct tme_scsi_cdrom *scsi_cdrom;
  struct tme_scsi_device *scsi_device;
  int arg_i;
  int usage;
  unsigned int cdrom_list_i;
  int (*cdrom_init) _TME_P((struct tme_scsi_cdrom *));
  int rc;

  /* check our arguments: */
  id = -1;
  cdrom_type = NULL;
  vendor = NULL;
  product = NULL;
  revision = NULL;
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the SCSI ID: */
    if (TME_ARG_IS(args[arg_i], "id")
	&& id < 0
	&& (id = tme_scsi_id_parse(args[arg_i + 1])) >= 0) {
      arg_i += 2;
    }

    /* the CD-ROM type: */
    else if (TME_ARG_IS(args[arg_i], "type")
	     && cdrom_type == NULL
	     && args[arg_i + 1] != NULL) {
      cdrom_type = args[arg_i + 1];
      arg_i += 2;
    }

    /* any inquiry vendor, product, or revision: */
    else if (TME_ARG_IS(args[arg_i], "vendor")
	     && vendor == NULL
	     && args[arg_i + 1] != NULL) {
      vendor = args[arg_i + 1];
      arg_i += 2;
    }
    else if (TME_ARG_IS(args[arg_i], "product")
	     && product == NULL
	     && args[arg_i + 1] != NULL) {
      product = args[arg_i + 1];
      arg_i += 2;
    }
    else if (TME_ARG_IS(args[arg_i], "revision")
	     && revision == NULL
	     && args[arg_i + 1] != NULL) {
      revision = args[arg_i + 1];
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      /* we must have been given an ID and a type: */
      if (id < 0
	  || cdrom_type == NULL) {
	usage = TRUE;
      }
      break;
    }

    /* this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s id %s type %s [ vendor %s ] [ product %s ] [ revision %s ]",
			    _("usage:"),
			    args[0],
			    _("TYPE"),
			    _("ID"),
			    _("VENDOR"),
			    _("PRODUCT"),
			    _("REVISION"));
    return (EINVAL);
  }

  /* make sure that this CD-ROM type is known: */
  cdrom_init = NULL;
  for (cdrom_list_i = 0;
       cdrom_list_i < TME_ARRAY_ELS(_tme_scsi_cdrom_list);
       cdrom_list_i++) {
    if (!strcmp(_tme_scsi_cdrom_list[cdrom_list_i]._tme_scsi_cdrom_list_type,
		cdrom_type)) {
      cdrom_init = _tme_scsi_cdrom_list[cdrom_list_i]._tme_scsi_cdrom_list_init;
      break;
    }
  }
  if (cdrom_init == NULL) {
    tme_output_append_error(_output, "%s", cdrom_type);
    return (ENOENT);
  }

  /* start the CD-ROM structure: */
  scsi_cdrom = tme_new0(struct tme_scsi_cdrom, 1);
  scsi_cdrom->tme_scsi_cdrom_element = element;
  scsi_cdrom->tme_scsi_cdrom_type = tme_strdup(cdrom_type);

  /* initialize the generic SCSI device structure: */
  scsi_device = &scsi_cdrom->tme_scsi_cdrom_device;
  rc = tme_scsi_device_new(scsi_device, id);
  assert (rc == TME_OK);

  scsi_device->tme_scsi_device_vendor
    = tme_strdup((vendor == NULL)
		 ? "TME"
		 : vendor);
  scsi_device->tme_scsi_device_product
    = tme_strdup((product == NULL)
		 ? "CDROM"
		 : product);
  scsi_device->tme_scsi_device_revision
    = tme_strdup((revision == NULL)
		 ? "0000"
		 : revision);
  
  /* set the commands for CD-ROMs: */
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_INQUIRY,
			 tme_scsi_cdrom_cdb_inquiry);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ0,
			 tme_scsi_cdrom_cdb_read0);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_MODE_SELECT0,
			 tme_scsi_cdrom_cdb_mode_select);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_MODE_SENSE0,
			 tme_scsi_cdrom_cdb_mode_sense);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_START_STOP,
			 tme_scsi_cdrom_cdb_start_stop);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_PREVENT_ALLOW,
			 tme_scsi_cdrom_cdb_prevent_allow);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ_CAPACITY,
			 tme_scsi_cdrom_cdb_read_capacity);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ1,
			 tme_scsi_cdrom_cdb_read1);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ_SUBCHANNEL,
			 tme_scsi_device_cdb_illegal);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ_TOC,
			 tme_scsi_cdrom_cdb_read_toc);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_PLAY_AUDIO2,
			 tme_scsi_device_cdb_illegal);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_READ_DISC_INFORMATION,
			 tme_scsi_device_cdb_illegal);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_CDROM_CACHE_SYNC,
			 tme_scsi_device_cdb_illegal);

  /* call the type-specific initialization function: */
  rc = (*cdrom_init)(scsi_cdrom);
  assert (rc == TME_OK);

  /* fill the element: */
  element->tme_element_private = scsi_cdrom;
  element->tme_element_connections_new = tme_scsi_cdrom_connections_new;

  return (TME_OK);
}
