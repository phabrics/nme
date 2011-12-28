#! /usr/local/bin/perl -w

# $Id: m68k-opmap-make.pl,v 1.10 2007/08/25 21:16:05 fredette Exp $

# m68k-opmap-make.pl - compiles the complete decoding of all legal
# first-instruction-word values into the opcode map used by the C
# decoder:

#
# Copyright (c) 2002, 2003, 2005 Matt Fredette
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

# globals:
$0 =~ /^(.*\/)?([^\/]+)$/; $PROG = $2;
$debug = 0;

# to silence -w:
undef($value);
$debug = $debug;

# emit our header:
print <<"EOF;";
/* generated automatically by $PROG, do not edit! */

/* includes: */
#include "m68k-impl.h"
EOF;

# we start with no previous CPU and no root inits:
#
undef ($cpu_name_previous);
$root_init_i_next = 0;

# opcode immediates:
#
%op_imm = ('0', 'ZERO',
	   '1', 'ONE',
	   '2', 'TWO',
	   '3', 'THREE',
	   '4', 'FOUR',
	   '5', 'FIVE',
	   '6', 'SIX',
	   '7', 'SEVEN',
	   '8', 'EIGHT',
	   );

# the instruction ordering hints:
#
@insn_i_to_func = split(/[\r\n\s]+/, <<'EOF;');
EOF;
for($insn_i = 0; $insn_i < @insn_i_to_func; $insn_i++) {
    $func = $insn_i_to_func[$insn_i];
    $func_to_insn_i{$func} = $insn_i;
}

