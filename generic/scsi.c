/* $Id: scsi.c,v 1.3 2007/01/07 23:27:50 fredette Exp $ */

/* generic/scsi.c - generic SCSI implementation support: */

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
_TME_RCSID("$Id: scsi.c,v 1.3 2007/01/07 23:27:50 fredette Exp $");

/* includes: */
#include <tme/generic/scsi.h>
#include <tme/scsi/scsi-cdb.h>
#include <tme/scsi/scsi-msg.h>
#include <stdlib.h>

/* this scores a SCSI connection: */
int
tme_scsi_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_scsi_connection *conn_scsi;
  struct tme_scsi_connection *conn_scsi_other;

  /* both sides must be SCSI connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SCSI);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SCSI);

  /* you cannot connect a bus to a bus, or a device to a device: */
  conn_scsi
    = (struct tme_scsi_connection *) conn;
  conn_scsi_other 
    = (struct tme_scsi_connection *) conn->tme_connection_other;
  /* XXX we need a way to distinguish a bus from a device: */
  *_score
    = 1;
  return (TME_OK);
}

/* this parses a SCSI ID: */
int
tme_scsi_id_parse(const char *id_string)
{
  unsigned long id;
  char *p1;

  /* catch a NULL string: */
  if (id_string == NULL) {
    return (-1);
  }

  /* convert the string: */
  id = strtoul(id_string, &p1, 0);
  if (p1 == id_string
      || *p1 != '\0') {
    return (-1);
  }
  return (id);
}

/* this implements state machines that determine the residual in a
   SCSI command or message phase: */
tme_uint32_t
tme_scsi_phase_resid(tme_scsi_control_t scsi_control,
		     tme_uint32_t *_state,
		     const tme_shared tme_uint8_t *bytes,
		     unsigned long count)
{
#define _TME_SCSI_PHASE_RESID_STATE_COUNT_MASK		(4095)
  tme_uint32_t state;
  tme_uint32_t transferred;
  tme_uint32_t seen;
  tme_uint32_t skip;
  tme_uint32_t resid;
  tme_uint8_t byte;

  /* get the opaque state, which must not be the value zero (the
     opaque stop state): */
  state = *_state;
  assert (state != 0);

  /* decompose the opaque state.  NB since the opaque start state for
     all SCSI bus phases is the value one, and we use internal state
     zero to represent the stop state, we subtract one from the bytes
     transferred value and add one to the internal state number: */
  transferred = (state - 1) & _TME_SCSI_PHASE_RESID_STATE_COUNT_MASK;
  state /= (_TME_SCSI_PHASE_RESID_STATE_COUNT_MASK + 1);
  seen = state & _TME_SCSI_PHASE_RESID_STATE_COUNT_MASK;
  state = (state / (_TME_SCSI_PHASE_RESID_STATE_COUNT_MASK + 1)) + 1;

  /* we can't have transferred more than we've seen: */
  assert (transferred <= seen);

  /* start with a residual of the number of bytes that we have already
     seen, and skip past those bytes in the buffer: */
  resid = seen - transferred;
  skip = TME_MIN(resid, count);
  bytes += skip;
  count -= skip;

  /* loop over the bytes: */
  for (; count > 0; ) {

    /* get this next byte: */
    byte = *(bytes++);
    count--;
    seen++;

    /* dispatch on the SCSI bus phase: */
    switch (TME_SCSI_PHASE(scsi_control)) {

      /* an unknown bus phase: */
    default: assert(FALSE);

    case TME_SCSI_PHASE_COMMAND:

      /* we only have state one.  transfer the number of bytes in the
	 CDB and move to the stop state: */
      assert (state == 1);
      switch (byte & TME_SCSI_CDB_GROUP_MASK) {
      default: abort();
      case TME_SCSI_CDB_GROUP_0: resid += TME_SCSI_CDB_GROUP_0_LEN; break;
      case TME_SCSI_CDB_GROUP_1: resid += TME_SCSI_CDB_GROUP_1_LEN; break;
      case TME_SCSI_CDB_GROUP_2: resid += TME_SCSI_CDB_GROUP_2_LEN; break;
      case TME_SCSI_CDB_GROUP_4: resid += TME_SCSI_CDB_GROUP_4_LEN; break;
      case TME_SCSI_CDB_GROUP_5: resid += TME_SCSI_CDB_GROUP_5_LEN; break;
      }
      state = 0;
      break;

    case TME_SCSI_PHASE_MESSAGE_IN:
    case TME_SCSI_PHASE_MESSAGE_OUT:

      /* dispatch on the state: */
      switch (state) {
      default: assert(FALSE);

	/* state one is the first byte of the message: */
      case 1:

	/* if this is an extended message: */
	if (byte == TME_SCSI_MSG_EXTENDED) {

	  /* transfer this first message byte and move to state two: */
	  resid += 1;
	  state = 2;
	}

	/* otherwise, if this is a two-byte message: */
	else if (TME_SCSI_MSG_IS_2(byte)) {

	  /* transfer the two message bytes and move to state zero: */
	  resid += 2;
	  state = 0;
	}

	/* otherwise, this is a one-byte message: */
	else {

	  /* transfer the one message byte and move to state zero: */
	  resid += 1;
	  state = 0;
	}
	break;

	/* the second byte of an extended message: */
      case 2:

	/* transfer this second message byte, followed by the extended
	   message itself, and move to state zero: */
	resid += (byte == 0 ? 1 + 256 : 1 + byte);
	state = 0;
	break;
      }
      break;
    }

    /* if we've reached state zero, return the opaque stop state and
       the detected residual: */
    if (state == 0) {
      *_state = 0;
      return (resid);
    }
  }

  /* compose the updated opaque state.  NB since the opaque start
     state for all SCSI bus phases is the value one, and we use
     internal state zero to represent the stop state, we add one from
     the bytes transferred value and subtract one to the internal
     state number: */
  state = (state - 1) * (_TME_SCSI_PHASE_RESID_STATE_COUNT_MASK + 1);
  state += seen;
  state *= (_TME_SCSI_PHASE_RESID_STATE_COUNT_MASK + 1);
  state += (transferred + 1) & _TME_SCSI_PHASE_RESID_STATE_COUNT_MASK;
  *_state = state;
  return (0);
#undef _TME_SCSI_PHASE_RESID_STATE_COUNT_MASK
}
