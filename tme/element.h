/* $Id: element.h,v 1.5 2003/05/16 21:48:14 fredette Exp $ */

/* tme/element.h - public header file for elements: */

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

#ifndef _TME_ELEMENT_H
#define _TME_ELEMENT_H

#include <tme/common.h>
_TME_RCSID("$Id: element.h,v 1.5 2003/05/16 21:48:14 fredette Exp $");

/* includes: */
#include <tme/connection.h>
#include <tme/module.h>
#include <tme/log.h>

/* macros: */

/* these make exported element new function symbol names: */
#define TME_ELEMENT_NEW_SYM(module) TME_MODULE_SYM(module,new)
#define TME_ELEMENT_SUB_NEW_SYM(module,sub) TME_MODULE_SUB_SYM(module,sub,_new)
#define TME_ELEMENT_X_NEW_SYM(class,agg,sub) TME_MODULE_X_SYM(class,agg,sub,_new)

/* these make exported element new function prototypes: */
#define _TME_ELEMENT_NEW_P(sym)				\
int sym _TME_P((struct tme_element *, _tme_const char * _tme_const *, const void *, char **))
#define TME_ELEMENT_NEW_P(module)			\
_TME_ELEMENT_NEW_P(TME_ELEMENT_NEW_SYM(module))
#define TME_ELEMENT_SUB_NEW_P(module,sub)		\
_TME_ELEMENT_NEW_P(TME_ELEMENT_SUB_NEW_SYM(module,sub))
#define TME_ELEMENT_X_NEW_P(class,agg,sub)		\
_TME_ELEMENT_NEW_P(TME_ELEMENT_X_NEW_SYM(class,agg,sub))

/* these declare exported new functions: */
#ifdef __STDC__
#define _TME_ELEMENT_NEW_DECL(sym)			\
_TME_ELEMENT_NEW_P(sym);				\
int sym(struct tme_element *element, const char * const *args, const void *extra, char **_output)
#else  /* !__STDC__ */
#define _TME_ELEMENT_NEW_DECL(sym)			\
_TME_ELEMENT_NEW_P(sym);				\
int sym(element, args, extra, _output)			\
  struct tme_element *element;				\
  const char **args;					\
  const void *extra;					\
  char **_output;
#endif /* !__STDC__ */
#define TME_ELEMENT_NEW_DECL(module)			\
_TME_ELEMENT_NEW_DECL(TME_ELEMENT_NEW_SYM(module))
#define TME_ELEMENT_SUB_NEW_DECL(module,sub)		\
_TME_ELEMENT_NEW_DECL(TME_ELEMENT_SUB_NEW_SYM(module,sub))
#define TME_ELEMENT_X_NEW_DECL(class,agg,sub)		\
_TME_ELEMENT_NEW_DECL(TME_ELEMENT_X_NEW_SYM(class,agg,sub))

/* structures: */
struct tme_connection;

/* an element: */
struct tme_element {

  /* elements can be kept in a list: */
  struct tme_element *tme_element_next;

  /* the module implementing the element: */
  void *tme_element_module;

  /* the element's private data structure: */
  void *tme_element_private;

  /* the logging handle: */
  struct tme_log_handle tme_element_log_handle;

  /* this function returns the possible new connection sides to the
     element: */
  int (*tme_element_connections_new) _TME_P((struct tme_element *, 
					     _tme_const char * _tme_const *, 
					     struct tme_connection **,
					     char **));

  /* the element command function: */
  int (*tme_element_command) _TME_P((struct tme_element *,
				     _tme_const char * _tme_const *,
				     char **));
};

/* prototypes: */
int tme_element_new _TME_P((struct tme_element *,
			    _tme_const char * _tme_const *, 
			    void *,
			    char **));
int tme_element_connect _TME_P((struct tme_element *,
				_tme_const char * _tme_const *,
				struct tme_element *,
				_tme_const char * _tme_const *,
				char **, int *));

#endif /* !_TME_ELEMENT_H */
