/* $Id: module.h,v 1.1 2003/05/16 21:48:14 fredette Exp $ */

/* tme/module.h - public header file for modules: */

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

#ifndef _TME_MODULE_H
#define _TME_MODULE_H

#include <tme/common.h>
_TME_RCSID("$Id: module.h,v 1.1 2003/05/16 21:48:14 fredette Exp $");

#include <ltdl.h>

/* macros: */

/* these make exported symbol names: */
#define TME_MODULE_SYM(module,sym) _TME_CONCAT3(module,_LTX_,sym)
#define TME_MODULE_SUB_SYM(module,sub,sym) _TME_CONCAT4(module,_LTX_,sub,sym)
#define TME_MODULE_X_SYM(class,agg,sub,sym) _TME_CONCAT5(class,agg,_LTX_,sub,sym)

/* prototypes: */
/* this initializes modules: */
void _tme_module_init _TME_P((void));
static _tme_inline int tme_module_init _TME_P((void)) {
  int rc;

  _tme_module_init();
  LTDL_SET_PRELOADED_SYMBOLS();
  rc = lt_dlinit();
  if (rc != 0) {
    return (-1);
  }
  return (TME_OK);
}

int tme_module_open _TME_P((_tme_const char *, void **, char **));
void *tme_module_symbol _TME_P((void *, _tme_const char *));
int tme_module_close _TME_P((void *));

#endif /* !_TME_MODULE_H */
