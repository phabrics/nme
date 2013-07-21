/* $Id: sun-fb.c,v 1.6 2010/06/05 19:19:01 fredette Exp $ */

/* machine/sun/sun-fb.c - Sun framebuffer emulation support: */

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

#include <tme/common.h>
_TME_RCSID("$Id: sun-fb.c,v 1.6 2010/06/05 19:19:01 fredette Exp $");

/* includes: */
#include "sun-fb.h"

/* macros: */

/* P4 register framebuffer sizes: */
#define TME_SUNFB_P4_SIZE_MASK		(0x0f000000)
#define  TME_SUNFB_P4_SIZE_1600_1280	(0x00000000)
#define  TME_SUNFB_P4_SIZE_1152_900	(0x01000000)
#define  TME_SUNFB_P4_SIZE_1024_1024	(0x02000000)
#define  TME_SUNFB_P4_SIZE_1280_1024	(0x03000000)
#define  TME_SUNFB_P4_SIZE_1440_1440	(0x04000000)
#define  TME_SUNFB_P4_SIZE_640_480	(0x05000000)

/* P4 register bits: */
					/* 0x00000080 is the diagnostic bit (?) */
					/* 0x00000040 is the readback bit (?) */
#define TME_SUNFB_P4_REG_VIDEO_ENABLE	(0x00000020)
#define TME_SUNFB_P4_REG_SYNC_RAMDAC	(0x00000010)
#define TME_SUNFB_P4_REG_IN_VTRACE	(0x00000008)
#define TME_SUNFB_P4_REG_INT_ACTIVE	(0x00000004)
#define TME_SUNFB_P4_REG_INT_RESET	(0x00000004)
#define TME_SUNFB_P4_REG_ENABLE_INT	(0x00000002)
#define TME_SUNFB_P4_REG_IN_VTRACE_1H	(0x00000001)
#define TME_SUNFB_P4_REG_RESET		(0x00000001)

/* P4 register read-only bits: */
#define TME_SUNFB_P4_RO_MASK		(TME_SUNFB_P4_ID_MASK \
					 | TME_SUNFB_P4_SIZE_MASK \
					 | TME_SUNFB_P4_REG_IN_VTRACE \
					 | TME_SUNFB_P4_REG_INT_ACTIVE \
					 | TME_SUNFB_P4_REG_IN_VTRACE_1H)

/* S4 register offsets: */
#define TME_SUNFB_S4_REG_BT458_ADDRESS	(0)
#define TME_SUNFB_S4_REG_BT458_CMAP	(4)
#define TME_SUNFB_S4_REG_BT458_CONTROL	(8)
#define TME_SUNFB_S4_REG_BT458_OMAP	(12)
#define TME_SUNFB_S4_SIZ_BT458		(16)
#define TME_SUNFB_S4_SIZ_REGS		(32)
#define TME_SUNFB_S4_REG(x)				\
  (TME_SUNFB_S4_SIZ_BT458				\
   + (&((struct tme_sunfb *) 0)->tme_sunfb_s4_regs.x	\
      - &((struct tme_sunfb *) 0)->tme_sunfb_s4_regs.tme_sunfb_s4_regs_first))

/* S4 control register bits: */
#define TME_SUNFB_S4_CONTROL_INT_ENABLE		(0x80)
#define TME_SUNFB_S4_CONTROL_VIDEO_ENABLE	(0x40)

/* S4 status register bits: */
#define TME_SUNFB_S4_STATUS_INT_PENDING		(0x80)
#define TME_SUNFB_S4_STATUS_SIZE_MASK		(0x70)
#define  TME_SUNFB_S4_STATUS_SIZE_1024_768	 (0x10)
#define  TME_SUNFB_S4_STATUS_SIZE_1152_900	 (0x30)
#define  TME_SUNFB_S4_STATUS_SIZE_1280_1024	 (0x40)
#define  TME_SUNFB_S4_STATUS_SIZE_1600_1280	 (0x50)
#define TME_SUNFB_S4_STATUS_ID_MASK		(0x0f)
#define  TME_SUNFB_S4_STATUS_ID_COLOR		 (0x01)
#define  TME_SUNFB_S4_STATUS_ID_MONO		 (0x02)
#define  TME_SUNFB_S4_STATUS_ID_MONO_ECL	 (0x03)

/* these evaluate to nonzero for different kinds of framebuffers: */
#define _TME_SUNFB_IS_BWTWO(sunfb)	((sunfb)->tme_sunfb_class == TME_FB_XLAT_CLASS_MONOCHROME)
#define _TME_SUNFB_IS_P4(sunfb)		((sunfb)->tme_sunfb_bus_handler_regs == tme_sunfb_bus_cycle_p4)
#define _TME_SUNFB_IS_S4(sunfb)		((sunfb)->tme_sunfb_bus_handler_regs == tme_sunfb_bus_cycle_s4)
#define _TME_SUNFB_HAS_BT458(sunfb)	((sunfb)->tme_sunfb_depth == 8)

/* we fill writable TLB entries for framebuffer memory for this many
   bytes at a time: */
#define TME_SUNFB_UPDATE_SIZE		(1024)

#if 0
#define TME_SUNFB_DEBUG
#endif

/* this returns the sunfb size value for the given resolution: */
tme_uint32_t
tme_sunfb_size(const char *size)
{
  if (TME_ARG_IS(size, "1600x1280")) {
    return (TME_SUNFB_SIZE_1600_1280);
  }
  else if (TME_ARG_IS(size, "1152x900")) {
    return (TME_SUNFB_SIZE_1152_900);
  }
  else if (TME_ARG_IS(size, "1024x1024")) {
    return (TME_SUNFB_SIZE_1024_1024);
  }
  else if (TME_ARG_IS(size, "1280x1024")) {
    return (TME_SUNFB_SIZE_1280_1024);
  }
  else if (TME_ARG_IS(size, "1440x1440")) {
    return (TME_SUNFB_SIZE_1440_1440);
  }
  else if (TME_ARG_IS(size, "640x480")) {
    return (TME_SUNFB_SIZE_640_480);
  }
  else if (TME_ARG_IS(size, "1024x768")) {
    return (TME_SUNFB_SIZE_1024_768);
  }
  return (TME_SUNFB_SIZE_NULL);
}

/* this returns the width for the given sunfb size: */
tme_uint32_t
tme_sunfb_size_width(tme_uint32_t sunfb_size)
{
  switch (sunfb_size) {
  default: assert(FALSE);
  case TME_SUNFB_SIZE_640_480: return (640);
  case TME_SUNFB_SIZE_1024_768: /* FALLTHROUGH */
  case TME_SUNFB_SIZE_1024_1024: return (1024);
  case TME_SUNFB_SIZE_1152_900: return (1152);
  case TME_SUNFB_SIZE_1280_1024: return (1280);
  case TME_SUNFB_SIZE_1600_1280: return (1600);
  case TME_SUNFB_SIZE_1440_1440: return (1440);
  }
}

