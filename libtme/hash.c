/* $Id: hash.c,v 1.2 2003/09/01 14:24:08 fredette Exp $ */

/* libtme/hash.c - hash table support: */

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
_TME_RCSID("$Id: hash.c,v 1.2 2003/09/01 14:24:08 fredette Exp $");

/* includes: */
#include <tme/hash.h>

/* our hash table size array: */
static unsigned long _tme_hash_sizes[] = {
  2,
  3,
  5,
  7,
  11,
  17,
  37,
  83,
  281,
  421,
  631,
  947,
  2131,
  7193,
  10789,
  16183,
  81929,
  414763,
  933217,
  10629917,
  35875969,
  80720929,
};

/* this allocates and returns a new hash: */
tme_hash_t
tme_hash_new(tme_hash_func_t hash_func,
	     tme_compare_func_t compare_func,
	     tme_hash_data_t value_null)
{
  tme_hash_t hash;

  hash = tme_new0(struct _tme_hash, 1);
  hash->_tme_hash_size = _tme_hash_sizes[0];
  hash->_tme_hash_table = 
    tme_new0(struct _tme_hash_bucket *,
	     hash->_tme_hash_size);
  hash->_tme_hash_count = 0;
  hash->_tme_hash_hash = hash_func;
  hash->_tme_hash_compare = compare_func;
  hash->_tme_hash_null = value_null;
  return (hash);
}

/* this destroys a hash: */
void
tme_hash_destroy(tme_hash_t hash)
{
  struct _tme_hash_bucket *bucket, *bucket_next;
  unsigned long bucket_i;

  /* free all of the buckets in the hash table: */
  for (bucket_i = 0;
       bucket_i < hash->_tme_hash_size;
       bucket_i++) {
    for (bucket = hash->_tme_hash_table[bucket_i];
	 bucket != NULL;
	 bucket = bucket_next) {
      bucket_next = bucket->_tme_hash_bucket_next;
      tme_free(bucket);
    }
  }
  tme_free(hash->_tme_hash_table);
  tme_free(hash);
}

/* this does an internal lookup in a hash table: */
static struct _tme_hash_bucket *
_tme_hash_lookup_internal(tme_hash_t hash,
			  tme_hash_data_t key,
			  struct _tme_hash_bucket ***__bucket)
{
  unsigned long bucket_i;
  struct _tme_hash_bucket **_bucket, *bucket;
  
  /* hash the key: */
  bucket_i = (*hash->_tme_hash_hash)(key) % hash->_tme_hash_size;

  /* walk the chain of buckets: */
  for (_bucket = hash->_tme_hash_table + bucket_i;
       (bucket = *_bucket) != NULL;
       _bucket = &bucket->_tme_hash_bucket_next) {
    
    /* compare the key in this bucket with the lookup key.
       if it succeeds, return the bucket: */
    if ((*hash->_tme_hash_compare)(key, bucket->_tme_hash_bucket_key)) {
      if (__bucket != NULL) {
	*__bucket = _bucket;
      }
      return (bucket);
    }
  }

  /* the lookup failed.  return where the bucket might be inserted: */
  if (__bucket != NULL) {
    *__bucket = _bucket;
  }
  return (NULL);
}

/* this inserts a value into a hash table: */
void
tme_hash_insert(tme_hash_t hash,
		tme_hash_data_t key,
		tme_hash_data_t value)
{
  struct _tme_hash_bucket *bucket, *bucket_next, **_bucket;
  struct _tme_hash hash_new;
  int size_i;
  unsigned long bucket_i;

  /* if this key is not already present in the hash table: */
  bucket = _tme_hash_lookup_internal(hash, key, &_bucket);
  if (bucket == NULL) {

    /* if we need to resize this hash table: */
    if ((hash->_tme_hash_count * 2) > hash->_tme_hash_size) {

      /* make a copy of the top of the hash: */
      hash_new = *hash;

      /* set the new size of the hash: */
      hash_new._tme_hash_size = hash->_tme_hash_count * 2;
      for (size_i = 0;
	   _tme_hash_sizes[size_i] < hash_new._tme_hash_size;
	   size_i++) {
	if (size_i + 1 == TME_ARRAY_ELS(_tme_hash_sizes)) {
	  abort();
	}
      }
      hash_new._tme_hash_size = _tme_hash_sizes[size_i];

      /* allocate the new hash table: */
      hash_new._tme_hash_table = 
	tme_new0(struct _tme_hash_bucket *,
		 hash_new._tme_hash_size);

      /* move everything from the old hash table into the new: */
      for (bucket_i = 0;
	   bucket_i < hash->_tme_hash_size;
	   bucket_i++) {
	for (bucket = hash->_tme_hash_table[bucket_i];
	     bucket != NULL;
	     bucket = bucket_next) {
	  bucket_next = bucket->_tme_hash_bucket_next;
	  _tme_hash_lookup_internal(&hash_new,
				    bucket->_tme_hash_bucket_key,
				    &_bucket);
	  bucket->_tme_hash_bucket_next = *_bucket;
	  *_bucket = bucket;
	}
      }

      /* free the old hash table: */
      tme_free(hash->_tme_hash_table);

      /* set the new top of the hash: */
      *hash = hash_new;
    
      /* do the internal lookup again: */
      _tme_hash_lookup_internal(hash, key, &_bucket);
    }

    /* create the new bucket and link it in: */
    bucket = tme_new(struct _tme_hash_bucket, 1);
    bucket->_tme_hash_bucket_next = *_bucket;
    *_bucket = bucket;

    /* increment the number of keys in the hash: */
    hash->_tme_hash_count++;
  }

  /* set the key and value in the bucket: */
  bucket->_tme_hash_bucket_key = key;
  bucket->_tme_hash_bucket_value = value;
}

