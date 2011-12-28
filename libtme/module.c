/* $Id: module.c,v 1.10 2010/06/05 19:04:42 fredette Exp $ */

/* libtme/module.c - module management: */

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
_TME_RCSID("$Id: module.c,v 1.10 2010/06/05 19:04:42 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <tme/module.h>
#include <tme/log.h>
#include <tme/misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ltdl.h>
#include <shlibvar.h>

/* the libtool 1.5 used in tme development is supposed to add code to
   configure that decides whether to use an already-installed libltdl
   instead of the libltdl that comes with tme.  unfortunately, on some
   systems it appears to decide to use an installed libltdl without
   checking that the installed ltdl.h is recent enough to define
   lt_ptr.  we try to compensate for this here: */
#ifndef lt_ptr
#ifdef lt_ptr_t
#define lt_ptr lt_ptr_t
#else  /* !lt_ptr_t */
#error "installed libtool is too old"
#endif /* !lt_ptr_t */
#endif /* !lt_ptr */

/* similarly, the installed libltdl may be so recent that its ltdl.h
   renames lt_preloaded_symbols with a macro, to a name different from
   what our libtool script makes.  it's possible that the renaming
   macro is meant to be undefined to handle this problem: */
#undef lt_preloaded_symbols

/* types: */
struct tme_module {

  /* the next module on a list: */
  struct tme_module *tme_module_next;

  /* the libltdl handle for this module: */
  lt_dlhandle tme_module_dlhandle;

  /* any "submodule" symbol prefix: */
  char *tme_module_submodule;
};

/* globals: */
static tme_mutex_t _tme_module_mutex;

/* this initializes modules: */
int
tme_module_init(void)
{
  int rc;
  tme_mutex_init(&_tme_module_mutex);
  LTDL_SET_PRELOADED_SYMBOLS();
  rc = lt_dlinit();
  if (rc != 0) {
    return (EINVAL);
  }
  return (TME_OK);
}

/* this finds a modules directory: */
static FILE *
_tme_modules_find(const char *top_name,
		  unsigned int top_name_length,
		  char **_modules_dir)
{
  unsigned int modules_dir_length;
  int pass;
  const char *search_path;
  const char *p1, *p2, *p3;
  char c;
  char *modules_index_pathname;
  FILE *modules_index;

  /* pass over the search path environment variables: */
  for (pass = 0; ++pass <= 2; ) {

    /* get the next search path environment variable value: */
    search_path = NULL;
    switch (pass) {
    case 1: search_path = getenv("LTDL_LIBRARY_PATH"); break;
    case 2: search_path = getenv(LTDL_SHLIBPATH_VAR); break;
    default: assert(FALSE);
    }
    if (search_path == NULL) {
      continue;
    }

    /* take apart this module path: */
    p1 = p2 = search_path;
    for (p3 = search_path;; p3++) {

      /* get the next character: */
      c = *p3;

      /* continue if this is not a delimiter, tracking the last
         non-slash: */
      if (c != ':'
	  && c != '\0') {
	if (c != '/') {
	  p2 = p3;
	}
	continue;
      }

      /* if this path is absolute: */
      if (*p1 == '/') {

	/* form the modules index pathname to try, remembering what
	   part of it is the modules directory pathname: */
	modules_dir_length = 
	  /* the search path part, less any trailing slashes: */
	  (p2 - p1) + 1
	  /* a slash: */
	  + 1
	  + top_name_length
	  /* a slash: */
	  + 1;
	modules_index_pathname =
	  tme_new(char,
		  modules_dir_length
		  + top_name_length
		  + strlen("-plugins.txt")
		  /* a NUL: */
		  + 1);
	memcpy(modules_index_pathname, p1, (p2 - p1) + 1);
	modules_index_pathname[(p2 - p1) + 1] = '/';
	memcpy(modules_index_pathname
	       + (p2 - p1) + 1
	       + 1,
	       top_name,
	       top_name_length);
	modules_index_pathname[((p2 - p1) + 1
				+ 1
				+ top_name_length)] = '/';
	memcpy(modules_index_pathname
	       + modules_dir_length,
	       top_name,
	       top_name_length);
	strcpy(modules_index_pathname
	       + modules_dir_length
	       + top_name_length,
	       "-plugins.txt");
	
	/* try to open the modules index: */
	modules_index = fopen(modules_index_pathname, "r");

	/* if we opened it, we're done: */
	if (modules_index != NULL) {
	  modules_index_pathname[modules_dir_length] = '\0';
	  *_modules_dir = modules_index_pathname;
	  return (modules_index);
	}

	/* keep trying: */
	tme_free(modules_index_pathname);
      }

      /* stop if this was the last path: */
      if (c == '\0') {
	break;
      }

      /* advance to the next path: */
      p1 = p2 = p3 + 1;
    }
  }

  /* this search failed: */
  return (NULL);
}

/* this opens a module: */
int
tme_module_open(const char *module_fake_pathname, void **_module, char **_output)
{
  char *module_raw_name;
  char *p1, c, *first_slash;
  FILE *modules_index;
  char *modules_dir;
  char line_buffer[1024];
  char **tokens;
  int tokens_count;
  char *module_basename;
  char *module_pathname;
  lt_dlhandle handle;
  struct tme_module *module;
  
  /* strip leading slashes from the fake module pathname: */
  for (; *module_fake_pathname == '/'; module_fake_pathname++);

  /* turn all of the non-alphanumerics in the fake module pathname
     into underscores, and remember where the first slash was: */
  module_raw_name = tme_strdup(module_fake_pathname);
  first_slash = NULL;
  for (p1 = module_raw_name;
       (c = *p1) != '\0';
       p1++) {
    if (!isalnum((unsigned char) c)) {
      *p1 = '_';
      if (c == '/'
	  && first_slash == NULL) {
	first_slash = p1;
      }
    }
  }

  /* if there were no slashes in the fake module pathname, there is no
     top name, which is incorrect: */
  if (first_slash == NULL) {
    tme_output_append_error(_output, module_fake_pathname);
    tme_free(module_raw_name);
    return (EINVAL);
  }

  /* open the modules index for this top name: */
  modules_index = _tme_modules_find(module_raw_name, 
				    (first_slash - module_raw_name),
				    &modules_dir);
  if (modules_index == NULL) {
    tme_output_append_error(_output, module_fake_pathname);
    tme_free(module_raw_name);
    return (ENOENT);
  }

  /* find the requested module in the index: */
  tokens = NULL;
  for (;;) {

    /* get the next line from the index: */
    tokens_count = 0;
    if (fgets(line_buffer, sizeof(line_buffer) - 1, modules_index) == NULL) {
      break;
    }
    line_buffer[sizeof(line_buffer) - 1] = '\0';
    if ((p1 = strchr(line_buffer, '\n')) != NULL) {
      *p1 = '\0';
    }

    /* tokenize this line: */
    tokens = tme_misc_tokenize(line_buffer, '#', &tokens_count);

    /* there must be either one or three tokens, and the first
       token must match the raw module name: */
    if ((tokens_count == 1
	 || tokens_count == 3)
	&& !strcmp(tokens[0], module_raw_name)) {
      break;
    }

    /* free the bad tokens: */
    tme_free_string_array(tokens, -1);
  }

  /* close the index: */
  fclose(modules_index);

  /* we no longer need the module raw name: */
  tme_free(module_raw_name);

  /* if we didn't find the module in the index: */
  if (tokens_count == 0) {
    tme_output_append_error(_output, module_fake_pathname);
    tme_free(modules_dir);
    return (ENOENT);
  }

  /* if there are three tokens, the module basename is the second,
     else it is the same as the raw module name: */
  module_basename = (tokens_count == 3
		     ? tokens[1]
		     : tokens[0]);
  
  /* form the real module pathname: */
  module_pathname = tme_renew(char,
			      modules_dir,
			      strlen(modules_dir)
			      + strlen(module_basename)
			      + 1);
  strcat(module_pathname, module_basename);
  
  /* dlopen the module: */
  tme_mutex_lock(&_tme_module_mutex);
  handle = lt_dlopenext(module_pathname);
  tme_mutex_unlock(&_tme_module_mutex);
  tme_free(module_pathname);
  if (handle == NULL) {
    tme_output_append_error(_output, module_fake_pathname);
    tme_free_string_array(tokens, -1);
    return (ENOENT);
  }

  /* return the new module: */
  module = tme_new(struct tme_module, 1);
  module->tme_module_dlhandle = handle;
  module->tme_module_submodule = (tokens_count == 3
				  ? tme_strdup(tokens[2])
				  : NULL);
  *_module = module;
  tme_free_string_array(tokens, -1);
  return (TME_OK);
}

/* this looks up a symbol: */
void *
tme_module_symbol(void *_module, const char *symbol)
{
  struct tme_module *module;
  char *module_symbol;
  lt_ptr address;

  /* recover the module: */
  module = (struct tme_module *) _module;

  /* form the symbol to look up: */
  if (module->tme_module_submodule == NULL) {
    module_symbol = tme_strdup(symbol);
  }
  else {
    module_symbol = tme_new(char, 
			    strlen(module->tme_module_submodule)
			    /* an underscore: */
			    + 1
			    + strlen(symbol)
			    /* a NUL: */
			    + 1);
    sprintf(module_symbol,
	    "%s_%s", 
	    module->tme_module_submodule,
	    symbol);
  }
    
  /* look up the symbol: */
  tme_mutex_lock(&_tme_module_mutex);
  address = lt_dlsym(module->tme_module_dlhandle, module_symbol);
  tme_mutex_unlock(&_tme_module_mutex);
  tme_free(module_symbol);
  return (address);
}

/* this immediately closes a module: */
int
tme_module_close(void *_module)
{
  struct tme_module *module;
  int rc;

  /* recover the module: */
  module = (struct tme_module *) _module;

  /* close the module: */
  tme_mutex_lock(&_tme_module_mutex);
  rc = lt_dlclose(module->tme_module_dlhandle);
  tme_mutex_unlock(&_tme_module_mutex);

  /* free our structure: */
  if (module->tme_module_submodule != NULL) {
    tme_free(module->tme_module_submodule);
  }
  tme_free(module);

  /* XXX assume success: */
  return (TME_OK);
}
