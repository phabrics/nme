/* $Id: tmesh-cmds.c,v 1.6 2006/11/15 23:12:30 fredette Exp $ */

/* tmesh/tmesh-cmds.c - functions implementing the tmesh commands: */

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
_TME_RCSID("$Id: tmesh-cmds.c,v 1.6 2006/11/15 23:12:30 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <stdlib.h>
#include "tmesh-impl.h"

/* macros: */

/* flags for ls: */
#define TMESH_LS_NORMAL		(0)
#define TMESH_LS_ALL		TME_BIT(0)
#define TMESH_LS_RECURSE	TME_BIT(1)
#define TMESH_LS_ABSOLUTE	TME_BIT(2)

/* the "source" command: */
static int
_tmesh_command_source(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_io_stack *stack_new;
  struct tmesh_io *io_new;
  struct tmesh_io *io_old;
  int rc;
  
  /* allocate the new io stack member: */
  stack_new = tme_new(struct tmesh_io_stack, 1);
  io_new = &stack_new->tmesh_io_stack_io;

  /* initialize the new io: */
  io_new->tmesh_io_name = value->tmesh_parser_value_arg;
  io_new->tmesh_io_input_line = 0;

  /* call the current input source's open function: */
  io_old = &tmesh->tmesh_io_stack->tmesh_io_stack_io;
  rc = (*io_old->tmesh_io_open)(io_new, io_old, _output);

  /* if the open succeeded, push this new input source onto the stack,
     otherwise free it: */
  if (rc == TME_OK) {
    _tmesh_gc_release(tmesh, io_new->tmesh_io_name);
    stack_new->tmesh_io_stack_next = tmesh->tmesh_io_stack;
    tmesh->tmesh_io_stack = stack_new;
  }
  else {
    tme_free(stack_new);
  }

  /* done: */
  return (rc);
}

/* the "mkdir" command: */
static int
_tmesh_command_mkdir(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  char *dirname;
  int rc;

  /* look up the new directory name: */
  dirname = value->tmesh_parser_value_pathname0;
  rc = _tmesh_fs_lookup(tmesh,
			&dirname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_LAST_PART_OK);

  /* if the lookup succeeded: */
  if (rc == TME_OK) {

    /* if the new directory name already exists, we can't
       make it: */
    if (entry != NULL) {
      rc = EEXIST;
    }

    /* otherwise, make the new directory: */
    else {
      _tmesh_fs_mkdir(parent, tme_strdup(dirname));
    }
  }

  return (rc);
}

/* the "rmdir" command: */
static int
_tmesh_command_rmdir(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry, *dir;
  char *dirname;
  int rc;

  /* look up the directory: */
  dirname = value->tmesh_parser_value_pathname0;
  rc = _tmesh_fs_lookup(tmesh,
			&dirname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_NORMAL);

  /* if the lookup succeeded: */
  if (rc == TME_OK) {

    /* if this pathname doesn't refer to a directory, we can't
       remove it: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_DIR) {
      rc = ENOTDIR;
    }

    /* otherwise, this is a directory: */
    else {
      dir = entry->tmesh_fs_dirent_value;

      /* if the directory isn't empty, we can't remove it: */
      if (dir->tmesh_fs_dirent_prev
	  != &dir->tmesh_fs_dirent_next->tmesh_fs_dirent_next) {
	rc = ENOTEMPTY;
      }

      /* you can't remove the current working directory,
	 or the "." or ".." directories: */
      else if (dir == tmesh->tmesh_cwd
	       || !strcmp(entry->tmesh_fs_dirent_name, ".")
	       || !strcmp(entry->tmesh_fs_dirent_name, "..")) {
	rc = EACCES;
      }

      /* otherwise, remove the directory: */
      else {

	/* unlink the directory in the parent: */
	_tmesh_fs_unlink(entry);

	/* free the "." and ".." directory entries: */
	tme_free(entry->tmesh_fs_dirent_next->tmesh_fs_dirent_name);
	tme_free(entry->tmesh_fs_dirent_next);
	tme_free(entry->tmesh_fs_dirent_name);
	tme_free(entry);
      }
    }
  }

  return (rc);
}

/* the "cd" command: */
static int
_tmesh_command_cd(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  char *dirname;
  int rc;

  /* look up the directory: */
  dirname = value->tmesh_parser_value_pathname0;
  rc = _tmesh_fs_lookup(tmesh,
			&dirname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_NORMAL);

  /* if the lookup succeeded: */
  if (rc == TME_OK) {

    /* if this pathname doesn't refer to a directory, we can't
       cd to it: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_DIR) {
      rc = ENOTDIR;
    }

    /* otherwise, this is a directory: */
    else {

      /* this is the new current working directory: */
      tmesh->tmesh_cwd = entry->tmesh_fs_dirent_value;
    }
  }

  return (rc);
}

/* the "pwd" command: */
static int
_tmesh_command_pwd(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  _tmesh_fs_pathname_dir(tmesh->tmesh_cwd, _output, NULL);
  tme_output_append(_output, "\n");
  return (TME_OK);
}

/* this outputs an argv: */
static void
_tmesh_ls_output_argv(char **_output, struct tmesh_parser_argv *argv, unsigned int skip)
{
  unsigned int argc;
  char **args;
  argc = argv->tmesh_parser_argv_argc;
  args = argv->tmesh_parser_argv_argv;
  assert(argc > 0 && argc >= skip);
  argc -= skip;
  args += skip;
  for (; argc-- > 0; ) {
    tme_output_append(_output, " ");
    tme_output_append(_output, *(args++));
  }
}

/* this lists an element: */
static void
_tmesh_ls_element(struct tmesh_fs_element *element,
		  char **_output,
		  int flags)
{
  int output_element_argv;
  struct tmesh_fs_element *element_other;
  struct tmesh_fs_element_conn *conn, *conn_other;

  /* we haven't yet output the element's argv: */
  output_element_argv = FALSE;

  /* loop over the element's connections: */
  for (conn = element->tmesh_fs_element_conns;
       conn != NULL;
       conn = conn->tmesh_fs_element_conn_next) {

    /* if we're not showing all connections, and this connection was
       made after this element was created, skip it: */
    if (!(flags & TMESH_LS_ALL)
	&& (conn->tmesh_fs_element_conn_gen
	    > element->tmesh_fs_element_gen)) {
      continue;
    }

    /* output this element's name and connection argv: */
    _tmesh_fs_pathname_element(element, _output, 
			       ((flags & TMESH_LS_ABSOLUTE)
				? NULL
				: element->tmesh_fs_element_parent));
    _tmesh_ls_output_argv(_output, &conn->tmesh_fs_element_conn_argv, 1);

    /* get the other side of this connection: */
    conn_other = conn->tmesh_fs_element_conn_other;
    element_other = conn_other->tmesh_fs_element_conn_element;
    tme_output_append(_output, " at ");      

    /* output the other element's name and connection argv: */
    _tmesh_fs_pathname_element(element_other, _output, 
			       ((flags & TMESH_LS_ABSOLUTE)
				? NULL
				: element->tmesh_fs_element_parent));
    _tmesh_ls_output_argv(_output, &conn_other->tmesh_fs_element_conn_argv, 1);

    /* if we haven't output the element's creation argv yet, do so: */
    if (!output_element_argv) {
      tme_output_append(_output, ":");
      _tmesh_ls_output_argv(_output, &element->tmesh_fs_element_argv, 0);
      output_element_argv = TRUE;
    }

    /* output a newline: */
    tme_output_append(_output, "\n");
  }

  /* if we haven't output the element's creation argv yet, do so: */
  if (!output_element_argv) {
    _tmesh_fs_pathname_element(element, _output, 
			       ((flags & TMESH_LS_ABSOLUTE)
				? NULL
				: element->tmesh_fs_element_parent));
    tme_output_append(_output, ":");
    _tmesh_ls_output_argv(_output, &element->tmesh_fs_element_argv, 0);
    tme_output_append(_output, "\n");
  }    
}

/* this lists a directory: */
static void
_tmesh_ls_dir(struct tmesh_fs_dirent *parent,
	      char **_output,
	      struct tmesh_fs_dirent *parent_top,
	      int flags)
{
  struct tmesh_fs_dirent *entry, *dir;
  struct tmesh_fs_element *element;
  int pass;

  /* list this directory: */
  for (pass = 0; ++pass < 2;) {

    /* if this is pass two, but we're not recursing, stop: */
    if (pass == 2
	&& !(flags & TMESH_LS_RECURSE)) {
      return;
    }

    /* loop over the entries in the directory: */
    entry = parent;
    do {
      
      /* dispatch on the directory entry's type: */
      switch (entry->tmesh_fs_dirent_type) {

	/* this is a subdirectory: */
      case TMESH_FS_DIRENT_DIR:
	dir = entry->tmesh_fs_dirent_value;

	/* handle a "." or ".." subdirectory: */
	if (!strcmp(entry->tmesh_fs_dirent_name, ".")
	    || !strcmp(entry->tmesh_fs_dirent_name, "..")) {
	
	  /* if we're listing all, and this is pass one: */
	  if ((flags & TMESH_LS_ALL)
	      && pass == 1) {
	    
	    /* output the directory's name: */
	    _tmesh_fs_pathname_dir(dir, _output, 
				   ((flags & TMESH_LS_ABSOLUTE)
				    ? NULL
				    : parent));
	    tme_output_append(_output, "\n");
	  }
	}

	/* otherwise this is a regular subdirectory: */
	else {

	  /* if this is pass one: */
	  if (pass == 1) {

	    /* output the directory's name: */
	    _tmesh_fs_pathname_dir(dir, _output, 
				   ((flags & TMESH_LS_ABSOLUTE)
				    ? NULL
				    : parent));
	    tme_output_append(_output, "/\n");
	  }

	  /* otherwise, this is pass two: */
	  else {

	    /* output the directory's name and recurse: */
	    tme_output_append(_output, "\n./");
	    _tmesh_fs_pathname_dir(dir, _output, 
				   ((flags & TMESH_LS_ABSOLUTE)
				    ? NULL
				    : parent_top));
	    tme_output_append(_output, ":\n");
	    _tmesh_ls_dir(dir, _output, parent_top, flags);
	  }
	}
	break;

	/* this is an element: */
      case TMESH_FS_DIRENT_ELEMENT:
	element = entry->tmesh_fs_dirent_value;
	_tmesh_ls_element(element, _output, flags);
	break;

      default: assert(FALSE);
      }
    
      /* if this was the last entry in the directory, we're done: */
      entry = entry->tmesh_fs_dirent_next;
    } while (entry != parent);
  }
}

/* the "ls" command: */
static int
_tmesh_command_ls(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  char *opts, *pathname, opt;
  struct tmesh_fs_dirent *parent, *entry;
  int type;
  void *what;
  int rc;
  int flags;

  /* take apart any options: */
  opts = value->tmesh_parser_value_strings[0];
  flags = TMESH_LS_NORMAL;
  if (opts != NULL) {
    assert(*opts == '-');
    for (; (opt = *(++opts)) != '\0'; ) {
      switch (opt) {

	/* 'a' lists all directory entries, and all connections on elements: */
      case 'a': flags |= TMESH_LS_ALL; break;

	/* 'l' lists absolute pathnames for elements: */
      case 'l': flags |= TMESH_LS_ABSOLUTE; break;

	/* 'R' recurses: */
      case 'R': flags |= TMESH_LS_RECURSE; break;

	/* an unknown option: */
      default:
	tme_output_append(_output,
			  "ls: %s '-%c'\n",
			  _("invalid option"),
			  opt);
	return (EINVAL);
      }
    }
  }

  /* if a path name is given, look it up: */
  pathname = value->tmesh_parser_value_strings[1];
  if (pathname != NULL) {
    rc = _tmesh_fs_lookup(tmesh,
			  &pathname,
			  &parent, &entry,
			  _output,
			  TMESH_SEARCH_NORMAL);
    
    /* if the lookup failed: */
    if (rc != TME_OK) {
      return (rc);
    }

    /* get what we're listing: */
    type = entry->tmesh_fs_dirent_type;
    what = entry->tmesh_fs_dirent_value;
  }

  /* otherwise, list the cwd: */
  else {
    type = TMESH_FS_DIRENT_DIR;
    what = tmesh->tmesh_cwd;
  }

  /* dispatch on the type: */
  switch (type) {
  case TMESH_FS_DIRENT_DIR:
    _tmesh_ls_dir(what, _output, what, flags);
    break;
  case TMESH_FS_DIRENT_ELEMENT:
    _tmesh_ls_element(what, _output, flags);
    break;
  default: assert(FALSE);
  }
    
  return (TME_OK);
}

/* the "connect" command: */
static int
_tmesh_command_connect(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  char *pathname;
  struct tmesh_fs_element *element0, *element1;
  char **element0_args;
  char **element1_args;
  char **creation_args;
  struct tmesh_fs_element_conn *conn0, *conn1, **prev;
  int which;
  int rc;
  
  /* get and NULL-terminate all argument lists: */
  element0_args = value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argv;
  assert(element0_args != NULL);
  element0_args[value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argc] = NULL;
  element1_args = value->tmesh_parser_value_argvs[1].tmesh_parser_argv_argv;
  if (element1_args != NULL) {
    element1_args[value->tmesh_parser_value_argvs[1].tmesh_parser_argv_argc] = NULL;
  }
  creation_args = value->tmesh_parser_value_argvs[2].tmesh_parser_argv_argv;
  if (creation_args != NULL) {
    creation_args[value->tmesh_parser_value_argvs[2].tmesh_parser_argv_argc] = NULL;
  }
  
  /* check any other element to connect to: */
  element1 = NULL;
  if (element1_args != NULL) {

    /* look up the element: */
    pathname = element1_args[0];
    rc = _tmesh_fs_lookup(tmesh,
			  &pathname,
			  &parent, &entry,
			  _output,
			  TMESH_SEARCH_NORMAL);
    if (rc != TME_OK) {
      return (rc);
    }

    /* this must be an element: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_ELEMENT) {
      tme_output_append(_output, element1_args[0]);
      return (ENOTSOCK);
    }
    element1 = entry->tmesh_fs_dirent_value;
  }

  /* check the element: */
  pathname = element0_args[0];
  rc = _tmesh_fs_lookup(tmesh,
			&pathname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_LAST_PART_OK);
  if (rc != TME_OK) {
    return (rc);
  }
  
  /* if the name exists: */
  if (entry != NULL) {

    /* it must be an element: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_ELEMENT) {
      tme_output_append(_output, value->tmesh_parser_value_pathname0);
      return (ENOTSOCK);
    }
    element0 = entry->tmesh_fs_dirent_value;

    /* we must have been given no creation arguments: */
    if (creation_args != NULL) {
      return (EEXIST);
    }
  }

  /* otherwise, the name doesn't exist: */
  else {

    /* we must have been given some creation arguments: */
    if (creation_args == NULL) {
      return (ENOENT);
    }

    /* allocate the new element: */
    element0 = tme_new0(struct tmesh_fs_element, 1);

    /* set this element's parent and link it into the directory: */
    element0->tmesh_fs_element_parent = parent;
    entry = _tmesh_fs_link(parent, tme_strdup(pathname), 
			   TMESH_FS_DIRENT_ELEMENT, element0);

    /* open the log for this element: */
    pathname = NULL;
    _tmesh_fs_pathname_element(element0, &pathname, NULL);
    (*tmesh->tmesh_support.tmesh_support_log_open)
      (&tmesh->tmesh_support,
       &element0->tmesh_fs_element_element.tme_element_log_handle,
       pathname,
       creation_args[0]);
    tme_free(pathname);

    /* create the element: */
    rc = tme_element_new(&element0->tmesh_fs_element_element,
			 (const char **) creation_args,
			 NULL,
			 _output);
    
    /* if the creation failed: */
    if (rc != TME_OK) {

      /* close the log for this element: */
      (*tmesh->tmesh_support.tmesh_support_log_close)
	(&tmesh->tmesh_support,
	 &element0->tmesh_fs_element_element.tme_element_log_handle);

      /* unlink and free this entry: */
      _tmesh_fs_unlink(entry);
      tme_free(entry->tmesh_fs_dirent_name);
      tme_free(entry);
      return (rc);
    }

    /* the creation succeeded.  set the generation number and preserve
       the creation arguments: */
    element0->tmesh_fs_element_gen = ++tmesh->tmesh_gen_last;
    element0->tmesh_fs_element_argv = value->tmesh_parser_value_argvs[2];
    _tmesh_gc_release_argv(tmesh, &element0->tmesh_fs_element_argv);
  }

  /* if we have another element, make a connection: */
  if (element1 != NULL) {
    rc = tme_element_connect(&element0->tmesh_fs_element_element, 
			     (const char **) element0_args,
			     &element1->tmesh_fs_element_element, 
			     (const char **) element1_args,
			     _output, &which);

    /* if the connection failed: */
    if (rc != TME_OK) {
      return (rc);
    }

    /* allocate the new connections: */
    for (prev = &element0->tmesh_fs_element_conns;
	 (conn0 = *prev) != NULL;
	 prev = &conn0->tmesh_fs_element_conn_next);
    *prev = conn0 = tme_new0(struct tmesh_fs_element_conn, 1);
    for (prev = &element1->tmesh_fs_element_conns;
	 (conn1 = *prev) != NULL;
	 prev = &conn1->tmesh_fs_element_conn_next);
    *prev = conn1 = tme_new0(struct tmesh_fs_element_conn, 1);

    /* remember the connections: */
    conn0->tmesh_fs_element_conn_element = element0;
    conn0->tmesh_fs_element_conn_gen = tmesh->tmesh_gen_last;
    conn0->tmesh_fs_element_conn_other = conn1;
    conn0->tmesh_fs_element_conn_argv = value->tmesh_parser_value_argvs[0];
    _tmesh_gc_release_argv(tmesh, &conn0->tmesh_fs_element_conn_argv);
    conn1->tmesh_fs_element_conn_element = element1;
    conn1->tmesh_fs_element_conn_gen = tmesh->tmesh_gen_last;
    conn1->tmesh_fs_element_conn_other = conn0;
    conn1->tmesh_fs_element_conn_argv = value->tmesh_parser_value_argvs[1];
    _tmesh_gc_release_argv(tmesh, &conn1->tmesh_fs_element_conn_argv);
  }

  /* done: */
  return (rc);
}

/* the "mv" command: */
static int
_tmesh_command_mv(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  /* TBD: */
  abort();
}

/* the "rm" command: */
static int
_tmesh_command_rm(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  /* TBD: */
  abort();
}

/* the "command" command: */
static int
_tmesh_command_command(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  char *pathname;
  struct tmesh_fs_element *element;
  char **element_args;
  int rc;

  /* look up the element: */
  element_args = value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argv;
  assert(element_args != NULL);
  element_args[value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argc] = NULL;
  pathname = element_args[0];
  rc = _tmesh_fs_lookup(tmesh,
			&pathname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_NORMAL);

  /* if the lookup succeeded: */
  if (rc == TME_OK) {

    /* if this pathname doesn't refer to an element, we can't
       power it: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_ELEMENT) {
      rc = ENOTSOCK;
    }

    /* otherwise, this is an element: */
    else {
      element = entry->tmesh_fs_dirent_value;

      /* if it doesn't support the "command" command: */
      if (element->tmesh_fs_element_element.tme_element_command == NULL) {
	rc = EOPNOTSUPP;
      }

      /* otherwise, run the command: */
      else {
	rc = ((*element->tmesh_fs_element_element.tme_element_command)
	      (&element->tmesh_fs_element_element,
	       (const char **) element_args,
	       _output));
      }
    }
  }

  return (rc);
}

/* the "log" command: */
static int
_tmesh_command_log(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  char *pathname;
  struct tmesh_fs_element *element;
  char **element_args;
#ifndef TME_NO_LOG
  unsigned long log_level;
  char *p1;
#endif /* !TME_NO_LOG */
  int rc;

  /* look up the element: */
  element_args = value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argv;
  assert(element_args != NULL);
  element_args[value->tmesh_parser_value_argvs[0].tmesh_parser_argv_argc] = NULL;
  pathname = element_args[0];
  rc = _tmesh_fs_lookup(tmesh,
			&pathname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_NORMAL);

  /* if the lookup succeeded: */
  if (rc == TME_OK) {

    /* if this pathname doesn't refer to an element, we can't
       set its logging level: */
    if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_ELEMENT) {
      rc = ENOTSOCK;
    }

    /* otherwise, this is an element: */
    else {
      element = entry->tmesh_fs_dirent_value;

      /* if logging is not supported: */
#ifdef TME_NO_LOG
      rc = EOPNOTSUPP;
#else  /* !TME_NO_LOG */

      /* assume our arguments are invalid: */
      rc = EINVAL;

      /* check our arguments: */
      if (element_args[1] != NULL
	  && element_args[2] == NULL) {

	/* assume a log level of zero: */
	log_level = 0;

	/* "off" means a log level of zero: */
	if (!strcmp(element_args[1], "off")) {
	  rc = TME_OK;
	}

	/* any other argument must be a log level: */
	else {
	  log_level = strtoul(element_args[1], &p1, 0);
	  if (p1 != element_args[1]
	      && *p1 == '\0') {
	    rc = TME_OK;
	  }
	}

	/* set the log level: */
	if (rc == TME_OK) {
	  element->tmesh_fs_element_element.tme_element_log_handle.tme_log_handle_level_max
	    = log_level;
	}
      }
#endif /* !TME_NO_LOG */
    }
  }

  return (rc);
}

/* the "alias" command: */
static int
_tmesh_command_alias(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output)
{
  struct tmesh_fs_dirent *parent, *entry;
  struct tmesh_fs_element *element;
  char *oldname;
  char *newname;
  int rc;

  /* look up the existing element: */
  oldname = value->tmesh_parser_value_pathname1;
  rc = _tmesh_fs_lookup(tmesh,
			&oldname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_NORMAL);

  /* if the lookup failed, return now: */
  if (rc != TME_OK) {
    return (rc);
  }

  /* if this pathname doesn't refer to an element, we can't alias it: */
  if (entry->tmesh_fs_dirent_type != TMESH_FS_DIRENT_ELEMENT) {
    return (ENOTSOCK);
  }

  /* get the element to alias: */
  element = entry->tmesh_fs_dirent_value;

  /* look up the new name: */
  newname = value->tmesh_parser_value_pathname0;
  rc = _tmesh_fs_lookup(tmesh,
			&newname,
			&parent, &entry,
			_output,
			TMESH_SEARCH_LAST_PART_OK);
  if (rc != TME_OK) {
    return (rc);
  }

  /* if the name exists: */
  if (entry != NULL) {
    return (EEXIST);
  }

  /* create the alias: */
  entry = _tmesh_fs_link(parent, tme_strdup(newname), 
			 TMESH_FS_DIRENT_ELEMENT, element);
  return (TME_OK);
}

/* this evaluates one or more commands: */
int
tmesh_eval(void *_tmesh, char **_output, int *_yield)
{
  struct tmesh *tmesh;
  struct tmesh_parser_value value;
  int rc;
  int (*command_func) _TME_P((struct tmesh *, struct tmesh_parser_value *, char **));
  
  /* recover our structure: */
  tmesh = (struct tmesh *) _tmesh;

  /* start the output: */
  *_output = NULL;

  /* we don't have any memory to gc: */
  tmesh->tmesh_gc_record = NULL;

  /* parse a command: */
  rc = _tmesh_yyparse(tmesh, &value, _output, _yield);

  /* if the parse succeeded: */
  if (rc == TME_OK && !*_yield) {

    /* dispatch on the command: */
    switch (value.tmesh_parser_value_token) {
    default: assert(FALSE);
    case TMESH_COMMAND_NOP: command_func = NULL; break;
    case TMESH_COMMAND_SOURCE: command_func = _tmesh_command_source; break;
    case TMESH_COMMAND_MKDIR: command_func = _tmesh_command_mkdir; break;
    case TMESH_COMMAND_RMDIR: command_func = _tmesh_command_rmdir; break;
    case TMESH_COMMAND_CD: command_func = _tmesh_command_cd; break;
    case TMESH_COMMAND_PWD: command_func = _tmesh_command_pwd; break;
    case TMESH_COMMAND_LS: command_func = _tmesh_command_ls; break;
    case TMESH_COMMAND_CONNECT: command_func = _tmesh_command_connect; break;
    case TMESH_COMMAND_RM: command_func = _tmesh_command_rm; break;
    case TMESH_COMMAND_MV: command_func = _tmesh_command_mv; break;
    case TMESH_COMMAND_COMMAND: command_func = _tmesh_command_command; break;
    case TMESH_COMMAND_LOG: command_func = _tmesh_command_log; break;
    case TMESH_COMMAND_ALIAS: command_func = _tmesh_command_alias; break;
    }
    
    /* call the function: */
    if (command_func != NULL) {
      rc = (*command_func)(tmesh, &value, _output);
    }
  }

  /* garbage collect: */
  _tmesh_gc_gc(tmesh);

  /* done: */
  return (rc);
}

/* this creates a new shell: */
void *
tmesh_new(const struct tmesh_support *support, const struct tmesh_io *first_io)
{
  struct tmesh *tmesh;

  /* allocate the new shell: */
  tmesh = tme_new(struct tmesh, 1);
  
  /* start the io stack: */
  tmesh->tmesh_io_stack = tme_new0(struct tmesh_io_stack, 1);
  tmesh->tmesh_io_stack->tmesh_io_stack_io = *first_io;
  
  /* create the root directory: */
  tmesh->tmesh_root = _tmesh_fs_mkdir(NULL, NULL);

  /* set the cwd: */
  tmesh->tmesh_cwd = tmesh->tmesh_root;

  /* save the support: */
  tmesh->tmesh_support = *support;

  /* done: */
  return (tmesh);
}
