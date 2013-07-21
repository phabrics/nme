/* $Id: gtk-keyboard.c,v 1.10 2007/02/15 02:15:41 fredette Exp $ */

/* host/gtk/gtk-keyboard.c - GTK keyboard support: */

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
_TME_RCSID("$Id: gtk-keyboard.c,v 1.10 2007/02/15 02:15:41 fredette Exp $");

/* includes: */
#include "gtk-display.h"
#include <gdk/gdkkeysyms.h>

/* macros: */

/* types: */

/* a GTK keysym: */
struct tme_gtk_keysym {

  /* the type of this keysym.  this is either
     TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT or
     TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC, depending on whether or not
     the keysym needed to be allocated: */
  int tme_gtk_keysym_type;

  /* the keysym itself: */
  tme_keyboard_keyval_t tme_gtk_keysym_keysym;
};

/* if X11 support is present: */
#ifndef X_DISPLAY_MISSING

/* includes: */
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#if TME_KEYBOARD_EVENT_TIME_UNDEF != CurrentTime
#error "TME_KEYBOARD_EVENT_TIME_UNDEF and CurrentTime disagree"
#endif

/* the X11 half of keyboard initialization: */
static void
_tme_gtk_keyboard_x11_new(struct tme_gtk_display *display)
{
  struct tme_keyboard_buffer *buffer;
  XModifierKeymap *modifier_keymap;
  int x_modifier, modifier;
  int keycode_to_modifier[(1 << (sizeof(KeyCode) * 8))];
  tme_keyboard_keyval_t *modifier_keysyms[TME_KEYBOARD_MODIFIER_MAX + 1];
  int modifier_keysyms_count[TME_KEYBOARD_MODIFIER_MAX + 1];
  int keycode_min, keycode_max, keycode, keycode_i;
  int keymap_width, keysym_i, keysym_j;
  KeySym *keymap, keysym, keysym_cases[2];
  const char *string;
  struct tme_gtk_keysym *gtk_keysym;
  int rc;
  
  /* get the keyboard buffer: */
  buffer = display->tme_gtk_display_keyboard_buffer;

  /* note all keycodes that are attached to modifiers: */
  for (keycode = 0;
       keycode < (int) TME_ARRAY_ELS(keycode_to_modifier);
       keycode++) {
    keycode_to_modifier[keycode] = TME_KEYBOARD_MODIFIER_NONE;
  }
  modifier_keymap = XGetModifierMapping(GDK_DISPLAY());
  for (x_modifier = 0;
       x_modifier < 8;
       x_modifier++) {

    /* turn this X modifier into a tme modifier: */
    switch (x_modifier) {
    case ShiftMapIndex: modifier = TME_KEYBOARD_MODIFIER_SHIFT ; break;
    case LockMapIndex: modifier = TME_KEYBOARD_MODIFIER_LOCK ; break;
    case ControlMapIndex: modifier = TME_KEYBOARD_MODIFIER_CONTROL ; break;
    case Mod1MapIndex: modifier = TME_KEYBOARD_MODIFIER_MOD1 ; break;
    case Mod2MapIndex: modifier = TME_KEYBOARD_MODIFIER_MOD2 ; break;
    case Mod3MapIndex: modifier = TME_KEYBOARD_MODIFIER_MOD3 ; break;
    case Mod4MapIndex: modifier = TME_KEYBOARD_MODIFIER_MOD4 ; break;
    default: assert (FALSE);
    case Mod5MapIndex: modifier = TME_KEYBOARD_MODIFIER_MOD5 ; break;
    }

    /* note all keycodes that are attached to this modifier: */
    for (keycode_i = 0;
	 keycode_i < modifier_keymap->max_keypermod;
	 keycode_i++) {
      keycode = *(modifier_keymap->modifiermap
		  + (x_modifier * modifier_keymap->max_keypermod)
		  + keycode_i);
      if (keycode != 0) {
	keycode_to_modifier[keycode] = modifier;
      }
    }
  }
  XFreeModifiermap(modifier_keymap);

  /* get the keycode range: */
  XDisplayKeycodes(GDK_DISPLAY(), &keycode_min, &keycode_max);

  /* get the keyboard mapping: */
  keymap = XGetKeyboardMapping(GDK_DISPLAY(), 
			       keycode_min,
			       (keycode_max - keycode_min) + 1,
			       &keymap_width);

  /* initialize the lists of modifiers to keysyms: */
  memset (modifier_keysyms_count, 0, sizeof(modifier_keysyms_count));

  /* loop over the keycodes in the keyboard mapping: */
  for (keycode = keycode_min;
       keycode <= keycode_max;
       keycode++) {

    /* if this keycode is attached to a modifier, we will take the
       first keysym that this keycode maps to as the keysym attached
       to the modifier: */
    modifier = keycode_to_modifier[keycode];

    /* loop over the keysyms that this keycode can map to: */
    for (keysym_i = 0;
	 keysym_i < keymap_width;
	 keysym_i++) {

      /* get this keysym: */
      keysym = *(keymap
		 + ((keycode - keycode_min)
		    * keymap_width)
		 + keysym_i);

      /* ignore NoSymbol: */
      if (keysym == NoSymbol) {
	continue;
      }
      
      /* get the upper- and lowercase versions of this keysym.  if
	 this keysym has no case, this sets both keysym_cases[] values
	 to keysym: */
      XConvertCase(keysym, &keysym_cases[0], &keysym_cases[1]);

      for (keysym_j = 0;
	   keysym_j < (int) TME_ARRAY_ELS(keysym_cases);
	   keysym_j++) {
	keysym = keysym_cases[keysym_j];
	
	/* ignore any keysym that doesn't have a string
	   name that GDK knows as the same keysym: */
	if ((string = XKeysymToString(keysym)) == NULL
	    || gdk_keyval_from_name(string) != keysym) {
	  continue;
	}

	/* skip this keysym if it's already known: */
	if (tme_hash_lookup(display->tme_gtk_display_keyboard_keysyms,
			    (tme_hash_data_t) string)
	    != (tme_hash_data_t) NULL) {

	  /* if there is a keycode associated with this keysym,
	     remove it if it's different from this keycode: */
	  if (tme_hash_lookup(display->tme_gtk_display_keyboard_keysym_to_keycode,
			      (tme_hash_data_t) keysym)
	      != tme_keyboard_hash_data_from_keyval((tme_uint32_t) keycode)) {
	    tme_hash_remove(display->tme_gtk_display_keyboard_keysym_to_keycode,
			    (tme_hash_data_t) keysym);
	  }

	  continue;
	}
	
	/* if this keysym is attached to a modifier: */
	if (modifier != TME_KEYBOARD_MODIFIER_NONE) {

	  /* grow the modifier to keysyms list for this modifier.
	     this list always has an extra entry in it, for the
	     TME_KEYBOARD_KEYVAL_UNDEF that will terminate the list: */
	  if (modifier_keysyms_count[modifier] == 0) {
	    modifier_keysyms[modifier]
	      = tme_new(tme_keyboard_keyval_t, 2);
	  }
	  else {
	    modifier_keysyms[modifier]
	      = tme_renew(tme_keyboard_keyval_t, 
			  modifier_keysyms[modifier],
			  modifier_keysyms_count[modifier] + 2);
	  }

	  /* store this modifier keysym: */
	  modifier_keysyms[modifier][modifier_keysyms_count[modifier]]
	    = keysym;
	  modifier_keysyms_count[modifier]++;

	  /* if this is a modifier that locks, tell the keyboard
	     buffer so that it can unlock it: */
	  /* XXX if this X server *doesn't* give these keysyms
	     locking behavior, we must *not* unlock them, because
	     inferring the extra transitions would be wrong: */
	  /* XXX we shouldn't attach them to the modifier either -
	     because the keyboard buffer input stage zero top half
	     assumes that keysyms attached to modifiers that it knows
	     about all have locking behavior.  this is a feature bug
	     in the keyboard buffer: */
	  if (!strcmp(string, "Caps_Lock")
	      || !strcmp(string, "Shift_Lock")
	      || !strcmp(string, "Num_Lock")) {
	    rc = tme_keyboard_buffer_in_mode(buffer,
					     keysym,
					     TME_KEYBOARD_MODE_UNLOCK);
	    assert (rc == TME_OK);
	  }
	  
	  /* don't attach more than one keysym per keycode to
	     this modifier: */
	  modifier = TME_KEYBOARD_MODIFIER_NONE;
	}

	/* remember that this keysym can be generated directly: */
	gtk_keysym = tme_new0(struct tme_gtk_keysym, 1);
	gtk_keysym->tme_gtk_keysym_type
	  = TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT;
	gtk_keysym->tme_gtk_keysym_keysym
	  = keysym;
	tme_hash_insert(display->tme_gtk_display_keyboard_keysyms,
			(tme_hash_data_t) string,
			(tme_hash_data_t) gtk_keysym);
	tme_hash_insert(display->tme_gtk_display_keyboard_keysym_to_keycode,
			(tme_hash_data_t) keysym,
			tme_keyboard_hash_data_from_keyval((tme_uint32_t) keycode));
      }
    }
  }

  /* free the keyboard mapping: */
  XFree(keymap);

  /* add in the modifiers information: */
  for (modifier = 0;
       modifier < TME_KEYBOARD_MODIFIER_MAX;
       modifier++) {
    if (modifier_keysyms_count[modifier] > 0) {
      modifier_keysyms[modifier][modifier_keysyms_count[modifier]]
	= TME_KEYBOARD_KEYVAL_UNDEF;
      rc = tme_keyboard_buffer_in_modifier(buffer,
					   modifier,
					   modifier_keysyms[modifier]);
      assert (rc == TME_OK);
      tme_free(modifier_keysyms[modifier]);
    }
  }
}

