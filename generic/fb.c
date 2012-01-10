/* $Id: fb.c,v 1.5 2007/08/25 21:05:30 fredette Exp $ */

/* generic/fb.c - generic framebuffer implementation support: */

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

#include <tme/common.h>
_TME_RCSID("$Id: fb.c,v 1.5 2007/08/25 21:05:30 fredette Exp $");

/* includes: */
#include <tme/generic/fb.h>

/* include the automatically-generated translation functions: */
#include "fb-xlat-auto.c"

/* macros: */
#define TME_FB_COLORSET_DIRECT_COLOR	(1)
#define TME_FB_COLORSET_PSEUDO_COLOR	(2)

/* this returns the best translation function: */
const struct tme_fb_xlat *
tme_fb_xlat_best(const struct tme_fb_xlat *xlat_user)
{
  unsigned int xlat_i;
  const struct tme_fb_xlat *xlat;
  const struct tme_fb_xlat *xlat_best;
  unsigned int xlat_best_score, xlat_score;

  /* loop over the xlats: */
  xlat_best = NULL;
  xlat_best_score = 0;
  for (xlat_i = 0;
       xlat_i < TME_ARRAY_ELS(tme_fb_xlats);
       xlat_i++) {

    /* get this xlat: */
    xlat = &tme_fb_xlats[xlat_i];
    xlat_score = 0;

    /* if this xlat only works for a particular value of the given
       member, and the user's value is different, we cannot use this
       xlat.  otherwise, increase this xlat's score: */
#define TME_FB_XLAT_SCORE(score, member, specific)	\
if ((xlat->member specific)				\
    && (xlat->member != xlat_user->member)) {		\
  continue;						\
}							\
if (xlat->member specific)				\
  xlat_score += score

    TME_FB_XLAT_SCORE(100, tme_fb_xlat_width, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_height, != 0);
    TME_FB_XLAT_SCORE(  0, tme_fb_xlat_scale, || TRUE);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_depth, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_bits_per_pixel, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_skipx, >= 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_scanline_pad, != 0);
    TME_FB_XLAT_SCORE(  0, tme_fb_xlat_src_order, || TRUE);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_class, != TME_FB_XLAT_CLASS_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_map, != TME_FB_XLAT_MAP_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_map_bits, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_mask_g, != TME_FB_XLAT_MASK_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_mask_r, != TME_FB_XLAT_MASK_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_src_mask_b, != TME_FB_XLAT_MASK_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_depth, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_bits_per_pixel, != 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_skipx, >= 0);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_scanline_pad, != 0);
    TME_FB_XLAT_SCORE(  0, tme_fb_xlat_dst_order, || TRUE);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_map, != TME_FB_XLAT_MAP_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_mask_g, != TME_FB_XLAT_MASK_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_mask_r, != TME_FB_XLAT_MASK_ANY);
    TME_FB_XLAT_SCORE(100, tme_fb_xlat_dst_mask_b, != TME_FB_XLAT_MASK_ANY);

#undef TME_FB_XLAT_SCORE

    /* update the best xlat: */
    if (xlat_best == NULL
	|| xlat_best_score < xlat_score) {
      xlat_best = xlat;
      xlat_best_score = xlat_score;
    }
  }

  /* return the best xlat: */
  assert (xlat_best != NULL);
  return (xlat_best);
}

/* this returns nonzero iff the translation function is optimal: */
int
tme_fb_xlat_is_optimal(const struct tme_fb_xlat *xlat)
{
  return (xlat->tme_fb_xlat_width != 0
	  && xlat->tme_fb_xlat_height != 0
	  && xlat->tme_fb_xlat_src_depth != 0
	  && xlat->tme_fb_xlat_src_bits_per_pixel != 0
	  && xlat->tme_fb_xlat_src_skipx >= 0
	  && xlat->tme_fb_xlat_src_scanline_pad != 0
	  && xlat->tme_fb_xlat_src_class != TME_FB_XLAT_CLASS_ANY
	  && xlat->tme_fb_xlat_src_map != TME_FB_XLAT_MAP_ANY
	  && xlat->tme_fb_xlat_src_map_bits != 0
	  && xlat->tme_fb_xlat_src_mask_g != TME_FB_XLAT_MASK_ANY
	  && xlat->tme_fb_xlat_src_mask_r != TME_FB_XLAT_MASK_ANY
	  && xlat->tme_fb_xlat_src_mask_b != TME_FB_XLAT_MASK_ANY
	  && xlat->tme_fb_xlat_dst_depth != 0
	  && xlat->tme_fb_xlat_dst_bits_per_pixel != 0
	  && xlat->tme_fb_xlat_dst_skipx >= 0
	  && xlat->tme_fb_xlat_dst_scanline_pad != 0
	  && xlat->tme_fb_xlat_dst_map != TME_FB_XLAT_MAP_ANY
	  && xlat->tme_fb_xlat_dst_mask_g != TME_FB_XLAT_MASK_ANY
	  && xlat->tme_fb_xlat_dst_mask_r != TME_FB_XLAT_MASK_ANY
	  && xlat->tme_fb_xlat_dst_mask_b != TME_FB_XLAT_MASK_ANY
	  );
}

