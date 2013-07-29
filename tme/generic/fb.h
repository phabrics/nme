/* $Id: fb.h,v 1.4 2008/09/24 22:45:55 fredette Exp $ */

/* tme/generic/fb.h - header file for generic framebuffer support: */

/*
 * Copyright (c) 2003 Matt Fredette
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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

#ifndef _TME_GENERIC_FB_H
#define _TME_GENERIC_FB_H

#include <tme/common.h>
_TME_RCSID("$Id: fb.h,v 1.4 2008/09/24 22:45:55 fredette Exp $");

/* includes: */
#include <tme/element.h>

/* macros: */

/* translation scaling: */
#define TME_FB_XLAT_SCALE_HALF		(2 / 2)
#define TME_FB_XLAT_SCALE_NONE		(2)
#define TME_FB_XLAT_SCALE_DOUBLE	(2 * 2)

/* the different framebuffer classes: */
#define TME_FB_XLAT_CLASS_ANY		(0)
#define TME_FB_XLAT_CLASS_MONOCHROME	(1)
#define TME_FB_XLAT_CLASS_COLOR		(2)

/* the different mapping types: */
#define TME_FB_XLAT_MAP_ANY		(0)
#define TME_FB_XLAT_MAP_LINEAR		(1)
#define TME_FB_XLAT_MAP_INDEX		(2)

/* when we have primary intensities but no destination pixel subfield
   masks, we use these masks: */
#define TME_FB_XLAT_MASK_DEFAULT_G	(0x07e0)
#define TME_FB_XLAT_MASK_DEFAULT_R	(0xf800)
#define TME_FB_XLAT_MASK_DEFAULT_B	(0x001f)

/* since you can't have a noncontiguous subfield mask, we use one as
   the wildcarded subfield mask: */
#define TME_FB_XLAT_MASK_ANY		(0x5)

/* this is the maximum value that will be used as an index in an index
   mapping: */
#define TME_FB_XLAT_MAP_INDEX_MASK_MAX	(0xffff)
#if (TME_FB_XLAT_MAP_INDEX_MASK_MAX > 0xffff)
#error "maximum index mask is greater than 0xffff"
#endif
#if ((TME_FB_XLAT_MASK_DEFAULT_G | TME_FB_XLAT_MASK_DEFAULT_R | TME_FB_XLAT_MASK_DEFAULT_B) > TME_FB_XLAT_MAP_INDEX_MASK_MAX)
#error "default masks are bigger than the maximum index mask"
#endif

/* TME_FB_XLAT_MAP_BASE_MASK returns a base mask, one without any
   least significant zero bits: */
#define TME_FB_XLAT_MAP_BASE_MASK(mask)			\
  ((mask) / _TME_FIELD_MASK_FACTOR(mask))

/* the return value of tme_fb_xlat_colors_get() is a colorset
   signature.  if TME_FB_COLORSET_NONE, the colorset is tied, at least
   in part, to the source framebuffer characteristics.  otherwise, the
   colorset is only tied to the destination framebuffer
   characteristics - and two such colorsets, with the same signature
   and with the same destination framebuffer characteristics, will
   always be identical.  this allows the caller to avoid unnecessary
   color reallocation: */
#define TME_FB_COLORSET_NONE		(0)

/* types: */

/* a framebuffer connection: */
struct tme_fb_connection {

  /* the generic connection side: */
  struct tme_connection tme_fb_connection;

  /* this is called when the framebuffer mode changes: */
  int (*tme_fb_connection_mode_change) _TME_P((struct tme_fb_connection *));

  /* this is called before the framebuffer's display is updated: */
  int (*tme_fb_connection_update) _TME_P((struct tme_fb_connection *));

  /* scanlines have this many displayed pixels: */
  unsigned int tme_fb_connection_width;

  /* frames have this many displayed scanlines: */
  unsigned int tme_fb_connection_height;

  /* pixels have this depth, in bits: */
  unsigned int tme_fb_connection_depth;

  /* pixels have this many bits.  this must be at least the same as
     the depth, but it can also be greater, and it must be a power of
     two: */
  unsigned int tme_fb_connection_bits_per_pixel;

  /* scanlines begin with bits for this many *undisplayed* pixels.
     these pixels are not included in tme_fb_connection_width: */
  unsigned int tme_fb_connection_skipx;

  /* scanlines are padded to this many bits.  this must be a power of
     two greater than or equal to 8: */
  unsigned int tme_fb_connection_scanline_pad;

  /* scanline data has this endianness.  this is either TME_ENDIAN_BIG
     or TME_ENDIAN_LITTLE: */
  unsigned int tme_fb_connection_order;

  /* the real framebuffer memory: */
  tme_uint8_t *tme_fb_connection_buffer;

  /* the offsets of the first and last bytes updated in the real
     framebuffer memory.  if the first offset is greater than the last
     offset, none of the framebuffer memory was updated: */
  tme_uint32_t tme_fb_connection_offset_updated_first;
  tme_uint32_t tme_fb_connection_offset_updated_last;

  /* the class of the framebuffer: */
  unsigned int tme_fb_connection_class;

  /* any masks for pixel value subfields: */
  tme_uint32_t tme_fb_connection_mask_g;
  tme_uint32_t tme_fb_connection_mask_r;
  tme_uint32_t tme_fb_connection_mask_b;

