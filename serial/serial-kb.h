/* $Id: serial-kb.h,v 1.3 2006/09/30 12:35:01 fredette Exp $ */

/* serial/serial-kb.h - implementation header file for serial keyboard
   emulation: */

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

#ifndef _SERIAL_SERIAL_KB_H
#define _SERIAL_SERIAL_KB_H

#include <tme/common.h>
_TME_RCSID("$Id: serial-kb.h,v 1.3 2006/09/30 12:35:01 fredette Exp $");

/* includes: */
#include <tme/generic/keyboard.h>
#include <tme/generic/serial.h>
#include <tme/element.h>
#include <tme/threads.h>

/* macros: */

/* the callout flags: */
#define TME_SERIAL_KB_CALLOUT_CHECK		(0)
#define TME_SERIAL_KB_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SERIAL_KB_CALLOUTS_MASK		(-2)
#define  TME_SERIAL_KB_CALLOUT_SERIAL_CTRL	TME_BIT(1)
#define  TME_SERIAL_KB_CALLOUT_SERIAL_CONFIG	TME_BIT(2)
#define  TME_SERIAL_KB_CALLOUT_SERIAL_READ	TME_BIT(3)
#define  TME_SERIAL_KB_CALLOUT_KEYBOARD_CTRL	TME_BIT(4)
#define  TME_SERIAL_KB_CALLOUT_KEYBOARD_READ	TME_BIT(5)

/* types: */

/* a serial keyboard: */
struct tme_serial_kb {

  /* backpointer to our element: */
  struct tme_element *tme_serial_kb_element;

  /* our mutex: */
  tme_mutex_t tme_serial_kb_mutex;

  /* our type: */
  _tme_const char *tme_serial_kb_type;

  /* our type-specific state: */
  void *tme_serial_kb_type_state;

  /* our type-specific pre-map-adding function: */
  int (*tme_serial_kb_type_map_add_pre) _TME_P((struct tme_serial_kb *,
						struct tme_keyboard_map *));

  /* our type-specific post-map-adding function: */
  int (*tme_serial_kb_type_map_add_post) _TME_P((struct tme_serial_kb *,
						 _tme_const struct tme_keyboard_map *));

  /* our type-specific event function: */
  tme_uint8_t (*tme_serial_kb_type_event) _TME_P((struct tme_serial_kb *,
						  _tme_const struct tme_keyboard_event *));

  /* our type-specific serial control function: */
  int (*tme_serial_kb_type_serial_ctrl) _TME_P((struct tme_serial_kb *,
						unsigned int));

  /* our type-specific serial input function: */
  int (*tme_serial_kb_type_serial_input) _TME_P((struct tme_serial_kb *,
						 tme_uint8_t *,
						 unsigned int,
						 tme_serial_data_flags_t));

  /* our type-specific serial configuration: */
  struct tme_serial_config tme_serial_kb_type_config;

  /* our stored macros: */
  char **tme_serial_kb_macros;

  /* our stored map entries: */
  char **tme_serial_kb_map;

  /* our keyboard connection: */
  struct tme_keyboard_connection *tme_serial_kb_connection_kb;

  /* our serial connection: */
  struct tme_serial_connection *tme_serial_kb_connection_serial;

  /* the callout flags: */
  int tme_serial_kb_callout_flags;

  /* our keyboard buffer: */
  struct tme_keyboard_buffer *tme_serial_kb_keyboard_buffer;

  /* our serial output buffer: */
  struct tme_serial_buffer tme_serial_kb_serial_buffer;

  /* our current keyboard control outputs: */
  unsigned int tme_serial_kb_keyboard_ctrl;

  /* our current serial control outputs: */
  unsigned int tme_serial_kb_serial_ctrl;

  /* when nonzero, the rate-limiting sleep time, in microseconds: */
  unsigned long tme_serial_kb_rate_sleep;

  /* when nonzero, rate-limiting is active: */
  int tme_serial_kb_rate_limited;

  /* when nonzero, rate-limiting is active and sleeping: */
  int tme_serial_kb_rate_sleeping;

  /* our rate-limiting thread condition: */
  tme_cond_t tme_serial_kb_rate_cond;
};

/* prototypes: */
int _tme_serial_kb_sun_init _TME_P((struct tme_serial_kb *));

#endif /* !_SERIAL_SERIAL_KB_H */