#endif /* !X_DISPLAY_MISSING */

#if 1
/* this is used for debugging only: */
const char *_tme_gtk_keyboard_keyval_name _TME_P((tme_keyboard_keyval_t));
const char *
_tme_gtk_keyboard_keyval_name(tme_keyboard_keyval_t keyval)
{
  return (gdk_keyval_name(keyval));
}
#endif

/* this is a GTK callback for a key press or release event: */
static int
_tme_gtk_keyboard_key_event(GtkWidget *widget,
			    GdkEvent *gdk_event_raw,
			    struct tme_gtk_screen *screen)
{
  struct tme_gtk_display *display;
  GdkEventKey *gdk_event;
  struct tme_keyboard_event tme_event;
  int was_empty;
  int new_callouts;
  int rc;

  /* make a tme event from this gdk event: */
  gdk_event = &gdk_event_raw->key;
  tme_event.tme_keyboard_event_type
    = (gdk_event->type == GDK_KEY_PRESS
       ? TME_KEYBOARD_EVENT_PRESS
       : TME_KEYBOARD_EVENT_RELEASE);
  tme_event.tme_keyboard_event_modifiers
    = gdk_event->state;
  tme_event.tme_keyboard_event_keyval
    = gdk_event->keyval;
  tme_event.tme_keyboard_event_time
    = gdk_event->time;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;
  
