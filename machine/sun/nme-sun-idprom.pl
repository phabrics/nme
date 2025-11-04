#! /usr/pkg/bin/perl -w

# $Id: tme-sun-idprom.pl,v 1.2 2003/07/29 18:21:31 fredette Exp $

# machine/sun/sun-idprom.pl - dumps and makes Sun IDPROM contents:

# Copyright (c) 2003 Matt Fredette
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

# mapping from machine name to Sun code name:
@machine_codename = 
    ( '2/120',			'sun2 Multibus',
      '2/170',			'sun2 Multibus',
      '2/50',			'sun2 VME',
      '2/130',			'sun2 VME',
      '2/160',			'sun2 VME',
      '3/75',			'Carrera',
      '3/140',			'Carrera',
      '3/150',			'Carrera',
      '3/160',			'Carrera',
      '3/180',			'Carrera',
      '3/50',			'Model 25',
      '3/260',			'Sirius',
      '3/280',			'Sirius',
      '3/110',			'Prism',
      '3/60',			'Ferrari',
      '3/E',			'Sun3E',
      '4/260',			'Sunrise',
      '4/280',			'Sunrise',
      '4/110',			'Cobra',
      '4/150',			'Cobra',
      '4/330',			'Stingray',
      '4/310',			'Stingray',
      '4/350',			'Stingray',
      '4/360',			'Stingray',
      '4/370',			'Stingray',
      '4/380',			'Stingray',
      '4/390',			'Stingray',
      '4/470',			'Sunray',
      '4/490',			'Sunray',
      '386i/MB1',		'Roadrunner-MB1',
      '386i/150',		'Roadrunner',
      '386i/250',		'Roadrunner',
      '3/460',			'Pegasus',
      '3/470',			'Pegasus',
      '3/480',			'Pegasus',
      '3/80',			'Hydra',
      '*SPARCStation 1',	'Campus',
      'SS1',			'Campus',
      '4/60',			'Campus',
      '*SPARCstation IPC',	'Phoenix',
      'IPC',			'Phoenix',
      '4/40',			'Phoenix',
      '*SPARCStation 1+',	'Campus B',
      'SS1+',			'Campus B',
      '4/65',			'Campus B',
      '*SPARCstation SLC',	'Off-Campus',
      'SLC',			'Off-Campus',
      '4/20',			'Off-Campus',
      '*SPARCStation 2',	'Calvin',
      'SS2',			'Calvin',
      '4/75',			'Calvin',
      '*SPARCstation ELC',	'Node Warrior',
      'ELC',			'Node Warrior',
      '4/25',			'Node Warrior',
      '*SPARCstation IPX',	'Hobbes',
      'IPX',			'Hobbes',
      '4/50',			'Hobbes',
      '*SPARCengine 1',		'Polaris',
      '4/E',			'Polaris',
      '4/600',			'Galaxy',
      '4/630',			'Galaxy',
      '4/670',			'Galaxy',
      '4/690',			'Galaxy',
      '*SPARCstation 10',	'Campus-2',
      'SS10',			'Campus-2',
      '*SPARCstation 20',	'Kodiak',
      'SS20',			'Kodiak',
      '*SPARCclassic X',	'Hamlet',
      'Classic X',		'Hamlet',
      '4/10',			'Hamlet',
      '*SPARCClassic',		'Sunergy',
      '4/15',			'Sunergy',
      'classic',		'Sunergy',
      '4/30',			'Sunergy',
      'SPARCstation LX',	'Sunergy',
      'LX',			'Sunergy',
      'Voyager',		'Gypsy',
      'SS5',			'Aurora',
      'SS4',			'Perigee',
      );
%machine_codename = @machine_codename;