# loop over standard input:
for ($line = 1; defined($_ = <STDIN>); $line++) {
    chomp;

    # break the line into tokens:
    @tokens = split(' ', $_);

    # if this is the beginning of a new CPU:
    if ($tokens[0] eq "cpu-begin") {
	$cpu_name = $tokens[1];

	# initialize for this CPU:
	print STDERR "$PROG: initializing for $cpu_name...";

	# initialize the full map:
	undef(@map_line);
	for ($pattern = 65536; $pattern-- > 0;) {
	    $map_op0[$pattern] = "U";
	    $map_op1[$pattern] = "U";
	    $map_eax_size[$pattern] = "U";
	    $map_eax_cycles[$pattern] = "U";
	    $map_imm_operand[$pattern] = "U";
	    $map_imm_size[$pattern] = "U";
	    $map_eay_size[$pattern] = "U";
	    $map_eay_cycles[$pattern] = "U";
	}

	# initialize the special operations:
	undef(%specop);

	# we're done initializing and we're now reading patterns:
	$patterns = 0;
	print STDERR " done\n$PROG: reading $cpu_name patterns...";
    }

    # if this is a special-operation line:
    elsif ($tokens[0] eq "specop") {
	shift(@tokens);
	$specop = shift(@tokens);
	foreach (@tokens) {
	    $specop{$_} = $specop;
	}
    }

    # if this is a pattern:
    elsif ($tokens[0] =~ /^[01]/) {
    
	# the first token is the pattern.  die if this pattern has already
	# appeared:
	$pattern = oct("0b".shift(@tokens));
	if (defined($map_line[$pattern])) {
	    $func = $map_func[$pattern];

	    # to allow for an earlier, more specific iset pattern
	    # while still using very general iset patterns later,
	    # simply add to the function name on the earlier, more
	    # specific iset pattern, the number of asterixes for the
	    # number of later general iset pattern expansions that
	    # will collide with it.  these collisions will be ignored:
	    #
	    if ($func =~ /^(.*)\*$/) {
		$map_func[$pattern] = $1;
		next;
	    }
	    die "stdin:$line: duplicate pattern of line $map_line[$pattern]\n";
	}
	$map_line[$pattern] = $line;

	# fill this map entry:
	foreach $token (@tokens) {
	    ($what, $value) = split(/=/, $token, 2);
	    eval("\$map_".$what."[$pattern] = \$value;");
	}
	die "stdin:$line: no function given\n"
	    if (!defined($map_func[$pattern]));
	$patterns++;
    }

    # if this is the end of a CPU:
    elsif ($tokens[0] eq "cpu-end") {
	$cpu_name = $tokens[1];

	# note how many patterns we read:
	print STDERR " read $patterns $cpu_name patterns\n";
    
	# sanity-check the information read in, and force all unused
	# full map entries to illegal:
	print STDERR "$PROG: finding unused $cpu_name patterns...";
	$unused = 0;
	for ($pattern = 65536; $pattern-- > 0;) {

	    # if this is an unused map entry:
	    if (!defined($map_line[$pattern])) {
		$map_func[$pattern] = "illegal";
		$unused++;
		next;
	    }

	    # since the overwhelming majority of instructions use at
	    # most one effective address path (the eax path), only the
	    # size of the eax path operand is stored in the opcode
	    # maps.  any instruction that uses the eay path must do so
	    # with the same size as the eax path:
	    if ($map_eay_size[$pattern] ne "U") {

		# if this instruction isn't using the eax path, switch
		# to using the eax path instead.  the only instruction
		# allowed to do this is move.S.  temporarily rewrite
		# this function to a special function "movenonmemtomem":
		if ($map_eax_size[$pattern] eq "U") {
		    die "$PROG: pattern ".sprintf("%b", $pattern)." ($map_func[$pattern]) uses the eay path\n"
			if ($map_func[$pattern] !~ /^(move)(\d+)$/
			    && $map_func[$pattern] !~ /^(move_srp[id])(\d+)$/);
		    $map_func[$pattern] = $1."nonmemtomem".$2;
		    $map_eax_size[$pattern] = $map_eay_size[$pattern];
		    $map_eax_cycles[$pattern] = $map_eay_cycles[$pattern];
		    $map_op0[$pattern] =~ s/^memy/memx/;
		    $map_op1[$pattern] =~ s/^memy/memx/;
		}

		# otherwise, this instruction is using both ea paths.
		# the only instruction allowed to do this is move.S.
		# temporarily rewrite this function to a special
		# function "movememtomem":
		else {
		    die "$PROG: pattern ".sprintf("%b", $pattern)." ($map_func[$pattern]) uses both ea paths\n"
			if ($map_func[$pattern] !~ /^(move)(\d+)$/);
		    $map_func[$pattern] = $1."memtomem".$2;
		    die "$PROG: pattern ".sprintf("%b", $pattern)." ($map_func[$pattern]) uses both ea paths at different sizes\n"
			if ($map_eay_size[$pattern] ne $map_eax_size[$pattern]);
		    die "$PROG: pattern ".sprintf("%b", $pattern)." ($map_func[$pattern]) doesn't use eay write-only\n"
			if ($map_eay_cycles[$pattern] ne "wo");
		}
	    }
	}
	print STDERR " found $unused unused $cpu_name patterns\n";

	# start the opcode map initialization for this CPU.  if there
	# is a previous CPU, call its initialization function first:
	#
	$opcode_map_init = '';
	if (defined($cpu_name_previous)) {
	    $opcode_map_init .= "\n  tme_m68k_opcodes_init_${cpu_name_previous}(opcodes);\n";
	}
	$opcode_map_init .= "\n";

	# loop over the root patterns:
	#
	%param_to_local = ();
	$local_next = 0;
	undef (@root_init_calls);
	$root_group_i_next = 0;
	for ($root = 0; $root < 1024; $root++) {

	    # loop over the patterns under this root:
	    #
	    %param_to_submask = ();
	    print STDERR "root $root\n" if ($debug);
	    for ($sub = 0, $pattern = $root << 6; $sub < 64 ; $sub++, $pattern++) {

		# start the opcode parameters:
		#
		$line = $map_line[$pattern];
		@params = ();

		# the opcode function.  NB that a memory-to-memory move
		# generates the Y effective address after generating the
		# normal EA, and that a nonmemory-to-memory move generates
		# only the Y EA:
		#
		$func = $map_func[$pattern];
		if ($func =~ /^movememtomem(\d+)$/) {
		    $func = "move${1}";
		    push (@params,
			  'TME_M68K_OPCODE_EA_Y',
			  'TME_M68K_OPCODE_SPECOP');
		}
		elsif ($func =~ /^(move.*)nonmemtomem(\d+)$/) {
		    $func = "${1}${2}";
		    push (@params,
			  'TME_M68K_OPCODE_EA_Y');
		}
		$insn_i = $func_to_insn_i{$func};
		if (!defined($insn_i)) {
		    $insn_i = $func_to_insn_i{$func} = (@insn_i_to_func + 0);
		    push (@insn_i_to_func, $func);
		}
		unshift(@params,
			"TME_M68K_OPCODE_INSN($insn_i)");

		# the two operands:
		#
		undef ($eax_size);
		undef ($cycles);
		for ($op_i = 0; $op_i < 2; $op_i++) {
		    eval("\$op = \$map_op${op_i}[\$pattern];");

		    # if this operand is a register:
		    #
		    if ($op =~ /^\%([ad][0-7])\.(\d+)/) {
			($op, $op_size) = ($1, $2);
			$op =~ tr/a-z/A-Z/;
			if ($op_size == 16) {
			    $op .= " << 1";
			}
			elsif ($op_size == 8) {
			    $op .= " << 2";
			}
			$op = "tme_m68k_ireg_uint${op_size}(TME_M68K_IREG_${op})";
		    }

		    # if this operand is the effective address:
		    #
		    elsif ($op eq "eax.32") {
			$op = "_tme_m68k_ea_address";
			$eax_size = $map_eax_size[$pattern];
			$cycles = $map_eax_cycles[$pattern];
			if ($eax_size ne 'UNSIZED') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has a sized control EA\n";
			}
			if ($cycles ne 'U'
			    && $cycles ne 'un') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has cycles ($cycles) on a control EA\n";
			}
		    }

		    # if this operand is an opcode immediate:
		    #
		    elsif ($op =~ /^imm(\d+)\.(\d+)/) {
			($op, $op_size) = ($1, $2);
			$op = $op_imm{$op};
			if ($op_size == 16) {
			    $op .= " << 1";
			}
			elsif ($op_size == 8) {
			    $op .= " << 2";
			}
			$op = "tme_m68k_ireg_uint${op_size}(TME_M68K_IREG_${op})";
		    }

		    # if this operand is a memory buffer:
		    #
		    elsif ($op =~ /^mem([xy])\.(\d+)/) {
			$op = "tme_m68k_ireg_mem${1}${2}";
			$eax_size = $map_eax_size[$pattern];
			$cycles = $map_eax_cycles[$pattern];
			if ($eax_size eq 'UNSIZED') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has an unsized EA\n";
			}
			if ($cycles eq 'U'
			    || $cycles eq 'un') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has no cycles on an EA\n";
			}
		    }

		    elsif ($op ne 'U') {
			die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) op${op_i} unknown: $op\n";
		    }

		    # if this operand is an immediate:
		    #
		    if ($map_imm_operand[$pattern] eq $op_i) {
			if ($op ne 'U') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) op${op_i} is already $op\n";
			}
			$op_size = $map_imm_size[$pattern];
			$op = 'TME_M68K_IREG_IMM32';
			$imm_size = '16';
			if ($op_size eq '8') {
			    $op .= " << 2";
			}
			elsif ($op_size eq '16') {
			    $op .= " << 1";
			}
			elsif ($op_size eq '16S32') {
			    $op_size = '32';
			}
			elsif ($op_size eq '32') {
			    $imm_size = '32';
			}
			else {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has bad immediate size $op_size\n";
			}
			$op = "tme_m68k_ireg_uint${op_size}(${op})";
			push (@params, 
			      'TME_M68K_OPCODE_IMM_'.$imm_size);
		    }

		    # if this operand is defined:
		    #
		    if ($op ne 'U') {
			push (@params, 
			      'TME_M68K_OPCODE_OP'
			      .$op_i
			      .'('.$op.')');
		    }
		}

		# any EA operand:
		#
		if (defined($eax_size)
		    && $eax_size ne 'U') {
		    if ($eax_size eq 'UNSIZED') {
			if ($cycles ne 'un'
			    && $cycles ne 'U') {
			    die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has an unsized EA with $cycles cycles\n";
			}
			push (@params, "TME_M68K_OPCODE_EA_UNSIZED");
		    }
		    elsif ($eax_size eq '8'
			   || $eax_size eq '16'
			   || $eax_size eq '32') {
			push (@params, "TME_M68K_OPCODE_EA_SIZE(TME_M68K_SIZE_${eax_size})");
		    }
		    else {
			die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has an $eax_size sized EA\n";
		    }
		    if ($cycles eq 'U'
			|| $cycles eq 'un') {
			# nothing to do
			#
		    }
		    elsif ($cycles eq 'ro') {
			push (@params, 
			      'TME_M68K_OPCODE_EA_READ');
		    }
		    elsif ($cycles eq 'rw') {
			push (@params, 
			      'TME_M68K_OPCODE_EA_READ',
			      'TME_M68K_OPCODE_EA_WRITE');
		    }
		    elsif ($cycles eq 'wo') {
			push (@params, 
			      'TME_M68K_OPCODE_EA_WRITE');
		    }
		    else {
			die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has bad EA cycles\n";
		    }
		}

		# any opcode specop:
		#
		$specop = $specop{$func};
		if (defined($specop)) {
		    if ($specop ne 'specop16'
			&& $specop ne 'fpgen') {
			die "$PROG: pattern ".sprintf("%016b", $pattern)." ($map_func[$pattern]) has a bad specop\n";
		    }
		    push (@params,
			  "TME_M68K_OPCODE_SPECOP");
		}
		
		# each param has an associated submask - a 64-bit mask
		# with a set bit for each sub in this root that needs
		# that param.  for each param used by this pattern,
		# set this sub's bit in the param's submask:
		#
		foreach $param (@params) {
		    $_ = $param_to_submask{$param};
		    if (!defined($_)) {
			$_ = '0:0';
		    }
		    ($submask_hi, $submask_lo) = split(/:/, $_);
		    if ($sub > 31) {
			$submask_hi |= (1 << ($sub & 31));
		    }
		    else {
			$submask_lo |= (1 << $sub);
		    }
		    $param_to_submask{$param} = "${submask_hi}:${submask_lo}";
		}
	    }

	    # within a root, params usually share submasks.  gather
	    # the sets of params that share submasks into a hash
	    # indexed by submask, and then sort those keys to get a
	    # list of the submasks for the params passed to a root
	    # initialization function:
	    #
	    undef(%submasks);
	    foreach $param (sort(keys(%param_to_submask))) {
		$submask = $param_to_submask{$param};
		$submasks{$submask} .= "${param};";
	    }
	    @submasks = (sort(keys(%submasks)));

	    # if this root hasn't changed since the previous CPU,
	    # don't bother emitting anything for it - the previous
	    # CPU's initialization will take care of it:
	    #
	    $root_key = '';
	    foreach $submask (@submasks) {
		$root_key .= "${submask} $submasks{$submask} %";
	    }
	    $_ = $root_key_previous[$root];
	    if (defined($_)
		&& $_ eq $root_key) {
		next;
	    }
	    $root_key_previous[$root] = $root_key;

	    # if a root initialization function doesn't already exist
	    # for this submasks list:
	    #
	    $submasks = join(' ', @submasks);
	    $root_init_i = $submasks_to_root_init_i{$submasks};
	    if (!defined($root_init_i)) {

		# create a new root initialization function:
		#
		$root_init_i = $submasks_to_root_init_i{$submasks} = $root_init_i_next++;
		print "\n/* root init ${root_init_i}: */\n";
		print "static void\n_tme_m68k_opcode_root_init_${root_init_i}(tme_uint32_t *root, const tme_uint32_t *params)\n{\n";

		# loop over the subs:
		#
		for ($sub = 0; $sub < 64; $sub++) {

		    # make a list of params for this sub:
		    #
		    @params = ();
		    for ($param_i = 0; $param_i < @submasks; $param_i++) {
			($submask_hi, $submask_lo) = split(/:/, $submasks[$param_i]);
			if ((($sub > 31)
			     ? $submask_hi
			     : $submask_lo)
			    & (1 << ($sub & 31))) {
			    push (@params, "params[${param_i}]");
			}
		    }

		    # add in this sub's parameters:
		    #
		    if (@params > 0) {
			print "  root[${sub}] = ".join(' | ', @params).";\n";
		    }
		}
		print "}\n";
	    }
	    print STDERR "root init ${root_init_i} submasks are $submasks\n" if ($debug);
 
	    # loop over the parameters for the root initialization function:
	    #
	    @root_init_params = ();
	    for ($param_i = 0; $param_i < @submasks; $param_i++) {

		# get the params that this root needs to provide for
		# this root initialization parameter, and store certain
		# params in constant locals, to try to make compilation
		# easier:
		#
		@params = split(/;/, $submasks{$submasks[$param_i]});
		foreach $param (@params) {
		    if ($param =~ /^TME_M68K_OPCODE_OP\d/) {
			$local = $param_to_local{$param};
			if (!defined($local)) {
			    $local = $param_to_local{$param} = $local_next++;
			    $opcode_map_init = "  const tme_uint32_t param${local} = $param;\n".$opcode_map_init;
			}
			$param = "param${local}";
		    }
		}

		push (@root_init_params, join(' | ', @params));
	    }

	    # assume that the best place to initialize this root is at
	    # the end of all current root initializations:
	    #
	    $root_init_call_best = $#root_init_calls;
	    $root_init_call_best_score = 0;

	    # loop over all current root initializations, tracking the
	    # values that each leaves in params[], and finding the
	    # current root initialization that leaves params[] closest
	    # to what this root initialization needs:
	    #
	    undef (@params_state);
	    for ($root_init_call = 0; 
		 $root_init_call < @root_init_calls;
		 $root_init_call++) {

		# get this current root initialization, and update the params[] state:
		#
		($junk, $junk, @root_init_params_other) = split(/\#/, $root_init_calls[$root_init_call]);
		splice(@params_state, 0, @root_init_params_other + 0, @root_init_params_other);

		# score our closeness to this current root initialization's params[]:
		#
		$root_init_call_score = 0;
		for ($param_i = 0; 
		     $param_i < @params_state && $param_i < @root_init_params;
		     $param_i++) {
		    if ($params_state[$param_i] eq $root_init_params[$param_i]) {
			$root_init_call_score++;
		    }
		}

		# update the closest current root initialization:
		#
		if ($root_init_call_best_score <= $root_init_call_score) {
		    $root_init_call_best_score = $root_init_call_score;
		    $root_init_call_best = $root_init_call;
		}
	    }
	    
	    # add this root initialization to the list:
	    #
	    splice(@root_init_calls, $root_init_call_best + 1, 0,
		   join("\#", $root, $root_init_i, @root_init_params));
	}

	# make the root initialization function calls:
	#
	undef (@params_state);
	@root_init_group = ();
	$root_init_i_group = -1;
	for ($root_init_call = 0; 
	     $root_init_call < @root_init_calls;
	     $root_init_call++) {

	    # get this next root:
	    #
	    ($root, $root_init_i, @root_init_params) = split(/\#/, $root_init_calls[$root_init_call]);

	    # if this root uses a different init function than the
	    # current group, flush this group and start a new one:
	    #
	    if ($root_init_i != $root_init_i_group) {
		&root_init_group_flush();
	    }

	    # loop over this root's params:
	    #
	    for ($param_i = 0; $param_i < @root_init_params; $param_i++) {

		# if this param doesn't already have the right value:
		#
		if (!defined($params_state[$param_i])
		    || $params_state[$param_i] ne $root_init_params[$param_i]) {

		    # flush the current group:
		    #
		    &root_init_group_flush();

		    # set the new param value:
		    #
		    $opcode_map_init .= "  params[${param_i}] = ".$root_init_params[$param_i].";\n";
		    $params_state[$param_i] = $root_init_params[$param_i];
		}
	    }

	    # add this root to the current group:
	    #
	    push (@root_init_group, $root);
	}

	# flush the last group:
	#
	&root_init_group_flush();

	# define the opcode map:
	#
	print "\n";
	print "/* the ${cpu_name} opcode map: */\n";
	print "tme_uint32_t tme_m68k_opcodes_${cpu_name}[65536];\n";
	
	# define the opcode map initialization function:
	#
	print "\n";
	print "/* the ${cpu_name} opcode map initialization: */\n";
	print "void\ntme_m68k_opcodes_init_${cpu_name}(tme_uint32_t *opcodes)\n{\n";
	print "  tme_uint32_t params[64];\n";
	print $opcode_map_init;
	print "}\n";

	$cpu_name_previous = $cpu_name;
    }

    # anything else is an error:
    else {
	print STDERR "stdin:$line $PROG error: don't know how to handle: ".join(" ", @tokens)."\n";
	exit(1);
    }
}

