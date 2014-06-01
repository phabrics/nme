/* $Id: sun-cgsix.c,v 1.3 2010/06/05 19:16:12 fredette Exp $ */

/* machine/sun/sun-cgsix.c - Sun cgsix emulation: */

/*
 * Copyright (c) 2009 Fredette
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
_TME_RCSID("$Id: sun-cgsix.c,v 1.3 2010/06/05 19:16:12 fredette Exp $");

/* includes: */
#include <tme/machine/sun.h>
#include <tme/generic/bus-device.h>
#include <tme/generic/fb.h>
#include "sun-fb.h"

/* macros: */

/* cgsix types: */
#define TME_SUNCG6_TYPE_NULL		(0)
#define TME_SUNCG6_TYPE_501_2325	(1)

/* register subregions: */
#define tme_suncg6_bus_subregion_p4 tme_suncg6_sunfb.tme_sunfb_bus_subregion_regs
#define tme_suncg6_bus_handler_p4 tme_suncg6_sunfb.tme_sunfb_bus_handler_regs
#define tme_suncg6_bus_subregion_fbc tme_suncg6_sunfb.tme_sunfb_bus_subregions[1]
#define tme_suncg6_bus_handler_fbc tme_suncg6_sunfb.tme_sunfb_bus_handlers[1]
#define tme_suncg6_bus_subregion_dac tme_suncg6_sunfb.tme_sunfb_bus_subregions[2]
#define tme_suncg6_bus_handler_dac tme_suncg6_sunfb.tme_sunfb_bus_handlers[2]
#define tme_suncg6_bus_subregion_fhc_thc tme_suncg6_sunfb.tme_sunfb_bus_subregions[4]
#define tme_suncg6_bus_handler_fhc_thc tme_suncg6_sunfb.tme_sunfb_bus_handlers[4]
#define tme_suncg6_bus_subregion_tec tme_suncg6_sunfb.tme_sunfb_bus_subregions[3]
#define tme_suncg6_bus_handler_tec tme_suncg6_sunfb.tme_sunfb_bus_handlers[3]
#define tme_suncg6_bus_subregion_alt tme_suncg6_sunfb.tme_sunfb_bus_subregions[6]
#define tme_suncg6_bus_handler_alt tme_suncg6_sunfb.tme_sunfb_bus_handlers[6]

/* this converts a group member to a register offset: */
#define _TME_SUNCG6_REG_X(module_cap, module, n, x)		\
  (_TME_CONCAT4(TME_SUNCG6_REG_,module_cap,_GROUP,n)		\
   + ((tme_uint8_t *) &((struct tme_suncg6 *) 0)		\
      ->_TME_CONCAT4(tme_suncg6_,module,_group,n)		\
      ._TME_CONCAT4(tme_suncg6_,module,_,x))			\
   - ((tme_uint8_t *) &((struct tme_suncg6 *) 0)		\
      ->_TME_CONCAT4(tme_suncg6_,module,_group,n)))

/* this expands part of a large ternary expression that converts a
   register address into pointer to a register: */
#define TME_SUNCG6_REG_POINTER(suncg6, reg, module_cap, module, n)\
  (((reg)							\
    >= _TME_CONCAT4(TME_SUNCG6_REG_,module_cap,_GROUP,n))	\
   && ((reg)							\
       < (_TME_CONCAT4(TME_SUNCG6_REG_,module_cap,_GROUP,n)	\
	  + sizeof((suncg6)					\
		   ->_TME_CONCAT4(tme_suncg6_,module,_group,n)))))\
  ? (((tme_suncg6_reg_t *)					\
      &((suncg6)						\
	->_TME_CONCAT4(tme_suncg6_,module,_group,n)))		\
     + (((reg)							\
	 - _TME_CONCAT4(TME_SUNCG6_REG_,module_cap,_GROUP,n))	\
	/ sizeof(tme_suncg6_reg_t)))
 
/* some register offsets and sizes: */
#define TME_SUNCG6_REG_DAC			(0x200000)
#define TME_SUNCG6_REG_ALT			(0x280000)
#define TME_SUNCG6_SIZ_ALT			(0x1000)
#define TME_SUNCG6_REG_THC(n,x)			_TME_SUNCG6_REG_X(THC,thc,n,x)
#define TME_SUNCG6_SIZ_THC			(0x1000)
#define TME_SUNCG6_REG_FBC(n,x)			_TME_SUNCG6_REG_X(FBC,fbc,n,x)
#define TME_SUNCG6_REG_TEC(n,x)			_TME_SUNCG6_REG_X(TEC,tec,n,x)
#define TME_SUNCG6_TEC_SIZE			(0x1000)

/* the FHC register: */
#define	TME_SUNCG6_FHC_ID_MASK			(0xff000000)
#define	TME_SUNCG6_FHC_REVISION_MASK		(0x00f00000)
#define	TME_SUNCG6_FHC_DISABLE_FROP		(0x00080000)
#define	TME_SUNCG6_FHC_DISABLE_ROW		(0x00040000)
#define	TME_SUNCG6_FHC_DISABLE_SRC		(0x00020000)
#define	TME_SUNCG6_FHC_DISABLE_DST		(0x00010000)
#define	TME_SUNCG6_FHC_RESET			(0x00008000)
#define	TME_SUNCG6_FHC_ENDIAN_LITTLE		(0x00002000)
#define	TME_SUNCG6_FHC_SIZE_MASK		(0x00001800)
#define	 TME_SUNCG6_FHC_SIZE_1024_768		 (0x00000000)
#define	 TME_SUNCG6_FHC_SIZE_1152_900		 (0x00000800)
#define	 TME_SUNCG6_FHC_SIZE_1280_1024		 (0x00001000)
#define	 TME_SUNCG6_FHC_SIZE_1600_1280		 (0x00001800)
#define	TME_SUNCG6_FHC_CPU_MASK			(0x00000600)
#define	 TME_SUNCG6_FHC_CPU_SPARC		 (0x00000000)
#define	 TME_SUNCG6_FHC_CPU_68K			 (0x00000200)
#define	 TME_SUNCG6_FHC_CPU_IA32		 (0x00000400)
#define	TME_SUNCG6_FHC_TEST			(0x00000100)
#define	TME_SUNCG6_FHC_TESTX_MASK		(0x000000f0)
#define	TME_SUNCG6_FHC_TESTY_MASK		(0x0000000f)

