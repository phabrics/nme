#! /bin/sh
# Generated from sparc-bus-auto.m4 by GNU Autoconf 2.72.
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


# $Id: sparc-bus-auto.sh,v 1.1 2006/09/30 12:55:59 fredette Exp $

# ic/sparc/sparc-bus-auto.sh - automatically generates C code
# for SPARC bus emulation support:

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

header=false

for option
do
    case $option in
    --header) header=true ;;
    esac
done

PROG=`basename $0`
cat <<EOF
/* automatically generated by $PROG, do not edit! */
_TME_RCSID("\$Id: sparc-bus-auto.sh,v 1.1 2006/09/30 12:55:59 fredette Exp $");

/* it was easiest, but probably not the most correct, to reuse the
   m68020 bus router, at least for the early SPARCs.  there are
   probably real differences between the m68020 bus router and the bus
   router used by the MB89600, for example, but I don't have a
   datasheet for the latter.  the placement of devices in the Sun
   4/2x0 address space, however, hints that the two probably are very
   similar. */

/* we use OP3, OP2, OP1, and OP0 to represent bytes of lesser
   significance to more significance, respectively, matching Table 5-5
   in the MC68020 User's Manual (linear page 56 in my .ps copy).

   the Motorola OPn convention numbers bytes by decreasing
   significance (OP2 is less significant than OP1), and since Motorola
   CPUs are big-endian, this means that a higher numbered byte is
   meant to go to a higher address, which is good, because we can then
   use this to easily form indexes for TME_BUS_LANE_ROUTE, which
   expects a higher numbered index to correspond to a higher address
   in memory.

   however, since the same Motorola OPn convention always calls the
   least significant byte of any value OP3, regardless of the total
   size of the value, we need to adjust each OPn given the total
   size of the value, so that OP3 in a 24-bit value means address + 2,
   but OP3 in a 32-bit value means address + 3: */
#define SIZ8_OP(n)	((n) - 3)
#define SIZ16_OP(n)	((n) - 2)
#define SIZ24_OP(n)	((n) - 1)
#define SIZ32_OP(n)	((n) - 0)
EOF

