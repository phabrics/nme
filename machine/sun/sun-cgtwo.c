/* $Id: sun-cgtwo.c,v 1.8 2010/06/05 19:18:05 fredette Exp $ */

/* machine/sun/sun-cgtwo.c - Sun cgtwo emulation: */

/*
 * Copyright (c) 2003, 2004, 2005 Matt Fredette
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
_TME_RCSID("$Id: sun-cgtwo.c,v 1.8 2010/06/05 19:18:05 fredette Exp $");

/* includes: */
#include <tme/machine/sun.h>
#include <tme/generic/bus-device.h>
#include <tme/generic/fb.h>
#include "sun-fb.h"

/* macros: */

/* cgtwo types: */
#define TME_SUNCG2_TYPE_NULL			(0)
#define TME_SUNCG2_TYPE_SUN3			(1)

/* a cgtwo has eight planes: */
#define TME_SUNCG2_PLANE_MAX			(8)

/* every rasterop unit has its alternate, prime registers: */
#define TME_SUNCG2_ROPC_PRIME			(9)

/* every so many CSR reads, we fake a retrace: */
#define TME_SUNCG2_CYCLE_RETRACE		(10)

/* this gives the index into the entire colormap of a pixel's
   intensity for a given primary: */
#define TME_SUNCG2_CMAP_INDEX(pixel, primary)	((((primary) - TME_SUNCG2_REG_CMAP_R) / sizeof(tme_uint16_t)) + (pixel))

/* this gives a pixel's intensity for a given primary: */
#define TME_SUNCG2_CMAP_VALUE(suncg2, pixel, primary)			\
  (tme_betoh_u16((suncg2)->tme_suncg2_cmap_raw[TME_SUNCG2_CMAP_INDEX(pixel, primary)]) & 0xff)

/* a plane bitmap has one bit per pixel: */
#define TME_SUNCG2_REG_BITMAP(x)		(0x000000 + (TME_SUNCG2_SIZ_BITMAP * (x)))
#define TME_SUNCG2_SIZ_BITMAP			((1024 * 1024) / 8)

/* the pixmap has one byte per pixel: */
#define TME_SUNCG2_REG_PIXMAP			(0x100000)
#define TME_SUNCG2_SIZ_PIXMAP			(1024 * 1024)

/* the raster op data: */
#define TME_SUNCG2_REG_ROP_DATA			(0x200000)

/* register offsets and sizes.  many registers are decoded at all
   size-aligned addresses within a 4KB page: */
#define TME_SUNCG2_SIZ_REG_PAGE			(0x001000)
#define TME_SUNCG2_REG_ROPC_UNIT(x)		(0x300000 + ((x) * TME_SUNCG2_SIZ_REG_PAGE))
#define TME_SUNCG2_REG_ROPC_UNIT_PRIME(x)	(TME_SUNCG2_REG_ROPC_UNIT(x) + 0x000800)
#define TME_SUNCG2_REG_CSR			(0x309000)
#define TME_SUNCG2_REG_PLANE_MASK		(0x30a000)
#define TME_SUNCG2_REG_SUN2_PAN_HI		(0x30b000)
#define TME_SUNCG2_REG_SUN3_DOUBLE_BUF		(0x30b000)
#define TME_SUNCG2_REG_SUN2_ZOOM		(0x30c000)
#define TME_SUNCG2_REG_SUN3_DMA_BASE		(0x30c000)
#define TME_SUNCG2_REG_SUN2_PAN_LO		(0x30d000)
#define TME_SUNCG2_REG_SUN3_DMA_WIDTH		(0x30d000)
#define TME_SUNCG2_REG_SUN2_ZOOM_VAR		(0x30e000)
#define TME_SUNCG2_REG_SUN3_FRAME_COUNT		(0x30e000)
#define TME_SUNCG2_REG_INTVEC			(0x30f000)
#define TME_SUNCG2_REG_CMAP_R			(0x310000)
#define TME_SUNCG2_REG_CMAP_G			(0x310200)
#define TME_SUNCG2_REG_CMAP_B			(0x310400)
#define TME_SUNCG2_SIZ_REGS			(0x310600)

/* the bits in the Control/Status register: */
#define TME_SUNCG2_CSR_ENABLE_VIDEO		(0x0001)
#define TME_SUNCG2_CSR_CMAP_UPDATE		(0x0002)
#define TME_SUNCG2_CSR_INT_ENABLE		(0x0004)
#define TME_SUNCG2_CSR_ROP_MODE_MASK		(0x0038)
#define  TME_SUNCG2_CSR_ROP_MODE_PRWWRD		 (0x0000)
#define  TME_SUNCG2_CSR_ROP_MODE_SRWPIX		 (0x0008)
#define  TME_SUNCG2_CSR_ROP_MODE_PWWWRD		 (0x0010)
#define  TME_SUNCG2_CSR_ROP_MODE_SWWPIX		 (0x0018)
#define  TME_SUNCG2_CSR_ROP_MODE_PRRWRD		 (0x0020)
#define  TME_SUNCG2_CSR_ROP_MODE_PRWPIX		 (0x0028)
#define  TME_SUNCG2_CSR_ROP_MODE_PWRWRD		 (0x0030)
#define  TME_SUNCG2_CSR_ROP_MODE_PWWPIX		 (0x0038)
#define TME_SUNCG2_CSR_INT_ACTIVE		(0x0040)
#define TME_SUNCG2_CSR_RETRACE			(0x0080)
#define TME_SUNCG2_CSR_SIZE_MASK_SUN2		(0x0f00)
#define TME_SUNCG2_CSR_SIZE_MASK_SUN3		(0x0100)
#define  TME_SUNCG2_CSR_SIZE_1152_900		 (0x0000)
#define  TME_SUNCG2_CSR_SIZE_1024_1024	 	 (0x0100)

/* the raster ops: */
#define TME_SUNCG2_ROP_SRC			(0xcc)
#define TME_SUNCG2_ROP_DST			(0xaa)
#define TME_SUNCG2_ROP_NOT(x)			((x) ^ 0xff)

/* the callout flags: */
#define TME_SUNCG2_CALLOUT_CHECK		(0)
#define TME_SUNCG2_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SUNCG2_CALLOUTS_MASK		(-2)
#define  TME_SUNCG2_CALLOUT_MODE_CHANGE		TME_BIT(1)
#define	 TME_SUNCG2_CALLOUT_INT			TME_BIT(2)

/* state flags: */
#define TME_SUNCG2_FLAG_INVALID_DISPLAYED	TME_BIT(0)
#define TME_SUNCG2_FLAG_INVALID_PIXMAP		TME_BIT(1)
#define TME_SUNCG2_FLAG_INVALID_BITMAPS		TME_BIT(2)
#define TME_SUNCG2_FLAG_CALLOUT_THREAD_RUNNING	TME_BIT(3)

/* structures: */

/* the card: */
struct tme_suncg2 {

  /* our simple bus device header: */
  struct tme_bus_device tme_suncg2_device;
#define tme_suncg2_element tme_suncg2_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_suncg2_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_suncg2_rwlock;

  /* the framebuffer connection: */
  struct tme_fb_connection *tme_suncg2_fb_connection;

  /* the callout flags: */
  int tme_suncg2_callout_flags;

  /* the type of the cgtwo: */
  tme_uint32_t tme_suncg2_type;

  /* the size of the bwtwo: */
  tme_uint32_t tme_suncg2_size;

  /* the number of displayed pixels: */
  tme_uint32_t tme_suncg2_pixel_count;

  /* the raw memory: */
  tme_uint8_t *tme_suncg2_raw_memory;

  /* the displayed memory: */
  tme_uint8_t *tme_suncg2_displayed_memory;

