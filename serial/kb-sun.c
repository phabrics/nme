/* $Id: kb-sun.c,v 1.2 2003/10/16 02:48:25 fredette Exp $ */

/* serial/kb-sun.c - Sun serial keyboard emulation: */

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
_TME_RCSID("$Id: kb-sun.c,v 1.2 2003/10/16 02:48:25 fredette Exp $");

/* includes: */
#include "serial-kb.h"

/* macros: */

/* Sun keyboard serial output data: */
#define KB_SUN_DATAOUT_RESET		(0xff)
#define KB_SUN_DATAOUT_LAYOUT		(0xfe)
#define KB_SUN_DATAOUT_IDLE		(0x7f)
#define KB_SUN_DATAOUT_ERROR		(0x7e)
#define KB_SUN_DATAOUT_KEYCODE_MASK	(0x7f)
#define KB_SUN_DATAOUT_KEYCODE_RELEASE	(0x80)

/* Sun keyboard serial input data.  these are also
   used as state machine states: */
#define KB_SUN_DATAIN_NONE		(0x00)
#define KB_SUN_DATAIN_RESET		(0x01)
#define KB_SUN_DATAIN_BELL_ON		(0x02)
#define KB_SUN_DATAIN_BELL_OFF		(0x03)
#define KB_SUN_DATAIN_CLICK_ON		(0x0a)
#define KB_SUN_DATAIN_CLICK_OFF		(0x0b)
#define KB_SUN_DATAIN_LEDS_SET		(0x0e)
#define KB_SUN_DATAIN_LAYOUT_GET	(0x0f)

/* Sun LED numbers: */
#define KB_SUN_LED_NUM_LOCK		(0x1)
#define KB_SUN_LED_COMPOSE		(0x2)
#define KB_SUN_LED_SCROLL_LOCK		(0x4)
#define KB_SUN_LED_CAPS_LOCK		(0x8)

/* types: */

/* keyboard type information: */
struct tme_kb_sun_type_info {

  /* the keyboard type name: */
  const char *tme_kb_sun_type_name;

  /* the keyboard type code: */
  tme_uint8_t tme_kb_sun_type_code;

  /* the keyboard type layout: */
  tme_uint8_t tme_kb_sun_type_layout;
};

/* the Sun serial keyboard state: */
struct tme_serial_kb_sun {

  /* the keyboard type information: */
  const struct tme_kb_sun_type_info *tme_serial_kb_sun_type;

  /* our serial input state: */
  tme_uint8_t tme_serial_kb_sun_input_state;

  /* the Num_Lock modifier: */
  int tme_serial_kb_sun_mod_num_lock;
};

/* globals: */
const struct tme_kb_sun_type_info tme_kb_sun_types[] = {

  /* the type 2 keyboard: */
  { "sun-type-2", 2, 0x00 },

  /* the type 3 keyboard: */
  { "sun-type-3", 3, 0x00 },

  /* the type 4 keyboards: */
  { "sun-type-4-us", 4, 0x00 },

  /* the type 5 keyboards: */
  { "sun-type-5-us", 5, 0x21 },
  { "sun-type-5-unix", 5, 0x22 },
};