/* this returns the height for the given sunfb size: */
tme_uint32_t
tme_sunfb_size_height(tme_uint32_t sunfb_size)
{
  switch (sunfb_size) {
  default: assert(FALSE);
  case TME_SUNFB_SIZE_640_480: return (480);
  case TME_SUNFB_SIZE_1024_768: return (768);
  case TME_SUNFB_SIZE_1152_900: return (900);
  case TME_SUNFB_SIZE_1024_1024: /* FALLTHROUGH */
  case TME_SUNFB_SIZE_1280_1024: return (1024);
  case TME_SUNFB_SIZE_1600_1280: return (1280);
  case TME_SUNFB_SIZE_1440_1440: return (1440);
  }
}

/* XXX FIXME - this should be in a tme/ic/bt458.c: */
/* this determines the closest regular colormap indices for the
   overlay map: */
int
tme_bt458_omap_best(struct tme_bt458 *bt458)
{
  unsigned int omap_i;
  unsigned int cmap_i;
  tme_int32_t score;
  unsigned int cmap_i_best;
  tme_int32_t score_best;
  tme_int32_t score_part;
  int changed;

  /* loop over the overlay map indices: */
  changed = FALSE;
  for (omap_i = 0;
       omap_i < TME_ARRAY_ELS(bt458->tme_bt458_omap_primaries[0]);
       omap_i++) {

    /* silence gcc -Wuninitialized: */
    cmap_i_best = 0;

    /* loop over the regular colormap indices: */
    score_best = (256 * 256 * 256);
    for (cmap_i = 0;
	 cmap_i < 256;
	 cmap_i++) {

      /* score this colormap entry, by taking the product of the
	 distances between this colormap entry's primaries and this
	 overlay map entry's primaries: */
      score = bt458->tme_bt458_omap_r[omap_i];
      score -= (bt458->tme_bt458_cmap_r)[cmap_i];
      score_part = bt458->tme_bt458_omap_g[omap_i];
      score_part -= (bt458->tme_bt458_cmap_g)[cmap_i];
      score *= score_part;
      score_part = bt458->tme_bt458_omap_b[omap_i];
      score_part -= (bt458->tme_bt458_cmap_b)[cmap_i];
      score *= score_part;
      if (score < 0) {
	score = -score;
      }

      /* update the best colormap entry: */
      if (score < score_best) {
	score_best = score;
	cmap_i_best = cmap_i;
      }
    }

    /* save the closest index: */
    changed |= bt458->tme_bt458_omap_cmap_indices[omap_i] - cmap_i_best;
    bt458->tme_bt458_omap_cmap_indices[omap_i] = cmap_i_best;
  }

  return (changed);
}

/* this handles a mode change callout: */
static int
_tme_sunfb_mode_change(struct tme_sunfb *sunfb)
{
  struct tme_fb_connection *conn_fb_other;
  struct tme_fb_connection *conn_fb;
  int rc;

  /* if this framebuffer has a Bt458: */
  if (_TME_SUNFB_HAS_BT458(sunfb)) {

    /* update the best regular colormap indices for the colors in the
       overlay map.  if any of them have changed, we may have do to a
       full update: */
    if (tme_bt458_omap_best(&sunfb->tme_sunfb_bt458)) {

      /* if this framebuffer has an update-full function, call it: */
      if (sunfb->tme_sunfb_update_full != NULL) {
	(*sunfb->tme_sunfb_update_full)(sunfb);
      }
    }
  }

  /* get both sides of the framebuffer connection: */
  conn_fb_other = sunfb->tme_sunfb_fb_connection;
  conn_fb = (struct tme_fb_connection *) conn_fb_other->tme_fb_connection.tme_connection_other;

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);
      
  /* do the callout: */
  rc = (conn_fb_other != NULL
	? ((*conn_fb_other->tme_fb_connection_mode_change)
	   (conn_fb_other))
	: TME_OK);
      
  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  return (rc);
}

/* the sunfb callout function.  it must be called with the mutex locked: */
static void
_tme_sunfb_callout(struct tme_sunfb *sunfb)
{
  struct tme_bus_connection *conn_bus;
  int int_asserted;
  int callouts_blocked;
  int rc;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (sunfb->tme_sunfb_callout_flags
      & TME_SUNFB_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  sunfb->tme_sunfb_callout_flags
    |= TME_SUNFB_CALLOUT_RUNNING;

  /* initially, no callouts are blocked: */
  callouts_blocked = 0;

  /* loop forever: */
  for (;;) {

    /* any callout, successful or not, will clear this bit.  if we get
       to the bottom of the loop and this bit is still set, there are
       no more (unblocked) callouts to make, so we can stop: */
    callouts_blocked |= TME_SUNFB_CALLOUT_RUNNING;

    /* we always clear the interrupt callout flag, because we only
       need it to get callouts to run at all: */
    sunfb->tme_sunfb_callout_flags &= ~TME_SUNFB_CALLOUT_INT;

    /* if our interrupt signal has changed, and this callout isn't
       blocked: */
    int_asserted
      = (_TME_SUNFB_IS_S4(sunfb)
	 ? ((sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_control
	     & TME_SUNFB_S4_CONTROL_INT_ENABLE)
	    && (sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_status
		& TME_SUNFB_S4_STATUS_INT_PENDING))
	 : FALSE);
    if ((!int_asserted != !sunfb->tme_sunfb_int_asserted)
	&& (callouts_blocked & TME_SUNFB_CALLOUT_INT) == 0) {

      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						sunfb->tme_sunfb_device.tme_bus_device_connection,
						&sunfb->tme_sunfb_device.tme_bus_device_connection_rwlock);

      /* unlock our mutex: */
      tme_mutex_unlock(&sunfb->tme_sunfb_mutex);
      
      /* call out the bus interrupt signal edge: */
      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 sunfb->tme_sunfb_bus_signal_int
	 | TME_BUS_SIGNAL_EDGE
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));

      /* lock our mutex: */
      tme_mutex_lock(&sunfb->tme_sunfb_mutex);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if this callout failed: */
      if (rc != TME_OK) {

	/* reschedule this callout but block it until some other
           callout succeeds: */
	sunfb->tme_sunfb_callout_flags |= TME_SUNFB_CALLOUT_INT;
	callouts_blocked |= TME_SUNFB_CALLOUT_INT;
	continue;
      }
      
      /* note the new state of the interrupt signal: */
      sunfb->tme_sunfb_int_asserted = int_asserted;
    }

    /* if we need to call out a mode change, and this callout isn't
       blocked: */
    if ((sunfb->tme_sunfb_callout_flags & TME_SUNFB_CALLOUT_MODE_CHANGE)
	&& (callouts_blocked & TME_SUNFB_CALLOUT_MODE_CHANGE) == 0) {

      /* clear this callout: */
      sunfb->tme_sunfb_callout_flags &= ~TME_SUNFB_CALLOUT_MODE_CHANGE;

      /* call out the mode change: */
      rc = _tme_sunfb_mode_change(sunfb);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if the callout failed: */
      if (rc != TME_OK) {

	/* reschedule this callout but block it until some other
           callout succeeds: */
	sunfb->tme_sunfb_callout_flags |= TME_SUNFB_CALLOUT_MODE_CHANGE;
	callouts_blocked |= TME_SUNFB_CALLOUT_MODE_CHANGE;
	continue;
      }
    }

    /* if no more (unblocked) callouts can run, we can stop: */
    if (callouts_blocked & TME_SUNFB_CALLOUT_RUNNING) {
      break;
    }
  }

  /* clear that callouts are running: */
  sunfb->tme_sunfb_callout_flags &= ~TME_SUNFB_CALLOUT_RUNNING;
}

