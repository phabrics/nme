/* $Id: tmesh-util.c,v 1.2 2003/10/25 17:08:02 fredette Exp $ */

/* tmesh/tmesh-util.c - the tme shell utility functions: */

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
_TME_RCSID("$Id: tmesh-util.c,v 1.2 2003/10/25 17:08:02 fredette Exp $");

/* includes: */
#include "tmesh-impl.h"

/* macros: */

/* this looks up a pathname: */
int
_tmesh_fs_lookup(struct tmesh *tmesh,
		 char **_pathname,
		 struct tmesh_fs_dirent **_dir,
		 struct tmesh_fs_dirent **_entry,
		 char **_output,
		 int flags)
{
  struct tmesh_fs_dirent *dir, *entry;
  const char *pathname, *part_start, *part_end;
  unsigned int part_length;
  char c;
  int rc;

  /* get the pathname: */
  pathname = *_pathname;
  *_pathname = NULL;

  /* initialize dir and entry.  if the pathname begins with a '/',
     start at the root directory, otherwise start in the current
     directory: */
  if (*pathname == '/') {
    dir = tmesh->tmesh_root;
    for (; *pathname == '/'; pathname++);
  }
  else {
    dir = tmesh->tmesh_cwd;
  }
  entry = (*pathname == '\0' ? dir : NULL);

  /* look up the parts of this pathname: */
  part_start = part_end = pathname;
  for (;;) {
    c = *part_end;

    /* forward slashes and NULs delimit pathname parts: */
    if (c == '/'
	|| c == '\0') {

      /* if this pathname part has some length, look it up: */
      if (part_end > part_start) {
	part_length = part_end - part_start;

	/* if there is a delayed descent, descend now: */
	if (entry != NULL) {
	  dir = entry;
	}

	/* check all entries in this directory.  remember that the
	   directory entries form a circular linked list: */
	entry = dir;
	for (;;) {
	  
	  /* check this entry.  if it matches, we're done: */
	  if (!memcmp(entry->tmesh_fs_dirent_name,
		      part_start,
		      part_length)
	      && entry->tmesh_fs_dirent_name[part_length] == '\0') {
	    break;
	  }

	  /* if this was the last entry in the directory, stop: */
	  entry = entry->tmesh_fs_dirent_next;
	  if (entry == dir) {
	    entry = NULL;
	    break;
	  }
	}
	
	/* if we couldn't find a matching entry: */
	if (entry == NULL) {

	  /* assume we're returning an error: */
	  rc = ENOENT;
	  
	  /* if this was the last pathname part, and failure on the
	     last pathname part is OK, don't return an error: */
	  if (c == '\0'
	      && (flags & TMESH_SEARCH_LAST_PART_OK)) {
	    *_pathname = (char *) part_start;
	    rc = TME_OK;
	  }

	  /* return what we know: */
	  *_dir = dir;
	  *_entry = entry;
	  if (rc != TME_OK) {
	    tme_output_append_raw(_output, part_start, part_length);
	  }
	  return (rc);
	}
      }

      /* we did find a matching entry: */
      assert(entry != NULL);

      /* if the delimiter is a NUL, return success: */
      if (c == '\0') {
	*_dir = dir;
	*_entry = entry;
	return (TME_OK);
      }

      /* otherwise, the delimiter must be a slash: */
      assert(c == '/');

      /* if the entry is not a directory, return an error: */
      if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_DIR) {
	*_dir = dir;
	*_entry = entry;
	return (ENOTDIR);
      }

      /* start on another part: */
      part_start = part_end = part_end + 1;
    }

    /* otherwise, this is not a delimiter, so just continue on: */
    else {
      part_end++;
    }
  }
  /* NOTREACHED */
}

/* this creates a new directory entry and links it into a directory: */
struct tmesh_fs_dirent *
_tmesh_fs_link(struct tmesh_fs_dirent *parent, char *name, int type, void *value)
{
  struct tmesh_fs_dirent *entry;
  assert(parent->tmesh_fs_dirent_type == TMESH_FS_DIRENT_DIR);

  /* create the new entry: */
  entry = tme_new(struct tmesh_fs_dirent, 1);
  entry->tmesh_fs_dirent_type = type;
  entry->tmesh_fs_dirent_name = name;
  entry->tmesh_fs_dirent_value = value;

  /* link in the new entry: */
  entry->tmesh_fs_dirent_next = parent;
  entry->tmesh_fs_dirent_prev = parent->tmesh_fs_dirent_prev;
  entry->tmesh_fs_dirent_next->tmesh_fs_dirent_prev = &entry->tmesh_fs_dirent_next;
  *entry->tmesh_fs_dirent_prev = entry;

  return (entry);
}

