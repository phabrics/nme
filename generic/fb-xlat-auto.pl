#! /usr/bin/env perl

# $Id: $

# fb-xlat-auto.pl - Add any more framebuffer translation destination
# keys at compile-time that may be needed

#
# Copyright (c) 2017 Ruben Agin
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
use strict;
use warnings;
use X11::Protocol;

shift;

my $args = join ' ', @ARGV;
my ($src, $dst) = split "dst ", $args;
my $area;
my $area_smallest = 0;
my $area_largest = 0;
my @orders = ("l", "m");
my $x = X11::Protocol->new();
my $pixmap_order = $orders[$x->{'image_byte_order'}];
my %srcs;
my %dsts;

foreach (split / /, $dst) {
    if (/(\d+)x(\d+)(.*)/) {
	$area = $1 * $2;
	$area_smallest = $area if $area_smallest == 0 || $area < $area_smallest;
	$area_largest = $area if $area > $area_largest;
	$dsts{$3} = "";
    } else {
	$dsts{$_} = "";
    }
}

print "src $args" if ($x->{'image_byte_order'} != $x->{'bitmap_bit_order'});

foreach my $screen (@{$x->{'screens'}}) {
    my $pixmap = $x->{'pixmap_formats'}->{$screen->{'root_depth'}};
    my $visual = $x->{'visuals'}->{$screen->{'root_visual'}};
    my $dst_map="";
    my $dst_masks = "_r0x%x_g0x%x_b0x%x";

    $area = $screen->{'width_in_pixels'} * $screen->{'height_in_pixels'};
    $area_smallest = $area if $area_smallest == 0 || $area < $area_smallest;
    $area_largest = $area if $area > $area_largest;
    $dst_masks = sprintf $dst_masks, $visual->{'red_mask'}, $visual->{'green_mask'}, $visual->{'blue_mask'};
    for ($x->interp('VisualClass', $visual->{'class'})) {
	if (/DirectColor/) { $dst_map="mi"; }
	elsif (/TrueColor/) { $dst_map="ml"; }
	elsif (/PseudoColor/) { $dst_map="ml"; 
				$dst_masks=""; }
	elsif (/StaticColor/) { $dst_map="ml"; 
				$dst_masks=""; }
	elsif (/StaticGray/) { $dst_map="ml"; 
			       $dst_masks=""; }
	elsif (/GrayScale/) { $dst_map="ml"; 
			      $dst_masks=""; }
	else { }
    }
    my $dst_key = "d".$screen->{'root_depth'}."b".$pixmap->{'bits_per_pixel'}."s0p".$pixmap->{'scanline_pad'}."o".$pixmap_order.$dst_map.$dst_masks;
    $dsts{$dst_key} = "";
}

# add in the new frame buffer translations to compile:
foreach (split / /, $src) {
    $srcs{$_} = [""];
    if (/(\d+)x(\d+).*/) {
	# calculate the area of this frame buffer multiplied by 100:
	$area = $1 * $2;

	# if this frame buffer would consume at least 70 percent of
	# the smallest destination screen, make an xlat function that
	# scales this frame buffer down by two:
	push @{$srcs{$_}}, "_h" if $area/$area_smallest > 0.7;

	# if this frame buffer would consume less than 30 percent of
	# the largest destination screen, make an xlat function that
	# scales this frame buffer up by two:
	push @{$srcs{$_}}, "_d" if $area/$area_smallest < 0.3;
	
    }
}

print "src ";
foreach $src (keys %srcs) {
    print "$src$_ " foreach @{$srcs{$src}};
}

print "dst ", join ' ',keys %dsts;