# mapping from Sun code name to IDPROM machine type byte:
@codename_machtype = 
    ( 'sun2 Multibus',	0x01,
      'sun2 VME',	0x02,
      'Carrera',	0x11,
      'Model 25',	0x12,
      'Sirius',		0x13,
      'Prism',		0x14,
      'Ferrari',	0x17,
      'Sun3E',		0x18,
      'Sunrise',	0x21,
      'Cobra',		0x22,
      'Stingray',	0x23,
      'Sunray',		0x24,
      'Roadrunner',	0x31,
      'Roadrunner-MB1',	0x3a,
      'Pegasus',	0x41,
      'Hydra',		0x42,
      'Campus',		0x51,
      'Phoenix',	0x52,
      'Campus B',	0x53,
      'Off-Campus',	0x54,
      'Calvin',		0x55,
      'Node Warrior',	0x56,
      'Hobbes',		0x57,
      'Polaris',	0x61,
      'Galaxy',		0x71,
      'Campus-2',	0x72,
      'Kodiak',		0x72,
      'Sunergy',	0x80,
      'Gypsy',		0x80,
      'Aurora',		0x80,
      'Perigee',	0x80,
      );
%codename_machtype = @codename_machtype;

# mapping from Sun code name to sometime during the
# shipping/availability period, if known, for those machines that
# shipped with a real manufacture date in the IDPROM:
%codename_shipping =
    ( 'sun2 Multibus',	457370182,
      'Carrera',	587239630,
      'Ferrari',	564526348,
      );