/* the callout thread: */
static void
_tme_sunfb_callout_thread(void *_sunfb)
{
  struct tme_sunfb *sunfb;

  /* recover our data structure: */
  sunfb = _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* loop forever: */
  for (;;) {

    /* make any callouts: */
    _tme_sunfb_callout(sunfb);

    /* wait on the condition: */
    tme_cond_wait_yield(&sunfb->tme_sunfb_callout_cond,
			&sunfb->tme_sunfb_mutex);
  }
  /* NOTREACHED */
}

/* this is called before the framebuffer's display is updated: */
int
tme_sunfb_memory_update(struct tme_fb_connection *conn_fb)
{
  struct tme_sunfb *sunfb;
  int int_asserted;
  struct tme_token *tlb_token;

  /* recover our data structure: */
  sunfb = conn_fb->tme_fb_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* if this framebuffer supports interrupts, one is now pending.  the
     interrupt should also be asserted if it's enabled: */
  if (_TME_SUNFB_IS_S4(sunfb)) {
    sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_status
      |= TME_SUNFB_S4_STATUS_INT_PENDING;
    int_asserted
      = ((sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_control
	  & TME_SUNFB_S4_CONTROL_INT_ENABLE) != 0);
  }
  else {
    int_asserted = FALSE;
  }

  /* if this interrupt should be asserted and it isn't already, or if
     we have other callouts to make, notify the callout thread: */
  if ((int_asserted && !sunfb->tme_sunfb_int_asserted)
      || (sunfb->tme_sunfb_callout_flags & TME_SUNFB_CALLOUTS_MASK) != 0) {
    tme_cond_notify(&sunfb->tme_sunfb_callout_cond, FALSE);
  }
      
  /* set the offsets of the first and last bytes updated in the real
     framebuffer memory: */
  conn_fb->tme_fb_connection_offset_updated_first = sunfb->tme_sunfb_offset_updated_first;
  conn_fb->tme_fb_connection_offset_updated_last = sunfb->tme_sunfb_offset_updated_last;

  /* reset the offsets of the first and last bytes updated in the real
     framebuffer memory: */
  sunfb->tme_sunfb_offset_updated_first = 0 - (tme_uint32_t) 1;
  sunfb->tme_sunfb_offset_updated_last = 0;
  
  /* invalidate any outstanding writable TLB entry: */
  tlb_token = sunfb->tme_sunfb_tlb_token;
  if (tlb_token != NULL) {
    tme_token_invalidate(tlb_token);
    sunfb->tme_sunfb_tlb_token = NULL;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  return (TME_OK);
}

/* the sunfb memory bus cycle handler: */
static int
_tme_sunfb_bus_cycle_memory(void *_sunfb, struct tme_bus_cycle *cycle_init)
{
  struct tme_sunfb *sunfb;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* run the cycle: */
  assert (cycle_init->tme_bus_cycle_address
	  >= sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first);
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (sunfb->tme_sunfb_memory
			     - sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first),
			    sunfb->tme_sunfb_memory_address_last_displayed);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sunfb memory pad bus cycle handler: */
static int
_tme_sunfb_bus_cycle_memory_pad(void *_sunfb, struct tme_bus_cycle *cycle_init)
{
  struct tme_sunfb *sunfb;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* run the cycle: */
  assert (cycle_init->tme_bus_cycle_address
	  > sunfb->tme_sunfb_memory_address_last_displayed);
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (sunfb->tme_sunfb_memory_pad
			     - (sunfb->tme_sunfb_memory_address_last_displayed
				+ 1)),
			    sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last);

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sunfb P4 bus cycle handler: */
int
tme_sunfb_bus_cycle_p4(void *_sunfb, struct tme_bus_cycle *cycle_init)
{
  struct tme_sunfb *sunfb;
  tme_uint32_t p4_old, p4_new;
  tme_bus_addr32_t undecoded;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* get the old P4 value: */
  p4_old = tme_betoh_u32(sunfb->tme_sunfb_p4);

  /* the entire register bus subregion is all decoded (or, rather, not
     decoded) as the P4: */
  undecoded
    = (cycle_init->tme_bus_cycle_address
       & (sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_last
	  - sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first
	  - sizeof(sunfb->tme_sunfb_p4)));
  cycle_init->tme_bus_cycle_address
    -= undecoded;

  /* run the cycle: */
  assert (cycle_init->tme_bus_cycle_address
	  >= sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first);
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (((tme_uint8_t *) &sunfb->tme_sunfb_p4)
			     - sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first),
			    (sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first
			     + sizeof(sunfb->tme_sunfb_p4)
			     - 1));
  cycle_init->tme_bus_cycle_address
    += undecoded;

  /* get the new P4 value: */
  p4_new = tme_betoh_u32(sunfb->tme_sunfb_p4);

  /* put back the unchanging bits: */
  p4_new
    = ((p4_new
	& ~TME_SUNFB_P4_RO_MASK)
       | (p4_old
	  & TME_SUNFB_P4_RO_MASK));

  /* we do not support these bits: */
  if (p4_new
      & (TME_SUNFB_P4_REG_SYNC_RAMDAC
	 | TME_SUNFB_P4_REG_ENABLE_INT)) {
    abort();
  }

  /* set the new P4 value: */
  sunfb->tme_sunfb_p4 = tme_htobe_u32(p4_new);

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the Brooktree BT458 bus cycle handler: */
int
tme_sunfb_bus_cycle_bt458(void *_sunfb, struct tme_bus_cycle *cycle_init)
{
  struct tme_sunfb *sunfb;
  struct tme_bt458 *bt458;
  unsigned int reg;
  tme_uint32_t value_packed;
  tme_uint8_t value;
  unsigned int byte_count;
  unsigned int bt458_address;
  unsigned int bt458_rgb;
  unsigned int map_count;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* we only emulate 8-bit and aligned 32-bit accesses: */
  reg = cycle_init->tme_bus_cycle_address % TME_SUNFB_S4_SIZ_BT458;
  if (cycle_init->tme_bus_cycle_size != sizeof(tme_uint8_t)
      && (cycle_init->tme_bus_cycle_size != sizeof(tme_uint32_t)
	  || (reg % sizeof(tme_uint32_t)) != 0)) {
    abort();
  }
  reg &= (TME_SUNFB_S4_SIZ_BT458 - sizeof(tme_uint32_t));

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* get the Bt458 address registers: */
  bt458 = &sunfb->tme_sunfb_bt458;
  bt458_address = bt458->tme_bt458_address;
  bt458_rgb = bt458->tme_bt458_rgb;

  /* set the number of colormap values per read and write: */
  map_count = ((sunfb->tme_sunfb_flags & TME_SUNFB_FLAG_BT458_CMAP_PACKED)
	       ? cycle_init->tme_bus_cycle_size
	       : sizeof(tme_uint8_t));

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    if (cycle_init->tme_bus_cycle_size == sizeof(tme_uint32_t)) {
      tme_bus_cycle_xfer_reg(cycle_init, 
			     &value_packed,
			     TME_BUS32_LOG2);

      /* for a 32-bit write to an 8-bit register, a framebuffer either
	 uses byte lane D24..D31 or byte lane D0..D7: */
      value
	= (value_packed
	   >> (((sunfb->tme_sunfb_flags
		 & (TME_SUNFB_FLAG_BT458_BYTE_D0_D7
		    | TME_SUNFB_FLAG_BT458_BYTE_D24_D31))
		== TME_SUNFB_FLAG_BT458_BYTE_D24_D31)
	       ? 24
	       : 0));
    }
    else {
      tme_bus_cycle_xfer_reg(cycle_init,
			     &value,
			     TME_BUS8_LOG2);
      value_packed = value;
      value_packed <<= 24;
    }

    /* if this is a write to the address register: */
    if (reg == TME_SUNFB_S4_REG_BT458_ADDRESS) {

      /* set the Bt458 address register: */
      bt458_address = value;
    }

    /* if this is a write to the colormap register: */
    else if (reg == TME_SUNFB_S4_REG_BT458_CMAP) {
      
      /* write the colormap: */
      do {

	/* write one colormap primary: */
	(bt458->tme_bt458_cmap_primaries[bt458_rgb])[bt458_address] = value_packed >> 24;

	/* advance the packed value: */
	value_packed <<= 8;

	/* advance the Bt458 address registers: */
	bt458_rgb++;
	if (bt458_rgb == TME_ARRAY_ELS(bt458->tme_bt458_cmap_primaries)) {
	  bt458_address++;
	  bt458_rgb = 0;
	}
      } while (--map_count > 0);

      /* calling out a mode change on every colormap register write is
	 expensive and unnecessary.  instead, arrange for the update
	 function to eventually cause a mode change: */
      sunfb->tme_sunfb_callout_flags |= TME_SUNFB_CALLOUT_MODE_CHANGE;
    }

    /* if this is a write to the control register: */
    else if (reg == TME_SUNFB_S4_REG_BT458_CONTROL) {

      /* dispatch on the address: */
      switch (bt458_address) {

	/* the read mask register: */
      case TME_BT458_REG_CONTROL_MASK_READ:
	/* all planes must be on: */
	if (value != 0xff) {
	  abort();
	}
	break;

	/* the blink mask register: */
      case TME_BT458_REG_CONTROL_MASK_BLINK:
	/* no planes must be blinking: */
	if (value != 0x00) {
	  abort();
	}
	break;

	/* the command register: */
      case TME_BT458_REG_CONTROL_COMMAND:

	/* XXX FIXME - we should validate values written to the
           command register: */
	break;

	/* the test register: */
      case TME_BT458_REG_CONTROL_TEST:
	break;

      default:
	break;
      }

      /* the Bt458 datasheet says, "If an invalid address is loaded
	 into the address register, data written to the device will
	 be ignored and invalid data will be read by the MPU." */
      if (bt458_address >= TME_BT458_REG_CONTROL_FIRST
	  && bt458_address <= TME_BT458_REG_CONTROL_LAST) {
	
	/* write this register: */
	bt458->tme_bt458_regs[bt458_address - TME_BT458_REG_CONTROL_FIRST] = value;
      }
    }

    /* this must be a write to the overlay map register: */
    else {
      assert (reg == TME_SUNFB_S4_REG_BT458_OMAP);

      /* write the overlay map: */
      do {

	/* if the Bt458 address isn't in the overlay map: */
	if (bt458_address >= 4) {
	  abort();
	}

	/* write one overlay map primary: */
	bt458->tme_bt458_omap_primaries[bt458_rgb][bt458_address] = value_packed >> 24;

	/* advance the packed value: */
	value_packed <<= 8;

	/* advance the Bt458 address registers: */
	bt458_rgb++;
	if (bt458_rgb == TME_ARRAY_ELS(bt458->tme_bt458_omap_primaries)) {
	  bt458_address++;
	  bt458_rgb = 0;
	}
      } while (--map_count > 0);

      /* calling out a mode change on every overlap map register write
	 is expensive and unnecessary.  instead, arrange for the
	 update function to eventually cause a mode change: */
      sunfb->tme_sunfb_callout_flags |= TME_SUNFB_CALLOUT_MODE_CHANGE;
    }
  }

  /* otherwise, this is a read: */
  else {
    assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);
  
    /* zero the value read: */
    value_packed = 0;

    /* assume only one byte will be read: */
    byte_count = sizeof(tme_uint8_t);

    /* if this is a read of the address register: */
    if (reg == TME_SUNFB_S4_REG_BT458_ADDRESS) {

      /* read the Bt458 address register: */
      value_packed = bt458_address;
    }

    /* if this is a read to the colormap register: */
    else if (reg == TME_SUNFB_S4_REG_BT458_CMAP) {
      
      /* read the colormap: */
      byte_count = map_count;
      do {

	/* advance the packed value: */
	value_packed <<= 8;

	/* read one colormap primary: */
	value_packed |= (bt458->tme_bt458_cmap_primaries[bt458_rgb])[bt458_address];

	/* advance the Bt458 address registers: */
	bt458_rgb++;
	if (bt458_rgb == TME_ARRAY_ELS(bt458->tme_bt458_cmap_primaries)) {
	  bt458_address++;
	  bt458_rgb = 0;
	}
      } while (--map_count > 0);
    }
  
    /* if this is a read of the control register: */
    else if (reg == TME_SUNFB_S4_REG_BT458_CONTROL) {

      /* the Bt458 datasheet says, "If an invalid address is loaded
	 into the address register, data written to the device will
	 be ignored and invalid data will be read by the MPU." */
      if (bt458_address >= TME_BT458_REG_CONTROL_FIRST
	  && bt458_address <= TME_BT458_REG_CONTROL_LAST) {
	
	/* read this register: */
	value_packed = bt458->tme_bt458_regs[bt458_address - TME_BT458_REG_CONTROL_FIRST];
      }
    }

    /* this must be a read of the overlay map register: */
    else {
      assert (reg == TME_SUNFB_S4_REG_BT458_OMAP);

      /* read the overlay map: */
      byte_count = map_count;
      do {

	/* if the Bt458 address isn't in the overlay map: */
	if (bt458_address >= 4) {
	  abort();
	}

	/* advance the packed value: */
	value_packed <<= 8;

	/* read one overlay map primary: */
	value_packed |= bt458->tme_bt458_omap_primaries[bt458_rgb][bt458_address];

	/* advance the Bt458 address registers: */
	bt458_rgb++;
	if (bt458_rgb == TME_ARRAY_ELS(bt458->tme_bt458_omap_primaries)) {
	  bt458_address++;
	  bt458_rgb = 0;
	}
      } while (--map_count > 0);
    }

    /* run the bus cycle: */
    if (cycle_init->tme_bus_cycle_size == sizeof(tme_uint32_t)) {
      if (byte_count == sizeof(tme_uint8_t)) {
	value_packed |= (value_packed << 8);
	value_packed |= (value_packed << 16);
      }
      tme_bus_cycle_xfer_reg(cycle_init, 
			     &value_packed,
			     TME_BUS32_LOG2);
    }
    else {
      value = value_packed;
      tme_bus_cycle_xfer_reg(cycle_init, 
			     &value,
			     TME_BUS8_LOG2);
    }
  }
    
  /* update the Bt458 address registers: */
  bt458->tme_bt458_address = bt458_address;
  bt458->tme_bt458_rgb = ((reg == TME_SUNFB_S4_REG_BT458_CMAP
			   || reg == TME_SUNFB_S4_REG_BT458_OMAP)
			  ? bt458_rgb
			  : 0);

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the sunfb S4 bus cycle handler: */
int
tme_sunfb_bus_cycle_s4(void *_sunfb, struct tme_bus_cycle *cycle_init)
{
  struct tme_sunfb *sunfb;
  tme_bus_addr32_t undecoded;
  tme_uint8_t sunfb_s4_status;

  /* if this bus cycle happened in the Bt458 registers: */
  if ((cycle_init->tme_bus_cycle_address % TME_SUNFB_S4_SIZ_REGS)
      < TME_SUNFB_S4_SIZ_BT458) {

    /* call the Bt458 cycle handler: */
    return (tme_sunfb_bus_cycle_bt458(_sunfb, cycle_init));
  }

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* the entire register bus subregion is all decoded (or, rather, not
     decoded) as the S4 registers: */
  undecoded
    = (cycle_init->tme_bus_cycle_address
       & (((tme_bus_addr32_t) 0)
	  - TME_SUNFB_S4_SIZ_REGS));

  /* save the status register: */
  sunfb_s4_status = sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_status;

  /* if this is a write, and an interrupt is pending, and this write
     covers the status register: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
      && (sunfb_s4_status & TME_SUNFB_S4_STATUS_INT_PENDING)
      && (cycle_init->tme_bus_cycle_address
	  <= (undecoded
	      + TME_SUNFB_S4_REG(tme_sunfb_s4_regs_status)))
      && (cycle_init->tme_bus_cycle_size
	  > ((undecoded
	      + TME_SUNFB_S4_REG(tme_sunfb_s4_regs_status))
	     - cycle_init->tme_bus_cycle_address))) {
    
    /* clear the interrupt: */
    sunfb_s4_status &= ~TME_SUNFB_S4_STATUS_INT_PENDING;
  }

  /* run the cycle: */
  assert (cycle_init->tme_bus_cycle_address
	  >= sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first);
  tme_bus_cycle_xfer_memory(cycle_init, 
			    (((tme_uint8_t *) &sunfb->tme_sunfb_s4_regs)
			     - TME_SUNFB_S4_SIZ_BT458
			     - undecoded),
			    (undecoded
			     | (TME_SUNFB_S4_SIZ_REGS - 1)));

  /* restore the status register: */
  sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_status = sunfb_s4_status;

  /* make any callouts: */
  _tme_sunfb_callout(sunfb);

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* no faults: */
  return (TME_OK);  
}

