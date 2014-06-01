/* $Id: scsi-disk.c,v 1.3 2003/08/07 22:11:23 fredette Exp $ */

/* scsi/scsi-disk.c - implementation of SCSI disk emulation: */

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
_TME_RCSID("$Id: scsi-disk.c,v 1.3 2003/08/07 22:11:23 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-disk.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else  /* HAVE_STDARG_H */
#include <varargs.h>
#endif /* HAVE_STDARG_H */

/* macros: */

/* globals: */

/* the list of disks that we emulate: */
const struct {

  /* the type name: */
  const char *_tme_scsi_disk_list_type;

  /* the initialization function: */
  int (*_tme_scsi_disk_list_init) _TME_P((struct tme_scsi_disk *));
} _tme_scsi_disk_list[] = {
  
  /* the generic TME SCSI-1 disk: */
  { "tme-scsi-1", tme_scsi_disk_tme_init },
  
  /* the ACB4000 emulation: */
  { "acb4000", tme_scsi_disk_acb4000_init },
};

/* this implements the disk any-Group READ and WRITE commands: */
void
tme_scsi_disk_cdb_xfer(struct tme_scsi_device *scsi_device,
		       tme_uint32_t lba,
		       tme_uint32_t transfer_length,
		       int read)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  union tme_value64 off;
  int lun;
  int rc;

  /* recover our disk: */
  scsi_disk = (struct tme_scsi_disk *) scsi_device;

  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the disk connection: */
  conn_scsi_disk
    = scsi_disk->tme_scsi_disk_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_disk->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  /* set the 64-bit offset, and the long size: */
  (void) tme_value64_set(&off, lba);
  (void) tme_value64_mul(&off,
			 &conn_scsi_disk->tme_scsi_disk_connection_block_size);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = transfer_length;
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    *= conn_scsi_disk->tme_scsi_disk_connection_block_size.tme_value64_uint32_lo;

  /* get the disk buffer: */
  if (read) {
    rc
      = ((*conn_disk->tme_disk_connection_read)
	 (conn_disk,
	  &off,
	  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid,
	  &scsi_device->tme_scsi_device_dma.tme_scsi_dma_out));
    scsi_device->tme_scsi_device_dma.tme_scsi_dma_in = NULL;
  }
  else {

    /* if this disk is read-only: */
    if (conn_disk->tme_disk_connection_write == NULL) {

      /* XXX we should return ILLEGAL REQUEST sense here: */
      abort();
    }

    rc
      = ((*conn_disk->tme_disk_connection_write)
	 (conn_disk,
	  &off,
	  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid,
	  &scsi_device->tme_scsi_device_dma.tme_scsi_dma_in));
    scsi_device->tme_scsi_device_dma.tme_scsi_dma_out = NULL;
  }

  /* if we couldn't get the disk buffer: */
  if (rc != TME_OK) {
    
    /* XXX we should return MEDIUM ERROR or HARDWARE ERROR sense here: */
    abort();
  }

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}
#define tme_scsi_disk_cdb_read(d, l, t) tme_scsi_disk_cdb_xfer(d, l, t, TRUE)
#define tme_scsi_disk_cdb_write(d, l, t) tme_scsi_disk_cdb_xfer(d, l, t, FALSE)

/* this implements the disk FORMAT UNIT command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_format_unit)
{
  abort();
}

/* this implements the disk Group 0 READ command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_read0)
{
  const tme_uint8_t *cdb;
  tme_uint32_t lba;
  tme_uint32_t transfer_length;

  cdb = &scsi_device->tme_scsi_device_cdb[0];

  /* get the LBA: */
  lba = cdb[1] & 0x1f;
  lba = (lba << 8) | cdb[2];
  lba = (lba << 8) | cdb[3];

  /* get the transfer length.  zero means 256: */
  transfer_length = cdb[4];
  if (transfer_length == 0) {
    transfer_length = 256;
  }

  /* call the generic read handler: */
  tme_scsi_disk_cdb_read(scsi_device,
			 lba,
			 transfer_length);
}

