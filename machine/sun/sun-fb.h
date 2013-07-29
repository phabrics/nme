/* $Id: sun-fb.h,v 1.4 2009/11/08 17:02:39 fredette Exp $ */

/* machine/sun/sun-fb.h - header file for Sun framebuffer emulation: */

/*
 * Copyright (c) 2004, 2006 Matt Fredette
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

#ifndef _MACHINE_SUN_FB_H
#define _MACHINE_SUN_FB_H

#include <tme/common.h>
_TME_RCSID("$Id: sun-fb.h,v 1.4 2009/11/08 17:02:39 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>
#include <tme/generic/bus-device.h>
#include <tme/generic/fb.h>
#include <tme/ic/bt458.h>
#include <tme/completion.h>

/* macros: */

/* Sun framebuffer sizes: */
/* NB: TME_SUNFB_SIZE_1152_900 should be first, since whatever size is
   first is what most framebuffers will default to: */
#define TME_SUNFB_SIZE_NULL			(0)
#define TME_SUNFB_SIZE_1152_900			TME_BIT(0)
#define TME_SUNFB_SIZE_1024_1024		TME_BIT(1)
#define TME_SUNFB_SIZE_1280_1024		TME_BIT(2)
#define TME_SUNFB_SIZE_1600_1280		TME_BIT(3)
#define TME_SUNFB_SIZE_1440_1440		TME_BIT(4)
#define TME_SUNFB_SIZE_1024_768			TME_BIT(5)
#define TME_SUNFB_SIZE_640_480			TME_BIT(6)

/* P4 register framebuffer identifiers: */
#define TME_SUNFB_P4_ID_MASK			(0xf0000000)
#define  TME_SUNFB_P4_ID_BWTWO			(0x00000000)
#define  TME_SUNFB_P4_ID_CGFOUR			(0x40000000)
#define  TME_SUNFB_P4_ID_CGEIGHT		(0x45000000)
#define  TME_SUNFB_P4_ID_CGSIX			(0x60000000)

/* offsets in many P4 framebuffers: */
#define TME_SUNFB_P4_OFFSET_P4			(0x00000000)
#define TME_SUNFB_P4_OFFSET_BITMAP		(0x00100000)

/* offsets in many S4 framebuffers: */
#define TME_SUNFB_S4_OFFSET_REGS		(0x00400000)
#define TME_SUNFB_S4_OFFSET_MEMORY		(0x00800000)

/* flags: */
#define TME_SUNFB_FLAG_BT458_CMAP_PACKED	TME_BIT(0)
#define TME_SUNFB_FLAG_BT458_BYTE_D0_D7		(0)
#define TME_SUNFB_FLAG_BT458_BYTE_D24_D31	TME_BIT(1)

/* callout flags: */
#define TME_SUNFB_CALLOUT_RUNNING		TME_BIT(0)
#define TME_SUNFB_CALLOUTS_MASK			(-2)
#define  TME_SUNFB_CALLOUT_MODE_CHANGE		TME_BIT(1)
#define	 TME_SUNFB_CALLOUT_INT			TME_BIT(2)

/* the maximum number of bus subregions for registers that a Sun
   framebuffer can have: */
#define TME_SUNFB_BUS_SUBREGIONS_MAX	(8)

/* the log handle: */
#define TME_SUNFB_LOG_HANDLE(sunfb)		(&(sunfb)->tme_sunfb_element->tme_element_log_handle)

#define TME_SUNFB_BUS_TRANSITION		(1)

/* a Sun framebuffer: */
struct tme_sunfb {

  /* our simple bus device header: */
  struct tme_bus_device tme_sunfb_device;
#define tme_sunfb_element tme_sunfb_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_sunfb_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_sunfb_rwlock;

  /* the framebuffer connection: */
  struct tme_fb_connection *tme_sunfb_fb_connection;

  /* more bus subregions: */
  struct tme_bus_subregion tme_sunfb_bus_subregions[TME_SUNFB_BUS_SUBREGIONS_MAX];

  /* bus cycle handlers for the subregions: */
  tme_bus_cycle_handler tme_sunfb_bus_handlers[TME_SUNFB_BUS_SUBREGIONS_MAX];

  /* some of the bus subregions have specific purposes: */
#define tme_sunfb_bus_subregion_memory tme_sunfb_device.tme_bus_device_subregions
#define tme_sunfb_bus_subregion_regs tme_sunfb_bus_subregions[0]
#define tme_sunfb_bus_handler_regs tme_sunfb_bus_handlers[0]

  /* the class of the framebuffer: */
  unsigned int tme_sunfb_class;

  /* the depth of the framebuffer: */
  unsigned int tme_sunfb_depth;

  /* the size of the framebuffer: */
  tme_uint32_t tme_sunfb_size;

  /* framebuffer flags: */
  tme_uint32_t tme_sunfb_flags;

  /* the callout flags: */
  int tme_sunfb_callout_flags;

  /* this is nonzero if the interrupt is asserted: */
  int tme_sunfb_int_asserted;