/* this unlinks a directory entry: */
void
_tmesh_fs_unlink(struct tmesh_fs_dirent *entry)
{
  *entry->tmesh_fs_dirent_prev = entry->tmesh_fs_dirent_next;
  entry->tmesh_fs_dirent_next->tmesh_fs_dirent_prev = entry->tmesh_fs_dirent_prev;
}

/* this creates a new directory: */
struct tmesh_fs_dirent *
_tmesh_fs_mkdir(struct tmesh_fs_dirent *parent, char *name)
{
  struct tmesh_fs_dirent *dir;

  /* create the "." directory entry: */
  dir = tme_new(struct tmesh_fs_dirent, 1);
  dir->tmesh_fs_dirent_next = dir;
  dir->tmesh_fs_dirent_prev = &dir->tmesh_fs_dirent_next;
  dir->tmesh_fs_dirent_type = TMESH_FS_DIRENT_DIR;
  dir->tmesh_fs_dirent_name = tme_strdup(".");
  dir->tmesh_fs_dirent_value = dir;

  /* if we have a parent directory, create our entry in that directory,
     otherwise, this is the root directory, so we're our own parent: */
  if (parent != NULL) {
    _tmesh_fs_link(parent, name, TMESH_FS_DIRENT_DIR, dir);
  }
  else {
    parent = dir;
  }

  /* make the ".." directory entry: */
  _tmesh_fs_link(dir, tme_strdup(".."), TMESH_FS_DIRENT_DIR, parent);

  /* return the new directory: */
  return (dir);
}

/* this recursively recovers a pathname: */
static unsigned int
_tmesh_fs_pathname(struct tmesh_fs_dirent *parent, int type, void *value,
		   char **_output,
		   struct tmesh_fs_dirent *parent_stop)
{
  struct tmesh_fs_dirent *entry_2dots, *parent_parent, *entry;
  unsigned int pathname_length;

  /* get this parent's parent directory: */
  entry_2dots = parent->tmesh_fs_dirent_next;
  assert(parent->tmesh_fs_dirent_type == TMESH_FS_DIRENT_DIR);
  assert(!strcmp(parent->tmesh_fs_dirent_name, "."));
  assert(entry_2dots->tmesh_fs_dirent_type == TMESH_FS_DIRENT_DIR);
  assert(!strcmp(entry_2dots->tmesh_fs_dirent_name, ".."));
  parent_parent = entry_2dots->tmesh_fs_dirent_value;

  /* if we've reached a common parent directory, this is a base case: */
  if (parent == parent_stop) {
    pathname_length = 0;
  }

  /* otherwise, if this parent is its own parent, this parent is the
     root directory, which is always a base case: */
  else if (parent_parent == parent) {
    tme_output_append(_output, "/");
    pathname_length = 1;
    
    /* if the value we're looking for also happens to be the root
       directory, we have nothing more to do: */
    if (value == parent) {
      assert(type == TMESH_FS_DIRENT_DIR);
      return (pathname_length);
    }
  }

  /* otherwise, recurse into the parent parent: */
  else {
    pathname_length = 
      _tmesh_fs_pathname(parent_parent, TMESH_FS_DIRENT_DIR, parent,
			 _output, parent_stop);
  }

  /* find the entry in the parent directory pointing to this value.
     remember that the directory entries form a circular linked list: */
  entry = parent;
  for (;;) {
    
    /* check this entry.  if it matches, we're done: */
    if (entry->tmesh_fs_dirent_value == value) {
      assert(entry->tmesh_fs_dirent_type == type);
      break;
    }
    
    /* if this was the last entry in the directory, the system
       is inconsistent: */
    entry = entry->tmesh_fs_dirent_next;
    assert(entry != parent);
  }
  
  /* add in this name: */
  if (pathname_length > 1) {
    tme_output_append(_output, "/");
    pathname_length++;
  }
  tme_output_append(_output, "%s", entry->tmesh_fs_dirent_name);
  return (pathname_length + strlen(entry->tmesh_fs_dirent_name));
}

/* this recovers the pathname of a directory: */
void
_tmesh_fs_pathname_dir(struct tmesh_fs_dirent *dir,
		       char **_output,
		       struct tmesh_fs_dirent *parent_stop)
{
  struct tmesh_fs_dirent *entry_2dots, *parent;
  
  /* get this directory's parent directory: */
  entry_2dots = dir->tmesh_fs_dirent_next;
  assert(dir->tmesh_fs_dirent_type == TMESH_FS_DIRENT_DIR);
  assert(!strcmp(dir->tmesh_fs_dirent_name, "."));
  assert(entry_2dots->tmesh_fs_dirent_type == TMESH_FS_DIRENT_DIR);
  assert(!strcmp(entry_2dots->tmesh_fs_dirent_name, ".."));
  parent = entry_2dots->tmesh_fs_dirent_value;
  
