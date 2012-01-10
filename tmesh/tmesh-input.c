#include <stdlib.h>
#ifndef lint
#if 0
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#else
#if defined(__NetBSD__) && defined(__IDSTRING)
__IDSTRING(yyrcsid, "$NetBSD: skeleton.c,v 1.25 2003/08/07 11:17:54 agc Exp $");
#endif /* __NetBSD__ && __IDSTRING */
#endif /* 0 */
#endif /* lint */
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 2 "tmesh-input.y"
/* $Id: tmesh-input.y,v 1.4 2006/11/15 23:11:31 fredette Exp $ */

/* tmesh/tmesh-input.y - the tme shell scanner and parser: */

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
_TME_RCSID("$Id: tmesh-input.y,v 1.4 2006/11/15 23:11:31 fredette Exp $");

/* includes: */
#include <tme/threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tmesh-impl.h"

/* macros: */

/* internal token numbers: */
#define TMESH_TOKEN_UNDEF		(-1)
#define TMESH_TOKEN_EOF			(0)

/* internal character numbers: */
#define TMESH_C_EOF_SEMICOLON		(TMESH_C_YIELD - 1)
#define TMESH_C_UNDEF			(TMESH_C_EOF_SEMICOLON - 2)

#define YYSTYPE struct tmesh_parser_value
#define YYDEBUG 1
#define YYMAXDEPTH 10000

/* types: */

/* globals: */
static tme_mutex_t _tmesh_input_mutex;
static struct tmesh *_tmesh_input;
static char **_tmesh_output;
static int _tmesh_input_yielding;
static YYSTYPE *_tmesh_input_parsed;

/* prototypes: */
static int yylex _TME_P((void));
static void yyerror _TME_P((char *));
static void _tmesh_scanner_in_args _TME_P((void));
static void _tmesh_parser_argv_arg _TME_P((struct tmesh_parser_argv *, char *, int));