  /* recover our data structure: */
  display = screen->tme_gtk_screen_display;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* if this is a press of the mouse mode off key, turn mouse mode off
     and return now: */
  if (gdk_event->type == GDK_KEY_PRESS
      && (gdk_event->keyval
	  == screen->tme_gtk_screen_mouse_keyval)) {
    _tme_gtk_mouse_mode_off(screen,
			    gdk_event->time);

    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);
    
    return (TRUE);
  }

  /* get any keycode associated with this keysym: */
  tme_event.tme_keyboard_event_keycode
    = tme_keyboard_hash_data_to_keyval(tme_hash_lookup(display->tme_gtk_display_keyboard_keysym_to_keycode,
						       tme_keyboard_hash_data_from_keyval(tme_event.tme_keyboard_event_keyval)));

  /* remember if the keyboard buffer was empty: */
  was_empty
    = tme_keyboard_buffer_is_empty(display->tme_gtk_display_keyboard_buffer);

  /* add this tme event to the keyboard buffer: */
  rc = tme_keyboard_buffer_copyin(display->tme_gtk_display_keyboard_buffer,
				  &tme_event);
  assert (rc == TME_OK);

  /* if the keyboard buffer was empty and now it isn't,
     call out the keyboard controls: */
  if (was_empty
      && !tme_keyboard_buffer_is_empty(display->tme_gtk_display_keyboard_buffer)) {
    new_callouts |= TME_GTK_DISPLAY_CALLOUT_KEYBOARD_CTRL;
  }

  /* run any callouts: */
  _tme_gtk_display_callout(display, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  /* don't process this event any further: */
  return (TRUE);
}