  /* output this directory's pathname: */
  _tmesh_fs_pathname(parent, TMESH_FS_DIRENT_DIR, dir, 
		     _output, parent_stop);
}

/* this recovers the pathname of an element: */
void
_tmesh_fs_pathname_element(struct tmesh_fs_element *element,
			   char **_output,
			   struct tmesh_fs_dirent *parent_stop)
{
  _tmesh_fs_pathname(element->tmesh_fs_element_parent,
		     TMESH_FS_DIRENT_ELEMENT, element,
		     _output, parent_stop);
}

/* this allocates garbage-collectable memory: */
void *
_tmesh_gc_malloc(struct tmesh *tmesh, unsigned int size)
{
  struct tmesh_gc_record *mem;
  
  /* allocate the memory: */
  mem = tme_new(struct tmesh_gc_record, 1);
  mem->tmesh_gc_record_mem = tme_malloc(size);
  
  /* add this memory to our list: */
  mem->tmesh_gc_record_next = tmesh->tmesh_gc_record;
  if (mem->tmesh_gc_record_next != NULL) {
    mem->tmesh_gc_record_next->tmesh_gc_record_prev = &mem->tmesh_gc_record_next;
  }
  mem->tmesh_gc_record_prev = &tmesh->tmesh_gc_record;
  tmesh->tmesh_gc_record = mem;

  /* return the real buffer: */
  return (mem->tmesh_gc_record_mem);
}

/* this reallocates garbage-collectable memory: */
void *
_tmesh_gc_realloc(struct tmesh *tmesh, void *p, unsigned int size)
{
  struct tmesh_gc_record *mem;

  /* recover the memory structure: */
  for (mem = tmesh->tmesh_gc_record;
       mem != NULL;
       mem = mem->tmesh_gc_record_next) {
    if (mem->tmesh_gc_record_mem == p) {
      break;
    }
  }
  assert(mem != NULL);

  /* reallocate the memory: */
  mem->tmesh_gc_record_mem = tme_realloc(p, size);

  /* return the real buffer: */
  return (mem->tmesh_gc_record_mem);
}

/* this frees or releases garbage-collectable memory: */
static void
__tmesh_gc_free(struct tmesh *tmesh, void *p, int release)
{
  struct tmesh_gc_record *mem;

  /* recover the memory structure: */
  for (mem = tmesh->tmesh_gc_record;
       mem != NULL;
       mem = mem->tmesh_gc_record_next) {
    if (mem->tmesh_gc_record_mem == p) {
      break;
    }
  }
  assert(mem != NULL);

  /* remove this memory from our list: */
  *mem->tmesh_gc_record_prev = mem->tmesh_gc_record_next;
  if (mem->tmesh_gc_record_next != NULL) {
    mem->tmesh_gc_record_next->tmesh_gc_record_prev = mem->tmesh_gc_record_prev;
  }

  /* free this memory if we're not releasing it: */
  if (!release) {
    tme_free(mem->tmesh_gc_record_mem);
  }
  tme_free(mem);
}

/* this frees garbage-collectable memory: */
void
_tmesh_gc_free(struct tmesh *tmesh, void *p)
{
  __tmesh_gc_free(tmesh, p, FALSE);
}

/* this releases garbage-collectable memory: */
void
_tmesh_gc_release(struct tmesh *tmesh, void *p)
{
  __tmesh_gc_free(tmesh, p, TRUE);
}

/* this releases an argv: */
void
_tmesh_gc_release_argv(struct tmesh *tmesh, struct tmesh_parser_argv *argv)
{
  unsigned int argc;
  char **args;
  argc = argv->tmesh_parser_argv_argc;
  args = argv->tmesh_parser_argv_argv;
  assert(argc > 0);
  _tmesh_gc_release(tmesh, args);
  for (; argc-- > 0; ) {
    _tmesh_gc_release(tmesh, *(args++));
  }
}

/* this garbage-collects: */
void
_tmesh_gc_gc(struct tmesh *tmesh)
{
  struct tmesh_gc_record *mem, *mem_next;

  /* while we have memory to collect: */
  for (mem = tmesh->tmesh_gc_record;
       mem != NULL;
       mem = mem_next) {
    mem_next = mem->tmesh_gc_record_next;
    tme_free(mem->tmesh_gc_record_mem);
    tme_free(mem);
  }

  /* we don't have any memory allocated: */
  tmesh->tmesh_gc_record = NULL;
}