  /* the callout thread condition: */
  tme_cond_t tme_sunfb_callout_cond;

  /* the (relative) bus address of the last byte of displayed
     framebuffer memory: */
  tme_bus_addr32_t tme_sunfb_memory_address_last_displayed;

  /* the memory.  usually, this memory is displayed directly, but this
     won't be the case when there is an overlay plane, for example: */
  tme_uint8_t *tme_sunfb_memory;

  /* any memory pad: */
  tme_uint8_t *tme_sunfb_memory_pad;

  /* a framebuffer memory update function: */
  int (*tme_sunfb_memory_update)(struct tme_fb_connection *conn_fb);

  /* this forces the next update to be a full one: */
  void (*tme_sunfb_update_full) _TME_P((struct tme_sunfb *));

  /* the token for one outstanding writable TLB entry: */
  struct tme_token *tme_sunfb_tlb_token;

  /* the offsets of the first and last bytes updated in the real
     framebuffer memory: */
  tme_uint32_t tme_sunfb_offset_updated_first;
  tme_uint32_t tme_sunfb_offset_updated_last;

  /* these are used for index-mapping pixel values or pixel subfield
     values to intensities, or vice-versa.  if these are NULL,
     everything is linearly mapped: */
  void *tme_sunfb_cmap_primaries[3];
#define tme_sunfb_cmap_g tme_sunfb_cmap_primaries[0]
#define tme_sunfb_cmap_r tme_sunfb_cmap_primaries[1]
#define tme_sunfb_cmap_b tme_sunfb_cmap_primaries[2]

  /* a P4 register: */
  tme_uint32_t tme_sunfb_p4;

  /* many Sun 8-bit framebuffers use the Brooktree Bt458 RAMDAC: */
  struct tme_bt458 tme_sunfb_bt458;

  /* S4 basic registers: */
  struct {
    tme_uint8_t tme_sunfb_s4_regs_control;
#define tme_sunfb_s4_regs_first tme_sunfb_s4_regs_control
    tme_uint8_t tme_sunfb_s4_regs_status;
    tme_uint8_t tme_sunfb_s4_regs_cursor_start;
    tme_uint8_t tme_sunfb_s4_regs_cursor_end;
    tme_uint8_t tme_sunfb_s4_regs_h_blank_set;
    tme_uint8_t tme_sunfb_s4_regs_h_blank_clear;
    tme_uint8_t tme_sunfb_s4_regs_h_sync_set;
    tme_uint8_t tme_sunfb_s4_regs_h_sync_clear;
    tme_uint8_t tme_sunfb_s4_regs_comp_sync_clear;
    tme_uint8_t tme_sunfb_s4_regs_v_blank_set_high;
    tme_uint8_t tme_sunfb_s4_regs_v_blank_set_low;
    tme_uint8_t tme_sunfb_s4_regs_v_blank_clear;
    tme_uint8_t tme_sunfb_s4_regs_v_sync_set;
    tme_uint8_t tme_sunfb_s4_regs_v_sync_clear;
    tme_uint8_t tme_sunfb_s4_regs_xfer_holdoff_set;
    tme_uint8_t tme_sunfb_s4_regs_xfer_holdoff_clear;
  } tme_sunfb_s4_regs;

  /* if the given type is valid, it returns NULL and updates the
     framebuffer structure, else it returns a string of valid types: */
  const char *(*tme_sunfb_type_set) _TME_P((struct tme_sunfb *, const char *));

  /* any interrupt signal: */
  tme_uint32_t tme_sunfb_bus_signal_int;
};

/* prototypes: */

/* miscellaneous: */
tme_uint32_t tme_sunfb_size _TME_P((const char *));
tme_uint32_t tme_sunfb_size_width _TME_P((tme_uint32_t));
tme_uint32_t tme_sunfb_size_height _TME_P((tme_uint32_t));

/* this creates a new Sun framebuffer: */
int tme_sunfb_new(struct tme_sunfb *sunfb, const char * const *args, char **_output);

/* some standard register bus cycle handlers: */
int tme_sunfb_bus_cycle_p4 _TME_P((void *, struct tme_bus_cycle *));
int tme_sunfb_bus_cycle_s4 _TME_P((void *, struct tme_bus_cycle *));
int tme_sunfb_bus_cycle_bt458 _TME_P((void *, struct tme_bus_cycle *));

/* this is called before the framebuffer's display is updated: */
int tme_sunfb_memory_update _TME_P((struct tme_fb_connection *));

#if TME_SUNFB_BUS_TRANSITION

/* this is the bus cycle transition glue: */
struct tme_completion;
int tme_sunfb_bus_cycle_transition _TME_P((void *,
					   struct tme_bus_cycle *,
					   void (*) _TME_P((struct tme_sunfb *,
							    struct tme_bus_cycle *,
							    tme_uint32_t *,
							    struct tme_completion *))));

#endif /* TME_SUNFB_BUS_TRANSITION */

#endif /* !_MACHINE_SUN_FB_H */
