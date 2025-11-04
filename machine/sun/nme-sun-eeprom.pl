#! /usr/pkg/bin/perl -w

# $Id: tme-sun-eeprom.pl,v 1.2 2005/01/14 11:44:18 fredette Exp $

# machine/sun/tme-sun-eeprom.pl - dumps and makes Sun EEPROM contents:

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

sub binary_struct {

    # the EEPROM definition:
    #
    <<'EOF;';
# amount of memory installed and tested, in MB:
#
0x014 installed-#megs generic_char_dec 8
0x015 selftest-#megs generic_char_dec 0

# screen resolution:
#
0x016 screen-resolution generic_char_hex 0x00=1152x900 0x12=1024x1024 0x13=1600x1280 0x14=1440x1440 0x15=640x480 0x16=1280x1024
0x050 screen-#columns generic_char_dec 80
0x051 screen-#rows generic_char_dec 34

# the console device:
#
0x01f console-device generic_char_hex 0x00=onboard-bwtwo 0x10=ttya 0x11=ttyb 0x12=color-fb 0x20=p4-option

# true if the watchdog causes a reset.
#
0x017 watchdog-reboot? sun_eeprom_boolean false

# any boot device:
#
0x018 boot-device? sun_eeprom_boolean true
0x019 boot-device sun_eeprom_boot_device sd(0,0,0)

# any OEM banner and/or logo bitmap.
#
0x020 oem-banner? sun_eeprom_boolean false
0x068 oem-banner generic_string_buffer80
0x18f oem-logo? sun_eeprom_boolean false
0x290 oem-logo generic_char_hex512

# keyboard parameters.
#
0x01e keyboard-type generic_char_hex 0x00=sun *=other
0x18d keyboard-locale generic_char_hex
0x18e keyboard-id generic_char_hex
0x021 keyboard-click? sun_eeprom_boolean false

# the "diagnostic" boot device and file:
#
0x022 diag-device sun_eeprom_boot_device le(0,0,0)
0x028 diag-file generic_string_buffer40

# inverse video (white-on-black, not implemented?)
#
0x027 inverse-video? sun_eeprom_boolean false

# default parameters for ttya and ttyb:
#
0x058 ttya-mode sun_eeprom_tty_mode 9600,8,n,1,-
0x060 ttyb-mode sun_eeprom_tty_mode 9600,8,n,1,-

# security mode and password (only on PROM revisions > 2.7.0).
#
0x492 security-mode generic_char_hex 0x00=none 0x01=command 0x5e=full
0x493 security-password generic_string_buffer8

# the 3/80 diagnostic "switch".
#
0x70b diag-switch? generic_char_hex 0x06=false 0x12=true *=max

# any user-defined keymap:
#
0x18c .keymap? generic_char_hex 0x00=false 0x58=true
0x190 .keymap-uppercase generic_char_hex128
0x210 .keymap-lowercase generic_char_hex128

# a short test pattern.
#
0x0b8 .test-pattern generic_shorteb_hex 0x55aa

# "Factory Defined"
#
0x000 .testarea generic_longeb_hex
0x004 .write-count generic_shorteb_dec3
0x00c .checksum generic_char_hex3
0x010 last-hardware-update generic_longeb_hex

# make sure the EEPROM has the required length:
#
0x7ff .padding generic_char_hex

EOF;
}

# this parses a set of sun EEPROM boolean values:
#
sub type_sun_eeprom_boolean_values {
    my ($type, $count, $values) = @_;
    if (!defined($values)) {
	$values = 'false,' x $count;
	chop($values);
	$values .= ' ';
	$values .= 'true,' x $count;
	chop($values);
    }
    split(' ', $values);
}

# this packs a sun EEPROM boolean value:
#
sub type_sun_eeprom_boolean_pack {
    my ($type, $count, $value) = @_;
    my ($bad, @parts);

    @parts = split(/,/, $value);
    foreach (@parts) {
	if ($value eq 'true') {
	    $_ = 0x12;
	}
	elsif ($value eq 'false') {
	    $_ = 0x00;
	}
	else {
	    $bad = $_;
	    $_ = 0;
	}
    }
    ($bad, pack("C$count", @parts));
}

# this unpacks a sun EEPROM boolean value:
#
sub type_sun_eeprom_boolean_unpack {
    my ($type, $count, $packed) = @_;
    my (@parts);

    @parts = unpack("C$count", $packed);
    foreach (@parts) {
	if ($_ == 0x00) {
	    $_ = 'false';
	}
	else {
	    $_ = 'true';
	}
    }
    join(',', @parts);
}

# this parses a set of sun EEPROM boot device values:
#
sub type_sun_eeprom_boot_device_values {
    my ($type, $count, $values) = @_;
    if (!defined($values)) {
	('');
    }
    else {
	split(' ', $values);
    }
}

# this packs a sun EEPROM boot device value:
#
sub type_sun_eeprom_boot_device_pack {
    my ($type, $count, $value) = @_;

    if ($value =~ /^([a-z])([a-z])\((\d+),(\d+),(\d+)\)$/) {
	(undef, $1.$2.pack("CCC", $3 + 0, $4 + 0, $5 + 0));
    }
    else {
	($value, undef);
    }
}

# this unpacks a sun EEPROM boot device value:
#
sub type_sun_eeprom_boot_device_unpack {
    my ($type, $count, $packed) = @_;
    substr($packed, 0, 2).sprintf("(%d,%d,%d)", unpack("CCC", substr($packed, 2, 3)));
}

# this parses a set of sun EEPROM tty mode values:
#
sub type_sun_eeprom_tty_mode_values {
    my ($type, $count, $values) = @_;
    if (!defined($values)) {
	('');
    }
    else {
	split(' ', $values);
    }
}

# this packs a sun EEPROM tty mode value:
#
sub type_sun_eeprom_tty_mode_pack {
    my ($type, $count, $value) = @_;

    if ($value =~ /^(\d+),8,n,1,([\-h])$/) {
	(undef, pack('CnCN', ($1 == 9600 ? 0x00 : 0x12), $1, ($2 eq 'h' ? 0x00 : 0x12), 0));
    }
    else {
	($value, undef);
    }
}

# this unpacks a sun EEPROM tty mode value:
#
sub type_sun_eeprom_tty_mode_unpack {
    my ($type, $count, $packed) = @_;
    my ($baud_set, $baud, $no_flow);
    ($baud_set, $baud, $no_flow) = unpack('CnC', $packed);
    unless ($baud_set) {
	$baud = 9600;
    }
    $baud.',8,n,1,'.($no_flow ? '-' : 'h');
}
