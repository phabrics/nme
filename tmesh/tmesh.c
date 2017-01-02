/* $Id: tmesh.c,v 1.4 2009/08/30 17:06:38 fredette Exp $ */

/* tmesh/tmesh.c - the tme shell: */

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
_TME_RCSID("$Id: tmesh.c,v 1.4 2009/08/30 17:06:38 fredette Exp $");

/* includes: */
#include <tme/tme.h>
#include <tme/tmesh.h>
#include <tme/hash.h>
#include <tme/libopenvpn/openvpn-setup.h>
#include <stdio.h>
#include <string.h>
/* macros: */

/* the binary log message buffer size: */
#define _TMESH_LOG_MESSAGE_BINARY_BUFFER_SIZE	(5 * TME_LOG_MESSAGE_SIZE_MAX_BINARY)

/* the binary log message handle, size, and errno: */
#define _TMESH_LOG_MESSAGE_BINARY_ERRNO		(0xff)
#if TME_LOG_MESSAGE_SIZE_MAX_BINARY & (TME_LOG_MESSAGE_SIZE_MAX_BINARY - 1)
#error "TME_LOG_MESSAGE_SIZE_MAX_BINARY must be a power of two"
#endif
#define _TMESH_LOG_MESSAGE_BINARY_SIZE \
  ((TME_LOG_MESSAGE_SIZE_MAX_BINARY - 1) * (_TMESH_LOG_MESSAGE_BINARY_ERRNO + 1))
#define _TMESH_LOG_MESSAGE_BINARY_HANDLE \
  (~ (tme_uint32_t) (_TMESH_LOG_MESSAGE_BINARY_SIZE + _TMESH_LOG_MESSAGE_BINARY_ERRNO))

/* types: */

/* an input buffer: */
struct _tmesh_input {
  tme_thread_handle_t _tmesh_input_handle;
  char _tmesh_input_buffer[1024];
  unsigned int _tmesh_input_buffer_head;
  unsigned int _tmesh_input_buffer_tail;
  int _tmesh_input_buffer_eof;
};

/* a binary log message: */
struct _tmesh_log_message_binary {
  tme_uint32_t _tmesh_log_message_binary_handle_size_errno;
  tme_uint32_t _tmesh_log_message_binary_level;
};

/* globals: */
const char *argv0;

/* our shell instance: */
static void *_tmesh;

/* our current input: */
static struct tmesh_io *_tmesh_io;

/* our log and its mutex: */
static FILE *_tmesh_log;
static tme_mutex_t _tmesh_log_mutex;

/* the log mode: */
static unsigned int _tmesh_log_mode = TME_LOG_MODE_TEXT;

/* the next log handle number: */
static tme_uint32_t _tmesh_log_handle_next;

/* a format hash: */
static tme_hash_t _tmesh_log_hash_format;

/* this removes all consumed characters from a buffer, and shifts
   everything else down: */
static void
_tmesh_remove_consumed(struct _tmesh_input *input)
{
  input->_tmesh_input_buffer_head -= input->_tmesh_input_buffer_tail;
  memmove(input->_tmesh_input_buffer,
	  input->_tmesh_input_buffer
	  + input->_tmesh_input_buffer_tail,
	  input->_tmesh_input_buffer_head);
  input->_tmesh_input_buffer_tail = 0;
}

/* our getc function: */
static int
_tmesh_getc(struct tmesh_io *io)
{
  struct _tmesh_input *input;
  int c;

  /* recover our input: */
  input = io->tmesh_io_private;

  /* if the buffer isn't empty yet, return the next character: */
  if (input->_tmesh_input_buffer_tail
      < input->_tmesh_input_buffer_head) {
    c = input->_tmesh_input_buffer[input->_tmesh_input_buffer_tail++];
    return (c);
  }

  /* if we're at EOF, return EOF: */
  if (input->_tmesh_input_buffer_eof) {
    return (TMESH_C_EOF);
  }

  /* otherwise, we must yield: */
  return (TMESH_C_YIELD);
}