/* the THC miscellaneous register: */
#define	TME_SUNCG6_THC_MISC_UNKNOWN_20		(0xfff00000)
#define	TME_SUNCG6_THC_MISC_REVISION_MASK	(0x000f0000)
#define	TME_SUNCG6_THC_MISC_UNKNOWN_13		(0x0000e000)
#define	TME_SUNCG6_THC_MISC_RESET		(0x00001000)
#define	TME_SUNCG6_THC_MISC_UNKNOWN_11		(0x00000800)
#define	TME_SUNCG6_THC_MISC_ENABLE_VIDEO	(0x00000400)
#define	TME_SUNCG6_THC_MISC_SYNC		(0x00000200)
#define	TME_SUNCG6_THC_MISC_VSYNC		(0x00000100)
#define	TME_SUNCG6_THC_MISC_ENABLE_SYNC		(0x00000080)
#define	TME_SUNCG6_THC_MISC_CURSOR_RESOLUTION	(0x00000040)
#define	TME_SUNCG6_THC_MISC_INT_ENABLE		(0x00000020)
#define	TME_SUNCG6_THC_MISC_INT_ACTIVE		(0x00000010)
#define	TME_SUNCG6_THC_MISC_UNKNOWN_0		(0x0000000f)

/* the FBC rasterop register: */
#define TME_SUNCG6_FBC_RASTEROP_PLANE_ONES	(0x80000000)
#define TME_SUNCG6_FBC_RASTEROP_PLANE_ZEROES	(0x40000000)
#define TME_SUNCG6_FBC_RASTEROP_PIXEL_ONES	(0x20000000)
#define TME_SUNCG6_FBC_RASTEROP_PIXEL_ZEROES	(0x10000000)
#define TME_SUNCG6_FBC_RASTEROP_PATTERN_ONES	(0x08000000)
#define TME_SUNCG6_FBC_RASTEROP_PATTERN_ZEROES	(0x04000000)
#define TME_SUNCG6_FBC_RASTEROP_POLYG_NONOVERLAP (0x02000000)
#define TME_SUNCG6_FBC_RASTEROP_POLYG_OVERLAP	(0x01000000)
#define TME_SUNCG6_FBC_RASTEROP_ATTR_SUPP	(0x00800000)
#define TME_SUNCG6_FBC_RASTEROP_ATTR_UNSUPP	(0x00400000)
#define TME_SUNCG6_FBC_RASTEROP_RAST_BOOL	(0x00000000)
#define TME_SUNCG6_FBC_RASTEROP_RAST_LINEAR	(0x00020000)
#define TME_SUNCG6_FBC_RASTEROP_PLOT_PLOT	(0x00000000)
#define TME_SUNCG6_FBC_RASTEROP_PLOT_UNPLOT	(0x00010000)
#define TME_SUNCG6_FBC_RASTEROP_ROP_11_1(x)	((x) << 14)
#define TME_SUNCG6_FBC_RASTEROP_ROP_11_0(x)	((x) << 12)
#define TME_SUNCG6_FBC_RASTEROP_ROP_10_1(x)	((x) << 10)
#define TME_SUNCG6_FBC_RASTEROP_ROP_10_0(x)	((x) <<  8)
#define TME_SUNCG6_FBC_RASTEROP_ROP_01_1(x)	((x) <<  6)
#define TME_SUNCG6_FBC_RASTEROP_ROP_01_0(x)	((x) <<  4)
#define TME_SUNCG6_FBC_RASTEROP_ROP_00_1(x)	((x) <<  2)
#define TME_SUNCG6_FBC_RASTEROP_ROP_00_0(x)	((x) <<  0)
#define  TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR	 (0x0)
#define  TME_SUNCG6_FBC_RASTEROP_ROP_INVERT	 (0x1)
#define  TME_SUNCG6_FBC_RASTEROP_ROP_NOP	 (0x2)
#define  TME_SUNCG6_FBC_RASTEROP_ROP_SET	 (0x3)

/* common FBC rasterop functions: */

/* copy: */
#define TME_SUNCG6_FBC_RASTEROP_FUNC_COPY					\
  (TME_SUNCG6_FBC_RASTEROP_PLANE_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PIXEL_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PATTERN_ONES					\
   | TME_SUNCG6_FBC_RASTEROP_POLYG_OVERLAP					\
   | TME_SUNCG6_FBC_RASTEROP_ATTR_SUPP						\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR))