/* this is called to look up a keysym: */
static tme_keyboard_keyval_t
_tme_gtk_keyboard_lookup(struct tme_keyboard_connection *conn_keyboard,
			 const struct tme_keyboard_lookup *lookup)
{
  struct tme_gtk_display *display;
  struct tme_gtk_keysym *keysym;
  struct tme_gtk_keysym_bad **_keysym_bad, *keysym_bad;
  char *string;
  const char *string_other;
  guint _keysym;

  /* recover our data structure: */
  display = conn_keyboard
    ->tme_keyboard_connection.tme_connection_element->tme_element_private;
  
  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* if this is the "no more lookups" call, log complaints about all
     keysyms that we were asked to look up, but have no way of
     generating: */
  if (lookup == NULL) {
    for (; display->tme_gtk_display_keyboard_keysyms_bad != NULL; ) {
      keysym_bad
	= display->tme_gtk_display_keyboard_keysyms_bad;

      /* log the complaint: */
      tme_log(&display->tme_gtk_display_element->tme_element_log_handle, 0, ENOENT,
	      (&display->tme_gtk_display_element->tme_element_log_handle,
	       _("cannot generate keysym '%s' directly%s"),
	       keysym_bad->tme_gtk_keysym_bad_string,
	       (keysym_bad->tme_keysym_bad_flags   
		== TME_KEYBOARD_LOOKUP_FLAG_OK_DIRECT
		? ""
		: _(", or through a macro"))));

      /* free this record: */
      display->tme_gtk_display_keyboard_keysyms_bad
	= keysym_bad->tme_gtk_keysym_bad_next;
      tme_free(keysym_bad->tme_gtk_keysym_bad_string);
      tme_free(keysym_bad->tme_gtk_keysym_bad_context);
      tme_free(keysym_bad);
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);

    return (TME_OK);
  }

  /* if this lookup has context, find any existing bad keysym record
     with the same context: */
  _keysym_bad = NULL;
  keysym_bad = NULL;
  if (lookup->tme_keyboard_lookup_context_length > 0) {
    for (_keysym_bad
	   = &display->tme_gtk_display_keyboard_keysyms_bad;
	 (keysym_bad = *_keysym_bad) != NULL;
	 _keysym_bad
	   = &keysym_bad->tme_gtk_keysym_bad_next) {
      
      /* stop if this bad keysym record has this context: */
      if ((keysym_bad->tme_gtk_keysym_bad_context_length
	   == lookup->tme_keyboard_lookup_context_length)
	  && !memcmp(keysym_bad->tme_gtk_keysym_bad_context,
		     lookup->tme_keyboard_lookup_context,
		     lookup->tme_keyboard_lookup_context_length)) {
	break;
      }
    }
  }

  /* look up this keysym: */
  keysym
    = ((struct tme_gtk_keysym *)
       tme_hash_lookup(display->tme_gtk_display_keyboard_keysyms,
		       (tme_hash_data_t) lookup->tme_keyboard_lookup_string));
  
  /* if the lookup failed and we're allowed to allocate this keysym: */
  if (keysym == NULL
      && (lookup->tme_keyboard_lookup_flags
	  & TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC_NOW)) {
      
    /* duplicate the lookup string: */
    string = tme_strdup(lookup->tme_keyboard_lookup_string);
    
    /* create the new keysym: */
    keysym = tme_new(struct tme_gtk_keysym, 1);
    keysym->tme_gtk_keysym_type
      = TME_KEYBOARD_LOOKUP_FLAG_OK_ALLOC;
    
    /* if GDK knows a keysym for this name: */
    keysym->tme_gtk_keysym_keysym
      = gdk_keyval_from_name(string);
    
    /* if GDK doesn't know a keysym for this name, or if the string
       for this keysym isn't the same name, find an unused keysym: */
    if (keysym->tme_gtk_keysym_keysym == GDK_VoidSymbol
	|| (string_other = gdk_keyval_name(keysym->tme_gtk_keysym_keysym)) == NULL
	|| strcmp(string, string_other)) {
      
      /* loop until we have an unused keysym that isn't also
	 TME_KEYBOARD_KEYVAL_UNDEF, or until we have exhausted the
	 GDK keysym space: */
      for (_keysym = display->tme_gtk_display_keyboard_keysym_alloc_next;;) {
	if ((_keysym + 1) == 0) {
	  abort();
	}
	if (_keysym != TME_KEYBOARD_KEYVAL_UNDEF
	    && _keysym != GDK_VoidSymbol
	    && gdk_keyval_name(_keysym) == NULL) {
	  break;
	}
	_keysym++;
      }
      display->tme_gtk_display_keyboard_keysym_alloc_next = _keysym + 1;
      
      /* use this new keysym: */
      keysym->tme_gtk_keysym_keysym
	= _keysym;
    }
    
    /* add this newly allocated keysym: */
    tme_hash_insert(display->tme_gtk_display_keyboard_keysyms,
		    (tme_hash_data_t) string,
		    (tme_hash_data_t) keysym);
  }

  /* if the lookup failed or didn't give the right kind of keysym: */
  if (keysym == NULL
      || !(keysym->tme_gtk_keysym_type
	   & lookup->tme_keyboard_lookup_flags)) {

    /* if this lookup has context, and no bad keysym record
       exists for this context, create a new one: */
    if (lookup->tme_keyboard_lookup_context_length > 0
	&& keysym_bad == NULL) {

      /* create the new bad keysym record: */
      keysym_bad = tme_new0(struct tme_gtk_keysym_bad, 1);
      keysym_bad->tme_gtk_keysym_bad_next
	= display->tme_gtk_display_keyboard_keysyms_bad;
      keysym_bad->tme_gtk_keysym_bad_string
	= tme_strdup(lookup->tme_keyboard_lookup_string);
      keysym_bad->tme_keysym_bad_flags
	= lookup->tme_keyboard_lookup_flags;
      keysym_bad->tme_gtk_keysym_bad_context_length
	= lookup->tme_keyboard_lookup_context_length;
      keysym_bad->tme_gtk_keysym_bad_context
	= tme_dup(tme_uint8_t,
		  lookup->tme_keyboard_lookup_context,
		  lookup->tme_keyboard_lookup_context_length);

      /* link in the new bad keysym record: */
      display->tme_gtk_display_keyboard_keysyms_bad
	= keysym_bad;
    }

    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_gtk_display_mutex);

    /* return failure: */
    return (TME_KEYBOARD_KEYVAL_UNDEF);
  }

  /* otherwise, this lookup succeeded.  if a bad keysym record existed
     for this context, forget it - this successful lookup forgives
     that earlier failure: */
  if (keysym_bad != NULL) {
    *_keysym_bad = keysym_bad->tme_gtk_keysym_bad_next;
    tme_free(keysym_bad->tme_gtk_keysym_bad_context);
    tme_free(keysym_bad);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  return (keysym->tme_gtk_keysym_keysym);
}

