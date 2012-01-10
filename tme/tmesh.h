/* $Id: tmesh.h,v 1.1 2003/05/16 21:48:15 fredette Exp $ */

/* tme/tmesh.h - header file for tmesh: */

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

#ifndef _TME_TMESH_H
#define _TME_TMESH_H

#include <tme/common.h>
_TME_RCSID("$Id: tmesh.h,v 1.1 2003/05/16 21:48:15 fredette Exp $");

/* includes: */
#include <tme/log.h>

/* macros: */
#define TMESH_C_EOF			(-1)
#define TMESH_C_YIELD			(TMESH_C_EOF - 1)
#define tmesh_init()			(TME_OK)

/* tmesh input/output: */
struct tmesh_io {

  /* the name for this io: */
  char *tmesh_io_name;

  /* the private state for this io: */
  void *tmesh_io_private;

  /* the input line number for this io: */
  unsigned long tmesh_io_input_line;

  /* the getc function: */
  int (*tmesh_io_getc) _TME_P((struct tmesh_io *));

  /* the close function: */
  void (*tmesh_io_close) _TME_P((struct tmesh_io *, 
				 struct tmesh_io *));

  /* the open function: */
  int (*tmesh_io_open) _TME_P((struct tmesh_io *, 
			       struct tmesh_io *,
			       char **));
};

/* tmesh support: */
struct tmesh_support {

  /* the private state for this support: */
  void *tmesh_support_private;

  /* the log handle open function: */
  void (*tmesh_support_log_open) _TME_P((struct tmesh_support *, 
					 struct tme_log_handle *, 
					 _tme_const char *,
					 _tme_const char *));

  /* the log handle close function: */
  void (*tmesh_support_log_close) _TME_P((struct tmesh_support *, 
					  struct tme_log_handle *));
};

/* prototypes: */
void *tmesh_new _TME_P((_tme_const struct tmesh_support *, _tme_const struct tmesh_io *));
int tmesh_eval _TME_P((void *, char **, int *));

#endif /* !_TME_TMESH_H */