/* our close function: */
static void
_tmesh_close(struct tmesh_io *io_old, struct tmesh_io *io_new)
{
  struct _tmesh_input *input;

  /* recover our input: */
  input = io_old->tmesh_io_private;

  /* close the file and free the input: */
  tme_thread_close(input->_tmesh_input_handle);
  tme_free(input);

  /* set the new, emerging input: */
  _tmesh_io = io_new;
  _tmesh_remove_consumed(io_new->tmesh_io_private);
}

/* our open function: */
static int
_tmesh_open(struct tmesh_io *io_new, struct tmesh_io *io_old, char **_output)
{
  struct _tmesh_input *input;
  int saved_errno;

  /* allocate a new input: */
  input = tme_new0(struct _tmesh_input, 1);

  /* try to open the file: */
  input->_tmesh_input_handle =
    tme_open(io_new->tmesh_io_name, TME_FILE_RO);
  
  /* if the open failed: */
  if (input->_tmesh_input_handle == TME_INVALID_HANDLE) {
    saved_errno = errno;
    tme_free(input);
    return (saved_errno);
  }

  /* set the new input: */
  io_new->tmesh_io_private = input;
  io_new->tmesh_io_getc = _tmesh_getc;
  io_new->tmesh_io_close = _tmesh_close;
  io_new->tmesh_io_open = _tmesh_open;
  _tmesh_io = io_new;

  return (TME_OK);
}

/* our log output function: */
static void
_tmesh_log_output(struct tme_log_handle *handle)
{
  FILE *fp;
  
  /* lock the log mutex: */
  tme_mutex_lock(&_tmesh_log_mutex);

  /* if this is an error, report it to stderr, else it goes to the
     log: */
  fp = (handle->tme_log_handle_errno != TME_OK
	? stderr
	: _tmesh_log);

  fprintf(fp,
	  "[%s.%lu]",
	  (char *) handle->tme_log_handle_private,
	  handle->tme_log_handle_level);

  if (handle->tme_log_handle_message != NULL) {
    fprintf(fp, ": %s", handle->tme_log_handle_message);
    tme_free(handle->tme_log_handle_message);
    handle->tme_log_handle_message = NULL;
  }
  
  if (handle->tme_log_handle_errno != TME_OK) {
    fprintf(fp, ": %s", strerror(handle->tme_log_handle_errno));
  }

  fputc('\n', fp);

  /* unlock the log mutex: */
  tme_mutex_unlock(&_tmesh_log_mutex);
}

/* our binary log output function: */
static void
_tmesh_log_output_binary(struct tme_log_handle *handle)
{
  struct _tmesh_log_message_binary *binary_message;
  tme_uint32_t handle_size_errno;
  struct _tmesh_log_message_binary binary_message_buffer;
  tme_uint32_t buffer_size;
  
  /* lock the log mutex: */
  tme_mutex_lock(&_tmesh_log_mutex);

  /* make values for the binary message header: */
  binary_message = (struct _tmesh_log_message_binary *) handle->tme_log_handle_private;
  handle_size_errno = binary_message->_tmesh_log_message_binary_handle_size_errno;
  TME_FIELD_MASK_DEPOSITU(handle_size_errno,
			  _TMESH_LOG_MESSAGE_BINARY_SIZE,
			  (handle->tme_log_handle_message_size
			   - sizeof(*binary_message)));
  TME_FIELD_MASK_DEPOSITU(handle_size_errno,
			  _TMESH_LOG_MESSAGE_BINARY_ERRNO,
			  handle->tme_log_handle_errno);

  /* write the binary message header: */
  binary_message = (struct _tmesh_log_message_binary *) handle->tme_log_handle_message;
  if (_TME_ALIGNOF_INT32_T == 1) {
    binary_message->_tmesh_log_message_binary_handle_size_errno = handle_size_errno;
    binary_message->_tmesh_log_message_binary_level = handle->tme_log_handle_level;
  }
  else {
    binary_message_buffer._tmesh_log_message_binary_handle_size_errno = handle_size_errno;
    binary_message_buffer._tmesh_log_message_binary_level = handle->tme_log_handle_level;
    memcpy(binary_message,
	   &binary_message_buffer,
	   sizeof(binary_message_buffer));
  }

  /* if there isn't enough room in the buffer for a maximum-sized
     message: */
  buffer_size
    = ((((char *) binary_message)
	+ handle->tme_log_handle_message_size)
       - (char *) handle->tme_log_handle_private);
  if (buffer_size
      > (_TMESH_LOG_MESSAGE_BINARY_BUFFER_SIZE
	 - TME_LOG_MESSAGE_SIZE_MAX_BINARY)) {

    /* write out the buffer: */
    fwrite(handle->tme_log_handle_private,
	   1,
	   buffer_size,
	   _tmesh_log);

    /* reset the buffer: */
    handle->tme_log_handle_message = handle->tme_log_handle_private;
  }

  /* otherwise, there is enough room in the buffer for a maximum-sized
     message: */
  else {

    /* advance the buffer: */
    handle->tme_log_handle_message += handle->tme_log_handle_message_size;
  }

  /* reset for the next message: */
  handle->tme_log_handle_message_size = sizeof(*binary_message);

  /* unlock the log mutex: */
  tme_mutex_unlock(&_tmesh_log_mutex);
}