/* the sunfb TLB filler: */
static int
_tme_sunfb_tlb_fill(void *_sunfb,
		    struct tme_bus_tlb *tlb, 
		    tme_bus_addr_t address_wider,
		    unsigned int cycles)
{
  struct tme_sunfb *sunfb;
  tme_bus_addr32_t address;
  struct tme_token *tlb_token;
  struct tme_token *tlb_token_other;
  tme_uint32_t offset_updated_first;
  tme_uint32_t offset_updated_last;
  unsigned int subregion_i;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* if this address falls in the bus subregion for memory: */
  if ((sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
       <= address)
      && (address
	  <= sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last)) {

    /* if this address falls in the memory pad: */
    if (address > sunfb->tme_sunfb_memory_address_last_displayed) {

      /* this TLB entry covers this range: */
      tlb->tme_bus_tlb_addr_first = (sunfb->tme_sunfb_memory_address_last_displayed + 1);
      tlb->tme_bus_tlb_addr_last = sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last;

      /* bus cycles to this range are to the memory pad and can be
         fast: */
      tlb->tme_bus_tlb_cycle = _tme_sunfb_bus_cycle_memory_pad;
      tlb->tme_bus_tlb_emulator_off_write
	= (sunfb->tme_sunfb_memory_pad
	   - tlb->tme_bus_tlb_addr_first);
    }

    /* otherwise, this address does not fall in the memory pad: */
    else {

      /* if this TLB entry is not for writing: */
      if ((cycles & TME_BUS_CYCLE_WRITE) == 0) {

	/* this TLB entry covers this range: */
	tlb->tme_bus_tlb_addr_first = sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first;
	tlb->tme_bus_tlb_addr_last = sunfb->tme_sunfb_memory_address_last_displayed;

	/* this TLB entry allows only (fast) reading to the memory: */
	tlb->tme_bus_tlb_emulator_off_read = sunfb->tme_sunfb_memory - tlb->tme_bus_tlb_addr_first;
	tlb->tme_bus_tlb_rwlock = &sunfb->tme_sunfb_rwlock;
	tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ;
	tlb->tme_bus_tlb_cycle = _tme_sunfb_bus_cycle_memory;
	tlb->tme_bus_tlb_cycle_private = _sunfb;
	return (TME_OK);
      }

      /* get the token for this writable TLB entry: */
      tlb_token = tlb->tme_bus_tlb_token;

      /* if a different writable TLB entry's token is outstanding: */
      tlb_token_other = sunfb->tme_sunfb_tlb_token;
      if (__tme_predict_true(tlb_token_other != NULL)) {
	if (tlb_token_other != tlb_token) {

	  /* invalidate this other TLB entry: */
	  tme_token_invalidate(tlb_token_other);
	}
      }

      /* save the token for this writable TLB entry: */
      sunfb->tme_sunfb_tlb_token = tlb_token;

      /* update the offsets of the first and last bytes updated in the
	 real framebuffer memory: */
      offset_updated_first = address;
      offset_updated_first -= (tme_uint32_t) sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first;
      offset_updated_last = offset_updated_first + TME_SUNFB_UPDATE_SIZE;
      offset_updated_first
	= TME_MIN(offset_updated_first,
		  sunfb->tme_sunfb_offset_updated_first);
      offset_updated_last
	= TME_MAX(offset_updated_last,
		  sunfb->tme_sunfb_offset_updated_last);
      offset_updated_last
	= TME_MIN(offset_updated_last,
		  (((tme_uint32_t)
		    sunfb->tme_sunfb_memory_address_last_displayed)
		   - ((tme_uint32_t)
		      sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first)));
      sunfb->tme_sunfb_offset_updated_first = offset_updated_first;
      sunfb->tme_sunfb_offset_updated_last = offset_updated_last;

      /* this TLB entry covers this range: */
      tlb->tme_bus_tlb_addr_first
	= (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
	   + offset_updated_first);
      tlb->tme_bus_tlb_addr_last
	= (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
	   + offset_updated_last);

      /* bus cycles to this range are to the memory and can be fast: */
      tlb->tme_bus_tlb_cycle = _tme_sunfb_bus_cycle_memory;
      tlb->tme_bus_tlb_emulator_off_write
	= (sunfb->tme_sunfb_memory
	   - sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first);
    }

    /* this TLB entry allows fast reading and writing: */
    tlb->tme_bus_tlb_emulator_off_read = tlb->tme_bus_tlb_emulator_off_write;
  }

  /* otherwise, this address doesn't fall in the bus subregion for memory: */
  else {

    /* search the other bus subregions: */
    for (subregion_i = 0;; subregion_i++) {

      /* this address must be in some subregion: */
      assert (subregion_i < TME_SUNFB_BUS_SUBREGIONS_MAX);

      /* if this subregion is defined, and this address falls in it: */
      if (sunfb->tme_sunfb_bus_handlers[subregion_i] != NULL
	  && (sunfb->tme_sunfb_bus_subregions[subregion_i].tme_bus_subregion_address_first
	      <= address)
	  && (address
	      <= sunfb->tme_sunfb_bus_subregions[subregion_i].tme_bus_subregion_address_last)) {

	/* this TLB entry covers this range: */
	tlb->tme_bus_tlb_addr_first = sunfb->tme_sunfb_bus_subregions[subregion_i].tme_bus_subregion_address_first;
	tlb->tme_bus_tlb_addr_last = sunfb->tme_sunfb_bus_subregions[subregion_i].tme_bus_subregion_address_last;

	/* bus cycles to this range are handled by this handler: */
	tlb->tme_bus_tlb_cycle = sunfb->tme_sunfb_bus_handlers[subregion_i];

	break;
      }
    }
  }

  /* the fast reading and writing rwlock: */
  tlb->tme_bus_tlb_rwlock = &sunfb->tme_sunfb_rwlock;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler private data: */
  tlb->tme_bus_tlb_cycle_private = _sunfb;

  return (TME_OK);
}

