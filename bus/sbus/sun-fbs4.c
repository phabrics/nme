/* $Id: sun-fbs4.c,v 1.2 2009/08/29 21:27:00 fredette Exp $ */

/* bus/sbus/sun-fbs4.c - Sun SBus S4 framebuffer emulation: */

/*
 * Copyright (c) 2006 Matt Fredette
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
_TME_RCSID("$Id: sun-fbs4.c,v 1.2 2009/08/29 21:27:00 fredette Exp $");

/* includes: */
#include <tme/machine/sun.h>

/* this creates a new SBus bwtwo: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_sbus,bwtwo) {
  const char *sub_args[4];

  /* the bwtwo implementation requires an explicit type: */
  sub_args[0] = args[0];
  sub_args[1] = "type";
  sub_args[2] = "sbus";
  sub_args[3] = NULL;

  /* we don't expect any arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    return (EINVAL);
  }

  /* create the new bwtwo: */
  return (tme_sun_bwtwo(element, sub_args, _output));
}

/* this creates a new Sbus cgthree: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_sbus,cgthree) {
  return (tme_sun_cgthree(element, args, _output));
}

/* this creates a new Sbus cgsix: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_sbus,cgsix) {
  return (tme_sun_cgsix(element, args, _output));
}