/* this returns the number of bytes required for a source framebuffer
   scanline: */
static unsigned long
_tme_fb_xlat_src_bypl(const struct tme_fb_connection *src)
{
  /* NB that this definition must match the one in the
     automatically-generated xlat functions: */
  const unsigned long src_bypl
    = (((((src->tme_fb_connection_skipx
	   + src->tme_fb_connection_width)
	  * src->tme_fb_connection_bits_per_pixel)
	 + (src->tme_fb_connection_scanline_pad - 1))
	& -src->tme_fb_connection_scanline_pad)
       / 8);
  return (src_bypl);
}

/* this returns the number of bytes required for a source framebuffer: */
static unsigned long
_tme_fb_xlat_src_bypb_real(const struct tme_fb_connection *src)
{
  /* NB that these definitions must match those in the
     automatically-generated xlat functions: */
  const unsigned long src_bypl
    = _tme_fb_xlat_src_bypl(src);
  const unsigned long src_bypb_real
    = (((src->tme_fb_connection_height * src_bypl) + 3) & -4);
  return (src_bypb_real);
}

/* this returns the number of bytes allocated for a source framebuffer.
   this includes the guard regions that are needed to guarantee that
   the translation function main loop terminates: */
static unsigned long
_tme_fb_xlat_src_bypb(const struct tme_fb_connection *src)
{
  /* NB that this definition must match the one in the
     automatically-generated xlat functions: */
  const unsigned long src_bypl
    = _tme_fb_xlat_src_bypl(src);
  const unsigned long src_bypb_real
    = _tme_fb_xlat_src_bypb_real(src);
  const unsigned long src_bypb
    = ((src_bypb_real + (src_bypl * 2)) & -4);
  return (src_bypb);
}

/* this forces the next translation to retranslate the entire buffer: */
void
tme_fb_xlat_redraw(struct tme_fb_connection *src)
{
  const tme_uint32_t *src_user;
  tme_uint32_t *src_back;
  unsigned int count32;

  src_user
    = ((const tme_uint32_t *) 
       src->tme_fb_connection_buffer);
  src_back
    = ((tme_uint32_t *) 
       (src->tme_fb_connection_buffer
	+ _tme_fb_xlat_src_bypb(src)));
  for (count32 = _tme_fb_xlat_src_bypb_real(src) / sizeof(tme_uint32_t);
       count32-- > 0; ) {
    *(src_back++) = ~(*(src_user++));
  }
}

/* this allocates memory for a source framebuffer: */
int
tme_fb_xlat_alloc_src(struct tme_fb_connection *src)
{

  /* allocate the buffer.  remember, this is really two buffers - the
     first half is the real, current framebuffer, and the second half
     holds the last frame that was translated: */
  src->tme_fb_connection_buffer
    = tme_new0(tme_uint8_t,
	       _tme_fb_xlat_src_bypb(src) * 2);

  /* force the next translation to do a complete redraw: */
  tme_fb_xlat_redraw(src);
  
  return (TME_OK);
}

/* this internal function gets or sets the needed colors on a
   destination framebuffer connection that is using the common
   translation functions: */