/* this is called when the keyboard controls change: */
static int
_tme_gtk_keyboard_ctrl(struct tme_keyboard_connection *conn_keyboard, 
		       unsigned int ctrl)
{
  struct tme_gtk_display *display;

  /* recover our data structure: */
  display = conn_keyboard
    ->tme_keyboard_connection.tme_connection_element->tme_element_private;

  /* ring the bell: */
  if (ctrl & TME_KEYBOARD_CTRL_BELL) {
    gdk_beep();
  }

  return (TME_OK);
}

/* this is called to read the keyboard: */
static int
_tme_gtk_keyboard_read(struct tme_keyboard_connection *conn_keyboard, 
		       struct tme_keyboard_event *event)
{
  struct tme_gtk_display *display;
  int rc;

  /* recover our data structure: */
  display = conn_keyboard
    ->tme_keyboard_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_gtk_display_mutex);

  /* copy an event out of the keyboard buffer: */
  rc = tme_keyboard_buffer_copyout(display->tme_gtk_display_keyboard_buffer,
				   event);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_gtk_display_mutex);

  return (rc);
}

/* this breaks a keyboard connection: */
static int
_tme_gtk_keyboard_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new keyboard connection: */
static int
_tme_gtk_keyboard_connection_make(struct tme_connection *conn,
				  unsigned int state)
{
  struct tme_gtk_display *display;

  /* recover our data structure: */
  display = conn->tme_connection_element->tme_element_private;

  /* both sides must be keyboard connections: */
  assert(conn->tme_connection_type
	 == TME_CONNECTION_KEYBOARD);
  assert(conn->tme_connection_other->tme_connection_type
	 == TME_CONNECTION_KEYBOARD);

  /* we are always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* save our connection: */
    tme_mutex_lock(&display->tme_gtk_display_mutex);
    display->tme_gtk_display_keyboard_connection
      = (struct tme_keyboard_connection *) conn->tme_connection_other;
    tme_mutex_unlock(&display->tme_gtk_display_mutex);
  }

  return (TME_OK);
}

