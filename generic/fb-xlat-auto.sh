#! /bin/sh
# Generated from fb-xlat-auto.m4 by GNU Autoconf 2.72.
## -------------------- ##
## M4sh Initialization. ##
## -------------------- ##

# Be more Bourne compatible
DUALCASE=1; export DUALCASE # for MKS sh
if test ${ZSH_VERSION+y} && (emulate sh) >/dev/null 2>&1
then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else case e in #(
  e) case `(set -o) 2>/dev/null` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac ;;
esac
fi



# Reset variables that may have inherited troublesome values from
# the environment.

# IFS needs to be set, to space, tab, and newline, in precisely that order.
# (If _AS_PATH_WALK were called with IFS unset, it would have the
# side effect of setting IFS to empty, thus disabling word splitting.)
# Quoting is to prevent editors from complaining about space-tab.
as_nl='
'
export as_nl
IFS=" ""	$as_nl"

PS1='$ '
PS2='> '
PS4='+ '

# Ensure predictable behavior from utilities with locale-dependent output.
LC_ALL=C
export LC_ALL
LANGUAGE=C
export LANGUAGE

# We cannot yet rely on "unset" to work, but we need these variables
# to be unset--not just set to an empty or harmless value--now, to
# avoid bugs in old shells (e.g. pre-3.0 UWIN ksh).  This construct
# also avoids known problems related to "unset" and subshell syntax
# in other old shells (e.g. bash 2.01 and pdksh 5.2.14).
for as_var in BASH_ENV ENV MAIL MAILPATH CDPATH
do eval test \${$as_var+y} \
  && ( (unset $as_var) || exit 1) >/dev/null 2>&1 && unset $as_var || :
done

# Ensure that fds 0, 1, and 2 are open.
if (exec 3>&0) 2>/dev/null; then :; else exec 0</dev/null; fi
if (exec 3>&1) 2>/dev/null; then :; else exec 1>/dev/null; fi
if (exec 3>&2)            ; then :; else exec 2>/dev/null; fi

# The user is always right.
if ${PATH_SEPARATOR+false} :; then
  PATH_SEPARATOR=:
  (PATH='/bin;/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 && {
    (PATH='/bin:/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 ||
      PATH_SEPARATOR=';'
  }
fi


# Find who we are.  Look in the path if we contain no directory separator.
as_myself=
case $0 in #((
  *[\\/]* ) as_myself=$0 ;;
  *) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
for as_dir in $PATH
do
  IFS=$as_save_IFS
  case $as_dir in #(((
    '') as_dir=./ ;;
    */) ;;
    *) as_dir=$as_dir/ ;;
  esac
    test -r "$as_dir$0" && as_myself=$as_dir$0 && break
  done
IFS=$as_save_IFS

     ;;
esac
# We did not find ourselves, most probably we were run as 'sh COMMAND'
# in which case we are not to be found in the path.
if test "x$as_myself" = x; then
  as_myself=$0
fi
if test ! -f "$as_myself"; then
  printf "%s\n" "$as_myself: error: cannot find myself; rerun with an absolute file name" >&2
  exit 1
fi


if test "x$CONFIG_SHELL" = x; then
  as_bourne_compatible="if test \${ZSH_VERSION+y} && (emulate sh) >/dev/null 2>&1
then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on \${1+\"\$@\"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '\${1+\"\$@\"}'='\"\$@\"'
  setopt NO_GLOB_SUBST
else case e in #(
  e) case \`(set -o) 2>/dev/null\` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac ;;
esac
fi
"
  as_required="as_fn_return () { (exit \$1); }
as_fn_success () { as_fn_return 0; }
as_fn_failure () { as_fn_return 1; }
as_fn_ret_success () { return 0; }
as_fn_ret_failure () { return 1; }

exitcode=0
as_fn_success || { exitcode=1; echo as_fn_success failed.; }
as_fn_failure && { exitcode=1; echo as_fn_failure succeeded.; }
as_fn_ret_success || { exitcode=1; echo as_fn_ret_success failed.; }
as_fn_ret_failure && { exitcode=1; echo as_fn_ret_failure succeeded.; }
if ( set x; as_fn_ret_success y && test x = \"\$1\" )
then :

else case e in #(
  e) exitcode=1; echo positional parameters were not saved. ;;
esac
fi
test x\$exitcode = x0 || exit 1
blah=\$(echo \$(echo blah))
test x\"\$blah\" = xblah || exit 1
test -x / || exit 1"
  as_suggested=""
  if (eval "$as_required") 2>/dev/null
then :
  as_have_required=yes
else case e in #(
  e) as_have_required=no ;;
esac
fi
  if test x$as_have_required = xyes && (eval "$as_suggested") 2>/dev/null
then :

else case e in #(
  e) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
as_found=false
for as_dir in /bin$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR$PATH
do
  IFS=$as_save_IFS
  case $as_dir in #(((
    '') as_dir=./ ;;
    */) ;;
    *) as_dir=$as_dir/ ;;
  esac
  as_found=:
  case $as_dir in #(
	 /*)
	   for as_base in sh bash ksh sh5; do
	     # Try only shells that exist, to save several forks.
	     as_shell=$as_dir$as_base
	     if { test -f "$as_shell" || test -f "$as_shell.exe"; } &&
		    as_run=a "$as_shell" -c "$as_bourne_compatible""$as_required" 2>/dev/null
then :
  CONFIG_SHELL=$as_shell as_have_required=yes
		   break 2
fi
	   done;;
       esac
  as_found=false
done
IFS=$as_save_IFS
if $as_found
then :

else case e in #(
  e) if { test -f "$SHELL" || test -f "$SHELL.exe"; } &&
	      as_run=a "$SHELL" -c "$as_bourne_compatible""$as_required" 2>/dev/null
then :
  CONFIG_SHELL=$SHELL as_have_required=yes
fi ;;
esac
fi


      if test "x$CONFIG_SHELL" != x
then :
  export CONFIG_SHELL
             # We cannot yet assume a decent shell, so we have to provide a
# neutralization value for shells without unset; and this also
# works around shells that cannot unset nonexistent variables.
# Preserve -v and -x to the replacement shell.
BASH_ENV=/dev/null
ENV=/dev/null
(unset BASH_ENV) >/dev/null 2>&1 && unset BASH_ENV ENV
case $- in # ((((
  *v*x* | *x*v* ) as_opts=-vx ;;
  *v* ) as_opts=-v ;;
  *x* ) as_opts=-x ;;
  * ) as_opts= ;;
esac
exec $CONFIG_SHELL $as_opts "$as_myself" ${1+"$@"}
# Admittedly, this is quite paranoid, since all the known shells bail
# out after a failed 'exec'.
printf "%s\n" "$0: could not re-execute with $CONFIG_SHELL" >&2
exit 255
fi

    if test x$as_have_required = xno
then :
  printf "%s\n" "$0: This script requires a shell more modern than all"
  printf "%s\n" "$0: the shells that I found on your system."
  if test ${ZSH_VERSION+y} ; then
    printf "%s\n" "$0: In particular, zsh $ZSH_VERSION has bugs and should"
    printf "%s\n" "$0: be upgraded to zsh 4.3.4 or later."
  else
    printf "%s\n" "$0: Please tell bug-autoconf@gnu.org about your system,
$0: including any error possibly output before this
$0: message. Then install a modern shell, or manually run
$0: the script under such a shell if you do have one."
  fi
  exit 1
fi ;;
esac
fi
fi
SHELL=${CONFIG_SHELL-/bin/sh}
export SHELL
# Unset more variables known to interfere with behavior of common tools.
CLICOLOR_FORCE= GREP_OPTIONS=
unset CLICOLOR_FORCE GREP_OPTIONS

## --------------------- ##
## M4sh Shell Functions. ##
## --------------------- ##
# as_fn_unset VAR
# ---------------
# Portably unset VAR.
as_fn_unset ()
{
  { eval $1=; unset $1;}
}
as_unset=as_fn_unset

## -------------------- ##
## Main body of script. ##
## -------------------- ##


# $Id: fb-xlat-auto.sh,v 1.12 2009/08/30 21:51:53 fredette Exp $

# generic/fb-xlat-auto.sh - automatically generates C code
# for many framebuffer translation functions:

#
# Copyright (c) 2003, 2005 Matt Fredette
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Matt Fredette.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

# this script generates one or more C functions.  each function
# translates an image from a source image format into a destination
# image format, optionally halving or doubling the image size in the
# process.
#
# the source and destination image formats are represented by "keys",
# strings that describe the different attributes of a format.
#
# a source image format key has the form: WxHdDbBsSpPoO[cC][mMS][_rR_gG_bB][_X]
#
# a destination image format key has the form: dDbBsSpPoO[mM][_rR_gG_bB]
#
# all of the lowercase letters and underscores are literals, and all
# of the uppercase letters represent attribute values.  the different
# attributes are:
#
# W is the width of the source image.  if zero, the generated function
# will be able to translate source images of any width, but it will be
# suboptimal.  if nonzero, the generated function will only be able to
# translate source images of that width, but it will have some
# optimization.  this attribute is only used in source keys, since the
# destination width is always the scaled source width.
#
# H is the height of the source image.  if zero, the generated
# function will be able to translate source images of any height, but
# it will be suboptimal.  if nonzero, the generated function will only
# be able to translate source images of that height, but it will have
# some optimization.  this attribute is only used in source keys,
# since the destination height is always the scaled source height.
#
# D is the color (d)epth, in bits, of the source/destination image.
# if zero, the generated function will be able to handle
# source/destination images of any depth, but it will be suboptimal.
# if nonzero, the generated function will only be able to handle
# source/destination images of that depth, but it will have some
# optimization.
#
# B is the (b)its-per-pixel of the source/destination image.  this
# must be at least as large as the depth, and it may be larger.  if
# zero, the generated function will be able to handle
# source/destination images of any bits-per-pixel, but it will be
# suboptimal.  if nonzero, the generated function will only be able to
# handle source/destination images of that bits-per-pixel, but it will
# have some optimization.
#
# S is the (s)kip-x-bits of the source/destination image.  this is the
# number of bits at the beginning of each image scanline that are not
# displayed.  if an underscore, the generated function will be able to
# handle source/destination images with any skip-x-bits, but it will
# be suboptimal.  if not an underscore, the generated function will
# only be able to handle source/destination images with that many
# skip-x-bits, but it will have some optimization.
#
# P is the (p)adding of the source/destination image.  this is how
# many bits each image scanline is padded to.  if zero, the generated
# function will be able to handle source/destination images with any
# padding, but it will be suboptimal.  if nonzero, the generated
# function will only be able to handle source/destination images with
# that padding, but it will have some optimization.
#
# O is the byte and bit (o)rder of the source/destination image.  this
# is always m or l for most- or least-significant, respectively.
# there is no "any" wildcard value.  this script cannot handle image
# formats where a word-unit of pixels has one order for its bytes in
# memory, but the opposite order for its pixels within the word-unit.
#
# C is the optional class.  it is 'm' for a monochrome (grayscale) image,
# and 'c' for a color image.  if not given, an image with a depth of
# one is considered monochrome, and any other image is considered color.
#
# M is the optional mapping type.  it is 'l' for a monochrome image
# that linearly maps pixels into intensities and for a color image
# that linearly maps pixel subfields into intensities.  it is 'i' for
# images that index pixels or pixel subfields into intensities.  if
# not given, monochrome images and color images with subfields are
# assumed to be linear, and color images without subfields are assumed
# to be indexed.
#
# S is the optional mapping size. if the mapping type is given, this
# must be given, and it is the size of the mapping range - i.e., the
# number of bits per intensity.  if the mapping type is not given, and
# the image is linearly mapped monochrome, the mapping size is assumed
# to be the same as the image depth.
#
# R, G, and B are the optional subfield masks.  if an image's pixels
# can be decomposed into subfields for red, green, and blue, these are
# the masks for the subfields.
#
# X is the optional scaling.  it is `h' to halve the source image,
# or `d' to double it.  this attribute is only used in source keys.

# this script generates a set of functions for the product of all
# source image formats and all destination image formats.
#
# "all source image formats" includes all source image formats given
# on the command line, plus:
#
#   the "any" source image format, unscaled
#   the "any" source image format, halved
#   the "any" source image format, doubled
#
# "all destination image formats" includes all destination
# image formats given on the command line, plus:
#
#   the "any" destination image format
#
src_all=
dst_all=
for order in m l; do
  src_key="0x0d0b0s_p0o${order}_r0x0_g0x0_b0x0"
  src_all="${src_all} ${src_key} ${src_key}_h ${src_key}_d"
  dst_all="${dst_all} d0b0s_p0o${order}_r0x0_g0x0_b0x0"
done

which=
for arg
do
    case $arg in
    src|dst) which=$arg ;;
    *)
	if test "x${which}" != x; then
	    eval "${which}_all=\"$arg \$${which}_all\""
	fi
	;;
    esac
done

PROG=`basename $0`
cat <<EOF
/* automatically generated by $PROG, do not edit! */

/*
 * Copyright (c) 2003, 2005 Matt Fredette
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
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

_TME_RCSID("\$Id: fb-xlat-auto.sh,v 1.12 2009/08/30 21:51:53 fredette Exp $");

/* the central feature of these translation functions is the "bit
   FIFO", a first-in, first-out stream of bits.  source bit FIFOs are
   used to read pixel bits out of source images, and destination bit
   FIFOs are used to write pixel bits into destination images.

   a bit FIFO has a visible part and an invisible part.

   the visible part of a bit FIFO is 32 bits wide, meaning the
   translation code can read (for a source bit FIFO) or write (for a
   destination bit FIFO) up to 32 bits of pixel data before a bit
   FIFO shift.

   the invisible part of a source bit FIFO contains pixel bits that
   have already been read from the source image memory, but that have
   yet to be shifted in to the visible part.

   the invisible part of a destination bit FIFO contains the pixel
   bits that have been shifted out of the visible part, but that have
   yet to be written to the destination image memory.

   depending on various attributes of an image format, it may be
   possible for the translation code to always read or write the
   entire 32 visible bits of a bit FIFO at a time, and as a result
   always shift the bit FIFO 32 bits at a time.  this is desirable
   because it minimizes bit FIFO shifts.

   when this does happen, it may also be the case that the visible
   part of the bit FIFO always perfectly corresponds to an aligned
   32-bit word in the image buffer.

   this is the optimal situation, as it makes it unnecessary to track
   the invisible part in C local variables at all - each 32-bit FIFO
   shift of a source bit FIFO just reads the next 32-bit word directly
   into the visible part, and each 32-bit FIFO shift of a destination
   bit FIFO just writes the next 32-bit word directly out of the
   visible part.

   within the visible part of a bit FIFO, which bits belong to which
   pixels depends on the "order" of the image format.  most-significant
   (big-endian) images have earlier (more to the left on the screen)
   pixels in more-significant bit positions.  least-significant
   (little-endian) images have earlier pixels in less-significant bit
   bit positions.

   bit significance *within* a pixel never depends on image order.
   once the bits belonging to a pixel have been identified according
   to image order, the least significant bit in that pixel's value is
   always the bit with the least absolute value.

   some pictures may help explain this better.  each picture shows the
   32 bit visible part of a bit FIFO, with the bits marked from 0
   (least significant) to 31 (most significant).  all of these
   examples are for a two-bit-deep/two-bits-per-pixel image, and in
   the visible part sixteen pixels are numbered.  lesser numbered
   pixels are earlier (more to the left on the screen).  some of the
   invisible part is also shown, along with the direction that the bit
   FIFO shifts in:

   a source bit FIFO for a little-endian source image with two bits
   per pixel looks like:

                        |
                        | 31 30 29 28     7  6  5  4  3  2  1  0
      --+--+--+--+--+--+|+--+--+--+--+-  --+--+--+--+--+--+--+--+
    .. p18 | p17 | p16 ||| p15 | p14 | .. p3  | p2  | p1  | p0  |
      --+--+--+--+--+--+|+--+--+--+--+-  --+--+--+--+--+--+--+--+
                        |
   invisible part       |             visible part

          shift -> shift -> shift -> shift -> shift ->

   a source bit FIFO for a big-endian source image with two bits per
   pixel looks like:

                                            |
     31 30 29 28 27 26 25 26  4  3  2  1  0 |
    +--+--+--+--+--+--+--+--  -+--+--+--+--+|+--+--+--+--+--+--
    | p0  | p1  | p2  | p3  .. | p14 | p15 ||| p16 | p17 | p18 ..
    +--+--+--+--+--+--+--+--  -+--+--+--+--+|+--+--+--+--+--+--
                                            |
               visible part                 |    invisible part

        <- shift <- shift <- shift <- shift <- shift

   a destination bit FIFO for a little-endian destination image with
   two bits per pixel looks like:

                                            |
     31 30 29 28 27 26 25 26  4  3  2  1  0 |
    +--+--+--+--+--+--+--+--  -+--+--+--+--+|+--+--+--+--+--+--
    | p23 | p22 | p21 | p20 .. | p9  | p8  ||| p7  | p6  | p5  ..
    +--+--+--+--+--+--+--+--  -+--+--+--+--+|+--+--+--+--+--+--
                                            |
               visible part                 |    invisible part

          shift -> shift -> shift -> shift -> shift ->

   a destination bit FIFO for a big-endian destination image with
   two bits per pixel looks like:

                        |
                        | 31 30 29 28     7  6  5  4  3  2  1  0
      --+--+--+--+--+--+|+--+--+--+--+-  --+--+--+--+--+--+--+--+
    .. p5  | p6  | p7  ||| p8  | p9  | .. p20 | p21 | p22 | p23 |
      --+--+--+--+--+--+|+--+--+--+--+-  --+--+--+--+--+--+--+--+
                        |
   invisible part       |             visible part

        <- shift <- shift <- shift <- shift <- shift

  each translation function has at least one source bit FIFO and at
  least one destination bit FIFO.  the general idea is to read source
  pixel value(s) from the source image, map those source pixel
  value(s) somehow into destination pixel value(s), and write those
  destination pixel value(s) to the destination image.

  translation functions that do not scale the image have exactly one
  source bit FIFO and exactly one destination bit FIFO.  one pixel
  read from the source bit FIFO becomes one pixel written to the
  destination bit FIFO.

  translation functions that halve the image size have two source bit
  FIFOs and one destination bit FIFO.  one source bit FIFO sources
  bits from an even-numbered scanline, and the other sources bits from
  an odd-numbered scanline.  four pixels read from the source bit
  FIFOs - two from each source bit FIFO, making a 2x2 square - become
  one pixel written to the destination bit FIFO.

  translation functions that double the image size have one source
  bit FIFO and two destination bit FIFOs.  one destination bit FIFO
  sinks bits for an even-numbered scanline, and the other sinks bits
  for an odd-numbered scanline.  one pixel read from the source bit
  FIFO becomes four pixels written to the destination bit FIFOs -
  two to each destination bit FIFO, making a 2x2 square.

  translation functions don't necessarily always translate the entire
  source image into the entire destination image.  instead, the buffer
  holding the current source image is expected to be twice as large as
  necessary (plus some overhead), with the second half of the buffer
  holding a copy of the last translated ("old") source image.

  before setting up and translating pixels using bit FIFOs, a
  translation function treats the the current and old source images as
  an array of aligned 32-bit words and compares them.  if it finds no
  32-bit word that has changed, no translation is done and the
  function returns.

  otherwise, initial source pixel x and y coordinates are derived from
  the offset of the 32-bit word that failed comparison, and the source
  primary bit FIFO is primed.  if this translation function halves the
  image size, the source secondary bit FIFO is primed from the same x
  coordinate on the "other" (think y ^ 1) scanline.

  then, initial destination pixel x and y coordinates are derived from
  the initial source x and y coordinates, and the destination primary
  bit FIFO is primed.  if this translation function doubles the image
  size, the destination secondary bit FIFO is primed from the same x
  coordinate on the "other" (think y ^ 1) scanline.

  as mentioned previously, the buffer holding the current source image
  is expected to be twice as large as necessary (plus some overhead),
  with the second half of the buffer holding a copy of the last
  translated ("old") source image.

  this "overhead" is approximately two extra scanlines worth of data,
  that is initialized to all-bits-zero and must always remain zero.
  this extra data is present at the end of both the current ("new")
  and last translated ("old") source images.

  these extra, always-blank scanlines guarantee that the pixel
  translation loop terminates.  the pixel translation loop *never*
  checks for the end of the image buffer.  instead, it terminates only
  after it has read in TME_FB_XLAT_RUN consecutive aligned 32-bit
  words that have *not* changed between the new and old source images.
  this small amount of extra memory overhead simplifies the pixel
  translation loop, because it doesn't have to worry about going past
  the end of the actual image.
*/


