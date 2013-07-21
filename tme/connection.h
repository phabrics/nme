/* $Id: connection.h,v 1.11 2009/08/30 13:07:32 fredette Exp $ */

/* tme/connection.h - public header file for connections: */

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

#ifndef _TME_CONNECTION_H
#define _TME_CONNECTION_H

#include <tme/common.h>
_TME_RCSID("$Id: connection.h,v 1.11 2009/08/30 13:07:32 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */
#define TME_CONNECTION_CLOSED	(0)
#define TME_CONNECTION_HALF	(1)
#define TME_CONNECTION_FULL	(2)

/* structures: */
struct tme_element;

/* one side of a connection: */
struct tme_connection {

  /* connection sides can be kept in a list: */
  struct tme_connection *tme_connection_next;

  /* a backpointer to the element: */
  struct tme_element *tme_connection_element;

  /* the element's identifier for this connection: */
  tme_uint32_t tme_connection_id;

  /* the type of this connection: */
  unsigned int tme_connection_type;

  /* the other side of the connection: */
  struct tme_connection *tme_connection_other;

  /* the connection scoring function: */
  int (*tme_connection_score) _TME_P((struct tme_connection *, unsigned int *));

  /* the connection making function: */
  int (*tme_connection_make) _TME_P((struct tme_connection *, unsigned int));

  /* the connection breaking function: */
  int (*tme_connection_break) _TME_P((struct tme_connection *, unsigned int));
};

/* all connection types: */
#define TME_CONNECTION_BUS_GENERIC	(0)
#define TME_CONNECTION_BUS_M68K		(1)
#define TME_CONNECTION_SERIAL		(2)
#define TME_CONNECTION_ETHERNET		(3)
#define TME_CONNECTION_KEYBOARD		(4)
#define TME_CONNECTION_FRAMEBUFFER	(5)
#define TME_CONNECTION_SCSI		(6)
#define TME_CONNECTION_DISK		(7)
#define TME_CONNECTION_MOUSE		(8)
#define TME_CONNECTION_TAPE		(9)
#define TME_CONNECTION_BUS_SPARC	(10)
#define TME_CONNECTION_BUS_UPA		(11)

#endif /* !_TME_CONNECTION_H */