/* this is called to handle serial input: */
static int
_tme_kb_sun_input(struct tme_serial_kb *serial_kb,
		  tme_uint8_t *data,
		  unsigned int count,
		  tme_serial_data_flags_t data_flags)
{
  struct tme_serial_kb_sun *kb_sun;
  tme_uint8_t c, output_buffer[2];
  int new_callouts;
  unsigned int old_empty, old_controls;
  tme_keyboard_modifiers_t modifiers_clear, modifiers_set;
  int modifier;

  /* recover our data structure: */
  kb_sun = serial_kb->tme_serial_kb_type_state;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* note if the serial buffer was empty, and its old controls: */
  old_empty
    = tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer);
  old_controls
    = serial_kb->tme_serial_kb_keyboard_ctrl;

  /* loop over the data: */
  for (; count > 0; count--) {
    
    /* get this data: */
    c = *(data++);

    /* dispatch on our state: */
    switch (kb_sun->tme_serial_kb_sun_input_state) {
    case KB_SUN_DATAIN_NONE:

      /* dispatch on the data: */
      switch (c) {

      case KB_SUN_DATAIN_RESET:

	/* add the RESET byte and our keyboard type byte to our output: */
	output_buffer[0] = KB_SUN_DATAOUT_RESET;
	output_buffer[1] = kb_sun->tme_serial_kb_sun_type->tme_kb_sun_type_code;
	tme_serial_buffer_copyin(&serial_kb->tme_serial_kb_serial_buffer,
				 output_buffer,
				 2, 
				 TME_SERIAL_DATA_NORMAL,
				 TME_SERIAL_COPY_FULL_IS_OVERRUN);
	break;

      case KB_SUN_DATAIN_BELL_ON:
	serial_kb->tme_serial_kb_keyboard_ctrl
	  |= TME_KEYBOARD_CTRL_BELL;
	break;

      case KB_SUN_DATAIN_BELL_OFF:
	serial_kb->tme_serial_kb_keyboard_ctrl
	  &= ~TME_KEYBOARD_CTRL_BELL;
	break;

      case KB_SUN_DATAIN_CLICK_ON:
      case KB_SUN_DATAIN_CLICK_OFF:
	/* do nothing for now; the generic keyboard interface
	   doesn't support keyboard click: */
	break;

	/* KB_SUN_DATAIN_LEDS_SET is only supported on type 4 and
           later keyboards: */
      case KB_SUN_DATAIN_LEDS_SET:
	if (kb_sun->tme_serial_kb_sun_type->tme_kb_sun_type_code
	    >= 4) {
	  kb_sun->tme_serial_kb_sun_input_state = c;
	}
	break;
	    
	/* KB_SUN_DATAIN_LAYOUT_GET is only supported on type 4 and
           later keyboards: */
      case KB_SUN_DATAIN_LAYOUT_GET:
	if (kb_sun->tme_serial_kb_sun_type->tme_kb_sun_type_code
	    >= 4) {

	  /* add the LAYOUT byte and our keyboard layout byte to our output: */
	  output_buffer[0] = KB_SUN_DATAOUT_LAYOUT;
	  output_buffer[1] = kb_sun->tme_serial_kb_sun_type->tme_kb_sun_type_layout;
	  tme_serial_buffer_copyin(&serial_kb->tme_serial_kb_serial_buffer,
				   output_buffer,
				   2, 
				   TME_SERIAL_DATA_NORMAL,
				   TME_SERIAL_COPY_FULL_IS_OVERRUN);
	}
	break;

      default:
	/* ignore this byte: */
	break;
      }
      break;

    case KB_SUN_DATAIN_LEDS_SET:

      /* update the keyboard controls and the output modifiers: */
      /* XXX the mapping of Sun keyboard LED to generic LED is
         arbitrary: */
      modifiers_clear = 0;
      modifiers_set = 0;
      modifier = kb_sun->tme_serial_kb_sun_mod_num_lock;
      if (c & KB_SUN_LED_NUM_LOCK) {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  |= TME_KEYBOARD_CTRL_LED0;
	if (modifier != TME_KEYBOARD_MODIFIER_NONE) {
	  modifiers_set |= (1 << modifier);
	}
      }
      else {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  &= ~TME_KEYBOARD_CTRL_LED0;
	if (modifier != TME_KEYBOARD_MODIFIER_NONE) {
	  modifiers_clear |= (1 << modifier);
	}
      }
      if (c & KB_SUN_LED_COMPOSE) {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  |= TME_KEYBOARD_CTRL_LED1;
      }
      else {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  &= ~TME_KEYBOARD_CTRL_LED1;
      }
      if (c & KB_SUN_LED_SCROLL_LOCK) {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  |= TME_KEYBOARD_CTRL_LED2;
      }
      else {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  &= ~TME_KEYBOARD_CTRL_LED2;
      }
      if (c & KB_SUN_LED_CAPS_LOCK) {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  |= TME_KEYBOARD_CTRL_LED3;
	modifiers_set |= (1 << TME_KEYBOARD_MODIFIER_LOCK);
      }
      else {
	serial_kb->tme_serial_kb_keyboard_ctrl
	  &= ~TME_KEYBOARD_CTRL_LED3;
	modifiers_clear |= (1 << TME_KEYBOARD_MODIFIER_LOCK);
      }

      tme_keyboard_buffer_out_modifiers(serial_kb->tme_serial_kb_keyboard_buffer,
					modifiers_clear,
					modifiers_set);
      
      kb_sun->tme_serial_kb_sun_input_state = KB_SUN_DATAIN_NONE;
      break;

    default: assert(FALSE);
    }
  }

  /* if the serial buffer was empty before and it isn't now,
     update the serial controls and call them out: */
  if (old_empty
      && !tme_serial_buffer_is_empty(&serial_kb->tme_serial_kb_serial_buffer)) {
    new_callouts
      |= TME_SERIAL_KB_CALLOUT_SERIAL_CTRL;
  }

  /* if the keyboard controls have changed, call them out: */
  if (old_controls
      != serial_kb->tme_serial_kb_keyboard_ctrl) {
    new_callouts
      |= TME_SERIAL_KB_CALLOUT_KEYBOARD_CTRL;
  }

  /* set these new callouts: */
  serial_kb->tme_serial_kb_callout_flags |= new_callouts;

  return (TME_OK);
}

/* this is called before a map entry has been added: */
static int
_tme_kb_sun_map_add_pre(struct tme_serial_kb *serial_kb,
			struct tme_keyboard_map *map)
{
  /* make sure that the keyboard code is within range: */
  if (map->tme_keyboard_map_keycode >= KB_SUN_DATAOUT_IDLE) {
    return (EINVAL);
  }
  return (TME_OK);
}

