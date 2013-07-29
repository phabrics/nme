/* $Id: runlength.h,v 1.1 2010/02/07 14:07:25 fredette Exp $ */

/* tme/runlength.h - header file for run length: */

/*
 * Copyright (c) 2010 Matt Fredette
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

#ifndef _TME_RUNLENGTH_H
#define _TME_RUNLENGTH_H

#include <tme/common.h>
_TME_RCSID("$Id: runlength.h,v 1.1 2010/02/07 14:07:25 fredette Exp $");

/* types: */

/* a runlength value: */
typedef tme_uint32_t tme_runlength_t;

/* the runlength state: */
struct tme_runlength {

  /* the runlength history count: */
  unsigned int tme_runlength_history_count;

  /* the runlength history: */
  tme_runlength_t *_tme_runlength_history;

  /* the next position in the runlength history: */
  unsigned int _tme_runlength_history_next;

  /* the runlength history sum: */
  double _tme_runlength_history_sum;

  /* the runlength cycles target: */
  double _tme_runlength_cycles_elapsed_target;

  /* the start time of the run: */
  union tme_value64 tme_runlength_cycles_start;

  /* the current runlength value: */
  tme_runlength_t tme_runlength_value;
};

/* prototypes: */

/* this initializes runlength state: */
void tme_runlength_init _TME_P((struct tme_runlength *));

/* this sets the runlength cycles target: */
void tme_runlength_target_cycles _TME_P((struct tme_runlength *, union tme_value64));

/* this updates the runlength: */
void tme_runlength_update _TME_P((struct tme_runlength *));

#endif /* !_TME_RUNLENGTH_H */
