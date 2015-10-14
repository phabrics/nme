/* tmesh/tmesh-threads.c - the tme thread utility functions: */

/*
 * Copyright (c) 2015 Ruben Agin
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
_TME_RCSID("$Id: threads-sjlj.c,v 1.18 2010/06/05 19:10:28 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/module.h>
#ifdef HAVE_GTK
#ifndef G_ENABLE_DEBUG
#define G_ENABLE_DEBUG (0)
#endif /* !G_ENABLE_DEBUG */
#include <gtk/gtk.h>

/* nonzero iff we're using the gtk main loop: */
static int tme_using_gtk;

void tme_threads_gtk_init(void)
{
  char **argv;
  char *argv_buffer[3];
  int argc;

  /* if we've already initialized GTK: */
  if (tme_using_gtk) {
    return;
  }

  /* make sure we aren't running setuid */
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

  /* we are now using GTK: */
  tme_using_gtk = TRUE;
}
#else 
#define tme_using_gtk FALSE
#endif /* !HAVE_GTK */

void tme_threads_run() {
  tme_sjlj_threads_run(tme_using_gtk);

  _tme_thread_suspended();
#ifdef HAVE_GTK
  /* if we're using the GTK main loop, yield to GTK and
     call gtk_main(): */
  if (tme_using_gtk) {
    gtk_main();
  } else
#endif /* HAVE_GTK */
  while(1) {
    usleep(1000000);
  }
}

/* this initializes modules: */
static _tme_inline int tme_module_init _TME_P((void)) {
  int rc;
  _tme_module_init();
  LTDL_SET_PRELOADED_SYMBOLS();
  rc = lt_dlinit();
  if (rc != 0) {
    return (-1);
  }
  return (TME_OK);
}

/* this initializes libtme: */
void tme_init _TME_P((void)) {
  /* initialize the threading system: */
  tme_threads_init();

  /* initialize the module system: */
  tme_module_init();
}