print STDERR "$PROG: $root_init_i_next total root inits\n";

# emit the insn array:
#
print "\n";
print "/* the insn array: */\n";
print "const _tme_m68k_insn tme_m68k_opcode_insns[] = {\n";
print "  tme_m68k_".join(",\n  tme_m68k_", @insn_i_to_func)."\n";
print "};\n\n";

# done:
exit(0);

# this flushes the current group of root inits:
#
sub root_init_group_flush {
    my ($root);
    my ($root_group_i);
    
    # if there is only one root in this group:
    #
    if (@root_init_group == 1) {
	$root = $root_init_group[0];
	
	$opcode_map_init .= "\n  /* root $root: */\n";
	$opcode_map_init .= "  _tme_m68k_opcode_root_init_${root_init_i_group}(opcodes + ($root * 64), params);\n\n";
    }

    # if there are multiple roots in this group:
    #
    elsif (@root_init_group > 1) {
	
	# emit the group:
	#
	$root_group_i = $root_group_i_next++;
	if ($root_group_i == 0) {
	    $opcode_map_init = "  tme_uint16_t root_i;\n".$opcode_map_init;
	}
	$opcode_map_init = '  const tme_uint16_t root_group'.$root_group_i.'[] = {'.join(', ', @root_init_group)."};\n".$opcode_map_init;

	# emit the group root init call:
	#
	$opcode_map_init .= "\n  /* roots ".join(', ', @root_init_group).": */\n";
	$opcode_map_init .= "  for (root_i = 0; root_i < ".(@root_init_group + 0)."; root_i++) {\n";
	$opcode_map_init .= "    _tme_m68k_opcode_root_init_${root_init_i_group}(opcodes + (root_group".$root_group_i."[root_i] * 64), params);\n";
	$opcode_map_init .= "  }\n\n";
    }

    # initialize for the next group:
    #
    @root_init_group = ();
    $root_init_i_group = $root_init_i;
}
