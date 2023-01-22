#! /bin/sh
# Generated from ../../generic/bus-device-auto.m4 by GNU Autoconf 2.69.
## -------------------- ##
## M4sh Initialization. ##
## -------------------- ##

# Be more Bourne compatible
DUALCASE=1; export DUALCASE # for MKS sh
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on ${1+"$@"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '${1+"$@"}'='"$@"'
  setopt NO_GLOB_SUBST
else
  case `(set -o) 2>/dev/null` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
esac
fi


as_nl='
'
export as_nl
# Printing a long string crashes Solaris 7 /usr/bin/printf.
as_echo='\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\'
as_echo=$as_echo$as_echo$as_echo$as_echo$as_echo
as_echo=$as_echo$as_echo$as_echo$as_echo$as_echo$as_echo
# Prefer a ksh shell builtin over an external printf program on Solaris,
# but without wasting forks for bash or zsh.
if test -z "$BASH_VERSION$ZSH_VERSION" \
    && (test "X`print -r -- $as_echo`" = "X$as_echo") 2>/dev/null; then
  as_echo='print -r --'
  as_echo_n='print -rn --'
elif (test "X`printf %s $as_echo`" = "X$as_echo") 2>/dev/null; then
  as_echo='printf %s\n'
  as_echo_n='printf %s'
else
  if test "X`(/usr/ucb/echo -n -n $as_echo) 2>/dev/null`" = "X-n $as_echo"; then
    as_echo_body='eval /usr/ucb/echo -n "$1$as_nl"'
    as_echo_n='/usr/ucb/echo -n'
  else
    as_echo_body='eval expr "X$1" : "X\\(.*\\)"'
    as_echo_n_body='eval
      arg=$1;
      case $arg in #(
      *"$as_nl"*)
	expr "X$arg" : "X\\(.*\\)$as_nl";
	arg=`expr "X$arg" : ".*$as_nl\\(.*\\)"`;;
      esac;
      expr "X$arg" : "X\\(.*\\)" | tr -d "$as_nl"
    '
    export as_echo_n_body
    as_echo_n='sh -c $as_echo_n_body as_echo'
  fi
  export as_echo_body
  as_echo='sh -c $as_echo_body as_echo'
fi

# The user is always right.
if test "${PATH_SEPARATOR+set}" != set; then
  PATH_SEPARATOR=:
  (PATH='/bin;/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 && {
    (PATH='/bin:/bin'; FPATH=$PATH; sh -c :) >/dev/null 2>&1 ||
      PATH_SEPARATOR=';'
  }
fi


# IFS
# We need space, tab and new line, in precisely that order.  Quoting is
# there to prevent editors from complaining about space-tab.
# (If _AS_PATH_WALK were called with IFS unset, it would disable word
# splitting by setting IFS to empty value.)
IFS=" ""	$as_nl"

# Find who we are.  Look in the path if we contain no directory separator.
as_myself=
case $0 in #((
  *[\\/]* ) as_myself=$0 ;;
  *) as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
for as_dir in $PATH
do
  IFS=$as_save_IFS
  test -z "$as_dir" && as_dir=.
    test -r "$as_dir/$0" && as_myself=$as_dir/$0 && break
  done
IFS=$as_save_IFS

     ;;
esac
# We did not find ourselves, most probably we were run as `sh COMMAND'
# in which case we are not to be found in the path.
if test "x$as_myself" = x; then
  as_myself=$0
fi
if test ! -f "$as_myself"; then
  $as_echo "$as_myself: error: cannot find myself; rerun with an absolute file name" >&2
  exit 1
fi

# Unset variables that we do not need and which cause bugs (e.g. in
# pre-3.0 UWIN ksh).  But do not cause bugs in bash 2.01; the "|| exit 1"
# suppresses any "Segmentation fault" message there.  '((' could
# trigger a bug in pdksh 5.2.14.
for as_var in BASH_ENV ENV MAIL MAILPATH
do eval test x\${$as_var+set} = xset \
  && ( (unset $as_var) || exit 1) >/dev/null 2>&1 && unset $as_var || :
done
PS1='$ '
PS2='> '
PS4='+ '

# NLS nuisances.
LC_ALL=C
export LC_ALL
LANGUAGE=C
export LANGUAGE

# CDPATH.
(unset CDPATH) >/dev/null 2>&1 && unset CDPATH