static tme_uint32_t
_tme_fb_xlat_colors_get_set(const struct tme_fb_connection *src,
			    unsigned int scale,
			    struct tme_fb_connection *dst,
			    struct tme_fb_color **_colors,
			    int get)
{
  tme_uint32_t src_mask;
  tme_uint32_t src_mask_g;
  tme_uint32_t src_mask_r;
  tme_uint32_t src_mask_b;
  tme_uint32_t src_shift_g;
  tme_uint32_t src_shift_r;
  tme_uint32_t src_shift_b;
  const void *src_map_g;
  const void *src_map_r;
  const void *src_map_b;
  tme_uint32_t value_g;
  tme_uint32_t value_r;
  tme_uint32_t value_b;
  tme_uint32_t src_max_g;
  tme_uint32_t src_max_r;
  tme_uint32_t src_max_b;
  tme_uint32_t color_count;
  tme_uint32_t color_i;
  tme_uint32_t color_j;
  int compress_colors;
  unsigned int dst_depth_g;
  unsigned int dst_depth_r;
  unsigned int dst_depth_b;
  struct tme_fb_color *colors;
  tme_uint32_t *pixels;
  tme_uint32_t invert_mask;
  tme_uint32_t colorset;

  /* get how to decompose source pixels into subfields.  if source
     pixels have no subfields, act as if all of the subfields are the
     entire source pixel: */
  src_mask = (0xffffffff >> (32 - src->tme_fb_connection_depth));
  src_mask_g = src->tme_fb_connection_mask_g;
  src_mask_r = src->tme_fb_connection_mask_r;
  src_mask_b = src->tme_fb_connection_mask_b;
  if (src_mask_g == 0) {
    src_mask_g = src_mask;
    src_mask_r = src_mask;
    src_mask_b = src_mask;
  }

  /* if source intensities are index mapped, their common maximum is
     the mask of the mapping size range in bits: */
  src_map_g = src->tme_fb_connection_map_g;
  src_map_r = src->tme_fb_connection_map_r;
  src_map_b = src->tme_fb_connection_map_b;
  if (src_map_g != NULL) {
    src_max_g = (0xffffffff >> (32 - src->tme_fb_connection_map_bits));
    src_max_r = src_max_g;
    src_max_b = src_max_g;
  }

  /* otherwise, source intensities are linearly mapped, and each
     primary's maximum intensity is the base mask of its subfield: */
  else {
    src_max_g = TME_FB_XLAT_MAP_BASE_MASK(src_mask_g);
    src_max_r = TME_FB_XLAT_MAP_BASE_MASK(src_mask_r);
    src_max_b = TME_FB_XLAT_MAP_BASE_MASK(src_mask_b);
  }

  /* the source intensity maximums must be greater than zero, and not
     larger than 16 bits: */
  assert (src_max_g > 0 && src_max_g <= 0xffff);
  assert (src_max_r > 0 && src_max_r <= 0xffff);
  assert (src_max_b > 0 && src_max_b <= 0xffff);

  /* get any inversion mask: */
  invert_mask = (src->tme_fb_connection_inverted ? 0xffff : 0);

  /* if we're halving, the intensity maximums are four times what they
     would be otherwise, because the intensities from four pixels are
     added together: */
  if (scale == TME_FB_XLAT_SCALE_HALF) {
    src_max_g *= 4;
    src_max_r *= 4;
    src_max_b *= 4;
  }

  /* if we're not halving, and either the source pixel mask is less
     than the maximum index mask or source pixels have no subfields,
     we will index map source pixels directly to destination
     pixels: */
  if (scale != TME_FB_XLAT_SCALE_HALF
      && (src_mask <= TME_FB_XLAT_MAP_INDEX_MASK_MAX
	  || src_mask_g == src_mask)) {

    /* we will allocate as many colors as we have source pixels: */
    color_count = src_mask + 1;
    colorset = TME_FB_COLORSET_NONE;
  }
  
  /* otherwise, either we're halving, or source pixels are too big to
     map directly and they have subfields.  the translation code will 
     decompose pixels into intensities: */

  /* if the source class is monochrome, we only have to deal with
     the green primary: */
  else if (src->tme_fb_connection_class == TME_FB_XLAT_CLASS_MONOCHROME) {

    /* the translation function will index map green intensity values
       directly into pixels.  we may need to scale the intensities so
       they can be indexed: */
    for (; src_max_g > TME_FB_XLAT_MAP_INDEX_MASK_MAX; src_max_g >>= 1);

    /* allocate colors for src_max_g intensities: */
    color_count = src_max_g + 1;
    colorset = TME_FB_COLORSET_NONE;
    src_mask_g = TME_FB_XLAT_MAP_INDEX_MASK_MAX;
    src_map_g = NULL;
    src_mask_r = src_mask_g;
    src_mask_b = src_mask_g;
    src_map_r = src_map_g;
    src_map_b = src_map_g;
    src_max_r = src_max_g;
    src_max_b = src_max_g;
  }

  /* otherwise, we have to deal with all three primaries: */

  /* if the destination is color and has subfields: */
  else if (dst->tme_fb_connection_class == TME_FB_XLAT_CLASS_COLOR
	   && dst->tme_fb_connection_mask_g != 0) {
	
    /* if the destination maps intensities linearly, we don't need to
       allocate colors at all: */
    if (dst->tme_fb_connection_map_g == NULL) {

      /* however, we can't do this yet with a source framebuffer that
	 is inverted: */
      if (src->tme_fb_connection_inverted) {
	abort();
      }
	  
      /* nothing to do */
      *_colors = NULL;
      dst->tme_fb_connection_map_pixel_count = 0;
      return (TME_FB_COLORSET_NONE);
    }

    /* otherwise, the translation function will index map the
       intensity values into subfield values.  we may need to scale
       the intensities so they can be indexed: */
    for (; src_max_g > TME_FB_XLAT_MAP_INDEX_MASK_MAX; src_max_g >>= 1);
    for (; src_max_r > TME_FB_XLAT_MAP_INDEX_MASK_MAX; src_max_r >>= 1);
    for (; src_max_b > TME_FB_XLAT_MAP_INDEX_MASK_MAX; src_max_b >>= 1);

    /* size the color array: */
    color_count = src_max_g + 1 + src_max_r + 1 + src_max_b + 1;
    colorset = TME_FB_COLORSET_DIRECT_COLOR;

    /* if we're getting the needed colors: */
    if (get) {

      /* allocate the color array: */
      colors = tme_new0(struct tme_fb_color, color_count);

      /* make the colors to allocate from all of the different
	 primary intensities: */
      color_i = 0;
#define _TME_FB_XLAT_INDEX_COLORS(value, max, primary)		\
  do {								\
    for (value = 0; value <= (max); value++, color_i++) {	\
      colors[color_i].primary = ((0xffff * value) / (max)) ^ invert_mask;\
    }								\
  } while (/* CONSTCOND */ 0)
      _TME_FB_XLAT_INDEX_COLORS(value_g, src_max_g, tme_fb_color_value_g);
      _TME_FB_XLAT_INDEX_COLORS(value_r, src_max_r, tme_fb_color_value_r);
      _TME_FB_XLAT_INDEX_COLORS(value_b, src_max_b, tme_fb_color_value_b);
#undef _TME_FB_XLAT_INDEX_COLORS

      /* return the needed colors: */
      *_colors = colors;
      dst->tme_fb_connection_map_pixel_count = color_count;
      return (colorset);
    }

    /* set up the intensity index maps: */
    colors = *_colors;
    color_i = 0;
#define __TME_FB_XLAT_INDEX_SUBFIELDS(value, max, primary_mask, primary_shift, primary_map, type)\
  do {											\
    dst->primary_map = tme_new(type, (max) + 1);					\
    for (value = 0; value <= (max); value++, color_i++) {				\
      ((type *) dst->primary_map)[value]						\
	= ((colors[color_i].tme_fb_color_pixel						\
	    & dst->primary_mask)							\
	   >> primary_shift);								\
    }											\
  } while (/* CONSTCOND */  0)
#define _TME_FB_XLAT_INDEX_SUBFIELDS(value, max, primary_mask, primary_shift, primary_map)\
  do {											\
    for (primary_shift = 0;								\
	 ((dst->primary_mask >> primary_shift) & 1) == 0;				\
	 primary_shift++);								\
    if (TME_FB_XLAT_MAP_BASE_MASK(dst->primary_mask) <= 0xff) {			\
      __TME_FB_XLAT_INDEX_SUBFIELDS(value, max, primary_mask, primary_shift, primary_map, tme_uint8_t);\
    }											\
    else {										\
      __TME_FB_XLAT_INDEX_SUBFIELDS(value, max, primary_mask, primary_shift, primary_map, tme_uint16_t);\
    }											\
  } while (/* CONSTCOND */ 0)
    _TME_FB_XLAT_INDEX_SUBFIELDS(value_g, src_max_g, tme_fb_connection_mask_g, src_shift_g, tme_fb_connection_map_g);
    _TME_FB_XLAT_INDEX_SUBFIELDS(value_r, src_max_r, tme_fb_connection_mask_r, src_shift_r, tme_fb_connection_map_r);
    _TME_FB_XLAT_INDEX_SUBFIELDS(value_b, src_max_b, tme_fb_connection_mask_b, src_shift_b, tme_fb_connection_map_b);
#undef _TME_FB_XLAT_INDEX_SUBFIELDS
#undef __TME_FB_XLAT_INDEX_SUBFIELDS
  }

  /* otherwise, the destination is either not color or it doesn't
     have subfields.  we need to allocate colors to map fake
     source pixels: */
  else {
    src_mask_g = TME_FB_XLAT_MASK_DEFAULT_G;
    src_mask_r = TME_FB_XLAT_MASK_DEFAULT_R;
    src_mask_b = TME_FB_XLAT_MASK_DEFAULT_B;
    src_map_g = NULL;
    src_map_r = NULL;
    src_map_b = NULL;
    src_max_g = TME_FB_XLAT_MAP_BASE_MASK(src_mask_g);
    src_max_r = TME_FB_XLAT_MAP_BASE_MASK(src_mask_r);
    src_max_b = TME_FB_XLAT_MAP_BASE_MASK(src_mask_b);
    color_count = (src_mask_g | src_mask_r | src_mask_b) + 1;
    colorset = TME_FB_COLORSET_PSEUDO_COLOR;
  }

  /* if we get here, we're allocating colors from source pixel values
     (or possibly fake source pixel values): */

  /* if we're getting the needed colors: */
  if (get) {

    /* if the number of colors we need is clearly more than the
       destination's depth can possibly handle, and the destination is
       either monochrome or is color with no pixel subfields, asking
       for this huge number of distinct colors will probably get us a
       poorly representative subset.
       
       in general, we can't help this.  but in the specific case of
       source pixels that directly map to intensity(s), we can remove
       some of the less significant bits of the source pixel subfield
       mask(s), so that we only ask to allocate as many distinct
       colors as the destination's depth can handle: */
    compress_colors
      = ((color_count >> dst->tme_fb_connection_depth) > 0
	 && (dst->tme_fb_connection_class == TME_FB_XLAT_CLASS_MONOCHROME
	     || dst->tme_fb_connection_mask_g == 0)
	 && src_map_g == NULL);
    if (compress_colors) {

      /* get the depth for each primary: */
      if (src->tme_fb_connection_class == TME_FB_XLAT_CLASS_MONOCHROME) {
	dst_depth_g = dst->tme_fb_connection_depth;
	dst_depth_r = 0;
	dst_depth_b = 0;
      }
      else {
	dst_depth_r = dst->tme_fb_connection_depth;
	dst_depth_g = (dst_depth_r + 2) / 3;
	dst_depth_r -= dst_depth_g;
	dst_depth_b = (dst_depth_r + 1) / 2;
	dst_depth_r -= dst_depth_b;
      }

      /* remove some of the less significant bits in the subfield
         masks and recalculate the source intensity maximums: */
#define _TME_FB_XLAT_LIMIT_MASK(src_mask_i, src_max_i, dst_depth_i)		\
  do {										\
    src_mask_i ^= (src_mask_i & (src_mask_i >> dst_depth_i));			\
    src_max_i = (src_mask_i ? TME_FB_XLAT_MAP_BASE_MASK(src_mask_i) : 1);	\
  } while (/* CONSTCOND */ 0)
      _TME_FB_XLAT_LIMIT_MASK(src_mask_g, src_max_g, dst_depth_g);
      _TME_FB_XLAT_LIMIT_MASK(src_mask_r, src_max_r, dst_depth_r);
      _TME_FB_XLAT_LIMIT_MASK(src_mask_b, src_max_b, dst_depth_b);
#undef _TME_FB_XLAT_LIMIT_MASK
    }

    /* make shift counts for the subfields and shift their trailing zeroes off: */
    for (src_shift_g = 0; (src_mask_g & 1) == 0; src_shift_g++, src_mask_g >>= 1);
    for (src_shift_r = 0; (src_mask_r & 1) == 0; src_shift_r++, src_mask_r >>= 1);
    for (src_shift_b = 0; (src_mask_b & 1) == 0; src_shift_b++, src_mask_b >>= 1);

    /* allocate the color array: */
    colors = tme_new0(struct tme_fb_color, color_count);

    /* loop over the source pixels: */
    for (color_i = 0;
	 color_i < color_count;
	 color_i++) {
      
      /* get the raw primary values: */
      value_g = (color_i >> src_shift_g) & src_mask_g;
      value_r = (color_i >> src_shift_r) & src_mask_r;
      value_b = (color_i >> src_shift_b) & src_mask_b;

      /* we give a distinct pixel value for each distinct color we ask
	 for, since we may ask for a lot of duplicates.  this isn't
	 really a pixel value at all, anywhere - it only serves to
	 group duplicate colors together in the color array: */
      if (compress_colors) {
	color_j = value_g;
	color_j = (color_j * (src_mask_r + 1)) + value_r;
	color_j = (color_j * (src_mask_b + 1)) + value_b;
      }
      else {
	color_j = color_i;
      }

      /* if the primaries are index mapped, turn the raw primary
	 values into intensities: */
      if (src_map_g != NULL) {

	/* if intensities are stored as 8 bits: */
	if (src_max_g <= 0xff) {
	  value_g = ((const tme_uint8_t *) src_map_g)[value_g];
	  value_r = ((const tme_uint8_t *) src_map_r)[value_r];
	  value_b = ((const tme_uint8_t *) src_map_b)[value_b];
	}

	/* otherwise, intensities are stored as 16 bits: */
	else {
	  value_g = ((const tme_uint16_t *) src_map_g)[value_g];
	  value_r = ((const tme_uint16_t *) src_map_r)[value_r];
	  value_b = ((const tme_uint16_t *) src_map_b)[value_b];
	}
      }

      /* scale the intensities to 16 bits and possibly invert them: */
      value_g = ((0xffff * value_g) / src_max_g) ^ invert_mask;
      value_r = ((0xffff * value_r) / src_max_r) ^ invert_mask;
      value_b = ((0xffff * value_b) / src_max_b) ^ invert_mask;

      /* if the source class is monochrome, use only the green primary: */
      if (src->tme_fb_connection_class == TME_FB_XLAT_CLASS_MONOCHROME) {
	value_r = value_g;
	value_b = value_g;
      }

      /* allocate this color: */
      colors[color_i].tme_fb_color_pixel = color_j;
      colors[color_i].tme_fb_color_value_g = value_g;
      colors[color_i].tme_fb_color_value_r = value_r;
      colors[color_i].tme_fb_color_value_b = value_b;
    }

    /* return the needed colors: */
    *_colors = colors;
    dst->tme_fb_connection_map_pixel_count = color_count;
    return (colorset);
  }

  /* otherwise, take in the allocated colors: */
  colors = *_colors;
  pixels = tme_new(tme_uint32_t, color_count);
  for (color_i = 0;
       color_i < color_count;
       color_i++) {
    pixels[color_i] = colors[color_i].tme_fb_color_pixel;
  }
  dst->tme_fb_connection_map_pixel = pixels;
  dst->tme_fb_connection_map_pixel_count = color_count;
  tme_free(colors);
  return (0);
}

/* this gets the needed colors on a destination framebuffer connection
   that is using the common translation functions: */
tme_uint32_t
tme_fb_xlat_colors_get(const struct tme_fb_connection *src,
		       unsigned int scale,
		       struct tme_fb_connection *dst,
		       struct tme_fb_color **_colors)
{
  return (_tme_fb_xlat_colors_get_set(src, scale, dst, _colors, TRUE));
}

/* this sets the needed colors on a destination framebuffer connection
   that is using the common translation functions: */
void
tme_fb_xlat_colors_set(const struct tme_fb_connection *src,
		       unsigned int scale,
		       struct tme_fb_connection *dst,
		       struct tme_fb_color *colors)
{
  _tme_fb_xlat_colors_get_set(src, scale, dst, &colors, FALSE);
}

/* this scores a framebuffer connection: */
int
tme_fb_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;

  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);

  /* one side must be a real display and the other side must be a
     framebuffer emulator: */
  conn_fb = (struct tme_fb_connection *) conn;
  conn_fb_other = (struct tme_fb_connection *) conn->tme_connection_other;
  *_score = ((conn_fb->tme_fb_connection_mode_change != NULL)
	     != (conn_fb_other->tme_fb_connection_mode_change != NULL));
  return (TME_OK);
}

