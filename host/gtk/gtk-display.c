/* $Id: gtk-display.c,v 1.4 2010/06/05 14:28:17 fredette Exp $ */

/* host/gtk/gtk-display.c - GTK display support: */

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
_TME_RCSID("$Id: gtk-display.c,v 1.4 2010/06/05 14:28:17 fredette Exp $");

/* includes: */
#include "gtk-display.h"

/* macros: */

/* the GTK display callout function.  it must be called with the mutex locked: */
void
_tme_gtk_display_callout(struct tme_gtk_display *display,
			 int new_callouts)
{
  struct tme_keyboard_connection *conn_keyboard;
  struct tme_mouse_connection *conn_mouse;
  int callouts, later_callouts;
  unsigned int ctrl;
  int rc;
  
  /* add in any new callouts: */
  display->tme_gtk_display_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (display->tme_gtk_display_callout_flags
      & TME_GTK_DISPLAY_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  display->tme_gtk_display_callout_flags
    |= TME_GTK_DISPLAY_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = display->tme_gtk_display_callout_flags)
	  & TME_GTK_DISPLAY_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    display->tme_gtk_display_callout_flags
      = (callouts
	 & ~TME_GTK_DISPLAY_CALLOUTS_MASK);
    callouts
      &= TME_GTK_DISPLAY_CALLOUTS_MASK;

    /* get our keyboard connection: */
    conn_keyboard = display->tme_gtk_display_keyboard_connection;

    /* if we need to call out new keyboard control information: */
    if (callouts & TME_GTK_DISPLAY_CALLOUT_KEYBOARD_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (!tme_keyboard_buffer_is_empty(display->tme_gtk_display_keyboard_buffer)) {
	ctrl |= TME_KEYBOARD_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&display->tme_gtk_display_mutex);
      
      /* do the callout: */
      rc = (conn_keyboard != NULL
	    ? ((*conn_keyboard->tme_keyboard_connection_ctrl)
	       (conn_keyboard,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&display->tme_gtk_display_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_GTK_DISPLAY_CALLOUT_KEYBOARD_CTRL;
      }
    }

    /* get our mouse connection: */
    conn_mouse = display->tme_gtk_display_mouse_connection;

    /* if we need to call out new mouse control information: */
    if (callouts & TME_GTK_DISPLAY_CALLOUT_MOUSE_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (!tme_mouse_buffer_is_empty(display->tme_gtk_display_mouse_buffer)) {
	ctrl |= TME_MOUSE_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&display->tme_gtk_display_mutex);
      
      /* do the callout: */
      rc = (conn_mouse != NULL
	    ? ((*conn_mouse->tme_mouse_connection_ctrl)
	       (conn_mouse,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&display->tme_gtk_display_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_GTK_DISPLAY_CALLOUT_MOUSE_CTRL;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  display->tme_gtk_display_callout_flags = later_callouts;

  /* yield to GTK: */
  tme_threads_gtk_yield();
}

/* this is a GTK callback for an enter notify event, that has the
   widget grab focus and then continue normal event processing: */
gint
_tme_gtk_display_enter_focus(GtkWidget *widget,
			     GdkEvent *gdk_event_raw,
			     gpointer junk)
{

  /* grab the focus: */
  gtk_widget_grab_focus(widget);

  /* continue normal event processing: */
  return (FALSE);
}

/* this creates a menu of radio buttons: */
GtkWidget *
_tme_gtk_display_menu_radio(void *state,
			    tme_gtk_display_menu_items_t menu_items)
{
  GtkWidget *menu;
  GSList *menu_group;
  struct tme_gtk_display_menu_item menu_item_buffer;
  GtkSignalFunc menu_func;
  GtkWidget *menu_item;

  /* create the menu: */
  menu = gtk_menu_new();

  /* create the menu items: */
  menu_group = NULL;
  for (menu_item_buffer.tme_gtk_display_menu_item_which = 0;
       ;
       menu_item_buffer.tme_gtk_display_menu_item_which++) {
    menu_func = (*menu_items)(state, &menu_item_buffer);
    if (menu_func == GTK_SIGNAL_FUNC(NULL)) {
      break;
    }
    menu_item
      = gtk_radio_menu_item_new_with_label(menu_group,
					   menu_item_buffer.tme_gtk_display_menu_item_string);
    if (menu_item_buffer.tme_gtk_display_menu_item_widget != NULL) {
      *menu_item_buffer.tme_gtk_display_menu_item_widget = menu_item;
    }
    menu_group
      = gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menu_item));
    gtk_signal_connect(GTK_OBJECT(menu_item), 
		       "activate",
		       menu_func,
		       (gpointer) state);
    gtk_menu_append(GTK_MENU(menu), menu_item);
    gtk_widget_show(menu_item);
  }

  /* return the menu: */
  return (menu);
}

/* this makes a new connection side for a GTK display: */
static int
_tme_gtk_display_connections_new(struct tme_element *element, 
				 const char * const *args, 
				 struct tme_connection **_conns,
				 char **_output)
{
  struct tme_gtk_display *display;

  /* recover our data structure: */
  display = (struct tme_gtk_display *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* make any new keyboard connections: */
  _tme_gtk_keyboard_connections_new(display, _conns);

  /* make any new mouse connections: */
  _tme_gtk_mouse_connections_new(display, _conns);

  /* make any new screen connections: */
  _tme_gtk_screen_connections_new(display, _conns);

  /* done: */
  return (TME_OK);
}

/* the new GTK display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_gtk,display) {
  struct tme_gtk_display *display;
  int arg_i;
  int usage;
  
  /* check our arguments: */
  usage = FALSE;
  arg_i = 1;
  for (;;) {

    if (0) {
    }

    /* if we've run out of arguments: */
    else if (args[arg_i + 0] == NULL) {

      break;
    }

    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s", 
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  if (usage) {
    tme_output_append_error(_output,
			    "%s %s",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* call gtk_init if we haven't already: */
  tme_threads_gtk_init();

  /* start our data structure: */
  display = tme_new0(struct tme_gtk_display, 1);
  display->tme_gtk_display_element = element;

  /* create the tooltips group: */
  display->tme_gtk_display_tooltips = gtk_tooltips_new();

  /* create the keyboard: */
  _tme_gtk_keyboard_new(display);

  /* create the mouse: */
  _tme_gtk_mouse_new(display);

  /* create the first screen: */
  _tme_gtk_screen_new(display);

  /* start the threads: */
  tme_mutex_init(&display->tme_gtk_display_mutex);
  tme_thread_create((tme_thread_t) _tme_gtk_screen_th_update, display);

  /* fill the element: */
  element->tme_element_private = display;
  element->tme_element_connections_new = _tme_gtk_display_connections_new;

  return (TME_OK);
}
