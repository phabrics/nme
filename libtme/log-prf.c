/* $Id: log-prf.c,v 1.4 2009/08/30 16:56:42 fredette Exp $ */

/* libtme/log-prf.c - a printf function body: */

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

{
  
  /* start the variable arguments: */
#ifdef HAVE_STDARG_H
  va_start(prf_args, prf_format);
#else  /* HAVE_STDARG_H */
  va_start(prf_args);
#endif /* HAVE_STDARG_H */

  /* we are in state zero and our aggregate begins at the
     beginning of the format: */
  prf_state = 0;
  prf_agg = prf_format;

  /* to silence gcc -Wuninitialized: */
  prf_flag_ls = (const char *) NULL;
  prf_flag_0 = FALSE;
  prf_width = -1;

  /* process format characters until we get to the NUL: */
  for(;;) {

    /* get the next format character: */
    prf_char = *(prf_format++);

    /* handle a 'NUL' specially: */
    if (prf_char == '\0') {

      /* if we were in state zero, dump any aggregate: */
      if (prf_state == 0) {
	if ((prf_format - prf_agg) > 1) {
	  PRF_OUT_MEM(prf_agg, (prf_format - prf_agg) - 1);
	}
      }

      /* we're done: */
      break;
    }

    /* dispatch on our state: */
    switch (prf_state) {

      /* state 0: "": */
    case 0:

      /* on a '%', dump any aggregate and move to state one: */
      if (prf_char == '%') {
	assert(prf_format > prf_agg);
	if ((prf_format - prf_agg) > 1) {
	  PRF_OUT_MEM(prf_agg, (prf_format - prf_agg) - 1);
	}
	prf_state = 1;
      }

      /* otherwise, remain in state zero, and the character gets added
	 automatically to the aggregate: */
      else {
	/* nothing */
      }
      break;

      /* state 1: "%": */
    case 1:

      /* if this is another '%', print a single '%' and move to state zero: */
      if (prf_char == '%') {
	PRF_OUT_CHAR('%');
	prf_agg = prf_format;
	prf_state = 0;
	break;
      }

      /* reset all flags, precisions, etc., and enter state two: */
      prf_flag_ls = "ll" + 2;
      prf_flag_0 = FALSE;
      prf_width = -1;
      prf_state = 2;

      /* FALLTHROUGH */

      /* state 2: "%" followed by zero or more flags, precisions, etc: */
    case 2:

      /* dispatch on this character: */
      prf_digit = 9;
      switch (prf_char) {

	/* the 'l' flag: */
      case 'l': prf_flag_ls -= (prf_flag_ls[0] == '\0' || prf_flag_ls[1] == '\0'); break;

	/* a width: */
      case '0':
	if (prf_width < 0) {
	  prf_flag_0 = TRUE;
	}
	prf_digit--;
	/* FALLTHROUGH */
      case '1': prf_digit--;
      case '2': prf_digit--;
      case '3': prf_digit--;
      case '4': prf_digit--;
      case '5': prf_digit--;
      case '6': prf_digit--;
      case '7': prf_digit--;
      case '8': prf_digit--;
      case '9':
	if (prf_width < 0) {
	  prf_width = 0;
	}
	prf_width = (prf_width * 10) + prf_digit;
	break;

	/* the 'd', 'u', 'x', and 'X' conversions: */
      case 'd':
      case 'u':
      case 'x':
      case 'X':
	if (prf_width < 0) {
	  sprintf(prf_format_buffer,
		  "%%%s%c",
		  prf_flag_ls,
		  prf_char);
	  prf_width = 0;
	}
	else {
	  sprintf(prf_format_buffer, 
		  "%%%s%d%s%c",
		  (prf_flag_0
		   ? "0"
		   : ""),
		  prf_width,
		  prf_flag_ls,
		  prf_char);
	}
	/* NB: we always allocate at least the specified field width,
	   plus the worst-case number of characters needed to
	   represent the value in decimal, plus one for the NUL.  this
	   should avoid buffer overflows entirely: */
	if (prf_flag_ls[0] == '\0') {
	  prf_value_d = va_arg(prf_args, int);
	  prf_value_buffer = tme_new(char, prf_width + ((sizeof(prf_value_d) * 3) + 1));
	  sprintf(prf_value_buffer, prf_format_buffer, prf_value_d);
	  PRF_OUT_ARG_CODE(TME_LOG_ARG_CODE_INT);
	}
	else if (prf_flag_ls[1] == '\0') {
	  prf_value_ld = va_arg(prf_args, long int);
	  prf_value_buffer = tme_new(char, prf_width + ((sizeof(prf_value_ld) * 3) + 1));
	  sprintf(prf_value_buffer, prf_format_buffer, prf_value_ld);
	  PRF_OUT_ARG_CODE(TME_LOG_ARG_CODE_LONG_INT);
	}
	else {
#if (0 prf_lld(+ 1))
	  prf_value_lld = va_arg(prf_args, long long int);
	  prf_value_buffer = tme_new(char, prf_width + ((sizeof(prf_value_lld) * 3) + 1));
	  sprintf(prf_value_buffer, prf_format_buffer, prf_value_lld);
	  PRF_OUT_ARG_CODE(TME_LOG_ARG_CODE_LONG_LONG_INT);
#else  /* long long int not supported */
	  abort();
#endif /* long long int not supported */
	}
	PRF_OUT_MEM(prf_value_buffer, strlen(prf_value_buffer));
	tme_free(prf_value_buffer);

	/* enter state zero: */
	prf_agg = prf_format;
	prf_state = 0;
	break;

	/* the 's' conversion: */
      case 's':
	prf_value_s = va_arg(prf_args, const char *);
	PRF_OUT_ARG_CODE(TME_LOG_ARG_CODE_STRING);
	PRF_OUT_MEM(prf_value_s, strlen(prf_value_s));

	/* enter state zero: */
	prf_agg = prf_format;
	prf_state = 0;
	break;

	/* the 'c' conversion: */
      case 'c':
	prf_value_c = va_arg(prf_args, int);
	PRF_OUT_ARG_CODE(TME_LOG_ARG_CODE_CHAR);
	PRF_OUT_CHAR(prf_value_c);

	/* enter state zero: */
	prf_agg = prf_format;
	prf_state = 0;
	break;

	/* ignore anything else: */
      default:
	prf_agg = prf_format;
	prf_state = 0;
	break;
      }
      break;

    default:
      assert(FALSE);
    }
  }

  /* end the variable arguments: */
  va_end(prf_args);
}
