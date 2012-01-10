/* $Id: hash.h,v 1.2 2007/02/15 01:25:29 fredette Exp $ */

/* tme/hash.h - public header file for hashes: */

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

#ifndef _TME_HASH_H
#define _TME_HASH_H

#include <tme/common.h>
_TME_RCSID("$Id: hash.h,v 1.2 2007/02/15 01:25:29 fredette Exp $");

/* macros: */

/* the NULL hash data type value: */
#define TME_HASH_DATA_NULL	(NULL)

/* these convert between the hash data type and various integral
   types: */
/* NB: these assume that an unsigned long can be cast to void *, and
   vice versa: */
#define tme_hash_data_from_ulong(x)	((tme_hash_data_t) (x))
#define tme_hash_data_from_long(x)	tme_hash_data_from_ulong((unsigned long) (x))
#define tme_hash_data_from_uint32(x)	tme_hash_data_from_ulong((unsigned long) (x))
#define tme_hash_data_from_int32(x)	tme_hash_data_from_uint32((tme_uint32_t) (x))
#define tme_hash_data_to_ulong(x)	((unsigned long) (x))
#define tme_hash_data_to_long(x)	((long) tme_hash_data_to_ulong(x))
#define tme_hash_data_to_uint32(x)	((tme_uint32_t) tme_hash_data_to_ulong(x))
#define tme_hash_data_to_int32(x)	((tme_int32_t) tme_hash_data_to_uint32(x))

/* types: */

/* the data type used for hash keys and values: */
typedef void *tme_hash_data_t;

/* a hash function: */
typedef unsigned long (*tme_hash_func_t) _TME_P((tme_hash_data_t));

/* a compare function: */
typedef int (*tme_compare_func_t) _TME_P((tme_hash_data_t, tme_hash_data_t));

/* a foreach function: */
typedef void (*tme_foreach_func_t) _TME_P((tme_hash_data_t, tme_hash_data_t, void *));

/* a foreach remove function: */
typedef int (*tme_foreach_remove_func_t) _TME_P((tme_hash_data_t, tme_hash_data_t, void *));

/* a hash bucket: */
struct _tme_hash_bucket {

  /* the next bucket in the chain: */
  struct _tme_hash_bucket *_tme_hash_bucket_next;

  /* the key and value for this bucket: */
  tme_hash_data_t _tme_hash_bucket_key;
  tme_hash_data_t _tme_hash_bucket_value;
};

/* a hash: */
typedef struct _tme_hash {
  
  /* the size of the hash: */
  unsigned long _tme_hash_size;

  /* the hash table: */
  struct _tme_hash_bucket **_tme_hash_table;

  /* the count of elements in the hash: */
  unsigned long _tme_hash_count;

  /* the hash function: */
  tme_hash_func_t _tme_hash_hash;

  /* the compare function: */
  tme_compare_func_t _tme_hash_compare;

  /* the NULL hash data type: */
  tme_hash_data_t _tme_hash_null;

} *tme_hash_t;

/* prototypes: */
tme_hash_t tme_hash_new _TME_P((tme_hash_func_t, tme_compare_func_t, tme_hash_data_t));
void tme_hash_destroy _TME_P((tme_hash_t));
void tme_hash_insert _TME_P((tme_hash_t, tme_hash_data_t, tme_hash_data_t));
tme_hash_data_t tme_hash_lookup _TME_P((tme_hash_t, tme_hash_data_t));
void tme_hash_remove _TME_P((tme_hash_t, tme_hash_data_t));
void tme_hash_foreach _TME_P((tme_hash_t, tme_foreach_func_t, void *));
unsigned long tme_hash_foreach_remove _TME_P((tme_hash_t, tme_foreach_remove_func_t, void *));

unsigned long tme_direct_hash _TME_P((tme_hash_data_t));
int tme_direct_compare _TME_P((tme_hash_data_t, tme_hash_data_t));

unsigned long tme_string_hash _TME_P((tme_hash_data_t));
int tme_string_compare _TME_P((tme_hash_data_t, tme_hash_data_t));

#endif /* !_TME_HASH_H */
