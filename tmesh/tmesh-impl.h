/* $Id: tmesh-impl.h,v 1.3 2006/11/15 23:11:56 fredette Exp $ */

/* tmesh/tmesh-impl.h - private header file for the tmesh implementation: */

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

#ifndef _TMESH_IMPL_H
#define _TMESH_IMPL_H

#include <tme/common.h>
_TME_RCSID("$Id: tmesh-impl.h,v 1.3 2006/11/15 23:11:56 fredette Exp $");

/* includes: */
#include <tme/element.h>
#include <tme/tmesh.h>
#include <errno.h>

/* macros: */

/* commands: */
#define TMESH_COMMAND_NOP	(0)
#define TMESH_COMMAND_SOURCE	(1)
#define TMESH_COMMAND_MKDIR	(2)
#define TMESH_COMMAND_RMDIR	(3)
#define TMESH_COMMAND_CD	(4)
#define TMESH_COMMAND_PWD	(5)
#define TMESH_COMMAND_LS	(6)
#define TMESH_COMMAND_CONNECT	(7)
#define TMESH_COMMAND_RM	(8)
#define TMESH_COMMAND_MV	(9)
#define TMESH_COMMAND_COMMAND	(10)
#define TMESH_COMMAND_LOG	(11)
#define TMESH_COMMAND_ALIAS	(12)

/* directory entry types: */
#define TMESH_FS_DIRENT_DIR	(0)
#define TMESH_FS_DIRENT_ELEMENT	(1)

/* lookup/find flags: */
#define TMESH_SEARCH_NORMAL		(0)
#define TMESH_SEARCH_LAST_PART_OK	TME_BIT(0)
#define TMESH_SEARCH_NO_RECURSE		TME_BIT(1)

/* memory allocation: */
#define _tmesh_gc_new(s, t, x)		((t *) _tmesh_gc_malloc(s, sizeof(t) * (x)))
#define _tmesh_gc_renew(s, t, m, x)	((t *) _tmesh_gc_realloc(s, m, sizeof(t) * (x)))

/* a garbage collection record: */
struct tmesh_gc_record {

  /* the next and previous records on the list: */
  struct tmesh_gc_record *tmesh_gc_record_next;
  struct tmesh_gc_record **tmesh_gc_record_prev;

  /* the memory to garbage collect: */
  void *tmesh_gc_record_mem;
};

/* a stack of ios: */
struct tmesh_io_stack {

  /* the next io on the stack: */
  struct tmesh_io_stack *tmesh_io_stack_next;

  /* the io itself: */
  struct tmesh_io tmesh_io_stack_io;
};

/* the scanner state: */
struct tmesh_scanner {

  /* nonzero iff we need to increment the line number: */
  unsigned int tmesh_scanner_next_line;

  /* any next token to return: */
  int tmesh_scanner_token_next;

  /* any next character to get: */
  int tmesh_scanner_c_next;

  /* nonzero iff we are inside a comment: */
  int tmesh_scanner_in_comment;

  /* nonzero iff we are inside quotes: */
  int tmesh_scanner_in_quotes;

  /* nonzero iff we are in arguments: */
  int tmesh_scanner_in_args;

  /* the collected token: */
  char *tmesh_scanner_token_string;
  unsigned int tmesh_scanner_token_string_len;
  unsigned int tmesh_scanner_token_string_size;
};

/* a parser argv: */
struct tmesh_parser_argv {

  /* the argument count: */
  unsigned int tmesh_parser_argv_argc;

  /* the size of the argument vector: */
  unsigned int tmesh_parser_argv_size;

  /* the argument vector: */
  char **tmesh_parser_argv_argv;
};

/* the parser value structure: */
struct tmesh_parser_value {
  
  /* a token: */
  int tmesh_parser_value_token;
#define tmesh_parser_value_command tmesh_parser_value_token

  /* up to two strings: */
  char *tmesh_parser_value_strings[2];
#define tmesh_parser_value_pathname0 tmesh_parser_value_strings[0]
#define tmesh_parser_value_pathname1 tmesh_parser_value_strings[1]
#define tmesh_parser_value_arg tmesh_parser_value_strings[0]