  /* the rasterop registers: */
  struct {
    tme_uint16_t tme_suncg2_ropc_dest;
    tme_uint16_t tme_suncg2_ropc_source1;
    tme_uint16_t tme_suncg2_ropc_source2;
    tme_uint16_t tme_suncg2_ropc_pattern;
    tme_uint16_t tme_suncg2_ropc_mask1;
    tme_uint16_t tme_suncg2_ropc_mask2;
    tme_uint16_t tme_suncg2_ropc_ena_shift;
    tme_uint16_t tme_suncg2_ropc_op;
    tme_uint16_t tme_suncg2_ropc_width;
    tme_uint16_t tme_suncg2_ropc_opcount;
    tme_uint16_t tme_suncg2_ropc_decoderout;
    tme_uint16_t tme_suncg2_ropc_x11;
    tme_uint16_t tme_suncg2_ropc_x12;
    tme_uint16_t tme_suncg2_ropc_x13;
    tme_uint16_t tme_suncg2_ropc_x14;
    tme_uint16_t tme_suncg2_ropc_x15;
  } tme_suncg2_ropc[TME_SUNCG2_ROPC_PRIME * 2];

  /* our csr: */
  tme_uint16_t tme_suncg2_csr;

  /* our interrupt vector: */
  tme_uint16_t tme_suncg2_intvec;

  /* our plane mask: */
  tme_uint16_t tme_suncg2_plane_mask;

  /* the raw colormap: */
  tme_uint16_t tme_suncg2_cmap_raw[(1 << TME_SUNCG2_PLANE_MAX) * 3];
  
  /* the cooked colormap: */
  tme_uint8_t tme_suncg2_cmap[(1 << TME_SUNCG2_PLANE_MAX) * 3];

  /* if this is zero, the next CSR read will have the retrace bit set: */
  unsigned int tme_suncg2_cycle_retrace;

  /* if this is TME_SUNCG2_PLANE_MAX, we are displaying the pixmap,
     else we are displaying this plane's bitmap: */
  unsigned int tme_suncg2_bitmap_mode_plane;

  /* state flags: */
  unsigned int tme_suncg2_flags;

  /* any outstanding TLBs: */
  struct tme_token *tme_suncg2_tlb_tokens[4];
  unsigned int tme_suncg2_tlb_head;

  /* a rasterop buffered SRC value: */
  tme_uint16_t tme_suncg2_rop_src_buffer;
};

/* globals: */

#ifndef TME_NO_LOG
static const char *_tme_suncg2_ropc_regs[] = 
  { "dest",
    "source1",
    "source2",
    "pattern",
    "mask1",
    "mask2",
    "ena_shift",
    "op",
    "width",
    "opcount",
    "decoderout",
    "x11",
    "x12",
    "x13",
    "x14",
    "x15",
  };
#endif /* !TME_NO_LOG */    

/* this invalidates any outstanding TLBs: */
static void
_tme_suncg2_tlb_invalidate(struct tme_suncg2 *suncg2, struct tme_bus_tlb *tlb_valid)
{
  unsigned int tlb_i;
  struct tme_token *token_valid;
  struct tme_token *token;

  token_valid = NULL;
  if (tlb_valid != NULL) {
    token_valid = tlb_valid->tme_bus_tlb_token;
  }

  for (tlb_i = 0; tlb_i < TME_ARRAY_ELS(suncg2->tme_suncg2_tlb_tokens); tlb_i++) {
    token = suncg2->tme_suncg2_tlb_tokens[tlb_i];
    suncg2->tme_suncg2_tlb_tokens[tlb_i] = NULL;
    if (token != NULL
	&& token != token_valid) {
      tme_token_invalidate(token);
    }
  }
}

/* this adds an outstanding TLB: */
static void
_tme_suncg2_tlb_add(struct tme_suncg2 *suncg2, struct tme_bus_tlb *tlb_valid)
{
  struct tme_token *token_valid;
  struct tme_token *token;

  token_valid = tlb_valid->tme_bus_tlb_token;

  token = suncg2->tme_suncg2_tlb_tokens[suncg2->tme_suncg2_tlb_head % TME_ARRAY_ELS(suncg2->tme_suncg2_tlb_tokens)];
  if (token != NULL
      && token != token_valid) {
    tme_token_invalidate(token);
  }
  suncg2->tme_suncg2_tlb_tokens[suncg2->tme_suncg2_tlb_head % TME_ARRAY_ELS(suncg2->tme_suncg2_tlb_tokens)] = token_valid;
  suncg2->tme_suncg2_tlb_head++;
}

/* this validates the bitmaps: */
static void
_tme_suncg2_validate_bitmaps(struct tme_suncg2 *suncg2, struct tme_bus_tlb *tlb)
{
  const tme_uint32_t *pixmap_pointer;
  tme_uint32_t pixmap;
  tme_uint32_t pixmap_resid;
  tme_uint32_t bitmaps_lo;
  tme_uint32_t bitmaps_hi;
  tme_uint8_t *bitmap_pointer;

  /* if the bitmaps are invalid: */
  if (suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_BITMAPS) {

    /* the pixmap must not be invalid: */
    assert (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_PIXMAP));

    /* invalidate any outstanding TLBs: */
    _tme_suncg2_tlb_invalidate(suncg2, tlb);

    /* if we are displaying the pixmap: */
    if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

      /* if the displayed memory is not invalid: */
      if (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED)) {

	/* copy the displayed memory back into the pixmap: */
	memcpy((suncg2->tme_suncg2_raw_memory
		+ TME_SUNCG2_REG_PIXMAP),
	       suncg2->tme_suncg2_displayed_memory,
	       TME_SUNCG2_SIZ_PIXMAP);
      }
    }

    /* otherwise, we're displaying a bitmap: */
    else {

      /* the displayed memory must be invalid: */
      assert (suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED);
    }

    /* start in the pixmap: */
    pixmap_pointer = (tme_uint32_t *) (suncg2->tme_suncg2_raw_memory + TME_SUNCG2_REG_PIXMAP + TME_SUNCG2_SIZ_PIXMAP);
    pixmap_resid = TME_SUNCG2_SIZ_PIXMAP;
    pixmap = 0;
  
    /* start in the bitmaps: */
    bitmap_pointer = suncg2->tme_suncg2_raw_memory + TME_SUNCG2_REG_BITMAP(0) + TME_SUNCG2_SIZ_BITMAP;
    bitmaps_lo = 0;
    bitmaps_hi = 0;

    /* do the translation: */
    do {

      /* if the pixmap data is empty, reload it: */
      if (__tme_predict_false((pixmap_resid % sizeof(pixmap)) == 0)) {
	pixmap = *(--pixmap_pointer);
	pixmap = tme_betoh_u32(pixmap);
      }

      /* translate another pixel's bits into the bitmaps: */
      bitmaps_lo >>= 1;
      if (pixmap & TME_BIT(0)) bitmaps_lo |= 0x00000080;
      if (pixmap & TME_BIT(1)) bitmaps_lo |= 0x00008000;
      if (pixmap & TME_BIT(2)) bitmaps_lo |= 0x00800000;
      if (pixmap & TME_BIT(3)) bitmaps_lo |= 0x80000000;
      bitmaps_hi >>= 1;
      if (pixmap & TME_BIT(4)) bitmaps_hi |= 0x00000080;
      if (pixmap & TME_BIT(5)) bitmaps_hi |= 0x00008000;
      if (pixmap & TME_BIT(6)) bitmaps_hi |= 0x00800000;
      if (pixmap & TME_BIT(7)) bitmaps_hi |= 0x80000000;

      /* we have translated another pixel: */
      pixmap >>= 8;
      pixmap_resid--;

      /* after every eight pixels, we have another eight bytes of bitmap
	 data to write out: */
      if (__tme_predict_false((pixmap_resid % 8) == 0)) {
	bitmap_pointer--;
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 0] = TME_FIELD_MASK_EXTRACTU(bitmaps_lo, 0x000000ff);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 1] = TME_FIELD_MASK_EXTRACTU(bitmaps_lo, 0x0000ff00);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 2] = TME_FIELD_MASK_EXTRACTU(bitmaps_lo, 0x00ff0000);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 3] = TME_FIELD_MASK_EXTRACTU(bitmaps_lo, 0xff000000);
	bitmaps_lo = 0;
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 4] = TME_FIELD_MASK_EXTRACTU(bitmaps_hi, 0x000000ff);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 5] = TME_FIELD_MASK_EXTRACTU(bitmaps_hi, 0x0000ff00);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 6] = TME_FIELD_MASK_EXTRACTU(bitmaps_hi, 0x00ff0000);
	bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 7] = TME_FIELD_MASK_EXTRACTU(bitmaps_hi, 0xff000000);
	bitmaps_hi = 0;
      }

      /* loop while we have pixmap bytes remaining: */
    } while (pixmap_resid > 0);

    /* the bitmaps are no longer invalid: */
    suncg2->tme_suncg2_flags &= ~TME_SUNCG2_FLAG_INVALID_BITMAPS;
  }

  /* otherwise, the bitmaps are valid: */
  else {

    /* if our caller needs the actual raw bitmap memory valid
       (indicated by tlb == NULL): */
    if (tlb == NULL) {

      /* invalidate any outstanding TLBs: */
      _tme_suncg2_tlb_invalidate(suncg2, NULL);

      /* if we're displaying a bitmap: */
      if (suncg2->tme_suncg2_bitmap_mode_plane != TME_SUNCG2_PLANE_MAX) {

	/* if the displayed memory is not invalid: */
	if (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED)) {

	  /* copy the displayed memory back into the bitmap: */
	  memcpy((suncg2->tme_suncg2_raw_memory
		  + TME_SUNCG2_REG_BITMAP(suncg2->tme_suncg2_bitmap_mode_plane)),
		 suncg2->tme_suncg2_displayed_memory,
		 TME_SUNCG2_SIZ_BITMAP);
	}
      }
    }
  }
}

