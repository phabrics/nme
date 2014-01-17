/* $Id: tape.h,v 1.1 2003/08/05 03:39:33 fredette Exp $ */

/* tme/generic/tape.h - header file for generic tape device support: */

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

#ifndef _TME_GENERIC_TAPE_H
#define _TME_GENERIC_TAPE_H

#include <tme/common.h>
_TME_RCSID("$Id: tape.h,v 1.1 2003/08/05 03:39:33 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* flags: */
#define TME_TAPE_FLAG_FIXED		TME_BIT(0)
#define TME_TAPE_FLAG_ILI		TME_BIT(1)
#define TME_TAPE_FLAG_MARK		TME_BIT(2)
#define TME_TAPE_FLAG_EOM		TME_BIT(3)

/* controls: */
#define TME_TAPE_CONTROL_LOAD		(0)
#define TME_TAPE_CONTROL_UNLOAD		(1)
#define TME_TAPE_CONTROL_DENSITY_GET	(2)
#define TME_TAPE_CONTROL_DENSITY_SET	(3)
#define TME_TAPE_CONTROL_BLOCK_SIZE_GET	(4)
#define TME_TAPE_CONTROL_BLOCK_SIZE_SET	(5)
#define TME_TAPE_CONTROL_MARK_WRITE	(6)
#define TME_TAPE_CONTROL_MARK_SKIPF	(7)
#define TME_TAPE_CONTROL_MARK_SKIPR	(8)
#define TME_TAPE_CONTROL_REWIND		(9)

/* types: */

/* a tape control function: */
struct tme_tape_control {

  /* the private state: */
  void *tme_tape_control_private;

  /* the control function: */
  int (*tme_tape_connection_control) _TME_P((void *,
					     unsigned int, ...));
};

/* a tape device connection: */
struct tme_tape_connection {

  /* the generic connection side: */
  struct tme_connection tme_tape_connection;

  /* this is called to get a buffer for reading the device: */
  int (*tme_tape_connection_read) _TME_P((struct tme_tape_connection *,
					  int *, unsigned long *,
					  unsigned long *,
					  _tme_const tme_uint8_t **));
  
  /* this is called to get a buffer for writing the device: */
  int (*tme_tape_connection_write) _TME_P((struct tme_tape_connection *,
					   int, unsigned long,
					   unsigned long *,
					   tme_uint8_t **));
  
  /* this releases a buffer: */
  int (*tme_tape_connection_release) _TME_P((struct tme_tape_connection *,
					     int *, unsigned long *));
  
  /* a control function: */
  int (*tme_tape_connection_control) _TME_P((struct tme_tape_connection *,
					     unsigned int, ...));
};

/* prototypes: */
int tme_tape_connection_score _TME_P((struct tme_connection *, unsigned int *));

#endif /* !_TME_GENERIC_TAPE_H */
