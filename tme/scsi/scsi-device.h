/* $Id: scsi-device.h,v 1.8 2007/01/19 00:41:43 fredette Exp $ */

/* tme/scsi/scsi-device.h - header file for generic SCSI device support: */

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

#ifndef _TME_SCSI_SCSI_DEVICE_H
#define _TME_SCSI_SCSI_DEVICE_H

#include <tme/common.h>
_TME_RCSID("$Id: scsi-device.h,v 1.8 2007/01/19 00:41:43 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/generic/scsi.h>

/* macros: */

/* the maximum number of logical units: */
#define TME_SCSI_DEVICE_LUN_COUNT		(8)

/* this makes a prototype for a message handler: */
#define _TME_SCSI_DEVICE_MSG_P(sym)		\
void sym _TME_P((struct tme_scsi_device *,	\
		  tme_scsi_control_t,		\
		  tme_scsi_control_t))

/* this declares a message handler: */
#ifdef __STDC__
#define _TME_SCSI_DEVICE_MSG_DECL(sym)		\
void sym(struct tme_scsi_device *scsi_device, tme_scsi_control_t control_old, tme_scsi_control_t control_new)
#else  /* !__STDC__ */
#define _TME_ELEMENT_NEW_DECL(sym)		\
void sym(scsi_device, control_old, control_new)	\
  struct tme_scsi_device *scsi_device;		\
  tme_scsi_control_t control_old;		\
  tme_scsi_control_t control_new;
#endif /* !__STDC__ */

/* this sets a message handler: */
#define TME_SCSI_DEVICE_DO_MSG(dev, code, sym)	\
do {						\
  (dev)->tme_scsi_device_do_msg[code] = (sym);	\
} while (/* CONSTCOND */ 0)

/* this sets an extended message handler: */
#define TME_SCSI_DEVICE_DO_MSG_EXT(dev, code, sym)\
do {						\
  (dev)->tme_scsi_device_do_msg_ext[code] = (sym);\
} while (/* CONSTCOND */ 0)

/* this makes a prototype for a CDB handler: */
#define _TME_SCSI_DEVICE_CDB_P(sym)		\
_TME_SCSI_DEVICE_MSG_P(sym)

/* this declares a message handler: */
#define _TME_SCSI_DEVICE_CDB_DECL(sym)		\
_TME_SCSI_DEVICE_MSG_DECL(sym)

/* this sets a CDB handler: */
#define TME_SCSI_DEVICE_DO_CDB(dev, code, sym)	\
do {						\
  (dev)->tme_scsi_device_do_cdb[code] = (sym);	\
} while (/* CONSTCOND */ 0)

/* this makes a prototype for a phase handler: */
#define _TME_SCSI_DEVICE_PHASE_P(sym)		\
_TME_SCSI_DEVICE_MSG_P(sym)

/* this declares a phase handler: */
#define _TME_SCSI_DEVICE_PHASE_DECL(sym)	\
_TME_SCSI_DEVICE_MSG_DECL(sym)

/* when a device is the target, this completes a command by
   running the STATUS phase, followed by the MESSAGE IN
   phase, followed by a BUS FREE phase: */
#define tme_scsi_device_target_do_smf(d, s, m)	\
do {						\
  (d)->tme_scsi_device_status = (s);		\
  (d)->tme_scsi_device_msg[0] = (m);		\
  tme_scsi_device_target_smf(d, 0, 0);		\
} while (/* CONSTCOND */ 0)

/* when a device is the target, this completes a command by running
   the DATA IN or DATA OUT phase, followed by STATUS phase, followed
   by the MESSAGE IN phase, followed by a BUS FREE phase: */
#define tme_scsi_device_target_do_dsmf(d, s, m)	\
do {						\
  (d)->tme_scsi_device_status = (s);		\
  (d)->tme_scsi_device_msg[0] = (m);		\
  tme_scsi_device_target_dsmf(d, 0, 0);		\
} while (/* CONSTCOND */ 0)

/* types: */

/* a SCSI device: */
struct tme_scsi_device {
  
  /* the mutex protecting the device: */
  tme_mutex_t tme_scsi_device_mutex;

  /* backpointer to the device's element: */
  struct tme_element *tme_scsi_device_element;

  /* this device's SCSI connection: */
  struct tme_scsi_connection *tme_scsi_device_connection;

  /* the callout flags: */
  int tme_scsi_device_callout_flags;

  /* the SCSI ID for this device: */
  tme_scsi_data_t tme_scsi_device_id;

  /* a mask of SCSI LUNs defined on this device: */
  tme_uint32_t tme_scsi_device_luns;

  /* the device vendor, product, and revision strings: */
  char *tme_scsi_device_vendor;
  char *tme_scsi_device_product;
  char *tme_scsi_device_revision;

  /* the SCSI control signals currently asserted: */
  tme_scsi_control_t tme_scsi_device_control;

  /* the current SCSI DMA structure: */
  struct tme_scsi_dma tme_scsi_device_dma;

  /* the current addressed LUN, or -1 if no LUN is selected: */
  int tme_scsi_device_addressed_lun;

  /* the SCSI message buffer.  this buffer is big enough for
     the largest possible SCSI extended message: */
  tme_uint8_t tme_scsi_device_msg[258];

  /* the SCSI CDB.  this buffer is big enough for a Group 4 (16-byte)
     CDB: */
  tme_uint8_t tme_scsi_device_cdb[16];

  /* the SCSI data buffer.  this buffer is big enough for a large
     MODE SELECT parameter list: */
  tme_uint8_t tme_scsi_device_data[256];

  /* the SCSI status byte: */
  tme_uint8_t tme_scsi_device_status;

  /* the SCSI LUN handler: */
  int (*tme_scsi_device_address_lun) _TME_P((struct tme_scsi_device *));

  /* the SCSI sense buffers.  these buffers are large enough for a
     large extended sense: */
  /* see section 6.1.3 for details about preserving/clearing sense: */
  struct tme_scsi_device_sense {
    tme_uint8_t tme_scsi_device_sense_data[128];
    unsigned int tme_scsi_device_sense_valid;
  } tme_scsi_device_sense[TME_SCSI_DEVICE_LUN_COUNT];
  int tme_scsi_device_sense_no_extended;

  /* the SCSI message handlers.  not counting extended messages, there
     are 129 possible messages (counting all 128 possible IDENTIFY
     messages as one): */
  void (*tme_scsi_device_do_msg[129]) _TME_P((struct tme_scsi_device *,
					      tme_scsi_control_t,
					      tme_scsi_control_t));
  void (*tme_scsi_device_do_msg_ext[256]) _TME_P((struct tme_scsi_device *,
						  tme_scsi_control_t,
						  tme_scsi_control_t));

  /* the SCSI CDB handlers.  there are 256 possible commands: */
  void (*tme_scsi_device_do_cdb[256]) _TME_P((struct tme_scsi_device *,
					      tme_scsi_control_t,
					      tme_scsi_control_t));

  /* the generic SCSI phase handler: */
  void (*tme_scsi_device_phase) _TME_P((struct tme_scsi_device *,
					tme_scsi_control_t,
					tme_scsi_control_t));
};

/* prototypes: */
int tme_scsi_device_new _TME_P((struct tme_scsi_device *, int));
void tme_scsi_device_target_phase _TME_P((struct tme_scsi_device *, tme_scsi_control_t));
int tme_scsi_device_connection_make _TME_P((struct tme_connection *, unsigned int));
int tme_scsi_device_connection_break _TME_P((struct tme_connection *, unsigned int));
int tme_scsi_device_connections_new _TME_P((struct tme_element *, _tme_const char * _tme_const *, struct tme_connection **, char **));
_TME_SCSI_DEVICE_PHASE_P(tme_scsi_device_target_f);
_TME_SCSI_DEVICE_PHASE_P(tme_scsi_device_target_mf);
_TME_SCSI_DEVICE_PHASE_P(tme_scsi_device_target_smf);
_TME_SCSI_DEVICE_PHASE_P(tme_scsi_device_target_dsmf);
_TME_SCSI_DEVICE_PHASE_P(tme_scsi_device_target_mc);
void tme_scsi_device_check_condition _TME_P((struct tme_scsi_device *, tme_uint8_t, tme_uint16_t));
int tme_scsi_device_address_lun_aware _TME_P((struct tme_scsi_device *));
int tme_scsi_device_address_lun_unaware _TME_P((struct tme_scsi_device *));

#endif /* !_TME_SCSI_SCSI_DEVICE_H */