/* our log open function: */
static void 
_tmesh_log_open(struct tmesh_support *support, 
		struct tme_log_handle *handle,
		const char *pathname, 
		const char *module)
{
  struct _tmesh_log_message_binary *binary_message;
  
  /* lock the log mutex: */
  tme_mutex_lock(&_tmesh_log_mutex);

  handle->tme_log_handle_level_max = 0;
  handle->tme_log_handle_mode = _tmesh_log_mode;

  /* if the log is binary: */
  if (handle->tme_log_handle_mode == TME_LOG_MODE_BINARY) {

    /* allocate the binary buffer: */
    handle->tme_log_handle_private = tme_malloc(_TMESH_LOG_MESSAGE_BINARY_BUFFER_SIZE);

    /* allocate the handle number and make the first message
       structure: */
    binary_message = (struct _tmesh_log_message_binary *) handle->tme_log_handle_private;
    memset (binary_message, 0, sizeof(*binary_message));
    TME_FIELD_MASK_DEPOSITU(binary_message->_tmesh_log_message_binary_handle_size_errno,
			    _TMESH_LOG_MESSAGE_BINARY_HANDLE,
			    _tmesh_log_handle_next);
    _tmesh_log_handle_next++;
    handle->tme_log_handle_message = (char *) binary_message;
    handle->tme_log_handle_message_size = sizeof(*binary_message);
    assert (handle->tme_log_handle_message_size < TME_LOG_MESSAGE_SIZE_MAX_BINARY);
    
    /* write a dummy first message for the handle that is only the
       pathname: */
    TME_FIELD_MASK_DEPOSITU(binary_message->_tmesh_log_message_binary_handle_size_errno,
			    _TMESH_LOG_MESSAGE_BINARY_SIZE,
			    strlen(pathname) + 1);
    fwrite(binary_message,
	   sizeof(*binary_message),
	   1,
	   _tmesh_log);
    fwrite(pathname,
	   1,
	   (strlen(pathname) + 1),
	   _tmesh_log);

    /* set the output function: */
    handle->tme_log_handle_output = _tmesh_log_output_binary;
    
    /* set the format hash: */
    handle->tme_log_handle_hash_format = _tmesh_log_hash_format;
  }

  /* otherwise, the log is text: */
  else {

    /* set the output function: */
    handle->tme_log_handle_message = NULL;
    handle->tme_log_handle_output = _tmesh_log_output;
    handle->tme_log_handle_private = tme_strdup(pathname);
  }

  /* unlock the log mutex: */
  tme_mutex_unlock(&_tmesh_log_mutex);
}

/* our log close function: */
static void 
_tmesh_log_close(struct tmesh_support *support, 
		 struct tme_log_handle *handle)
{
  tme_free(handle->tme_log_handle_private);
}

