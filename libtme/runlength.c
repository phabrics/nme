/* $Id: runlength.c,v 1.1 2010/02/07 14:07:25 fredette Exp $ */

/* libtme/runlength.c - run length: */

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

#include <tme/common.h>
_TME_RCSID("$Id: runlength.c,v 1.1 2010/02/07 14:07:25 fredette Exp $");

/* includes: */
#include <tme/runlength.h>
#include <tme/misc.h>

/* this initializes runlength state: */
void
tme_runlength_init(struct tme_runlength *runlength)
{
  unsigned long runlength_history_count;
  tme_runlength_t runlength_value;

  /* allocate the runlength history: */
  runlength_history_count = runlength->tme_runlength_history_count;
  assert (runlength_history_count > 0);
  runlength->_tme_runlength_history = tme_new(tme_runlength_t, runlength_history_count);

  /* set the initial runlength history sum: */
  runlength_value = runlength->tme_runlength_value;
  runlength->_tme_runlength_history_sum
    = (((double) runlength_value)
       * runlength->tme_runlength_history_count);
  
  /* initialize the runlength history: */
  do {
    runlength->_tme_runlength_history[runlength_history_count - 1] = runlength_value;
  } while (--runlength_history_count);
  runlength->_tme_runlength_history_next = 0;
}

/* this sets the runlength cycles target: */
void
tme_runlength_target_cycles(struct tme_runlength *runlength,
			    union tme_value64 cycles_elapsed_target_value64)
{
  double two_to_the_thirtysecond;

  /* make 2^32: */
  two_to_the_thirtysecond = 65536 * (double) 65536;

  /* set the target number of cycles to elapse during a run: */
  runlength->_tme_runlength_cycles_elapsed_target
    = ((cycles_elapsed_target_value64.tme_value64_uint32_hi
	* two_to_the_thirtysecond)
       + cycles_elapsed_target_value64.tme_value64_uint32_lo);
}

/* this updates the runlength: */
void
tme_runlength_update(struct tme_runlength *runlength)
{
  double two_to_the_thirtysecond;
  union tme_value64 cycles_elapsed;
  tme_runlength_t runlength_value;
  unsigned long runlength_history_next;
  double runlength_history_sum;

  /* make 2^32: */
  two_to_the_thirtysecond =  65536 * (double) 65536;

  /* get the number of cycles that elapsed during this run: */
  cycles_elapsed = tme_misc_cycles();
  (void) tme_value64_sub(&cycles_elapsed, &runlength->tme_runlength_cycles_start);

  /* get a better length for this run: */
  runlength_value
    = (runlength->tme_runlength_value
       * (runlength->_tme_runlength_cycles_elapsed_target
	  / ((cycles_elapsed.tme_value64_uint32_hi
	      * two_to_the_thirtysecond)
	     + cycles_elapsed.tme_value64_uint32_lo)));
  if (__tme_predict_false(runlength_value == 0)) {
    runlength_value += 1;
  }

  /* update the runlength history and sum: */
  runlength_history_next = runlength->_tme_runlength_history_next;
  runlength_history_sum
    = ((runlength->_tme_runlength_history_sum
	- runlength->_tme_runlength_history[runlength_history_next])
       + runlength_value);
  runlength->_tme_runlength_history[runlength_history_next] = runlength_value;
  runlength->_tme_runlength_history_sum = runlength_history_sum;
  if (runlength_history_next == 0) {
    runlength_history_next = runlength->tme_runlength_history_count;
  }
  runlength->_tme_runlength_history_next = runlength_history_next - 1;

  /* update the runlength value: */
  runlength->tme_runlength_value
    = (runlength_history_sum
       / runlength->tme_runlength_history_count);
}
