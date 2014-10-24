/* $Id: sun4-fdc.c,v 1.1 2006/11/15 22:54:33 fredette Exp $ */

/* machine/sun/sun-fdc.c - Sun onboard floppy disk controller implementation: */

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
_TME_RCSID("$Id: sun4-fdc.c,v 1.1 2006/11/15 22:54:33 fredette Exp $");

/* includes: */
#undef TME_NEC765_VERSION
#define TME_NEC765_VERSION TME_X_VERSION(0, 0)
#include <tme/element.h>
#include <tme/ic/nec765.h>
#include "sun4-impl.h"

/* this creates a new Sun-4 fdc: */
TME_ELEMENT_SUB_NEW_DECL(tme_machine_sun4,fdc) {
  struct tme_nec765_socket socket;

  /* create the nec765 socket: */
  memset (&socket, 0, sizeof(socket));
  socket.tme_nec765_socket_version = TME_NEC765_SOCKET_0;

  if (!TME_ARG_IS(args[1], "type")
      || args[2] == NULL) {
    tme_output_append_error(_output,
			    "%s %s type FDC-%s",
			    _("usage:"),
			    args[0],
			    _("TYPE"));
    return (EINVAL);
  }

  return (tme_element_new(element, args + 2, &socket, _output));
}
