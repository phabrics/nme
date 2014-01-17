dnl $Id: acinclude.m4,v 1.10 2010/02/15 14:51:50 fredette Exp $

dnl acinclude.m4 - additional tme autoconf macros:

dnl Copyright (c) 2001, 2003, 2004 Matt Fredette
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl 3. All advertising materials mentioning features or use of this software
dnl    must display the following acknowledgement:
dnl      This product includes software developed by Matt Fredette.
dnl 4. The name of the author may not be used to endorse or promote products
dnl    derived from this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
dnl INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
dnl (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
dnl SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
dnl HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
dnl STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
dnl ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.

dnl AC_CHECK_ALIGNOF(BITS)
AC_DEFUN([AC_CHECK_ALIGNOF],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(alignof_int$1_t, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_alignof_int$1_t, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(minimum alignment of int$1_t)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
#if (SIZEOF_SHORT * 8) == $1
#define _type short
#elif (SIZEOF_INT * 8) == $1
#define _type int
#else
#define _type long
#endif
  char try_align_buffer[sizeof(_type) * 2];
  int min_align, try_align, status;
  _type value;
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  min_align = sizeof(_type);
  for(try_align = sizeof(_type); try_align-- > 1;) {
    switch(fork()) {
    case -1: exit(1);
    case 0: value = *((_type *) &try_align_buffer[try_align]); 
      fprintf(stderr, "%d\n", (int) (value / 2)); exit(0);
    default: break;
    }
    wait(&status);
    if (!status && try_align < min_align) {
      min_align = try_align;
    }
  }
  fprintf(f, "%d\n", min_align * 8);
  exit(0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=$1, AC_CV_NAME=$1)])dnl
AC_CV_NAME=`expr $AC_CV_NAME / 8`
AC_MSG_RESULT($AC_CV_NAME)
AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME, [Define to the minimum alignment, in bytes, of int$1_t.])
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl AC_CHECK_SHIFTMAX(BITS)
AC_DEFUN([AC_CHECK_SHIFTMAX],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(shiftmax_int$1_t, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_shiftmax_int$1_t, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(maximum shift count for int$1_t)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
#if 8 == $1
#define _type char
#elif (SIZEOF_SHORT * 8) == $1
#define _type short
#elif (SIZEOF_INT * 8) == $1
#define _type int
#elif (SIZEOF_LONG * 8) == $1
#define _type long
#endif
  _type center, right, left;
  unsigned int shift, max_shift;
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  center = 3;
  center <<= ((sizeof(center) * 4) - 1);
  max_shift = 2047;
  sscanf("0", "%d", &shift);
  for (shift += (sizeof(center) * 8);
       shift < 2048;
       shift <<= 1) {
    right = (center >> shift);
    left = (center << shift);
    if (right != 0
	|| left != 0) {
      right = (center >> (shift | 1));
      left = (center << (shift | 1));
      max_shift = ((right == (center >> 1)
		    && left == (center << 1))
		   ? shift - 1
		   : (sizeof(center) * 8) - 1);
      break;
    }
  }
  fprintf(f, "%d\n", max_shift + 1);
  exit(0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=$1, AC_CV_NAME=$1)])dnl
AC_CV_NAME=`expr $AC_CV_NAME - 1`
AC_MSG_RESULT($AC_CV_NAME)
AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME, [Define to the maximum shift count for a int$1_t.])
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl AC_CHECK_SHIFTSIGNED(BITS)
AC_DEFUN([AC_CHECK_SHIFTSIGNED],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(shiftsigned_int$1_t, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_shiftsigned_int$1_t, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(for arithmetic right shifts of int$1_t)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
#if 8 == $1
#define _type signed char
#elif (SIZEOF_SHORT * 8) == $1
#define _type signed short
#elif (SIZEOF_INT * 8) == $1
#define _type signed int
#elif (SIZEOF_LONG * 8) == $1
#define _type signed long
#endif
  _type prime, shifted, dividend;
  unsigned int shift, divides;
  FILE *f;

  f=fopen("conftestval", "w");
  if (!f) exit(1);

  /* set value to the largest magnitude negative prime that it 
     will hold: */
#if $1 == 8
  prime = -127;
#elif $1 == 16
  prime = -32749;
#elif $1 == 32
  prime = -2147483647;
#elif $1 == 64
  /* this is a crafty way of constructing -9223372036854775783,
     which is 0x8000000000000000 - 0x19, without having to know 
     any compiler suffix for 64-bit literals: */
  prime = 1;
  prime <<= ($1 - 2);
  prime = -prime;
  prime *= 2;
  prime -= 0x19;
#else
#error "need another prime"
#endif

  /* all right shifts from a single bit up to SHIFTMAX_INT$1_T
     must be arithmetic, otherwise shifts aren't arithmetic: */
  for (shift = 1; 
       shift <= SHIFTMAX_INT$1_T;
       shift++) {
    shifted = prime >> shift;
    dividend = prime;
    divides = 0;
    do {
      dividend &= ~((_type) 1); /* guarantee no remainder */
      dividend /= 2;
    } while (++divides < shift);
    if (shifted != dividend) {
      break;
    }
  }

  fprintf(f, "%d\n", (shift > SHIFTMAX_INT$1_T ? 1 : 0));
  exit(0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=0, AC_CV_NAME=0)])dnl
