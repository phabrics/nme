#! /usr/local/bin/perl -w

# $Id: $

# gtk-keymap-make.pl - compiles the complete decoding of all legal
# first-instruction-word values into the opcode map used by the C
# decoder:

#
# Copyright (c) 2015 Ruben Agin
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
use X11::Protocol;

$x = X11::Protocol->new();

print "static int keycode_min = ".$x->min_keycode.";\n";
print "static int keycode_max = ".$x->max_keycode.";\n";

# note all keycodes that are attached to modifiers: 
for ($keycode = 0;
     $keycode <= $x->max_keycode;
     $keycode++) {
    $keycode_to_modifier[$keycode] = 8;
}

@modifier_keymap = $x->GetModifierMapping;

for ($x_modifier = 0;
     $x_modifier < 8;
     $x_modifier++) {

    $keymap_cnt = scalar @{$modifier_keymap[$x_modifier]};
    # note all keycodes that are attached to this modifier: 
    for ($keycode_i = 0;
	 $keycode_i < $keymap_cnt;
	 $keycode_i++) {
	$keycode = ${$modifier_keymap[$x_modifier]}[$keycode_i];
	if ($keycode != 0) {
	    $keycode_to_modifier[$keycode] = $x_modifier;
	}
    }
}

@modifs = ("TME_KEYBOARD_MODIFIER_SHIFT",
	   "TME_KEYBOARD_MODIFIER_LOCK",
	   "TME_KEYBOARD_MODIFIER_CONTROL",
	   "TME_KEYBOARD_MODIFIER_MOD1",
	   "TME_KEYBOARD_MODIFIER_MOD2",
	   "TME_KEYBOARD_MODIFIER_MOD3",
	   "TME_KEYBOARD_MODIFIER_MOD4",
	   "TME_KEYBOARD_MODIFIER_MOD5",
	   "TME_KEYBOARD_MODIFIER_NONE");

print "\nstatic int keycode_to_modifier[] = {\n";

# loop over the keycodes in the keyboard mapping:
for ($keycode = 0; 
     $keycode < $x->max_keycode; 
     $keycode++) {
    # if this keycode is attached to a modifier, we will take the
    # first keysym that this keycode maps to as the keysym attached
    # to the modifier: */
    print "  ".$modifs[$keycode_to_modifier[$keycode]].",\n";
}
print "  ".$modifs[$keycode_to_modifier[$keycode]]."\n};\n";

@keymap = $x->GetKeyboardMapping($x->min_keycode, $x->max_keycode - $x->min_keycode + 1);
$keymap_cnt = scalar @keymap;

$keymap_width = scalar @{$keymap[0]};
print "\nstatic int keymap_width = ".$keymap_width.";\n";

print "\nstatic guint keymap[] = {\n";

# loop over the keycodes in the keyboard mapping:
for ($keycode = 0;
     $keycode < $keymap_cnt;
     $keycode++) {

    print ",\n" if ($keycode != 0);
    print "  ";
    # loop over the keysyms that this keycode can map to: 
    for ($keysym_i = 0;
	 $keysym_i < $keymap_width;
	 $keysym_i++) {
	print ", " if ($keysym_i != 0);
	printf("0x%x", ${$keymap[$keycode]}[$keysym_i]);
    }
}
print "\n};\n";