/* fill: */
#define TME_SUNCG6_FBC_RASTEROP_FUNC_FILL					\
  (TME_SUNCG6_FBC_RASTEROP_PLANE_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PIXEL_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PATTERN_ONES					\
   | TME_SUNCG6_FBC_RASTEROP_POLYG_OVERLAP					\
   | TME_SUNCG6_FBC_RASTEROP_ATTR_SUPP						\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_0(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_0(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_1(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_1(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR))

/* flip: */
#define TME_SUNCG6_FBC_RASTEROP_FUNC_FLIP					\
  (TME_SUNCG6_FBC_RASTEROP_PLANE_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PIXEL_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PATTERN_ONES					\
   | TME_SUNCG6_FBC_RASTEROP_POLYG_OVERLAP					\
   | TME_SUNCG6_FBC_RASTEROP_ATTR_SUPP						\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_0(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_0(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_0(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_0(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT))

/* NB: I couldn't reason out how the rop?? values work.  the values
   used by X11 and the NetBSD cgsix driver make some sense, but the
   0x6c60 used by the SUNW,501-2325 PROM for all of its blits and
   draws is confusing: */
#define TME_SUNCG6_FBC_RASTEROP_FUNC_PROM					\
  (TME_SUNCG6_FBC_RASTEROP_PLANE_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PIXEL_ONES						\
   | TME_SUNCG6_FBC_RASTEROP_PATTERN_ONES					\
   | TME_SUNCG6_FBC_RASTEROP_POLYG_OVERLAP					\
   | TME_SUNCG6_FBC_RASTEROP_ATTR_SUPP						\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_11_0(TME_SUNCG6_FBC_RASTEROP_ROP_NOP)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_1(TME_SUNCG6_FBC_RASTEROP_ROP_SET)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_10_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_1(TME_SUNCG6_FBC_RASTEROP_ROP_INVERT)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_01_0(TME_SUNCG6_FBC_RASTEROP_ROP_NOP)		\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_1(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR)	\
   | TME_SUNCG6_FBC_RASTEROP_ROP_00_0(TME_SUNCG6_FBC_RASTEROP_ROP_CLEAR))

/* types: */

/* the cgsix pixel is eight bits: */
typedef tme_uint8_t tme_suncg6_pixel_t;

/* all cgsix registers are at least 32-bit aligned: */
typedef tme_uint32_t tme_suncg6_reg_t;

/* the card: */
struct tme_suncg6 {

  /* our generic Sun framebuffer: */
  struct tme_sunfb tme_suncg6_sunfb;
#define tme_suncg6_mutex tme_suncg6_sunfb.tme_sunfb_mutex
#define tme_suncg6_bus_subregion_memory tme_suncg6_sunfb.tme_sunfb_bus_subregion_memory
#define tme_suncg6_bus_subregion_regs tme_suncg6_sunfb.tme_sunfb_bus_subregion_regs

  /* the type of the cgsix: */
  tme_uint32_t tme_suncg6_type;

  /* the current framebuffer width and height: */
  tme_uint32_t tme_suncg6_width;
  tme_uint32_t tme_suncg6_height;

  /* the FHC: */
#define TME_SUNCG6_REG_FHC			(0x300000)
  tme_suncg6_reg_t tme_suncg6_fhc;

  /* the THC: */
#define TME_SUNCG6_REG_THC_UNKNOWN_0x0		(0x301000)
  struct {
#define TME_SUNCG6_REG_THC_GROUP0		(0x301090)
    tme_suncg6_reg_t tme_suncg6_thc_unknown_0x90;
    tme_suncg6_reg_t tme_suncg6_thc_unknown_0x94;
  } tme_suncg6_thc_group0;
  struct {
#define TME_SUNCG6_REG_THC_GROUP1		(0x301800)
    tme_suncg6_reg_t tme_suncg6_thc_hsync1;
    tme_suncg6_reg_t tme_suncg6_thc_hsync2;
    tme_suncg6_reg_t tme_suncg6_thc_hsync3;
    tme_suncg6_reg_t tme_suncg6_thc_vsync1;
    tme_suncg6_reg_t tme_suncg6_thc_vsync2;
    tme_suncg6_reg_t tme_suncg6_thc_refresh;
    tme_suncg6_reg_t tme_suncg6_thc_misc;
    tme_suncg6_reg_t tme_suncg6_thc_unknown_0x81c[56];
    tme_suncg6_reg_t tme_suncg6_thc_cursor_xy;
    tme_suncg6_reg_t tme_suncg6_thc_cursor_mask[32];
    tme_suncg6_reg_t tme_suncg6_thc_cursor_bits[32];
  } tme_suncg6_thc_group1;

  /* the FBC: */
  struct {
#define TME_SUNCG6_REG_FBC_GROUP0		(0x700000)
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0x0;
    tme_suncg6_reg_t tme_suncg6_fbc_misc;
    tme_suncg6_reg_t tme_suncg6_fbc_clip_check;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0xc;
    tme_suncg6_reg_t tme_suncg6_fbc_status;
    tme_suncg6_reg_t tme_suncg6_fbc_status_draw;
    tme_suncg6_reg_t tme_suncg6_fbc_status_blit;
    tme_suncg6_reg_t tme_suncg6_fbc_font;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0x20[24];
    tme_suncg6_reg_t tme_suncg6_fbc_x0;
    tme_suncg6_reg_t tme_suncg6_fbc_y0;
    tme_suncg6_reg_t tme_suncg6_fbc_z0;
    tme_suncg6_reg_t tme_suncg6_fbc_color0;
    tme_suncg6_reg_t tme_suncg6_fbc_x1;
    tme_suncg6_reg_t tme_suncg6_fbc_y1;
    tme_suncg6_reg_t tme_suncg6_fbc_z1;
    tme_suncg6_reg_t tme_suncg6_fbc_color1;
    tme_suncg6_reg_t tme_suncg6_fbc_x2;
    tme_suncg6_reg_t tme_suncg6_fbc_y2;
    tme_suncg6_reg_t tme_suncg6_fbc_z2;
    tme_suncg6_reg_t tme_suncg6_fbc_color2;
    tme_suncg6_reg_t tme_suncg6_fbc_x3;
    tme_suncg6_reg_t tme_suncg6_fbc_y3;
    tme_suncg6_reg_t tme_suncg6_fbc_z3;
    tme_suncg6_reg_t tme_suncg6_fbc_color3;
    tme_suncg6_reg_t tme_suncg6_fbc_offx;
    tme_suncg6_reg_t tme_suncg6_fbc_offy;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0xc8[2];
    tme_suncg6_reg_t tme_suncg6_fbc_inc_x;
    tme_suncg6_reg_t tme_suncg6_fbc_inc_y;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0xd8[2];
    tme_suncg6_reg_t tme_suncg6_fbc_clip_min_x;
    tme_suncg6_reg_t tme_suncg6_fbc_clip_min_y;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0xe8[2];
    tme_suncg6_reg_t tme_suncg6_fbc_clip_max_x;
    tme_suncg6_reg_t tme_suncg6_fbc_clip_max_y;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0xf8[2];
    tme_suncg6_reg_t tme_suncg6_fbc_fg;
    tme_suncg6_reg_t tme_suncg6_fbc_bg;
    tme_suncg6_reg_t tme_suncg6_fbc_rasterop;
    tme_suncg6_reg_t tme_suncg6_fbc_pm;
    tme_suncg6_reg_t tme_suncg6_fbc_pixelm;
    tme_suncg6_reg_t tme_suncg6_fbc_unknown_0x114[2];
    tme_suncg6_reg_t tme_suncg6_fbc_patalign;
    tme_suncg6_reg_t tme_suncg6_fbc_pattern[8];
  } tme_suncg6_fbc_group0;
  struct {
#define TME_SUNCG6_REG_FBC_GROUP1		(0x700900)
    tme_suncg6_reg_t tme_suncg6_fbc_rect_abs_x;
    tme_suncg6_reg_t tme_suncg6_fbc_rect_abs_y;
  } tme_suncg6_fbc_group1;

  /* in the FBC, the rectangle absolute coordinate registers seem to
     be indexed: */
#define TME_SUNCG6_RECT_ABS_COUNT		(2)
  unsigned int tme_suncg6_fbc_rect_abs_x_i;
  unsigned int tme_suncg6_fbc_rect_abs_y_i;
  tme_suncg6_reg_t tme_suncg6_fbc_rect_abs_x[TME_SUNCG6_RECT_ABS_COUNT];
  tme_suncg6_reg_t tme_suncg6_fbc_rect_abs_y[TME_SUNCG6_RECT_ABS_COUNT];

  /* the TEC: */
  struct {
#define TME_SUNCG6_REG_TEC_GROUP0		(0x701000)
    tme_suncg6_reg_t tme_suncg6_tec_mv;
    tme_suncg6_reg_t tme_suncg6_tec_clip;
    tme_suncg6_reg_t tme_suncg6_tec_vdc;
  } tme_suncg6_tec_group0;
};

/* this does a blit: */
static void
_tme_suncg6_blit(struct tme_suncg6 *suncg6)
{
  tme_uint32_t fb_width;
  tme_uint32_t fb_height;
  tme_int32_t src_x;
  tme_int32_t src_y;
  tme_int32_t dst_x;
  tme_int32_t dst_y;
  tme_int32_t blit_width;
  tme_int32_t blit_height;
  int unsupported;
  tme_uint32_t offset_updated_first;
  tme_uint32_t offset_updated_last;
  const tme_suncg6_pixel_t *src;
  tme_suncg6_pixel_t *dst;

  /* get the framebuffer width and height: */
  fb_width = suncg6->tme_suncg6_width;
  fb_height = suncg6->tme_suncg6_height;

  /* get the blit source: */
  src_x = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x0;
  src_y = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y0;

  /* get the blit destination: */
  dst_x = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x2;
  dst_y = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y2;

  /* get the blit width and height: */
  blit_width = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x1;
  blit_width
    = (blit_width < src_x
       ? 0
       : (blit_width + 1 - src_x));
  blit_height = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y1;
  blit_height
    = (blit_height < src_y
       ? 0
       : (blit_height + 1 - src_y));

  /* check that this blit is supported: */
  unsupported = FALSE;
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_check != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_status != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offx != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offy != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_x != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_y != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_x != (fb_width - 1));
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_y != (fb_height - 1));
  unsupported
    |= (blit_width
	!= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x3 < dst_x
	    ? 0
	    : (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x3 + 1 - dst_x)));
  unsupported
    |= (blit_height
	!= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y3 < dst_y
	    ? 0
	    : (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y3 + 1 - dst_y)));
  if (__tme_predict_false(unsupported)) {
    tme_log(TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb), 0, EINVAL,
	    (TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb),
	     _("unsupported blit parameters")));
    return;
  }

  /* if any of the least coordinates aren't in the framebuffer: */
  if (src_x >= fb_width
      || dst_x >= fb_width
      || src_y >= fb_height
      || dst_y >= fb_height) {
    return;
  }

  /* make sure the blit height and width stay in the framebuffer: */
  blit_width = TME_MIN(blit_width,
		       TME_MIN(fb_width - src_x,
			       fb_width - dst_x));
  blit_height = TME_MIN(blit_height,
			TME_MIN(fb_height - src_y,
				fb_height - dst_y));

  /* if the blit height or width are zero: */
  if (blit_width == 0
      || blit_height == 0) {
    return;
  }

  /* get the offsets of the first and last bytes of the
     destination: */
  offset_updated_first = ((fb_width * dst_y) + dst_x);
  offset_updated_last
    = (offset_updated_first
       + (blit_width - 1)
       + (fb_width * (blit_height - 1)));

  /* get the first source and destination rows: */
  dst = (tme_suncg6_pixel_t *) suncg6->tme_suncg6_sunfb.tme_sunfb_memory;
  src = dst + ((fb_width * src_y) + src_x);
  dst += offset_updated_first;

  /* if this is a copy: */
  if (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_rasterop
      == TME_SUNCG6_FBC_RASTEROP_FUNC_COPY
      /* XXX FIXME - the rasterop function that the SUNW,501-2325 PROM
	 uses for all of its blits and draws doesn't do a copy blit,
	 but for now we treat it like it does: */
      || (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_rasterop
	  == TME_SUNCG6_FBC_RASTEROP_FUNC_PROM)
      ) {

    /* if the copy is full-width: */
    if (blit_width == fb_width) {

      /* copy the source to the destination: */
      memmove(dst,
	      src,
	      sizeof(tme_suncg6_pixel_t) * blit_width * blit_height);
    }

    /* otherwise, the copy is not full-width: */
    else {

      /* copy the source to the destination: */
      do {
	memmove(dst,
		src,
		sizeof(tme_suncg6_pixel_t) * blit_width);
	dst += sizeof(tme_suncg6_pixel_t) * fb_width;
	src += sizeof(tme_suncg6_pixel_t) * fb_width;
      } while (--blit_height);
    }
  }

  /* otherwise, this is an unsupported blit: */
  else {
    tme_log(TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb), 0, EINVAL,
	    (TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb),
	     _("unsupported blit function")));
  }

  /* update the offsets of the first and last bytes updated in the
     real framebuffer memory: */
  offset_updated_first
    = TME_MIN(offset_updated_first,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first);
  offset_updated_last
    = TME_MAX(offset_updated_last,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last);
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first = offset_updated_first;
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last = offset_updated_last;
}

