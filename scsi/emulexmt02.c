/* $Id: emulexmt02.c,v 1.4 2007/08/25 22:54:59 fredette Exp $ */

/* scsi/emulexmt02.c - Emulex MT-02 SCSI tape emulation: */

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
_TME_RCSID("$Id: emulexmt02.c,v 1.4 2007/08/25 22:54:59 fredette Exp $");

/* includes: */
#include <tme/scsi/scsi-tape.h>

/* macros: */

/* the Emulex MT-02 is a SCSI<->QIC translator, and being QIC,
   it only supports a 512 byte block size: */
#define TME_EMULEXMT02_BLOCK_SIZE	(512)

/* types: */

/* the Emulex MT-02 returns extended sense plus some vendor-specific
   additional bytes: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_request_sense)
{
  int lun;
  struct tme_scsi_device_sense *sense;
  tme_uint8_t transfer_length;
  tme_uint8_t error;
  
  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;
  
  /* get the sense: */
  sense = &scsi_device->tme_scsi_device_sense[lun];
  
  /* see how much space for sense bytes the initiator
     has allocated.  zero means four: */
  transfer_length
    = scsi_device->tme_scsi_device_cdb[4];
  if (transfer_length == 0) {
    transfer_length = 4;
  }

  /* the sun2 PROM insists that the fifth byte of the sense
     have its least significant bit set: */
  sense->tme_scsi_device_sense_data[4] |= 0x01;

  /* the Emulex MT-02 returns eight additional sense bytes: */
  sense->tme_scsi_device_sense_data[7]
    = 0x08;
  
  /* the Emulex error code.  get this by dispatching on the
     generic sense key: */
  switch (sense->tme_scsi_device_sense_data[2] & 0xf) {
  case 0x0: /* NO SENSE */
    /* EOM: */
    if (sense->tme_scsi_device_sense_data[2] & 0x40) {
      error = 0x34; /* END OF MEDIA */
    }
    /* ILI: */
    else if (sense->tme_scsi_device_sense_data[2] & 0x20) {
      error = 0x19; /* BAD BLOCK */
    }
    else if (sense->tme_scsi_device_sense_data[2] & 0x80) {
      error = 0x1c; /* FILE MARK */
    }
    else {
      error = 0x00; /* NO SENSE */
    }
    break; 
  case 0x1: error = 0x18; break; /* RECOVERED ERROR */
  case 0x2: error = 0x04; break; /* NOT READY */
  case 0x3: error = 0x11; break; /* MEDIA ERROR */
  case 0x4: error = 0x0b; break; /* HARDWARE ERROR */
  case 0x5: error = 0x20; break; /* INVALID COMMAND */
  case 0x6: error = 0x30; break; /* UNIT ATTENTION */
  case 0x7: error = 0x17; break; /* DATA PROTECT */
  case 0x8: error = 0x19; break; /* BAD BLOCK */
  case 0xd: error = 0x14; break; /* BLOCK NOT FOUND */
  default: abort();
  }
  sense->tme_scsi_device_sense_data[8] = error;
  
  /* the Emulex retry count: */
  sense->tme_scsi_device_sense_data[9] = 0x00;
  sense->tme_scsi_device_sense_data[10] = 0x10;

  /* call the normal REQUEST SENSE handler: */
  tme_scsi_device_cdb_request_sense(scsi_device,
				    control_old,
				    control_new);
}