/* this validates the pixmap: */
static void
_tme_suncg2_validate_pixmap(struct tme_suncg2 *suncg2, struct tme_bus_tlb *tlb)
{
  tme_uint32_t *pixmap_pointer;
  tme_uint32_t pixmap;
  tme_uint32_t pixmap_resid;
  tme_uint32_t bitmaps_lo;
  tme_uint32_t bitmaps_hi;
  const tme_uint8_t *bitmap_pointer;

  /* if the pixmap is invalid: */
  if (suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_PIXMAP) {

    /* the bitmaps must not be invalid: */
    assert (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_BITMAPS));

    /* invalidate any outstanding TLBs: */
    _tme_suncg2_tlb_invalidate(suncg2, tlb);

    /* if we are displaying a bitmap: */
    if (suncg2->tme_suncg2_bitmap_mode_plane != TME_SUNCG2_PLANE_MAX) {

      /* if the displayed memory is not invalid: */
      if (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED)) {

	/* copy the displayed memory back into the bitmap: */
	memcpy((suncg2->tme_suncg2_raw_memory
		+ TME_SUNCG2_REG_BITMAP(suncg2->tme_suncg2_bitmap_mode_plane)),
	       suncg2->tme_suncg2_displayed_memory,
	       TME_SUNCG2_SIZ_BITMAP);
      }
    }

    /* otherwise, we're displaying the pixmap: */
    else {

      /* the displayed memory must be invalid: */
      assert (suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED);
    }

    /* start in the pixmap: */
    pixmap_pointer = (tme_uint32_t *) (suncg2->tme_suncg2_raw_memory + TME_SUNCG2_REG_PIXMAP);
    pixmap_resid = TME_SUNCG2_SIZ_PIXMAP;
    pixmap = 0;
  
    /* start in the bitmaps: */
    bitmap_pointer = suncg2->tme_suncg2_raw_memory + TME_SUNCG2_REG_BITMAP(0);
    bitmaps_lo = 0;
    bitmaps_hi = 0;

    /* do the translation: */
    do {

      /* if the bitmap data is empty, reload it: */
      if (__tme_predict_false((pixmap_resid % 8) == 0)) {
	bitmaps_lo
	  = ((((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 0]) << 0)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 1]) << 8)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 2]) << 16)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 3]) << 24));
	bitmaps_hi
	  = ((((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 4]) << 0)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 5]) << 8)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 6]) << 16)
	     | (((tme_uint32_t) bitmap_pointer[TME_SUNCG2_SIZ_BITMAP * 7]) << 24));
	bitmap_pointer++;
      }

      /* we are about to translate another pixel: */
      pixmap <<= 8;
      pixmap_resid--;

      /* translate another pixel's bits from the bitmaps: */
      if (bitmaps_lo & 0x00000080) pixmap |= TME_BIT(0);
      if (bitmaps_lo & 0x00008000) pixmap |= TME_BIT(1);
      if (bitmaps_lo & 0x00800000) pixmap |= TME_BIT(2);
      if (bitmaps_lo & 0x80000000) pixmap |= TME_BIT(3);
      bitmaps_lo <<= 1;
      if (bitmaps_hi & 0x00000080) pixmap |= TME_BIT(4);
      if (bitmaps_hi & 0x00008000) pixmap |= TME_BIT(5);
      if (bitmaps_hi & 0x00800000) pixmap |= TME_BIT(6);
      if (bitmaps_hi & 0x80000000) pixmap |= TME_BIT(7);
      bitmaps_hi <<= 1;
    
      /* if the pixmap data is full, write it: */
      if (__tme_predict_false((pixmap_resid % sizeof(pixmap)) == 0)) {
	*(pixmap_pointer++) = tme_htobe_u32(pixmap);
	pixmap = 0;
      }

      /* loop while we have pixmap bytes remaining: */
    } while (pixmap_resid > 0);

    /* the pixmap is no longer invalid, but the bitmaps are: */
    suncg2->tme_suncg2_flags &= ~TME_SUNCG2_FLAG_INVALID_PIXMAP;
  }

  /* otherwise, the pixmap is valid: */
  else {

    /* if our caller needs the actual raw pixmap memory valid
       (indicated by tlb == NULL): */
    if (tlb == NULL) {

      /* invalidate any outstanding TLBs: */
      _tme_suncg2_tlb_invalidate(suncg2, NULL);

      /* if we're displaying the pixmap: */
      if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

	/* if the displayed memory is not invalid: */
	if (!(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED)) {

	  /* copy the displayed memory back into the pixmap: */
	  memcpy((suncg2->tme_suncg2_raw_memory
		  + TME_SUNCG2_REG_PIXMAP),
		 suncg2->tme_suncg2_displayed_memory,
		 TME_SUNCG2_SIZ_PIXMAP);
	}
      }
    }
  }
}

/* this validates the displayed memory: */
static void
_tme_suncg2_validate_displayed(struct tme_suncg2 *suncg2, struct tme_bus_tlb *tlb)
{

  /* if the displayed memory is invalid: */
  if (suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_INVALID_DISPLAYED) {

    /* if we're displaying the pixmap: */
    if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

      /* validate the pixmap: */
      _tme_suncg2_validate_pixmap(suncg2, tlb);

      /* copy the pixmap into the displayed memory: */
      memcpy(suncg2->tme_suncg2_displayed_memory,
	     (suncg2->tme_suncg2_raw_memory
	      + TME_SUNCG2_REG_PIXMAP),
	     TME_SUNCG2_SIZ_PIXMAP);
    }

    /* otherwise, we're displaying a bitmap: */
    else {

      /* validate the bitmaps: */
      _tme_suncg2_validate_bitmaps(suncg2, tlb);

      /* copy the bitmap into the displayed memory: */
      memcpy(suncg2->tme_suncg2_displayed_memory,
	     (suncg2->tme_suncg2_raw_memory
	      + TME_SUNCG2_REG_BITMAP(suncg2->tme_suncg2_bitmap_mode_plane)),
	     TME_SUNCG2_SIZ_BITMAP);
    }

    /* the displayed memory is no longer invalid: */
    suncg2->tme_suncg2_flags &= ~TME_SUNCG2_FLAG_INVALID_DISPLAYED;
  }
}