/* this does a draw: */
static void
_tme_suncg6_draw(struct tme_suncg6 *suncg6)
{
  tme_uint32_t fb_width;
  tme_uint32_t fb_height;
  unsigned int rect_abs_i;
  tme_int32_t draw_width;
  tme_int32_t dst_x;
  tme_int32_t draw_height;
  tme_int32_t dst_y;
  int unsupported;
  tme_uint32_t offset_updated_first;
  tme_uint32_t offset_updated_last;
  tme_suncg6_pixel_t *dst;
  tme_int32_t resid_width;

  /* get the framebuffer width and height: */
  fb_width = suncg6->tme_suncg6_width;
  fb_height = suncg6->tme_suncg6_height;

  /* get the draw width and destination x: */
  rect_abs_i = suncg6->tme_suncg6_fbc_rect_abs_x_i;
  draw_width = suncg6->tme_suncg6_fbc_rect_abs_x[rect_abs_i % TME_SUNCG6_RECT_ABS_COUNT];
  rect_abs_i = rect_abs_i - 1;
  dst_x = suncg6->tme_suncg6_fbc_rect_abs_x[rect_abs_i % TME_SUNCG6_RECT_ABS_COUNT];
  draw_width
    = (draw_width < dst_x
       ? 0
       : (draw_width + 1 - dst_x));

  /* get the draw height and destination y: */
  rect_abs_i = suncg6->tme_suncg6_fbc_rect_abs_y_i;
  draw_height = suncg6->tme_suncg6_fbc_rect_abs_y[rect_abs_i % TME_SUNCG6_RECT_ABS_COUNT];
  rect_abs_i = rect_abs_i - 1;
  dst_y = suncg6->tme_suncg6_fbc_rect_abs_y[rect_abs_i % TME_SUNCG6_RECT_ABS_COUNT];
  draw_height
    = (draw_height < dst_y
       ? 0
       : (draw_height + 1 - dst_y));

  /* check that this draw is supported: */
  unsupported = FALSE;
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_check != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_status != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offx != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offy != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_x != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_y != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_x != (fb_width - 1));
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_y != (fb_height - 1));
  if (__tme_predict_false(unsupported)) {
    tme_log(TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb), 0, EINVAL,
	    (TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb),
	     _("unsupported draw parameters")));
    return;
  }

  /* if the draw coordinate isn't in the framebuffer: */
  if (dst_x >= fb_width
      || dst_y >= fb_height) {
    return;
  }

  /* make sure the draw height and width stay in the framebuffer: */
  draw_width = TME_MIN(draw_width,
		       fb_width - dst_x);
  draw_height = TME_MIN(draw_height,
			fb_height - dst_y);

  /* if the draw height or width are zero: */
  if (draw_width == 0
      || draw_height == 0) {
    return;
  }

  /* get the offsets of the first and last bytes of the
     destination: */
  offset_updated_first = ((fb_width * dst_y) + dst_x);
  offset_updated_last
    = (offset_updated_first
       + (draw_width - 1)
       + (fb_width * (draw_height - 1)));

  /* get the first destination row: */
  dst = (tme_suncg6_pixel_t *) suncg6->tme_suncg6_sunfb.tme_sunfb_memory;
  dst += offset_updated_first;

  /* if this is a fill: */
  if (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_rasterop
      == TME_SUNCG6_FBC_RASTEROP_FUNC_FILL
      /* XXX FIXME - the rasterop function that the SUNW,501-2325 PROM
	 uses for all of its blits and draws doesn't do a fill draw,
	 but for now we treat it like it does: */
      || (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_rasterop
	  == TME_SUNCG6_FBC_RASTEROP_FUNC_PROM)
      ) {

    /* if the fill is full-width: */
    if (draw_width == fb_width) {

      /* fill the destination: */
      memset(dst,
	     (tme_uint8_t) suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_fg,
	     sizeof(tme_suncg6_pixel_t) * draw_width * draw_height);
    }

    /* otherwise, the fill is not full-width: */
    else {

      /* fill the destination: */
      do {
	memset(dst,
	       (tme_uint8_t) suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_fg,
	       sizeof(tme_suncg6_pixel_t) * draw_width);
	dst += sizeof(tme_suncg6_pixel_t) * fb_width;
      } while (--draw_height);
    }
  }

  /* otherwise, if this is an invert: */
  else if (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_rasterop
	   == TME_SUNCG6_FBC_RASTEROP_FUNC_FLIP) {

    /* do the inversion: */
    /* XXX FIXME - the inversion almost certainly isn't this
       simple: */
    do {
      resid_width = draw_width;
      do {
	*(dst++) ^= 0xff;
      } while (--resid_width);
      dst = (dst - draw_width) + fb_width;
    } while (--draw_height);
  }

  /* otherwise, this is an unsupported draw: */
  else {
    tme_log(TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb), 0, EINVAL,
	    (TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb),
	     _("unsupported draw function")));
  }

  /* update the offsets of the first and last bytes updated in the
     real framebuffer memory: */
  offset_updated_first
    = TME_MIN(offset_updated_first,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first);
  offset_updated_last
    = TME_MAX(offset_updated_last,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last);
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first = offset_updated_first;
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last = offset_updated_last;
}