#line 96 "tmesh-input.c"
#define TMESH_TOKEN_SOURCE 257
#define TMESH_TOKEN_MKDIR 258
#define TMESH_TOKEN_RMDIR 259
#define TMESH_TOKEN_CD 260
#define TMESH_TOKEN_PWD 261
#define TMESH_TOKEN_LS 262
#define TMESH_TOKEN_CONNECT 263
#define TMESH_TOKEN_RM 264
#define TMESH_TOKEN_MV 265
#define TMESH_TOKEN_COMMAND 266
#define TMESH_TOKEN_LOG 267
#define TMESH_TOKEN_ALIAS 268
#define TMESH_TOKEN_AT 269
#define TMESH_TOKEN_PATHNAME 270
#define TMESH_TOKEN_ARG 271
#define TMESH_TOKEN_OPTS 272
#define YYERRCODE 256
const short yylhs[] = {                                        -1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    1,    2,    3,    4,    5,    6,
    7,    7,    8,    9,   10,   11,   12,   13,   15,   15,
   17,   17,   18,   18,   19,   20,   16,   16,   16,   14,
   14,
};
const short yylen[] = {                                         2,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    2,    1,    3,    3,    3,    3,    2,    4,
    3,    2,    3,    4,    3,    3,    4,    1,    1,    0,
    1,    2,    1,    2,    1,    1,    3,    5,    3,    1,
    0,
};
const short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   31,   14,    0,    1,    2,    3,    4,
    5,    6,    7,    8,    9,   10,   11,   12,    0,    0,
   13,   28,    0,    0,    0,    0,   19,   40,    0,    0,
    0,    0,    0,    0,    0,   22,   36,   32,   35,    0,
    0,   15,   16,   17,   18,   29,    0,   21,   23,    0,
   25,   26,    0,   33,    0,    0,   20,   24,   27,   34,
    0,    0,
};
const short yydgoto[] = {                                      16,
   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
   27,   28,   33,   39,   57,   29,   30,   65,   50,   51,
};
const short yysindex[] = {                                    -39,
  -46, -260, -260, -260, -260,  -45, -256, -253, -260, -260,
 -253, -253, -260,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -41,  -58,
    0,    0,  -38,  -36,  -35,  -34,    0,    0, -244,  -32,
  -31, -260,  -56,  -55, -260,    0,    0,    0,    0, -242,
 -242,    0,    0,    0,    0,    0,  -29,    0,    0,  -28,
    0,    0,  -27,    0, -238,  -57,    0,    0,    0,    0,
 -242, -238,
};
const short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,  -40,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -25,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -45,  -24,    0,    0,    0,    0,
    0,  -23,
};
const short yygindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    2,    0,    0,   29,   -3,  -49,  -26,    0,
};
#define YYTABLESIZE 231
const short yytable[] = {                                      49,
   49,   66,   61,   62,   34,   35,   36,   43,   44,   32,
   41,   42,   31,   37,   45,   38,   14,   46,   41,   15,
   52,   72,   53,   54,   55,   56,   58,   59,   64,   67,
   68,   69,   70,   30,   39,   38,   40,    0,    0,   71,
    0,    0,    0,   60,    0,    0,   63,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   47,    0,   48,   70,   48,   48,    1,    2,    3,    4,
    5,    6,    7,    8,    9,   10,   11,   12,   13,   41,
   14,
};
const short yycheck[] = {                                      58,
   58,   51,   59,   59,    3,    4,    5,   11,   12,  270,
    9,   10,   59,   59,   13,  272,  270,   59,   59,   59,
   59,   71,   59,   59,   59,  270,   59,   59,  271,   59,
   59,   59,  271,   59,   59,   59,    8,   -1,   -1,   66,
   -1,   -1,   -1,   42,   -1,   -1,   45,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  269,   -1,  271,  271,  271,  271,  256,  257,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  270,
  270,
};
#define YYFINAL 16
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 272
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"':'","';'",0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"TMESH_TOKEN_SOURCE",
"TMESH_TOKEN_MKDIR","TMESH_TOKEN_RMDIR","TMESH_TOKEN_CD","TMESH_TOKEN_PWD",
"TMESH_TOKEN_LS","TMESH_TOKEN_CONNECT","TMESH_TOKEN_RM","TMESH_TOKEN_MV",
"TMESH_TOKEN_COMMAND","TMESH_TOKEN_LOG","TMESH_TOKEN_ALIAS","TMESH_TOKEN_AT",
"TMESH_TOKEN_PATHNAME","TMESH_TOKEN_ARG","TMESH_TOKEN_OPTS",
};
const char * const yyrule[] = {
"$accept : command",
"command : command_source",
"command : command_mkdir",
"command : command_rmdir",
"command : command_cd",
"command : command_pwd",
"command : command_ls",
"command : command_connect",
"command : command_rm",
"command : command_mv",
"command : command_command",
"command : command_log",
"command : command_alias",
"command : error ';'",
"command : ';'",
"command_source : TMESH_TOKEN_SOURCE pathname ';'",
"command_mkdir : TMESH_TOKEN_MKDIR pathname ';'",
"command_rmdir : TMESH_TOKEN_RMDIR pathname ';'",
"command_cd : TMESH_TOKEN_CD pathname ';'",
"command_pwd : TMESH_TOKEN_PWD ';'",
"command_ls : TMESH_TOKEN_LS opts_opt pathname_opt ';'",
"command_connect : TMESH_TOKEN_CONNECT connection ';'",
"command_connect : connection ';'",
"command_rm : TMESH_TOKEN_RM pathname ';'",
"command_mv : TMESH_TOKEN_MV pathname pathname ';'",
"command_command : TMESH_TOKEN_COMMAND pathname_args ';'",
"command_log : TMESH_TOKEN_LOG pathname_args ';'",
"command_alias : TMESH_TOKEN_ALIAS pathname pathname ';'",
"pathname : TMESH_TOKEN_PATHNAME",
"pathname_opt : TMESH_TOKEN_PATHNAME",
"pathname_opt :",
"pathname_args : TMESH_TOKEN_PATHNAME",
"pathname_args : pathname_args TMESH_TOKEN_ARG",
"args : TMESH_TOKEN_ARG",
"args : args TMESH_TOKEN_ARG",
"colon : ':'",
"at : TMESH_TOKEN_AT",
"connection : pathname_args colon args",
"connection : pathname_args at args colon args",
"connection : pathname_args at args",
"opts_opt : TMESH_TOKEN_OPTS",
"opts_opt :",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
int yyparse(void);
#line 267 "tmesh-input.y"

/* this adds a new argument to an argument vector: */
static void
_tmesh_parser_argv_arg(struct tmesh_parser_argv *argv, char *arg, int new)
{

  /* if we're starting a new argv, allocate the initial vector, else
     make sure the argv has enough room for the new argument and a
     trailing NULL that _tmesh_command_connect will add later: */
  if (new) {
    argv->tmesh_parser_argv_size = 8;
    argv->tmesh_parser_argv_argv = 
      _tmesh_gc_new(_tmesh_input,
		    char *, 
		    argv->tmesh_parser_argv_size);
    argv->tmesh_parser_argv_argc = 0;
  }
  else if ((argv->tmesh_parser_argv_argc + 1)
	   >= argv->tmesh_parser_argv_size) {
    argv->tmesh_parser_argv_size +=
      (2
       + (argv->tmesh_parser_argv_size >> 1));
    argv->tmesh_parser_argv_argv = 
      _tmesh_gc_renew(_tmesh_input,
		      char *, 
		      argv->tmesh_parser_argv_argv,
		      argv->tmesh_parser_argv_size);
  }

  /* put in the new argument: */
  argv->tmesh_parser_argv_argv[argv->tmesh_parser_argv_argc++] = arg;
}

/* this is called by the parser when it encounters an error: */
static void
yyerror(char *msg)
{
  tme_output_append(_tmesh_output, msg);
  _tmesh_input->tmesh_scanner.tmesh_scanner_in_args = FALSE;
}

/* this is called by the parser when args can be expected: */
static void
_tmesh_scanner_in_args(void)
{
  _tmesh_input->tmesh_scanner.tmesh_scanner_in_args = TRUE;
}

/* this matches a collected token: */
static int
_tmesh_scanner_token(struct tmesh_scanner *scanner)
{
  int token;
  char *string;
  int keep_string;

  /* if we have no collected token, return no token: */
  if (scanner->tmesh_scanner_token_string_size == 0
      || scanner->tmesh_scanner_token_string_len == 0) {
    return (TMESH_TOKEN_UNDEF);
  }

  /* get the collected token: */
  string = scanner->tmesh_scanner_token_string;
  string[scanner->tmesh_scanner_token_string_len] = '\0';

  /* assume we won't need to keep this string: */
  keep_string = FALSE;

  /* the reserved word "at" is always recognized, since it can
     terminate a list of arguments: */
  if (!strcmp(string, "at")) {
    token = TMESH_TOKEN_AT;
    scanner->tmesh_scanner_in_args = FALSE;
  }
  
  /* if we're in arguments, every other collected token is an argument: */
  else if (scanner->tmesh_scanner_in_args) {
    token = TMESH_TOKEN_ARG;
    keep_string = TRUE;
  }

  /* otherwise, if we're not in arguments, every other collected token
     is either a reserved word, options, or a pathname: */
  else {
    if (!strcmp(string, "source")) {
      token = TMESH_TOKEN_SOURCE;
    }  
    else if (!strcmp(string, "cd")) {
      token = TMESH_TOKEN_CD;
    }  
    else if (!strcmp(string, "pwd")) {
      token = TMESH_TOKEN_CD;
    }  
    else if (!strcmp(string, "ls")) {
      token = TMESH_TOKEN_LS;
    }  
    else if (!strcmp(string, "rm")) {
      token = TMESH_TOKEN_RM;
    }  
    else if (!strcmp(string, "connect")) {
      token = TMESH_TOKEN_CONNECT;
    }  
    else if (!strcmp(string, "mkdir")) {
      token = TMESH_TOKEN_MKDIR;
    }  
    else if (!strcmp(string, "rmdir")) {
      token = TMESH_TOKEN_RMDIR;
    }  
    else if (!strcmp(string, "mv")) {
      token = TMESH_TOKEN_MV;
    }
    else if (!strcmp(string, "command")) {
      token = TMESH_TOKEN_COMMAND;
    }  
    else if (!strcmp(string, "log")) {
      token = TMESH_TOKEN_LOG;
    }  
    else if (!strcmp(string, "alias")) {
      token = TMESH_TOKEN_ALIAS;
    }  
    else if (string[0] == '-') {
      token = TMESH_TOKEN_OPTS;
      keep_string = TRUE;
    }
    else {
      token = TMESH_TOKEN_PATHNAME;
      keep_string = TRUE;
    }
  }

  /* if we need to keep this string, put it in yylval, else recycle it: */
  yylval.tmesh_parser_value_token = token;
  if (keep_string) {
    yylval.tmesh_parser_value_strings[0] = string;
    scanner->tmesh_scanner_token_string_size = 0;
  }
  else {
    yylval.tmesh_parser_value_strings[0] = NULL;
    scanner->tmesh_scanner_token_string_len = 0;
  }

  return (token);
}

/* our scanner: */
int
yylex(void)
{
  struct tmesh_scanner *scanner;
  struct tmesh_io_stack *stack;
  struct tmesh_io *source;
  int token, c;
  
  /* recover our scanner state: */
  scanner = &_tmesh_input->tmesh_scanner;
  stack = _tmesh_input->tmesh_io_stack;
  source = &stack->tmesh_io_stack_io;

  /* bump the input line: */
  source->tmesh_io_input_line += scanner->tmesh_scanner_next_line;
  scanner->tmesh_scanner_next_line = 0;

  /* if we previously scanned the next token to return, return it
     and clear it, unless it's EOF, which sticks: */
  token = scanner->tmesh_scanner_token_next;
  if (token != TMESH_TOKEN_UNDEF) {
    if (token != TMESH_TOKEN_EOF) {
      scanner->tmesh_scanner_token_next = TMESH_TOKEN_UNDEF;
    }
    return (token);
  }

  /* loop forever: */
  for (;;) {

    /* get the next character: */
    c = scanner->tmesh_scanner_c_next;
    if (c == TMESH_C_UNDEF) {
      c = (*source->tmesh_io_getc)(source);
    }
    scanner->tmesh_scanner_c_next = TMESH_C_UNDEF;

    /* if this is an EOF: */
    if (c == TMESH_C_EOF) {

      /* turn c into the EOF semicolon: */
      c = TMESH_C_EOF_SEMICOLON;

      /* if we have collected a token, save the EOF semicolon and return the token: */
      token = _tmesh_scanner_token(scanner);
      if (token != TMESH_TOKEN_UNDEF) {
	scanner->tmesh_scanner_c_next = c;
	return (token);
      }
    }

    /* if this is an EOF semicolon: */
    if (c == TMESH_C_EOF_SEMICOLON) {

      /* quoted strings and comments (and commands, for that matter) cannot cross EOF boundaries: */
      scanner->tmesh_scanner_in_quotes = FALSE;
      scanner->tmesh_scanner_in_comment = FALSE;

      /* close the now-finished source: */
      (*source->tmesh_io_close)(source, 
				(stack->tmesh_io_stack_next != NULL
				 ? &stack->tmesh_io_stack_next->tmesh_io_stack_io
				 : NULL));

      /* pop the io stack: */
      _tmesh_input->tmesh_io_stack = stack->tmesh_io_stack_next;
      tme_free(source->tmesh_io_name);
      tme_free(stack);
      
      /* if we have emptied the source stack, we are really at EOF,
	 and the next time we're called we will return that: */
      stack = _tmesh_input->tmesh_io_stack;
      source = &stack->tmesh_io_stack_io;
      if (stack == NULL) {
	scanner->tmesh_scanner_token_next = TMESH_TOKEN_EOF;
	return (TMESH_TOKEN_EOF);
      }

      /* return the EOF semicolon: */
      return (';');
    }

    /* if this is a yield: */
    if (c == TMESH_C_YIELD) {

      /* we are yielding: */
      _tmesh_input_yielding = TRUE;

      /* return an EOF token: */
      return (TMESH_TOKEN_EOF);
    }    

    /* if we're in a comment: */
    if (scanner->tmesh_scanner_in_comment) {
      if (c != '\n') {
	continue;
      }
      scanner->tmesh_scanner_in_comment = FALSE;
    }

    /* if this is quotation marks: */
    if (c == '"') {
      scanner->tmesh_scanner_in_quotes = !scanner->tmesh_scanner_in_quotes;
      continue;
    }

    /* other than quotation marks, every character either delimits
       tokens or is collected into the current token: */
    if (

	/* any character inside quotes is collected: */
	scanner->tmesh_scanner_in_quotes

	/* any alphanumeric character is collected: */
	|| isalnum(c)

	/* any period, slash, hyphen, and underscore character is collected: */
	|| c == '.'
	|| c == '/'
	|| c == '-'
	|| c == '_'
	) {

      /* allocate or grow the token buffer as needed.  we always
	 make sure there's room for this new character, and a trailing
	 NUL that _tmesh_scanner_token may add: */
      if (scanner->tmesh_scanner_token_string_size == 0) {
	scanner->tmesh_scanner_token_string_len = 0;
	scanner->tmesh_scanner_token_string_size = 8;
	scanner->tmesh_scanner_token_string = 
	  _tmesh_gc_new(_tmesh_input,
			char, 
			scanner->tmesh_scanner_token_string_size);
      }
      else if ((scanner->tmesh_scanner_token_string_len + 1)
	       >= scanner->tmesh_scanner_token_string_size) {
	scanner->tmesh_scanner_token_string_size +=
	  (2
	   + (scanner->tmesh_scanner_token_string_size >> 1));
	scanner->tmesh_scanner_token_string = 
	  _tmesh_gc_renew(_tmesh_input,
			  char, 
			  scanner->tmesh_scanner_token_string,
			  scanner->tmesh_scanner_token_string_size);
      }

      /* collect the character into the buffer: */
      scanner->tmesh_scanner_token_string[scanner->tmesh_scanner_token_string_len++] = c;
    }

    /* delimit this token: */
    else {

      /* if we have collected a token, save the delimiter and return the token: */
      token = _tmesh_scanner_token(scanner);
      if (token != TMESH_TOKEN_UNDEF) {
	scanner->tmesh_scanner_c_next = c;
	return (token);
      }

      /* a carriage return or a newline becomes a semicolon, and
	 a pound sign begins a comment: */
      if (c == '\n') {
	c = ';';
	scanner->tmesh_scanner_next_line = 1;
      }
      else if (c == '\r') {
	c = ';';
      }
      else if (c == '#') {
	scanner->tmesh_scanner_in_comment = TRUE;
	scanner->tmesh_scanner_in_args = FALSE;
	continue;
      }

      /* return a non-whitespace delimiter as a token, and this resets
         the args state: */
      if (!isspace(c)) {
	scanner->tmesh_scanner_in_args = FALSE;
	return (c);
      }
    }
  }
  /* NOTREACHED */
}

/* this is called to parse input: */
int
_tmesh_yyparse(struct tmesh *tmesh, struct tmesh_parser_value *value, char **_output, int *_yield)
{
  struct tmesh_scanner *scanner;
  int rc;
  int command;

  /* initialize the scanner: */
  scanner = &tmesh->tmesh_scanner;
  scanner->tmesh_scanner_token_next = TMESH_TOKEN_UNDEF;
  scanner->tmesh_scanner_c_next = TMESH_C_UNDEF;
  scanner->tmesh_scanner_in_comment = FALSE;
  scanner->tmesh_scanner_in_quotes = FALSE;
  scanner->tmesh_scanner_in_args = FALSE;
  scanner->tmesh_scanner_token_string_size = 0;

  /* lock the input mutex: */
  tme_mutex_lock(&_tmesh_input_mutex);

  /* set this tmesh for input: */
  _tmesh_input = tmesh;
  _tmesh_output = _output;
  
  /* assume that we will not have to yield: */
  _tmesh_input_yielding = FALSE;

  /* call the parser: */
  _tmesh_input_parsed = value;
  rc = (yyparse()
	? EINVAL
	: TME_OK);

  /* tell our caller if we're yielding: */
  *_yield = _tmesh_input_yielding;

  /* unlock the input mutex: */
  tme_mutex_unlock(&_tmesh_input_mutex);

  /* if the parse was successful, map the command token number to a command number: */
  if (rc == TME_OK && !*_yield) {
    switch (value->tmesh_parser_value_token) {
    default: assert(FALSE);
    case TMESH_TOKEN_UNDEF:	command = TMESH_COMMAND_NOP; break;
    case TMESH_TOKEN_SOURCE:	command = TMESH_COMMAND_SOURCE; break;
    case TMESH_TOKEN_MKDIR:	command = TMESH_COMMAND_MKDIR; break;
    case TMESH_TOKEN_RMDIR:	command = TMESH_COMMAND_RMDIR; break;
    case TMESH_TOKEN_CD:	command = TMESH_COMMAND_CD; break;
    case TMESH_TOKEN_PWD:	command = TMESH_COMMAND_PWD; break;
    case TMESH_TOKEN_LS:	command = TMESH_COMMAND_LS; break;
    case TMESH_TOKEN_CONNECT:	command = TMESH_COMMAND_CONNECT; break;
    case TMESH_TOKEN_RM:	command = TMESH_COMMAND_RM; break;
    case TMESH_TOKEN_COMMAND:	command = TMESH_COMMAND_COMMAND; break;
    case TMESH_TOKEN_LOG:	command = TMESH_COMMAND_LOG; break;
    case TMESH_TOKEN_ALIAS:	command = TMESH_COMMAND_ALIAS; break;
    }
    value->tmesh_parser_value_command = command;
  }

  /* done: */
  return (rc);
}
#line 706 "tmesh-input.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void);
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    if ((newss = (short *)realloc(yyss, newsize * sizeof *newss)) == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    if ((newvs = (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs)) == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != NULL)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
    goto yynewerror;
yynewerror:
    yyerror("syntax error");
    goto yyerrlab;
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 99 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 2:
#line 100 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 3:
#line 101 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 4:
#line 102 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 5:
#line 103 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 6:
#line 104 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 7:
#line 105 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 8:
#line 106 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 9:
#line 107 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 10:
#line 108 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 11:
#line 109 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 12:
#line 110 "tmesh-input.y"
{ *_tmesh_input_parsed = yyvsp[0]; YYACCEPT; }
break;
case 13:
#line 111 "tmesh-input.y"
{ YYABORT; }
break;
case 14:
#line 113 "tmesh-input.y"
{ _tmesh_input_parsed->tmesh_parser_value_token = TMESH_TOKEN_UNDEF; YYACCEPT; }
break;
case 15:
#line 118 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 16:
#line 122 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 17:
#line 127 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 18:
#line 132 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 20:
#line 140 "tmesh-input.y"
{
  yyval = yyvsp[-2];
  yyval.tmesh_parser_value_strings[1] = yyvsp[-1].tmesh_parser_value_strings[0];
  yyval.tmesh_parser_value_token = yyvsp[-3].tmesh_parser_value_token;
}
break;
case 21:
#line 149 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 22:
#line 151 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = TMESH_TOKEN_CONNECT; }
break;
case 23:
#line 156 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 24:
#line 161 "tmesh-input.y"
{
  yyval = yyvsp[-2];
  yyval.tmesh_parser_value_strings[1] = yyvsp[-1].tmesh_parser_value_strings[0];
  yyval.tmesh_parser_value_token = yyvsp[-3].tmesh_parser_value_token;
}
break;
case 25:
#line 170 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 26:
#line 175 "tmesh-input.y"
{ yyval = yyvsp[-1]; yyval.tmesh_parser_value_token = yyvsp[-2].tmesh_parser_value_token; }
break;
case 27:
#line 180 "tmesh-input.y"
{
  yyval = yyvsp[-2];
  yyval.tmesh_parser_value_strings[1] = yyvsp[-1].tmesh_parser_value_strings[0];
  yyval.tmesh_parser_value_token = yyvsp[-3].tmesh_parser_value_token;
}
break;
case 30:
#line 194 "tmesh-input.y"
{ yyval.tmesh_parser_value_strings[0] = NULL; }
break;
case 31:
#line 199 "tmesh-input.y"
{ 
  _tmesh_parser_argv_arg(&yyval.tmesh_parser_value_argvs[0], 
			 yyvsp[0].tmesh_parser_value_pathname0, 
			 TRUE);
  _tmesh_scanner_in_args();
}
break;
case 32:
#line 206 "tmesh-input.y"
{
  yyval = yyvsp[-1]; 
  _tmesh_parser_argv_arg(&yyval.tmesh_parser_value_argvs[0], 
			 yyvsp[0].tmesh_parser_value_arg, 
			 FALSE);
}
break;
case 33:
#line 216 "tmesh-input.y"
{
  _tmesh_parser_argv_arg(&yyval.tmesh_parser_value_argvs[0], 
			 yyvsp[0].tmesh_parser_value_arg, 
			 TRUE);
}
break;
case 34:
#line 222 "tmesh-input.y"
{ 
  yyval = yyvsp[-1]; 
  _tmesh_parser_argv_arg(&yyval.tmesh_parser_value_argvs[0], 
			 yyvsp[0].tmesh_parser_value_arg, 
			 FALSE);
}
break;
case 35:
#line 230 "tmesh-input.y"
{ _tmesh_scanner_in_args(); }
break;
case 36:
#line 233 "tmesh-input.y"
{ _tmesh_scanner_in_args(); }
break;
case 37:
#line 237 "tmesh-input.y"
{
  if (yyvsp[-2].tmesh_parser_value_argvs[0].tmesh_parser_argv_argc > 1) {
    yyerror(_("expected 'at'"));
    YYERROR;
  }
  yyval.tmesh_parser_value_argvs[0] = yyvsp[-2].tmesh_parser_value_argvs[0];
  yyval.tmesh_parser_value_argvs[1].tmesh_parser_argv_argv = NULL;
  yyval.tmesh_parser_value_argvs[2] = yyvsp[0].tmesh_parser_value_argvs[0];
}
break;
case 38:
#line 247 "tmesh-input.y"
{
  yyval.tmesh_parser_value_argvs[0] = yyvsp[-4].tmesh_parser_value_argvs[0];
  yyval.tmesh_parser_value_argvs[1] = yyvsp[-2].tmesh_parser_value_argvs[0];
  yyval.tmesh_parser_value_argvs[2] = yyvsp[0].tmesh_parser_value_argvs[0];
}
break;
case 39:
#line 253 "tmesh-input.y"
{
  yyval.tmesh_parser_value_argvs[0] = yyvsp[-2].tmesh_parser_value_argvs[0];
  yyval.tmesh_parser_value_argvs[1] = yyvsp[0].tmesh_parser_value_argvs[0];
  yyval.tmesh_parser_value_argvs[2].tmesh_parser_argv_argv = NULL;
}
break;
case 41:
#line 263 "tmesh-input.y"
{ yyval.tmesh_parser_value_strings[0] = NULL; }
break;
#line 1066 "tmesh-input.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