/* this handles a mode change callout: */
static int
_tme_suncg2_mode_change(struct tme_suncg2 *suncg2)
{
  struct tme_fb_connection *conn_fb_other;
  struct tme_fb_connection *conn_fb;
  unsigned int pixel_i;
  unsigned int mono_mask;
  tme_uint32_t color0;
  tme_uint32_t color1;
  tme_uint32_t color;
  unsigned int bitmap_mode_plane;
  unsigned int color_i;
  int rc;

  /* this makes a color for a pixel: */
#define _TME_SUNCG2_PIXEL_COLOR(pixel)					\
  ((TME_SUNCG2_CMAP_VALUE(suncg2, pixel, TME_SUNCG2_REG_CMAP_G) << 0)	\
   | (TME_SUNCG2_CMAP_VALUE(suncg2, pixel, TME_SUNCG2_REG_CMAP_R) << 8)	\
   | (TME_SUNCG2_CMAP_VALUE(suncg2, pixel, TME_SUNCG2_REG_CMAP_B) << 16))

  /* if we are to display a bitmap, the color for pixmap pixel zero
     must be the color for bitmap pixel zero, and the color for pixmap
     pixel 255 must be the color for bitmap pixel one: */
  color0 = _TME_SUNCG2_PIXEL_COLOR(0);
  color1 = _TME_SUNCG2_PIXEL_COLOR(255);

  /* we don't know what the monochrome mask is yet: */
  mono_mask = 0xff;

  /* loop over the other pixels: */
  for (pixel_i = 1; pixel_i < 255; pixel_i++) {

    /* get the color for this pixel: */
    color = _TME_SUNCG2_PIXEL_COLOR(pixel_i);

    /* if this matches color0: */
    if (color == color0) {

      /* any set bits in this pixel number can't be in the monochrome
	 mask: */
      mono_mask &= ~pixel_i;
    }

    /* else, if this matches color1: */
    else if (color == color1) {

      /* any clear bits in this pixel number can't be in the
	 monochrome mask: */
      mono_mask &= pixel_i;
    }

    /* otherwise, this is some other color: */
    else {

      /* we can't be in bitmap mode: */
      mono_mask = 0;
      break;
    }
  }

  /* if the monochrome mask is zero, we must display the pixmap, else
     we display the bitmap corresponding to the least significant set
     bit in the monochrome mask: */
  if (mono_mask == 0
      || (mono_mask & (mono_mask - 1)) != 0) {
    bitmap_mode_plane = TME_SUNCG2_PLANE_MAX;
  }
  else {
    for (bitmap_mode_plane = 0;
	 (mono_mask & 1) == 0;
	 bitmap_mode_plane++, mono_mask >>= 1);
  }

#undef _TME_SUNCG2_PIXEL_COLOR

  /* get both sides of the framebuffer connection: */
  conn_fb_other = suncg2->tme_suncg2_fb_connection;
  conn_fb = (struct tme_fb_connection *) conn_fb_other->tme_fb_connection.tme_connection_other;

  /* if we are displaying the pixmap: */
  if (bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

    /* the pixmap is eight bits deep: */
    conn_fb->tme_fb_connection_depth = TME_SUNCG2_PLANE_MAX;
    conn_fb->tme_fb_connection_bits_per_pixel = TME_SUNCG2_PLANE_MAX;
    
    /* recook the colormap: */
    for (color_i = 0;
	 color_i < TME_ARRAY_ELS(suncg2->tme_suncg2_cmap_raw);
	 color_i++) {
      suncg2->tme_suncg2_cmap[color_i] = tme_betoh_u16(suncg2->tme_suncg2_cmap_raw[color_i]);
    }
  }

  /* otherwise, we can display a bitmap: */
  else {

    /* a bitmap is one bit deep: */
    conn_fb->tme_fb_connection_depth = 1;
    conn_fb->tme_fb_connection_bits_per_pixel = 1;

    /* cook the monochrome colormap: */
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_G)] = TME_SUNCG2_CMAP_VALUE(suncg2, 0, TME_SUNCG2_REG_CMAP_G);
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_R)] = TME_SUNCG2_CMAP_VALUE(suncg2, 0, TME_SUNCG2_REG_CMAP_R);
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_B)] = TME_SUNCG2_CMAP_VALUE(suncg2, 0, TME_SUNCG2_REG_CMAP_B);
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(1, TME_SUNCG2_REG_CMAP_G)] = TME_SUNCG2_CMAP_VALUE(suncg2, 255, TME_SUNCG2_REG_CMAP_G);
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(1, TME_SUNCG2_REG_CMAP_R)] = TME_SUNCG2_CMAP_VALUE(suncg2, 255, TME_SUNCG2_REG_CMAP_R);
    suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(1, TME_SUNCG2_REG_CMAP_B)] = TME_SUNCG2_CMAP_VALUE(suncg2, 255, TME_SUNCG2_REG_CMAP_B);
  }

  /* if the display is changing: */
  if (bitmap_mode_plane != suncg2->tme_suncg2_bitmap_mode_plane) {

    /* log the change: */
    tme_log(&suncg2->tme_suncg2_element->tme_element_log_handle,
	    100, TME_OK,
	    (&suncg2->tme_suncg2_element->tme_element_log_handle,
	     "display changing from plane %u to plane %u",
	     suncg2->tme_suncg2_bitmap_mode_plane,
	     bitmap_mode_plane));

    /* if we were displaying the pixmap, validate the pixmap, else
       validate the bitmaps, to copy the current displayed memory back
       into the raw memory: */
    if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {
      _tme_suncg2_validate_pixmap(suncg2, NULL);
    }
    else {
      _tme_suncg2_validate_bitmaps(suncg2, NULL);
    }

    /* invalidate any outstanding TLBs: */
    _tme_suncg2_tlb_invalidate(suncg2, NULL);

    /* set the display: */
    suncg2->tme_suncg2_bitmap_mode_plane = bitmap_mode_plane;

    /* free any previously allocated memory: */
    if (suncg2->tme_suncg2_displayed_memory != NULL) {
      tme_free(suncg2->tme_suncg2_displayed_memory);
    }

    /* allocate memory for our framebuffer connection: */
    rc = tme_fb_xlat_alloc_src(conn_fb);
    assert (rc == TME_OK);
    suncg2->tme_suncg2_displayed_memory = conn_fb->tme_fb_connection_buffer;

    /* the displayed memory is now invalid: */
    suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_DISPLAYED;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);
      
  /* do the callout: */
  rc = (conn_fb_other != NULL
	? ((*conn_fb_other->tme_fb_connection_mode_change)
	   (conn_fb_other))
	: TME_OK);
      
  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  return (rc);
}