/* the Emulex MT-02 returns an all-bits-zero INQUIRY response: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_inquiry)
{
  int lun;
  
  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  /* return the INQUIRY data: */
  memset(scsi_device->tme_scsi_device_data,
	 0, 
	 sizeof(scsi_device->tme_scsi_device_data));

  /* The SunOS/sun3 4.1.1 tape bootblock will refuse to boot off of a
     SCSI device unless the INQUIRY response indicates a tape (so
     apparently it's impossible to install SunOS 4.1.1 on a real Sun3
     from an Emulex tape).

     Because we're lazy and don't want to implement tme-scsi-1 tape
     yet, we just indicate that we're a tape.

     However, this has some ramifications for NetBSD/sun2 1.6*, which
     was rigged to specially match an all-bits-zero INQUIRY for an
     Emulex tape.  To work around this, when running NetBSD/sun2 or
     NetBSD/sun3 in the emulator, specify vendor EMULEX product "MT-02
     QIC" in the tmesh configuration file, which will cause a regular,
     full INQUIRY response to be sent, but with the specific vendor
     and product so the right quirk entry gets matched: */
  scsi_device->tme_scsi_device_data[0] = TME_SCSI_TYPE_TAPE;

  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = 5;
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = scsi_device->tme_scsi_device_data;
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = NULL;

  /* return the data and the GOOD status: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* the Emulex MT-02 behaves as if the "Fixed" bit is always set in
   its READ and WRITE CDBs: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_read0)
{
  scsi_device->tme_scsi_device_cdb[1] |= 0x01;
  tme_scsi_tape_cdb_xfer0(scsi_device, TRUE);
}
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_write0)
{
  scsi_device->tme_scsi_device_cdb[1] |= 0x01;
  tme_scsi_tape_cdb_xfer0(scsi_device, FALSE);
}

/* the Emulex MT-02 MODE SENSE command: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_mode_sense)
{
  tme_uint8_t *data;
  tme_uint32_t blocks, block_size;
  int lun;
  
  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  data = &scsi_device->tme_scsi_device_data[0];

  /* the sense data length.  we will fill this in later: */
  data++;

  /* byte 1 is the medium type: */
  *(data++) = 0x00; /* default (only one medium type supported) */

  /* byte 2 is the WP (Write Protect), Buffered Mode, and Speed: */
  *(data++) = 0x80; /* write protected, unbuffered, default speed */

  /* byte 3 is the Block Descriptor Length.  we will fill this in
     later: */
  data++;

  /* the first Block Descriptor: */
  
  /* the Block Descriptor density code: */
  *(data++) = 0x05; /* QIC-24 */

  /* the Number of Blocks.  we assume a 60MB tape: */
  block_size = TME_EMULEXMT02_BLOCK_SIZE;
  blocks = (60 * 1024 * 1024) / block_size;
  *(data++) = (blocks >> 16) & 0xff;
  *(data++) = (blocks >>  8) & 0xff;
  *(data++) = (blocks >>  0) & 0xff;

  /* a reserved byte: */
  data++;

  /* the Block Length: */
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

/* the Emulex MT-02 MODE SELECT command: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_mode_select)
{
  
  /* read in the parameter list: */
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = scsi_device->tme_scsi_device_cdb[4];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid
    = TME_MIN(scsi_device->tme_scsi_device_dma.tme_scsi_dma_resid,
	      sizeof(scsi_device->tme_scsi_device_data));
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_in
    = &scsi_device->tme_scsi_device_data[0];
  scsi_device->tme_scsi_device_dma.tme_scsi_dma_out
    = NULL;

  /* XXX for now we discard the parameter list after reading it in: */
  tme_scsi_device_target_do_dsmf(scsi_device,
				 TME_SCSI_STATUS_GOOD,
				 TME_SCSI_MSG_CMD_COMPLETE);
}