/* this does a font draw: */
static void
_tme_suncg6_font(struct tme_suncg6 *suncg6,
		 tme_suncg6_reg_t font)
{
  tme_uint32_t fb_width;
  tme_uint32_t fb_height;
  tme_int32_t dst_x;
  tme_int32_t dst_y;
  tme_int32_t draw_width;
  int unsupported;
  tme_uint32_t offset_updated_first;
  tme_uint32_t offset_updated_last;
  tme_suncg6_pixel_t *dst;
  tme_uint16_t pixels;

  /* get the framebuffer width and height: */
  fb_width = suncg6->tme_suncg6_width;
  fb_height = suncg6->tme_suncg6_height;

  /* get the font draw destination: */
  dst_x = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x0;
  dst_y = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y0;

  /* get the font draw width: */
  draw_width = suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x1;
  draw_width
    = (draw_width < dst_x
       ? 0
       : (draw_width + 1 - dst_x));
  
  /* check that this font draw is supported: */
  unsupported = FALSE;
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_check != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_status != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offx != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_offy != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_x != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_min_y != 0);
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_x != (fb_width - 1));
  unsupported |= (suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_clip_max_y != (fb_height - 1));
  if (__tme_predict_false(unsupported)) {
    tme_log(TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb), 0, EINVAL,
	    (TME_SUNFB_LOG_HANDLE(&suncg6->tme_suncg6_sunfb),
	     _("unsupported font draw parameters")));
    return;
  }

  /* if the font draw coordinate isn't in the framebuffer: */
  if (dst_x >= fb_width
      || dst_y >= fb_height) {
    return;
  }

  /* make sure the font draw width stays in the framebuffer: */
  draw_width = TME_MIN(draw_width,
		       fb_width - dst_x);

  /* if the draw width is zero: */
  if (draw_width == 0) {
    return;
  }

  /* get the offsets of the first and last bytes of the
     destination: */
  offset_updated_first = ((fb_width * dst_y) + dst_x);
  offset_updated_last
    = (offset_updated_first
       + (draw_width - 1));

  /* get the first destination row: */
  dst = (tme_suncg6_pixel_t *) suncg6->tme_suncg6_sunfb.tme_sunfb_memory;
  dst += offset_updated_first;

  /* get the foreground and background pixels: */
  pixels = (tme_suncg6_pixel_t) suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_fg;
  pixels <<= (8 * sizeof(tme_suncg6_pixel_t));
  pixels += (tme_suncg6_pixel_t) suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_bg;

  /* do the font draw: */
  do {
    *(dst++)
      = ((tme_suncg6_pixel_t)
	 (pixels
	  >> ((0 - (((tme_int32_t) font) < 0))
	      & (8 * sizeof(tme_suncg6_pixel_t)))));
    font <<= 1;
  } while (--draw_width);

  /* advance after the font draw: */
  suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x0 += suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_inc_x;
  suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_x1 += suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_inc_x;
  suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_y0 += suncg6->tme_suncg6_fbc_group0.tme_suncg6_fbc_inc_y;

  /* update the offsets of the first and last bytes updated in the
     real framebuffer memory: */
  offset_updated_first
    = TME_MIN(offset_updated_first,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first);
  offset_updated_last
    = TME_MAX(offset_updated_last,
	      suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last);
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_first = offset_updated_first;
  suncg6->tme_suncg6_sunfb.tme_sunfb_offset_updated_last = offset_updated_last;
}

/* the ALT bus cycle handler: */
static void
_tme_suncg6_bus_cycle_alt(struct tme_sunfb *sunfb,
			  struct tme_bus_cycle *master_cycle,
			  tme_uint32_t *_master_fast_cycle_types,
			  struct tme_completion *master_completion)
{
  struct tme_suncg6 *suncg6;
  tme_bus_addr32_t reg;
  tme_suncg6_reg_t *_reg;
  tme_suncg6_reg_t value32_buffer;
  tme_suncg6_reg_t value32;

  /* recover our data structure: */
  suncg6 = (struct tme_suncg6 *) sunfb;

  /* get the register: */
  reg = master_cycle->tme_bus_cycle_address;