if test $AC_CV_NAME = 1; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(AC_TYPE_NAME, [], [Define if all right shifts of int$1_t are arithmetic.])
else
  AC_MSG_RESULT(no)
fi
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl AC_CHECK_FLOAT_FORMAT(TYPE)
AC_DEFUN([AC_CHECK_FLOAT_FORMAT],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(FLOAT_FORMAT_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_float_format_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(the floating point format of $1)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
  $1 value;
  unsigned short value_buffer[16];
#ifdef WORDS_BIGENDIAN
#define UINT16_PART(v, i) (value_buffer[(sizeof(v) / sizeof(value_buffer[0])) - ((i) + 1)])
#else
#define UINT16_PART(v, i) (value_buffer[(i)])
#endif

  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);

  /* see if this type is IEEE754 single precision: */
  value = 3.4793309169e-05;
  memcpy(value_buffer, &value, sizeof(value));
  if (sizeof(value) == 4
      && UINT16_PART(value, 1) == 0x3811
      && UINT16_PART(value, 0) == 0xef08) {
    fprintf(f, "IEEE754_SINGLE\n");
    exit (0);
  }

  /* see if this type is IEEE754 double precision: */
  value = 7.91534994289814532176e-04;
  memcpy(value_buffer, &value, sizeof(value));
  if (sizeof(value) == 8
      && UINT16_PART(value, 3) == 0x3f49
      && UINT16_PART(value, 2) == 0xefe0
      && UINT16_PART(value, 1) == 0x7501
      && UINT16_PART(value, 0) == 0xb288) {
    fprintf(f, "IEEE754_DOUBLE\n");
    exit (0);
  }

  /* see if this type might be IEEE754 extended 80-bit precision: */
  value = 1.1356523406294143949491931077970765006170e+04;
  memcpy(value_buffer, &value, sizeof(value));
  if (sizeof(value) >= 12
      && UINT16_PART(value, 0) == 0x7800
      && UINT16_PART(value, 1) == 0xd1cf
      && UINT16_PART(value, 2) == 0x17f7
      && UINT16_PART(value, 3) == 0xb172) {

    /* check for i387-style IEEE754 extended precision: */
    if (sizeof(value) == 12
	&& UINT16_PART(value, 4) == 0x400c) {
      fprintf(f, "IEEE754_EXTENDED80_I387\n");
      exit (0);
    }

    /* check for m68881-style IEEE754 extended precision: */
    if (sizeof(value) == 12
	&& UINT16_PART(value, 5) == 0x400c) {
      fprintf(f, "IEEE754_EXTENDED80_M68881\n");
      exit (0);
    }
  }

  /* otherwise, this is some native type: */
  fprintf(f, "NATIVE\n");
  exit (0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=NATIVE, AC_CV_NAME=NATIVE)])dnl
AC_MSG_RESULT($AC_CV_NAME)
if test $AC_CV_NAME != NATIVE; then
  AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME, [Define to the floating point format of a $1.])