/* macros: */

/* given a 32-bit aligned pointer into the current source image, this
   returns the corresponding 32-bit aligned pointer in the last
   translated ("old") source image.  since the old source image
   follows the current source image, this is simple pointer arithmetic
   using src_bypb: */
#define TME_FB_XLAT_SRC_OLD(raw)				\\
  ((tme_uint32_t *) (((tme_uint8_t *) (raw)) + src_bypb))

/* when the fast, aligned 32-bit word comparison loop finds a word
   that has changed in the source image, pixels are translated until
   TME_FB_XLAT_RUN consecutive aligned 32-bit words are processed that
   have *not* changed in the source image, at which point the fast
   comparison loop resumes.

   the idea is that after you've started translating pixels, once the
   FIFO shift operation has read TME_FB_XLAT_RUN consecutive raw,
   unchanged 32-bit words, all of the pixels from previous, changed
   32-bit words have been translated and shifted out of the source bit
   FIFO(s), and all bits remaining in the source bit FIFO(s) are for
   pixels in those unchanged 32-bit words.  since the pixels are
   unchanged, pixel translation can stop, and the entire state of the
   source bit FIFO(s) can be discarded.

   so, each time a raw, changed 32-bit word is read, xlat_run is
   reloaded with TME_FB_XLAT_RUN, and each time 32 bits worth of
   source image pixels are processed, it is decremented.  when it
   reaches zero, the source bit FIFO(s) are discarded, the destination
   bit FIFO(s) are flushed, the pixel translation loop breaks and the
   fast comparison loop continues: */
#define TME_FB_XLAT_RUN (2)

/* this shifts a source FIFO: */
#define TME_FB_XLAT_SHIFT_SRC(unaligned, fifo, next, bits, shift, raw, order)\\
do {								\\
								\\
  /* if the source FIFO may not be 32-bit aligned: */           \\
  if (unaligned) {                                              \\
                                                                \\
    /* we must be shifting between 1 and 32 bits: */		\\
    assert ((shift) >= 1 && (shift) <= 32);			\\
								\\
    /* the FIFO must have more than 32 bits in it already: */	\\
    assert (bits > 32);						\\
								\\
    /* shift the FIFO: */					\\
    if ((shift) == 32) {					\\
      fifo = next;						\\
    }								\\
    else if (order == TME_ENDIAN_BIG) {				\\
      fifo = (fifo << ((shift) & 31)) | (next >> (32 - (shift)));\\
      next <<= ((shift) & 31);					\\
    }								\\
    else {							\\
      fifo = (fifo >> ((shift) & 31)) | (next << (32 - (shift)));\\
      next >>= ((shift) & 31);					\\
    }								\\
    bits -= (shift);						\\
								\\
    /* if we have a new 32-bit word to read: */			\\
    if (bits <= 32) {						\\
      next = *raw;						\\
      if (*TME_FB_XLAT_SRC_OLD(raw) != next) {			\\
        *TME_FB_XLAT_SRC_OLD(raw) = next;			\\
        xlat_run = TME_FB_XLAT_RUN;				\\
      }								\\
      raw++;							\\
      next = (order == TME_ENDIAN_BIG				\\
              ? tme_betoh_u32(next)				\\
              : tme_letoh_u32(next));				\\
								\\
      /* before the load, if there were fewer than 32 bits	\\
         remaining in the FIFO, shift bits from the word	\\
         we just loaded into their proper positions: */		\\
      if (bits < 32) {						\\
        if (order == TME_ENDIAN_BIG) {				\\
          fifo |= (next >> bits);				\\
          next <<= (32 - bits);					\\
        }							\\
        else {							\\
          fifo |= (next << bits);				\\
          next >>= (32 - bits);					\\
        }							\\
      }								\\
								\\
      /* there are now 32 more bits in the FIFO: */		\\
      bits += 32;						\\
    }								\\
  }								\\
								\\
  /* otherwise, if the source FIFO is always 32-bit aligned: */ \\
  else {                                                        \\
                                                                \\
    /* we must be shifting exactly 32 bits: */                  \\
    assert((shift) == 32);                                      \\
                                                                \\
    /* load the next 32-bit word: */                            \\
    fifo = *raw;                                                \\
    if (*TME_FB_XLAT_SRC_OLD(raw) != fifo) {                    \\
      *TME_FB_XLAT_SRC_OLD(raw) = fifo;                         \\
      xlat_run = TME_FB_XLAT_RUN;                               \\
    }                                                           \\
    raw++;                                                      \\
    fifo = (order == TME_ENDIAN_BIG                             \\
            ? tme_betoh_u32(fifo)                               \\
            : tme_letoh_u32(fifo));                             \\
  }                                                             \\
} while (/* CONSTCOND */ 0)

/* this shifts a destination FIFO: */
#define TME_FB_XLAT_SHIFT_DST(unaligned, fifo, next, bits, shift, raw, order)\\
do {								\\
								\\
  /* if the destination FIFO may not be 32-bit aligned: */      \\
  if (unaligned) {						\\
								\\
    /* we must be shifting between 1 and 32 bits: */		\\
    assert ((shift) >= 1 && (shift) <= 32);			\\
								\\
    /* the FIFO must have fewer than 32 bits in it: */	        \\
    assert (bits < 32);						\\
								\\
    /* shift the FIFO: */					\\
    if (order == TME_ENDIAN_BIG) {				\\
      next |= (fifo >> bits);					\\
      fifo <<= (32 - bits);					\\
    }								\\
    else {							\\
      next |= (fifo << bits);					\\
      fifo >>= (32 - bits);					\\
    }								\\
    if (SHIFTMAX_INT32_T < 32 && bits == 0) {			\\
      fifo = 0;							\\
    }								\\
    bits += (shift);						\\
								\\
    /* if we have a completed 32-bit word to write: */		\\
    if (bits >= 32) {						\\
      *(raw++) = (order == TME_ENDIAN_BIG			\\
                  ? tme_htobe_u32(next)				\\
                  : tme_htole_u32(next));			\\
      bits -= 32;						\\
      assert(bits != 0 || fifo == 0);				\\
      next = fifo;						\\
    }								\\
  }								\\
								\\
  /* the destination FIFO is always 32-bit aligned: */		\\
  else {							\\
								\\
    /* we must be shifting exactly 32 bits: */                  \\
    assert((shift) == 32);                                      \\
                                                                \\
    /* store the next 32-bit word: */                           \\
    *(raw++) = (order == TME_ENDIAN_BIG                         \\
                ? tme_htobe_u32(fifo)                           \\
                : tme_htole_u32(fifo));                         \\
                                                                \\
  }								\\
                                                                \\
  /* clear the writable part of the FIFO: */			\\
  fifo = 0;							\\
} while (/* CONSTCOND */ 0)

/* _TME_FB_XLAT_MAP_LINEAR_SCALE gives the factor needed to scale a
   masked value up or down to a given size in bits.  for example, if a
   value's mask is 0xf800 (a five bit mask), and the value needs to be
   scaled up to seven bits, this gives an factor of four.  if a
   value's mask is 0x7e0 (a six bit mask), and the value needs to be
   scaled down to three bits, this gives a factor of eight: */
#define _TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out)	\\
  (TME_FB_XLAT_MAP_BASE_MASK(mask_in)				\\
   ^ TME_FB_XLAT_MAP_BASE_MASK(mask_out))
#define TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out)	\\
  (_TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out)		\\
   ? (TME_FB_XLAT_MAP_BASE_MASK(_TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out))\\
      + 1)							\\
   : 1)

/* this linearly maps a value from one mask to another: */
#define _TME_FB_XLAT_MAP_LINEAR(value, mask_in, mask_out)	\\
								\\
  /* if the value does not need to be scaled up: */		\\
  (((TME_FB_XLAT_MAP_BASE_MASK(mask_out)			\\
     <= TME_FB_XLAT_MAP_BASE_MASK(mask_in))			\\
    ?								\\
								\\
    /* extract the value and scale it down: */			\\
    (TME_FIELD_MASK_EXTRACTU(value, mask_in)			\\
     / TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out))		\\
								\\
    /* otherwise, the value needs to be scaled up: */		\\
    :								\\
								\\
    /* extract the value: */					\\
    ((TME_FIELD_MASK_EXTRACTU(value, mask_in)			\\
								\\
      /* scale it up: */					\\
      * TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out))	\\
								\\
     /* if the least significant bit of the value is set, add	\\
	in the scale minus one.  this makes the linear mapping	\\
	at least cover the entire range: */			\\
     + (((value /						\\
	  _TME_FIELD_MASK_FACTOR(mask_in))			\\
	 & 1)							\\
	* (TME_FB_XLAT_MAP_LINEAR_SCALE(mask_in, mask_out)	\\
	   - 1))))						\\
								\\
   /* finally, shift the value into position: */		\\
   * _TME_FIELD_MASK_FACTOR(mask_out))

/* this indexes a value: */
#define _TME_FB_XLAT_MAP_INDEX(value, mask_out, index)		\\
								\\
  /* intensities are either stored as 8 or 16 bits: */		\\
  (((TME_FB_XLAT_MAP_BASE_MASK(mask_out) <= 0xff)		\\
    ? ((const tme_uint8_t *) (index))[(value)]			\\
    : ((const tme_uint16_t *) (index))[(value)])		\\
								\\
   /* shift the value into position: */				\\
   * _TME_FIELD_MASK_FACTOR(mask_out))

/* this maps one subfield or intensity value into another subfield or
   intensity value: */
#define TME_FB_XLAT_MAP(value, mask_in, mask_out, indexed, index)\\
								\\
  /* do the linear mapping or the index mapping: */		\\
  ((!(indexed))							\\
   ? _TME_FB_XLAT_MAP_LINEAR(value, mask_in, mask_out)		\\
   : _TME_FB_XLAT_MAP_INDEX(TME_FIELD_MASK_EXTRACTU(value, mask_in), mask_out, index))
EOF

# the next function number:
xlat_next=0
xlat_array=