/* this implements the disk Group 0 WRITE command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_write0)
{
  const tme_uint8_t *cdb;
  tme_uint32_t lba;
  tme_uint32_t transfer_length;

  cdb = &scsi_device->tme_scsi_device_cdb[0];

  /* get the LBA: */
  lba = cdb[1] & 0x1f;
  lba = (lba << 8) | cdb[2];
  lba = (lba << 8) | cdb[3];

  /* get the transfer length.  zero means 256: */
  transfer_length = cdb[4];
  if (transfer_length == 0) {
    transfer_length = 256;
  }

  /* call the generic write handler: */
  tme_scsi_disk_cdb_write(scsi_device,
			  lba,
			  transfer_length);
}

/* this implements the disk INQUIRY command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_inquiry)
{
  int lun;
  struct tme_scsi_device_inquiry inquiry;
  tme_uint8_t *data;
  
  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* this is a direct-access device: */
  inquiry.tme_scsi_device_inquiry_type = TME_SCSI_TYPE_DISK;

  /* if this LUN is defined: */
  inquiry.tme_scsi_device_inquiry_lun_state
    = ((scsi_device->tme_scsi_device_luns
	& TME_BIT(lun))
       ? TME_SCSI_LUN_PRESENT
       : TME_SCSI_LUN_UNSUPPORTED);

  /* the device type qualifier: */
  inquiry.tme_scsi_device_inquiry_type_qualifier = 0x00;

  /* nonzero iff the LUN is removable: */
  inquiry.tme_scsi_device_inquiry_lun_removable = FALSE;

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

