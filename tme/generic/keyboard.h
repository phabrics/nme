/* $Id: keyboard.h,v 1.5 2007/02/15 01:27:37 fredette Exp $ */

/* tme/generic/keyboard.h - header file for generic keyboard support: */

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

#ifndef _TME_GENERIC_KEYBOARD_H
#define _TME_GENERIC_KEYBOARD_H

#include <tme/common.h>
_TME_RCSID("$Id: keyboard.h,v 1.5 2007/02/15 01:27:37 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* the undefined keyval: */
#define TME_KEYBOARD_KEYVAL_UNDEF	(0xffffffff)

/* modifiers: */
#define TME_KEYBOARD_MODIFIER_NONE		(-1)
#define TME_KEYBOARD_MODIFIER_SHIFT		(0)
#define TME_KEYBOARD_MODIFIER_LOCK		(1)
#define TME_KEYBOARD_MODIFIER_CONTROL		(2)
#define TME_KEYBOARD_MODIFIER_MOD1		(3)
#define TME_KEYBOARD_MODIFIER_MOD2		(4)
#define TME_KEYBOARD_MODIFIER_MOD3		(5)
#define TME_KEYBOARD_MODIFIER_MOD4		(6)
#define TME_KEYBOARD_MODIFIER_MOD5		(7)
#define TME_KEYBOARD_MODIFIER_MAX		TME_KEYBOARD_MODIFIER_MOD5

/* keysym notes: */
#define TME_KEYBOARD_KEYSYM_NOTE_UNDEF		(0)
#define TME_KEYBOARD_KEYSYM_NOTE_CAPS_LOCK	(1)
#define TME_KEYBOARD_KEYSYM_NOTE_SHIFT_LOCK	(2)
#define TME_KEYBOARD_KEYSYM_NOTE_NUM_LOCK	(3)

/* lookup flags: */
#define TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT	TME_BIT(0)
#define TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC	TME_BIT(1)
#define TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC_NOW	TME_BIT(2)

/* event types: */
#define TME_KEYBOARD_EVENT_PRESS		(1)
#define TME_KEYBOARD_EVENT_RELEASE		(0)

/* keyboard controls: */
#define TME_KEYBOARD_CTRL_OK_READ		TME_BIT(0)
#define TME_KEYBOARD_CTRL_ALL_UP		TME_BIT(1)
#define TME_KEYBOARD_CTRL_BELL			TME_BIT(2)
#define TME_KEYBOARD_CTRL_LED0			TME_BIT(3)
#define TME_KEYBOARD_CTRL_LED1			TME_BIT(4)
#define TME_KEYBOARD_CTRL_LED2			TME_BIT(5)
#define TME_KEYBOARD_CTRL_LED3			TME_BIT(6)

/* keyboard modes: */
#define TME_KEYBOARD_MODE_GLOBAL		(0)
#define TME_KEYBOARD_MODE_UNLOCK		TME_BIT(0)
#define TME_KEYBOARD_MODE_LOCK			TME_BIT(1)
#define TME_KEYBOARD_MODE_PASSTHROUGH		TME_BIT(2)
#define  TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS	TME_BIT(3)
#define  TME_KEYBOARD_MODE_FLAG_NO_RELEASES	TME_BIT(4)
#define  TME_KEYBOARD_MODE_FLAG_LOCK_SOFT	TME_BIT(5)

/* the undefined event time: */
#define TME_KEYBOARD_EVENT_TIME_UNDEF		(0)

/* this evaluates to nonzero iff a buffer is empty: */
#define tme_keyboard_buffer_is_empty(b)		\
  ((b)->tme_keyboard_buffer_head		\
   == ((b)->tme_keyboard_buffer_tail))

/* this evaluates to nonzero iff a buffer is full: */
#define tme_keyboard_buffer_is_full(b)		\
  ((((b)->tme_keyboard_buffer_head + 1)		\
    & ((b)->tme_keyboard_buffer_size - 1))	\
   == (b)->tme_keyboard_buffer_tail)

/* this convert between a tme_hash_data_t and a keyval: */
#define tme_keyboard_hash_data_from_keyval(x)	tme_hash_data_from_uint32(x)
#define tme_keyboard_hash_data_to_keyval(x)	tme_hash_data_to_uint32(x)

/* types: */

/* a modifiers mask: */
typedef tme_uint8_t tme_keyboard_modifiers_t;

/* a keyval - a keysym or a keycode: */
typedef tme_uint32_t tme_keyboard_keyval_t;

/* keyboard lookup information: */
struct tme_keyboard_lookup {

  /* the string name being looked up: */
  _tme_const char *tme_keyboard_lookup_string;

  /* flags for the lookup: */
  unsigned int tme_keyboard_lookup_flags;

  /* any context that can be used to associate lookups: */
  unsigned int tme_keyboard_lookup_context_length;
  _tme_const tme_uint8_t *tme_keyboard_lookup_context;
};

/* a keyboard keysym lookup function: */
typedef tme_keyboard_keyval_t (*tme_keyboard_keysym_lookup_t)
     _TME_P((void *, _tme_const struct tme_keyboard_lookup *));

/* one entry of a keyboard map: */
struct tme_keyboard_map {

  /* the keysym: */
  tme_keyboard_keyval_t tme_keyboard_map_keysym;

  /* any special keysym note: */
  unsigned int tme_keyboard_map_keysym_note;

  /* the keycode of the key with the keysym: */
  tme_keyboard_keyval_t tme_keyboard_map_keycode;

  /* any single modifier that this keycode triggers.  if this is
     nonzero then the set and clear modifiers must be zero: */
  int tme_keyboard_map_modifier;

  /* any modifiers that must be set or clear for in order for the
     keycode to generate this keyval: */
  tme_keyboard_modifiers_t tme_keyboard_map_modifiers_set;
  tme_keyboard_modifiers_t tme_keyboard_map_modifiers_clear;
};

/* a keyboard event: */
struct tme_keyboard_event {

  /* the type of the event: */
  int tme_keyboard_event_type;

  /* the modifier mask immediately before this event: */
  tme_keyboard_modifiers_t tme_keyboard_event_modifiers;

  /* the keysym or keycode: */
  tme_keyboard_keyval_t tme_keyboard_event_keyval;

  /* any keycode associated with a keysym: */
  tme_keyboard_keyval_t tme_keyboard_event_keycode;

  /* the time of the event: */
  tme_uint32_t tme_keyboard_event_time;
};

/* a keyboard buffer.  it is assumed that the user provides locking: */
struct tme_keyboard_buffer {

  /* the buffer size.  this must always be a power of two: */
  unsigned int tme_keyboard_buffer_size;

  /* the head and tail pointers: */
  unsigned int tme_keyboard_buffer_head;
  unsigned int tme_keyboard_buffer_tail;
  
  /* the buffered events: */
  struct tme_keyboard_event *tme_keyboard_buffer_events;

  /* any log handle: */
  struct tme_log_handle *tme_keyboard_buffer_log_handle;
};

/* a keyboard connection: */
struct tme_keyboard_connection {

  /* the generic connection side: */
  struct tme_connection tme_keyboard_connection;

  /* this is called when control lines change: */
  int (*tme_keyboard_connection_ctrl) _TME_P((struct tme_keyboard_connection *, 
					      unsigned int));

  /* this is called to read an event: */
  int (*tme_keyboard_connection_read) _TME_P((struct tme_keyboard_connection *, 
					      struct tme_keyboard_event *));

  /* this is called to lookup the keyval for a string: */
  tme_keyboard_keyval_t (*tme_keyboard_connection_lookup) _TME_P((struct tme_keyboard_connection *,
								  _tme_const struct tme_keyboard_lookup *));
};

/* prototypes: */
struct tme_keyboard_buffer *tme_keyboard_buffer_new _TME_P((unsigned int));
void tme_keyboard_buffer_destroy _TME_P((struct tme_keyboard_buffer *));
int tme_keyboard_buffer_in_modifier _TME_P((struct tme_keyboard_buffer *,
					    int, _tme_const tme_keyboard_keyval_t *));
int tme_keyboard_buffer_in_mode _TME_P((struct tme_keyboard_buffer *,
					tme_keyboard_keyval_t, int));
int tme_keyboard_buffer_in_macro _TME_P((struct tme_keyboard_buffer *,
					 _tme_const tme_keyboard_keyval_t *,
					 _tme_const tme_keyboard_keyval_t *));
int tme_keyboard_buffer_out_map _TME_P((struct tme_keyboard_buffer *,
					_tme_const struct tme_keyboard_map *));
int tme_keyboard_buffer_out_mode _TME_P((struct tme_keyboard_buffer *,
					 tme_keyboard_keyval_t, int));
tme_keyboard_modifiers_t 
tme_keyboard_buffer_out_modifiers _TME_P((struct tme_keyboard_buffer *,
					  tme_keyboard_modifiers_t,
					  tme_keyboard_modifiers_t));
int tme_keyboard_buffer_copyin _TME_P((struct tme_keyboard_buffer *,
				       _tme_const struct tme_keyboard_event *));
int tme_keyboard_buffer_copyout _TME_P((struct tme_keyboard_buffer *,
					struct tme_keyboard_event *));
int tme_keyboard_parse_macro _TME_P((_tme_const char *,
				     tme_keyboard_keysym_lookup_t,
				     void *,
				     tme_keyboard_keyval_t **,
				     tme_keyboard_keyval_t **));
int tme_keyboard_parse_map _TME_P((_tme_const char *,
				   tme_keyboard_keysym_lookup_t,
				   void *,
				   struct tme_keyboard_map *));

#endif /* !_TME_GENERIC_KEYBOARD_H */
