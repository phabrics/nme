/* $Id: serial.h,v 1.3 2003/04/29 20:28:05 fredette Exp $ */

/* tme/generic/serial.h - header file for a generic serial support: */

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

#ifndef _TME_GENERIC_SERIAL_H
#define _TME_GENERIC_SERIAL_H

#include <tme/common.h>
_TME_RCSID("$Id: serial.h,v 1.3 2003/04/29 20:28:05 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* parity types: */
#define TME_SERIAL_PARITY_NONE		(0)
#define TME_SERIAL_PARITY_ODD		(1)
#define TME_SERIAL_PARITY_EVEN		(2)

/* config flags: */
#define TME_SERIAL_FLAGS_CHECK_PARITY	TME_BIT(0)

/* serial port controls: */
#define TME_SERIAL_CTRL_DTR		TME_BIT(0)
#define TME_SERIAL_CTRL_RTS		TME_BIT(1)
#define TME_SERIAL_CTRL_DCD		TME_BIT(2)
#define TME_SERIAL_CTRL_CTS		TME_BIT(3)
#define TME_SERIAL_CTRL_BREAK		TME_BIT(4)
#define TME_SERIAL_CTRL_RI		TME_BIT(5)
#define TME_SERIAL_CTRL_OK_READ		TME_BIT(6)

/* special data flags: */
#define TME_SERIAL_DATA_NORMAL		(0)
#define TME_SERIAL_DATA_BAD_FRAME	TME_BIT(0)
#define TME_SERIAL_DATA_BAD_PARITY	TME_BIT(1)
#define TME_SERIAL_DATA_OVERRUN		TME_BIT(2)

/* copyin/copyout flags: */
#define TME_SERIAL_COPY_NORMAL		(0)
#define TME_SERIAL_COPY_FULL_IS_OVERRUN	TME_BIT(0)
#define TME_SERIAL_COPY_PEEK		TME_BIT(1)

/* this evaluates to nonzero iff a buffer is empty: */
#define tme_serial_buffer_is_empty(b)	\
  ((b)->tme_serial_buffer_head		\
   == ((b)->tme_serial_buffer_tail))

/* this evaluates to nonzero iff a buffer is full: */
#define tme_serial_buffer_is_full(b)	\
  ((((b)->tme_serial_buffer_head + 1)	\
    & ((b)->tme_serial_buffer_size - 1))\
   == (b)->tme_serial_buffer_tail)

/* types: */
typedef tme_uint8_t tme_serial_data_flags_t;

/* a serial configuration: */
struct tme_serial_config {
  
  /* the baud rate: */
  tme_uint32_t tme_serial_config_baud;

  /* the number of data bits per character: */
  tme_uint8_t tme_serial_config_bits_data;

  /* the number of stop bits per character: */
  tme_uint8_t tme_serial_config_bits_stop;

  /* the parity: */
  tme_uint8_t tme_serial_config_parity;

  /* flags: */
  tme_uint8_t tme_serial_config_flags;
};

/* a serial connection: */
struct tme_serial_connection {

  /* the generic connection side: */
  struct tme_connection tme_serial_connection;

  /* this is called when the serial configuration changes: */
  int (*tme_serial_connection_config) _TME_P((struct tme_serial_connection *, 
					      struct tme_serial_config *));

  /* this is called when control lines change: */
  int (*tme_serial_connection_ctrl) _TME_P((struct tme_serial_connection *, 
					    unsigned int));

  /* this is called to read data: */
  int (*tme_serial_connection_read) _TME_P((struct tme_serial_connection *, 
					    tme_uint8_t *, unsigned int,
					    tme_serial_data_flags_t *));
};

/* a serial buffer.  it is assumed that the user provides locking: */
struct tme_serial_buffer {

  /* the buffer size.  this must always be a power of two: */
  unsigned int tme_serial_buffer_size;

  /* our head and tail pointers: */
  unsigned int tme_serial_buffer_head;
  unsigned int tme_serial_buffer_tail;
  
  /* our buffer data: */
  tme_uint8_t *tme_serial_buffer_data;

  /* our buffer data flags: */
  tme_serial_data_flags_t *tme_serial_buffer_data_flags;
};

/* prototypes: */
int tme_serial_buffer_init _TME_P((struct tme_serial_buffer *, unsigned int));
unsigned int tme_serial_buffer_copyin _TME_P((struct tme_serial_buffer *,
					      _tme_const tme_uint8_t *,
					      unsigned int,
					      tme_serial_data_flags_t,
					      int));
unsigned int tme_serial_buffer_copyout _TME_P((struct tme_serial_buffer *,
					       tme_uint8_t *,
					       unsigned int,
					       tme_serial_data_flags_t *,
					       int));
unsigned int tme_serial_buffer_space_busy _TME_P((_tme_const struct tme_serial_buffer *));
unsigned int tme_serial_buffer_space_free _TME_P((_tme_const struct tme_serial_buffer *));
#endif /* !_TME_GENERIC_SERIAL_H */