#if TME_SUNFB_BUS_TRANSITION

/* this is the bus cycle transition glue: */
int
tme_sunfb_bus_cycle_transition(void *_sunfb,
			       struct tme_bus_cycle *master_cycle,
			       void (*handler) _TME_P((struct tme_sunfb *,
						       struct tme_bus_cycle *,
						       tme_uint32_t *,
						       struct tme_completion *)))
{
  struct tme_completion completion_buffer;
  struct tme_sunfb *sunfb;
  tme_uint32_t master_fast_cycle_types;

  /* initialize the completion buffer: */
  tme_completion_init(&completion_buffer);

#ifndef NDEBUG

  /* initialize the completion: */
  completion_buffer.tme_completion_error = 0x71aa;

#endif /* NDEBUG */

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) _sunfb;

  /* lock the mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* run the cycle handler: */
  (*handler)
    (sunfb,
     master_cycle,
     &master_fast_cycle_types,
     &completion_buffer);

  /* unlock the mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  /* check the completion: */
  assert (completion_buffer.tme_completion_error != (int) 0x71aa);

  return (completion_buffer.tme_completion_error);
}

#endif /* TME_SUNFB_BUS_TRANSITION */

/* this makes a new framebuffer connection: */
static int
_tme_sunfb_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_sunfb *sunfb;
  struct tme_fb_connection *conn_fb;
  struct tme_fb_connection *conn_fb_other;
  int rc;

  /* recover our data structures: */
  sunfb = conn->tme_connection_element->tme_element_private;
  conn_fb = (struct tme_fb_connection *) conn;
  conn_fb_other = (struct tme_fb_connection *) conn->tme_connection_other;

  /* both sides must be framebuffer connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_FRAMEBUFFER);

  /* lock our mutex: */
  tme_mutex_lock(&sunfb->tme_sunfb_mutex);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* if the other side of the connection is not supplying specific
       displayed memory that it wants us to use: */
    if (conn_fb->tme_fb_connection_buffer == NULL) {

      /* allocate displayed memory: */
      rc = tme_fb_xlat_alloc_src(conn_fb);
      assert (rc == TME_OK);
    }

    /* if this framebuffer just maps the displayed memory on the bus,
       do that: */
    if (sunfb->tme_sunfb_memory == NULL) {
      sunfb->tme_sunfb_memory = conn_fb->tme_fb_connection_buffer;
    }

    /* save our connection: */
    sunfb->tme_sunfb_fb_connection = conn_fb_other;
  }

  /* unlock our mutex: */
  tme_mutex_unlock(&sunfb->tme_sunfb_mutex);

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_sunfb_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a sunfb: */
static int
_tme_sunfb_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_sunfb *sunfb;
  struct tme_fb_connection *conn_fb;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  sunfb = (struct tme_sunfb *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* if we don't have a framebuffer connection, make one: */
  if (sunfb->tme_sunfb_fb_connection == NULL) {

    /* allocate the new framebuffer connection: */
    conn_fb = tme_new0(struct tme_fb_connection, 1);
    conn = &conn_fb->tme_fb_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_FRAMEBUFFER;
    conn->tme_connection_score = tme_fb_connection_score;
    conn->tme_connection_make = _tme_sunfb_connection_make;
    conn->tme_connection_break = _tme_sunfb_connection_break;

    /* fill in the framebuffer connection: */
    conn_fb->tme_fb_connection_mode_change = NULL;
    conn_fb->tme_fb_connection_update = NULL;

    /* class: */
    conn_fb->tme_fb_connection_class = sunfb->tme_sunfb_class;

    /* depth: */
    conn_fb->tme_fb_connection_depth = sunfb->tme_sunfb_depth;

    /* width and height: */
    conn_fb->tme_fb_connection_width = tme_sunfb_size_width(sunfb->tme_sunfb_size);
    conn_fb->tme_fb_connection_height = tme_sunfb_size_height(sunfb->tme_sunfb_size);

    /* we skip no pixels at the start of the scanline: */
    conn_fb->tme_fb_connection_skipx = 0;

    /* we pad to 32-bit boundaries: */
    conn_fb->tme_fb_connection_scanline_pad = 32;

    /* we are big-endian: */
    conn_fb->tme_fb_connection_order = TME_ENDIAN_BIG;

    /* we don't allocate memory until the connection is made, in case
       the other side of the connection wants to provide us with a
       specific memory region to use (maybe we're on a system with 
       real matching hardware and we can write directly to its buffer): */
    conn_fb->tme_fb_connection_buffer = NULL;

    /* assume that bits per pixel is the same as depth: */
    conn_fb->tme_fb_connection_bits_per_pixel = sunfb->tme_sunfb_depth;

    /* assume that our pixels don't have subfields: */
    conn_fb->tme_fb_connection_mask_g = 0;
    conn_fb->tme_fb_connection_mask_r = 0;
    conn_fb->tme_fb_connection_mask_b = 0;

    /* set any update function: */
    conn_fb->tme_fb_connection_update = sunfb->tme_sunfb_memory_update;

    /* if this is a bwtwo: */
    if (_TME_SUNFB_IS_BWTWO(sunfb)) {

      /* intensities are a single bit, linearly mapped, but inverted: */
      conn_fb->tme_fb_connection_map_bits = 1;
      conn_fb->tme_fb_connection_map_g = NULL;
      conn_fb->tme_fb_connection_map_r = NULL;
      conn_fb->tme_fb_connection_map_b = NULL;
      conn_fb->tme_fb_connection_inverted = TRUE;
    }

    /* otherwise, this is one of the eight-bit color framebuffers: */
    else {

      /* intensities are eight bits and index mapped: */
      conn_fb->tme_fb_connection_map_bits = 8;
      conn_fb->tme_fb_connection_map_g = sunfb->tme_sunfb_cmap_g;
      conn_fb->tme_fb_connection_map_r = sunfb->tme_sunfb_cmap_r;
      conn_fb->tme_fb_connection_map_b = sunfb->tme_sunfb_cmap_b;
    }

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new Sun framebuffer function: */
int
tme_sunfb_new(struct tme_sunfb *sunfb, 
	      const char * const *args,
	      char **_output)
{
  struct tme_bus_subregion *subregion;
  const struct tme_bus_subregion **_subregion_prev;
  unsigned int subregion_i;
  const char *sunfb_type_string;
  const char *sunfb_size_string;
  tme_uint32_t sunfb_size;
  tme_bus_addr_t fb_size;
  tme_uint32_t sunfb_p4;
  tme_uint8_t sunfb_s4_status;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  sunfb_type_string = NULL;
  sunfb_size_string = NULL;
  arg_i = 1;
  for (;;) {

    /* the framebuffer type: */
    if (TME_ARG_IS(args[arg_i + 0], "type")
	&& sunfb_type_string == NULL
	&& sunfb->tme_sunfb_type_set != NULL) {
      sunfb_type_string = args[arg_i + 1];
      if (sunfb_type_string == NULL
	  || (*sunfb->tme_sunfb_type_set)(sunfb, sunfb_type_string) != NULL) {
	usage = TRUE;
	break;
      }
      arg_i += 2;
    }

    /* the framebuffer size: */
    else if (TME_ARG_IS(args[arg_i + 0], "size")
	     && sunfb_size_string == NULL) {
      sunfb_size_string = args[arg_i + 1];
      if (sunfb_size_string == NULL) {
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

  /* set some common defaults based on framebuffer type: */

  /* if this is a P4 framebuffer: */
  if (_TME_SUNFB_IS_P4(sunfb)) {

    /* default possible P4 sizes: */
    if (sunfb->tme_sunfb_size == 0) {
      sunfb->tme_sunfb_size
	= (TME_SUNFB_SIZE_1600_1280
	   | TME_SUNFB_SIZE_1152_900
	   | TME_SUNFB_SIZE_1024_1024
	   | TME_SUNFB_SIZE_1280_1024
	   | TME_SUNFB_SIZE_1440_1440);
    }

    /* the memory address can't be zero: */
    assert (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first != 0);

    /* the P4 register is always at the same offset: */
    sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first = TME_SUNFB_P4_OFFSET_P4;

    /* the default size of the P4 register bus subregion: */
    if (sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_last == 0) {
      sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_last
	= (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
	   - 1);
    }
  }
  

  /* if this is an S4 framebuffer: */
  if (_TME_SUNFB_IS_S4(sunfb)) {

    /* default possible S4 framebuffer sizes: */
    if (sunfb->tme_sunfb_size == 0) {
      sunfb->tme_sunfb_size
	= (TME_SUNFB_SIZE_1024_768
	   | TME_SUNFB_SIZE_1152_900
	   | TME_SUNFB_SIZE_1280_1024
	   | TME_SUNFB_SIZE_1600_1280);
    }

    /* the default memory address: */
    if (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first == 0) {
      sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first = TME_SUNFB_S4_OFFSET_MEMORY;
    }

    /* the S4 registers are always at the same offset: */
    sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_first = TME_SUNFB_S4_OFFSET_REGS;

    /* the default size of the S4 register bus subregion: */
    if (sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_last == 0) {
      sunfb->tme_sunfb_bus_subregion_regs.tme_bus_subregion_address_last
	= (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
	   - 1);
    }
  }

  /* we must have some possible sizes: */
  assert (sunfb->tme_sunfb_size != 0 || usage);

  /* if no specific size is given: */
  if (sunfb_size_string == NULL) {

    /* use the first size supported by this framebuffer, which will
       usually be 1152x900: */
    sunfb_size = sunfb->tme_sunfb_size;
    sunfb_size = (sunfb_size & ~(sunfb_size - 1));
  }

  /* otherwise, convert the specific size: */
  else {
    sunfb_size = tme_sunfb_size(sunfb_size_string);
  }

  /* if the size is not supported by this framebuffer: */
  if ((sunfb->tme_sunfb_size & sunfb_size) == 0) {
    usage = TRUE;
  }

  if (usage) {

    /* start the usage message: */
    tme_output_append_error(_output,
			    "%s %s",
			    _("usage"),
			    args[0]);

    /* if this framebuffer has types, append a type argument to the
       usage message: */
    if (sunfb->tme_sunfb_type_set != NULL) {
      sunfb_type_string = (*sunfb->tme_sunfb_type_set)(sunfb, NULL);
      tme_output_append_error(_output, " type { %s }", sunfb_type_string);
    }

    /* append the supported types to the usage message: */
    tme_output_append_error(_output,
			    " [ size {");
    for (sunfb_size = sunfb->tme_sunfb_size;
	 sunfb_size != 0;
	 sunfb_size &= (sunfb_size - 1)) {
      switch (sunfb_size & ~(sunfb_size - 1)) {
      default: assert(FALSE);
      case TME_SUNFB_SIZE_1152_900: sunfb_size_string = "1152x900"; break;
      case TME_SUNFB_SIZE_1024_1024: sunfb_size_string = "1024x1024"; break;
      case TME_SUNFB_SIZE_1280_1024: sunfb_size_string = "1280x1024"; break;
      case TME_SUNFB_SIZE_1600_1280: sunfb_size_string = "1600x1280"; break;
      case TME_SUNFB_SIZE_1440_1440: sunfb_size_string = "1440x1440"; break;
      case TME_SUNFB_SIZE_1024_768: sunfb_size_string = "1024x768"; break;
      case TME_SUNFB_SIZE_640_480: sunfb_size_string = "640x480"; break;
      }
      tme_output_append_error(_output, " %s", sunfb_size_string);
    }
    tme_output_append_error(_output, " } ]");

    /* return the error: */
    return (EINVAL);
  }

  /* finish initializing the sunfb structure: */
  tme_mutex_init(&sunfb->tme_sunfb_mutex);
  tme_rwlock_init(&sunfb->tme_sunfb_rwlock);

  /* set the size: */
  sunfb->tme_sunfb_size = sunfb_size;

  /* calculate the number of bytes of displayed framebuffer memory: */
  fb_size = tme_sunfb_size_width(sunfb->tme_sunfb_size);
  fb_size *= tme_sunfb_size_height(sunfb->tme_sunfb_size);
  if (_TME_SUNFB_IS_BWTWO(sunfb)) {
    fb_size /= 8;
  }

  /* set the (relative) bus address of the last byte of displayed
     memory: */
  sunfb->tme_sunfb_memory_address_last_displayed
    = (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
       + fb_size
       - 1);

  /* assume that the real number of bytes of memory is the number of
     bytes displayed, rounded up to the nearest power of two: */
  if ((fb_size & (fb_size - 1)) != 0) {
    for (; (fb_size & (fb_size - 1)) != 0; fb_size &= (fb_size - 1));
    fb_size <<= 1;
  }

  /* set the (relative) bus address of the last byte of memory: */
  sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last
    = (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_first
       + fb_size
       - 1);

  /* if we need to, allocate pad (undisplayed) framebuffer memory: */
  if (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last
      > sunfb->tme_sunfb_memory_address_last_displayed) {

    /* allocate the pad memory: */
    sunfb->tme_sunfb_memory_pad
      = tme_new0(tme_uint8_t,
		 (sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_address_last
		  - sunfb->tme_sunfb_memory_address_last_displayed));
  }

  /* if this is a P4 framebuffer: */
  if (_TME_SUNFB_IS_P4(sunfb)) {

    /* get our initial P4 register: */
    sunfb_p4 = tme_betoh_u32(sunfb->tme_sunfb_p4);

    /* if the P4 register doesn't already have something in the size
       field, add it: */
    if ((sunfb_p4 & TME_SUNFB_P4_SIZE_MASK) == 0) {
      switch (sunfb_size) {
      default: assert(FALSE);
      case TME_SUNFB_SIZE_1152_900: sunfb_p4 |= TME_SUNFB_P4_SIZE_1152_900; break;
      case TME_SUNFB_SIZE_1024_1024: sunfb_p4 |= TME_SUNFB_P4_SIZE_1024_1024; break;
      case TME_SUNFB_SIZE_1280_1024: sunfb_p4 |= TME_SUNFB_P4_SIZE_1280_1024; break;
      case TME_SUNFB_SIZE_1600_1280: sunfb_p4 |= TME_SUNFB_P4_SIZE_1600_1280; break;
      case TME_SUNFB_SIZE_1440_1440: sunfb_p4 |= TME_SUNFB_P4_SIZE_1440_1440; break;
      case TME_SUNFB_SIZE_640_480: sunfb_p4 |= TME_SUNFB_P4_SIZE_640_480; break;
      }
    }

    /* set video as enabled: */
    sunfb_p4 |= TME_SUNFB_P4_REG_VIDEO_ENABLE;

    /* set the initial P4 register: */
    sunfb->tme_sunfb_p4 = tme_htobe_u32(sunfb_p4);
  }

  /* if this is an S4 framebuffer: */
  if (_TME_SUNFB_IS_S4(sunfb)) {

    /* set the initial S4 control register: */
    sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_control
      = (TME_SUNFB_S4_CONTROL_VIDEO_ENABLE);

    /* set the initial S4 status register: */
    switch (sunfb_size) {
    default: assert(FALSE);
    case TME_SUNFB_SIZE_1152_900: sunfb_s4_status = TME_SUNFB_S4_STATUS_SIZE_1152_900; break;
    case TME_SUNFB_SIZE_1024_768: sunfb_s4_status = TME_SUNFB_S4_STATUS_SIZE_1024_768; break;
    case TME_SUNFB_SIZE_1280_1024: sunfb_s4_status = TME_SUNFB_S4_STATUS_SIZE_1280_1024; break;
    case TME_SUNFB_SIZE_1600_1280: sunfb_s4_status = TME_SUNFB_S4_STATUS_SIZE_1600_1280; break;
    }
    sunfb->tme_sunfb_s4_regs.tme_sunfb_s4_regs_status
      = (sunfb_s4_status
	 | (_TME_SUNFB_IS_BWTWO(sunfb)
	    ? TME_SUNFB_S4_STATUS_ID_MONO
	    : TME_SUNFB_S4_STATUS_ID_COLOR));
  }

  /* make sure that the interrupt signal has been defined.  no known
     framebuffer uses a priority zero interrupt: */
#if TME_BUS_SIGNAL_INT(0) != 0
#error "TME_BUS_SIGNAL_INT() changed"
#endif
  assert (sunfb->tme_sunfb_bus_signal_int != TME_BUS_SIGNAL_INT(0));

  /* if this is a color framebuffer: */
  if (!_TME_SUNFB_IS_BWTWO(sunfb)) {

    /* if we don't have a specific memory update function, use the
       default: */
    if (sunfb->tme_sunfb_memory_update == NULL) {
      sunfb->tme_sunfb_memory_update = tme_sunfb_memory_update;
    }
  }

  /* make all of the defined subregions into a list.  the memory
     subregion is always in the list, and it is first: */
  _subregion_prev = &sunfb->tme_sunfb_bus_subregion_memory.tme_bus_subregion_next;
  for (subregion_i = 0;
       subregion_i < TME_SUNFB_BUS_SUBREGIONS_MAX;
       subregion_i++) {
    if (sunfb->tme_sunfb_bus_handlers[subregion_i] != NULL) {
      subregion = &sunfb->tme_sunfb_bus_subregions[subregion_i];
      *_subregion_prev = subregion;
      _subregion_prev = &subregion->tme_bus_subregion_next;
    }
  }
  *_subregion_prev = NULL;

  /* initialize our simple bus device descriptor: */
  sunfb->tme_sunfb_device.tme_bus_device_tlb_fill = _tme_sunfb_tlb_fill;

  /* fill the element: */
  sunfb->tme_sunfb_element->tme_element_private = sunfb;
  sunfb->tme_sunfb_element->tme_element_connections_new = _tme_sunfb_connections_new;

  /* initialize the timeout thread condition: */
  tme_cond_init(&sunfb->tme_sunfb_callout_cond);

  /* start the callout thread: */
  tme_thread_create((tme_thread_t) _tme_sunfb_callout_thread, sunfb);

  return (TME_OK);
}

/* the new sun cgthree function: */
int
tme_sun_cgthree(struct tme_element *element, const char * const *args, char **_output)
{
  struct tme_sunfb *sunfb;
  tme_uint8_t *cmap;
  int rc;

  /* start the sunfb structure: */
  sunfb = tme_new0(struct tme_sunfb, 1);
  sunfb->tme_sunfb_element = element;

  /* initialize the sunfb structure: */
  sunfb->tme_sunfb_class = TME_FB_XLAT_CLASS_COLOR;
  sunfb->tme_sunfb_depth = 8;
  sunfb->tme_sunfb_bus_handler_regs = tme_sunfb_bus_cycle_s4;
  sunfb->tme_sunfb_flags
    |= (TME_SUNFB_FLAG_BT458_CMAP_PACKED
	| TME_SUNFB_FLAG_BT458_BYTE_D0_D7);
  sunfb->tme_sunfb_bus_signal_int = TME_BUS_SIGNAL_INT(5);

  /* if the generic initialization fails: */
  rc = tme_sunfb_new(sunfb, args, _output);
  if (rc) {

    /* free the sunfb structure and return the error: */
    tme_free(sunfb);
    return (rc);
  }

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
