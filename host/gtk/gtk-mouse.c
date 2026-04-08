/* $Id: gtk-mouse.c,v 1.3 2007/03/03 15:33:22 fredette Exp $ */

/* host/gtk/gtk-mouse.c - GTK mouse support: */

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
_TME_RCSID("$Id: gtk-mouse.c,v 1.3 2007/03/03 15:33:22 fredette Exp $");

/* includes: */
#include "gtk-display.h"
#include <gdk/gdkkeysyms.h>

#if 0
/* this warps the pointer to the middle of the Draw: */
void
_tme_gtk_mouse_warp_pointer(struct tme_gtk_screen *screen)
{
  struct tme_gdk_display *display;

  /* get the display: */
  display = screen->screen.tme_screen_display;

  gdk_device_warp(gdk_seat_get_pointer(display->tme_gdk_display_seat),
		  gdk_screen_get_default(),
		  screen->screen.tme_screen_mouse_warp_x,
		  screen->screen.tme_screen_mouse_warp_y);
}
#endif

/* this is a GTK callback for a mouse event in the framebuffer event box: */
static void
_tme_gtk_mouse_motion_event(GtkEventControllerMotion* self,
			    gdouble x,
			    gdouble y,
			    struct tme_gtk_screen *screen)

{
  if(gtk_switch_get_state(GTK_SWITCH(screen->tme_gtk_screen_mouse_button)))
    _tme_mouse_event(0, x, y, screen->screen.tme_screen_display);
}

/* this is a GTK callback for a mouse event in the framebuffer event box: */
static void
_tme_gtk_mouse_button_down(GtkGesture* self,
			   int n_press,
			   gdouble x,
			   gdouble y,
			   struct tme_gtk_screen *screen)
{
  if(gtk_switch_get_state(GTK_SWITCH(screen->tme_gtk_screen_mouse_button)))
    _tme_mouse_event(gtk_gesture_single_get_current_button(self),
		     x, y, screen->screen.tme_screen_display);
}

/* this is a GTK callback for a mouse event in the framebuffer event box: */
static void
_tme_gtk_mouse_button_up(GtkGesture* self,
			 int n_press,
			 gdouble x,
			 gdouble y,
			 struct tme_gtk_screen *screen)
{
  if(gtk_switch_get_state(GTK_SWITCH(screen->tme_gtk_screen_mouse_button)))
    _tme_mouse_event(-gtk_gesture_single_get_current_button(self),
		     x, y, screen->screen.tme_screen_display);
}

/* this attaches the GTK mouse to a new screen: */
void
_tme_gtk_mouse_attach(struct tme_gtk_screen *screen)
{
  GtkEventController *motion, *event;
  GtkGesture *press;

  /* create the mouse label, button, and key: */
  screen->tme_gtk_screen_mouse_label = gtk_label_new(_("Mouse Mode"));
  screen->tme_gtk_screen_mouse_button = gtk_switch_new();
  //    = gtk_toggle_button_new_with_label(_("Mouse Mode"));
  screen->tme_gtk_screen_mouse_key = gtk_entry_new();
  
  gtk_header_bar_pack_start(GTK_HEADER_BAR(screen->tme_gtk_screen_header),
			    screen->tme_gtk_screen_mouse_label);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(screen->tme_gtk_screen_header),
			    screen->tme_gtk_screen_mouse_button);
  gtk_header_bar_pack_start(GTK_HEADER_BAR(screen->tme_gtk_screen_header),
			    screen->tme_gtk_screen_mouse_key);

#if GTK_MAJOR_VERSION == 4
  motion=gtk_event_controller_motion_new();
  gtk_widget_add_controller(screen->tme_gtk_screen_draw, motion);
  press = gtk_gesture_click_new();
  gtk_widget_add_controller(screen->tme_gtk_screen_draw, GTK_EVENT_CONTROLLER (press));
#elif GTK_MAJOR_VERSION == 3
  motion=screen->motion=gtk_event_controller_motion_new(screen->tme_gtk_screen_draw);
  press=screen->press=gtk_gesture_multi_press_new(screen->tme_gtk_screen_draw);
#endif
  
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE (press), 0);
  /* set the tip on the entry box: */
  gtk_widget_set_tooltip_text(screen->tme_gtk_screen_mouse_key,
			      "Press a key here to turn the mouse on.  The same key " \
			      "will turn the mouse off.");

  g_signal_connect_swapped(motion,
			   "enter",
			   G_CALLBACK(gtk_widget_grab_focus),
			   screen->tme_gtk_screen_draw);

  /* although the event mask doesn't include these events yet,
     set a signal handler for the mouse events: */
  g_signal_connect(motion,
		   "motion",
		   G_CALLBACK(_tme_gtk_mouse_motion_event), 
		   screen);

  g_signal_connect(press, "released", G_CALLBACK(_tme_gtk_mouse_button_up),
		   screen);

  g_signal_connect(press, "pressed", G_CALLBACK(_tme_gtk_mouse_button_down),
		   screen); 
  
  /* mouse mode is off: */
  screen->tme_gtk_screen_mouse_keyval = GDK_KEY_VoidSymbol;
}
