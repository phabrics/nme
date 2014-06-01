/* $Id: element.c,v 1.2 2006/09/30 12:34:21 fredette Exp $ */

/* libtme/element.c - element management: */

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
_TME_RCSID("$Id: element.c,v 1.2 2006/09/30 12:34:21 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/* this connects two elements together: */
int
tme_element_connect(struct tme_element *element0, 
		    const char * const *args0,
		    struct tme_element *element1, 
		    const char * const *args1,
		    char **_output, int *_which)
{
  struct tme_connection *conns0, *conns1;
  struct tme_connection *conn0, *conn1;
  struct tme_connection *conn0_next, *conn1_next;
  unsigned int score0, score1;
  struct tme_connection *best_conn0, *best_conn1;
  unsigned int score, best_score;
  int rc;

  /* get the possible new connections from the elements: */
  conns0 = NULL;
  rc = (*element0->tme_element_connections_new)(element0,
						args0,
						&conns0,
						_output);
  if (rc != TME_OK) {
    *_which = 0;
    return (rc);
  }
  conns1 = NULL;
  rc = (*element1->tme_element_connections_new)(element1,
						args1,
						&conns1,
						_output);
  if (rc != TME_OK) {
    *_which = 1;
    return (rc);
  }

  /* set the element on all new connections: */
  for (conn0 = conns0;
       conn0 != NULL; 
       conn0 = conn0->tme_connection_next) {
    conn0->tme_connection_element = element0;
  }
  for (conn1 = conns1;
       conn1 != NULL; 
       conn1 = conn1->tme_connection_next) {
    conn1->tme_connection_element = element1;
  }

  /* find the best way to connect these two elements: */
  best_conn0 = NULL;
  best_conn1 = NULL;
  best_score = 0;
  for (conn0 = conns0;
       conn0 != NULL; 
       conn0 = conn0->tme_connection_next) {
    for (conn1 = conns1;
	 conn1 != NULL; 
	 conn1 = conn1->tme_connection_next) {

      /* these two connections have to be the same type: */
      if (conn0->tme_connection_type
	  != conn1->tme_connection_type) {
	continue;
      }

      /* score the two connections: */
      conn0->tme_connection_other = conn1;
      (*conn0->tme_connection_score)(conn0, &score0);
      conn1->tme_connection_other = conn0;
      (*conn1->tme_connection_score)(conn1, &score1);
      score = (score0 * score1);
      
      /* if this is a new best score: */
      if (score > best_score) {
	best_score = score;
	best_conn0 = conn0;
	best_conn1 = conn1;
      }
    }
  }

  /* free all of the other connections: */
  for (conn0 = conns0; conn0 != NULL; conn0 = conn0_next) {
    conn0_next = conn0->tme_connection_next;
    if (conn0 != best_conn0) {
      tme_free(conn0);
    }
  }
  for (conn1 = conns1; conn1 != NULL; conn1 = conn1_next) {
    conn1_next = conn1->tme_connection_next;
    if (conn1 != best_conn1) {
      tme_free(conn1);
    }
  }

  /* if no connection was possible: */
  if (best_score == 0) {
    return (EOPNOTSUPP);
  }

  /* make the two connections: */
  best_conn0->tme_connection_other = best_conn1;
  (*best_conn0->tme_connection_make)(best_conn0, TME_CONNECTION_HALF);
  best_conn1->tme_connection_other = best_conn0;
  (*best_conn1->tme_connection_make)(best_conn1, TME_CONNECTION_FULL);
  (*best_conn0->tme_connection_make)(best_conn0, TME_CONNECTION_FULL);

  return (TME_OK);
}

/* this creates a new element: */
int
tme_element_new(struct tme_element *element, 
		const char * const *args,
		void *extra,
		char **_output)
{
  void *module;
  _TME_ELEMENT_NEW_P((*new));
  int rc;

  /* we must have at least one argument: */
  assert(args[0] != NULL);

  /* open the module: */
  rc = tme_module_open(args[0], &module, _output);
  if (rc != TME_OK) {
    return (rc);
  }
  element->tme_element_module = module;

  /* look up the "new" function: */
  new = (_TME_ELEMENT_NEW_P((*))) tme_module_symbol(module, "new");
  if (new == NULL) {
    return (ENOENT);
  }

  /* initialize more of the element structure: */
  element->tme_element_private = NULL;
  element->tme_element_connections_new = NULL;
  element->tme_element_command = NULL;

  /* call the new function: */
  rc = ((*new)(element, args, extra, _output));

  /* if the module we opened still isn't the module for the element,
     close the module: */
  if (element->tme_element_module != module) {
    tme_module_close(module);
  }

  return (rc);
}