fi
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl AC_CHECK_FLOAT_LIMITS(TYPE, LIMITS)
AC_DEFUN([AC_CHECK_FLOAT_LIMITS],
[changequote(<<, >>)dnl
dnl The names to #define.
define(<<AC_TYPE_NAME_MAX>>, translit(FLOAT_MAX_$1, [a-z *], [A-Z_P]))dnl
define(<<AC_TYPE_NAME_MIN>>, translit(FLOAT_MIN_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_float_limits_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(the limits of $1)
AC_CACHE_VAL(AC_CV_NAME,
[for limits in $2; do
  max=`echo $limits | sed -e 's%^\(.*\)/\(.*\)$%\1%'`
  min=`echo $limits | sed -e 's%^\(.*\)/\(.*\)$%\2%'`
  AC_TRY_COMPILE([#include <sys/types.h>
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif], [ $1 x; x = $max - $min; ], [AC_CV_NAME=$limits ; break], AC_CV_NAME= )
done])
if test "x$AC_CV_NAME" = x; then
  AC_MSG_ERROR(can't determine the limits of $1)
fi
AC_MSG_RESULT($AC_CV_NAME)
max=`echo $AC_CV_NAME | sed -e 's%^\(.*\)/\(.*\)$%\1%'`
min=`echo $AC_CV_NAME | sed -e 's%^\(.*\)/\(.*\)$%\2%'`
AC_DEFINE_UNQUOTED(AC_TYPE_NAME_MAX, ($max), [Define to the maximum value of a $1.])
AC_DEFINE_UNQUOTED(AC_TYPE_NAME_MIN, ($min), [Define to the minimum value of a $1.])
undefine([AC_TYPE_NAME_MAX])dnl
undefine([AC_TYPE_NAME_MIN])dnl
undefine([AC_CV_NAME])dnl
])

dnl AC_CHECK_FUNC_LONG(FUNC, FUNCTION-BODY, [INCLUDES, [LIBRARIES]])
AC_DEFUN([AC_CHECK_FUNC_LONG],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_FUNC_NAME>>, translit(have_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_have_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(AC_CV_NAME,
[ac_func_long_LIBS=$LIBS
LIBS="${LIBS-} $4"
AC_TRY_LINK([
$3
], [
$2
], AC_CV_NAME=yes, AC_CV_NAME=no)
LIBS=$ac_func_long_LIBS])dnl
AC_MSG_RESULT($AC_CV_NAME)
if test $AC_CV_NAME = yes; then
  AC_DEFINE_UNQUOTED(AC_FUNC_NAME, [], [Define if you have $1().])
fi
undefine([AC_FUNC_NAME])dnl
undefine([AC_CV_NAME])dnl
])

dnl The AC_HEADER_CHECK_PROTOTYPE, AC_HEADER_CHECK_PROTOTYPES,
dnl and AC_SYS_SOCKADDR_SA_LEN macros bear the following copyright:
dnl
dnl Copyright (C) 1997,1998,1999 by the Massachusetts Institute of Technology,
dnl Cambridge, MA, USA.  All Rights Reserved.
dnl
dnl This software is being provided to you, the LICENSEE, by the
dnl Massachusetts Institute of Technology (M.I.T.) under the following
dnl license.  By obtaining, using and/or copying this software, you agree
dnl that you have read, understood, and will comply with these terms and
dnl conditions:
dnl
dnl WITHIN THOSE CONSTRAINTS, permission to use, copy, modify and distribute
dnl this software and its documentation for any purpose and without fee or
dnl royalty is hereby granted, provided that you agree to comply with the
dnl following copyright notice and statements, including the disclaimer, and
dnl that the same appear on ALL copies of the software and documentation,
dnl including modifications that you make for internal use or for
dnl distribution:
dnl
dnl THIS SOFTWARE IS PROVIDED "AS IS", AND M.I.T. MAKES NO REPRESENTATIONS
dnl OR WARRANTIES, EXPRESS OR IMPLIED.  By way of example, but not
dnl limitation, M.I.T. MAKES NO REPRESENTATIONS OR WARRANTIES OF
dnl MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF
dnl THE LICENSED SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY THIRD PARTY
dnl PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS.
dnl
dnl The name of the Massachusetts Institute of Technology or M.I.T. may NOT
dnl be used in advertising or publicity pertaining to distribution of the
dnl software.  Title to copyright in this software and any associated
dnl documentation shall at all times remain with M.I.T., and USER agrees to
dnl preserve same.

dnl AC_HEADER_CHECK_PROTOTYPE(FUNCTION, INCLUDES, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([AC_HEADER_CHECK_PROTOTYPE], 
[AC_MSG_CHECKING([for a prototype for $1])
AC_CACHE_VAL(ac_cv_proto_$1,
[AC_TRY_COMPILE($2 [
struct bonch { int a, b; };
struct bonch $1();
], , eval "ac_cv_proto_$1=no", eval "ac_cv_proto_$1=yes")])
if eval "test \"`echo '$ac_cv_proto_'$1`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], , :, [$3])
else
  AC_MSG_RESULT(no)
ifelse([$4], , , [$4
])dnl
fi
])

dnl AC_HEADER_CHECK_PROTOTYPES(INCLUDES, FUNCTION...)
AC_DEFUN([AC_HEADER_CHECK_PROTOTYPES],
[for ac_func in $2
do
AC_HEADER_CHECK_PROTOTYPE($ac_func, $1, 
[changequote(, )dnl
  ac_tr_func=PROTO_`echo $ac_func | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_func)])dnl
done
])

dnl AC_SYS_SOCKADDR_SA_LEN
AC_DEFUN([AC_SYS_SOCKADDR_SA_LEN],
[AC_MSG_CHECKING([for sa_len in struct sockaddr])
AC_CACHE_VAL(ac_cv_sys_sockaddr_sa_len,
[AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
], [
int length;
struct sockaddr sock;
length = sock.sa_len;
], ac_cv_sys_sockaddr_sa_len=yes, ac_cv_sys_sockaddr_sa_len=no)])dnl
if test $ac_cv_sys_sockaddr_sa_len = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE_UNQUOTED(HAVE_SOCKADDR_SA_LEN, [], [Define if your struct sockaddr has sa_len.])
else
  AC_MSG_RESULT(no)
fi
])dnl