  /* convert the register into a pointer: */
  _reg = NULL;

  /* if this is a write: */
  if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
    value32 = value32_buffer;
  }

  /* otherwise, this must be a read: */
  else {
    assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* this must be a maintained register: */
    if (_reg == NULL) {
      abort();
    }
    value32 = *_reg;

    /* run the bus cycle: */
    value32_buffer = value32;
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
  }

  /* complete the cycle: */
  master_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(master_completion);
  *_master_fast_cycle_types = 0;

  tme_log(TME_SUNFB_LOG_HANDLE(sunfb), 2000, TME_OK,
	  (TME_SUNFB_LOG_HANDLE(sunfb),
	   _("ALT 0x%06" TME_PRIx32 " %s%s 0x%08" TME_PRIx32),
	   reg,
	   (_reg == NULL
	    ? "UNKNOWN "
	    : ""),
	   (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	    ? "<-"
	    : "->"),
	   value32));
}

/* the FHC and THC bus cycle handler: */
static void
_tme_suncg6_bus_cycle_fhc_thc(struct tme_sunfb *sunfb,
			      struct tme_bus_cycle *master_cycle,
			      tme_uint32_t *_master_fast_cycle_types,
			      struct tme_completion *master_completion)
{
  struct tme_suncg6 *suncg6;
  tme_bus_addr32_t reg;
  tme_suncg6_reg_t *_reg;
  tme_suncg6_reg_t value32_buffer;
  tme_suncg6_reg_t value32;
  tme_suncg6_reg_t reg_mask_ro;

  /* recover our data structure: */
  suncg6 = (struct tme_suncg6 *) sunfb;

  /* get the register: */
  reg = master_cycle->tme_bus_cycle_address;

  /* convert the register into a pointer: */
  if (reg >= TME_SUNCG6_REG_THC_UNKNOWN_0x0) {
    _reg = (TME_SUNCG6_REG_POINTER(suncg6, reg,THC,thc,0)
	    : TME_SUNCG6_REG_POINTER(suncg6, reg,THC,thc,1)
	    : NULL);
  }
  else {
    reg = TME_SUNCG6_REG_FHC;
    _reg = &suncg6->tme_suncg6_fhc;
  }

  /* if this is a write: */
  if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
    value32 = value32_buffer;

    /* assume that none of the register is read-only: */
    reg_mask_ro = 0;

    /* dispatch on the register: */
    switch (reg) {

      /* the FHC: */
    case TME_SUNCG6_REG_FHC:
      reg_mask_ro
	= (TME_SUNCG6_FHC_ID_MASK
	   + TME_SUNCG6_FHC_REVISION_MASK
	   + TME_SUNCG6_FHC_SIZE_MASK);
      break;

      /* the THC miscellaneous register: */
    case TME_SUNCG6_REG_THC(1,misc):
      reg_mask_ro
	= (TME_SUNCG6_THC_MISC_REVISION_MASK
	   );
      break;

    default:
      break;
    }

    /* if this is a maintained register: */
    if (_reg != NULL) {
      *_reg
	= ((*_reg
	    & reg_mask_ro)
	   + (value32
	      & ~reg_mask_ro));
    }
  }

  /* otherwise, this is a read: */
  else {
    assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register: */
    switch (reg) {

    default:
      break;
    }

    /* this must be a maintained register: */
    if (_reg == NULL) {
      abort();
    }
    value32 = *_reg;

    /* run the bus cycle: */
    value32_buffer = value32;
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
  }

  /* complete the cycle: */
  master_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(master_completion);
  *_master_fast_cycle_types = 0;

  tme_log(TME_SUNFB_LOG_HANDLE(sunfb), 2000, TME_OK,
	  (TME_SUNFB_LOG_HANDLE(sunfb),
	   _("%s 0x%06" TME_PRIx32 " %s%s 0x%08" TME_PRIx32),
	   (reg >= TME_SUNCG6_REG_THC_UNKNOWN_0x0
	    ? "THC"
	    : "FHC"),
	   reg,
	   (_reg == NULL
	    ? "UNKNOWN "
	    : ""),
	   (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	    ? "<-"
	    : "->"),
	   value32));
}

/* the FBC bus cycle handler: */
static void
_tme_suncg6_bus_cycle_fbc(struct tme_sunfb *sunfb,
			  struct tme_bus_cycle *master_cycle,
			  tme_uint32_t *_master_fast_cycle_types,
			  struct tme_completion *master_completion)
{
  struct tme_suncg6 *suncg6;
  tme_bus_addr32_t reg;
  tme_suncg6_reg_t *_reg;
  tme_suncg6_reg_t value32_buffer;
  tme_suncg6_reg_t value32;
  tme_suncg6_reg_t reg_mask_ro;

  /* recover our data structure: */
  suncg6 = (struct tme_suncg6 *) sunfb;

  /* get the register: */
  reg = master_cycle->tme_bus_cycle_address;

  /* convert the register into a pointer: */
  _reg
    = (TME_SUNCG6_REG_POINTER(suncg6, reg,FBC,fbc,0)
       : TME_SUNCG6_REG_POINTER(suncg6, reg,FBC,fbc,1)
       : NULL);

  /* if this is a write: */
  if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
    value32 = value32_buffer;

    /* assume that none of the register is read-only: */
    reg_mask_ro = 0;

    /* dispatch on the register: */
    switch (reg) {

      /* a write of the font register: */
    case TME_SUNCG6_REG_FBC(0,font):
      _tme_suncg6_font(suncg6, value32);
      break;

      /* a write of the rectangle absolute coordinate registers: */
    case TME_SUNCG6_REG_FBC(1,rect_abs_x):
      suncg6->tme_suncg6_fbc_rect_abs_x_i
	= ((suncg6->tme_suncg6_fbc_rect_abs_x_i + 1)
	   % TME_SUNCG6_RECT_ABS_COUNT);
      _reg = &suncg6->tme_suncg6_fbc_rect_abs_x[suncg6->tme_suncg6_fbc_rect_abs_x_i];
      break;
    case TME_SUNCG6_REG_FBC(1,rect_abs_y):
      suncg6->tme_suncg6_fbc_rect_abs_y_i
	= ((suncg6->tme_suncg6_fbc_rect_abs_y_i + 1)
	   % TME_SUNCG6_RECT_ABS_COUNT);
      _reg = &suncg6->tme_suncg6_fbc_rect_abs_y[suncg6->tme_suncg6_fbc_rect_abs_y_i];
      break;

    default:
      break;
    }