# permute over the source possibilities:
for src_key in ${src_all}; do

    # take apart the source possibility:
    src_parts=${src_key}

    # get the width:
        width=`printf "%s\n" ${src_parts} | sed -e 's/^\([0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^\([0-9][0-9]*\)\(.*\)$/\2/'`

    # get the height:
       height=`printf "%s\n" ${src_parts} | sed -e 's/^x\([0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^x\([0-9][0-9]*\)\(.*\)$/\2/'`

    # get the source depth:
    src_depth=`printf "%s\n" ${src_parts} | sed -e 's/^d\([0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^d\([0-9][0-9]*\)\(.*\)$/\2/'`

    # get the source bits per pixel:
     src_bipp=`printf "%s\n" ${src_parts} | sed -e 's/^b\([0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^b\([0-9][0-9]*\)\(.*\)$/\2/'`

    # get the source skip pixel count:
    src_skipx=`printf "%s\n" ${src_parts} | sed -e 's/^s\([_0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^s\([_0-9][0-9]*\)\(.*\)$/\2/'`

    # get the source scanline pad:
      src_pad=`printf "%s\n" ${src_parts} | sed -e 's/^p\([0-9][0-9]*\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^p\([0-9][0-9]*\)\(.*\)$/\2/'`

    # get the source "order" - i.e., byte order and leftmost (first)
    # pixel significance:
    src_order=`printf "%s\n" ${src_parts} | sed -e 's/^o\([ml]\)\(.*\)$/\1/'`
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^o\([ml]\)\(.*\)$/\2/'`

    # get the optional source "class" - monochrome or color:
    src_class=`printf "%s\n" ${src_parts} | sed -e 's/^c\([mc]\)\(.*\)$/\1/'`
    if test "x${src_class}" = "x${src_parts}"; then src_class= ; fi
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^c\([mc]\)\(.*\)$/\2/'`

    # get the optional source pixel mapping - linear or indexed, and
    # the optional number of bits in the map range:
      src_map=`printf "%s\n" ${src_parts} | sed -e 's/^m\([li]\)\([0-9]*\)\(.*\)$/\1/'`
    src_map_bits=`printf "%s\n" ${src_parts} | sed -e 's/^m\([li]\)\([0-9]*\)\(.*\)$/\2/'`
    if test "x${src_map}" = "x${src_parts}"; then src_map= ; src_map_bits= ; fi
    src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^m\([li]\)\([0-9]*\)\(.*\)$/\3/'`

    # get the optional source pixel subfield masks:
    src_mask_g= ; src_mask_r= ; src_mask_b=
    while true; do
	src_mask=`printf "%s\n" ${src_parts} | sed -e 's/^\(_[rgb]\)\(0x[0-9A-Fa-f][0-9A-Fa-f]*\)\(.*\)$/src_mask\1=\2/'`
	if test "x${src_mask}" = "x${src_parts}"; then break; fi
	eval ${src_mask}
	src_parts=`printf "%s\n" ${src_parts} | sed -e 's/^\(_[rgb]\)\(0x[0-9A-Fa-f][0-9A-Fa-f]*\)\(.*\)$/\3/'`
    done

    # get the source scaling:
    scale="${src_parts}_"

    # if the source class is not given, but the source depth is
    # known, assume monochrome if the source depth is one, else
    # assume color:
    #
    if test "x${src_class}" = x; then
	case "${src_depth}" in
	0) src_class= ;;
	1) src_class=m ;;
	*) src_class=c ;;
	esac
    fi

    # if the source mapping type is not given:
    #
    if test "x${src_map}" = x; then

	# if the source class is known as color:
	#
	if test "x${src_class}" = "xc"; then

	    # if it is known that there are not any subfield masks, assume an indexed
	    # mapping, else if it is known that there are subfield masks, assume a
	    # linear mapping:
 	    #
	    case "x${src_mask_r}" in
	    x0x0) ;;
	    x) src_map=i ;;
	    *) src_map=l ;;
	    esac

	# if the source class is known as monochrome:
	#
	elif test "x${src_class}" = "xm"; then

	    # assume a linear mapping:
	    #
	    src_map=l
	fi
    fi

    # if the size of the source mapping range in bits is not given:
    #
    if test "x${src_map_bits}" = x; then

	# if the source class is known as mono and the source mapping
	# type is known as linear, assume that the source depth is the
	# size of the mapping range in bits:
	#
	if test "x${src_class}" = xm && test "x${src_map}" = xl; then
	    src_map_bits=${src_depth}

	# otherwise, set the size of the mapping range to zero to indicate
	# unknown:
	#
	else
	    src_map_bits=0
	fi
    fi

    # permute over the destination possibilities:
    for dst_key in ${dst_all}; do

	# take apart the destination possibility:
	dst_parts=${dst_key}

	# get the destination depth:
	dst_depth=`printf "%s\n" ${dst_parts} | sed -e 's/^d\([0-9][0-9]*\)\(.*\)$/\1/'`
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^d\([0-9][0-9]*\)\(.*\)$/\2/'`

	# get the destination bits per pixel:
	 dst_bipp=`printf "%s\n" ${dst_parts} | sed -e 's/^b\([0-9][0-9]*\)\(.*\)$/\1/'`
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^b\([0-9][0-9]*\)\(.*\)$/\2/'`

	# get the destination skip pixel count:
	dst_skipx=`printf "%s\n" ${dst_parts} | sed -e 's/^s\([_0-9][0-9]*\)\(.*\)$/\1/'`
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^s\([_0-9][0-9]*\)\(.*\)$/\2/'`

	# get the destination scanline pad:
	  dst_pad=`printf "%s\n" ${dst_parts} | sed -e 's/^p\([0-9][0-9]*\)\(.*\)$/\1/'`
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^p\([0-9][0-9]*\)\(.*\)$/\2/'`

	# get the destination "order" - i.e., byte order and leftmost
	# (first) pixel significance:
	dst_order=`printf "%s\n" ${dst_parts} | sed -e 's/^o\([ml]\)\(.*\)$/\1/'`
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^o\([ml]\)\(.*\)$/\2/'`

	# get the optional destination pixel mapping - linear or indexed:
	  dst_map=`printf "%s\n" ${dst_parts} | sed -e 's/^m\([li]\)\(.*\)$/\1/'`
	if test "x${dst_map}" = "x${dst_parts}"; then dst_map= ; fi
	dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^m\([li]\)\(.*\)$/\2/'`

	# get the optional destination pixel subfield masks:
	dst_mask_g= ; dst_mask_r= ; dst_mask_b=
	while true; do
	    dst_mask=`printf "%s\n" ${dst_parts} | sed -e 's/^\(_[rgb]\)\(0x[0-9A-Fa-f][0-9A-Fa-f]*\)\(.*\)$/dst_mask\1=\2/'`
	    if test "x${dst_mask}" = "x${dst_parts}"; then break; fi
	    eval ${dst_mask}
	    dst_parts=`printf "%s\n" ${dst_parts} | sed -e 's/^\(_[rgb]\)\(0x[0-9A-Fa-f][0-9A-Fa-f]*\)\(.*\)$/\3/'`
	done

	# allocate the xlat number:
	xlat=${xlat_next}
	xlat_next=`expr ${xlat_next} + 1`
	xlat_info="tme_fb_xlat${xlat}"

	# start the function:
	printf "%s\n" ""
	printf "%s\n" "/* this translates frame buffer contents from this source format:"
	case "${width}x${height}" in
	0x0) printf %s "     any dimensions" ;;
	*x0) printf %s "     width $width, any height" ;;
	0x*) printf %s "     any width, height ${height}" ;;
	*)   printf %s "     ${width}x${height}" ;;
	esac
	xlat_info="${xlat_info}, ${width}, ${height}"
	case ${scale} in
	_h_) printf "%s\n" " (image will be halved)" ; value="HALF" ;;
	_d_) printf "%s\n" " (image will be doubled)" ; value="DOUBLE" ;;
	*) printf "%s\n" "" ; value="NONE" ;;
	esac
	xlat_info="${xlat_info}, TME_FB_XLAT_SCALE_${value}"
	printf %s "     "
	case ${src_depth} in
	0)  printf %s "any depth" ;;
	1)  printf %s "1 bit deep" ;;
	*)  printf %s "${src_depth} bits deep" ;;
	esac
	xlat_info="${xlat_info}, ${src_depth}"
	case ${src_bipp} in
	0)  printf %s ", any bits per pixel" ;;
	1)  printf %s ", 1 bit per pixel" ;;
	*)  printf %s ", ${src_bipp} bits per pixel" ;;
	esac
	xlat_info="${xlat_info}, ${src_bipp}"
	case ${src_skipx} in
	_) printf %s ", any number of pixels skipped" ; value="-1";;
	*) printf %s ", ${src_skipx} pixels skipped" ; value=${src_skipx} ;;
	esac
	xlat_info="${xlat_info}, ${value}"
	case ${src_pad} in
	0)  printf %s ", any scanline padding" ;;
	*)  printf %s ", ${src_pad}-bit scanline padding" ;;
	esac
	xlat_info="${xlat_info}, ${src_pad}"
	case ${src_order} in
	m)  printf %s ", MSB-first" ; value="BIG" ;;
	l)  printf %s ", LSB-first" ; value="LITTLE" ;;
	esac
	xlat_info="${xlat_info}, TME_ENDIAN_${value}"
	case "x${src_class}" in
	xm)  printf %s ", monochrome" ; value="MONOCHROME" ;;
	xc)  printf %s ", color" ; value="COLOR" ;;
	*)   printf %s ", either color or monochrome" ; value="ANY" ;;
	esac
	xlat_info="${xlat_info}, TME_FB_XLAT_CLASS_${value}"
	case "x${src_map}" in
	xl)  printf %s ", linearly mapped pixels" ; value="LINEAR" ;;
	xi)  printf %s ", index mapped pixels" ; value="INDEX" ;;
	*)   printf %s ", any pixel mapping" ; value="ANY" ;;
	esac
	xlat_info="${xlat_info}, TME_FB_XLAT_MAP_${value}"
	value=${src_map_bits}
	case "${src_map_bits}" in
	0)   printf %s ", any bits per mapped intensity" ;;
	1)   printf %s ", 1 bit per mapped intensity" ;;
	*)   printf %s ", ${src_map_bits} bits per mapped intensity" ;;
	esac
	xlat_info="${xlat_info}, ${value}"
	for primary in g r b; do
	    eval "src_mask=\$src_mask_${primary}"
	    case "x${src_mask}" in
	    x)  printf %s ", no ${primary} mask" ; value=0 ;;
	    x0x0) printf %s ", any ${primary} mask" ; value=TME_FB_XLAT_MASK_ANY ;;
	    *)  printf %s ", a ${primary} mask of ${src_mask}" ; value=${src_mask} ;;
	    esac
	    xlat_info="${xlat_info}, ${value}"
	done
	printf "%s\n" ""
	printf "%s\n" "   to this destination format:"
	printf %s "     "
	case ${dst_depth} in
	0)  printf %s "any depth" ;;
	1)  printf %s "1 bit deep" ;;
	*)  printf %s "${dst_depth} bits deep" ;;
	esac
	xlat_info="${xlat_info}, ${dst_depth}"
	case ${dst_bipp} in
	0)  printf %s ", any bits per pixel" ;;
	1)  printf %s ", 1 bit per pixel" ;;
	*)  printf %s ", ${dst_bipp} bits per pixel" ;;
	esac
	xlat_info="${xlat_info}, ${dst_bipp}"
	case ${dst_skipx} in
	_) printf %s ", any number of pixels skipped" ; value="-1";;
	*) printf %s ", ${dst_skipx} pixels skipped" ; value=${dst_skipx} ;;
	esac
	xlat_info="${xlat_info}, ${value}"
	case ${dst_pad} in
	0)  printf %s ", any scanline padding" ;;
	*)  printf %s ", ${dst_pad}-bit scanline padding" ;;
	esac
	xlat_info="${xlat_info}, ${dst_pad}"
	case ${dst_order} in
	m)  printf %s ", MSB-first" ; value="BIG" ;;
	l)  printf %s ", LSB-first" ; value="LITTLE" ;;
	esac
	xlat_info="${xlat_info}, TME_ENDIAN_${value}"
	case "x${dst_map}" in
	xl)  printf %s ", linearly mapped pixels" ; value="LINEAR" ;;
	xi)  printf %s ", index mapped pixels" ; value="INDEX" ;;
	*)   printf %s ", any pixel mapping" ; value="ANY" ;;
	esac
	xlat_info="${xlat_info}, TME_FB_XLAT_MAP_${value}"
	for primary in g r b; do
	    eval "dst_mask=\$dst_mask_${primary}"
	    case "x${dst_mask}" in
	    x)  printf %s ", no ${primary} mask" ; value=0 ;;
	    x0x0) printf %s ", any ${primary} mask" ; value=TME_FB_XLAT_MASK_ANY ;;
	    *)  printf %s ", a ${primary} mask of ${dst_mask}" ; value=${dst_mask} ;;
	    esac
	    xlat_info="${xlat_info}, ${value}"
	done
	save_IFS=$IFS
	IFS=
	xlat_array="${xlat_array}
  { $xlat_info }, "
	IFS=$save_IFS
	printf "%s\n" ""
	printf "%s\n" "*/"
	printf "%s\n" "static int"
	printf "%s\n" "tme_fb_xlat${xlat}(struct tme_fb_connection *src,"
	printf "%s\n" "             struct tme_fb_connection *dst)"
	printf "%s\n" "{"
	undef_macros=

	printf "%s\n" ""
	printf "%s\n" "  /* whenever possible we define macros instead of declaring"
	printf "%s\n" "     variables, for optimization: */"

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_x and src_y.  these are the current translation"
	printf "%s\n" "     coordinates in the source image: */"
	printf "%s\n" "  unsigned int src_x, src_y;"

	printf "%s\n" ""
	printf "%s\n" "  /* declare dst_x and dst_y.  these are the current translation"
	printf "%s\n" "     coordinates in the destination image.  since this function"
	if test $scale = _; then
	    printf "%s\n" "     does not scale the image, these coordinates are always"
	    printf "%s\n" "     the same as the coordinates in the source image: */"
	    printf "%s\n" "#define dst_x (src_x)"
	    printf "%s\n" "#define dst_y (src_y)"
	    undef_macros="${undef_macros} dst_x dst_y"
	    scale_op=
	    scale_math=
	else
	    if test $scale = _h_; then
		scale_op='/'
		scale_name='halves'
	    else
		scale_op='*'
		scale_name='doubles'
	    fi
	    scale_math=" ${scale_op} 2"
	    printf "%s\n" "     ${scale_name} the image, the coordinates in the destination image"
	    printf "%s\n" "     are always the coordinates in the source image${scale_math}: */"
	    printf "%s\n" "  unsigned int dst_x, dst_y;"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare pixel.  this holds a single pixel value being translated"
	printf "%s\n" "     for the destination image: */"
	printf "%s\n" "  tme_uint32_t pixel;"

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_width and dst_width.  these are in terms of pixels: */"
	value="(src_width${scale_math})"
	if test ${width} = 0; then
	    printf "%s\n" "  const unsigned int src_width = src->tme_fb_connection_width;"
	    if test $scale != _; then
		printf "%s\n" "  const unsigned int dst_width = ${value};"
	    else
		printf "%s\n" "#define dst_width ${value}"
		undef_macros="${undef_macros} dst_width"
	    fi
	else
	    printf "%s\n" "#define src_width (${width})"
	    printf "%s\n" "#define dst_width ${value}"
	    undef_macros="${undef_macros} src_width dst_width"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_depth, the source pixel depth, which is in"
	printf "%s\n" "     terms of bits.  declare src_mask, which is the corresponding"
	printf "%s\n" "     mask of one bits: */"
	value="(0xffffffff >> (32 - src_depth))"
	if test ${src_depth} = 0; then
	    printf "%s\n" "  const unsigned int src_depth = src->tme_fb_connection_depth;"
	    printf "%s\n" "  const tme_uint32_t src_mask = ${value};"
	else
	    printf "%s\n" "#define src_depth (${src_depth})"
	    printf "%s\n" "#define src_mask ${value}"
	    undef_macros="${undef_macros} src_depth src_mask"
	fi

	# if we are halving, or if source pixels may have subfields,
	# we may need to deal with intensities:
	#
	if test ${scale} = _h_ || test "x${src_mask_g}" != x; then

	    printf "%s\n" ""
	    printf "%s\n" "  /* declare src_indexed.  this is nonzero if source pixel subfields"
	    printf "%s\n" "     are index-mapped into intensities: */"
	    if test "x${src_map}" = x; then
		printf "%s\n" "  const unsigned int src_indexed = (src->tme_fb_connection_map_g != NULL);"
	    else
		if test "x${src_map}" = xi; then value=TRUE; else value=FALSE; fi
		printf "%s\n" "#define src_indexed (${value})"
		undef_macros="${undef_macros} src_indexed"
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "  /* declare src_mask_i, which is the mask for a source intensity: */"
	    if test "${src_map_bits}" = 0; then
		printf "%s\n" "  const tme_uint32_t src_mask_i = (0xffffffff >> (32 - src->tme_fb_connection_map_bits));"
	    else
		printf "%s\n" "#define src_mask_i (0xffffffff >> (32 - ${src_map_bits}))"
		undef_macros="${undef_macros} src_mask_i"
	    fi

	    if test "x${src_class}" != xm; then
		printf "%s\n" ""
		printf "%s\n" "  /* declare dst_indexed.  this is nonzero if intensities are index-mapped"
		printf "%s\n" "     into destination pixel subfields: */"
		if test "x${dst_map}" = x; then
		    printf "%s\n" "  const unsigned int dst_indexed = (dst->tme_fb_connection_map_g != NULL);"
		else
		    if test "x${dst_map}" = xi; then value=TRUE; else value=FALSE; fi
		    printf "%s\n" "#define dst_indexed (${value})"
		    undef_macros="${undef_macros} dst_indexed"
		fi

		printf "%s\n" ""
		printf "%s\n" "  /* declare dst_masks_default.  this is nonzero if the destination"
		printf "%s\n" "     subfield masks are the default: */"
		if test "x${dst_mask_g}" = "x0x0"; then
		    printf "%s\n" "  const unsigned int dst_masks_default = (dst->tme_fb_connection_mask_g == 0);"
		else
		    if test "x${dst_mask_g}" = x; then value=TRUE; else value=FALSE; fi
		    printf "%s\n" "#define dst_masks_default (${value})"
		    undef_macros="${undef_macros} dst_masks_default"
		fi
	    fi

	    for primary in g r b; do
		primary_cap=`printf "%s\n" ${primary} | tr a-z A-Z`
		printf "%s\n" ""
		printf "%s\n" "  /* declare src_mask_${primary}, the mask for the ${primary} subfield in a source pixel: */"
		eval "src_mask=\$src_mask_${primary}"
		if test "x${src_mask}" = x0x0; then
		    printf "%s\n" "  const tme_uint32_t src_mask_${primary}"
		    printf "%s\n" "    = (src->tme_fb_connection_mask_${primary} != 0"
		    printf "%s\n" "       ? src->tme_fb_connection_mask_${primary}"
		    printf "%s\n" "       : src_mask);"
		else
		    if test "x${src_mask}" = x; then src_mask="src_mask"; fi
		    printf "%s\n" "#define src_mask_${primary} (${src_mask})"
		    undef_macros="${undef_macros} src_mask_${primary}"
		fi
		printf "%s\n" ""
		printf "%s\n" "  /* declare value_${primary}, the intensity value for the ${primary} subfield in a pixel: */"
		printf "%s\n" "  tme_uint32_t value_${primary};"
		if test "x${src_class}" = xm; then break; fi
		printf "%s\n" ""
		printf "%s\n" "  /* declare dst_mask_${primary}, the mask for the ${primary} subfield in a destination pixel: */"
		eval "dst_mask=\$dst_mask_${primary}"
		if test "x${dst_mask}" = x0x0; then
		    printf "%s\n" "  const tme_uint32_t dst_mask_${primary}"
		    printf "%s\n" "    = (dst->tme_fb_connection_mask_${primary} != 0"
		    printf "%s\n" "       ? dst->tme_fb_connection_mask_${primary}"
		    printf "%s\n" "       : TME_FB_XLAT_MASK_DEFAULT_${primary_cap});"
		else
		    if test "x${dst_mask}" = x; then dst_mask="TME_FB_XLAT_MASK_DEFAULT_${primary_cap}"; fi
		    printf "%s\n" "#define dst_mask_${primary} (${dst_mask})"
		    undef_macros="${undef_macros} dst_mask_${primary}"
		fi
	    done
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_bipp and dst_bipp.  these are the bits-per-pixel"
	printf "%s\n" "     values for the source and destination images: */"
	if test ${src_bipp} = 0; then
	    printf "%s\n" "  const unsigned int src_bipp = src->tme_fb_connection_bits_per_pixel;"
	else
	    printf "%s\n" "#define src_bipp (${src_bipp})"
	    undef_macros="${undef_macros} src_bipp"
	fi
	if test ${dst_bipp} = 0; then
	    printf "%s\n" "  const unsigned int dst_bipp = dst->tme_fb_connection_bits_per_pixel;"
	else
	    printf "%s\n" "#define dst_bipp (${dst_bipp})"
	    undef_macros="${undef_macros} dst_bipp"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_skipx and dst_skipx.  these are the counts of"
	printf "%s\n" "     undisplayed pixels at the beginning of each scanline in the"
	printf "%s\n" "     source and destination images: */"
	if test ${src_skipx} = _; then
	    printf "%s\n" "  const unsigned int src_skipx = src->tme_fb_connection_skipx;"
	else
	    printf "%s\n" "#define src_skipx (${src_skipx})"
	    undef_macros="${undef_macros} src_skipx"
	fi
	if test ${dst_skipx} = _; then
	    printf "%s\n" "  const unsigned int dst_skipx = dst->tme_fb_connection_skipx;"
	else
	    printf "%s\n" "#define dst_skipx (${dst_skipx})"
	    undef_macros="${undef_macros} dst_skipx"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_pad and dst_pad.  these are the paddings, in bits,"
	printf "%s\n" "     of each scanline in the source and destination images: */"
	if test ${src_pad} = 0; then
	    printf "%s\n" "  const unsigned int src_pad = src->tme_fb_connection_scanline_pad;"
	else
	    printf "%s\n" "#define src_pad (${src_pad})"
	    undef_macros="${undef_macros} src_pad"
	fi
	if test ${dst_pad} = 0; then
	    printf "%s\n" "  const unsigned int dst_pad = dst->tme_fb_connection_scanline_pad;"
	else
	    printf "%s\n" "#define dst_pad (${dst_pad})"
	    undef_macros="${undef_macros} dst_pad"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_order and dst_order.  these are the bit and byte"
	printf "%s\n" "     orders (either TME_ENDIAN_BIG or TME_ENDIAN_LITTLE) of the"
	printf "%s\n" "     source and destination images.  since these values profoundly"
	printf "%s\n" "     affect optimization, they are always constant: */"
	if test ${src_order} = m; then value=BIG; else value=LITTLE; fi
	printf "%s\n" "#define src_order (TME_ENDIAN_${value})"
	undef_macros="${undef_macros} src_order"
	if test ${dst_order} = m; then value=BIG; else value=LITTLE; fi
	printf "%s\n" "#define dst_order (TME_ENDIAN_${value})"
	undef_macros="${undef_macros} dst_order"

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_bypl and dst_bypl.  these are the bytes per scanline"
	printf "%s\n" "     in the source and destination images.  these values are calculated"
	printf "%s\n" "     from the count of undisplayed and displayed pixels per scanline,"
	printf "%s\n" "     the number of bits per pixel, and the scanline padding: */"
	value="(((((src_skipx + src_width) * src_bipp) + (src_pad - 1)) & -src_pad) / 8)"
	if test ${src_skipx} = _ || test ${width} = 0 || test ${src_bipp} = 0 || test ${src_pad} = 0; then
	    printf "%s\n" "  const unsigned int src_bypl = ${value};"
	    src_bypl_var=true
	else
	    printf "%s\n" "#define src_bypl ${value}"
	    undef_macros="${undef_macros} src_bypl"
	    src_bypl_var=false
	fi
	value="(((((dst_skipx + dst_width) * dst_bipp) + (dst_pad - 1)) & -dst_pad) / 8)"
	if test ${dst_skipx} = _ || test ${width} = 0 || test ${dst_bipp} = 0 || test ${dst_pad} = 0; then
	    printf "%s\n" "  const unsigned int dst_bypl = ${value};"
	    dst_bypl_var=true
	else
	    printf "%s\n" "#define dst_bypl ${value}"
	    undef_macros="${undef_macros} dst_bypl"
	    dst_bypl_var=false
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_packed and dst_packed.  these are nonzero iff"
	printf "%s\n" "     every last bit in a scanline belongs to a displayed pixel."
	printf "%s\n" "     put another way, this is zero iff a scanline has undisplayed"
	printf "%s\n" "     pixels at its beginning or padding bits at its end.  when"
	printf "%s\n" "     a source image or destination image is packed, translation"
	printf "%s\n" "     doesn't have to worry about skipping FIFO bits to get to"
	printf "%s\n" "     bits belonging to displayed pixels: */"
	value="((src_width * src_bipp) == (src_bypl * 8))"
	if $src_bypl_var; then
	    printf "%s\n" "  const unsigned int src_packed = ${value};"
	else
	    printf "%s\n" "#define src_packed ${value}"
	    undef_macros="${undef_macros} src_packed"
	fi
	value="((dst_width * dst_bipp) == (dst_bypl * 8))"
	if $dst_bypl_var; then
	    printf "%s\n" "  const unsigned int dst_packed = ${value};"
	else
	    printf "%s\n" "#define dst_packed ${value}"
	    undef_macros="${undef_macros} dst_packed"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_bypb and src_bypb_real.  src_bypb is the bytes"
	printf "%s\n" "     per source image buffer with the \"translation termination"
	printf "%s\n" "     overhead\" of approximately two extra scanlines.  src_bypb_real"
	printf "%s\n" "     is the real bytes per source image buffer with no overhead."
	printf "%s\n" "     both values are padded to a multiple of 4 bytes (32 bits): */"
	if test ${height} = 0; then
	    value="src->tme_fb_connection_height"
	else
	    value="${height}"
	fi
	value_real="(((${value} * src_bypl) + 3) & -4)"
	value="((src_bypb_real + (src_bypl * 2)) & -4)"
	if test ${height} = 0 || $src_bypl_var; then
	    printf "%s\n" "  const unsigned int src_bypb_real = ${value_real};"
	    printf "%s\n" "  const unsigned int src_bypb = ${value};"
	else
	    printf "%s\n" "#define src_bypb_real ${value_real}"
	    printf "%s\n" "#define src_bypb ${value}"
	    undef_macros="${undef_macros} src_bypb_real src_bypb"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare the source primary bit FIFO:"
	printf "%s\n" ""
	printf "%s\n" "     src_raw0 points to the next aligned 32-bit word to be"
	printf "%s\n" "     read from the image buffer."
	printf "%s\n" ""
	printf "%s\n" "     src_fifo0 is the visible part of the bit FIFO."
	printf "%s\n" ""
	printf "%s\n" "     src_fifo0_next and src_fifo0_bits are only used when the"
	printf "%s\n" "     visible part of the bit FIFO is not guaranteed to always"
	printf "%s\n" "     correspond to an aligned 32-bit word in the image buffer."
	printf "%s\n" "     src_fifo0_next is the invisible part of the bit FIFO,"
	printf "%s\n" "     and src_fifo0_bits tracks the total number of bits in the"
	printf "%s\n" "     visible and invisible parts of the FIFO. */"
	printf "%s\n" "  const tme_uint32_t *src_raw0;"
	printf "%s\n" "  tme_uint32_t src_fifo0, src_fifo0_next;"
	printf "%s\n" "  unsigned int src_fifo0_bits;"

	if test ${scale} = _h_; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* since this function ${scale_name} the image, declare the"
	    printf "%s\n" "     source secondary bit FIFO.  these variables work in "
	    printf "%s\n" "     exactly the same way that their primary counterparts do: */"
	    printf "%s\n" "  const tme_uint32_t *src_raw1;"
	    printf "%s\n" "  tme_uint32_t src_fifo1, src_fifo1_next;"
	    printf "%s\n" "  unsigned int src_fifo1_bits;"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare the destination primary bit FIFO:"
	printf "%s\n" ""
	printf "%s\n" "     dst_raw0 points to the next aligned 32-bit word to be"
	printf "%s\n" "     written into the image buffer."
	printf "%s\n" ""
	printf "%s\n" "     dst_fifo0 is the visible part of the bit FIFO."
	printf "%s\n" ""
	printf "%s\n" "     dst_fifo0_next and dst_fifo0_bits are only used when the"
	printf "%s\n" "     visible part of the bit FIFO is not guaranteed to always"
	printf "%s\n" "     correspond to an aligned 32-bit word in the image buffer."
	printf "%s\n" "     dst_fifo0_next is the invisible part of the bit FIFO,"
	printf "%s\n" "     and dst_fifo0_bits tracks the total number of bits in the"
	printf "%s\n" "     invisible part of the FIFO. */"
	printf "%s\n" "  tme_uint32_t *dst_raw0;"
	printf "%s\n" "  tme_uint32_t dst_fifo0, dst_fifo0_next;"
	printf "%s\n" "  unsigned int dst_fifo0_bits;"

	if test ${scale} = _d_; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* since this function ${scale_name} the image, declare"
	    printf "%s\n" "     the destination secondary bit FIFO.  these variables work"
	    printf "%s\n" "     in exactly the same way that their primary counterparts"
	    printf "%s\n" "     do: */"
	    printf "%s\n" "  tme_uint32_t *dst_raw1;"
	    printf "%s\n" "  tme_uint32_t dst_fifo1, dst_fifo1_next;"
	    printf "%s\n" "  unsigned int dst_fifo1_bits;"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_off and dst_off.  these are used when priming a"
	printf "%s\n" "     source or destination bit FIFO, to identify an initial aligned"
	printf "%s\n" "     32-bit word in the source or destination image buffer, and an"
	printf "%s\n" "     initial bit offset within that word: */"
	printf "%s\n" "  unsigned int src_off, dst_off;"

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_fifo0_may_be_unaligned.  this is zero iff all"
	printf "%s\n" "     aligned 32-bit words in the source buffer contain a whole"
	printf "%s\n" "     number of displayed pixels, and at *all times during the"
	printf "%s\n" "     translation* the visible part of the bit FIFO is guaranteed"
	printf "%s\n" "     to correspond to an aligned 32-bit word in the image buffer."
	printf "%s\n" ""
	printf "%s\n" "     this is *not* so if any of the following are true:"
	printf "%s\n" ""
	printf "%s\n" "     - the source bits-per-pixel value is not known at compile"
	printf "%s\n" "       time.  in this case, we can't unroll the translation loop"
	printf "%s\n" "       for source pixels, and are forced to shift the FIFO after"
	printf "%s\n" "       each one."
	printf "%s\n" ""
	printf "%s\n" "     - if the source image is not packed.  in this case, there may"
	printf "%s\n" "       be undisplayed pixels in the FIFO, which we will need to"
	printf "%s\n" "       shift out."
	printf "%s\n" ""
	printf "%s\n" "     - if there are 24 bits per source pixel.  in this case, a"
	printf "%s\n" "       source pixel may cross a 32-bit boundary: */"
	src_fifo0_may_be_unaligned_var=false
	if test ${src_bipp} = 0; then
	    value=TRUE
	else
	    value="(!src_packed || (src_bipp == 24))"
	    src_fifo0_may_be_unaligned_var=${src_bypl_var}
	fi
	if $src_fifo0_may_be_unaligned_var; then
	    printf "%s\n" "  const unsigned int src_fifo0_may_be_unaligned = ${value};"
	else
	    printf "%s\n" "#define src_fifo0_may_be_unaligned ${value}"
	    undef_macros="${undef_macros} src_fifo0_may_be_unaligned"
	fi

	if test $scale = _h_; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* declare src_fifo1_may_be_unaligned.  this is zero iff all"
	    printf "%s\n" "     aligned 32-bit words in the source buffer contain a"
	    printf "%s\n" "     whole number of displayed pixels, *and* for every aligned"
	    printf "%s\n" "     32-bit word on one scanline all other scanlines have an"
	    printf "%s\n" "     aligned 32-bit word corresponding to the same x coordinate: */"
	    value="(src_fifo0_may_be_unaligned || (src_bypl & 3))"
	    if $src_bypl_var; then
		printf "%s\n" "  const unsigned int src_fifo1_may_be_unaligned = ${value};"
	    else
		printf "%s\n" "#define src_fifo1_may_be_unaligned ${value}"
		undef_macros="${undef_macros} src_fifo1_may_be_unaligned"
	    fi
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare dst_fifo0_may_be_unaligned.  this is zero iff all"
	printf "%s\n" "     aligned 32-bit words in the destination buffer contain a whole"
	printf "%s\n" "     number of displayed pixels, and at *all times during the"
	printf "%s\n" "     translation* the visible part of the bit FIFO is guaranteed"
	printf "%s\n" "     to correspond to an aligned 32-bit word in the image buffer."
	printf "%s\n" ""
	printf "%s\n" "     this is *not* so if any of the following are true:"
	printf "%s\n" ""
	printf "%s\n" "     - the destination bits-per-pixel value is not known at compile"
	printf "%s\n" "       time.  in this case, we can't unroll the translation loop"
	printf "%s\n" "       for destination pixels, and are forced to shift the FIFO"
	printf "%s\n" "       after each one."
	printf "%s\n" ""
	printf "%s\n" "     - if src_fifo0_may_be_unaligned is true.  in this case, we"
	printf "%s\n" "       definitely can't guarantee that any initial dst_x will"
	printf "%s\n" "       correspond to an aligned 32-bit word in the destination buffer."
	printf "%s\n" ""
	printf "%s\n" "     - if the destination image is not packed.  in this case, there may"
	printf "%s\n" "       be undisplayed pixels in the FIFO, which we will need to"
	printf "%s\n" "       shift out."
	printf "%s\n" ""
	printf "%s\n" "     - if there are 24 bits per destination pixel.  in this case,"
	printf "%s\n" "       a destination pixel may cross a 32-bit boundary."
	printf "%s\n" ""
	printf "%s\n" "     - if a possible initial dst_x doesn't correspond to an aligned"
	printf "%s\n" "       32-bit word in the destination buffer.  for this last one:"
	printf "%s\n" ""
	printf "%s\n" "     since we require that src_fifo0_may_be_unaligned is zero, we"
	printf "%s\n" "     know that the initial src_x = (Z * 32) / src_bipp for "
	printf "%s\n" "     some Z.  we also have the initial dst_x = src_x${scale_math}."
	printf "%s\n" "     the initial destination bit offset will then be:"
	printf "%s\n" ""
	printf "%s\n" "     (dst_skipx + dst_x) * dst_bipp"
	printf "%s\n" "     = (dst_skipx * dst_bipp) + (dst_x * dst_bipp)"
	printf "%s\n" ""
	printf "%s\n" "     if we additionally require that (dst_skipx * dst_bipp)"
	printf "%s\n" "     be 32-bit aligned, this reduces things to:"
	printf "%s\n" ""
	printf "%s\n" "     dst_x * dst_bipp"
	printf "%s\n" "     = (src_x${scale_math}) * dst_bipp"
	printf "%s\n" "     = (((Z * 32) / src_bipp)${scale_math}) * dst_bipp"
	printf "%s\n" ""
	printf "%s\n" "     which will be a multiple of 32 iff:"
	printf "%s\n" ""
	printf "%s\n" "      ((1 / src_bipp)${scale_math}) * dst_bipp >= 1 and integral"
	printf "%s\n" ""
	printf "%s\n" "     or, equivalently:"
	printf "%s\n" ""
	denom="src_bipp"
	numer="dst_bipp"
	case $scale in
	_) ;;
	_h_) denom="(${denom} * 2)" ;;
	_d_) numer="(${numer} * 2)" ;;
	esac
	printf "%s\n" "       (${numer} % ${denom}) == 0"
	printf "%s\n" "  */"
	dst_fifo0_may_be_unaligned_var=false
	if test ${dst_bipp} = 0; then
	    value=TRUE
	else
	  value="(src_fifo0_may_be_unaligned || !dst_packed || (dst_bipp == 24) || (dst_bypl % 4) || ((dst_skipx * dst_bipp) % 32) || (${numer} % ${denom}))"
	  if $src_bypl_var || $dst_bypl_var; then
	    dst_fifo0_may_be_unaligned_var=true
	  fi
	fi
	if $dst_fifo0_may_be_unaligned_var; then
	    printf "%s\n" "  const unsigned int dst_fifo0_may_be_unaligned = ${value};"
	else
	    printf "%s\n" "#define dst_fifo0_may_be_unaligned ${value}"
	    undef_macros="${undef_macros} dst_fifo0_may_be_unaligned"
	fi

	if test $scale = _d_; then
	    printf "%s\n" ""
	    printf "%s\n" "  /* declare dst_fifo1_may_be_unaligned.  this is zero iff all"
	    printf "%s\n" "     32-bit aligned values in the destination buffer contain a"
	    printf "%s\n" "     whole number of displayed pixels, and for every 32-bit"
	    printf "%s\n" "     aligned value on one scanline all other scanlines have a"
	    printf "%s\n" "     32-bit aligned value corresponding to the same x coordinate: */"
	    value="(dst_fifo0_may_be_unaligned || (dst_bypl & 3))"
	    if $src_bypl_var || $dst_bypl_var; then
		printf "%s\n" "  const unsigned int dst_fifo1_may_be_unaligned = ${value};"
	    else
		printf "%s\n" "#define dst_fifo1_may_be_unaligned ${value}"
		undef_macros="${undef_macros} dst_fifo1_may_be_unaligned"
	    fi
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_offset_updated_first and src_offset_updated_last,"
	printf "%s\n" "     which hold the offsets of the first and last updated bytes in"
	printf "%s\n" "     the source image: */"
	printf "%s\n" "  tme_uint32_t src_offset_updated_first;"
	printf "%s\n" "  tme_uint32_t src_offset_updated_last;"

	printf "%s\n" ""
	printf "%s\n" "  /* declare src_raw0_end.  when treating the source image as"
	printf "%s\n" "     an array of aligned 32-bit words, this variable holds the"
	printf "%s\n" "     address of the first word after the real source image."
	printf "%s\n" "     if the fast, aligned 32-bit word comparison loop passes"
	printf "%s\n" "     this point, the entire source image has been processed and"
 	printf "%s\n" "     the function terminates: */"
	printf "%s\n" "  const tme_uint32_t *src_raw0_end;"

	printf "%s\n" ""
	printf "%s\n" "  /* declare xlat_run.  see the comment for the TME_FB_XLAT_RUN"
	printf "%s\n" "     macro for an explanation of what this variable does: */"
	printf "%s\n" "  int xlat_run;"

	printf "%s\n" ""
	printf "%s\n" "  /* this silences gcc -Wuninitialized: */"
	printf "%s\n" "  src_fifo0_next = 0;"
	printf "%s\n" "  src_fifo0_bits = 0;"
	if test ${scale} = _h_; then
	    printf "%s\n" "  src_fifo1_next = 0;"
	    printf "%s\n" "  src_fifo1_bits = 0;"
	fi
	printf "%s\n" "  dst_fifo0_next = 0;"
	printf "%s\n" "  dst_fifo0_bits = 0;"
	if test ${scale} = _d_; then
	    printf "%s\n" "  dst_fifo1_next = 0;"
	    printf "%s\n" "  dst_fifo1_bits = 0;"
	fi

	printf "%s\n" ""
	printf "%s\n" "  /* initialize src_raw0 and src_raw0_end for the fast aligned 32-bit"
	printf "%s\n" "     word comparison loop.  on entry to (and when continuing) that loop,"
	printf "%s\n" "     src_raw0 always points to the aligned 32-bit word *before* the"
	printf "%s\n" "     next word to check.  src_raw0_end always points after the last"
	printf "%s\n" "     word to check."
	printf "%s\n" ""
	printf "%s\n" "     src_raw0 is actually part of the source primary bit FIFO, which"
	printf "%s\n" "     is good, because when the fast comparison fails on a word, src_raw0"
	printf "%s\n" "     is already primed and ready to work for that bit FIFO: */"
	printf "%s\n" "  src_offset_updated_first = src->tme_fb_connection_offset_updated_first;"
	printf "%s\n" "  src_offset_updated_last = TME_MIN(src->tme_fb_connection_offset_updated_last, src_bypb_real - 1);"
	printf "%s\n" "  src->tme_fb_connection_offset_updated_first = 0;"
	printf "%s\n" "  src->tme_fb_connection_offset_updated_last = src_bypb_real - 1;"
	printf "%s\n" "  if (src_offset_updated_first > src_offset_updated_last) {"
	printf "%s\n" "    return (FALSE);"
	printf "%s\n" "  }"
	printf "%s\n" "  src_raw0"
	printf "%s\n" "    = (((const tme_uint32_t *)"
	printf "%s\n" "        (src->tme_fb_connection_buffer"
	printf "%s\n" "         + (src_offset_updated_first"
	printf "%s\n" "            & (0 - (tme_uint32_t) sizeof(tme_uint32_t)))))"
	printf "%s\n" "       -1);"
	printf "%s\n" "  src_raw0_end"
	printf "%s\n" "    = ((const tme_uint32_t *)"
	printf "%s\n" "       (src->tme_fb_connection_buffer"
	printf "%s\n" "        + src_offset_updated_last"
	printf "%s\n" "        + 1));"

	printf "%s\n" ""
	printf "%s\n" "  /* initialize xlat_run to -1.  it can never go negative inside the"
	printf "%s\n" "     pixel translation loop, so if xlat_run stays negative for the"
	printf "%s\n" "     entire translation, it means that the source image hasn't changed"
	printf "%s\n" "     since the last translation.  this information is returned to the"
	printf "%s\n" "     caller to hopefully save more work in updating the display: */"
	printf "%s\n" "  xlat_run = -1;"

	printf "%s\n" ""
	printf "%s\n" "  /* this is the main translation loop, which contains the fast aligned"
	printf "%s\n" "     32-bit word comparison loop, and the pixel translation loop: */"
	printf "%s\n" "  for (;;) {"

	printf "%s\n" ""
	printf "%s\n" "    /* this is the fast aligned 32-bit word comparison loop.  it"
	printf "%s\n" "       terminates either when a word fails comparison, or when the"
	printf "%s\n" "       entire source image has been compared.  the if test that"
	printf "%s\n" "       follows checks for the latter case and breaks the main"
	printf "%s\n" "       translation loop: */"
	printf "%s\n" "    for (; (++src_raw0 < src_raw0_end"
	printf "%s\n" "            && *src_raw0 == *TME_FB_XLAT_SRC_OLD(src_raw0)); );"
	printf "%s\n" "    if (src_raw0 >= src_raw0_end) {"
	printf "%s\n" "      break;"
	printf "%s\n" "    }"

	printf "%s\n" ""
	printf "%s\n" "    /* calculate the byte offset into the source buffer of the"
	printf "%s\n" "       32-bit word that failed comparison: */"
	printf "%s\n" "    src_off = ((tme_uint8_t *) src_raw0) - src->tme_fb_connection_buffer;"

	printf "%s\n" ""
	printf "%s\n" "    /* calculate the source y pixel coordinate, and reduce"
	printf "%s\n" "       src_off from the byte offset into the buffer to the"
	printf "%s\n" "       byte offset into that scanline: */"
	printf "%s\n" "    src_y = src_off / src_bypl;"
	printf "%s\n" "    src_off = src_off % src_bypl;"

	printf "%s\n" ""
	printf "%s\n" "    /* while translating pixels, we use one or more \"bit FIFOs\","
	printf "%s\n" "       each composed of one or more 32-bit integers.  we load these"
	printf "%s\n" "       FIFOs 32 bits at a time. */"
	printf "%s\n" ""
	printf "%s\n" "    /* prime the visible part of the source primary bit FIFO: */"
	printf "%s\n" "    src_fifo0 = *src_raw0;"
	printf "%s\n" "    *TME_FB_XLAT_SRC_OLD(src_raw0) = src_fifo0;"
	printf "%s\n" "    src_raw0++;"
	printf "%s\n" "    src_fifo0 = ((src_order == TME_ENDIAN_BIG)"
	printf "%s\n" "                 ? tme_betoh_u32(src_fifo0)"
	printf "%s\n" "                 : tme_letoh_u32(src_fifo0));"
	printf "%s\n" ""
	printf "%s\n" "    /* if the source primary bit FIFO may be unaligned: */"
	printf "%s\n" "    if (src_fifo0_may_be_unaligned) {"
	printf "%s\n" ""
	printf "%s\n" "      /* prime the invisible part of the source primary bit FIFO and"
	printf "%s\n" "         assume that we will not have to shift it to finish: */"
	printf "%s\n" "      src_fifo0_next = *src_raw0;"
	printf "%s\n" "      *TME_FB_XLAT_SRC_OLD(src_raw0) = src_fifo0_next;"
	printf "%s\n" "      src_raw0++;"
	printf "%s\n" "      src_fifo0_next = ((src_order == TME_ENDIAN_BIG)"
	printf "%s\n" "                        ? tme_betoh_u32(src_fifo0_next)"
	printf "%s\n" "                        : tme_letoh_u32(src_fifo0_next));"
	printf "%s\n" "      src_fifo0_bits = 0;"
	printf "%s\n" ""
	printf "%s\n" "      /* if there are pixels that need to be skipped, the first 32 bits"
	printf "%s\n" "         we loaded into the FIFO may have first bits that belong to"
	printf "%s\n" "         those undisplayed (skipped) pixels.  it is *not* possible for"
	printf "%s\n" "         it to have first bits that belong to the scanline pad; there"
	printf "%s\n" "         might be pad bits in the *middle* of the first 32 bits, but any"
	printf "%s\n" "         first bits *must* belong to pixels, displayed or not: */"
	printf "%s\n" "      if (src_skipx > 0"
	printf "%s\n" "          && (src_off * 8) < (src_skipx * src_bipp)) {"
	printf "%s\n" ""
	printf "%s\n" "        /* see how many bits we will need to skip: */"
	printf "%s\n" "        src_fifo0_bits = (src_skipx * src_bipp) - (src_off * 8);"
	printf "%s\n" ""
	printf "%s\n" "        /* if it is more than 31 bits, this is an entire 32 bits of"
	printf "%s\n" "           undisplayed pixels.  just advance: */"
	printf "%s\n" "        if (src_fifo0_bits > 31) {"
	printf "%s\n" "          src_raw0--;"
	printf "%s\n" "          continue;"
	printf "%s\n" "        }"
	printf "%s\n" ""
	printf "%s\n" "        /* set the source x coordinate to zero: */"
	printf "%s\n" "        src_x = 0;"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* otherwise, the first 32 bits we load will have first bits for"
	printf "%s\n" "         a displayable pixel: */"
	printf "%s\n" "      else {"
	printf "%s\n" ""
	printf "%s\n" "        /* if the source bits per pixel is 24,  calculate the number of"
	printf "%s\n" "           bytes *before* the original src_raw0 of any split pixel, and"
	printf "%s\n" "           subtract this from src_off, to leave src_off as the byte offset"
	printf "%s\n" "           into the scanline of the beginning of a pixel: */"
	printf "%s\n" "        if (src_bipp == 24) {"
	printf "%s\n" "          src_fifo0_bits = (src_off % 3);"
	printf "%s\n" "          src_off -= src_fifo0_bits;"
	printf "%s\n" ""
	printf "%s\n" "          /* if this is a split pixel, we need to prime the source primary"
	printf "%s\n" "              bit FIFO starting with the part *before* the original src_raw0."
	printf "%s\n" "              we do not have to copy to the old; it passed comparison: */"
	printf "%s\n" "          if (src_fifo0_bits) {"
	printf "%s\n" "            src_raw0--;"
	printf "%s\n" "            src_fifo0_next = src_fifo0;"
	printf "%s\n" "            src_fifo0 = ((src_order == TME_ENDIAN_BIG)"
	printf "%s\n" "                         ? tme_betoh_u32(*(src_raw0 - 2))"
	printf "%s\n" "                         : tme_letoh_u32(*(src_raw0 - 2)));"
	printf "%s\n" "          }"
	printf "%s\n" "        }"
	printf "%s\n" ""
	printf "%s\n" "        /* calculate the source x coordinate: */"
	printf "%s\n" "        src_x = ((src_off * 8) / src_bipp) - src_skipx;"
	printf "%s\n" "      }"
	printf "%s\n" ""
	printf "%s\n" "      /* do any shifting to finish priming the source primary FIFO: */"
	printf "%s\n" "      if (src_fifo0_bits) {"
	printf "%s\n" "        if (src_order == TME_ENDIAN_BIG) {"
	printf "%s\n" "          src_fifo0 = (src_fifo0 << src_fifo0_bits) | (src_fifo0_next >> (32 - src_fifo0_bits));"
	printf "%s\n" "          src_fifo0_next <<= src_fifo0_bits;"
	printf "%s\n" "        }"
	printf "%s\n" "        else {"
	printf "%s\n" "          src_fifo0 = (src_fifo0 >> src_fifo0_bits) | (src_fifo0_next << (32 - src_fifo0_bits));"
	printf "%s\n" "          src_fifo0_next >>= src_fifo0_bits;"
	printf "%s\n" "        }"
	printf "%s\n" "      }"
	printf "%s\n" "      src_fifo0_bits = 64 - src_fifo0_bits;"
	printf "%s\n" "    }"
	printf "%s\n" ""
	printf "%s\n" "    /* otherwise, the source primary FIFO is aligned: */"
	printf "%s\n" "    else {"
	printf "%s\n" "      src_x = ((src_off * 8) / src_bipp) - src_skipx;"
	printf "%s\n" "    }"

	if test $scale = _h_; then
	    printf "%s\n" ""
	    printf "%s\n" "    /* when halving the image, we have a source secondary bit "
	    printf "%s\n" "       FIFO, providing pixel values at the same source x coordinate"
	    printf "%s\n" "       but on the \"other\" line.  prime the source secondary"
	    printf "%s\n" "       bit FIFO: */"
	    printf "%s\n" "    if (src_fifo1_may_be_unaligned) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* calculate the bit offset into the source buffer of"
	    printf "%s\n" "         this exact same pixel on the other line: */"
	    printf "%s\n" "      src_off = ((src_y ^ 1) * src_bypl * 8) + ((src_skipx + src_x) * src_bipp);"
	    printf "%s\n" ""
	    printf "%s\n" "      /* calculate how many bits offset from a 32-bit boundary the pixel is: */"
	    printf "%s\n" "      src_fifo1_bits = src_off % 32;"
	    printf "%s\n" ""
	    printf "%s\n" "      /* set src_raw1: */"
	    printf "%s\n" "      src_raw1 = (const tme_uint32_t *)"
	    printf "%s\n" "        (src->tme_fb_connection_buffer"
	    printf "%s\n" "         + ((src_off - src_fifo1_bits) / 8));"
	    printf "%s\n" ""
	    printf "%s\n" "      /* actually prime the FIFO by loading the first two words and"
	    printf "%s\n" "         shifting off any offset bits: */"
	    printf "%s\n" "      src_fifo1 = *src_raw1;"
	    printf "%s\n" "      *TME_FB_XLAT_SRC_OLD(src_raw1) = src_fifo1;"
	    printf "%s\n" "      src_raw1++;"
	    printf "%s\n" "      src_fifo1_next = *src_raw1;"
	    printf "%s\n" "      *TME_FB_XLAT_SRC_OLD(src_raw1) = src_fifo1_next;"
	    printf "%s\n" "      src_raw1++;"
	    printf "%s\n" "      if (src_order == TME_ENDIAN_BIG) {"
	    printf "%s\n" "        src_fifo1 = tme_betoh_u32(src_fifo1);"
	    printf "%s\n" "        src_fifo1_next = tme_betoh_u32(src_fifo1_next);"
	    printf "%s\n" "        if (src_fifo1_bits) {"
	    printf "%s\n" "          src_fifo1 = (src_fifo1 << src_fifo1_bits) | (src_fifo1_next >> (32 - src_fifo1_bits));"
	    printf "%s\n" "          src_fifo1_next <<= src_fifo1_bits;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	    printf "%s\n" "      else {"
	    printf "%s\n" "        src_fifo1 = tme_letoh_u32(src_fifo1);"
	    printf "%s\n" "        src_fifo1_next = tme_letoh_u32(src_fifo1_next);"
	    printf "%s\n" "        if (src_fifo1_bits) {"
	    printf "%s\n" "          src_fifo1 = (src_fifo1 >> src_fifo1_bits) | (src_fifo1_next << (32 - src_fifo1_bits));"
	    printf "%s\n" "          src_fifo1_next >>= src_fifo1_bits;"
	    printf "%s\n" "        }"
	    printf "%s\n" "      }"
	    printf "%s\n" "      src_fifo1_bits = 64 - src_fifo1_bits;"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise the source secondary FIFO is aligned: */"
	    printf "%s\n" "    else {"
	    printf "%s\n" "      src_raw1 = (const tme_uint32_t *)"
	    printf "%s\n" "        (((tme_uint8_t *) src_raw0)"
	    printf "%s\n" "         + (("
	    printf "%s\n" "             /* if src_y is even, this addend is now src_bypl * 4,"
	    printf "%s\n" "                else if src_y is odd, this addend is now src_bypl * 2: */"
	    printf "%s\n" "             ((src_bypl * 4) >> (src_y & 1))"
	    printf "%s\n" "             /* if src_y is even, this addend is now src_bypl,"
	    printf "%s\n" "                else if src_y is odd, this addend is now -src_bypl: */"
	    printf "%s\n" "             - (src_bypl * 3))"
	    printf "%s\n" "            /* this -4 compensates for src_raw0 already having"
	    printf "%s\n" "               been advanced by one: */"
	    printf "%s\n" "            - 4));"
	    printf "%s\n" "      src_fifo1 = *src_raw1;"
	    printf "%s\n" "      *TME_FB_XLAT_SRC_OLD(src_raw1) = src_fifo1;"
	    printf "%s\n" "      src_raw1++;"
	    printf "%s\n" "      src_fifo1 = ((src_order == TME_ENDIAN_BIG)"
	    printf "%s\n" "                   ? tme_betoh_u32(src_fifo1)"
	    printf "%s\n" "                   : tme_letoh_u32(src_fifo1));"
	    printf "%s\n" "    }"
	fi

	if test $scale != _; then
	    printf "%s\n" ""
	    printf "%s\n" "    /* calculate the destination coordinates: */"
	    printf "%s\n" "    dst_y = src_y${scale_math};"
	    printf "%s\n" "    dst_x = src_x${scale_math};"
	fi

	printf "%s\n" ""
	printf "%s\n" "    /* prime the destination primary bit FIFO: */"
	printf "%s\n" "    dst_fifo0 = 0;"
	printf "%s\n" "    if (dst_fifo0_may_be_unaligned) {"
	printf "%s\n" ""
	printf "%s\n" "      /* calculate the bit offset into the destination buffer of"
	printf "%s\n" "         the destination pixel: */"
	printf "%s\n" "      dst_off = (dst_y * dst_bypl * 8) + ((dst_skipx + dst_x) * dst_bipp);"
	printf "%s\n" ""
	printf "%s\n" "      /* calculate the number of bits that will be in the primed FIFO: */"
	printf "%s\n" "      dst_fifo0_bits = dst_off % 32;"
	printf "%s\n" ""
	printf "%s\n" "      /* set dst_raw0: */"
	printf "%s\n" "      dst_raw0 = (tme_uint32_t *)"
	printf "%s\n" "        (dst->tme_fb_connection_buffer"
	printf "%s\n" "         + ((dst_off - dst_fifo0_bits) / 8));"
	printf "%s\n" ""
	printf "%s\n" "      /* prime the primary destination FIFO: */"
	printf "%s\n" "      dst_fifo0_next = 0;"
	printf "%s\n" "      if (dst_fifo0_bits) {"
	printf "%s\n" "        dst_fifo0_next = (src_order == TME_ENDIAN_BIG"
	printf "%s\n" "                          ? (tme_betoh_u32(*dst_raw0) & (0xffffffffUL << (32 - dst_fifo0_bits)))"
	printf "%s\n" "                          : (tme_letoh_u32(*dst_raw0) & (0xffffffffUL >> (32 - dst_fifo0_bits))));"
	printf "%s\n" "      }"
	printf "%s\n" "    }"
	printf "%s\n" ""
	printf "%s\n" "    /* otherwise the destination primary FIFO is aligned: */"
	printf "%s\n" "    else {"
	printf "%s\n" "      dst_off = (dst_y * dst_bypl) + (((dst_skipx + dst_x) * dst_bipp) / 8);"
	printf "%s\n" "      dst_raw0 = (tme_uint32_t *) (dst->tme_fb_connection_buffer + dst_off);"
	printf "%s\n" "    }"

	if test $scale = _d_; then
	    printf "%s\n" ""
	    printf "%s\n" "    /* when doubling the image, we have a destination secondary bit "
	    printf "%s\n" "       FIFO, for pixel values at the same source x coordinate"
	    printf "%s\n" "       but on the \"other\" line.  prime the destination secondary"
	    printf "%s\n" "       bit FIFO: */"
	    printf "%s\n" "    dst_fifo1 = 0;"
	    printf "%s\n" "    if (dst_fifo1_may_be_unaligned) {"
	    printf "%s\n" ""
	    printf "%s\n" "      /* calculate the bit offset into the destination buffer of"
	    printf "%s\n" "         the destination pixel: */"
	    printf "%s\n" "      dst_off = ((dst_y + 1) * dst_bypl * 8) + ((dst_skipx + dst_x) * dst_bipp);"
	    printf "%s\n" ""
	    printf "%s\n" "      /* calculate the number of bits that will be in the primed FIFO: */"
	    printf "%s\n" "      dst_fifo1_bits = dst_off % 32;"
	    printf "%s\n" ""
	    printf "%s\n" "      /* set dst_raw1: */"
	    printf "%s\n" "      dst_raw1 = (tme_uint32_t *)"
	    printf "%s\n" "        (dst->tme_fb_connection_buffer"
	    printf "%s\n" "         + ((dst_off - dst_fifo1_bits) / 8));"
	    printf "%s\n" ""
	    printf "%s\n" "      /* prime the primary destination FIFO: */"
	    printf "%s\n" "      dst_fifo1_next = 0;"
	    printf "%s\n" "      if (dst_fifo1_bits) {"
	    printf "%s\n" "        dst_fifo1_next = (src_order == TME_ENDIAN_BIG"
	    printf "%s\n" "                          ? (tme_betoh_u32(*dst_raw1) & (0xffffffffUL << (32 - dst_fifo1_bits)))"
	    printf "%s\n" "                          : (tme_letoh_u32(*dst_raw1) & (0xffffffffUL >> (32 - dst_fifo1_bits))));"
	    printf "%s\n" "      }"
	    printf "%s\n" "    }"
	    printf "%s\n" ""
	    printf "%s\n" "    /* otherwise, the destination secondary FIFO is aligned: */"
	    printf "%s\n" "    else {"
	    printf "%s\n" "      dst_raw1 = (tme_uint32_t *)"
	    printf "%s\n" "        (((tme_uint8_t *) dst_raw0)"
	    printf "%s\n" "         + dst_bypl);"
	    printf "%s\n" "    }"
	fi

	# we want to unroll the pixel translation loop to read as many
	# source pixels as possible out of the source bit FIFOs and
	# write as many destination pixels as possible into the
	# destination bit FIFOs before shifting them.
	#
	# it is very common to want to unroll the translation loop a
	# different number of times on source and destination pixels,
	# so we always unroll the maximum possible, tracking within
	# each unrolled iteration the relative unrolled iteration for
	# the source and destination bit FIFOs.
	#

	# src_unroll and dst_unroll are the number of times we will
	# unroll the translation loop on source and destination
	# pixels, respectively.  assume no unrolling:
	#
	src_unroll=1
	dst_unroll=1

	# src_iter_scale is the number of source pixels read out of a
	# source bit FIFO in one unrolled iteration of the translation
	# loop.  if this function halves the source image, this is
	# two, otherwise this is one.
	#
	# dst_iter_scale is the number of destination pixels written
	# to a destination bit FIFO in one unrolled iteration of the
	# translation loop.  if this function doubles the source
	# image, this is two, otherwise this is one.
	#
	# src_fifo_shift is the number of bits that the source bit
	# FIFO(s) must be shifted after all unrolled iterations on
	# source pixels.  assume no unrolling, so if this function
	# halves the source image and src_bipp is less than 24, two
	# pixels' bits must be shifted, otherwise one pixel's bits
	# must be shifted.
	#
	# dst_fifo_shift is the number of bits that the destination bit
	# FIFO(s) must be shifted after all unrolled iterations on
	# destination pixels.  we are assuming no unrolling, so if this
	# function doubles the source image and dst_bipp is less than
	# 24, two pixels' bits must be shifted, otherwise one pixel's
	# bits must be shifted:
	#
	if test $scale = _h_; then
	    src_iter_scale=2
	    src_fifo_shift="(src_bipp < 24 ? (src_bipp * 2) : src_bipp)"
	else
	    src_iter_scale=1
	    src_fifo_shift="src_bipp"
	fi
	if test $scale = _d_; then
	    dst_iter_scale=2
	    dst_fifo_shift="(dst_bipp < 24 ? (dst_bipp * 2) : dst_bipp)"
	else
	    dst_iter_scale=1
	    dst_fifo_shift="dst_bipp"
	fi

	# if src_bipp is known now, see how many times we can unroll the
	# translation loop on source pixels:
	#
	if test ${src_bipp} != 0; then

	    # in general, we can unroll once for each pixel in the
	    # visible part of a source bit FIFO:
	    #
	    src_unroll=`expr 32 / ${src_bipp}`

	    # if this function halves the source image, we can unroll
	    # half as many times:
	    #
	    src_unroll=`expr ${src_unroll} / ${src_iter_scale}`

	    # if we think we can unroll zero times, this means that
	    # src_bipp is greater than 16 and this function halves the
	    # source image - the zero is telling us that two whole
	    # pixels cannot fit in the visible part of a source bit
	    # FIFO.  so we unroll once on source pixels, and shift
	    # only one pixel's bits after the unrolling (the halving
	    # code will be shifting one pixel's worth of bits to get
	    # to the next pixel it has to read):
	    #
	    if test ${src_unroll} = 0; then
		src_unroll=1
		src_fifo_shift=${src_bipp}

	    # otherwise, we will be shifting the entire visible part
	    # of the source bit FIFO(s) after all unrolled iterations:
	    #
	    else
		src_fifo_shift=32
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "    /* since src_bipp is known at code-generation time, the"
	    printf "%s\n" "       pixel translation loop is unrolled to translate all"
	    printf "%s\n" "       source pixels in the 32-bit visible part of the source"
	    printf "%s\n" "       bit FIFO(s) before shifting."
	    printf "%s\n" ""
	    printf "%s\n" "       in this case, src_bipp is known to be ${src_bipp}, so "`expr ${src_unroll} \* ${src_iter_scale}`" pixels will"
	    printf "%s\n" "       be read out of the source bit FIFO(s) before shifting, and"
	    printf "%s\n" "       when the source bit FIFO(s) are shifted, they are shifted"
	    printf "%s\n" "       ${src_fifo_shift} bits at a time: */"
	fi

	# if dst_bipp is known now, see how many times we can unroll the
	# translation loop on destination pixels:
	#
	if test ${dst_bipp} != 0; then

	    # in general, we can unroll once for each pixel in the
	    # visible part of a destination bit FIFO:
	    #
	    dst_unroll=`expr 32 / ${dst_bipp}`

	    # if this function doubles the source image, we can unroll
	    # half as many times:
	    #
	    dst_unroll=`expr ${dst_unroll} / ${dst_iter_scale}`

	    # if we think we can unroll zero times, this means that
	    # dst_bipp is greater than 16 and this function doubles the
	    # source image - the zero is telling us that two whole
	    # pixels cannot fit in the visible part of a destination bit
	    # FIFO.  so we unroll once on destination pixels, and shift
	    # only one pixel's bits after the unrolling (the doubling
	    # code will be shifting one pixel's worth of bits to get
	    # out the first pixel it has to write):
	    #
	    if test ${dst_unroll} = 0; then
		dst_unroll=1
		dst_fifo_shift=${dst_bipp}

	    # otherwise, we will be shifting the entire visible part
	    # of the source bit FIFO(s) after all unrolled iterations:
	    #
	    else
		dst_fifo_shift=32
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "    /* since dst_bipp is known at code-generation time, the pixel"
	    printf "%s\n" "       translation loop is unrolled to translate all destination"
	    printf "%s\n" "       pixels in the 32-bit visible part of the destination bit"
	    printf "%s\n" "       FIFO(s) before shifting."
	    printf "%s\n" ""
	    printf "%s\n" "       in this case, dst_bipp is known to be ${dst_bipp}, so "`expr ${dst_unroll} \* ${dst_iter_scale}`" pixels will"
	    printf "%s\n" "       be written into the destination bit FIFO(s) before shifting,"
	    printf "%s\n" "       and when the destination bit FIFO(s) are shifted, they are"
	    printf "%s\n" "       shifted ${dst_fifo_shift} bits at a time: */"
	fi

	# unroll the translation loop the maximum number amount of times:
	#
	if test `expr ${src_unroll} \> ${dst_unroll}` = 1; then
	    unroll=${src_unroll}
	else
	    unroll=${dst_unroll}
	fi

	printf "%s\n" ""
	printf "%s\n" "    /* src_unroll = ${src_unroll}, src_iter_scale = ${src_iter_scale}"
	printf "%s\n" "       dst_unroll = ${dst_unroll}, dst_iter_scale = ${dst_iter_scale} */"
	printf "%s\n" "    for (xlat_run = TME_FB_XLAT_RUN;"
	printf "%s\n" "         xlat_run > 0; ) {"
	iter=0

	while test `expr ${iter} \< ${unroll}` = 1; do
	    printf "%s\n" ""
	    printf "%s\n" "      /* iter #${iter} */"
	    printf "%s\n" ""
	    iter_next=`expr ${iter} + 1`

	    # the number of bits to skip in a source FIFO to get to
	    # this iteration's pixel:
	    src_shift=`expr ${iter} % ${src_unroll}`
	    src_shift='('`expr ${src_shift} \* ${src_iter_scale}`' * src_bipp)'

	    printf "%s\n" "      /* get a pixel from the source primary FIFO: */"
	    printf "%s\n" "      pixel ="
	    printf "%s\n" "        ((src_fifo0"
	    printf "%s\n" "          >> (src_order == TME_ENDIAN_BIG"
	    printf "%s\n" "              ? (32 - (src_bipp + ${src_shift}))"
	    printf "%s\n" "              : ${src_shift})));"
	    indent0=X

	    if test $scale != _h_; then

		if test "x${src_mask_g}" = x; then
		    printf "%s\n" ""
		    printf "%s\n" "      /* since source pixels are known at compile time to"
		    printf "%s\n" "         not have subfields, map the source pixel into the"
		    printf "%s\n" "         destination pixel: */"
		    printf "%s\n" "      pixel = dst->tme_fb_connection_map_pixel[pixel & src_mask];"
		else
		    printf "%s\n" ""
		    printf "%s\n" "      /* if the source depth is within the threshold for pixel"
		    printf "%s\n" "         mapping, or if source pixels don't have subfields,"
		    printf "%s\n" "         map the source pixel into the destination pixel: */"
		    printf "%s\n" "      if (src_mask <= TME_FB_XLAT_MAP_INDEX_MASK_MAX"
		    printf "%s\n" "          || src_mask_g == src_mask) {"
		    printf "%s\n" "        pixel = dst->tme_fb_connection_map_pixel[pixel & src_mask];"
		    printf "%s\n" "      }"
		    printf "%s\n" ""
		    printf "%s\n" "      /* otherwise, we will decompose this source pixel"
		    printf "%s\n" "         and then compose the destination pixel: */"
		    printf "%s\n" "      else {"
		    printf "%s\n" ""
		    printf "%s\n" "        /* map the pixel subfields into intensities: */"
		    printf "%s\n" "        value_g = TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		    if test "x${src_class}" != xm; then
			printf "%s\n" "        value_r = TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
			printf "%s\n" "        value_b = TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		    fi
		    indent0="  "
		    src_mask_i=src_mask_i
		    src_mask_i_max=TME_FB_XLAT_MAP_INDEX_MASK_MAX
		fi

	    else

		printf "%s\n" ""
		printf "%s\n" "      /* map the pixel subfields into intensities: */"
		printf "%s\n" "      value_g = TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "      value_r = TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "      value_b = TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi

		printf "%s\n" ""
		printf "%s\n" "      /* get a second pixel, from the source secondary FIFO: */"
		printf "%s\n" "      pixel ="
		printf "%s\n" "        ((src_fifo1"
		printf "%s\n" "          >> (src_order == TME_ENDIAN_BIG"
		printf "%s\n" "              ? (32 - (src_bipp + ${src_shift}))"
		printf "%s\n" "              : ${src_shift})));"

		printf "%s\n" ""
		printf "%s\n" "      /* map the pixel subfields into intensities and accumulate them: */"
		printf "%s\n" "      value_g += TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "      value_r += TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "      value_b += TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi

		printf "%s\n" ""
		printf "%s\n" "      /* get third and fourth pixels, from the source primary"
		printf "%s\n" "         FIFO and source secondary FIFO, respectively.  if"
		printf "%s\n" "         src_bipp is 24 or greater, these pixels are not yet"
		printf "%s\n" "         entirely in the first parts of the FIFOs, so we need"
		printf "%s\n" "         to shift: */"
		printf "%s\n" "      if (src_bipp >= 24) {"
		printf "%s\n" "        TME_FB_XLAT_SHIFT_SRC(src_fifo0_may_be_unaligned,"
		printf "%s\n" "                              src_fifo0,"
		printf "%s\n" "                              src_fifo0_next,"
		printf "%s\n" "                              src_fifo0_bits,"
		printf "%s\n" "                              src_bipp,"
		printf "%s\n" "                              src_raw0,"
		printf "%s\n" "                              src_order);"
		printf "%s\n" "        TME_FB_XLAT_SHIFT_SRC(src_fifo1_may_be_unaligned,"
		printf "%s\n" "                              src_fifo1,"
		printf "%s\n" "                              src_fifo1_next,"
		printf "%s\n" "                              src_fifo1_bits,"
		printf "%s\n" "                              src_bipp,"
		printf "%s\n" "                              src_raw1,"
		printf "%s\n" "                              src_order);"
		printf "%s\n" "        pixel ="
		printf "%s\n" "          ((src_fifo0"
		printf "%s\n" "            >> (src_order == TME_ENDIAN_BIG"
		printf "%s\n" "                ? (32 - (src_bipp + ${src_shift}))"
		printf "%s\n" "                : ${src_shift})));"

		printf "%s\n" ""
		printf "%s\n" "        /* map the pixel subfields into intensities and accumulate them: */"
		printf "%s\n" "        value_g += TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "        value_r += TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "        value_b += TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi
		printf "%s\n" ""

		printf "%s\n" "        pixel ="
		printf "%s\n" "          ((src_fifo1"
		printf "%s\n" "            >> (src_order == TME_ENDIAN_BIG"
		printf "%s\n" "                ? (32 - (src_bipp + ${src_shift}))"
		printf "%s\n" "                : ${src_shift})));"

		printf "%s\n" ""
		printf "%s\n" "        /* map the pixel subfields into intensities and accumulate them: */"
		printf "%s\n" "        value_g += TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "        value_r += TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "        value_b += TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi
		printf "%s\n" "      }"
		printf "%s\n" ""
		printf "%s\n" "      /* otherwise, the third and fourth pixels are already in the"
		printf "%s\n" "         visible parts of the source bit FIFOs; we just have to"
		printf "%s\n" "         reach over the pixels we already read to get at them: */"
		printf "%s\n" "      else {"
		printf "%s\n" "        pixel ="
		printf "%s\n" "          ((src_fifo0"
		printf "%s\n" "            >> (src_order == TME_ENDIAN_BIG"
		printf "%s\n" "                ? (32 - (src_bipp + (${src_shift} + src_bipp)))"
		printf "%s\n" "                : (${src_shift} + src_bipp))));"
		printf "%s\n" ""
		printf "%s\n" "        /* map the pixel subfields into intensities and accumulate them: */"
		printf "%s\n" "        value_g += TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "        value_r += TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "        value_b += TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi
		printf "%s\n" ""
		printf "%s\n" "        pixel ="
		printf "%s\n" "          ((src_fifo1"
		printf "%s\n" "            >> (src_order == TME_ENDIAN_BIG"
		printf "%s\n" "                ? (32 - (src_bipp + (${src_shift} + src_bipp)))"
		printf "%s\n" "                : (${src_shift} + src_bipp))));"
		printf "%s\n" ""
		printf "%s\n" "        /* map the pixel subfields into intensities and accumulate them: */"
		printf "%s\n" "        value_g += TME_FB_XLAT_MAP(pixel, src_mask_g, src_mask_i, src_indexed, src->tme_fb_connection_map_g);"
		if test "x${src_class}" != xm; then
		    printf "%s\n" "        value_r += TME_FB_XLAT_MAP(pixel, src_mask_r, src_mask_i, src_indexed, src->tme_fb_connection_map_r);"
		    printf "%s\n" "        value_b += TME_FB_XLAT_MAP(pixel, src_mask_b, src_mask_i, src_indexed, src->tme_fb_connection_map_b);"
		fi
		printf "%s\n" "      }"
		indent0=
		src_mask_i='((src_mask_i * 4) + 3)'
		src_mask_i_max='(TME_FB_XLAT_MAP_INDEX_MASK_MAX / 4)'
	    fi

	    # if we may have intensities:
	    #
	    if test "X${indent0}" != XX; then
		printf "%s\n" ""
		indent1=X
		case "x${src_class}" in
		xm)
		    printf "%s\n" "${indent0}      /* since the source_class is known as monochrome at code-generation"
		    printf "%s\n" "${indent0}         time, we map the green intensity directly into a pixel.  we may have"
		    printf "%s\n" "${indent0}         to scale the intensity to be in the lookup range: */"
		    printf "%s\n" "${indent0}      if (src_mask_i > ${src_mask_i_max}) {"
		    printf "%s\n" "${indent0}        value_g /= TME_FB_XLAT_MAP_LINEAR_SCALE(${src_mask_i}, TME_FB_XLAT_MAP_INDEX_MASK_MAX);"
		    printf "%s\n" "${indent0}      }"
		    printf "%s\n" "${indent0}      pixel = dst->tme_fb_connection_map_pixel[value_g];"
		    ;;
		x)
		    printf "%s\n" "${indent0}      /* if the source class is monochrome, we map the green intensity"
		    printf "%s\n" "${indent0}         directly into a pixel.  we may have to scale the intensity"
		    printf "%s\n" "${indent0}         to be in the lookup range: */"
		    printf "%s\n" "${indent0}      if (src->tme_fb_connection_class == TME_FB_XLAT_CLASS_MONOCHROME) {"
		    printf "%s\n" "${indent0}        if (src_mask_i > ${src_mask_i_max}) {"
		    printf "%s\n" "${indent0}          value_g /= TME_FB_XLAT_MAP_LINEAR_SCALE(${src_mask_i}, TME_FB_XLAT_MAP_INDEX_MASK_MAX);"
		    printf "%s\n" "${indent0}        }"
		    printf "%s\n" "${indent0}        pixel = dst->tme_fb_connection_map_pixel[value_g];"
		    printf "%s\n" "${indent0}      }"
		    printf "%s\n" ""
		    printf "%s\n" "${indent0}      /* otherwise, we have to consider all three intensities: */"
		    printf "%s\n" "${indent0}      else {"
		    indent1="${indent0}  "
		    ;;
		*)
		    printf "%s\n" "${indent0}      /* we have to consider all three intensities: */"
		    indent1="${indent0}"
		esac

		if test "X${indent1}" != "XX"; then
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}      /* if destination intensities are indexed: */"
		    printf "%s\n" "${indent1}      if (dst_indexed) {"
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}        /* we may have to scale the intensities to be in the lookup range: */"
		    printf "%s\n" "${indent1}        if (src_mask_i > ${src_mask_i_max}) {"
		    printf "%s\n" "${indent1}          value_g /= TME_FB_XLAT_MAP_LINEAR_SCALE(${src_mask_i}, TME_FB_XLAT_MAP_INDEX_MASK_MAX);"
		    printf "%s\n" "${indent1}          value_r /= TME_FB_XLAT_MAP_LINEAR_SCALE(${src_mask_i}, TME_FB_XLAT_MAP_INDEX_MASK_MAX);"
		    printf "%s\n" "${indent1}          value_b /= TME_FB_XLAT_MAP_LINEAR_SCALE(${src_mask_i}, TME_FB_XLAT_MAP_INDEX_MASK_MAX);"
		    printf "%s\n" "${indent1}        }"
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}        /* form the pixel: */"
		    printf "%s\n" "${indent1}        pixel  = _TME_FB_XLAT_MAP_INDEX(value_g, dst_mask_g, dst->tme_fb_connection_map_g);"
		    printf "%s\n" "${indent1}        pixel |= _TME_FB_XLAT_MAP_INDEX(value_r, dst_mask_r, dst->tme_fb_connection_map_r);"
		    printf "%s\n" "${indent1}        pixel |= _TME_FB_XLAT_MAP_INDEX(value_b, dst_mask_b, dst->tme_fb_connection_map_b);"
		    printf "%s\n" "${indent1}        pixel |= 0xff000000;"
		    printf "%s\n" "${indent1}      }"
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}      /* otherwise, destination intensities are linear: */"
		    printf "%s\n" "${indent1}      else {"
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}        /* form the pixel: */"
		    printf "%s\n" "${indent1}        pixel  = _TME_FB_XLAT_MAP_LINEAR(value_g, ${src_mask_i}, dst_mask_g);"
		    printf "%s\n" "${indent1}        pixel |= _TME_FB_XLAT_MAP_LINEAR(value_r, ${src_mask_i}, dst_mask_r);"
		    printf "%s\n" "${indent1}        pixel |= _TME_FB_XLAT_MAP_LINEAR(value_b, ${src_mask_i}, dst_mask_b);"
		    printf "%s\n" "${indent1}        pixel |= 0xff000000;"
		    printf "%s\n" ""
		    printf "%s\n" "${indent1}        /* if destination pixels are indexed: */"
		    printf "%s\n" "${indent1}        if (dst_masks_default) {"
		    printf "%s\n" "${indent1}          pixel = dst->tme_fb_connection_map_pixel[pixel];"
		    printf "%s\n" "${indent1}        }"
		    printf "%s\n" "${indent1}      }"

		    if test "X${indent1}" != "X${indent0}"; then
			printf "%s\n" "${indent0}      }"
		    fi
		    if test "X${indent0}" != X; then
			printf "%s\n" "${indent0}    }"
		    fi
		fi
	    fi

	    printf "%s\n" ""
	    if test $scale = _h_; then

		printf "%s\n" "      /* if we just read the last pixels on these"
		printf "%s\n" "         source scanlines: */"
		if test `expr ${iter_next} % ${src_unroll}` = 0; then
		    printf "%s\n" "      if ((src_x +="
		    printf "%s\n" "           (src_packed"
		    printf "%s\n" "            ? `expr ${src_unroll} \* 2`"
		    printf "%s\n" "            : 2))"
		    printf "%s\n" "           == src_width) {"
		else
		    printf "%s\n" "      if (!src_packed"
		    printf "%s\n" "          && (src_x += 2) == src_width) {"
		fi
		printf "%s\n" ""
		printf "%s\n" "        /* we need to rapidly shift the source FIFOs"
		printf "%s\n" "           to skip not only pad bits and undisplayed"
		printf "%s\n" "           pixels on the next line, but actually the"
		printf "%s\n" "           *entire* next line."
		printf "%s\n" ""
		printf "%s\n" "           note that this sounds like when we're done,"
		printf "%s\n" "           the bits at the fronts of the FIFOs will be"
		printf "%s\n" "           the *first* pixels on the next scanlines."
		printf "%s\n" ""
		printf "%s\n" "           but the bits at the fronts of the FIFOs now"
		printf "%s\n" "           are the *last* pixels on the current scanlines -"
		printf "%s\n" "           they haven't been shifted off yet.  so when"
		printf "%s\n" "           we're done, we want one pixel's worth of bits"
		printf "%s\n" "           in the FIFOs before the first pixel on the"
		printf "%s\n" "           next scanlines: */"
		printf "%s\n" ""
		printf "%s\n" "        /* calculate the number of bits that we need"
		printf "%s\n" "           to shift the source primary FIFO, after"
		printf "%s\n" "           discarding any bits in it now: */"
		printf "%s\n" "        src_off = ((src_bypl * 8) - (src_width * src_bipp)) + (src_bypl * 8);"
		printf "%s\n" "        src_off -= (src_fifo0_may_be_unaligned"
		printf "%s\n" "                    ? src_fifo0_bits"
		printf "%s\n" "                    : 32);"
		printf "%s\n" ""
		printf "%s\n" "        /* rapidly advance src_raw0 by the number of"
		printf "%s\n" "           whole 32-bit words.  this will leave it pointing"
		printf "%s\n" "           to the 32-bit word that has the first bit that"
		printf "%s\n" "           we want to end up as the first bit in the source"
		printf "%s\n" "           primary FIFO: */"
		printf "%s\n" "        src_raw0 += (src_off / 32);"
		printf "%s\n" ""
		printf "%s\n" "        /* reprime the source primary FIFO: */"
		printf "%s\n" "        src_fifo0 = *src_raw0;"
		printf "%s\n" "        *TME_FB_XLAT_SRC_OLD(src_raw0) = src_fifo0;"
		printf "%s\n" "        src_raw0++;"
		printf "%s\n" "        src_fifo0 = ((src_order == TME_ENDIAN_BIG)"
		printf "%s\n" "                     ? tme_betoh_u32(src_fifo0)"
		printf "%s\n" "                     : tme_letoh_u32(src_fifo0));"
		printf "%s\n" ""
		printf "%s\n" "        /* if the source primary FIFO may be unaligned: */"
		printf "%s\n" "        if (src_fifo0_may_be_unaligned) {"
		printf "%s\n" ""
		printf "%s\n" "          /* reprime the top half of the FIFO, leaving a"
		printf "%s\n" "             total of 64 bits in it: */"
		printf "%s\n" "          src_fifo0_next = *src_raw0;"
		printf "%s\n" "          *TME_FB_XLAT_SRC_OLD(src_raw0) = src_fifo0_next;"
		printf "%s\n" "          src_raw0++;"
		printf "%s\n" "          src_fifo0_next = ((src_order == TME_ENDIAN_BIG)"
		printf "%s\n" "                            ? tme_betoh_u32(src_fifo0_next)"
		printf "%s\n" "                            : tme_letoh_u32(src_fifo0_next));"
		printf "%s\n" "          src_fifo0_bits = 64;"
		printf "%s\n" ""
		printf "%s\n" "          /* if we have to shift off bits left over"
		printf "%s\n" "             from rapidly advancing whole 32-bit words: */"
		printf "%s\n" "          src_off %= 32;"
		printf "%s\n" "          if (src_off > 0) {"
		printf "%s\n" "            TME_FB_XLAT_SHIFT_SRC(TRUE,"
		printf "%s\n" "                                  src_fifo0,"
		printf "%s\n" "                                  src_fifo0_next,"
		printf "%s\n" "                                  src_fifo0_bits,"
		printf "%s\n" "                                  src_off,"
		printf "%s\n" "                                  src_raw0,"
		printf "%s\n" "                                  src_order);"
		printf "%s\n" "          }"
		printf "%s\n" "        }"
		printf "%s\n" ""
		printf "%s\n" "        /* otherwise, the source primary FIFO is always aligned: */"
		printf "%s\n" "        else {"
		printf "%s\n" "          assert ((src_off % 32) == 0);"
		printf "%s\n" "        }"
		printf "%s\n" ""
		printf "%s\n" "        /* calculate the number of bits that we need"
		printf "%s\n" "           to shift the source secondary FIFO, after"
		printf "%s\n" "           discarding any bits in it now: */"
		printf "%s\n" "        src_off = ((src_bypl * 8) - (src_width * src_bipp)) + (src_bypl * 8);"
		printf "%s\n" "        src_off -= (src_fifo1_may_be_unaligned"
		printf "%s\n" "                    ? src_fifo1_bits"
		printf "%s\n" "                    : 32);"
		printf "%s\n" ""
		printf "%s\n" "        /* rapidly advance src_raw1 by the number of"
		printf "%s\n" "           whole 32-bit words.  this will leave it pointing"
		printf "%s\n" "           to the 32-bit word that has the first bit that"
		printf "%s\n" "           we want to end up as the first bit in the source"
		printf "%s\n" "           secondary FIFO: */"
		printf "%s\n" "        src_raw1 += (src_off / 32);"
		printf "%s\n" ""
		printf "%s\n" "        /* reprime the source secondary FIFO: */"
		printf "%s\n" "        src_fifo1 = *src_raw1;"
		printf "%s\n" "        *TME_FB_XLAT_SRC_OLD(src_raw1) = src_fifo1;"
		printf "%s\n" "        src_raw1++;"
		printf "%s\n" "        src_fifo1 = ((src_order == TME_ENDIAN_BIG)"
		printf "%s\n" "                     ? tme_betoh_u32(src_fifo1)"
		printf "%s\n" "                     : tme_letoh_u32(src_fifo1));"
		printf "%s\n" ""
		printf "%s\n" "        /* if the source secondary FIFO may be unaligned: */"
		printf "%s\n" "        if (src_fifo1_may_be_unaligned) {"
		printf "%s\n" ""
		printf "%s\n" "          /* reprime the top half of the FIFO, leaving a"
		printf "%s\n" "             total of 64 bits in it: */"
		printf "%s\n" "          src_fifo1_next = *src_raw1;"
		printf "%s\n" "          *TME_FB_XLAT_SRC_OLD(src_raw1) = src_fifo1_next;"
		printf "%s\n" "          src_raw1++;"
		printf "%s\n" "          src_fifo1_next = ((src_order == TME_ENDIAN_BIG)"
		printf "%s\n" "                            ? tme_betoh_u32(src_fifo1_next)"
		printf "%s\n" "                            : tme_letoh_u32(src_fifo1_next));"
		printf "%s\n" "          src_fifo1_bits = 64;"
		printf "%s\n" ""
		printf "%s\n" "          /* if we have to shift off bits left over"
		printf "%s\n" "             from rapidly advancing whole 32-bit words: */"
		printf "%s\n" "          src_off %= 32;"
		printf "%s\n" "          if (src_off > 0) {"
		printf "%s\n" "            TME_FB_XLAT_SHIFT_SRC(TRUE,"
		printf "%s\n" "                                  src_fifo1,"
		printf "%s\n" "                                  src_fifo1_next,"
		printf "%s\n" "                                  src_fifo1_bits,"
		printf "%s\n" "                                  src_off,"
		printf "%s\n" "                                  src_raw1,"
		printf "%s\n" "                                  src_order);"
		printf "%s\n" "          }"
		printf "%s\n" "        }"
		printf "%s\n" ""
		printf "%s\n" "        /* otherwise, the source secondary FIFO is always aligned: */"
		printf "%s\n" "        else {"
		printf "%s\n" "          assert ((src_off % 32) == 0);"
		printf "%s\n" "        }"
		printf "%s\n" ""
		printf "%s\n" "        /* we are now on the first pixel of the next scanline: */"
		printf "%s\n" "        src_x = 0;"
		printf "%s\n" "      }"
	    else
		printf "%s\n" "      /* if the source buffer is not packed, and we just"
		printf "%s\n" "         read the last pixel on this source scanline: */"
		printf "%s\n" "      if (!src_packed"
		printf "%s\n" "          && ++src_x == src_width) {"
		printf "%s\n" ""
		printf "%s\n" "        /* calculate the number of bits between the"
		printf "%s\n" "           last bit of the last pixel and the first bit"
		printf "%s\n" "           of the first displayed pixel on the next"
		printf "%s\n" "           scanline.  this is equal to the number of"
		printf "%s\n" "           pad bits plus bits for undisplayed pixels: */"
		printf "%s\n" "        src_off = ((src_bypl * 8) - (src_width * src_bipp));"
		printf "%s\n" ""
		printf "%s\n" "        /* while there are bits to shift: */"
		printf "%s\n" "        for (; src_off > 0; src_off -= TME_MIN(src_off, 32)) {"
		printf "%s\n" ""
		printf "%s\n" "          /* shift the source primary FIFO: */"
		printf "%s\n" "          TME_FB_XLAT_SHIFT_SRC(src_fifo0_may_be_unaligned,"
		printf "%s\n" "                                src_fifo0,"
		printf "%s\n" "                                src_fifo0_next,"
		printf "%s\n" "                                src_fifo0_bits,"
		printf "%s\n" "                                TME_MIN(src_off, 32),"
		printf "%s\n" "                                src_raw0,"
		printf "%s\n" "                                src_order);"
		printf "%s\n" "        }"
		printf "%s\n" ""
		printf "%s\n" "        /* we are now on the first pixel of the next scanline: */"
		printf "%s\n" "        src_x = 0;"
		printf "%s\n" "      }"
	    fi

	    if test `expr ${iter_next} % ${src_unroll}` = 0; then

		printf "%s\n" ""
	        if test "${src_fifo_shift}" = 32; then
		    printf "%s\n" "      /* we've just translated another 32-bit word of the"
		    printf "%s\n" "         source image, so decrement xlat_run: */"
		    printf "%s\n" "      xlat_run--;"
		else
		    printf "%s\n" "      /* if we've just finished translating another 32-bit"
		    printf "%s\n" "         word of the source image, decrement xlat_run: */"
		    printf "%s\n" "      if (src_fifo0_bits <= 32 + ${src_fifo_shift}) {"
		    printf "%s\n" "        xlat_run--;"
		    printf "%s\n" "      }"
		fi
		printf "%s\n" ""
		printf "%s\n" "      /* shift the source primary FIFO: */"
		printf "%s\n" "      TME_FB_XLAT_SHIFT_SRC(src_fifo0_may_be_unaligned,"
		printf "%s\n" "                            src_fifo0,"
		printf "%s\n" "                            src_fifo0_next,"
		printf "%s\n" "                            src_fifo0_bits,"
		printf "%s\n" "                            ${src_fifo_shift},"
		printf "%s\n" "                            src_raw0,"
		printf "%s\n" "                            src_order);"
		if test $scale = _h_; then
		    printf "%s\n" ""
		    printf "%s\n" "      /* shift the source secondary FIFO: */"
		    printf "%s\n" "      TME_FB_XLAT_SHIFT_SRC(src_fifo1_may_be_unaligned,"
		    printf "%s\n" "                            src_fifo1,"
		    printf "%s\n" "                            src_fifo1_next,"
		    printf "%s\n" "                            src_fifo1_bits,"
		    printf "%s\n" "                            ${src_fifo_shift},"
		    printf "%s\n" "                            src_raw1,"
		    printf "%s\n" "                            src_order);"
		fi
	    fi

	    # the number of bits to skip in a destination FIFO to get to
	    # this iteration's pixel:
	    dst_shift=`expr ${iter} % ${dst_unroll}`
	    dst_shift='('`expr ${dst_shift} \* ${dst_iter_scale}`' * dst_bipp)'

	    printf "%s\n" ""
	    printf "%s\n" "      /* put the pixel into the destination primary FIFO: */"
	    printf "%s\n" "      dst_fifo0 |="
	    printf "%s\n" "        (pixel"
	    printf "%s\n" "         << (dst_order == TME_ENDIAN_BIG"
	    printf "%s\n" "             ? ((32 - dst_bipp) - ${dst_shift})"
	    printf "%s\n" "             : ${dst_shift}));"

	    if test $scale = _d_; then

		printf "%s\n" ""
		printf "%s\n" "      /* put the pixel into the destination secondary FIFO: */"
		printf "%s\n" "      dst_fifo1 |="
		printf "%s\n" "        (pixel"
		printf "%s\n" "         << (dst_order == TME_ENDIAN_BIG"
		printf "%s\n" "             ? ((32 - dst_bipp) - ${dst_shift})"
		printf "%s\n" "             : ${dst_shift}));"

		printf "%s\n" ""
		if test `expr ${dst_bipp} \>= 24` = 1; then
		    printf "%s\n" "      /* put the pixel into both FIFOs again.  in"
		    printf "%s\n" "         this case, dst_bipp is known to be ${dst_bipp},"
		    printf "%s\n" "         meaning the FIFOs cannot entirely take these"
		    printf "%s\n" "         further pixels, so we need to shift the FIFOs: */"
		    indent0=""
		    indent1=X
		elif test ${dst_bipp} = 0; then
		    printf "%s\n" "      /* put the pixel into both FIFOs again.  if"
		    printf "%s\n" "         dst_bipp is 24 or greater, the FIFOs can"
		    printf "%s\n" "         not entirely take these further pixels,"
		    printf "%s\n" "         so we need to shift the FIFOs: */"
		    printf "%s\n" "      if (dst_bipp >= 24) {"
		    indent0="  "
		    indent1="  "
		else
		    printf "%s\n" "      /* put the pixel into both FIFOs again.  in"
		    printf "%s\n" "         this case, dst_bipp is known to be ${dst_bipp},"
		    printf "%s\n" "         meaning the FIFOs can take these further pixels"
		    printf "%s\n" "         without shifting the FIFOs, as long as we shift"
		    printf "%s\n" "         the pixels one pixel further: */"
		    indent0=X
		    indent1=""
		fi
		if test "X${indent0}" != "XX"; then
		    printf "%s\n" "${indent0}      TME_FB_XLAT_SHIFT_DST(dst_fifo0_may_be_unaligned,"
		    printf "%s\n" "${indent0}                            dst_fifo0,"
		    printf "%s\n" "${indent0}                            dst_fifo0_next,"
		    printf "%s\n" "${indent0}                            dst_fifo0_bits,"
		    printf "%s\n" "${indent0}                            dst_bipp,"
		    printf "%s\n" "${indent0}                            dst_raw0,"
		    printf "%s\n" "${indent0}                            dst_order);"
		    printf "%s\n" "${indent0}      TME_FB_XLAT_SHIFT_DST(dst_fifo1_may_be_unaligned,"
		    printf "%s\n" "${indent0}                            dst_fifo1,"
		    printf "%s\n" "${indent0}                            dst_fifo1_next,"
		    printf "%s\n" "${indent0}                            dst_fifo1_bits,"
		    printf "%s\n" "${indent0}                            dst_bipp,"
		    printf "%s\n" "${indent0}                            dst_raw1,"
		    printf "%s\n" "${indent0}                            dst_order);"
		    printf "%s\n" ""
		    printf "%s\n" "${indent0}      /* now that we've shifted by dst_bipp, we can"
		    printf "%s\n" "${indent0}         put the further pixels exactly where the"
		    printf "%s\n" "${indent0}         first pixels went in the FIFOs: */"
		    printf "%s\n" "${indent0}      dst_fifo0 |="
		    printf "%s\n" "${indent0}        (pixel"
		    printf "%s\n" "${indent0}         << (dst_order == TME_ENDIAN_BIG"
		    printf "%s\n" "${indent0}             ? ((32 - dst_bipp) - ${dst_shift})"
		    printf "%s\n" "${indent0}             : ${dst_shift}));"
		    printf "%s\n" "${indent0}      dst_fifo1 |="
		    printf "%s\n" "${indent0}        (pixel"
		    printf "%s\n" "${indent0}         << (dst_order == TME_ENDIAN_BIG"
		    printf "%s\n" "${indent0}             ? ((32 - dst_bipp) - ${dst_shift})"
		    printf "%s\n" "${indent0}             : ${dst_shift}));"
		fi
		if test ${dst_bipp} = 0; then
		    printf "%s\n" "      }"
		    printf "%s\n" ""
		    printf "%s\n" "      /* otherwise, the FIFOs can take these further pixels,"
		    printf "%s\n" "         as long as we shift the pixels one pixel further: */"
		    printf "%s\n" "      else {"
		fi
		if test "X${indent1}" != "XX"; then
		    printf "%s\n" "${indent1}      dst_fifo0 |="
		    printf "%s\n" "${indent1}        (pixel"
		    printf "%s\n" "${indent1}         << (dst_order == TME_ENDIAN_BIG"
		    printf "%s\n" "${indent1}             ? ((32 - dst_bipp) - (${dst_shift} + dst_bipp))"
		    printf "%s\n" "${indent1}             : (${dst_shift} + dst_bipp)));"
		    printf "%s\n" "${indent1}      dst_fifo1 |="
		    printf "%s\n" "${indent1}        (pixel"
		    printf "%s\n" "${indent1}         << (dst_order == TME_ENDIAN_BIG"
		    printf "%s\n" "${indent1}             ? ((32 - dst_bipp) - (${dst_shift} + dst_bipp))"
		    printf "%s\n" "${indent1}             : (${dst_shift} + dst_bipp)));"
		fi
		if test ${dst_bipp} = 0; then
		    printf "%s\n" "      }"
		fi
	    fi

	    printf "%s\n" ""
	    printf "%s\n" "      /* if the destination buffer is not packed, and we just"
	    printf "%s\n" "         wrote the last pixel on this destination scanline: */"
	    if test $scale = _d_; then value=2; else value=1; fi
	    printf "%s\n" "      if (!dst_packed"
	    printf "%s\n" "          && (dst_x += ${value}) == dst_width) {"
	    printf "%s\n" ""
	    printf "%s\n" "        /* calculate the number of bits between the"
	    printf "%s\n" "           last bit of the last pixel and the first bit"
	    printf "%s\n" "           of the first displayed pixel on the next"
	    printf "%s\n" "           scanline.  this is equal to the number of"
	    printf "%s\n" "           pad bits plus bits for undisplayed pixels: */"
	    printf "%s\n" "        dst_off = ((dst_bypl * 8) - (dst_width * dst_bipp));"
	    printf "%s\n" ""
	    printf "%s\n" "        /* while there are bits to shift: */"
	    printf "%s\n" "        for (; dst_off > 0; dst_off -= TME_MIN(dst_off, 32)) {"
	    printf "%s\n" ""
	    printf "%s\n" "          /* shift the destination primary FIFO: */"
	    printf "%s\n" "          TME_FB_XLAT_SHIFT_DST(dst_fifo0_may_be_unaligned,"
	    printf "%s\n" "                                dst_fifo0,"
	    printf "%s\n" "                                dst_fifo0_next,"
	    printf "%s\n" "                                dst_fifo0_bits,"
	    printf "%s\n" "                                TME_MIN(dst_off, 32),"
	    printf "%s\n" "                                dst_raw0,"
	    printf "%s\n" "                                dst_order);"
	    if test $scale = _d_; then
		printf "%s\n" ""
		printf "%s\n" "          /* shift the destination secondary FIFO: */"
		printf "%s\n" "          TME_FB_XLAT_SHIFT_DST(dst_fifo1_may_be_unaligned,"
		printf "%s\n" "                                dst_fifo1,"
		printf "%s\n" "                                dst_fifo1_next,"
		printf "%s\n" "                                dst_fifo1_bits,"
		printf "%s\n" "                                TME_MIN(dst_off, 32),"
		printf "%s\n" "                                dst_raw1,"
		printf "%s\n" "                                dst_order);"
	    fi
	    printf "%s\n" "        }"
	    printf "%s\n" ""
	    printf "%s\n" "        /* we are now on the first pixel of the next scanline: */"
	    printf "%s\n" "        dst_x = 0;"
	    printf "%s\n" "      }"

	    if test `expr ${iter_next} % ${dst_unroll}` = 0; then
		printf "%s\n" ""
		printf "%s\n" "      /* shift the destination primary FIFO: */"
		printf "%s\n" "      TME_FB_XLAT_SHIFT_DST(dst_fifo0_may_be_unaligned,"
		printf "%s\n" "                            dst_fifo0,"
		printf "%s\n" "                            dst_fifo0_next,"
		printf "%s\n" "                            dst_fifo0_bits,"
		printf "%s\n" "                            ${dst_fifo_shift},"
		printf "%s\n" "                            dst_raw0,"
		printf "%s\n" "                            dst_order);"
		if test $scale = _d_; then
		    printf "%s\n" ""
		    printf "%s\n" "      /* shift the destination secondary FIFO: */"
		    printf "%s\n" "      TME_FB_XLAT_SHIFT_DST(dst_fifo1_may_be_unaligned,"
		    printf "%s\n" "                            dst_fifo1,"
		    printf "%s\n" "                            dst_fifo1_next,"
		    printf "%s\n" "                            dst_fifo1_bits,"
		    printf "%s\n" "                            ${dst_fifo_shift},"
		    printf "%s\n" "                            dst_raw1,"
		    printf "%s\n" "                            dst_order);"
		fi
	    fi

	    iter=${iter_next}
	done
	printf "%s\n" ""
	printf "%s\n" "    }"

	if test `expr ${iter} % ${dst_unroll}` != 0; then
	    printf "%s\n" "$PROG internal error - the last iter did not shift the destination FIFOs ($iter and $dst_unroll)" 1>&2
	    exit 1
	fi

	printf "%s\n" ""
	printf "%s\n" "    /* if the destination FIFOs may be unaligned, there"
	printf "%s\n" "       may be bits left in the FIFO that we need to flush: */"
	printf "%s\n" "    if (dst_fifo0_may_be_unaligned"
	printf "%s\n" "        && dst_fifo0_bits > 0) {"
	printf "%s\n" "      dst_fifo0 = *dst_raw0;"
	printf "%s\n" "      if (dst_order == TME_ENDIAN_BIG) {"
	printf "%s\n" "        dst_fifo0_next |= (tme_betoh_u32(dst_fifo0) & (0xffffffff >> dst_fifo0_bits));"
	printf "%s\n" "        dst_fifo0_next = tme_htobe_u32(dst_fifo0_next);"
	printf "%s\n" "      }"
	printf "%s\n" "      else {"
	printf "%s\n" "        dst_fifo0_next |= (tme_letoh_u32(dst_fifo0) & (0xffffffff << dst_fifo0_bits));"
	printf "%s\n" "        dst_fifo0_next = tme_htole_u32(dst_fifo0_next);"
	printf "%s\n" "      }"
	printf "%s\n" "      *dst_raw0 = dst_fifo0;"
	printf "%s\n" "    }"
	if test $scale = _d_; then
	    printf "%s\n" "    if (dst_fifo1_may_be_unaligned"
	    printf "%s\n" "        && dst_fifo1_bits > 0) {"
	    printf "%s\n" "      dst_fifo1 = *dst_raw1;"
	    printf "%s\n" "      if (dst_order == TME_ENDIAN_BIG) {"
	    printf "%s\n" "        dst_fifo1_next |= (tme_betoh_u32(dst_fifo1) & (0xffffffff >> dst_fifo1_bits));"
	    printf "%s\n" "        dst_fifo1_next = tme_htobe_u32(dst_fifo1_next);"
	    printf "%s\n" "      }"
	    printf "%s\n" "      else {"
	    printf "%s\n" "        dst_fifo1_next |= (tme_letoh_u32(dst_fifo1) & (0xffffffff << dst_fifo1_bits));"
	    printf "%s\n" "        dst_fifo1_next = tme_htole_u32(dst_fifo1_next);"
	    printf "%s\n" "      }"
	    printf "%s\n" "      *dst_raw1 = dst_fifo1;"
	    printf "%s\n" "    }"
	fi

	printf "%s\n" ""
	printf "%s\n" "    /* loop back to compare more 32-bit words: */"
	printf "%s\n" "    src_raw0--;"
	printf "%s\n" "  }"

	printf "%s\n" ""
	printf "%s\n" "  /* return nonzero iff we did some translating: */"
	printf "%s\n" "  return (xlat_run >= 0);"

	printf "%s\n" ""
	for macro in ${undef_macros}; do
	    printf "%s\n" "#undef ${macro}"
	done
	printf "%s\n" "}"

    done
done

printf "%s\n" ""
printf "%s\n" "/* the xlat function array: */"
printf "%s\n" "static const struct tme_fb_xlat tme_fb_xlats[] = {$xlat_array};"

# done:
exit 0