if test "x$CONFIG_SHELL" = x; then
  as_bourne_compatible="if test -n \"\${ZSH_VERSION+set}\" && (emulate sh) >/dev/null 2>&1; then :
  emulate sh
  NULLCMD=:
  # Pre-4.2 versions of Zsh do word splitting on \${1+\"\$@\"}, which
  # is contrary to our usage.  Disable this feature.
  alias -g '\${1+\"\$@\"}'='\"\$@\"'
  setopt NO_GLOB_SUBST
else
  case \`(set -o) 2>/dev/null\` in #(
  *posix*) :
    set -o posix ;; #(
  *) :
     ;;
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
if ( set x; as_fn_ret_success y && test x = \"\$1\" ); then :

else
  exitcode=1; echo positional parameters were not saved.
fi
test x\$exitcode = x0 || exit 1
test -x / || exit 1"
  as_suggested=""
  if (eval "$as_required") 2>/dev/null; then :
  as_have_required=yes
else
  as_have_required=no
fi
  if test x$as_have_required = xyes && (eval "$as_suggested") 2>/dev/null; then :

else
  as_save_IFS=$IFS; IFS=$PATH_SEPARATOR
as_found=false
for as_dir in /bin$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR$PATH
do
  IFS=$as_save_IFS
  test -z "$as_dir" && as_dir=.
  as_found=:
  case $as_dir in #(
	 /*)
	   for as_base in sh bash ksh sh5; do
	     # Try only shells that exist, to save several forks.
	     as_shell=$as_dir/$as_base
	     if { test -f "$as_shell" || test -f "$as_shell.exe"; } &&
		    { $as_echo "$as_bourne_compatible""$as_required" | as_run=a "$as_shell"; } 2>/dev/null; then :
  CONFIG_SHELL=$as_shell as_have_required=yes
		   break 2
fi
	   done;;
       esac
  as_found=false
done
$as_found || { if { test -f "$SHELL" || test -f "$SHELL.exe"; } &&
	      { $as_echo "$as_bourne_compatible""$as_required" | as_run=a "$SHELL"; } 2>/dev/null; then :
  CONFIG_SHELL=$SHELL as_have_required=yes
fi; }
IFS=$as_save_IFS


      if test "x$CONFIG_SHELL" != x; then :
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
# out after a failed `exec'.
$as_echo "$0: could not re-execute with $CONFIG_SHELL" >&2
exit 255
fi

    if test x$as_have_required = xno; then :
  $as_echo "$0: This script requires a shell more modern than all"
  $as_echo "$0: the shells that I found on your system."
  if test x${ZSH_VERSION+set} = xset ; then
    $as_echo "$0: In particular, zsh $ZSH_VERSION has bugs and should"
    $as_echo "$0: be upgraded to zsh 4.3.4 or later."
  else
    $as_echo "$0: Please tell bug-autoconf@gnu.org about your system,
$0: including any error possibly output before this
$0: message. Then install a modern shell, or manually run
$0: the script under such a shell if you do have one."
  fi
  exit 1
fi
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


# $Id: bus-device-auto.sh,v 1.3 2009/08/29 17:52:04 fredette Exp $

# generic/bus-device-auto.sh - automatically generates C code for
# generic bus device support:

#
# Copyright (c) 2004 Matt Fredette
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
_TME_RCSID("\$Id: bus-device-auto.sh,v 1.3 2009/08/29 17:52:04 fredette Exp $");
EOF

if $header; then :; else
    cat <<EOF

/* this gives the number of entries that must be in a generic bus
   router array for a device with a bus size of 8 * (2 ^ siz_lg2)
   bits: */
#define TME_BUS_ROUTER_INIT_SIZE(siz_lg2)                       \\
  TME_BUS_ROUTER_INIT_INDEX(siz_lg2, (1 << (siz_lg2)) + 1, 0)

EOF
fi