/* this implements the disk MODE SELECT command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_mode_select)
{
  abort();
}

/* this implements the disk MODE SENSE command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_mode_sense)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  tme_uint8_t *data;
  union tme_value64 _blocks;
  tme_uint32_t blocks, block_size;
  int lun;
  
  /* recover our disk: */
  scsi_disk = (struct tme_scsi_disk *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the disk connection: */
  conn_scsi_disk
    = scsi_disk->tme_scsi_disk_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_disk->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  data = &scsi_device->tme_scsi_device_data[0];

  /* byte 0 is the sense data length.  we will fill this in later: */
  data++;

  /* byte 1 is the medium type: */
  *(data++) = 0x00; /* default (only one medium type supported) */

  /* byte 2 is the WP (Write Protect) bit: */
  *(data++) = 0x00; /* not write protected */

  /* byte 3 is the Block Descriptor Length.  we will fill this in
     later: */
  data++;

  /* the first Block Descriptor: */
  
  /* the Block Descriptor density code: */
  *(data++) = 0x00; /* default (only one density supported) */

  /* the Number of Blocks: */
  _blocks = conn_disk->tme_disk_connection_size;
  (void) tme_value64_div(&_blocks,
			 &conn_scsi_disk->tme_scsi_disk_connection_block_size);
  blocks = _blocks.tme_value64_uint32_lo;
  *(data++) = (blocks >> 16) & 0xff;
  *(data++) = (blocks >>  8) & 0xff;
  *(data++) = (blocks >>  0) & 0xff;

  /* a reserved byte: */
  data++;

  /* the Block Length: */
  block_size
    = conn_scsi_disk->tme_scsi_disk_connection_block_size.tme_value64_uint32_lo;
  *(data++) = (block_size >> 16) & 0xff;
  *(data++) = (block_size >>  8) & 0xff;
  *(data++) = (block_size >>  0) & 0xff;

  /* fill in the Block Descriptor Length: */
  scsi_device->tme_scsi_device_data[3]
    = (data - &scsi_device->tme_scsi_device_data[4]);

  /* there are no vendor-unique bytes: */

  /* fill in the sense data length: */
  scsi_device->tme_scsi_device_data[0]
    = (data - &scsi_device->tme_scsi_device_data[1]);

  /* set the DMA pointer and length: */
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN((data
	       - &scsi_device->tme_scsi_device_data[0]),
	      scsi_device->tme_scsi_device_cdb[4]);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the disk START/STOP command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_start_stop)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  int lun;
  int rc;

  /* recover our disk: */
  scsi_disk = (struct tme_scsi_disk *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the disk connection: */
  conn_scsi_disk
    = scsi_disk->tme_scsi_disk_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_disk->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  /* call out the START or STOP control: */
  rc
    = ((*conn_disk->tme_disk_connection_control)
       (conn_disk,
	((scsi_device->tme_scsi_device_cdb[4] & 0x01)
	 ? TME_DISK_CONTROL_START
	 : TME_DISK_CONTROL_STOP)));
  assert (rc == TME_OK);

  /* finish the command: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_GOOD,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the disk PREVENT/ALLOW command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_prevent_allow)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  int lun;
  int rc;

  /* recover our disk: */
  scsi_disk = (struct tme_scsi_disk *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the disk connection: */
  conn_scsi_disk
    = scsi_disk->tme_scsi_disk_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_disk->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  /* call out the PREVENT or ALLOW control: */
  rc
    = ((*conn_disk->tme_disk_connection_control)
       (conn_disk,
	((scsi_device->tme_scsi_device_cdb[4] & 0x01)
	 ? TME_DISK_CONTROL_PREVENT
	 : TME_DISK_CONTROL_ALLOW)));
  assert (rc == TME_OK);

  /* finish the command: */
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_GOOD,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the disk READ CAPACITY command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_read_capacity)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  tme_uint8_t *data;
  union tme_value64 _blocks;
  tme_uint32_t lba, block_size;
  int lun;
  
  /* recover our disk: */
  scsi_disk = (struct tme_scsi_disk *) scsi_device;

  /* get the active LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* get the disk connection: */
  conn_scsi_disk
    = scsi_disk->tme_scsi_disk_connections[lun];
  conn_disk
    = ((struct tme_disk_connection *)
       conn_scsi_disk->tme_scsi_disk_connection.tme_disk_connection.tme_connection_other);

  data = &scsi_device->tme_scsi_device_data[0];

  /* we require that the Partial Medium Indicator bit be zero: */
  if (scsi_device->tme_scsi_device_cdb[8] & 0x01) {
    abort();
  }

  /* the last lba: */
  _blocks = conn_disk->tme_disk_connection_size;
  (void) tme_value64_div(&_blocks,
			 &conn_scsi_disk->tme_scsi_disk_connection_block_size);
  lba = _blocks.tme_value64_uint32_lo - 1;
  *(data++) = (lba >> 24) & 0xff;
  *(data++) = (lba >> 16) & 0xff;
  *(data++) = (lba >>  8) & 0xff;
  *(data++) = (lba >>  0) & 0xff;

  /* the Block Length: */
  block_size
    = conn_scsi_disk->tme_scsi_disk_connection_block_size.tme_value64_uint32_lo;
  *(data++) = (block_size >> 24) & 0xff;
  *(data++) = (block_size >> 16) & 0xff;
  *(data++) = (block_size >>  8) & 0xff;
  *(data++) = (block_size >>  0) & 0xff;

  /* set the DMA pointer and length: */
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = (data
       - &scsi_device->tme_scsi_device_data[0]);
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;

  /* finish the command: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the disk Group 1 READ command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_read1)
{
  const tme_uint8_t *cdb;
  tme_uint32_t lba;
  tme_uint32_t transfer_length;

  cdb = &scsi_device->tme_scsi_device_cdb[0];

  /* get the LBA: */
  lba = cdb[2] & 0x1f;
  lba = (lba << 8) | cdb[3];
  lba = (lba << 8) | cdb[4];
  lba = (lba << 8) | cdb[5];

  /* get the transfer length: */
  transfer_length = cdb[7];
  transfer_length = (transfer_length << 8) | cdb[8];

  /* call the generic read handler: */
  tme_scsi_disk_cdb_read(scsi_device,
			 lba,
			 transfer_length);
}

/* this implements the disk Group 1 WRITE command: */
_TME_SCSI_DEVICE_CDB_DECL(tme_scsi_disk_cdb_write1)
{
  const tme_uint8_t *cdb;
  tme_uint32_t lba;
  tme_uint32_t transfer_length;

  cdb = &scsi_device->tme_scsi_device_cdb[0];

  /* get the LBA: */
  lba = cdb[2] & 0x1f;
  lba = (lba << 8) | cdb[3];
  lba = (lba << 8) | cdb[4];
  lba = (lba << 8) | cdb[5];

  /* get the transfer length: */
  transfer_length = cdb[7];
  transfer_length = (transfer_length << 8) | cdb[8];

  /* call the generic write handler: */
  tme_scsi_disk_cdb_write(scsi_device,
			  lba,
			  transfer_length);
}

/* the disk control handler: */
#ifdef HAVE_STDARG_H
int tme_scsi_disk_control(struct tme_disk_connection *conn_disk,
			  unsigned int control,
			  ...)
