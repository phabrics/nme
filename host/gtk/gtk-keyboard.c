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
static int
_tme_gtk_keyboard_key_event(GtkWidget *widget,
			    GdkEvent *gdk_event_raw,
			    struct tme_gtk_screen *screen)
{
  struct tme_display *display;
  GdkEventKey *gdk_event;
  struct tme_keyboard_event tme_event;

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

  _tme_thread_resumed();
  
  /* recover our data structure: */
  display = screen->screen.tme_screen_display;

  /* lock the mutex: */
  tme_mutex_lock(&display->tme_display_mutex);

  /* if this is a press of the mouse mode off key, turn mouse mode off
     and return now: */
  if (gdk_event->type == GDK_KEY_PRESS
      && (gdk_event->keyval
	  == screen->tme_gtk_screen_mouse_keyval)) {
    _tme_gtk_mouse_mode_off(screen,
			    gdk_event->time);

    /* unlock the mutex: */
    tme_mutex_unlock(&display->tme_display_mutex);
    
    _tme_thread_suspended();
    return (TRUE);
  }

  _tme_keyboard_key_event(&tme_event, display);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&display->tme_display_mutex);

  _tme_thread_suspended();

  /* don't process this event any further: */
  return (TRUE);
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
  gtk_widget_add_events(screen->tme_gtk_screen_gtkframe,
			GDK_ENTER_NOTIFY_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK);
  gtk_widget_add_events (gtk_widget_get_toplevel(screen->tme_gtk_screen_gtkframe),
			 GDK_KEY_PRESS_MASK
			 | GDK_KEY_RELEASE_MASK);

  /* set a signal handler for these events: */
  g_signal_connect(screen->tme_gtk_screen_gtkframe,
		   "enter_notify_event",
		   G_CALLBACK(_tme_display_enter_focus),
		   NULL);
  g_signal_connect_after(screen->tme_gtk_screen_gtkframe,
			 "key_press_event",
			 G_CALLBACK(_tme_gtk_keyboard_key_event), 
			 screen);
  g_signal_connect_after(screen->tme_gtk_screen_gtkframe,
			 "key_release_event",
			 G_CALLBACK(_tme_gtk_keyboard_key_event), 
			 screen);
  
  /* the event box can focus, and have it grab the focus now: */
  gtk_widget_set_can_focus(screen->tme_gtk_screen_gtkframe, TRUE);
  gtk_widget_grab_focus(screen->tme_gtk_screen_gtkframe);
}