    /* if this is a maintained register: */
    if (_reg != NULL) {
      *_reg
	= ((*_reg
	    & reg_mask_ro)
	   + (value32
	      & ~reg_mask_ro));
    }
  }

  /* otherwise, this is a read: */
  else {
    assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register: */
    switch (reg) {

      /* a read of the status register: */
    case TME_SUNCG6_REG_FBC(0,status):
      *_reg = 0;
      break;

      /* a read of the blit status register: */
    case TME_SUNCG6_REG_FBC(0,status_blit):

      /* trigger a blit: */
      _tme_suncg6_blit(suncg6);

      *_reg = 0;
      break;

      /* a read of the drawing status register: */
    case TME_SUNCG6_REG_FBC(0,status_draw):

      /* trigger a draw: */
      _tme_suncg6_draw(suncg6);

      *_reg = 0;
      break;

    default:
      break;
    }

    /* this must be a maintained register: */
    if (_reg == NULL) {
      abort();
    }
    value32 = *_reg;

    /* run the bus cycle: */
    value32_buffer = value32;
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
  }

  /* complete the cycle: */
  master_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(master_completion);
  *_master_fast_cycle_types = 0;

  tme_log(TME_SUNFB_LOG_HANDLE(sunfb), 2000, TME_OK,
	  (TME_SUNFB_LOG_HANDLE(sunfb),
	   _("FBC 0x%06" TME_PRIx32 " %s%s 0x%08" TME_PRIx32),
	   reg,
	   (_reg == NULL
	    ? "UNKNOWN "
	    : ""),
	   (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	    ? "<-"
	    : "->"),
	   value32));
}

/* the TEC bus cycle handler: */
static void
_tme_suncg6_bus_cycle_tec(struct tme_sunfb *sunfb,
			  struct tme_bus_cycle *master_cycle,
			  tme_uint32_t *_master_fast_cycle_types,
			  struct tme_completion *master_completion)
{
  struct tme_suncg6 *suncg6;
  tme_bus_addr32_t reg;
  tme_suncg6_reg_t *_reg;
  tme_suncg6_reg_t value32_buffer;
  tme_suncg6_reg_t value32;
  tme_suncg6_reg_t reg_mask_ro;

  /* recover our data structure: */
  suncg6 = (struct tme_suncg6 *) sunfb;

  /* get the register: */
  reg = master_cycle->tme_bus_cycle_address;

  /* convert the register into a pointer: */
  _reg
    = (TME_SUNCG6_REG_POINTER(suncg6, reg,TEC,tec,0)
       : NULL);

  /* if this is a write: */
  if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
    value32 = value32_buffer;

    /* assume that none of the register is read-only: */
    reg_mask_ro = 0;

    /* dispatch on the register: */
    switch (reg) {

    default:
      break;
    }

    /* if this is a maintained register: */
    if (_reg != NULL) {
      *_reg
	= ((*_reg
	    & reg_mask_ro)
	   + (value32
	      & ~reg_mask_ro));
    }
  }

  /* otherwise, this is a read: */
  else {
    assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register: */
    switch (reg) {

    default:
      break;
    }

    /* this must be a maintained register: */
    if (_reg == NULL) {
      abort();
    }
    value32 = *_reg;

    /* run the bus cycle: */
    value32_buffer = value32;
    tme_bus_cycle_xfer_reg(master_cycle, 
			   &value32_buffer,
			   TME_BUS32_LOG2);
  }

  /* complete the cycle: */
  master_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(master_completion);
  *_master_fast_cycle_types = 0;

  tme_log(TME_SUNFB_LOG_HANDLE(sunfb), 2000, TME_OK,
	  (TME_SUNFB_LOG_HANDLE(sunfb),
	   _("TEC 0x%06" TME_PRIx32 " %s%s 0x%08" TME_PRIx32),
	   reg,
	   (_reg == NULL
	    ? "UNKNOWN "
	    : ""),
	   (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
	    ? "<-"
	    : "->"),
	   value32));
}

#if TME_SUNFB_BUS_TRANSITION

/* this is the bus cycle transition glue: */
#define _tme_suncg6_bus_cycle_transition(regs)						\
static int										\
_TME_CONCAT(_tme_suncg6_bus_cycle_transition_,regs)(void *_sunfb,			\
						    struct tme_bus_cycle *master_cycle)	\
{											\
  return (tme_sunfb_bus_cycle_transition(_sunfb,					\
					 master_cycle,					\
					 _TME_CONCAT(_tme_suncg6_bus_cycle_,regs)));	\
}											\
struct _tme_suncg6_bus_cycle_transition
_tme_suncg6_bus_cycle_transition(alt);
_tme_suncg6_bus_cycle_transition(fhc_thc);
_tme_suncg6_bus_cycle_transition(fbc);
_tme_suncg6_bus_cycle_transition(tec);
#define _tme_suncg6_bus_cycle_alt _tme_suncg6_bus_cycle_transition_alt
#define _tme_suncg6_bus_cycle_fhc_thc _tme_suncg6_bus_cycle_transition_fhc_thc
#define _tme_suncg6_bus_cycle_fbc _tme_suncg6_bus_cycle_transition_fbc
#define _tme_suncg6_bus_cycle_tec _tme_suncg6_bus_cycle_transition_tec
#undef _tme_suncg6_bus_cycle_transition

#endif /* TME_SUNFB_BUS_TRANSITION */

/* this sets the cgsix type: */
static const char *
_tme_suncg6_type_set(struct tme_sunfb *sunfb, const char *cg6_type_string)
{
  struct tme_suncg6 *suncg6;
  tme_uint32_t cg6_type;
  tme_suncg6_reg_t fhc;
  tme_suncg6_reg_t thc_misc;

  /* recover our data structure: */
  suncg6 = (struct tme_suncg6 *) sunfb;

  /* see if this is a good type: */
  cg6_type = TME_SUNCG6_TYPE_NULL;
  if (TME_ARG_IS(cg6_type_string, "501-2325")) {
    cg6_type = TME_SUNCG6_TYPE_501_2325;
  }

  /* set the new type: */
  suncg6->tme_suncg6_type = cg6_type;

  /* zero various registers: */
  fhc = 0;
  thc_misc = 0;

  /* dispatch on the new type: */
  switch (cg6_type) {

    /* if this was a bad type, return a string of types: */
  default: assert(FALSE);
  case TME_SUNCG6_TYPE_NULL:
    return ("501-2325");

  case TME_SUNCG6_TYPE_501_2325:
    TME_FIELD_MASK_DEPOSITU(fhc, TME_SUNCG6_FHC_ID_MASK, 0x63);
    TME_FIELD_MASK_DEPOSITU(fhc, TME_SUNCG6_FHC_REVISION_MASK, 0xb);
    TME_FIELD_MASK_DEPOSITU(thc_misc, TME_SUNCG6_THC_MISC_REVISION_MASK, 0x4);
    sunfb->tme_sunfb_bus_signal_int = TME_BUS_SIGNAL_INT(5);
    break;
  }

  /* set various initial registers: */
  suncg6->tme_suncg6_fhc = fhc;
  suncg6->tme_suncg6_thc_group1.tme_suncg6_thc_misc = thc_misc;

  /* success: */
  return (NULL);
}

