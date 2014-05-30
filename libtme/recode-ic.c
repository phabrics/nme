/* $Id: recode-ic.c,v 1.3 2010/02/07 14:33:47 fredette Exp $ */

/* libtme/recode-ic.c - generic support for recode ICs: */

/*
 * Copyright (c) 2008 Matt Fredette
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
_TME_RCSID("$Id: recode-ic.c,v 1.3 2010/02/07 14:33:47 fredette Exp $");

#if TME_HAVE_RECODE

/* includes: */
#include "recode-impl.h"

/* this initializes a new recode ic: */
void
tme_recode_ic_new(struct tme_recode_ic *ic,
		  tme_recode_thunk_off_t size_run)
{

  /* initialize some pointers to NULL: */
  ic->tme_recode_ic_flags_groups = NULL;
  ic->tme_recode_ic_conds_groups = NULL;

  /* initialize the dummy guest register information for the
     non-register source and destination operands: */
  (ic->tme_recode_ic_reginfo + TME_RECODE_OPERAND_ZERO)->tme_recode_reginfo_all
    = (TME_RECODE_REGINFO_TYPE_FIXED
       | !TME_RECODE_REGINFO_TAGS_VALID);
  (ic->tme_recode_ic_reginfo + TME_RECODE_OPERAND_IMM)->tme_recode_reginfo_all
    = (TME_RECODE_REGINFO_TYPE_FIXED
       | !TME_RECODE_REGINFO_TAGS_VALID);

  /* allocate the memory for building and running thunks: */
  tme_recode_host_thunks_alloc(ic, size_run);

  /* call the host function: */
  tme_recode_host_ic_new(ic);

  /* set the initial thunk offset of the first variable thunk: */
  ic->tme_recode_ic_thunk_off_variable
    = tme_recode_build_to_thunk_off(ic, ic->tme_recode_ic_thunk_build_next);
}

#ifdef TME_RECODE_DEBUG

#include <stdio.h>

/* this returns an opcode name: */
const char *
tme_recode_opcode_dump(unsigned int opcode)
{
  switch (opcode) {
  case TME_RECODE_OPCODE_ANDN: return ("andn");
  case TME_RECODE_OPCODE_ORN: return ("orn");
  case TME_RECODE_OPCODE_XORN: return ("xorn");
  case TME_RECODE_OPCODE_SUB: return ("sub");
  case TME_RECODE_OPCODE_SUBC: return ("subc");
  case TME_RECODE_OPCODE_AND: return ("and");
  case TME_RECODE_OPCODE_OR: return ("or");
  case TME_RECODE_OPCODE_XOR: return ("xor");
  case TME_RECODE_OPCODE_ADD: return ("add");
  case TME_RECODE_OPCODE_ADDC: return ("addc");
  case TME_RECODE_OPCODE_SHLL: return ("shll");
  case TME_RECODE_OPCODE_SHRL: return ("shrl");
  case TME_RECODE_OPCODE_SHRA: return ("shra");
  case TME_RECODE_OPCODE_EXTZ: return ("extz");
  case TME_RECODE_OPCODE_EXTS: return ("exts");
  case TME_RECODE_OPCODE_GUEST: return ("guest");
  case TME_RECODE_OPCODE_DEFC: return ("defc");
  case TME_RECODE_OPCODE_IF: return ("if");
  case TME_RECODE_OPCODE_ELSE: return ("else");
  case TME_RECODE_OPCODE_ENDIF: return ("endif");
  default: return ("?opcode?");
  }
}


/* this dumps a tme_recode_uguest_t: */
void
tme_recode_uguest_dump(tme_recode_uguest_t val)
{
  unsigned int resid;

  printf("0x");
  resid = sizeof(val);
  do {
    resid -= sizeof(unsigned int);
    printf("%0*x",
	   (int) (sizeof(unsigned int) * 2),
	   (unsigned int) (val >> (resid * 8)));
  } while (resid > 0);
}

#endif /* TME_RECODE_DEBUG */

#endif /* TME_HAVE_RECODE */