/* the suncg2 callout function.  it must be called with the mutex locked: */
static void
_tme_suncg2_callout(struct tme_suncg2 *suncg2, int new_callouts)
{
  int callouts, later_callouts;
  int rc;

  /* add in any new callouts: */
  suncg2->tme_suncg2_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (suncg2->tme_suncg2_callout_flags
      & TME_SUNCG2_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  suncg2->tme_suncg2_callout_flags
    |= TME_SUNCG2_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = suncg2->tme_suncg2_callout_flags)
	  & TME_SUNCG2_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    suncg2->tme_suncg2_callout_flags
      = (callouts
	 & ~TME_SUNCG2_CALLOUTS_MASK);
    callouts
      &= TME_SUNCG2_CALLOUTS_MASK;

    /* if we need a mode change: */
    if (callouts & TME_SUNCG2_CALLOUT_MODE_CHANGE) {

      /* call out the mode change: */
      rc = _tme_suncg2_mode_change(suncg2);

      /* if the callout failed: */
      if (rc != TME_OK) {

	/* remember that this callout should be attempted again at
           some later time: */
	later_callouts |= TME_SUNCG2_CALLOUT_MODE_CHANGE;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  suncg2->tme_suncg2_callout_flags = later_callouts;
}

/* the callout thread: */
static void
_tme_suncg2_callout_thread(void *_suncg2)
{
  struct tme_suncg2 *suncg2;

  /* recover our data structure: */
  suncg2 = _suncg2;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* the callout thread is no longer running: */
  suncg2->tme_suncg2_flags &= ~TME_SUNCG2_FLAG_CALLOUT_THREAD_RUNNING;

  /* make any callouts: */
  _tme_suncg2_callout(suncg2, 0);

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);
}

/* this is called before the framebuffer's display is updated: */
static int
_tme_suncg2_update(struct tme_fb_connection *conn_fb)
{
  struct tme_suncg2 *suncg2;

  /* recover our data structure: */
  suncg2 = conn_fb->tme_fb_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* validate the displayed memory: */
  _tme_suncg2_validate_displayed(suncg2, NULL);

  /* if we still need callouts, and the callout thread isn't running,
     start it: */
  if ((suncg2->tme_suncg2_callout_flags & TME_SUNCG2_CALLOUTS_MASK) != 0
      && !(suncg2->tme_suncg2_flags & TME_SUNCG2_FLAG_CALLOUT_THREAD_RUNNING)) {
    tme_thread_create(_tme_suncg2_callout_thread, suncg2);
    suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_CALLOUT_THREAD_RUNNING;
  }
      
  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  return (TME_OK);
}

/* the suncg2 displayed memory bus cycle handler: */
static int
_tme_suncg2_bus_cycle_displayed(void *_suncg2, struct tme_bus_cycle *cycle_init)
{
  struct tme_suncg2 *suncg2;
  unsigned int plane_i;
  tme_bus_addr32_t address_first;
  tme_bus_addr32_t address_last;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* if we're displaying the pixmap: */
  plane_i = suncg2->tme_suncg2_bitmap_mode_plane;
  if (plane_i == TME_SUNCG2_PLANE_MAX) {

    /* the displayed memory is the pixmap memory: */
    address_first = TME_SUNCG2_REG_PIXMAP;
    address_last = TME_SUNCG2_REG_PIXMAP + suncg2->tme_suncg2_pixel_count - 1;
  }

  /* otherwise, we're in bitmap mode: */
  else {

    /* the displayed memory is the displayed bitmap memory: */
    address_first = TME_SUNCG2_REG_BITMAP(plane_i);
    address_last = address_first + (suncg2->tme_suncg2_pixel_count / 8) - 1;
  }

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (suncg2->tme_suncg2_displayed_memory
			     - address_first),
			    address_last);

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the suncg2 raw memory bus cycle handler: */
static int
_tme_suncg2_bus_cycle_raw(void *_suncg2, struct tme_bus_cycle *cycle_init)
{
  struct tme_suncg2 *suncg2;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (suncg2->tme_suncg2_raw_memory
			     - TME_SUNCG2_REG_BITMAP(0)),
			    (TME_SUNCG2_REG_PIXMAP
			     + TME_SUNCG2_SIZ_PIXMAP
			     - 1));

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this catches unsupported rasterop configurations: */
static void
_tme_suncg2_rop_unsupported(struct tme_suncg2 *suncg2)
{
  /* nothing */
}

/* this does a raster op: */
static tme_uint16_t 
_tme_suncg2_rop_op(struct tme_suncg2 *suncg2,
		   unsigned int ropc_unit,
		   tme_uint16_t src,
		   tme_uint16_t dst)
{
  switch ((tme_uint8_t) suncg2->tme_suncg2_ropc[ropc_unit].tme_suncg2_ropc_op) {
  default:
    _tme_suncg2_rop_unsupported(suncg2); 
    /* FALLTHROUGH */
  case (TME_SUNCG2_ROP_SRC): return (src);
  case (TME_SUNCG2_ROP_NOT(TME_SUNCG2_ROP_DST)): return (~dst);
  }
}

/* the bus cycle handler for the rasterop data: */
static int
_tme_suncg2_bus_cycle_rop_data(void *_suncg2, struct tme_bus_cycle *cycle_init)
{
  struct tme_suncg2 *suncg2;
  tme_uint32_t address;
  tme_uint32_t address_aligned;
  tme_uint16_t data;
  tme_uint8_t src;
  tme_uint8_t dst;
  tme_uint8_t pixel;
  int supported;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* decode the address: */
  address
    = (cycle_init->tme_bus_cycle_address
       - TME_SUNCG2_REG_ROP_DATA);
  address_aligned
    = (address & (((tme_uint32_t) 0) - sizeof(tme_uint16_t)));

  /* assume that this access is unsupported: */
  supported = FALSE;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* if this is a read: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {

    /* dispatch on the rop mode: */
    switch (suncg2->tme_suncg2_csr & TME_SUNCG2_CSR_ROP_MODE_MASK) {

    default:
      _tme_suncg2_rop_unsupported(suncg2);
      data = 0;
      break;

      /* the single pixel, LD_SRC write, LD_DST write, mode: */
    case TME_SUNCG2_CSR_ROP_MODE_SWWPIX:
      
      /* this mode has partial support: */
      supported = TRUE;

      /* XXX FIXME does a load reset the state? */
      
      /* just load two pixels from the pixmap: */
      _tme_suncg2_validate_pixmap(suncg2, NULL);
      data = (suncg2->tme_suncg2_raw_memory[TME_SUNCG2_REG_PIXMAP + address_aligned]
	      & ((tme_uint8_t) suncg2->tme_suncg2_plane_mask));
      data = ((data << 8)
	      | (suncg2->tme_suncg2_raw_memory[TME_SUNCG2_REG_PIXMAP + address_aligned + 1]
		 & ((tme_uint8_t) suncg2->tme_suncg2_plane_mask)));
      break;
    }
  }

  /* do the bus cycle: */
  tme_bus_cycle_xfer_reg(cycle_init,
			 &data,
			 TME_BUS16_LOG2);

  /* get the data: */
  data = (((cycle_init->tme_bus_cycle_size == sizeof(tme_uint16_t)
	    || (address % sizeof(tme_uint16_t)) == 1)
	   ? data
	   : (data >> 8))
	  & (0xffff >> (sizeof(tme_uint16_t) - cycle_init->tme_bus_cycle_size)));

  /* log the cycle: */
  tme_log(&suncg2->tme_suncg2_element->tme_element_log_handle,
	  100, TME_OK,
	  (&suncg2->tme_suncg2_element->tme_element_log_handle,
	   ((cycle_init->tme_bus_cycle_size == sizeof(tme_uint16_t))
	    ? "rop data offset 0x%05x size 16bits %s 0x%04x"
	    : "rop data offset 0x%05x size  8bits %s 0x%02x"),
	   address,
	   ((cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE)
	    ? "<-"
	    : "->"),
	   data));

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* dispatch on the rop mode: */
    switch (suncg2->tme_suncg2_csr & TME_SUNCG2_CSR_ROP_MODE_MASK) {

    default:
      _tme_suncg2_rop_unsupported(suncg2);
      break;

      /* the single pixel, LD_SRC write, LD_DST write, mode: */
    case TME_SUNCG2_CSR_ROP_MODE_SWWPIX:
      
      /* this mode has partial support: */
      supported = TRUE;

      /* validate the pixmap: */
      _tme_suncg2_validate_pixmap(suncg2, NULL);

      /* the current pixel value is DST: */
      dst = suncg2->tme_suncg2_raw_memory[TME_SUNCG2_REG_PIXMAP + address];
      
      /* the current source buffer is SRC: */
      src = suncg2->tme_suncg2_rop_src_buffer;

      /* make the new pixel value: */
      pixel = _tme_suncg2_rop_op(suncg2,
				 8,
				 src,
				 dst);

      /* store the pixel: */
      suncg2->tme_suncg2_raw_memory[TME_SUNCG2_REG_PIXMAP + address]
	= ((dst & ((tme_uint8_t) (~suncg2->tme_suncg2_plane_mask)))
	   | (pixel & ((tme_uint8_t) suncg2->tme_suncg2_plane_mask)));

      /* the displayed memory is now invalid: */
      suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_DISPLAYED;

      /* load the source buffer: */
      suncg2->tme_suncg2_rop_src_buffer = data;
      break;
    }
  }

  /* if this cycle was unsupported, abort: */
  if (__tme_predict_false(!supported)) {
    _tme_suncg2_rop_unsupported(suncg2);
  }
  
  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the bus cycle handler for the registers: */
static int
_tme_suncg2_bus_cycle_regs(void *_suncg2, struct tme_bus_cycle *cycle_init)
{
  struct tme_suncg2 *suncg2;
  tme_bus_addr32_t address;
  tme_uint16_t *reg;
  tme_uint16_t reg_old, reg_new;
  tme_uint16_t junk;
  unsigned int ropc_unit;
  unsigned int ropc_unit_prime;
  unsigned int ropc_reg;
  int new_callouts;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* coarsely decode the address: */
  address
    = (cycle_init->tme_bus_cycle_address
       & (((tme_bus_addr32_t) 0)
	  - TME_SUNCG2_SIZ_REG_PAGE));

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* the rasterop registers go from [TME_SUNCG2_REG_ROPC_UNIT(0)..TME_SUNCG2_REG_CSR): */
  assert (address >= TME_SUNCG2_REG_ROPC_UNIT(0));
  if (address < TME_SUNCG2_REG_CSR) {

    /* get the rasterop unit number: */
    ropc_unit = (address - TME_SUNCG2_REG_ROPC_UNIT(0)) / TME_SUNCG2_SIZ_REG_PAGE;

    /* see if this is the prime registers: */
    ropc_unit_prime
      = ((cycle_init->tme_bus_cycle_address & (TME_SUNCG2_SIZ_REG_PAGE / 2))
	 ? TME_SUNCG2_ROPC_PRIME
	 : 0);

    /* get the register number: */
    ropc_reg
      = ((cycle_init->tme_bus_cycle_address
	  % sizeof(suncg2->tme_suncg2_ropc[ropc_unit_prime + ropc_unit]))
	 / sizeof(tme_uint16_t));

    /* get a pointer to the single register: */
    reg = (((tme_uint16_t *) &suncg2->tme_suncg2_ropc[ropc_unit_prime + ropc_unit]) + ropc_reg);

    /* do the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init,
			   reg,
			   TME_BUS16_LOG2);

#ifndef TME_NO_LOG
    /* log the transfer: */
    tme_log(&suncg2->tme_suncg2_element->tme_element_log_handle,
	    100, TME_OK,
	    (&suncg2->tme_suncg2_element->tme_element_log_handle,
	     "ropc unit %u%s reg %s %s 0x%04x",
	     ropc_unit,
	     (ropc_unit_prime
	      ? " PRIME"
	      : ""),
	     _tme_suncg2_ropc_regs[ropc_reg],
	     ((cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE)
	      ? "<-"
	      : "->"),
	     *reg));
#endif /* !TME_NO_LOG */
  }

  /* the CSR is the entire page at TME_SUNCG2_REG_CSR: */
  else if (address == TME_SUNCG2_REG_CSR) {

    /* if this is a read: */
    if ((cycle_init->tme_bus_cycle_type & TME_BUS_CYCLE_READ) != 0) {

      /* if it's time to set the retrace bit, set it: */
      if (suncg2->tme_suncg2_cycle_retrace-- == 0) {
	suncg2->tme_suncg2_csr |= TME_SUNCG2_CSR_RETRACE;
	suncg2->tme_suncg2_cycle_retrace = TME_SUNCG2_CYCLE_RETRACE;
      }

      /* otherwise, clear it: */
      else {
	suncg2->tme_suncg2_csr &= ~TME_SUNCG2_CSR_RETRACE;
      }
    }

    /* do the bus cycle: */
    reg_old = suncg2->tme_suncg2_csr;
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &suncg2->tme_suncg2_csr,
			   TME_BUS16_LOG2);
    reg_new = suncg2->tme_suncg2_csr;

    /* put back the unchanging bits: */
    reg_new
      = ((reg_new
	  & ~(TME_SUNCG2_CSR_INT_ACTIVE
	      | TME_SUNCG2_CSR_RETRACE
	      | 0xff00))
	 | (reg_old
	    & (TME_SUNCG2_CSR_INT_ACTIVE
	       | TME_SUNCG2_CSR_RETRACE
	       | 0xff00)));
    suncg2->tme_suncg2_csr = reg_new;

    /* log the transfer: */
    tme_log(&suncg2->tme_suncg2_element->tme_element_log_handle,
	    100, TME_OK,
	    (&suncg2->tme_suncg2_element->tme_element_log_handle,
	     "csr %s 0x%04x",
	     ((cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE)
	      ? "<-"
	      : "->"),
	     reg_new));

    /* we do not support these bits: */
    if ((reg_new
	 ^ reg_old)
	& TME_SUNCG2_CSR_INT_ENABLE) {
      abort();
    }

    /* if the cmap update bit is transitioning from a zero to a one,
       we need a mode change: */
    if ((reg_old & TME_SUNCG2_CSR_CMAP_UPDATE) == 0
	&& (reg_new & TME_SUNCG2_CSR_CMAP_UPDATE) != 0) {
      new_callouts |= TME_SUNCG2_CALLOUT_MODE_CHANGE;
    }
  }
  
  /* if this is the interrupt vector: */
  else if (address == TME_SUNCG2_REG_INTVEC) {
    tme_bus_cycle_xfer_reg(cycle_init,
			   &suncg2->tme_suncg2_intvec,
			   TME_BUS16_LOG2);
  }
  
  /* if this is the plane mask: */
  else if (address == TME_SUNCG2_REG_PLANE_MASK) {

    /* do the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init,
			   &suncg2->tme_suncg2_plane_mask,
			   TME_BUS16_LOG2);

    /* log the transfer: */
    tme_log(&suncg2->tme_suncg2_element->tme_element_log_handle,
	    100, TME_OK,
	    (&suncg2->tme_suncg2_element->tme_element_log_handle,
	     "plane mask %s 0x%04x",
	     ((cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE)
	      ? "<-"
	      : "->"),
	     suncg2->tme_suncg2_plane_mask));
  }

  /* XXX FIXME - the sun3 PROM zeroes the cg3 DMA base register,
     the cg3 double buffer register, the cg3 DMA width register,
     and the cg3 frame count registers: */
  else if (address == TME_SUNCG2_REG_SUN3_DMA_BASE
	   || address == TME_SUNCG2_REG_SUN3_DOUBLE_BUF
	   || address == TME_SUNCG2_REG_SUN3_DMA_WIDTH
	   || address == TME_SUNCG2_REG_SUN3_FRAME_COUNT) {
    tme_bus_cycle_xfer_reg(cycle_init,
			   &junk,
			   TME_BUS16_LOG2);
  }

  /* XXX FIXME - this is a partial implementation: */
  else {
    abort();
  }

  /* make any new callouts: */
  _tme_suncg2_callout(suncg2, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the bus cycle handler for the suncg2 colormap registers: */
static int
_tme_suncg2_bus_cycle_cmap(void *_suncg2, struct tme_bus_cycle *cycle_init)
{
  struct tme_suncg2 *suncg2;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (((tme_uint8_t *) suncg2->tme_suncg2_cmap_raw)
			     - TME_SUNCG2_REG_CMAP_R),
			    TME_SUNCG2_SIZ_REGS - 1);

  /* if this is a write and colormap updates are enabled, we need to
     call out a mode change.  we don't make the callout now, assuming
     that more writes to the colormap are coming soon.  we will
     eventually make the callout when the framebuffer updates: */
  if ((cycle_init->tme_bus_cycle_type & TME_BUS_CYCLE_WRITE)
      && (suncg2->tme_suncg2_csr
	  & TME_SUNCG2_CSR_CMAP_UPDATE)) {
    suncg2->tme_suncg2_callout_flags |= TME_SUNCG2_CALLOUT_MODE_CHANGE;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the suncg2 TLB filler: */
static int
_tme_suncg2_tlb_fill(void *_suncg2, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address_wider, unsigned int cycles)
{
  struct tme_suncg2 *suncg2;
  tme_uint8_t *memory;
  tme_bus_addr32_t address;
  tme_bus_addr32_t address_first;
  tme_bus_addr32_t address_last;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) _suncg2;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* the fast reading and writing rwlock: */
  tlb->tme_bus_tlb_rwlock = &suncg2->tme_suncg2_rwlock;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler private data: */
  tlb->tme_bus_tlb_cycle_private = _suncg2;

  /* lock the mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* we require a connection: */
  assert (suncg2->tme_suncg2_fb_connection != NULL);
  assert (suncg2->tme_suncg2_displayed_memory != NULL);

  /* if this address falls in a bitmap: */
  if ((TME_SUNCG2_REG_BITMAP(0)
       <= address)
      && (address
	  < TME_SUNCG2_REG_BITMAP(TME_SUNCG2_PLANE_MAX))) {

    /* the cycle handler: */
    tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_raw;

    /* if we're displaying the pixmap: */
    if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

      /* validate the bitmaps: */
      _tme_suncg2_validate_bitmaps(suncg2, tlb);

      /* this TLB entry covers all of the bitmap memory: */
      address_first = TME_SUNCG2_REG_BITMAP(0);
      address_last = TME_SUNCG2_REG_PIXMAP - 1;
      memory = suncg2->tme_suncg2_raw_memory + address_first;

      /* the displayed memory is now invalid: */
      suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_DISPLAYED;
    }

    /* otherwise, we're displaying a bitmap: */
    else {

      /* calculate the first and last addresses of the displayed
	 memory: */
      address_first = TME_SUNCG2_REG_BITMAP(suncg2->tme_suncg2_bitmap_mode_plane);
      address_last = address_first + (suncg2->tme_suncg2_pixel_count / 8) - 1;

      /* if this address is before the displayed memory, this TLB
	 entry covers from the beginning of the bitmap memory up to
	 the displayed memory: */
      if (address < address_first) {
	address_last = address_first - 1;
	address_first = TME_SUNCG2_REG_BITMAP(0);
	memory = suncg2->tme_suncg2_raw_memory + address_first;

	/* validate the bitmaps: */
	_tme_suncg2_validate_bitmaps(suncg2, tlb);
      }

      /* otherwise, if this address is after the displayed memory,
	 this TLB entry covers from right after the displayed memory
	 to the end of the bitmap memory: */
      else if (address > address_last) {
	address_first = address_last + 1;
	address_last = TME_SUNCG2_REG_PIXMAP - 1;
	memory = suncg2->tme_suncg2_raw_memory + address_first;

	/* validate the bitmaps: */
	_tme_suncg2_validate_bitmaps(suncg2, tlb);
      }

      /* otherwise, this address is in the displayed memory: */
      else {
	memory = suncg2->tme_suncg2_displayed_memory;
	tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_displayed;

	/* validate the displayed memory: */
	_tme_suncg2_validate_displayed(suncg2, tlb);
      }
    }

    /* the address range: */
    tlb->tme_bus_tlb_addr_first = address_first;
    tlb->tme_bus_tlb_addr_last = address_last;

    /* this TLB entry allows fast reading and fast writing: */
    tlb->tme_bus_tlb_emulator_off_read = memory - address_first;
    tlb->tme_bus_tlb_emulator_off_write = memory - address_first;

    /* add this TLB entry: */
    _tme_suncg2_tlb_add(suncg2, tlb);

    /* the pixmap is now invalid: */
    suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_PIXMAP;
  }
    
  /* if this address falls in the pixmap: */
  else if ((TME_SUNCG2_REG_PIXMAP
	    <= address)
	   && (address
	       < (TME_SUNCG2_REG_PIXMAP + TME_SUNCG2_SIZ_PIXMAP))) {

    /* the cycle handler: */
    tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_raw;

    /* if we're displaying the pixmap: */
    if (suncg2->tme_suncg2_bitmap_mode_plane == TME_SUNCG2_PLANE_MAX) {

      /* calculate the first and last addresses of the displayed memory: */
      address_first = TME_SUNCG2_REG_PIXMAP;
      address_last = TME_SUNCG2_REG_PIXMAP + suncg2->tme_suncg2_pixel_count - 1;

      /* if this address is after the displayed memory, we're in the pad: */
      if (address > address_last) {
	address_first = address_last + 1;
	address_last = TME_SUNCG2_REG_PIXMAP + TME_SUNCG2_SIZ_PIXMAP - 1;
	memory = suncg2->tme_suncg2_raw_memory + address_first;

	/* validate the pixmap: */
	_tme_suncg2_validate_pixmap(suncg2, tlb);
      }

      /* otherwise, this address is in the displayed memory: */
      else {
	memory = suncg2->tme_suncg2_displayed_memory;
	tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_displayed;

	/* validate the displayed memory: */
	_tme_suncg2_validate_displayed(suncg2, tlb);
      }
    }

    /* otherwise, we're displaying a bitmap: */
    else {

      /* validate the pixmap: */
      _tme_suncg2_validate_pixmap(suncg2, tlb);

      /* this TLB covers all of pixmap memory: */
      address_first = TME_SUNCG2_REG_PIXMAP;
      address_last = TME_SUNCG2_REG_PIXMAP + TME_SUNCG2_SIZ_PIXMAP - 1;
      memory = suncg2->tme_suncg2_raw_memory + address_first;

      /* the displayed memory is now invalid: */
      suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_DISPLAYED;
    }

    /* the address range: */
    tlb->tme_bus_tlb_addr_first = address_first;
    tlb->tme_bus_tlb_addr_last = address_last;

    /* this TLB entry allows fast reading and fast writing: */
    tlb->tme_bus_tlb_emulator_off_read = memory - address_first;
    tlb->tme_bus_tlb_emulator_off_write = memory - address_first;

    /* add this TLB entry: */
    _tme_suncg2_tlb_add(suncg2, tlb);

    /* the bitmaps are now invalid: */
    suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_BITMAPS;
  }

  /* if this address falls in the rasterop data: */
  else if ((TME_SUNCG2_REG_ROP_DATA
	    <= address)
	   && (address
	       < TME_SUNCG2_REG_ROPC_UNIT(0))) {

    /* the cycle handler: */
    tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_rop_data;
    
    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUNCG2_REG_ROP_DATA;
    tlb->tme_bus_tlb_addr_last = (TME_SUNCG2_REG_ROPC_UNIT(0) - 1);

    /* this TLB entry cannot allow fast reading or fast writing, since
       this is the rasterop data: */
  }

  /* if this address falls in the registers: */
  else if ((TME_SUNCG2_REG_ROPC_UNIT(0)
	    <= address)
	   && (address 
	       < TME_SUNCG2_REG_CMAP_R)) {

    /* the cycle handler: */
    tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_regs;

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUNCG2_REG_ROPC_UNIT(0);
    tlb->tme_bus_tlb_addr_last = (TME_SUNCG2_REG_CMAP_R - 1);

    /* this TLB entry cannot allow fast reading or fast writing, since
       these are registers: */
  }

  /* if this address falls in the colormap registers: */
  else if ((TME_SUNCG2_REG_CMAP_R
	    <= address)
	   && (address 
	       <= (TME_SUNCG2_SIZ_REGS
		   - 1))) {

    /* the cycle handler: */
    tlb->tme_bus_tlb_cycle = _tme_suncg2_bus_cycle_cmap;

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_SUNCG2_REG_CMAP_R;
    tlb->tme_bus_tlb_addr_last = TME_SUNCG2_SIZ_REGS - 1;

    /* this TLB entry allows fast reading: */
    tlb->tme_bus_tlb_emulator_off_read
      = (((tme_uint8_t *) suncg2->tme_suncg2_cmap_raw)
	 - TME_SUNCG2_REG_CMAP_R);
  }

  /* XXX FIXME - this is a partial implementation: */
  else {
    abort();
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  return (TME_OK);
}

/* this makes a new framebuffer connection: */
static int
_tme_suncg2_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_suncg2 *suncg2;
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;
  int rc;

  /* recover our data structures: */
  suncg2 = conn->tme_connection_element->tme_element_private;
  conn_fb = (struct tme_fb_connection *) conn;
  conn_fb_other = (struct tme_fb_connection *) conn->tme_connection_other;

  /* both sides must be framebuffer connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);

  /* lock our mutex: */
  tme_mutex_lock(&suncg2->tme_suncg2_mutex);

  /* once the connection is made, we know whether or not the other
     side of the connection is supplying specific memory that it wants
     us to use, or if we should allocate memory ourselves: */
  if (conn_fb->tme_fb_connection_buffer == NULL) {
    rc = tme_fb_xlat_alloc_src(conn_fb);
    assert (rc == TME_OK);
  }
  suncg2->tme_suncg2_displayed_memory = conn_fb->tme_fb_connection_buffer;

  /* invalidate any outstanding TLBs: */
  _tme_suncg2_tlb_invalidate(suncg2, NULL);

  /* the displayed memory is now invalid: */
  suncg2->tme_suncg2_flags |= TME_SUNCG2_FLAG_INVALID_DISPLAYED;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* save our connection: */
    suncg2->tme_suncg2_fb_connection = conn_fb_other;
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&suncg2->tme_suncg2_mutex);

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_suncg2_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a suncg2: */
static int
_tme_suncg2_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_suncg2 *suncg2;
  struct tme_fb_connection *conn_fb;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  suncg2 = (struct tme_suncg2 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* if we don't have a framebuffer connection, make one: */
  if (suncg2->tme_suncg2_fb_connection == NULL) {

    /* allocate the new framebuffer connection: */
    conn_fb = tme_new0(struct tme_fb_connection, 1);
    conn = &conn_fb->tme_fb_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_FRAMEBUFFER;
    conn->tme_connection_score = tme_fb_connection_score;
    conn->tme_connection_make = _tme_suncg2_connection_make;
    conn->tme_connection_break = _tme_suncg2_connection_break;

    /* fill in the framebuffer connection: */
    conn_fb->tme_fb_connection_mode_change = NULL;
    conn_fb->tme_fb_connection_update = _tme_suncg2_update;

    /* height and width: */
    conn_fb->tme_fb_connection_width = tme_sunfb_size_width(suncg2->tme_suncg2_size);
    conn_fb->tme_fb_connection_height = tme_sunfb_size_height(suncg2->tme_suncg2_size);

    /* we are color: */
    conn_fb->tme_fb_connection_class = TME_FB_XLAT_CLASS_COLOR;
    conn_fb->tme_fb_connection_depth = TME_SUNCG2_PLANE_MAX;
    conn_fb->tme_fb_connection_bits_per_pixel = TME_SUNCG2_PLANE_MAX;

    /* we skip no pixels at the start of the scanline: */
    conn_fb->tme_fb_connection_skipx = 0;

    /* we pad to 32-bit boundaries: */
    conn_fb->tme_fb_connection_scanline_pad = 32;

    /* we are big-endian: */
    conn_fb->tme_fb_connection_order = TME_ENDIAN_BIG;

    /* we don't allocate memory until the connection is made, in case
       the other side of the connection wants to provide us with a
       specific memory region to use (maybe we're on a system with a
       real cgtwo and we can write directly to its buffer): */
    conn_fb->tme_fb_connection_buffer = NULL;

    /* our pixels don't have subfields: */
    conn_fb->tme_fb_connection_mask_g = 0;
    conn_fb->tme_fb_connection_mask_r = 0;
    conn_fb->tme_fb_connection_mask_b = 0;

    /* intensities are eight bits and index mapped: */
    conn_fb->tme_fb_connection_map_bits = (8 * sizeof(suncg2->tme_suncg2_cmap[0]));
    conn_fb->tme_fb_connection_map_g = &suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_G)];
    conn_fb->tme_fb_connection_map_r = &suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_R)];
    conn_fb->tme_fb_connection_map_b = &suncg2->tme_suncg2_cmap[TME_SUNCG2_CMAP_INDEX(0, TME_SUNCG2_REG_CMAP_B)];

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new sun cgtwo function: */
int
tme_sun_cgtwo(struct tme_element *element, const char * const *args, char **_output)
{
  struct tme_suncg2 *suncg2;
  tme_uint32_t cg2_type;
  tme_uint32_t cg2_size;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  cg2_type = TME_SUNCG2_TYPE_NULL;
  cg2_size = TME_SUNFB_SIZE_1152_900;
  arg_i = 1;
  for (;;) {

    /* the framebuffer type: */
    if (TME_ARG_IS(args[arg_i + 0], "type")) {
      if (TME_ARG_IS(args[arg_i + 1], "sun3")) {
	cg2_type = TME_SUNCG2_TYPE_SUN3;
      }
      else {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* the framebuffer size: */
    else if (TME_ARG_IS(args[arg_i + 0], "size")) {
      cg2_size = tme_sunfb_size(args[arg_i + 1]);
      if (cg2_size != TME_SUNFB_SIZE_1152_900
	  && cg2_size != TME_SUNFB_SIZE_1024_1024) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* if we ran out of arguments: */
    else if (args[arg_i] == NULL) {
      break;
    }

    /* otherwise this is a bad argument: */
    else {
      tme_output_append_error(_output,
			      "%s %s, ",
			      args[arg_i],
			      _("unexpected"));
      usage = TRUE;
      break;
    }
  }

  /* a cgtwo type must have been given: */
  if (cg2_type == TME_SUNCG2_TYPE_NULL) {
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s type sun3 [ size { 1152x900 | 1024x1024 } ]",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the suncg2 structure: */
  suncg2 = tme_new0(struct tme_suncg2, 1);
  suncg2->tme_suncg2_element = element;
  tme_mutex_init(&suncg2->tme_suncg2_mutex);
  tme_rwlock_init(&suncg2->tme_suncg2_rwlock);

  /* set the cgtwo type: */
  suncg2->tme_suncg2_type = cg2_type;

  /* set the cgtwo size: */
  suncg2->tme_suncg2_size = cg2_size;

  /* start displaying the pixmap: */
  suncg2->tme_suncg2_bitmap_mode_plane = TME_SUNCG2_PLANE_MAX;

  /* set our initial CSR: */
  suncg2->tme_suncg2_csr
    = (TME_SUNCG2_CSR_ENABLE_VIDEO
       | (cg2_size == TME_SUNFB_SIZE_1024_1024
	  ? TME_SUNCG2_CSR_SIZE_1024_1024
	  : TME_SUNCG2_CSR_SIZE_1152_900));

  /* if this is a sun3 cgtwo: */
  if (cg2_type == TME_SUNCG2_TYPE_SUN3) {
    /* nothing to do */
  }

  /* calculate the pixel count: */
  suncg2->tme_suncg2_pixel_count
    = (tme_sunfb_size_width(cg2_size)
       * tme_sunfb_size_height(cg2_size));

  /* allocate the raw memory: */
  suncg2->tme_suncg2_raw_memory
    = tme_new0(tme_uint8_t, TME_SUNCG2_REG_PIXMAP + TME_SUNCG2_SIZ_PIXMAP);

  /* initialize our simple bus device descriptor: */
  suncg2->tme_suncg2_device.tme_bus_device_element = element;
  suncg2->tme_suncg2_device.tme_bus_device_tlb_fill = _tme_suncg2_tlb_fill;
  suncg2->tme_suncg2_device.tme_bus_device_address_last = TME_SUNCG2_SIZ_REGS - 1;

  /* fill the element: */
  element->tme_element_private = suncg2;
  element->tme_element_connections_new = _tme_suncg2_connections_new;

  return (TME_OK);
}