/* this scores a keyboard connection: */
static int
_tme_gtk_keyboard_connection_score(struct tme_connection *conn,
				   unsigned int *_score)
{
  struct tme_keyboard_connection *conn_keyboard;

  /* both sides must be keyboard connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_KEYBOARD);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_KEYBOARD);

  /* the other side cannot be a real keyboard: */
  conn_keyboard
    = (struct tme_keyboard_connection *) conn->tme_connection_other;
  *_score = (conn_keyboard->tme_keyboard_connection_read == NULL
	     && conn_keyboard->tme_keyboard_connection_lookup == NULL);
  return (TME_OK);
}

/* this makes a new connection side for a GTK keyboard: */
int
_tme_gtk_keyboard_connections_new(struct tme_gtk_display *display, 
				  struct tme_connection **_conns)
{
  struct tme_keyboard_connection *conn_keyboard;
  struct tme_connection *conn;

  /* if we don't have a keyboard connection yet: */
  if (display->tme_gtk_display_keyboard_connection == NULL) {

    /* create our side of a keyboard connection: */
    conn_keyboard = tme_new0(struct tme_keyboard_connection, 1);
    conn = &conn_keyboard->tme_keyboard_connection;

    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_KEYBOARD;
    conn->tme_connection_score = _tme_gtk_keyboard_connection_score;
    conn->tme_connection_make = _tme_gtk_keyboard_connection_make;
    conn->tme_connection_break = _tme_gtk_keyboard_connection_break;

    /* fill in the keyboard connection: */
    conn_keyboard->tme_keyboard_connection_ctrl = _tme_gtk_keyboard_ctrl;
    conn_keyboard->tme_keyboard_connection_read = _tme_gtk_keyboard_read;
    conn_keyboard->tme_keyboard_connection_lookup = _tme_gtk_keyboard_lookup;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* this attaches the GTK keyboard to a new screen: */
void
_tme_gtk_keyboard_attach(struct tme_gtk_screen *screen)
{

  /* make sure the event box for the framebuffer gets enter, key_press
     and key_release events.  we have to add these latter two events
     to both the event box widget itself, and the top-level window,
     since GTK 1.x appears to not select KeyRelease events at the
     top-level: */
  gtk_widget_add_events(screen->tme_gtk_screen_event_box,
			GDK_ENTER_NOTIFY_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK);
  gtk_widget_add_events (gtk_widget_get_toplevel(screen->tme_gtk_screen_event_box),
			 GDK_KEY_PRESS_MASK
			 | GDK_KEY_RELEASE_MASK);

  /* set a signal handler for these events: */
  gtk_signal_connect(GTK_OBJECT(screen->tme_gtk_screen_event_box),
		     "enter_notify_event",
		     GTK_SIGNAL_FUNC(_tme_gtk_display_enter_focus),
		     NULL);
  gtk_signal_connect_after(GTK_OBJECT(screen->tme_gtk_screen_event_box),
			   "key_press_event",
			   GTK_SIGNAL_FUNC(_tme_gtk_keyboard_key_event), 
			   screen);
  gtk_signal_connect_after(GTK_OBJECT(screen->tme_gtk_screen_event_box),
			   "key_release_event",
			   GTK_SIGNAL_FUNC(_tme_gtk_keyboard_key_event), 
			   screen);

  /* the event box can focus, and have it grab the focus now: */
  GTK_WIDGET_SET_FLAGS(screen->tme_gtk_screen_event_box, GTK_CAN_FOCUS);
  gtk_widget_grab_focus(screen->tme_gtk_screen_event_box);
}

/* this initializes keyboard part of the display: */
void
_tme_gtk_keyboard_new(struct tme_gtk_display *display)
{

  /* we have no keyboard connection: */
  display->tme_gtk_display_keyboard_connection = NULL;
  
  /* allocate the keyboard buffer: */
  display->tme_gtk_display_keyboard_buffer
    = tme_keyboard_buffer_new(1024);
  display->tme_gtk_display_keyboard_buffer->tme_keyboard_buffer_log_handle
    = &display->tme_gtk_display_element->tme_element_log_handle;

  /* allocate the keysyms hash: */
  display->tme_gtk_display_keyboard_keysyms
    = tme_hash_new(tme_string_hash,
		   tme_string_compare,
		   (tme_hash_data_t) 0);

  /* allocate the keysym-to-keycode hash: */
  display->tme_gtk_display_keyboard_keysym_to_keycode
    = tme_hash_new(tme_direct_hash,
		   tme_direct_compare,
		   (tme_hash_data_t) TME_KEYBOARD_KEYVAL_UNDEF);

  /* there are no bad keysyms: */
  display->tme_gtk_display_keyboard_keysyms_bad = NULL;

  /* initialize the input stages of the keyboard buffer.  this needs
     information that we can't get from GTK or GDK, like which keysyms
     are attached to which modifiers, and which keysyms can be
     generated by the keyboard: */
#ifndef X_DISPLAY_MISSING
  if (TRUE) {
    _tme_gtk_keyboard_x11_new(display);
  }
#endif /* !X_DISPLAY_MISSING */
  else {
    abort();
  }
}