/* this vendor-unique command (0xd) puts us into qic02 mode: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_qic02)
{
  tme_scsi_device_target_do_smf(scsi_device,
				TME_SCSI_STATUS_GOOD,
				TME_SCSI_MSG_CMD_COMPLETE);
}

/* this implements the tape READ BLOCK LIMITS command: */
static _TME_SCSI_DEVICE_CDB_DECL(_tme_emulexmt02_cdb_block_limits)
{
  tme_uint8_t *data;
  int lun;
  
  /* get the addressed LUN: */
  lun = scsi_device->tme_scsi_device_addressed_lun;

  data = &scsi_device->tme_scsi_device_data[0];

  /* a reserved byte: */
  data++;

  /* the Maximum Block Length: */
  *(data++) = (TME_EMULEXMT02_BLOCK_SIZE >> 16) & 0xff;
  *(data++) = (TME_EMULEXMT02_BLOCK_SIZE >>  8) & 0xff;
  *(data++) = (TME_EMULEXMT02_BLOCK_SIZE >>  0) & 0xff;

  /* the Minimum Block Length: */
  *(data++) = (TME_EMULEXMT02_BLOCK_SIZE >>  8) & 0xff;
  *(data++) = (TME_EMULEXMT02_BLOCK_SIZE >>  0) & 0xff;

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

/* this enforces the block size at tape connection time: */
static void
_tme_emulexmt02_connected(struct tme_scsi_tape *scsi_tape,
			  int lun)
{
  struct tme_scsi_tape_connection *conn_scsi_tape;
  struct tme_tape_connection *conn_tape;
  unsigned long sizes[3];
  int rc;
  
  /* get the tape connection: */
  conn_scsi_tape
    = scsi_tape->tme_scsi_tape_connections[lun];
  conn_tape
    = ((struct tme_tape_connection *)
       conn_scsi_tape->tme_scsi_tape_connection.tme_tape_connection.tme_connection_other);

  /* set the block size: */
  sizes[0] = TME_EMULEXMT02_BLOCK_SIZE;
  sizes[1] = TME_EMULEXMT02_BLOCK_SIZE;
  sizes[2] = TME_EMULEXMT02_BLOCK_SIZE;
  rc
    = ((*conn_tape->tme_tape_connection_control)
       (conn_tape,
	TME_TAPE_CONTROL_BLOCK_SIZE_SET,
	sizes));
  assert (rc == TME_OK);
}

/* this initializes Emulex MT-02 SCSI tape emulation: */
int
tme_scsi_tape_emulexmt02_init(struct tme_scsi_tape *scsi_tape)
{
  struct tme_scsi_device *scsi_device;
  
  scsi_device = &scsi_tape->tme_scsi_tape_device;

  /* Emulex MT-02 boards don't really support the INQUIRY command: */
  /* XXX FIXME - this is a hack.  if the user has specified EMULEX
     for the vendor name, don't override the INQUIRY handling: */
  if (strcmp(scsi_device->tme_scsi_device_vendor, "EMULEX")) {
    TME_SCSI_DEVICE_DO_CDB(scsi_device,
			   TME_SCSI_CDB_INQUIRY,
			   _tme_emulexmt02_cdb_inquiry);
  }

  /* our type-specific connection function: */
  scsi_tape->tme_scsi_tape_connected
    = _tme_emulexmt02_connected;

  /* our block sizes: */
  scsi_tape->tme_scsi_tape_block_size_min = TME_EMULEXMT02_BLOCK_SIZE;
  scsi_tape->tme_scsi_tape_block_size_max = TME_EMULEXMT02_BLOCK_SIZE;
  scsi_tape->tme_scsi_tape_block_size_current = TME_EMULEXMT02_BLOCK_SIZE;

  /* our type-specific CDB functions: */
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 0x0d,
			 _tme_emulexmt02_cdb_qic02);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_READ0,
			 _tme_emulexmt02_cdb_read0);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_WRITE0,
			 _tme_emulexmt02_cdb_write0);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_MODE_SENSE,
			 _tme_emulexmt02_cdb_mode_sense);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_MODE_SELECT,
			 _tme_emulexmt02_cdb_mode_select);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_BLOCK_LIMITS,
			 _tme_emulexmt02_cdb_block_limits);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_REQUEST_SENSE,
			 _tme_emulexmt02_cdb_request_sense);
  TME_SCSI_DEVICE_DO_CDB(scsi_device,
			 TME_SCSI_CDB_TAPE_RESERVE,
			 tme_scsi_device_cdb_illegal);

  return (TME_OK);
}
