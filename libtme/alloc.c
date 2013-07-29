/* $Id: alloc.c,v 1.1 2003/05/16 21:48:12 fredette Exp $ */

/* libtme/alloc.c - memory allocation utility functions: */

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

#include <tme/common.h>
_TME_RCSID("$Id: alloc.c,v 1.1 2003/05/16 21:48:12 fredette Exp $");

/* includes: */
#include <stdlib.h>

void *
tme_malloc(unsigned int size)
{
  void *p;
  p = malloc(size);
  if (p == NULL) {
    abort();
  }
  return (p);
}

void *
tme_malloc0(unsigned int size)
{
  void *p;
  p = tme_malloc(size);
  memset(p, 0, size);
  return (p);
}

void *
tme_realloc(void *p, unsigned int size)
{
  p = realloc(p, size);
  if (p == NULL) {
    abort();
  }
  return (p);
}

void *
tme_memdup(const void *p1, unsigned int size)
{
  void *p2;
  p2 = tme_malloc(size);
  memcpy(p2, p1, size);
  return (p2);
}

void
tme_free(void *p)
{
  free(p);
}

char *
tme_strdup(const char *s)
{
  return (tme_memdup(s, strlen(s) + 1));
}