  /* these are used for index-mapping pixel values or pixel subfield
     values to intensities, or vice-versa.  if these are NULL,
     everything is linearly mapped: */
  const void *tme_fb_connection_map_g;
  const void *tme_fb_connection_map_r;
  const void *tme_fb_connection_map_b;

  /* if this is nonzero, intensities are inverted (a smaller value is
     more intense than a greater value): */
  int tme_fb_connection_inverted;

  /* in a source framebuffer connection, this is the size of an
     intensity, in bits.  this cannot be greater than 16: */
  unsigned int tme_fb_connection_map_bits;

  /* in a destination framebuffer connection, this is used to map
     source pixel values into destination pixel values: */
  const tme_uint32_t *tme_fb_connection_map_pixel;
  tme_uint32_t tme_fb_connection_map_pixel_count;
};

/* one frame buffer translation function: */
struct tme_fb_xlat {

  /* the translation function itself: */
  int (*tme_fb_xlat_func) _TME_P((struct tme_fb_connection *, struct tme_fb_connection *));

  /* iff nonzero, this function applies only when scanlines have this
     many displayed pixels: */
  unsigned int tme_fb_xlat_width;

  /* iff nonzero, this function applies only when there are this many
     scanlines: */
  unsigned int tme_fb_xlat_height;

  /* the scaling that this function does: */
  unsigned int tme_fb_xlat_scale;

  /* iff nonzero, this function applies only when source pixels have
     this depth: */
  unsigned int tme_fb_xlat_src_depth;

  /* iff nonzero, this function applies only when source pixels have
     this many bits: */
  unsigned int tme_fb_xlat_src_bits_per_pixel;

  /* iff >= 0, this function applies only when source scanlines begin
     with this many *undisplayed* pixels.  these pixels are not
     counted in tme_fb_xlat_width: */
  int tme_fb_xlat_src_skipx;

  /* iff nonzero, this function applies only when source scanlines are
     padded to this many bits: */
  unsigned int tme_fb_xlat_src_scanline_pad;

  /* this function applies only when source scanline data has this
     order (endianness): */
  int tme_fb_xlat_src_order; 

  /* iff not TME_FB_XLAT_CLASS_ANY, this function applies only to
     a source framebuffer of this class: */
  int tme_fb_xlat_src_class;

  /* iff not TME_FB_XLAT_MAP_ANY, this function applies only to
     a source framebuffer with this type of intensity mapping: */
  int tme_fb_xlat_src_map;

  /* iff nonzero, this function applies only when source intensities
     have this many bits: */
  unsigned int tme_fb_xlat_src_map_bits;

  /* iff not TME_FB_XLAT_MASK_ANY, this function applies only to a
     source framebuffer with these subfield masks: */
  tme_uint32_t tme_fb_xlat_src_mask_g;
  tme_uint32_t tme_fb_xlat_src_mask_r;
  tme_uint32_t tme_fb_xlat_src_mask_b;

  /* iff nonzero, this function applies only when destination pixels
     have this depth: */
  unsigned int tme_fb_xlat_dst_depth;

  /* iff nonzero, this function applies only when destination pixels
     have this many bits: */
  unsigned int tme_fb_xlat_dst_bits_per_pixel;

  /* iff >= 0, this function applies only when destination scanlines
     begin with this many *undisplayed* pixels.  these pixels are not
     counted in tme_fb_xlat_width: */
  int tme_fb_xlat_dst_skipx;

  /* iff nonzero, this function applies only when destination
     scanlines are padded to this many bits: */
  unsigned int tme_fb_xlat_dst_scanline_pad;

  /* this function applies only when destination scanline data has
     this order (endianness): */
  int tme_fb_xlat_dst_order; 

  /* iff not TME_FB_XLAT_MAP_ANY, this function applies only to
     a destination framebuffer with this type of intensity mapping: */
  int tme_fb_xlat_dst_map;

  /* iff not TME_FB_XLAT_MASK_ANY, this function applies only to a
     destination framebuffer with these subfield masks: */
  tme_uint32_t tme_fb_xlat_dst_mask_g;
  tme_uint32_t tme_fb_xlat_dst_mask_r;
  tme_uint32_t tme_fb_xlat_dst_mask_b;
};

/* a color: */
struct tme_fb_color {
  tme_uint32_t tme_fb_color_pixel;
  tme_uint16_t tme_fb_color_value_g;
  tme_uint16_t tme_fb_color_value_r;
  tme_uint16_t tme_fb_color_value_b;
};

/* prototypes: */
_tme_const struct tme_fb_xlat *tme_fb_xlat_best _TME_P((_tme_const struct tme_fb_xlat *));
int tme_fb_xlat_is_optimal _TME_P((_tme_const struct tme_fb_xlat *));
int tme_fb_xlat_alloc_src _TME_P((struct tme_fb_connection *));
void tme_fb_xlat_redraw _TME_P((struct tme_fb_connection *));
tme_uint32_t tme_fb_xlat_colors_get _TME_P((const struct tme_fb_connection *,
					    unsigned int,
					    struct tme_fb_connection *,
					    struct tme_fb_color **));
void tme_fb_xlat_colors_set _TME_P((const struct tme_fb_connection *,
				    unsigned int,
				    struct tme_fb_connection *,
				    struct tme_fb_color *));
int tme_fb_connection_score _TME_P((struct tme_connection *, unsigned int *));

#endif /* !_TME_GENERIC_FB_H */