# if our standard input is not a terminal, assume we're checking an IDPROM there:
unless (-t STDIN) {
    
    # read in the IDPROM:
    $size = sysread(STDIN, $idprom, 32);
    if ($size != 32) {
	print STDERR "fatal: IDPROM has wrong size ($size), must be exactly 32\n";
	exit(1);
    }

    # unpack the IDPROM:
    ($format, 
     $machtype,
     $ether0, 
     $ether1, 
     $ether2, 
     $ether3, 
     $ether4, 
     $ether5, 
     $date, 
     $serial_and_cksum) = 
	(unpack("C C C6 N N", $idprom));
    $serial = ($serial_and_cksum >> 8);

    # check the format:
    if ($format != 1) {
	print "warning: IDPROM has invalid format byte ($format), must be 1\n";
    }

    # check the checksum, which only covers the first 16 bytes:
    @bytes = (unpack("C16", $idprom));
    $cksum_old = pop(@bytes);
    $cksum_new = 0;
    foreach $byte (@bytes) {
	$cksum_new ^= $byte;
    }
    if ($cksum_new != $cksum_old) {
	print "warning: IDPROM has bad cksum ";
	print sprintf("(0x%02x)", $cksum_old);
	print ", should be ";
	print sprintf("0x%02x", $cksum_new);
	print "\n";
    }

    # turn the machine type into a set of code names and machines:
    @codenames = ();
    @machines = ();
    for ($i = 0; $i < @codename_machtype; $i += 2) {
	if ($codename_machtype[$i + 1] == $machtype) {
	    $codename = $codename_machtype[$i + 0];
	    push (@codenames, $codename);
	    @codename_machines = ();
	    for ($j = 0; $j < @machine_codename; $j += 2) {
		if ($machine_codename[$j + 1] eq $codename) {
		    $machine = $machine_codename[$j + 0];
		    if ($machine =~ /^\*(.*)$/) {
			push (@codename_machines, $1);
			last;
		    }
		    else {
			push(@codename_machines, $machine);
		    }
		}
	    }
	    if (@codename_machines > 2) {
		$codename_machines[$#codename_machines] = "or ".$codename_machines[$#codename_machines];
		$codename_machines = join(", ", @codename_machines);
	    }
	    elsif (@codename_machines == 2) {
		$codename_machines = join(" or ", @codename_machines);
	    }
	    else {
		$codename_machines = $codename_machines[0];
	    }
	    push(@machines, $codename_machines);
	}
    }
    
    # display a PROM-like banner:
    if (@codenames == 0) {
	print "warning: IDPROM has unknown machine type ";
	print sprintf("(0x%02x)", $machtype);
	print "\n";
    }
    elsif (@codenames == 1) {
	$codename = $codenames[0];
	$machines = $machines[0];
	if ($machines =~ /^\d+i?\//
	    || $machines =~ / or /) {
	    print "Sun Workstation, Model ";
	}
	print $machines;
	print " (codename \"$codename\")" unless ($codename =~ /^sun2/);
	print "\n";
    }
    else {
	print "Sun Workstation, Model ";
	for ($i = 0; $i < @codenames; $i++) {
	    $codename = $codenames[$i];
	    $machines[$i] .= " (codename \"$codename\")"
		unless ($codename =~ /^sun2/);
	}
	print join((@codenames == 2 ? " or " : ", or "), @machines);
	print "\n";
    }
    print "Serial \#$serial, Ethernet address";
    $sep = ' ';
    foreach $byte ($ether0, $ether1, $ether2, $ether3, $ether4, $ether5) {
	print $sep.sprintf("%x", $byte);
	$sep = ':';
    }
    print "\n";
    print "Manufacture date ".gmtime($date)."\n";
}

# otherwise, we're generating a new IDPROM:
else {
    
    # check our command line:
    $usage = 0;

    # the machine or code name:
    if (@ARGV == 0) {
	print STDERR "missing machine name\n";
	$usage = 1;
    }
    else {
	$name = shift(@ARGV);
	if (defined($codename_machtype{$name})) {
	    $codename = $name;
	}
	else {
	    # this should be a machine name:
	    $codename = $machine_codename{$name};
	    if (!defined($codename)) {
		$codename = $machine_codename{"*$name"};
	    }
	    if (!defined($codename)) {
		print STDERR "unknown machine name '$name'\n";
		$usage = 1;
	    }
	}
    }
    
    # the Ethernet address:
    if (@ARGV == 0) {
	print STDERR "missing Ethernet address\n";
	$usage = 1;
    }
    else {
	$ether = shift(@ARGV);
	@ether = split(/:/, $ether);
	unless ($ether =~ /^[0-9a-fA-F:]+/
		&& @ether == 6) {
	    print STDERR "bad Ethernet address '$ether'\n";
	    $usage = 1;
	}
    }

    # the optional serial number:
    if (@ARGV > 0) {
	$serial = shift(@ARGV);
	unless ($serial =~ /^\d+$/) {
	    print STDERR "bad serial number '$serial'\n";
	    $usage = 1;
	}
    }
    else {
	$serial = 1 + int(rand(100000));
    }

    # standard output must not be a terminal:
    if (-t STDOUT) {
	print STDERR "standard output must not be a terminal\n";
	$usage = 1;
    }

    # any other arguments are unexpected:
    if (@ARGV > 0) {
	print STDERR "unexpected argument '$ARGV[0]'\n";
	$usage = 1;
    }

    if ($usage) {
	print STDERR "usage: $0 MACHINE ETHERNET [ SERIAL ] > IDPROM\n";
	print STDERR "       $0 < IDPROM\n";
	exit(1);
    }

    # start the IDPROM:
    $idprom = "";

    # add in the format byte:
    $idprom .= pack("C", 1);

    # add in the machine type:
    $machtype = $codename_machtype{$codename};
    if (!defined($machtype)) {
	die "internal error - machtype byte for codename $codename missing\n";
    }
    $idprom .= pack("C", $machtype);

    # add in the ethernet address:
    foreach $byte (@ether) {
	$byte = hex($byte);
	$idprom .= pack("C", $byte);
    }
    if ($ether[0] != 0x08
	|| $ether[1] != 0x00
	|| $ether[2] != 0x20) {
	print STDERR "warning: Ethernet address $ether doesn't begin with Sun's OUI 8:0:20\n";
    }

    # add in a date, if possible:
    $date = $codename_shipping{$codename};
    if (defined($date)) {
	# smear the date within 30 days of the known date:
	$date += (int(rand(60)) - 30) * (24 * 60 * 60);
    }
    else {
	$date = 0;
    }
    $idprom .= pack("N", $date);

    # add in the serial number:
    $idprom .= substr(pack("N", $serial), 1);
    
    # calculate the checksum:
    @bytes = (unpack("C*", $idprom));
    $cksum_new = 0;
    foreach $byte (@bytes) {
	$cksum_new ^= $byte;
    }
    $idprom .= pack("C", $cksum_new);

    # add the extra 16 bytes:
    $idprom .= pack("NNNN", 0, 0, 0, 0);

    # dump out the IDPROM:
    print $idprom;
}

# done:
exit(0);
    
