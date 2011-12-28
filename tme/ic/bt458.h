/* $Id: bt458.h,v 1.1 2007/01/07 23:22:21 fredette Exp $ */

/* tme/ic/bt458.h - public header file for Brooktree Bt458 (and TI TLC34058) emulation */

/*
 * Copyright (c) 2006 Matt Fredette
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

#ifndef _TME_IC_BT458_H
#define _TME_IC_BT458_H

#include <tme/common.h>
_TME_RCSID("$Id: bt458.h,v 1.1 2007/01/07 23:22:21 fredette Exp $");

/* macros: */

/* the bt458 control register addresses: */
#define TME_BT458_REG_CONTROL_MASK_READ		(0x04)
#define TME_BT458_REG_CONTROL_MASK_BLINK	(0x05)
#define TME_BT458_REG_CONTROL_COMMAND		(0x06)
#define TME_BT458_REG_CONTROL_TEST		(0x07)

#define TME_BT458_REG_CONTROL_FIRST		(TME_BT458_REG_CONTROL_MASK_READ)
#define TME_BT458_REG_CONTROL_LAST		(TME_BT458_REG_CONTROL_TEST)

/* the bt458 command register: */
#define TME_BT458_COMMAND_5PIXELS		TME_BIT(7)
#define TME_BT458_COMMAND_ENABLE_PALETTE	TME_BIT(6)
#define TME_BT458_COMMAND_BLINK_RATE_MASK	(0x30)
#define  TME_BT458_COMMAND_BLINK_RATE_16_48	 (0x00)
#define  TME_BT458_COMMAND_BLINK_RATE_16_16	 (0x10)
#define  TME_BT458_COMMAND_BLINK_RATE_32_32	 (0x20)
#define  TME_BT458_COMMAND_BLINK_RATE_64_64	 (0x30)
#define TME_BT458_COMMAND_ENABLE_BLINK_OL1	TME_BIT(3)
#define TME_BT458_COMMAND_ENABLE_BLINK_OL0	TME_BIT(2)
#define TME_BT458_COMMAND_ENABLE_DISPLAY_OL1	TME_BIT(1)
#define TME_BT458_COMMAND_ENABLE_DISPLAY_OL0	TME_BIT(0)

/* the bt458 state: */ 
struct tme_bt458 {

  /* the bt458 address register: */
  tme_uint8_t tme_bt458_address;

  /* the bt458 rgb address register: */
  tme_uint8_t tme_bt458_rgb;

  /* the bt458 registers: */
  tme_uint8_t tme_bt458_regs[TME_BT458_REG_CONTROL_LAST + 1 - TME_BT458_REG_CONTROL_FIRST];

  /* the bt458 colormap: */
  tme_uint8_t *tme_bt458_cmap_primaries[3];
#define tme_bt458_cmap_r tme_bt458_cmap_primaries[0]
#define tme_bt458_cmap_g tme_bt458_cmap_primaries[1]
#define tme_bt458_cmap_b tme_bt458_cmap_primaries[2]

  /* the bt458 overlay map: */
  tme_uint8_t tme_bt458_omap_primaries[3][4];
#define tme_bt458_omap_r tme_bt458_omap_primaries[0]
#define tme_bt458_omap_g tme_bt458_omap_primaries[1]
#define tme_bt458_omap_b tme_bt458_omap_primaries[2]

  /* the closest regular colormap indices for the overlay map: */
  tme_uint8_t tme_bt458_omap_cmap_indices[4];
};

/* prototypes: */

/* this updates the best regular colormap indices for the colors in
   the overlay map.  it returns nonzero if any of them changed: */
int tme_bt458_omap_best _TME_P((struct tme_bt458 *));

#endif /* !_TME_IC_BT458_H */