  /* up to three argument vectors: */
  struct tmesh_parser_argv tmesh_parser_value_argvs[3];
};

/* a directory entry: */
struct tmesh_fs_dirent {

  /* the next and previous entries in this directory: */
  struct tmesh_fs_dirent *tmesh_fs_dirent_next;
  struct tmesh_fs_dirent **tmesh_fs_dirent_prev;

  /* the type of this directory entry: */
  int tmesh_fs_dirent_type;

  /* the name in this directory entry: */
  char *tmesh_fs_dirent_name;

  /* the value in this directory entry: */
  void *tmesh_fs_dirent_value;
};

/* an element: */
struct tmesh_fs_element {

  /* the parent directory of this element: */
  struct tmesh_fs_dirent *tmesh_fs_element_parent;

  /* the real element: */
  struct tme_element tmesh_fs_element_element;

  /* the generation number of this element: */
  unsigned long tmesh_fs_element_gen;

  /* the arguments for this element: */
  struct tmesh_parser_argv tmesh_fs_element_argv;

  /* the element connections: */
  struct tmesh_fs_element_conn {

    /* the next element connection: */
    struct tmesh_fs_element_conn *tmesh_fs_element_conn_next;
    
    /* backpointer to the element: */
    struct tmesh_fs_element *tmesh_fs_element_conn_element;
    
    /* the generation number of this connection: */
    unsigned long tmesh_fs_element_conn_gen;
    
    /* the other side of this element connection: */
    struct tmesh_fs_element_conn *tmesh_fs_element_conn_other;
    
    /* the arguments for this side of the connection: */
    struct tmesh_parser_argv tmesh_fs_element_conn_argv;
  } *tmesh_fs_element_conns;
};

/* the tmesh structure: */
struct tmesh {

  /* the stack of ios: */
  struct tmesh_io_stack *tmesh_io_stack;

  /* the scanner: */
  struct tmesh_scanner tmesh_scanner;

  /* the root directory: */
  struct tmesh_fs_dirent *tmesh_root;

  /* the current working directory: */
  struct tmesh_fs_dirent *tmesh_cwd;

  /* garbage-collectable memory: */
  struct tmesh_gc_record *tmesh_gc_record;
  
  /* the last generation number: */
  unsigned long tmesh_gen_last;

  /* the support: */
  struct tmesh_support tmesh_support;
};

/* prototypes: */
int _tmesh_yyparse _TME_P((struct tmesh *, struct tmesh_parser_value *, char **, int *));
void _tmesh_io_collect_output _TME_P((char **, const char *));
void _tmesh_io_collect_outputn _TME_P((char **, const char *, unsigned int));
int _tmesh_fs_lookup _TME_P((struct tmesh *, char **, struct tmesh_fs_dirent **, struct tmesh_fs_dirent **, char **, int));
struct tmesh_fs_dirent * _tmesh_fs_link _TME_P((struct tmesh_fs_dirent *, char *, int, void *));
void _tmesh_fs_unlink _TME_P((struct tmesh_fs_dirent *));
struct tmesh_fs_dirent * _tmesh_fs_mkdir _TME_P((struct tmesh_fs_dirent *, char *));
void _tmesh_fs_pathname_dir _TME_P((struct tmesh_fs_dirent *, char **, struct tmesh_fs_dirent *));
void _tmesh_fs_pathname_element _TME_P((struct tmesh_fs_element *, char **, struct tmesh_fs_dirent *));
void *_tmesh_gc_malloc _TME_P((struct tmesh *, unsigned int));
void *_tmesh_gc_realloc _TME_P((struct tmesh *, void *, unsigned int));
void _tmesh_gc_free _TME_P((struct tmesh *, void *));
void _tmesh_gc_release _TME_P((struct tmesh *, void *));
void _tmesh_gc_release_argv _TME_P((struct tmesh *, struct tmesh_parser_argv *));
void _tmesh_gc_gc _TME_P((struct tmesh *));

#endif /* !_TMESH_IMPL_H */