/* this looks up a key in the hash: */
tme_hash_data_t
tme_hash_lookup(tme_hash_t hash,
		tme_hash_data_t key)
{
  struct _tme_hash_bucket *bucket;

  bucket = _tme_hash_lookup_internal(hash, key, NULL);
  return (bucket != NULL
	  ? bucket->_tme_hash_bucket_value
	  : hash->_tme_hash_null);
}

/* this removes a key in the hash: */
void
tme_hash_remove(tme_hash_t hash,
		tme_hash_data_t key)
{
  struct _tme_hash_bucket *bucket, **_bucket;

  bucket = _tme_hash_lookup_internal(hash, key, &_bucket);
  if (bucket != NULL) {
    *_bucket = bucket->_tme_hash_bucket_next;
    tme_free(bucket);
    hash->_tme_hash_count--;
  }
}

/* this calls a function for each key and value in the hash: */
void
tme_hash_foreach(tme_hash_t hash,
		 tme_foreach_func_t func,
		 void *private)
{
  struct _tme_hash_bucket *bucket;
  unsigned long bucket_i;

  /* walk all of the buckets in the hash table: */
  for (bucket_i = 0;
       bucket_i < hash->_tme_hash_size;
       bucket_i++) {
    for (bucket = hash->_tme_hash_table[bucket_i];
	 bucket != NULL;
	 bucket = bucket->_tme_hash_bucket_next) {
      (*func)(bucket->_tme_hash_bucket_key,
	      bucket->_tme_hash_bucket_value,
	      private);
    }
  }
}

/* this calls a function for each key and value in the hash.
   if the function returns TRUE the entry is removed: */
unsigned long
tme_hash_foreach_remove(tme_hash_t hash,
			tme_foreach_remove_func_t func,
			void *private)
{
  struct _tme_hash_bucket **_bucket, *bucket;
  unsigned long bucket_i, count;

  /* walk all of the buckets in the hash table: */
  count = 0;
  for (bucket_i = 0;
       bucket_i < hash->_tme_hash_size;
       bucket_i++) {
    for (_bucket = &hash->_tme_hash_table[bucket_i];
	 (bucket = *_bucket) != NULL; ) {
      if ((*func)(bucket->_tme_hash_bucket_key,
		  bucket->_tme_hash_bucket_value,
		  private)) {
	*_bucket = bucket->_tme_hash_bucket_next;
	tme_free(bucket);
	hash->_tme_hash_count--;
	count++;
      }
      else {
	_bucket = &bucket->_tme_hash_bucket_next;
      }
    }
  }
  return (count);
}

/* this hashes a direct value: */
unsigned long
tme_direct_hash(tme_hash_data_t key)
{
  return ((unsigned long) key);
}

/* this compares two direct values: */
int
tme_direct_compare(tme_hash_data_t key0,
		   tme_hash_data_t key1)
{
  return (key0 == key1);
}

/* this hashes a string value: */
unsigned long
tme_string_hash(tme_hash_data_t key)
{
  /* this is cribbed from the Dragon book: */
  const char *p;
  char c;
  unsigned long h, g;

  p = (const char *) key;
  h = 0;

  for (; (c = *(p++)) != '\0'; ) {
    h = (h << 4) + c;
    g = (h & 0xf0000000);
    if (g) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return (h);
}

/* this compares two string values: */
int
tme_string_compare(tme_hash_data_t key0,
		   tme_hash_data_t key1)
{
  return (!strcmp((const char *) key0, (const char *) key1));
}