static int *passes;
/* our thread: */
static _tme_thret
_tmesh_th(void *_passes)
{
  int yield, rc;
  struct tmesh_io *io;
  struct _tmesh_input *input;
  char *output;
  unsigned int consumed;
  int i, num_passes = *(int *)_passes;
  
  if(!num_passes) tme_thread_enter(NULL);

  /* this command may have changed the current io, so reload: */
  io = _tmesh_io;
  input = io->tmesh_io_private;

  /* loop while we have a current input buffer: */
  for (;;) {

    yield = FALSE;
    
    /* run commands until we have to yield: */
    for (i=0; !_passes || (i<num_passes);i++) {

      /* all characters already read have been consumed: */
      consumed = input->_tmesh_input_buffer_tail;

      /* run a command: */
      rc = tmesh_eval(_tmesh, &output, &yield);

      /* if we're yielding: */
      if (yield) {

	/* if the current io has not changed, mark how many
	   characters were consumed by successful commands: */
	if (io == _tmesh_io) {
	  input->_tmesh_input_buffer_tail = consumed;
	}

	break;
      }

      /* this command may have changed the current io, so reload: */
      io = _tmesh_io;
      input = io->tmesh_io_private;

      /* display this command's output: */
      if (rc == TME_OK) {
	if (output != NULL
	    && *output != '\0') {
	  printf("%s\n", output);
	}
      }
      else {
	fprintf(stderr, "%s:%lu: ",
		io->tmesh_io_name,
		io->tmesh_io_input_line);
	if (output != NULL
	    && *output != '\0') {
	  fprintf(stderr, "%s: ", output);
	}
	fprintf(stderr, "%s\n", strerror(rc));
      }
      if (output != NULL) {
	tme_free(output);
      }
    }
    /* this command may have changed the current io, so reload: */
    io = _tmesh_io;
    input = io->tmesh_io_private;

    /* If return to console & running interactive, then put up the next prompt, else exit thread: */
    if((yield || (passes && !num_passes))
       && (input->_tmesh_input_handle == TME_STD_HANDLE(stdin))
       && isatty(fileno(stdin))
       && isatty(fileno(stdout)))
      if(num_passes) {
	if(yield) break;
      } else {
	printf("%s> ", argv0);
	fflush(stdout);
      }
    
    /* Reset number of passes for interactive operation */
    _passes = passes = NULL;
    
    /* remove all consumed characters: */
    _tmesh_remove_consumed(input);

    /* if the current input buffer is full, a command is too long: */
    if (input->_tmesh_input_buffer_head
	== sizeof(input->_tmesh_input_buffer)) {
      fprintf(stderr, "%s: command too long\n", argv0);
      input->_tmesh_input_buffer_head = 0;
    }

    /* try to read more input: */
    rc = (num_passes) ?
      (tme_thread_read_yield(
			     input->_tmesh_input_handle,
			     input->_tmesh_input_buffer
			     + input->_tmesh_input_buffer_head,
			     sizeof(input->_tmesh_input_buffer)
			     - input->_tmesh_input_buffer_head,
			     NULL)) : 
      (tme_thread_read(
		       input->_tmesh_input_handle,
		       input->_tmesh_input_buffer
		       + input->_tmesh_input_buffer_head,
		       sizeof(input->_tmesh_input_buffer)
		       - input->_tmesh_input_buffer_head));
    
    /* if the read failed: */
    if (rc < 0) {
      fprintf(stderr, "%s: %s\n",
	      io->tmesh_io_name,
	      strerror(errno));
      continue;
    }

    /* add characters in our current input buffer, or set EOF: */
    if (rc > 0) {
      input->_tmesh_input_buffer_head += rc;
    }
    else {
      input->_tmesh_input_buffer_eof = TRUE;
    }
  }
  
  if(!num_passes) tme_thread_exit();
}

static void
do_usage(const char *prog_name, char *msg)
{
  if (msg != NULL)
    fputs(msg, stderr);
  fprintf(stderr, "\
usage: %s [OPTIONS] INITIAL-CONFIG\n		\
where OPTIONS are:\n				\
  --log LOGFILE          log to LOGFILE\n			\
  -c, --noninteractive   read no commands from standard input\n	\
",
	  prog_name);
#ifdef TME_THREADS_POSIX
#define fpe(msg) fprintf(stderr, "\t%s", msg);          /* Shorter */
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
  fpe("--cpus            cpusetmask\n");
#endif // HAVE_PTHREAD_SETAFFINITY_NP
#ifdef HAVE_PTHREAD_SETSCHEDPARAM  
  fpe("-a <policy><prio> Set scheduling policy and priority in\n");
  fpe("                  thread attributes object\n");
  fpe("                  <policy> can be\n");
  fpe("                     f  SCHED_FIFO\n");
  fpe("                     r  SCHED_RR\n");
  fpe("                     o  SCHED_OTHER\n");
  fpe("-A                Use default thread attributes object\n");
  fpe("-i {e|i}          Set inherit scheduler attribute to\n");
  fpe("                  'explicit' or 'inherit'\n");
  fpe("-m <policy><prio> Set scheduling policy and priority on\n");
  fpe("                  main thread before pthread_create() call\n");
#endif // HAVE_PTHREAD_SETSCHEDPARAM  
#endif // TME_THREADS_POSIX
  exit(1);
}