#else  /* HAVE_STDARG_H */
int tme_scsi_disk_control(conn_disk, control, va_alist)
     struct tme_disk_connection *conn_disk;
     unsigned int control;
     va_dcl
#endif /* HAVE_STDARG_H */
{
  struct tme_scsi_disk *scsi_disk;

  /* recover our device: */
  scsi_disk = (struct tme_scsi_disk *) conn_disk->tme_disk_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&scsi_disk->tme_scsi_disk_mutex);

  /* unlock the mutex: */
  tme_mutex_unlock(&scsi_disk->tme_scsi_disk_mutex);

  return (TME_OK);
}

/* this breaks a connection: */
int 
tme_scsi_disk_connection_break(struct tme_connection *conn,
			       unsigned int state)
{
  abort();
}

/* this makes a new disk connection: */
int
tme_scsi_disk_connection_make(struct tme_connection *conn,
			      unsigned int state)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  int lun;

  /* both sides must be disk connections: */
  assert (conn->tme_connection_type == TME_CONNECTION_DISK);
  assert (conn->tme_connection_other->tme_connection_type == TME_CONNECTION_DISK);

  /* recover our data structures: */
  scsi_disk = conn->tme_connection_element->tme_element_private;
  conn_scsi_disk = (struct tme_scsi_disk_connection *) conn;

  /* we're always set up to answer calls across the connection,
     so we only have to do work when the connection has gone full,
     namely taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock the mutex: */
    tme_mutex_lock(&scsi_disk->tme_scsi_disk_mutex);

    /* make this disk connection: */
    lun = conn_scsi_disk->tme_scsi_disk_connection_lun;
    assert (scsi_disk->tme_scsi_disk_connections[lun]
	    == NULL);
    scsi_disk->tme_scsi_disk_connections[lun]
      = conn_scsi_disk;
    scsi_disk->tme_scsi_disk_device.tme_scsi_device_luns
      |= (1 << lun);

    /* unlock the mutex: */
    tme_mutex_unlock(&scsi_disk->tme_scsi_disk_mutex);
  }
  
  return (TME_OK);
}