# permute over initiator bus port width:
#
i_width=8
while test ${i_width} != 32; do
    i_width=`expr ${i_width} \* 2`

    # permute over initiator endianness:
    #
    for endian in b l; do
	if test ${endian} = b; then endian_what=big; else endian_what=little; fi

	# start the array:
	#
	$as_echo ""
	$as_echo "/* the ${i_width}-bit ${endian_what}-endian bus master bus router: */"
	what="const tme_bus_lane_t tme_bus_device_router_${i_width}e${endian}"
	if $header; then
	    $as_echo "extern ${what}[];"
	    continue
	fi
	$as_echo "${what}[TME_BUS_ROUTER_INIT_SIZE(TME_BUS${i_width}_LOG2)] = {"

	# permute over initiator maximum cycle size:
	#
	i_size=0
	while test `expr ${i_size} \< ${i_width}` = 1; do
	    i_size=`expr ${i_size} + 8`

	    # permute over initiator address offset:
	    #
	    i_offset=0
	    while test `expr ${i_offset} \< ${i_width}` = 1; do

		# calculate the initiator least and greatest lanes:
		#
		placeholder=false
		if test ${endian} = b; then
		    i_lane_greatest=`expr 0 - 8 + ${i_width} - ${i_offset}`
		    i_lane_least=`expr 8 + \( ${i_lane_greatest} \) - ${i_size}`
		    if test `expr \( ${i_lane_least} \) \< 0` = 1; then
			placeholder=true
		    fi
		else
		    i_lane_least=$i_offset
		    i_lane_greatest=`expr 0 - 8 + ${i_offset} + ${i_size}`
		    if test `expr \( ${i_lane_greatest} \) \>= ${i_width}` = 1; then
			placeholder=true
		    fi
		fi

		# permute over responder bus port width:
		#
		r_width=4
		while test `expr ${r_width} \< ${i_width}` = 1; do
		    r_width=`expr ${r_width} \* 2`

		    # permute over responder bus port least lane:
		    #
		    r_lane_least=0
		    while test `expr ${r_lane_least} \< ${i_width}` = 1; do
			r_lane_greatest=`expr 0 - 8 + ${r_lane_least} + ${r_width}`

			# emit the initiator information:
			#
			$as_echo ""
			$as_echo "  /* initiator maximum cycle size: ${i_size} bits"
			$as_echo "     initiator address offset: ${i_offset} bits"
			if $placeholder; then
			    $as_echo "     (a ${i_width}-bit initiator cannot request ${i_size} bits at an ${i_offset}-bit offset - this is an array placeholder)"
			fi

			# emit the responder information:
			#
			$as_echo "     responder bus port size: ${r_width} bits"
			$as_echo_n "     responder port least lane: D"`expr ${r_lane_least} + 7`"-D${r_lane_least}"

			# if the responder bus port greatest lane is
			# greater than the initiator bus port width,
			# part of the responder's port is outside of
			# the initiator's port:
			#
			if test `expr \( ${r_lane_greatest} \) \>= ${i_width}` = 1; then
			    $as_echo ""
			    $as_echo_n "     (responder port not correctly positioned for this initiator)"
			fi
			$as_echo ": */"

			# permute over the lanes:
			#
			lane=0
			if test ${endian} = b; then
			    route=`expr ${i_size} / 8`
			    route_increment=-1
			else
			    route=-1
			    route_increment=1
			fi
			while test `expr ${lane} \< ${i_width}` = 1; do
			    $as_echo_n "  /* D"`expr ${lane} + 7`"-D${lane} */	"

			    # see if this lane is on in the responder:
			    #
			    if test `expr ${lane} \>= ${r_lane_least}` = 1 \
			       && test `expr ${lane} \<= \( ${r_lane_greatest} \)` = 1; then
				r_lane_on=true
			    else
				r_lane_on=false
			    fi

			    # see if this lane is on in the initiator:
			    #
			    if test `expr ${lane} \>= \( ${i_lane_least} \)` = 1 \
			       && test `expr ${lane} \<= \( ${i_lane_greatest} \)` = 1; then
			        i_lane_on=true
				route=`expr \( ${route} + ${route_increment} \)`
			    else
				i_lane_on=false
			    fi

			    # if this is a placeholder entry:
			    #
			    if $placeholder; then
				$as_echo_n "TME_BUS_LANE_ABORT"

			    # otherwise, this is a real entry:
			    #
			    else
				if $i_lane_on; then
				    $as_echo_n "TME_BUS_LANE_ROUTE(${route})"
				    if $r_lane_on; then :; else
					$as_echo_n " | TME_BUS_LANE_WARN"
				    fi
				else
				    $as_echo_n "TME_BUS_LANE_UNDEF"
				fi
			    fi

			    $as_echo ","
			    lane=`expr ${lane} + 8`
			done

			r_lane_least=`expr ${r_lane_least} + 8`
		    done
		done

		i_offset=`expr ${i_offset} + 8`
	    done
	done

	# finish the array:
	#
	$as_echo "};"
    done

    # permute over read/write:
    #
    for name in read write; do
	capname=`$as_echo $name | tr a-z A-Z`
	if test $name = read; then
	    naming="reading"
	    from="from"
	    constbuffer=""
	else
	    naming="writing"
	    from="to"
	    constbuffer="const "
	fi

	$as_echo ""
	$as_echo "/* the ${i_width}-bit bus master DMA ${name} function: */"
	if $header; then
	    $as_echo "int tme_bus_device_dma_${name}_${i_width} _TME_P((struct tme_bus_device *,"
	    $as_echo "                                       tme_bus_addr_t,"
	    $as_echo "                                       tme_bus_addr_t,"
	    $as_echo "                                       ${constbuffer}tme_uint8_t *,"
	    $as_echo "                                       unsigned int));"
	    continue
	fi
	$as_echo "int"
	$as_echo "tme_bus_device_dma_${name}_${i_width}(struct tme_bus_device *bus_device,"
	$as_echo "                           tme_bus_addr_t address_init,"
	$as_echo "                           tme_bus_addr_t size,"
	$as_echo "                           ${constbuffer}tme_uint8_t *buffer,"
	$as_echo "                           unsigned int locks)"
	$as_echo "{"
	$as_echo "  struct tme_bus_tlb *tlb, tlb_local;"
	$as_echo "  struct tme_bus_connection *conn_bus;"
	$as_echo "  tme_bus_addr_t count_minus_one, count;"
	$as_echo "  struct tme_bus_cycle cycle;"
	$as_echo "  tme_bus_addr_t address_resp;"
	$as_echo "  int shift;"
	$as_echo "  int err;"
	$as_echo ""
	$as_echo "  /* assume no error: */"
	$as_echo "  err = TME_OK;"
	$as_echo ""
	$as_echo "  /* loop while we have more bytes to ${name}: */"
	$as_echo "  for (; err == TME_OK && size > 0; ) {"
	$as_echo ""
	$as_echo "    /* hash this address into a TLB entry: */"
	$as_echo "    tlb = (*bus_device->tme_bus_device_tlb_hash)"
	$as_echo "            (bus_device,"
	$as_echo "             address_init,"
	$as_echo "             TME_BUS_CYCLE_${capname});"
	$as_echo ""
	$as_echo "    /* busy this TLB entry: */"
	$as_echo "    tme_bus_tlb_busy(tlb);"
	$as_echo ""
	$as_echo "    /* if this TLB entry is invalid, doesn't cover this address, or if it doesn't"
	$as_echo "       allow ${naming}, reload it: */"
	$as_echo "    if (tme_bus_tlb_is_invalid(tlb)"
	$as_echo "        || address_init < tlb->tme_bus_tlb_addr_first"
	$as_echo "        || address_init > tlb->tme_bus_tlb_addr_last"
	$as_echo "        || (tlb->tme_bus_tlb_emulator_off_${name} == TME_EMULATOR_OFF_UNDEF"
	$as_echo "            && !(tlb->tme_bus_tlb_cycles_ok & TME_BUS_CYCLE_${capname}))) {"
	$as_echo ""
	$as_echo "      /* unbusy this TLB entry for filling: */"
	$as_echo "      tme_bus_tlb_unbusy_fill(tlb);"
	$as_echo ""
	$as_echo "      /* pass this TLB's token: */"
	$as_echo "      tlb_local.tme_bus_tlb_token = tlb->tme_bus_tlb_token;"
	$as_echo ""
	$as_echo "      /* get our bus connection: */"
	$as_echo "      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,"
	$as_echo "                                                bus_device->tme_bus_device_connection,"
	$as_echo "                                                &bus_device->tme_bus_device_connection_rwlock);"
	$as_echo ""
	$as_echo "      /* unlock the device: */"
	$as_echo "      (*bus_device->tme_bus_device_unlock)(bus_device, locks);"
	$as_echo ""
	$as_echo "      /* reload the TLB entry: */"
	$as_echo "      err = (*conn_bus->tme_bus_tlb_fill)"
	$as_echo "              (conn_bus,"
	$as_echo "               &tlb_local,"
	$as_echo "               address_init,"
	$as_echo "               TME_BUS_CYCLE_${capname});"
	$as_echo ""
	$as_echo "      /* lock the device: */"
	$as_echo "      (*bus_device->tme_bus_device_lock)(bus_device, locks);"
	$as_echo ""
	$as_echo "      /* return if we couldn't fill the TLB entry: */"
	$as_echo "      if (err != TME_OK) {"
	$as_echo "        return (err);"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* store the TLB entry: */"
	$as_echo "      *tlb = tlb_local;"
	$as_echo ""
	$as_echo "      /* loop to check the newly filled TLB entry: */"
	$as_echo "      continue;"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* if this TLB entry allows fast ${naming}: */"
	$as_echo "    if (tlb->tme_bus_tlb_emulator_off_${name} != TME_EMULATOR_OFF_UNDEF) {"
	$as_echo ""
	$as_echo "      /* see how many bytes we can fast ${name} ${from} this TLB entry,"
	$as_echo "         starting at this address: */"
	$as_echo "      count_minus_one = (tlb->tme_bus_tlb_addr_last - address_init);"
	$as_echo ""
	$as_echo "      /* ${name} that many bytes or size bytes, whichever is smaller: */"
	$as_echo "      count_minus_one = TME_MIN(count_minus_one,"
	$as_echo "                                (size - 1));"
	$as_echo "      count = count_minus_one + 1;"
	$as_echo "      assert (count != 0);"
	$as_echo ""
	$as_echo "      /* do the bus ${name}: */"
	$as_echo "      tme_memory_bus_${name}_buffer((tlb->tme_bus_tlb_emulator_off_${name} + address_init), buffer, count, tlb->tme_bus_tlb_rwlock, sizeof(tme_uint8_t), sizeof(tme_uint${i_width}_t));"
	$as_echo ""
	$as_echo "      /* unbusy this TLB entry: */"
	$as_echo "      tme_bus_tlb_unbusy(tlb);"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* otherwise, we have to do a slow ${name}: */"
	$as_echo "    else {"
	$as_echo ""
	$as_echo "      /* get the size of this bus cycle: */"
	$as_echo "      count = (1 << TME_BUS${i_width}_LOG2);"
	$as_echo "      count -= (address_init & (count - 1));"
	$as_echo "      count = TME_MIN(count, size);"
	$as_echo ""
	$as_echo "      /* fill the cycle structure: */"
	$as_echo "      cycle.tme_bus_cycle_type = TME_BUS_CYCLE_${capname};"
	$as_echo "      cycle.tme_bus_cycle_size = count;"
	$as_echo "      cycle.tme_bus_cycle_buffer = (tme_uint8_t *) buffer; /* XXX this breaks const */"
	$as_echo "      cycle.tme_bus_cycle_buffer_increment = 1;"
	$as_echo "      cycle.tme_bus_cycle_lane_routing"
	$as_echo "        = (bus_device->tme_bus_device_router"
	$as_echo "           + TME_BUS_ROUTER_INIT_INDEX(TME_BUS${i_width}_LOG2, count, address_init));"
	$as_echo ""
	$as_echo "      /* XXX this should come from a socket configuration: */"
	$as_echo "      cycle.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS${i_width}_LOG2);"
	$as_echo ""
	$as_echo "      /* form the physical address for the bus cycle handler: */"
	$as_echo "      address_resp = tlb->tme_bus_tlb_addr_offset + address_init;"
	$as_echo "      shift = tlb->tme_bus_tlb_addr_shift;"
	$as_echo "      if (shift < 0) {"
	$as_echo "        address_resp <<= (0 - shift);"
	$as_echo "      }"
	$as_echo "      else if (shift > 0) {"
	$as_echo "        address_resp >>= shift;"
	$as_echo "      }"
	$as_echo "      cycle.tme_bus_cycle_address = address_resp;"
	$as_echo ""
	$as_echo "      /* unbusy this TLB entry: */"
	$as_echo "      tme_bus_tlb_unbusy(tlb);"
	$as_echo ""
	$as_echo "      /* unlock the device: */"
	$as_echo "      (*bus_device->tme_bus_device_unlock)(bus_device, locks);"
	$as_echo ""
	$as_echo "      /* run the bus cycle: */"
	$as_echo "      err = (*tlb->tme_bus_tlb_cycle)"
	$as_echo "           (tlb->tme_bus_tlb_cycle_private, &cycle);"
	$as_echo ""
	$as_echo "      /* if the TLB entry was invalidated before the ${name}: */"
	$as_echo "      if (err == EBADF"
	$as_echo "          && tme_bus_tlb_is_invalid(tlb)) {"
	$as_echo "        count = 0;"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* otherwise, any other error might be a bus error: */"
	$as_echo "      else if (err != TME_OK) {"
	$as_echo "        err = tme_bus_tlb_fault(tlb, &cycle, err);"
	$as_echo "        assert (err != TME_OK);"
	$as_echo "      }"
	$as_echo ""
	$as_echo "      /* lock the device: */"
	$as_echo "      (*bus_device->tme_bus_device_lock)(bus_device, locks);"
	$as_echo "    }"
	$as_echo ""
	$as_echo "    /* update the address, buffer, and size and continue: */"
	$as_echo "    address_init += count;"
	$as_echo "    buffer += count;"
	$as_echo "    size -= count;"
	$as_echo "  }"
	$as_echo ""
	$as_echo "  return (err);"
	$as_echo "}"
    done

done

# done:
#
exit 0
