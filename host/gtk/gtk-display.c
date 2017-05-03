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

static void _tme_gtk_init(void) {
  char **argv;
  char *argv_buffer[3];
  int argc;

  /* GTK requires program to be running non-setuid */
#ifdef HAVE_SETUID
  setuid(getuid());
#endif
  
  /* conjure up an argv.  this is pretty bad: */
  argv = argv_buffer;
  argc = 0;
  argv[argc++] = "tmesh";
#if 1
  argv[argc++] = "--gtk-debug=signals";
#endif
  argv[argc] = NULL;
  gtk_init(&argc, &argv);
}

/* this is a GTK callback for an enter notify event, that has the
   widget grab focus and then continue normal event processing: */
gint
_tme_display_enter_focus(GtkWidget *widget,
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
_tme_display_menu_radio(void *state,
			    tme_display_menu_items_t menu_items)
{
  GtkWidget *menu;
  GSList *menu_group;
  struct tme_display_menu_item menu_item_buffer;
  GCallback menu_func;
  GtkWidget *menu_item;

  /* create the menu: */
  menu = gtk_menu_new();

  /* create the menu items: */
  menu_group = NULL;
  for (menu_item_buffer.tme_display_menu_item_which = 0;
       ;
       menu_item_buffer.tme_display_menu_item_which++) {
    menu_func = (*menu_items)(state, &menu_item_buffer);
    if (menu_func == G_CALLBACK(NULL)) {
      break;
    }
    menu_item
      = gtk_radio_menu_item_new_with_label(menu_group,
					   menu_item_buffer.tme_display_menu_item_string);
    if (menu_item_buffer.tme_display_menu_item_widget != NULL) {
      *menu_item_buffer.tme_display_menu_item_widget = menu_item;
    }
    menu_group
      = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menu_item));
    g_signal_connect(menu_item, 
		     "activate",
		     menu_func,
		     (gpointer) state);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
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
  struct tme_display *display;

  /* recover our data structure: */
  display = (struct tme_display *) element->tme_element_private;

  /* we never take any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* make any new keyboard connections: */
  _tme_display_connections_new(element, args, _conns, _output);

  /* make any new screen connections: */
  _tme_gtk_screen_connections_new(display, _conns);

  /* done: */
  return (TME_OK);
}

/* the new GTK display function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_gtk,display) {
  struct tme_display *display;

  /* start our data structure: */
  tme_display_init(element);

  /* recover our data structure: */
  display = element->tme_element_private;

  /* setup the thread loop function: */
  tme_threads_init(_tme_gtk_screen_update, display);
  _tme_gtk_init();
  
  /* create the first screen: */
  _tme_gtk_screen_new(display);

  /* fill the element: */
  element->tme_element_connections_new = _tme_gtk_display_connections_new;

  return (TME_OK);
}
