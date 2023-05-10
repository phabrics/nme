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

/* macros: */

/* types: */

/* this is a GTK callback for a key press or release event: */
static gboolean
_tme_gtk_keyboard_key_down(
  GtkEventControllerKey* self,
  guint keyval,
  guint keycode,
  GdkModifierType state,
  struct tme_gtk_screen *screen)
{
  if(screen->tme_gtk_screen_mouse_keyval
     != keyval)
    return _tme_keyboard_key_event((state<<1)+1, keyval, screen->screen.tme_screen_display);

  gtk_switch_set_active(GTK_SWITCH(screen->tme_gtk_screen_mouse_button), FALSE);

  return (TRUE);
  
}

static gboolean
_tme_gtk_keyboard_key_up(
  GtkEventControllerKey* self,
  guint keyval,
  guint keycode,
  GdkModifierType state,
  struct tme_gtk_screen *screen)
{
  return _tme_keyboard_key_event(state<<1, keyval, screen->screen.tme_screen_display);
}

/* this sets the mouse mode, e.g., on/off.  it is called with the mutex locked: */
static gboolean
_tme_gtk_mouse_key_down(
  GtkEventControllerKey* self,
  guint keyval,
  guint keycode,
  GdkModifierType* state,
  struct tme_gtk_screen *screen)
{
  struct tme_gtk_display *display;
  int rc;
  char *status;

  /* recover our data structure: */
  display = screen->screen.tme_screen_display;

  /* lock the mutex: */
  tme_mutex_lock(&display->display.tme_display_mutex);

#if GTK_MAJOR_VERSION == 4
  gtk_editable_set_text(GTK_EDITABLE(screen->tme_gtk_screen_mouse_key), gdk_keyval_name(keyval));
#elif GTK_MAJOR_VERSION == 3
  gtk_entry_set_text(GTK_ENTRY(screen->tme_gtk_screen_mouse_key), gdk_keyval_name(keyval));
#endif  

  /* we are now in mouse mode: */
  screen->tme_gtk_screen_mouse_keyval
    = keyval;

  gtk_widget_grab_focus(screen->tme_gtk_screen_draw);

  /* unlock the mutex: */
  tme_mutex_unlock(&display->display.tme_display_mutex);

  return (TRUE);
}

/* this attaches the GTK keyboard to a new screen: */
void
_tme_gtk_keyboard_attach(struct tme_gtk_screen *screen)
{
  GtkEventController *key, *mouse;

#if GTK_MAJOR_VERSION == 4
  key=gtk_event_controller_key_new();
  gtk_widget_add_controller(screen->tme_gtk_screen_draw, key);
  gtk_widget_set_focusable(screen->tme_gtk_screen_draw, TRUE);
  mouse=gtk_event_controller_key_new();
  gtk_widget_add_controller(screen->tme_gtk_screen_mouse_key, mouse);
#elif GTK_MAJOR_VERSION == 3
  key=screen->key=gtk_event_controller_key_new(screen->tme_gtk_screen_draw);
  gtk_widget_set_can_focus(screen->tme_gtk_screen_draw, TRUE);
  mouse=screen->mouse=gtk_event_controller_key_new(screen->tme_gtk_screen_mouse_key);
#endif
  
  g_signal_connect_after(key,
			 "key-pressed",
			 G_CALLBACK(_tme_gtk_keyboard_key_down), 
			 screen);

  g_signal_connect_after(key,
			 "key-released",
			 G_CALLBACK(_tme_gtk_keyboard_key_up), 
			 screen); 

  g_signal_connect(mouse,
		   "key-pressed",
		   G_CALLBACK(_tme_gtk_mouse_key_down), 
		   screen);

}
