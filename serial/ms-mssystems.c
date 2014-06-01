/* $Id: ms-mssystems.c,v 1.3 2010/02/14 00:42:25 fredette Exp $ */

/* serial/ms-mssystems.c - MouseSystems serial mouse emulation: */

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
_TME_RCSID("$Id: ms-mssystems.c,v 1.3 2010/02/14 00:42:25 fredette Exp $");

/* includes: */
#include "serial-ms.h"

/* macros: */

/* the buttons: */
#define TME_MS_MSSYSTEMS5_BUTTON_2	TME_BIT(0)
#define TME_MS_MSSYSTEMS5_BUTTON_1	TME_BIT(1)
#define TME_MS_MSSYSTEMS5_BUTTON_0	TME_BIT(2)

/* types: */

/* this subroutine sets the axis delta field in a packet, given an
   original axis delta and the offset of the first byte in the packet
   for the field.

   this is necessary because the first byte of a packet is the only
   one allowed to have a value in the range 0x80..0x8f: */
static inline void
_tme_ms_mssystems5_delta(int delta, tme_uint8_t *d0)
{

  if (delta > 0x7f) {
    d0[0] = 0x7f;
    delta -= 0x7f;
    d0[2] = TME_MIN(delta, 0x7f);
  }

  else if (delta <= (char) 0x8f) {
    d0[0] = 0x90;
    delta -= (char) 0x90;
    d0[2] = TME_MAX(delta, (char) 0x90);
  }

  else {
    d0[0] = delta;
    d0[2] = 0;
  }
}

/* this is called to make serial data from mouse events: */
static int
_tme_ms_mssystems5_events(struct tme_serial_ms *serial_ms,
			  const struct tme_mouse_event *event,
			  unsigned int count)
{
  tme_uint8_t packet[5], byte0;
  unsigned int buttons;

  for (; count-- > 0; event++) {

    /* start the first byte of the packet.  this byte has
       button and packet length information: */
    byte0 = 0x80;

    /* add in the buttons.  a set bit means a button is released: */
    byte0 |= (TME_MS_MSSYSTEMS5_BUTTON_0
	      | TME_MS_MSSYSTEMS5_BUTTON_1
	      | TME_MS_MSSYSTEMS5_BUTTON_2);
    buttons = event->tme_mouse_event_buttons;
    if (buttons & TME_MOUSE_BUTTON_0) {
      byte0 ^= TME_MS_MSSYSTEMS5_BUTTON_0;
    }
    if (buttons & TME_MOUSE_BUTTON_1) {
      byte0 ^= TME_MS_MSSYSTEMS5_BUTTON_1;
    }
    if (buttons & TME_MOUSE_BUTTON_2) {
      byte0 ^= TME_MS_MSSYSTEMS5_BUTTON_2;
    }

    /* put in the deltas.  in the MouseSystems protocol, a negative
       y delta corresponds to "down" mouse movement: */
    _tme_ms_mssystems5_delta(event->tme_mouse_event_delta_x,
			     &packet[1]);
    _tme_ms_mssystems5_delta(-event->tme_mouse_event_delta_y,
			     &packet[2]);

    /* put in the first byte: */
    packet[0] = byte0;

    /* copy the packet into the serial buffer: */
    tme_serial_buffer_copyin(&serial_ms->tme_serial_ms_serial_buffer,
			     packet,
			     sizeof(packet),
			     TME_SERIAL_DATA_NORMAL,
			     TME_SERIAL_COPY_FULL_IS_OVERRUN);
  }

  return (TME_OK);
}

/* this initializes MouseSystems serial mouse emulation: */
int
_tme_serial_ms_mssystems5_init(struct tme_serial_ms *serial_ms)
{
  struct tme_serial_config *config;

  /* set our type-dependent functions: */
  serial_ms->tme_serial_ms_type_events = _tme_ms_mssystems5_events;
  serial_ms->tme_serial_ms_type_serial_ctrl = NULL;
  serial_ms->tme_serial_ms_type_serial_input = NULL;

  /* set our serial configuration: */
  config = &serial_ms->tme_serial_ms_type_config;
  memset(config, 0, sizeof(*config));
  config->tme_serial_config_baud = 1200;
  config->tme_serial_config_bits_data = 8;
  config->tme_serial_config_bits_stop = 1;
  config->tme_serial_config_parity = TME_SERIAL_PARITY_NONE;

  /* set our rate-limiting: */
  serial_ms->tme_serial_ms_rate_usec
    = TME_MAX(serial_ms->tme_serial_ms_rate_usec,
	      (1000000
	       / (config->tme_serial_config_baud
		  / 5 /* size of one packet */)));

  return (TME_OK);
}