/* the new sun cgsix function: */
int
tme_sun_cgsix(struct tme_element *element, const char * const *args, char **_output)
{
  struct tme_suncg6 *suncg6;
  struct tme_sunfb *sunfb;
  tme_uint8_t *cmap;
  int rc;
  tme_suncg6_reg_t fhc;

  /* start the suncg6 structure: */
  suncg6 = tme_new0(struct tme_suncg6, 1);
  suncg6->tme_suncg6_sunfb.tme_sunfb_element = element;

  /* initialize the sunfb structure: */
  sunfb = &suncg6->tme_suncg6_sunfb;
  sunfb->tme_sunfb_class = TME_FB_XLAT_CLASS_COLOR;
  sunfb->tme_sunfb_depth = 8;
  sunfb->tme_sunfb_type_set = _tme_suncg6_type_set;
  sunfb->tme_sunfb_flags
    |= ((!TME_SUNFB_FLAG_BT458_CMAP_PACKED)
	| TME_SUNFB_FLAG_BT458_BYTE_D24_D31);

  /* possible cgsix framebuffer sizes: */
  suncg6->tme_suncg6_sunfb.tme_sunfb_size
    = (TME_SUNFB_SIZE_1024_768
       | TME_SUNFB_SIZE_1152_900
       | TME_SUNFB_SIZE_1280_1024
       | TME_SUNFB_SIZE_1600_1280);

  /* assume that this is not a P4 cgsix: */
  suncg6->tme_suncg6_bus_subregion_p4.tme_bus_subregion_address_first = 1;
  suncg6->tme_suncg6_bus_subregion_p4.tme_bus_subregion_address_last = 0;
  suncg6->tme_suncg6_bus_handler_p4 = NULL;

  /* the DAC registers: */
  suncg6->tme_suncg6_bus_subregion_dac.tme_bus_subregion_address_first = TME_SUNCG6_REG_DAC;
  suncg6->tme_suncg6_bus_subregion_dac.tme_bus_subregion_address_last = TME_SUNCG6_REG_ALT - 1;
  suncg6->tme_suncg6_bus_handler_dac = tme_sunfb_bus_cycle_bt458;

  /* the ALT registers: */
  suncg6->tme_suncg6_bus_subregion_alt.tme_bus_subregion_address_first = TME_SUNCG6_REG_ALT;
  suncg6->tme_suncg6_bus_subregion_alt.tme_bus_subregion_address_last = TME_SUNCG6_REG_ALT + TME_SUNCG6_SIZ_ALT - 1;
  suncg6->tme_suncg6_bus_handler_alt = _tme_suncg6_bus_cycle_alt;

  /* the FHC and THC registers: */
  suncg6->tme_suncg6_bus_subregion_fhc_thc.tme_bus_subregion_address_first = TME_SUNCG6_REG_FHC;
  suncg6->tme_suncg6_bus_subregion_fhc_thc.tme_bus_subregion_address_last
    = (TME_SUNCG6_REG_THC_UNKNOWN_0x0
       + TME_SUNCG6_SIZ_THC
       - 1);
  suncg6->tme_suncg6_bus_handler_fhc_thc = _tme_suncg6_bus_cycle_fhc_thc;

  /* the FBC registers: */
  suncg6->tme_suncg6_bus_subregion_fbc.tme_bus_subregion_address_first
    = TME_SUNCG6_REG_FBC_GROUP0;
  suncg6->tme_suncg6_bus_subregion_fbc.tme_bus_subregion_address_last
    = TME_SUNCG6_REG_TEC_GROUP0 - 1;
  suncg6->tme_suncg6_bus_handler_fbc = _tme_suncg6_bus_cycle_fbc;

  /* the TEC registers: */
  suncg6->tme_suncg6_bus_subregion_tec.tme_bus_subregion_address_first
    = TME_SUNCG6_REG_TEC_GROUP0;
  suncg6->tme_suncg6_bus_subregion_tec.tme_bus_subregion_address_last
    = (TME_SUNCG6_REG_TEC_GROUP0
       + TME_SUNCG6_TEC_SIZE
       - 1);
  suncg6->tme_suncg6_bus_handler_tec = _tme_suncg6_bus_cycle_tec;

  /* the memory address: */
  sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first = TME_SUNFB_S4_OFFSET_MEMORY;

  /* if the generic initialization fails: */
  rc = tme_sunfb_new(sunfb, args, _output);
  if (rc) {

    /* free the sunfb structure and return the error: */
    tme_free(sunfb);
    return (rc);
  }

  /* set the size in the initial FHC: */
  fhc = suncg6->tme_suncg6_fhc;
  fhc &= (TME_SUNCG6_FHC_ID_MASK
	  + TME_SUNCG6_FHC_REVISION_MASK);
  switch (sunfb->tme_sunfb_size) {
  default: assert(FALSE);
  case TME_SUNFB_SIZE_1152_900: fhc += TME_SUNCG6_FHC_SIZE_1152_900; break;
  case TME_SUNFB_SIZE_1024_768: fhc += TME_SUNCG6_FHC_SIZE_1024_768; break;
  case TME_SUNFB_SIZE_1280_1024: fhc += TME_SUNCG6_FHC_SIZE_1280_1024; break;
  case TME_SUNFB_SIZE_1600_1280: fhc += TME_SUNCG6_FHC_SIZE_1600_1280; break;
  }
  suncg6->tme_suncg6_fhc = fhc;

  /* save the size: */
  suncg6->tme_suncg6_width = tme_sunfb_size_width(suncg6->tme_suncg6_sunfb.tme_sunfb_size);
  suncg6->tme_suncg6_height = tme_sunfb_size_height(suncg6->tme_suncg6_sunfb.tme_sunfb_size);

  /* allocate the colormap arrays: */
  cmap = tme_new0(tme_uint8_t, 256 * 3);
  sunfb->tme_sunfb_cmap_g = &cmap[256 * 0];
  sunfb->tme_sunfb_cmap_r = &cmap[256 * 1];
  sunfb->tme_sunfb_cmap_b = &cmap[256 * 2];
  sunfb->tme_sunfb_bt458.tme_bt458_cmap_g = sunfb->tme_sunfb_cmap_g;
  sunfb->tme_sunfb_bt458.tme_bt458_cmap_r = sunfb->tme_sunfb_cmap_r;
  sunfb->tme_sunfb_bt458.tme_bt458_cmap_b = sunfb->tme_sunfb_cmap_b;

  return (TME_OK);
}
