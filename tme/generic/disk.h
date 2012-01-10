/* $Id: disk.h,v 1.2 2003/08/07 22:13:30 fredette Exp $ */

/* tme/generic/disk.h - header file for generic disk device support: */

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

#ifndef _TME_GENERIC_DISK_H
#define _TME_GENERIC_DISK_H

#include <tme/common.h>
_TME_RCSID("$Id: disk.h,v 1.2 2003/08/07 22:13:30 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* controls: */
#define TME_DISK_CONTROL_START		(0)
#define TME_DISK_CONTROL_STOP		(1)
#define TME_DISK_CONTROL_PREVENT	(2)
#define TME_DISK_CONTROL_ALLOW		(3)

/* types: */

/* a disk device connection: */
struct tme_disk_connection {

  /* the generic connection side: */
  struct tme_connection tme_disk_connection;

  /* total size: */
  union tme_value64 tme_disk_connection_size;

  /* this is called to get a buffer corresponding to a range on the
     device, for reading: */
  int (*tme_disk_connection_read) _TME_P((struct tme_disk_connection *,
					  _tme_const union tme_value64 *,
					  unsigned long,
					  _tme_const tme_uint8_t **));
  
  /* this is called to get a buffer corresponding to a range on the
     device, for writing: */
  int (*tme_disk_connection_write) _TME_P((struct tme_disk_connection *,
					   _tme_const union tme_value64 *,
					   unsigned long,
					   tme_uint8_t **));
  
  /* this releases a buffer: */
  int (*tme_disk_connection_release) _TME_P((struct tme_disk_connection *,
					     _tme_const tme_uint8_t *));
  
  /* a control function: */
  int (*tme_disk_connection_control) _TME_P((struct tme_disk_connection *,
					     unsigned int, ...));
};

/* prototypes: */
int tme_disk_connection_score _TME_P((struct tme_connection *, unsigned int *));
tme_uint32_t tme_disk_dimension_parse _TME_P((_tme_const char *));

#endif /* !_TME_GENERIC_DISK_H */