#ifdef TME_THREADS_POSIX
#define handle_error_en(en, msg)					\
  do { errno = en; perror(msg); /* exit(EXIT_FAILURE) ;*/ } while (0)

#ifdef HAVE_PTHREAD_SETSCHEDPARAM  
static int
get_policy(char p, int *policy)
{
  switch (p) {
  case 'f': *policy = SCHED_FIFO;     return 1;
  case 'r': *policy = SCHED_RR;       return 1;
  case 'o': *policy = SCHED_OTHER;    return 1;
  default:  return 0;
  }
}

static void
display_sched_attr(int policy, struct sched_param *param)
{
  printf("    policy=%s, priority=%d\n",
	 (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
	 (policy == SCHED_RR)    ? "SCHED_RR" :
	 (policy == SCHED_OTHER) ? "SCHED_OTHER" :
	 "???",
	 param->sched_priority);
}

static void
display_thread_sched_attr(char *msg)
{
  int policy, s;
  struct sched_param param;

  s = pthread_getschedparam(pthread_self(), &policy, &param);
  if (s != 0)
    handle_error_en(s, "pthread_getschedparam");

  printf("%s\n", msg);
  display_sched_attr(policy, &param);
}
#endif // HAVE_PTHREAD_SETSCHEDPARAM  
#endif // TME_THREADS_POSIX

int
main(int argc, char **argv)
{
  int usage;
  const char *opt;
  int arg_i;
  const char *pre_threads_filename;
  const char *log_filename;
  int interactive, _passes;
  struct tmesh_io io;
  struct tmesh_support support;
  struct _tmesh_input *input_stdin;
  char *output;
  int rc;
  tme_threadid_t tmesh_thread;
  struct env_set *es;
#ifdef TME_THREADS_POSIX
  pthread_t thread;
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
  int j, cpus=-1;
  tme_cpuset_t cpuset;
#endif // HAVE_PTHREAD_SETAFFINITY_NP
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
  int inheritsched, use_null_attrib, policy;
  pthread_attr_t attr, *attrp;
  char *attr_sched_str, *main_sched_str, *inheritsched_str;
  struct sched_param param;

  use_null_attrib = 0;
  attr_sched_str = NULL;
  main_sched_str = NULL;
  inheritsched_str = NULL;
#endif // HAVE_PTHREAD_SETSCHEDPARAM
#endif // TME_THREADS_POSIX

  /* check our command line: */
  usage = FALSE;
  pre_threads_filename = NULL;
  log_filename = "/dev/null";
  interactive = TRUE;
  if ((argv0 = strrchr(argv[0], '/')) == NULL) argv0 = argv[0]; else argv0++;
  for (arg_i = 1;
       (arg_i < argc
	&& *argv[arg_i] == '-');
       arg_i++) {
    opt = argv[arg_i];
    if (!strcmp(opt, "--log")) {
      if (++arg_i < argc) {
	log_filename = argv[arg_i];
      }
      else {
	usage = TRUE;
	break;
      }
    }
    else if (!strcmp(opt, "--log-mode")) {
      ++arg_i;
      if (arg_i >= argc
	  || strcmp(argv[arg_i], "binary")) {
	usage = TRUE;
	break;
      }
      _tmesh_log_mode = TME_LOG_MODE_BINARY;
      if (_tmesh_log_hash_format == NULL) {
	_tmesh_log_hash_format = tme_hash_new(tme_direct_hash, tme_direct_compare, TME_HASH_DATA_NULL);
      }
    }
#ifdef TME_THREADS_POSIX
#ifdef HAVE_PTHREAD_SETAFFINITY_NP
    else if (!strcmp(opt, "--cpus")) {
      if (++arg_i < argc) {
	cpus=strtol(argv[arg_i], NULL, 0);
      } else {
	usage = TRUE;
	break;
      }
    }
#endif // HAVE_PTHREAD_SETAFFINITY_NP
#ifdef HAVE_PTHREAD_SETSCHEDPARAM  
    else if (!strcmp(opt, "-m")) {
      if (++arg_i < argc) {
	main_sched_str=argv[arg_i];
      } else {
	usage = TRUE;
	break;
      }
    }
    else if (!strcmp(opt, "-a")) {
      if (++arg_i < argc) {
	attr_sched_str=argv[arg_i];
      } else {
	usage = TRUE;
	break;
      }
    }
    else if (!strcmp(opt, "-A")) {
      use_null_attrib = 1;
    }
    else if (!strcmp(opt, "-i")) {
      if (++arg_i < argc) {
	inheritsched_str=argv[arg_i];
      } else {
	usage = TRUE;
	break;
      }
    }
#endif // HAVE_PTHREAD_SETSCHEDPARAM  
#endif // TME_THREADS_POSIX
    else if (!strcmp(opt, "-c")
	     || !strcmp(opt, "--noninteractive")) {
      interactive = FALSE;
    }
    else {
      if (strcmp(opt, "-h")
	  && strcmp(opt, "--help")
	  && strcmp(opt, "-h")) {
	fprintf(stderr, "%s: unknown option %s\n",
		argv0, opt);
      }
      usage = TRUE;
      break;
    }
  }
  if (arg_i < argc) {
    pre_threads_filename = argv[arg_i++];
  }
  else {
    usage = TRUE;
  }
  if (usage) do_usage(argv0, NULL);

  if(init_static()) {
    es = openvpn_setup(&argv[arg_i], argc - arg_i, NULL);
#ifdef WIN32
    set_win_sys_path_via_env(es);
    win32_signal_close(&win32_signal);
#endif
  } else
    exit(1);
  
  if (!strcmp(log_filename, "-")) {
    _tmesh_log = stdout;
  }
  else {
    _tmesh_log = fopen(log_filename, "a");
    if (_tmesh_log == NULL) {
      perror(log_filename);
      exit(1);
    }
  }

  /* initialize libtme: */
  tme_init();

  //  tme_thread_enter(NULL);
  
#ifdef TME_THREADS_POSIX
  thread = pthread_self();

#ifdef HAVE_PTHREAD_SETAFFINITY_NP
  /* Set affinity mask to include CPUs */

  tme_cpuset_init(cpuset);
  for (j = 0; j < sizeof(cpus); j++)
    if(cpus & (1<<j))
      tme_cpuset_set(j, cpuset);
  
  rc = pthread_setaffinity_np(thread, tme_cpuset_size(cpuset), tme_cpuset_ref(cpuset));
  if (rc != 0)
    handle_error_en(rc, "pthread_setaffinity_np");
  
  /* Check the actual affinity mask assigned to the thread */
  
  rc = pthread_getaffinity_np(thread, tme_cpuset_size(cpuset), tme_cpuset_ref(cpuset));
  if (rc != 0)
    handle_error_en(rc, "pthread_getaffinity_np");

  printf("Set returned by pthread_getaffinity_np() contained:\n");
  
  for (j = 0; j < sizeof(cpus); j++)
    if (tme_cpuset_isset(j, cpuset))
      printf("    CPU %d\n", j);
#endif // HAVE_PTHREAD_SETAFFINITY_NP

#ifdef HAVE_PTHREAD_SETSCHEDPARAM  
  if (use_null_attrib &&
      (inheritsched_str != NULL || attr_sched_str != NULL)) {
    do_usage(argv0, "Can't specify -A with -i or -a\n");
  }

  if (main_sched_str != NULL) {
    if (!get_policy(main_sched_str[0], &policy))
      do_usage(argv0, "Bad policy for main thread (-m)\n");
    param.sched_priority = strtol(&main_sched_str[1], NULL, 0);
    
    rc = pthread_setschedparam(pthread_self(), policy, &param);
    if (rc != 0)
      handle_error_en(rc, "pthread_setschedparam");
  }
  
  display_thread_sched_attr("Scheduler settings of main thread");
  printf("\n");

  /* Initialize thread attributes object according to options */

  attrp = NULL;

  if (!use_null_attrib) {
    rc = pthread_attr_init(&attr);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_init");
    attrp = &attr;
  }

  if (inheritsched_str != NULL) {
    if (inheritsched_str[0] == 'e')
      inheritsched = PTHREAD_EXPLICIT_SCHED;
    else if (inheritsched_str[0] == 'i')
      inheritsched = PTHREAD_INHERIT_SCHED;
    else
      do_usage(argv0, "Value for -i must be 'e' or 'i'\n");

    rc = pthread_attr_setinheritsched(&attr, inheritsched);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_setinheritsched");
  }

  if (attr_sched_str != NULL) {
    if (!get_policy(attr_sched_str[0], &policy))
      do_usage(argv0,
	       "Bad policy for 'attr' (-a)\n");
    param.sched_priority = strtol(&attr_sched_str[1], NULL, 0);

    rc = pthread_attr_setschedpolicy(&attr, policy);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_setschedpolicy");
    rc = pthread_attr_setschedparam(&attr, &param);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_setschedparam");
  }

  tme_thread_set_defattr(attrp);
  
  /* If we initialized a thread attributes object, display
     the scheduling attributes that were set in the object */

  if (attrp != NULL) {
    rc = pthread_attr_getschedparam(&attr, &param);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_getschedparam");
    rc = pthread_attr_getschedpolicy(&attr, &policy);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_getschedpolicy");

    printf("Scheduler settings in 'attr'\n");
    display_sched_attr(policy, &param);

    rc = pthread_attr_getinheritsched(&attr, &inheritsched);
    printf("    inheritsched is %s\n",
	   (inheritsched == PTHREAD_INHERIT_SCHED)  ? "INHERIT" :
	   (inheritsched == PTHREAD_EXPLICIT_SCHED) ? "EXPLICIT" :
	   "???");
    printf("\n");
  }
#endif // HAVE_PTHREAD_SETSCHEDPARAM  
#endif

  /* initialize libtmesh: */
  (void) tmesh_init();

  /* create our stdin input buffer, and stuff it with the command to
     source the pre-threads commands: */
  input_stdin = tme_new0(struct _tmesh_input, 1);
  input_stdin->_tmesh_input_handle = TME_STD_HANDLE(stdin);

  snprintf(input_stdin->_tmesh_input_buffer,
	   sizeof(input_stdin->_tmesh_input_buffer) - 1,
	   "source %s\n",
	   pre_threads_filename);
  input_stdin->_tmesh_input_buffer[sizeof(input_stdin->_tmesh_input_buffer) - 1] = '\0';
  input_stdin->_tmesh_input_buffer_head = strlen(input_stdin->_tmesh_input_buffer);

  /* create our stdin io: */
  io.tmesh_io_name = "*stdin*";
  io.tmesh_io_private = input_stdin;
  io.tmesh_io_input_line = 0;
  io.tmesh_io_getc = _tmesh_getc;
  io.tmesh_io_close = _tmesh_close;
  io.tmesh_io_open = _tmesh_open;
  _tmesh_io = &io;

  /* create our support: */
  tme_mutex_init(&_tmesh_log_mutex);
  support.tmesh_support_log_open = _tmesh_log_open;
  support.tmesh_support_log_close = _tmesh_log_close;

  /* create our shell: */
  _tmesh = tmesh_new(&support, &io);

  /* evaluate pre-threads once (to avoid synchronization problems with init code): */
  _passes = 1;
  _tmesh_th(passes = &_passes);
  
  /* create our thread: */
  if(interactive) {
    _passes = 0;
    tme_thread_create(&tmesh_thread, (tme_thread_t) _tmesh_th, passes = &_passes);
  }

  /* run the threads: */
  tme_threads_run();

#if defined(TME_THREADS_POSIX) && defined(HAVE_PTHREAD_SETSCHEDPARAM)
  /* Destroy unneeded thread attributes object */

  if (!use_null_attrib) {
    rc = pthread_attr_destroy(&attr);
    if (rc != 0)
      handle_error_en(rc, "pthread_attr_destroy");
  }
#endif

  if(interactive)
    tme_thread_join(tmesh_thread);

  /* done: */
  exit(0);
}
