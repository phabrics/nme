#! /usr/local/bin/perl -w

# $Id: m68k-iset-expand.pl,v 1.5 2007/08/25 21:14:29 fredette Exp $

# m68k-inst-expand.pl - expands the m68k instruction templates into
# full instruction word patterns and their decodings:

#
# Copyright (c) 2002, 2003 Matt Fredette
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
$debug = 0;
@threefield = ("000", "001", "010", "011", "100", "101", "110", "111");

# our single command line argument is the CPU name:
if (@ARGV != 1) {
    print STDERR "usage: $0 CPU-NAME\n";
    exit(1);
}
$cpu_name = shift(@ARGV);
print "cpu-begin $cpu_name\n";

# to silence perl -w:
undef($field_q);

# loop over the instruction set on standard input:
$pass = 1;
for($line = 1; defined($_ = <STDIN>); $line++) {
    chomp;

    # split by spaces:
    @tokens = split(/[,\s]+/, $_);

    # strip comments:
    for ($token_i = 0; $token_i < @tokens; $token_i++) {
	splice(@tokens, $token_i)
	    if ($tokens[$token_i] =~ /^\!/);
    }

    # skip blank lines:
    next if (@tokens == 0);

    # handle the preprocessor.  this is the dumbest preprocessor ever:
    if ($tokens[0] eq ".if") {
	$pass = 0;
	foreach $token (@tokens) {
	    $pass = 1 if ($token eq $cpu_name);
	}
	next;
    }
    elsif ($tokens[0] eq ".endif") {
	$pass = 1;
	next;
    }
    next if (!$pass);

    # handle an EAX category.  an EAX category defines valid
    # combinations of an EA "mode" and "reg" values found in an
    # instruction patterns "x" field.  see linear pp 60 of M68000PM.ps
    # for a list of all possible mode and reg combinations.
    #
    # different combinations are valid depending on the instruction
    # and operand being described, however most instructions use one
    # of a small handful of common combinations.
    # 
    # an EA category is defined as:
    #
    # eax-cat NAME DEFAULT-CYCLES COMBINATIONS
    #
    # NAME is a user-assigned name for the category
    #
    # DEFAULT-CYCLES specifies the default memory cycle behavior
    # for this category of effective address.  specific later uses
    # of this category may still override this default.  this is
    # one of "un", "ro", "wo", or "rw", for no cycles, read-only,
    # write-only, and read-write, respectively
    #
    # COMBINATIONS is a string of "y" and "n" characters indicating
    # which mode and reg combinations are valid.  positions zero
    # through six in the string correspond to mode values zero 
    # through six, and positions seven through 14 correspond to
    # mode value 7, reg values zero through seven:
    #
    if ($tokens[0] eq "eax-cat") {

	# get the category definition:
	$eax_cat_name = $tokens[1];
	$eax_cat_cycles_default = $tokens[2];
	@eax_cat_modes = split(//, $tokens[3]);

	# expand this category once for each possible operand size,
	# remembering what operand each valid combination of mode and
	# reg means:
        foreach $eax_size (0, 8, 16, 32) {
	    @eax_cat_fields = ();
	    @eay_cat_fields = ();
	    for ($mode = 0; $mode < @threefield; $mode++) {
		for ($reg = 0; $reg < @threefield; $reg++) {
		
		    # EXCEPTION: 8-bit address register direct is always
		    # illegal:
		    next if ($eax_size == 8 && $mode == 1);
		    
		    if ($eax_cat_modes[$mode + ($mode < 7 ? 0 : $reg)] eq "y") {
			undef($field_x_value);
			undef($field_y_value);
			if ($mode == 0) {
			    # data register direct:
			    $field_x_value = "\%d${reg}.${eax_size}";
			}
			elsif ($mode == 1) {
			    # address register direct:
			    $field_x_value = "\%a${reg}.${eax_size}";
			}
			elsif ($mode == 7
			       && $reg == 4) {
			    # immediate:
			    $field_x_value = "imm.${eax_size}";
			}
			elsif ($eax_size == 0) {
			    # unsized, effective address complex:
			    $field_x_value = "eax.32";
			}
			else {
			    $field_x_value = "memx.${eax_size}.$eax_cat_cycles_default";
			    $field_y_value = "memy.${eax_size}.$eax_cat_cycles_default";
			}
			$field_y_value = $field_x_value if (!defined($field_y_value));
			push(@eax_cat_fields, "$threefield[$mode]$threefield[$reg]/$field_x_value");
			push(@eay_cat_fields, "$threefield[$reg]$threefield[$mode]/$field_y_value");
		    }
		}
	    }
	    eval("\@eax_${eax_cat_name}${eax_size} = \@eax_cat_fields;");
	    eval("\@eay_${eax_cat_name}${eax_size} = \@eay_cat_fields;");
	}
	next;
    }

    # handle a "specop" line.  some instructions have a separate opcode
    # word that needs to be read and decoded specially.  we expand
    # any .s suffixes on the instruction names but otherwise pass
    # this line on untouched:
    #
    if ($tokens[0] eq "specop") {
	foreach $token (@tokens) {
	    $token = "${1}8 ${1}16 ${1}32" if ($token =~ /^(\S+)\.s$/);
	}
	print join(" ", @tokens)."\n";
	next;
    }

    # form the instruction template from the first four tokens:
    $template = join("", splice(@tokens, 0, 4));

    # the next token is the function:
    $func = shift(@tokens);

    # the remaining tokens are the arguments:
    @args = @tokens;
    undef(@tokens);

    print "template $template, func $func, args: ".join(" ", @args)."\n" if ($debug);

    # break this template down into fields and sanity-check them.
    # we work from the leftmost position in the template (the most
    # significant bit, bit 15) to the rightmost position.
    #
    @fields = ("root");
    for ($field_start = 15; $field_start >= 0; $field_start = $field_end - 1) {

	# find the extent of this field.  a wildcard field (indicated
	# by the "?" character, which is internally rewritten to the "w"
	# character) is always a single bit wide, otherwise a constant
	# field ends right before a non-constant bit, and any other field
	# ends when it ends:
	#
	$field_char = substr($template, 15 - $field_start, 1);
	$field_char =~ tr/\?/w/;
	for ($field_end = $field_start; $field_end >= 0; $field_end--) {

	    # get the next field character:
	    $field_char_next = substr($template, 15 - ($field_end - 1), 1);
	    $field_char_next =~ tr/\?/w/;

	    # stop if this is the first character of the next field:
	    last if ($field_char eq "w"
		     || ($field_char =~ /[01]/
			 ? $field_char_next !~ /[01]/
			 : $field_char_next ne $field_char));
	}

	# save this field:
	push (@fields, ($field_char =~ /[01]/
			? substr($template, 15 - $field_start, $field_start - $field_end + 1)
			: $field_char));

	# sanity check this field.  some fields must be a fixed size,
	# some fields must be used by some argument:
	#
	$field_size = $field_start - $field_end + 1;
	print "$#fields: $field_char-field from $field_start to $field_end" if ($debug);
	if ($field_char =~ /[01w]/) {
	    # constant and wildcard fields can have any size
	}
	elsif ($field_char =~ /[sS]/) {
	    # operand-size fields must be size two:
	    die "stdin:$line: $field_char field must be size two\n"
		if ($field_size != 2);
	}
	elsif ($field_char =~ /[xy]/) {
	    # effective address fields must be size six:
	    die "stdin:$line: $field_char field must be size six\n"
		if ($field_size != 6);
	    # effective address fields must be used by some argument:
	    for ($arg_i = 0; $arg_i < @args; $arg_i++) {
		last if ($args[$arg_i] =~ /^$field_char([sS]|(\d+))\//);
	    }
	    die "stdin:$line: $field_char field not used by any argument\n"
		if ($arg_i == @args);
	    print ", arg #$arg_i" if ($debug);
	    eval("\$field_${field_char}_arg = $arg_i;");
	}
	elsif ($field_char =~ /[daDAq]/) {
	    # register and small-immediate fields must be size three:
	    die "stdin:$line: $field_char field must be size three\n"
		if ($field_size != 3);
	}
	elsif ($field_char eq "c") {
	    # condition code fields must be size four:
	    die "stdin:$line: c field must be size four\n"
		if ($field_size != 4);
	}
	else {
	    die "stdin:$line: unknown field type $field_char\n";
	}
	print "\n" if ($debug);
    }
    print "fields: ".join(" ", @fields)."\n" if ($debug);

    # recursively go over the fields of this template, forming real,
    # legal instruction patterns, and their full decoding:
    $field_depth = 0;
    undef(@field_n0_values);
    @field_n0_values = ("");
    @pattern_stack = ("");
    @instruction_stack = ("");
    print "root\n" if ($debug);
    $pattern = "";
    $instruction = "";
    for (;;) {

	# find the next field value, popping the stack as needed, and
	# form the next pattern:
	for (; !defined($field_value = eval("shift(\@field_n${field_depth}_values)")); ) {
	    eval("undef(\$field_$fields[$field_depth]);");
	    last if (--$field_depth < 0);
	    $field_type = $fields[$field_depth];
	    pop(@pattern_stack);
	    pop(@instruction_stack);
	}
	last if ($field_depth < 0);
	($pattern_part, $field_value, $instruction_part) = split(/\//, $field_value, 3);
	$pattern_part = "" if (!defined($pattern_part));
	$field_value = $pattern_part if (!defined($field_value) || $field_value eq "");
	$instruction_part = "" if (!defined($instruction_part));
	$pattern = $pattern_stack[$field_depth].$pattern_part;
	$instruction = $instruction_stack[$field_depth].$instruction_part;
	eval("\$field_$fields[$field_depth] = \"$field_value\";");

	# if we don't have a complete pattern, descend into the
	# next field and expand it:
	if ($field_depth < $#fields) {
	    $field_type = $fields[++$field_depth];
	    push(@pattern_stack, $pattern);
	    push(@instruction_stack, $instruction);
	    @field_values = ();

	    # a constant field:
	    if ($field_type =~ /^[01]/) {
		@field_values = ($field_type);
	    }

	    # a wildcard field:
	    elsif ($field_type eq "w") {
		@field_values = ("0", "1");
	    }

	    # a size field:
	    elsif ($field_type eq "s") {
		@field_values = ("00/8", "01/16", "10/32");
	    }

	    # the wacko size field:
	    elsif ($field_type eq "S") {
		@field_values = ("01/8", "11/16", "10/32");
	    }

	    # an eax or eay field:
	    elsif ($field_type =~ /[xy]/) {
		
		# take apart the argument:
		$arg_i = eval("\$field_${field_type}_arg;");
		die "stdin:$line: $args[$arg_i] is a bad EA argument\n"
		    unless ($args[$arg_i] =~ /^${field_type}([sS]|(\d+))\/([^\/]+)(\/(\S\S))?$/);
		($eax_size, $eax_cat_name, $eax_arg_cycles) = ($1, $3, $5);

		# if the EA size depends on a size field, get it:
		if ($eax_size =~ /[sS]/) {
		    die "stdin:$line: $args[$arg_i] argument with no size field?\n"
			if (!defined($eax_size = eval("\$field_$eax_size;")));
		}

		# get the field values:
		eval("\@field_values = \@ea${field_type}_${eax_cat_name}${eax_size};");

		# if this argument is overriding the default cycles on the
		# EA category, enforce the override on the field values:
		if (defined($eax_arg_cycles)
		    && $eax_arg_cycles ne "") {
		    foreach $field_value (@field_values) {
			$field_value =~ s/\.(un|ro|wo|rw)$/\.$eax_arg_cycles/;
		    }
		}
	    }

	    # a register field:
	    elsif ($field_type =~ /[daDA]/) {
		@field_values = ("000/0", "001/1", "010/2", "011/3", "100/4", "101/5", "110/6", "111/7");
	    }

	    # a quick constant field:
	    elsif ($field_type eq "q") {
		@field_values = ("000/8", "001/1", "010/2", "011/3", "100/4", "101/5", "110/6", "111/7");
	    }

	    # a condition code field:
	    elsif ($field_type eq "c") {
		@field_values = ("0000/t",  "0001/f",  "0010/hi", "0011/ls",
				 "0100/cc", "0101/cs", "0110/ne", "0111/eq",
				 "1000/vc", "1001/vs", "1010/pl", "1011/mi",
				 "1100/ge", "1101/lt", "1110/gt", "1111/le");
	    }

	    # this field must have some values:
	    die "stdin:$line: $field_type-field has no values\n"
		if (@field_values == 0);
	    print "".("  " x $field_depth)."$field_type-field values: ".join(" ", @field_values)."\n" if ($debug);
	    eval("\@field_n${field_depth}_values = \@field_values;");

	    # loop to pick up the first value from this depth,
	    # and possibly continue descending:
	    next;
	}

	# EXCEPTION: a move of an address register to a predecrement
	# or postincrement EA with that same address register, must
	# store the original value of the address register.  since the
	# predecrement and postincrement code in the executer updates
	# the address register before the move has happened, we wrap
	# the normal move function in another that gives an op1
	# argument that is the original value of the address register:
	if ($func =~ /^move_srp[di]\.S/) {
	    $func = 'move.S';
	}
	if ($func eq 'move.S'
	    && substr($pattern, (15 - 5), 6) eq '001'.substr($pattern, (15 - 11), 3)) {
	    if (substr($pattern, (15 - 8), 3) eq '100') {
		$func = 'move_srpd.S';
	    }
	    elsif (substr($pattern, (15 - 8), 3) eq '011') {
		$func = 'move_srpi.S';
	    }
	}
	    
	# the function name:
	if ($func =~ /^(.*)\.([sS])$/) {
	    die "stdin:$line: $func with no size field?\n"
		if (!defined($_ = eval("\$field_$2;")));
	    $instruction .= " func=$1$_";
	}
	else {
	    $instruction .= " func=$func";
	}

	# handle the arguments:
	for ($arg_i = 0; $arg_i < @args ; $arg_i++) {
	    $arg = $args[$arg_i];

	    # replace any .s with the s-field:
	    $arg = "$1.$field_s" if ($arg =~ /^(.*)\.s$/);
	    $arg = "$1.$field_S" if ($arg =~ /^(.*)\.S$/);

	    # an immediate argument:
	    $arg = "#$field_s" if ($arg eq "#s");
	    $arg = "#$field_S" if ($arg eq "#S");
	    if ($arg =~ /^\#(\d+)$/ || $arg =~ /^\#(16S32)$/) {
		$instruction .= " imm_size=$1 imm_operand=$arg_i";
	    }

	    # an EA argument:
	    elsif ($arg =~ /^([xy])/) {
		$field_type = $1;
		$field_value = eval("\$field_${field_type};");
		if ($field_value =~ /^mem${field_type}\.(\d+)\.(\S\S)$/) {
		    die "stdin:$line: unsized ea${field_type} memory argument $field_value (internal error?)\n"
			if ($1 == 0);
		    die "stdin:$line: ea${field_type} memory argument with no cycles (internal error?)\n"
			if ($2 eq "un");
		    $instruction .= " ea${field_type}_size=$1 ea${field_type}_cycles=$2";
		    $field_value = "mem${field_type}.$1";
		}
		elsif ($field_value =~ /^imm\.(\d+)$/) {
		    $instruction .= " imm_size=$1 imm_operand=$arg_i";
		    $field_value = "";
		}
		elsif ($field_value eq "eax.32") {
		    $instruction .= " eax_size=UNSIZED";
		}
		$instruction .= " op${arg_i}=$field_value" if ($field_value ne "");
	    }

	    # a register argument:
	    elsif ($arg =~ /^\%([daDA])(\.\d+)$/) {
		$field_type = $1;
		$reg = $field_type;
		$reg =~ tr/A-Z/a-d/;
		$instruction .= " op${arg_i}=\%$reg".eval("\$field_$1;").$2;
	    }

	    # a register number argument:
	    elsif ($arg =~ /^\#([daDa])/) {
		$field_type = $1;
		$instruction .= " op${arg_i}=imm".eval("\$field_$1;").".32";
	    }

	    # a quick integer argument:
	    elsif ($arg =~ /^\#q\.(\d+)/) {
		$instruction .= " op${arg_i}=imm${field_q}.$1";
	    }

	    # a hard-coded immediate argument:
	    elsif ($arg =~ /^\#(\d+)\.(\d+)/) {
		$instruction .= " op${arg_i}=imm$1.$2";
	    }

	    # handle a dummy argument:
	    elsif ($arg eq "X") {
	    }

	    else {
		die "stdin:$line: argument type $arg unknown\n";
	    }
	}
	    
	# we have a complete pattern and decoded instruction:
	print "$pattern$instruction\n";
    }
   
}

print "cpu-end $cpu_name\n";

# done:
exit(0);