# emit the 32-bit bus router:
if $header; then :; else
    printf "%s\n" ""
    printf "%s\n" "/* the 32-bit bus router used on the early SPARCs: */"
    printf "%s\n" "static const tme_bus_lane_t tme_sparc32_router[TME_SPARC_BUS_ROUTER_SIZE(TME_BUS32_LOG2)] = {"

    # permute over maximum cycle size:
    for transfer in 1 2 3 4; do
	# these are real 68020 SIZ1 and SIZ0 bits:
	case ${transfer} in
	1) transfer_bits="01" ;;
	2) transfer_bits="10" ;;
	3) transfer_bits="11" ;;
	4) transfer_bits="00" ;;
	esac

	# permute over A1 and A0:
	for address in 0 1 2 3; do
	    case $address in
	    0) address_bits=00 ;;
	    1) address_bits=01 ;;
	    2) address_bits=10 ;;
	    3) address_bits=11 ;;
	    esac

	    # permute over the size of the responding device's port:
	    for port_size in 1 2 4; do

		# permute over the byte lane position of the responding device's port:
		for port_pos in 0 1 2 3; do

		    # get a string describing the byte lanes connected
		    # to this device.  NB that the sparc 32-bit bus
		    # router assumes that 8-bit devices are always
		    # connected to D31-D24, and that 16-bit devices
		    # are always connected to D31-D24 and D23-D16, and
		    # cannot dynamically adapt to other
		    # configurations:
		    port_pos_end=`expr ${port_pos} + ${port_size}`
		    port_pos_lane=$port_pos
		    port_lanes=""
		    while test `expr ${port_pos_lane} \< ${port_pos_end}` = 1; do
			port_lanes=" D"`expr \( \( ${port_pos_lane} + 1 \) \* 8 \) - 1`"-D"`expr ${port_pos_lane} \* 8`"${port_lanes}"
			port_pos_lane=`expr ${port_pos_lane} + 1`
		    done
		    if test `expr ${port_pos_end} \> 4` = 1; then
			port_lanes="${port_lanes} - invalid, array placeholder"
		    elif ( test $port_size = 1 && test $port_pos != 3 ) \
		         || ( test $port_size = 2 && test $port_pos != 2 ); then
		        port_lanes="${port_lanes} - incorrect for 32-bit sparc"
		    fi

		    # find the byte lane that would provide OP3.  note
		    # that it may not exist (lane < 0), or that it may
		    # not be within the port:
		    opn=`expr 4 - ${transfer}`
		    opn_lane=`expr 3 - \( ${address} % ${port_size} \)`
		    op3_lane=`expr \( ${opn_lane} \) - \( 3 - ${opn} \)`

		    printf "%s\n" ""
		    printf "%s\n" "  /* [sparc] initiator maximum cycle size: "`expr ${transfer} \* 8`" bits"
		    printf "%s\n" "     [sparc] initiator A1,A0: ${address_bits}"
		    printf "%s\n" "     [gen]   responder port size: "`expr ${port_size} \* 8`" bits"
		    printf "%s\n" "     [gen]   responder port least lane: ${port_pos} (lanes${port_lanes})"
		    printf "%s\n" "     (code ${transfer}.${address}.${port_size}.${port_pos}, OP3 lane ${op3_lane}): */"

		    # emit the bus router information for each lane:
		    for lane in 0 1 2 3; do
			lane_warn=

			# if the sparc expects this byte lane to be connected
			# to the device at this port size:
			if test `expr ${lane} \< \( 4 - ${port_size} \)` = 0; then

			    # if this lane is routed by the sparc when
			    # reading at this transfer size and
			    # address alignment:
			    opn=`expr 3 - \( ${lane} - ${op3_lane} \)`
			    if test `expr \( ${opn} \) \> 3` = 0 \
			       && test `expr \( ${opn} \) \< \( 4 - ${transfer} \)` = 0; then
				lane_read="OP(${opn})"

			    # otherwise this lane isn't routed by the
			    # sparc when reading at this transfer size
			    # and address alignment:
			    else
				lane_read="IGNORE"
			    fi

			# otherwise the sparc does not expect this byte
			# lane to be connected to the device at this
			# port size:
			else
			    lane_read="IGNORE"

			    # if this lane is connected to the device
			    # anyways, issue a warning:
			    if test `expr ${lane} \< ${port_pos}` = 0 \
			       && test `expr ${lane} \< ${port_pos_end}` = 1; then
				lane_warn=" | TME_BUS_LANE_WARN"
			    fi
			fi

			# dispatch on how this lane is routed by the
			# sparc when writing at this transfer size and
			# address alignment:
			case "${transfer_bits}${address_bits}.${lane}" in
			01??.3) lane_write="OP(3)" ;;
			01??.2) lane_write="OP(3)" ;;
			01??.1) lane_write="OP(3)" ;;
			01??.0) lane_write="OP(3)" ;;

			10?0.3) lane_write="OP(2)" ;;
			10?0.2) lane_write="OP(3)" ;;
			10?0.1) lane_write="OP(2)" ;;
			10?0.0) lane_write="OP(3)" ;;

			10?1.3) lane_write="OP(2)" ;;
			10?1.2) lane_write="OP(2)" ;;
			10?1.1) lane_write="OP(3)" ;;
			10?1.0) lane_write="OP(2)" ;;

			1100.3) lane_write="OP(1)" ;;
			1100.2) lane_write="OP(2)" ;;
			1100.1) lane_write="OP(3)" ;;
			1100.0) lane_write="UNDEF" ;; # XXX this is supposed to be OP0, but we can't deal with that

			1101.3) lane_write="OP(1)" ;;
			1101.2) lane_write="OP(1)" ;;
			1101.1) lane_write="OP(2)" ;;
			1101.0) lane_write="OP(3)" ;;

			1110.3) lane_write="OP(1)" ;;
			1110.2) lane_write="OP(2)" ;;
			1110.1) lane_write="OP(1)" ;;
			1110.0) lane_write="OP(2)" ;;

			1111.3) lane_write="OP(1)" ;;
			1111.2) lane_write="OP(1)" ;;
			1111.1) lane_write="OP(2)" ;;
			1111.0) lane_write="OP(1)" ;;

			0000.3) lane_write="OP(0)" ;;
			0000.2) lane_write="OP(1)" ;;
			0000.1) lane_write="OP(2)" ;;
			0000.0) lane_write="OP(3)" ;;

			0001.3) lane_write="OP(0)" ;;
			0001.2) lane_write="OP(0)" ;;
			0001.1) lane_write="OP(1)" ;;
			0001.0) lane_write="OP(2)" ;;

			0010.3) lane_write="OP(0)" ;;
			0010.2) lane_write="OP(1)" ;;
			0010.1) lane_write="OP(0)" ;;
			0010.0) lane_write="OP(1)" ;;

			0011.3) lane_write="OP(0)" ;;
			0011.2) lane_write="OP(0)" ;;
			0011.1) lane_write="OP(1)" ;;
			0011.0) lane_write="OP(0)" ;;

			esac

			# emit the comment for this lane:
			printf %s "  /* D"`expr \( \( ${lane} + 1 \) \* 8 \) - 1`"-D"`expr ${lane} \* 8`" */	"

			# if this port size/position combination is
			# invalid, override everything and abort if
			# this router entry is ever touched:
			if test `expr ${port_pos_end} \> 4` = 1; then
			    printf "%s\n" "TME_BUS_LANE_ABORT,"
			else
			    if test $lane_read != "IGNORE"; then
				if test $lane_read != $lane_write; then
				    printf "%s\n" "$PROG internal error: code ${transfer}.${address}.${port_size}.${port_pos}, reading $lane_read but writing $lane_write" 1>&2
				    exit 1
				fi
				printf %s "TME_BUS_LANE_ROUTE(SIZ"`expr ${transfer} \* 8`"_$lane_read)"
			    elif test $lane_write = "UNDEF"; then
				printf %s "TME_BUS_LANE_UNDEF"
			    else
				printf %s "TME_BUS_LANE_ROUTE(SIZ"`expr ${transfer} \* 8`"_$lane_write) | TME_BUS_LANE_ROUTE_WRITE_IGNORE"
			    fi
			    printf "%s\n" "${lane_warn},"
			fi
		    done
		done
	    done
	done
   done
   printf "%s\n" "};"
fi

cat <<EOF
#undef SIZ8_OP
#undef SIZ16_OP
#undef SIZ24_OP
#undef SIZ32_OP
EOF

# done:
exit 0