/* this returns the new connections possible: */
int
tme_scsi_disk_connections_new(struct tme_element *element,
			      const char * const *args,
			      struct tme_connection **_conns,
			      char **_output)
{
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_disk_connection *conn_scsi_disk;
  struct tme_disk_connection *conn_disk;
  struct tme_connection *conn;
  tme_uint32_t block_size;
  int lun;
  int arg_i;
  int usage;
  int rc;

  /* recover our device: */
  scsi_disk = (struct tme_scsi_disk *) element->tme_element_private;

  /* check our arguments: */
  lun = -1;
  block_size = 0;
  arg_i = 1;
  usage = FALSE;

  /* loop reading our arguments: */
  for (;;) {

    /* the LUN to attach to: */
    if (TME_ARG_IS(args[arg_i + 0], "lun")
	&& lun < 0
	&& (lun = tme_scsi_lun_parse(args[arg_i + 1])) >= 0
	&& lun < TME_SCSI_DEVICE_LUN_COUNT
	&& scsi_disk->tme_scsi_disk_connections[lun] == NULL) {
      arg_i += 2;
    }

    /* the block size: */
    else if (TME_ARG_IS(args[arg_i + 0], "block-size")
	     && block_size == 0
	     && (block_size = 
		 tme_disk_dimension_parse(args[arg_i + 1])) > 0) {
      arg_i += 2;
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {
      break;
    }

    /* this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s, ",
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s [ lun %s ] [ block-size %s ]",
			    _("usage:"),
			    args[0],
			    _("LOGICAL-UNIT"),
			    _("BLOCK-SIZE"));
    return (EINVAL);
  }

  /* return any SCSI device SCSI connection: */
  rc = tme_scsi_device_connections_new(element,
				       args,
				       _conns,
				       _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* if we don't have a particular lun, see if there is a free lun.
     if there isn't a free lun, return now: */
  if (lun < 0) {
    for (lun = 0;
	 lun < TME_SCSI_DEVICE_LUN_COUNT;
	 lun++) {
      if (scsi_disk->tme_scsi_disk_connections[lun] == NULL) {
	break;
      }
    }
    if (lun == TME_SCSI_DEVICE_LUN_COUNT) {
      return (TME_OK);
    }
  }

  /* if we don't have a particular block size, assume 512: */
  if (block_size == 0) {
    block_size = 512;
  }

  /* create our side of a disk connection: */
  conn_scsi_disk = tme_new0(struct tme_scsi_disk_connection, 1);
  conn_disk = &conn_scsi_disk->tme_scsi_disk_connection;
  conn = &conn_disk->tme_disk_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_DISK;
  conn->tme_connection_score = tme_disk_connection_score;
  conn->tme_connection_make = tme_scsi_disk_connection_make;
  conn->tme_connection_break = tme_scsi_disk_connection_break;

  /* fill in the disk connection: */
  conn_disk->tme_disk_connection_control = tme_scsi_disk_control;

  /* fill in the internal disk connection: */
  conn_scsi_disk->tme_scsi_disk_connection_lun = lun;
  (void) tme_value64_set(&conn_scsi_disk->tme_scsi_disk_connection_block_size,
			 block_size);

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new SCSI disk function: */
TME_ELEMENT_SUB_NEW_DECL(tme_scsi,disk) {
  int id;
  const char *disk_type;
  const char *vendor;
  const char *product;
  const char *revision;
  struct tme_scsi_disk *scsi_disk;
  struct tme_scsi_device *scsi_device;
  int arg_i;
  int usage;
  unsigned int disk_list_i;
  int (*disk_init) _TME_P((struct tme_scsi_disk *));
  int rc;

  /* check our arguments: */
  id = -1;
  disk_type = NULL;
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

    /* the disk type: */
    else if (TME_ARG_IS(args[arg_i], "type")
	     && disk_type == NULL
	     && args[arg_i + 1] != NULL) {
      disk_type = args[arg_i + 1];
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
	  || disk_type == NULL) {
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

  /* make sure that this disk type is known: */
  disk_init = NULL;
  for (disk_list_i = 0;
       disk_list_i < TME_ARRAY_ELS(_tme_scsi_disk_list);
       disk_list_i++) {
    if (!strcmp(_tme_scsi_disk_list[disk_list_i]._tme_scsi_disk_list_type,
		disk_type)) {
      disk_init = _tme_scsi_disk_list[disk_list_i]._tme_scsi_disk_list_init;
      break;
    }
  }
  if (disk_init == NULL) {
    tme_output_append_error(_output, "%s", disk_type);
    return (ENOENT);
  }

  /* start the disk structure: */
  scsi_disk = tme_new0(struct tme_scsi_disk, 1);
  scsi_disk->tme_scsi_disk_element = element;
  scsi_disk->tme_scsi_disk_type = tme_strdup(disk_type);

  /* initialize the generic SCSI device structure: */
  scsi_device = &scsi_disk->tme_scsi_disk_device;
  rc = tme_scsi_device_new(scsi_device, id);
  assert (rc == TME_OK);

  scsi_device->tme_scsi_device_vendor
    = tme_strdup((vendor == NULL)
		 ? "TME"
		 : vendor);
  scsi_device->tme_scsi_device_product
    = tme_strdup((product == NULL)
		 ? "DISK"
		 : product);
  scsi_device->tme_scsi_device_revision
    = tme_strdup((revision == NULL)
		 ? "0000"
		 : revision);
  
  /* set the commands for direct-access devices: */
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_INQUIRY,
			 tme_scsi_disk_cdb_inquiry);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_FORMAT_UNIT,
			 tme_scsi_disk_cdb_format_unit);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_READ0,
			 tme_scsi_disk_cdb_read0);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_WRITE0,
			 tme_scsi_disk_cdb_write0);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_MODE_SELECT,
			 tme_scsi_disk_cdb_mode_select);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_MODE_SENSE,
			 tme_scsi_disk_cdb_mode_sense);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_START_STOP,
			 tme_scsi_disk_cdb_start_stop);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_PREVENT_ALLOW,
			 tme_scsi_disk_cdb_prevent_allow);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_READ_CAPACITY,
			 tme_scsi_disk_cdb_read_capacity);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_READ1,
			 tme_scsi_disk_cdb_read1);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_DISK_WRITE1,
			 tme_scsi_disk_cdb_write1);

  /* call the type-specific initialization function: */
  rc = (*disk_init)(scsi_disk);
  assert (rc == TME_OK);

  /* fill the element: */
  element->tme_element_private = scsi_disk;
  element->tme_element_connections_new = tme_scsi_disk_connections_new;

  return (TME_OK);
}