/* this is called after a map entry has been added: */
static int
_tme_kb_sun_map_add_post(struct tme_serial_kb *serial_kb,
			 const struct tme_keyboard_map *map)
{
  struct tme_serial_kb_sun *kb_sun;
  int mode;
  int rc;

  /* recover our data structure: */
  kb_sun = serial_kb->tme_serial_kb_type_state;

  /* if this key is attached to a modifier: */
  if (map->tme_keyboard_map_modifier
      != TME_KEYBOARD_MODIFIER_NONE) {

    /* modifiers cannot autorepeat: */
    mode
      = (TME_KEYBOARD_MODE_PASSTHROUGH
	 | TME_KEYBOARD_MODE_FLAG_NO_AUTOREPEATS);

    /* if this key is attached to the lock modifier, it soft-locks: */
    if (map->tme_keyboard_map_modifier
	== TME_KEYBOARD_MODIFIER_LOCK) {
      mode |= TME_KEYBOARD_MODE_FLAG_LOCK_SOFT;
    }

    /* set the output mode on this key: */
    rc = tme_keyboard_buffer_out_mode(serial_kb->tme_serial_kb_keyboard_buffer,
				      map->tme_keyboard_map_keycode,
				      mode);
    assert (rc == TME_OK);

    /* if this key is Num_Lock, remember
       which modifier: */
    if (map->tme_keyboard_map_keysym_note
	== TME_KEYBOARD_KEYSYM_NOTE_NUM_LOCK) {
      kb_sun->tme_serial_kb_sun_mod_num_lock
	= map->tme_keyboard_map_modifier;
    }
  }

  return (TME_OK);
}

/* this is called to get serial data from a keyboard event: */
static tme_uint8_t
_tme_kb_sun_event(struct tme_serial_kb *serial_kb,
		  const struct tme_keyboard_event *event)
{
  tme_uint8_t data;

  /* Sun keyboard key-release data has the high bit set: */
  data = event->tme_keyboard_event_keyval;
  if (event->tme_keyboard_event_type
      == TME_KEYBOARD_EVENT_RELEASE) {
    data |= KB_SUN_DATAOUT_KEYCODE_RELEASE;
  }
  return (data);
}

/* this initializes Sun serial keyboard emulation: */
int
_tme_serial_kb_sun_init(struct tme_serial_kb *serial_kb)
{
  struct tme_serial_kb_sun *kb_sun;
  const struct tme_kb_sun_type_info *type_info;
  struct tme_serial_config *config;
  unsigned int type_info_i;
  int rc;

  /* allocate the Sun serial keyboard state: */
  kb_sun = tme_new0(struct tme_serial_kb_sun, 1);
  serial_kb->tme_serial_kb_type_state = kb_sun;

  /* initialize our state: */
  kb_sun->tme_serial_kb_sun_mod_num_lock
    = TME_KEYBOARD_MODIFIER_NONE;

  /* find the information for this keyboard type: */
  type_info = NULL;
  for (type_info_i = 0;
       type_info_i < TME_ARRAY_ELS(tme_kb_sun_types);
       type_info_i++) {
    if (!strcmp(tme_kb_sun_types[type_info_i].tme_kb_sun_type_name,
		serial_kb->tme_serial_kb_type)) {
      type_info = &tme_kb_sun_types[type_info_i];
      break;
    }
  }
  assert (type_info != NULL);
  kb_sun->tme_serial_kb_sun_type = type_info;

  /* set the global output keyboard mode.  most keys on Sun keyboards
     can follow the host: */
  rc = tme_keyboard_buffer_out_mode(serial_kb->tme_serial_kb_keyboard_buffer,
				    TME_KEYBOARD_KEYVAL_UNDEF,
				    TME_KEYBOARD_MODE_PASSTHROUGH);
  assert (rc == TME_OK);

  /* set our type-dependent functions: */
  serial_kb->tme_serial_kb_type_map_add_pre = _tme_kb_sun_map_add_pre;
  serial_kb->tme_serial_kb_type_map_add_post = _tme_kb_sun_map_add_post;
  serial_kb->tme_serial_kb_type_event = _tme_kb_sun_event;
  serial_kb->tme_serial_kb_type_serial_ctrl = NULL;
  serial_kb->tme_serial_kb_type_serial_input = _tme_kb_sun_input;

  /* set our serial configuration: */
  config = &serial_kb->tme_serial_kb_type_config;
  memset(config, 0, sizeof(*config));
  config->tme_serial_config_baud = 1200;
  config->tme_serial_config_bits_data = 8;
  config->tme_serial_config_bits_stop = 1;
  config->tme_serial_config_parity = TME_SERIAL_PARITY_NONE;

  return (TME_OK);
}
