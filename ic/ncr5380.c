/* $Id: ncr5380.c,v 1.6 2010/06/05 15:18:59 fredette Exp $ */

/* ic/ncr5380.c - implementation of NCR 5380 emulation: */

/*
 * Copyright (c) 2004 Matt Fredette
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
_TME_RCSID("$Id: ncr5380.c,v 1.6 2010/06/05 15:18:59 fredette Exp $");

/* XXX FIXME - TLB usage here is not thread-safe: */

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/scsi.h>

/* macros: */

/* some registers are read-only: */
#define TME_NCR5380_REG_WR(x)		(x)
#define TME_NCR5380_REG_RO(x)		((x) | TME_NCR5380_SIZ_REGS)
#define TME_NCR5380_REG_INDEX(x)	((x) % TME_NCR5380_SIZ_REGS)

/* register offsets: */
#define TME_NCR5380_REG_ODR	TME_NCR5380_REG_WR(0)
#define TME_NCR5380_REG_CSD	TME_NCR5380_REG_RO(0)
#define TME_NCR5380_REG_ICR	TME_NCR5380_REG_WR(1)
#define TME_NCR5380_REG_MR2	TME_NCR5380_REG_WR(2)
#define TME_NCR5380_REG_TCR	TME_NCR5380_REG_WR(3)
#define TME_NCR5380_REG_SER	TME_NCR5380_REG_WR(4)
#define TME_NCR5380_REG_CSB	TME_NCR5380_REG_RO(4)
#define TME_NCR5380_REG_BSR	TME_NCR5380_REG_RO(5)
#define TME_NCR5380_REG_SDS	TME_NCR5380_REG_WR(5)
#define TME_NCR5380_REG_SDT	TME_NCR5380_REG_WR(6)
#define TME_NCR5380_REG_IDR	TME_NCR5380_REG_RO(6)
#define TME_NCR5380_REG_SDI	TME_NCR5380_REG_WR(7)
#define TME_NCR5380_REG_RPI	TME_NCR5380_REG_RO(7)
#define TME_NCR5380_SIZ_REGS	(8)

/* a mask of read-only registers: */
#define TME_NCR5380_REGS_RO					\
  (TME_BIT(TME_NCR5380_REG_INDEX(TME_NCR5380_REG_CSD))		\
   | TME_BIT(TME_NCR5380_REG_INDEX(TME_NCR5380_REG_CSB))	\
   | TME_BIT(TME_NCR5380_REG_INDEX(TME_NCR5380_REG_BSR))	\
   | TME_BIT(TME_NCR5380_REG_INDEX(TME_NCR5380_REG_IDR))	\
   | TME_BIT(TME_NCR5380_REG_INDEX(TME_NCR5380_REG_RPI)))

/* bits in the Initiator Command Register: */
#define TME_NCR5380_ICR_DBUS	TME_BIT(0)
#define TME_NCR5380_ICR_ATN	TME_BIT(1)
#define TME_NCR5380_ICR_SEL	TME_BIT(2)
#define TME_NCR5380_ICR_BSY	TME_BIT(3)
#define TME_NCR5380_ICR_ACK	TME_BIT(4)
#define TME_NCR5380_ICR_LA	TME_BIT(5)
#define TME_NCR5380_ICR_DIFF	TME_BIT(5)
#define TME_NCR5380_ICR_TEST	TME_BIT(6)
#define TME_NCR5380_ICR_AIP	TME_BIT(6)
#define TME_NCR5380_ICR_RST	TME_BIT(7)

/* bits in Mode Register 2: */
#define TME_NCR5380_MR2_ARB	TME_BIT(0)
#define TME_NCR5380_MR2_DMA	TME_BIT(1)
#define TME_NCR5380_MR2_BSY	TME_BIT(2)
#define TME_NCR5380_MR2_EOP	TME_BIT(3)
#define TME_NCR5380_MR2_PINT	TME_BIT(4)
#define TME_NCR5380_MR2_PCHK	TME_BIT(5)
#define TME_NCR5380_MR2_TARG	TME_BIT(6)
#define TME_NCR5380_MR2_BLK	TME_BIT(7)

/* bits in the Target Command Register: */
#define TME_NCR5380_TCR_I_O	TME_BIT(0)
#define TME_NCR5380_TCR_C_D	TME_BIT(1)
#define TME_NCR5380_TCR_MSG	TME_BIT(2)
#define TME_NCR5380_TCR_REQ	TME_BIT(3)

/* bits in the Current SCSI Bus Status Register: */
#define TME_NCR5380_CSB_DBP	TME_BIT(0)
#define TME_NCR5380_CSB_SEL	TME_BIT(1)
#define TME_NCR5380_CSB_I_O	TME_BIT(2)
#define TME_NCR5380_CSB_C_D	TME_BIT(3)
#define TME_NCR5380_CSB_MSG	TME_BIT(4)
#define TME_NCR5380_CSB_REQ	TME_BIT(5)
#define TME_NCR5380_CSB_BSY	TME_BIT(6)
#define TME_NCR5380_CSB_RST	TME_BIT(7)

/* bits in the Bus and Status Register: */
#define TME_NCR5380_BSR_ACK	TME_BIT(0)
#define TME_NCR5380_BSR_ATN	TME_BIT(1)
#define TME_NCR5380_BSR_BSY	TME_BIT(2)
#define TME_NCR5380_BSR_PHSM	TME_BIT(3)
#define TME_NCR5380_BSR_INT	TME_BIT(4)
#define TME_NCR5380_BSR_SPER	TME_BIT(5)
#define TME_NCR5380_BSR_DRQ	TME_BIT(6)
#define TME_NCR5380_BSR_EDMA	TME_BIT(7)

/* this evaluates to true is the bus phase in the CSB matches the TCR.
   NB that the bus phase bits don't line up perfectly between the two
   registers: */
#define TME_NCR5380_BUS_PHASE_MATCH(ncr5380)					\
((((ncr5380)->tme_ncr5380_regs[TME_NCR5380_REG_CSB]				\
   ^ ((ncr5380)->tme_ncr5380_regs[TME_NCR5380_REG_TCR] * TME_NCR5380_CSB_I_O))	\
  & (TME_NCR5380_CSB_MSG							\
     | TME_NCR5380_CSB_C_D							\
     | TME_NCR5380_CSB_I_O)) == 0)

/* the callout flags: */
#define TME_NCR5380_CALLOUT_CHECK		(0)
#define TME_NCR5380_CALLOUT_RUNNING		TME_BIT(0)
#define TME_NCR5380_CALLOUTS_MASK		(-2)
#define  TME_NCR5380_CALLOUT_FLUSH_INT_DMA	TME_BIT(1)
#define  TME_NCR5380_CALLOUT_TERMINAL_DMA	TME_BIT(2)
#define	 TME_NCR5380_CALLOUT_INT		TME_BIT(3)
#define  TME_NCR5380_CALLOUT_SCSI_CYCLE		TME_BIT(4)

#if 1
#define TME_NCR5380_DEBUG
#endif

/* structures: */

/* the IC: */
struct tme_ncr5380 {

  /* our simple bus device header: */
  struct tme_bus_device tme_ncr5380_device;
#define tme_ncr5380_element tme_ncr5380_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_ncr5380_mutex;

  /* the SCSI bus connection: */
  struct tme_scsi_connection *tme_ncr5380_scsi_connection;

  /* the callout flags: */
  int tme_ncr5380_callout_flags;

  /* the current SCSI cycle we want called out: */
  tme_scsi_control_t tme_ncr5380_scsi_control;
  tme_scsi_data_t tme_ncr5380_scsi_data;
  tme_uint32_t tme_ncr5380_scsi_events;
  tme_uint32_t tme_ncr5380_scsi_actions;

  /* if our interrupt line is currently asserted: */
  int tme_ncr5380_last_int_asserted;

  /* the last SCSI cycle we called out: */
  tme_scsi_control_t tme_ncr5380_last_scsi_control;
  tme_scsi_data_t tme_ncr5380_last_scsi_data;
  tme_uint32_t tme_ncr5380_last_scsi_events;
  tme_uint32_t tme_ncr5380_last_scsi_actions;

  /* it's easiest to just model the registers as a chunk of memory.
     we have twice as much memory as address space, because some of
     the addresses refer to two different registers: */
  tme_uint8_t tme_ncr5380_regs[TME_NCR5380_SIZ_REGS * 2];

  /* this is nonzero if DMA is running: */
  int tme_ncr5380_dma_running;

  /* our DMA TLB set: */
  struct tme_bus_tlb tme_ncr5380_dma_tlb;
  int tme_ncr5380_dma_tlb_added;

  /* our DMA pseudoaddress, prefetch, and residual: */
  tme_bus_addr32_t tme_ncr5380_dma_address;
  unsigned int tme_ncr5380_dma_prefetch;
  unsigned long tme_ncr5380_dma_resid;

  /* the internal DMA buffer: */
  tme_uint8_t tme_ncr5380_int_dma;
};

/* globals: */

/* the NCR5380 bus router: */
static const tme_bus_lane_t tme_ncr5380_router[TME_BUS_ROUTER_SIZE(TME_BUS8_LOG2)] = {
  
  /* [ncr]  initiator cycle size: 8 bits
     [gen]  responding port size: 8 bits
     [gen]  responding port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
};

#ifdef TME_NCR5380_DEBUG
static void
_tme_ncr5380_reg_put(struct tme_ncr5380 *ncr5380,
		     int reg,
		     tme_uint8_t val_new)
{
  const char *reg_name;
  tme_uint8_t val_old;

  val_old = ncr5380->tme_ncr5380_regs[reg];
  if (val_old == val_new) {
    return;
  }
  ncr5380->tme_ncr5380_regs[reg] = val_new;

  switch (reg) {
  case TME_NCR5380_REG_ODR: reg_name = "ODR"; break;
  case TME_NCR5380_REG_CSD: reg_name = "CSD"; break;
  case TME_NCR5380_REG_ICR: reg_name = "ICR"; break;
  case TME_NCR5380_REG_MR2: reg_name = "MR2"; break;
  case TME_NCR5380_REG_TCR: reg_name = "TCR"; break;
  case TME_NCR5380_REG_SER: reg_name = "SER"; break;
  case TME_NCR5380_REG_CSB: reg_name = "CSB"; break;
  case TME_NCR5380_REG_BSR: reg_name = "BSR"; break;
  case TME_NCR5380_REG_SDS: reg_name = "SDS"; break;
  case TME_NCR5380_REG_SDT: reg_name = "SDT"; break;
  case TME_NCR5380_REG_IDR: reg_name = "IDR"; break;
  case TME_NCR5380_REG_SDI: reg_name = "SDI"; break;
  case TME_NCR5380_REG_RPI: reg_name = "RPI"; break;
  default: reg_name = "???"; break;
  }
  tme_log(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
	  100, TME_OK,
	  (&ncr5380->tme_ncr5380_element->tme_element_log_handle,
	   "%s now 0x%02x",
	   reg_name,
	   val_new));
}
#define TME_NCR5380_REG_PUT _tme_ncr5380_reg_put
#else  /* !TME_NCR5380_DEBUG */
#define TME_NCR5380_REG_PUT(n, r, v) ((n)->tme_ncr5380_regs[(r)] = (v))
#endif /* !TME_NCR5380_DEBUG */

/* this resets the NCR 5380: */
static int
_tme_ncr5380_reset(struct tme_ncr5380 *ncr5380,
		   int external)
{

  /* "When a SCSI RST is applied externally the ASI resets all
     registers and logic and issues an interrupt. The only register
     bits not affected are the Assert RST bit (bit 7) in the ICR and
     the TARGET Mode bit (bit 6) in MR2." */
  if (external) {
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR] &= TME_NCR5380_ICR_RST;
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2] &= TME_NCR5380_MR2_TARG;
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR] = TME_NCR5380_BSR_INT;
  }
  else {
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR] = 0;
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2] = 0;
    ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR] = 0;
  }

  /* reset the other registers: */
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ODR] = 0;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_TCR] = 0;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_SER] = 0;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_IDR] = 0;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_SDI] = 0;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_RPI] = 0;

  /* assume that the interrupt signal has changed: */
  return (TME_NCR5380_CALLOUT_INT);
}

/* this is the NCR 5380 update function: */
static int
_tme_ncr5380_update(struct tme_ncr5380 *ncr5380)
{
  tme_scsi_control_t scsi_control;
  tme_scsi_data_t scsi_data;
  tme_uint32_t scsi_events;
  tme_uint32_t scsi_actions;
  tme_uint8_t icr_new;
  tme_uint8_t mr2_new;
  tme_uint8_t tcr_new;
  tme_uint8_t csb_new;
  tme_uint8_t bsr_new;
  int dma_running_old;
  int id;
  int new_callouts;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* if RST is being asserted: */
  if ((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR] & TME_NCR5380_ICR_RST)
      || (ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB] & TME_NCR5380_CSB_RST)) {

    /* do an external reset: */
    new_callouts |= _tme_ncr5380_reset(ncr5380, TRUE);
  }

  /* get the ICR, MR2, TCR, CSB, and BSR: */
  icr_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR];
  mr2_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2];
  tcr_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_TCR];
  csb_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB];
  bsr_new
    = (ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR]
       & ~(TME_NCR5380_BSR_PHSM
	   | TME_NCR5380_BSR_DRQ));

  /* get if DMA is running: */
  dma_running_old = ncr5380->tme_ncr5380_dma_running;

  /* if ARB is not set in MR2, clear AIP and LA: */
  if (!(mr2_new & TME_NCR5380_MR2_ARB)) {
    icr_new &= ~(TME_NCR5380_ICR_AIP | TME_NCR5380_ICR_LA);
  }

  /* "The SCSI BSY signal has become inactive while the MR2 BSY
     (Monitor BSY) bit is set. This will cause an interrupt, remove
     all ASI signals from the SCSI bus and reset the DMA MODE bit in
     MR2." */
  if ((mr2_new & TME_NCR5380_MR2_BSY)
      && !(csb_new & TME_NCR5380_CSB_BSY)) {

    /* set BSY and INT in the BSR and call out an interrupt: */
    bsr_new |= (TME_NCR5380_BSR_BSY
		| TME_NCR5380_BSR_INT);
    new_callouts |= TME_NCR5380_CALLOUT_INT;
    
    /* make sure we're not asserting anything on the bus: */
    assert (!(mr2_new & TME_NCR5380_MR2_TARG));
    icr_new &= ~(TME_NCR5380_ICR_ATN
		 | TME_NCR5380_ICR_ACK);

    /* reset the DMA bit and stop DMA: */
    mr2_new &= ~TME_NCR5380_MR2_DMA;
  }

  /* if the DMA bit is clear, stop DMA: */
  if (!(mr2_new & TME_NCR5380_MR2_DMA)) {
    ncr5380->tme_ncr5380_dma_running = FALSE;
  }

  /* if we have a phase match: */
  if (TME_NCR5380_BUS_PHASE_MATCH(ncr5380)) {
    bsr_new |= TME_NCR5380_BSR_PHSM;
  }

  /* otherwise, we don't have a phase match.  a mismatch while REQ is
     asserted and DMA is set in MR2 stops DMA if it's running, and
     always causes an interrupt (even if DMA wasn't running): */
  else if ((csb_new & TME_NCR5380_CSB_REQ)
	   && (mr2_new & TME_NCR5380_MR2_DMA)) {

    /* stop DMA: */
    ncr5380->tme_ncr5380_dma_running = FALSE;

    /* call out an interrupt: */
    bsr_new |= TME_NCR5380_BSR_INT;
    new_callouts |= TME_NCR5380_CALLOUT_INT;
  }

  /* if DMA is running and EDMA is set: */
  if (ncr5380->tme_ncr5380_dma_running
      && (bsr_new & TME_NCR5380_BSR_EDMA)) {

    /* stop DMA: */
    ncr5380->tme_ncr5380_dma_running = FALSE;

    /* if EOP is set, call out an interrupt: */
    if (mr2_new & TME_NCR5380_MR2_EOP) {
      bsr_new |= TME_NCR5380_BSR_INT;
      new_callouts |= TME_NCR5380_CALLOUT_INT;
    }
  }
    
  /* if DMA is running, set DRQ: */
  if (ncr5380->tme_ncr5380_dma_running) {
    bsr_new |= TME_NCR5380_BSR_DRQ;
  }

  /* otherwise, DMA is not running.  if it previously was: */
  else if (dma_running_old) {

    /* NB that when sending on the SCSI bus as an initiator, the NCR
       5380 does a DMA read and starts driving the SCSI data bus with
       the read byte, *before* it sees REQ from the target.  while
       this speeds up transfers (the data has likely already settled
       on the SCSI bus when the NCR 5380 sees REQ, so the NCR 5380
       simply needs to assert ACK and start the DMA read of the next
       byte), when something unusual happens that stops DMA (the
       target disconnects, unexpectedly changes phase, etc., but *not*
       EDMA) the 5380 has already done a DMA read of a byte that it
       can't send.

       this is the "NCR 5380 prefetch" problem.  we have to emulate it: */
    if (!(bsr_new & TME_NCR5380_BSR_EDMA)) {
      ncr5380->tme_ncr5380_dma_address += ncr5380->tme_ncr5380_dma_prefetch;
    }

    /* call out the terminal DMA address: */
    new_callouts |= TME_NCR5380_CALLOUT_TERMINAL_DMA;
  }
      
  /* set new register values: */
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_ICR, icr_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_MR2, mr2_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_TCR, tcr_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_CSB, csb_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_BSR, bsr_new);

  /* assume that we aren't driving the SCSI bus at all: */
  scsi_control = 0;
  scsi_data = 0;

  /* form the new SCSI control lines: */  
#define _TME_NCR5380_CONTROL(reg, _reg, _control) \
  do { if ((reg) & (_reg)) { scsi_control |=  (_control); } } while (/* CONSTCOND */ 0)

  /* these control lines can only be asserted when the NCR 5380 is in
     target mode: */
  if (ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2] & TME_NCR5380_MR2_TARG) {
    _TME_NCR5380_CONTROL(tcr_new, TME_NCR5380_TCR_I_O, TME_SCSI_SIGNAL_I_O);
    _TME_NCR5380_CONTROL(tcr_new, TME_NCR5380_TCR_C_D, TME_SCSI_SIGNAL_C_D);
    _TME_NCR5380_CONTROL(tcr_new, TME_NCR5380_TCR_MSG, TME_SCSI_SIGNAL_MSG);
    _TME_NCR5380_CONTROL(tcr_new, TME_NCR5380_TCR_REQ, TME_SCSI_SIGNAL_REQ);
  }

  /* these control lines can only be asserted when the NCR 5380 is in
     initiator mode: */
  else {
    _TME_NCR5380_CONTROL(icr_new, TME_NCR5380_ICR_ATN, TME_SCSI_SIGNAL_ATN);
    _TME_NCR5380_CONTROL(icr_new, TME_NCR5380_ICR_ACK, TME_SCSI_SIGNAL_ACK);
  }

  /* these control lines can be asserted in both target and initiator
     modes: */
  _TME_NCR5380_CONTROL(icr_new, TME_NCR5380_ICR_SEL, TME_SCSI_SIGNAL_SEL);
  _TME_NCR5380_CONTROL(icr_new, TME_NCR5380_ICR_BSY, TME_SCSI_SIGNAL_BSY);
  _TME_NCR5380_CONTROL(icr_new, TME_NCR5380_ICR_RST, TME_SCSI_SIGNAL_RST);

#undef _TME_NCR5380_CONTROL

  /* "[DBUS] should be set when transferring data out of the ASI
     in either TARGET or INITIATOR mode, for both DMA or
     programmed-I/O.  In INITIATOR mode the drivers are only
     enabled if: Mode Register 2 TARGET MODE bit is 0, and I/O is
     false, and C/D, I/O, MSG match the contents of the Target
     Command Register (phasematch is true). In TARGET mode only
     the MR2 bit needs to be set with this bit." */
  if ((icr_new & TME_NCR5380_ICR_DBUS)
      && ((mr2_new & TME_NCR5380_MR2_TARG)
	  || ((csb_new & TME_NCR5380_CSB_I_O) == 0
	      && (bsr_new & TME_NCR5380_BSR_PHSM)))) {
	  
    /* we are driving the data bus: */
    /* XXX we should do this for arbitration too, even if DBUS isn't set: */
    scsi_data = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ODR];
  }

  /* if we're supposed to be arbitrating for the bus, but the
     arbitration isn't in progress yet: */
  if ((mr2_new & TME_NCR5380_MR2_ARB)
      && !(icr_new & TME_NCR5380_ICR_AIP)
      && (id = ffs(ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ODR])) > 0) {

    /* wait for the bus to be free, and if SER is nonzero, wait for a
       selection or reselection, too: */
    scsi_events = TME_SCSI_EVENT_BUS_FREE;
    if (ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_SER] != 0) {
      scsi_events
	|= (TME_SCSI_EVENT_IDS_SELF(ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_SER])
	    | TME_SCSI_EVENT_SELECTED
	    | TME_SCSI_EVENT_RESELECTED);
    }

    /* if the bus becomes free, arbitrate for it: */
    scsi_actions = TME_SCSI_ACTION_ARBITRATE_HALF | TME_SCSI_ACTION_ID_SELF(id - 1);
  }

  /* otherwise, if DMA is running: */
  else if (ncr5380->tme_ncr5380_dma_running) {

    /* don't wait for any event, and do the DMA: */
    scsi_events = TME_SCSI_EVENT_NONE;
    scsi_actions
      = ((mr2_new & TME_NCR5380_MR2_TARG)
	 ? TME_SCSI_ACTION_DMA_TARGET
	 : TME_SCSI_ACTION_DMA_INITIATOR);
  }

  /* otherwise, just wait for a bus change: */
  else {
    scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
    scsi_actions = TME_SCSI_ACTION_NONE;
  }

  /* set the new SCSI state: */
  ncr5380->tme_ncr5380_scsi_control = scsi_control;
  ncr5380->tme_ncr5380_scsi_data = scsi_data;
  ncr5380->tme_ncr5380_scsi_events = scsi_events;
  ncr5380->tme_ncr5380_scsi_actions = scsi_actions;
  new_callouts |= TME_NCR5380_CALLOUT_SCSI_CYCLE;

  /* return the new callouts: */
  return (new_callouts);
}

/* this calls out to fill a TLB entry for the given address and cycle type: */
static int
_tme_ncr5380_bus_tlb_fill(struct tme_ncr5380 *ncr5380,
			  struct tme_bus_tlb *tlb,
			  tme_bus_addr32_t address,
			  tme_uint8_t cycle_type)
{
  struct tme_bus_tlb *tlb_volatile;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* copy our volatile TLB entry into the local storage: */
  tlb_volatile
    = &ncr5380->tme_ncr5380_dma_tlb;
  *tlb = *tlb_volatile;
  rc = TME_OK;

  /* if the TLB entry is valid, covers this address and allows the
     needed access, return success: */
  if (tme_bus_tlb_is_valid(tlb)
      && address >= (tme_bus_addr32_t) tlb->tme_bus_tlb_addr_first
      && address <= (tme_bus_addr32_t) tlb->tme_bus_tlb_addr_last
      && (((cycle_type == TME_BUS_CYCLE_READ
	    ? tlb->tme_bus_tlb_emulator_off_read
	    : tlb->tme_bus_tlb_emulator_off_write) != TME_EMULATOR_OFF_UNDEF)
	  || (tlb->tme_bus_tlb_cycles_ok & cycle_type))) {
    return (TME_OK);
  }
  
  /* get our bus connection: */
  conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					    ncr5380->tme_ncr5380_device.tme_bus_device_connection,
					    &ncr5380->tme_ncr5380_device.tme_bus_device_connection_rwlock);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);
      
  /* do the callout: */
  rc = (conn_bus != NULL
	? ((*conn_bus->tme_bus_tlb_fill)
	   (conn_bus,
	    tlb,
	    address,
	    cycle_type))
	: EAGAIN);
	
  /* lock the mutex: */
  tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

  /* if the call was successful, copy the TLB entry into the backing TLB entry: */
  if (rc == TME_OK) {
    *tlb_volatile = *tlb;
  }
  return (rc);
}

/* this does a bus cycle to or from our internal DMA buffer: */
static int
_tme_ncr5380_bus_cycle_dma(struct tme_ncr5380 *ncr5380,
			   struct tme_bus_tlb *tlb,
			   tme_bus_addr32_t address,
			   tme_uint8_t cycle_type)
{
  struct tme_bus_cycle cycle_init;
  int rc;

  /* use our internal DMA buffer: */ 
  cycle_init.tme_bus_cycle_buffer
    = &ncr5380->tme_ncr5380_int_dma;
  
  /* use the 8-bit bus router and an 8-bit cycle size: */
  cycle_init.tme_bus_cycle_lane_routing
    = &tme_ncr5380_router[0];
  cycle_init.tme_bus_cycle_size 
    = sizeof(tme_uint8_t);
  
  assert (tlb->tme_bus_tlb_addr_shift == 0);
  cycle_init.tme_bus_cycle_address
    = (address
       + tlb->tme_bus_tlb_addr_offset);
  
  cycle_init.tme_bus_cycle_buffer_increment
    = 1;

  cycle_init.tme_bus_cycle_type
    = cycle_type;

  cycle_init.tme_bus_cycle_port = 
    TME_BUS_CYCLE_PORT(0, TME_BUS8_LOG2);
  
  /* unlock the mutex: */
  tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);

  /* run the bus cycle: */
  rc = ((*tlb->tme_bus_tlb_cycle)
	(tlb->tme_bus_tlb_cycle_private,
	 &cycle_init));
	
  /* lock the mutex: */
  tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

  return (rc);
}

/* the NCR 5380 callout function.  it must be called with the mutex locked: */
static void
_tme_ncr5380_callout(struct tme_ncr5380 *ncr5380, int new_callouts)
{
  struct tme_scsi_connection *conn_scsi;
  struct tme_bus_connection *conn_bus;
  struct tme_bus_tlb tlb;
  tme_scsi_control_t scsi_control;
  tme_scsi_data_t scsi_data;
  tme_uint32_t scsi_events;
  tme_uint32_t scsi_actions;
  struct tme_scsi_dma scsi_dma_buffer;
  struct tme_scsi_dma *scsi_dma;
  int callouts, later_callouts;
  tme_bus_addr32_t address;
  tme_uint8_t cycle_type;
  int rc;
  int int_asserted;

  /* add in any new callouts: */
  ncr5380->tme_ncr5380_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (ncr5380->tme_ncr5380_callout_flags
      & TME_NCR5380_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  ncr5380->tme_ncr5380_callout_flags
    |= TME_NCR5380_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; ((callouts
	   = ncr5380->tme_ncr5380_callout_flags)
	  & TME_NCR5380_CALLOUTS_MASK); ) {

    /* clear the needed callouts: */
    ncr5380->tme_ncr5380_callout_flags
      = (callouts
	 & ~TME_NCR5380_CALLOUTS_MASK);
    callouts
      &= TME_NCR5380_CALLOUTS_MASK;
    
    /* NB that the ncr5380 structure is volatile across callouts, when
       we have the mutex unlocked.  once we decide to do something
       based on some part of the ncr5380 state, we load much of that
       state into locals, because we can't assume that, once the
       callout returns, we'll have the same state to make the same
       decision about doing the same work that we had before the
       callout.  adding and using more locals in this way should make
       things more thread-safe, in that we use more thread local
       storage (i.e., the stack) that isn't volatile. */

    /* NB that these callouts are in a deliberate order.  if our
       internal DMA buffer needs to be flushed, that has priority over
       calling out the terminal DMA address, since flushing the
       internal DMA buffer affects the terminal DMA address.
       similarly, if we need to call out the terminal DMA address,
       that takes priority over calling out an interrupt, since we
       have to give the DMA hardware a chance to update its state
       before the CPU is interrupted. */

    /* if our internal DMA buffer needs to be flushed: */
    if (callouts & TME_NCR5380_CALLOUT_FLUSH_INT_DMA) {

      /* get our DMA address: */
      address = ncr5380->tme_ncr5380_dma_address;

      /* call out for a writable TLB entry covering this address: */
      rc = _tme_ncr5380_bus_tlb_fill(ncr5380,
				     &tlb,
				     address,
				     TME_BUS_CYCLE_WRITE);

      /* if the callout succeeded: */
      if (rc == TME_OK) {

	/* call out the slow bus write cycle: */
	rc = _tme_ncr5380_bus_cycle_dma(ncr5380,
					&tlb,
					address,
					TME_BUS_CYCLE_WRITE);

	/* NB that we don't care about bus errors here.  there's no
	   way to signal them to the chip.  it's possible that other
	   DMA hardware feeding the chip may note the bus error,
	   though: */
	rc = TME_OK;
      }

      /* if all callouts succeeded: */
      if (rc == TME_OK) {
	
	/* the internal DMA buffer has been flushed, so we can
	   increment the DMA address: */
	if (ncr5380->tme_ncr5380_dma_address == address) {
	  ncr5380->tme_ncr5380_dma_address = address + 1;
	}
      }

      /* otherwise, at least one callout failed: */
      else {

	/* remember that this callout should be attempted again at
           some later time: */
	later_callouts |= TME_NCR5380_CALLOUT_FLUSH_INT_DMA;
      }
    }

    /* if we need to call out the terminal DMA address: */
    if (callouts & TME_NCR5380_CALLOUT_TERMINAL_DMA) {

      /* if the internal DMA buffer needs to be flushed, we can't make
	 this callout yet, so pretend that it failed: */
      if (later_callouts & TME_NCR5380_CALLOUT_FLUSH_INT_DMA) {
	rc = EAGAIN;
      }

      /* otherwise, we can make this callout: */
      else {

	/* get the terminal DMA address: */
	address = ncr5380->tme_ncr5380_dma_address;

	/* get our bus connection: */
	conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						  ncr5380->tme_ncr5380_device.tme_bus_device_connection,
						  &ncr5380->tme_ncr5380_device.tme_bus_device_connection_rwlock);

	/* unlock the mutex: */
	tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);
      
	/* do the callout: */
	rc = (conn_bus != NULL
	      ? ((*conn_bus->tme_bus_tlb_fill)
		 (conn_bus,
		  NULL,
		  address,
		  TME_BUS_CYCLE_UNDEF))
	      : TME_OK);
	
	/* lock the mutex: */
	tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

      }

      /* if at least one callout failed: */
      if (rc != TME_OK) {

	/* remember that this callout should be attempted again at
           some later time: */
	later_callouts |= TME_NCR5380_CALLOUT_TERMINAL_DMA;
      }
    }

    /* if our interrupt signal has changed: */
    int_asserted = ((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR] & TME_NCR5380_BSR_INT) != 0);
    if (!int_asserted != !ncr5380->tme_ncr5380_last_int_asserted) {

      /* if the internal DMA buffer needs to be flushed, or if we need
	 to call out the terminal DMA address, we can't make this
	 callout yet, so pretend that it failed: */
      if (later_callouts
	  & (TME_NCR5380_CALLOUT_FLUSH_INT_DMA
	     | TME_NCR5380_CALLOUT_TERMINAL_DMA)) {
	rc = EAGAIN;
      }

      /* otherwise, we can make this callout: */
      else {

	/* get our bus connection: */
	conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						  ncr5380->tme_ncr5380_device.tme_bus_device_connection,
						  &ncr5380->tme_ncr5380_device.tme_bus_device_connection_rwlock);

	/* unlock our mutex: */
	tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);
	
	/* call out the bus interrupt signal edge: */
	rc = (*conn_bus->tme_bus_signal)
	  (conn_bus,
	   TME_BUS_SIGNAL_INT_UNSPEC
	   | (int_asserted
	      ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	      : TME_BUS_SIGNAL_LEVEL_NEGATED));

	/* lock our mutex: */
	tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);
      }

      /* if all callouts succeeded: */
      if (rc == TME_OK) {
	  
	/* note the new state of the interrupt signal: */
	ncr5380->tme_ncr5380_last_int_asserted = int_asserted;
      }

      /* otherwise, at least one callout failed: */
      else {

	/* remember that this callout should be attempted again at
           some later time: */
	later_callouts |= TME_NCR5380_CALLOUT_INT;
      }
    }

    /* get our SCSI state: */
    scsi_control = ncr5380->tme_ncr5380_scsi_control;
    scsi_data = ncr5380->tme_ncr5380_scsi_data;
    scsi_events = ncr5380->tme_ncr5380_scsi_events;
    scsi_actions = ncr5380->tme_ncr5380_scsi_actions;

    /* if our SCSI state has changed, and we're either not doing a DMA
       action or our internal DMA buffer doesn't need to be flushed
       (otherwise, the SCSI callout will have to wait, since it may
       need to reuse the internal DMA buffer): */
    if ((scsi_control != ncr5380->tme_ncr5380_last_scsi_control
	 || scsi_data != ncr5380->tme_ncr5380_last_scsi_data
	 || scsi_events != ncr5380->tme_ncr5380_last_scsi_events
	 || scsi_actions != ncr5380->tme_ncr5380_last_scsi_actions)
	&& !(later_callouts & TME_NCR5380_CALLOUT_FLUSH_INT_DMA)) {

      /* assume this isn't a DMA SCSI callout: */
      scsi_dma = NULL;

      /* assume that any callouts we need to set up the SCSI cycle
         callout will succeed: */
      rc = TME_OK;

      /* if this is a DMA SCSI callout: */
      if (scsi_actions
	  & (TME_SCSI_ACTION_DMA_INITIATOR
	     | TME_SCSI_ACTION_DMA_TARGET)) {

	/* get our DMA address: */
	address = ncr5380->tme_ncr5380_dma_address;

	/* get the DMA cycle type.  I_O is true if the initiator should
	   be writing and the target should be writing, and it is false
	   if the initiator should be reading and the target should be
	   writing: */
	cycle_type = (((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB]
			& TME_NCR5380_CSB_I_O)
		       ^ ((ncr5380->tme_ncr5380_scsi_actions & TME_SCSI_ACTION_DMA_TARGET)
			  ? TME_NCR5380_CSB_I_O
			  : 0))
		      ? TME_BUS_CYCLE_WRITE
		      : TME_BUS_CYCLE_READ);

	/* call out for a TLB entry covering this address: */
	rc = _tme_ncr5380_bus_tlb_fill(ncr5380,
				       &tlb,
				       address,
				       cycle_type);

	/* if the callout failed with EAGAIN: */
	if (rc == EAGAIN) {

	  /* call out a SCSI cycle that just waits for a bus change: */
	  scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
	  scsi_actions = TME_SCSI_ACTION_NONE;
	  scsi_data = 0;

	  /* act like the callout succeeded: */
	  rc = TME_OK;
	}

	/* if the callout failed with ENOENT: */
	else if (rc == ENOENT) {

	  /* set EDMA, update, and continue: */
	  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_BSR,
			      ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR]
			      | TME_NCR5380_BSR_EDMA);
	  ncr5380->tme_ncr5380_callout_flags
	    |= (_tme_ncr5380_update(ncr5380)
		| later_callouts);
	  continue;
	}

	/* otherwise, if the callout succeeded: */
	else if (rc == TME_OK) {

	  /* start the DMA structure.  we assume that this TLB entry
	     allows fast transfers, and we fill in the resid field
	     appropriately: */
	  /* XXX parity? */
	  scsi_dma = &scsi_dma_buffer;
	  scsi_dma_buffer.tme_scsi_dma_flags = TME_SCSI_DMA_8BIT;
	  scsi_dma_buffer.tme_scsi_dma_resid = (tlb.tme_bus_tlb_addr_last - address) + 1;
	  scsi_dma_buffer.tme_scsi_dma_sync_offset = 0;
	  scsi_dma_buffer.tme_scsi_dma_sync_period = 0;

	  /* if we're doing a DMA read: */
	  if (cycle_type == TME_BUS_CYCLE_READ) {

#ifndef NDEBUG
	    /* we aren't doing a DMA write: */
	    scsi_dma_buffer.tme_scsi_dma_in = NULL;
#endif /* NDEBUG */

	    /* if this TLB entry supports fast reading: */
	    if (tlb.tme_bus_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {

	      /* read from the fast reading address: */
	      /* XXX FIXME - this breaks volatile: */
	      scsi_dma_buffer.tme_scsi_dma_out = (const tme_uint8_t *) tlb.tme_bus_tlb_emulator_off_read + address;
	    }

	    /* otherwise, this TLB entry doesn't support fast reading: */
	    else {

	      /* call out the slow bus read cycle: */
	      rc = _tme_ncr5380_bus_cycle_dma(ncr5380,
					      &tlb,
					      address,
					      TME_BUS_CYCLE_READ);

	      /* NB that we don't care about bus errors here.  there's no
		 way to signal them to the chip.  it's possible that other
		 DMA hardware feeding the chip may note the bus error,
		 though: */
	      rc = TME_OK;

	      /* read from the internal DMA buffer: */
	      scsi_dma_buffer.tme_scsi_dma_out = &ncr5380->tme_ncr5380_int_dma;
	      scsi_dma_buffer.tme_scsi_dma_resid = sizeof(ncr5380->tme_ncr5380_int_dma);
	    }
	  }

	  /* otherwise, we're doing a DMA write: */
	  else {

#ifndef NDEBUG
	    /* we aren't doing a DMA write: */
	    scsi_dma_buffer.tme_scsi_dma_out = NULL;
#endif /* NDEBUG */

	    /* if this TLB entry supports fast writing: */
	    if (tlb.tme_bus_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {

	      /* write to the fast writing address: */
	      /* XXX FIXME - this breaks volatile: */
	      scsi_dma_buffer.tme_scsi_dma_in = (tme_uint8_t *) tlb.tme_bus_tlb_emulator_off_write + address;
	    }

	    /* otherwise, this TLB entry doesn't support fast writing: */
	    else {

	      /* write to the internal DMA buffer: */
	      scsi_dma_buffer.tme_scsi_dma_in = &ncr5380->tme_ncr5380_int_dma;
	      scsi_dma_buffer.tme_scsi_dma_resid = sizeof(ncr5380->tme_ncr5380_int_dma);
	    }
	  }
	}
      }

      /* if this callout has succeeded so far: */
      if (rc == TME_OK) {

	/* get this card's SCSI connection: */
	conn_scsi = ncr5380->tme_ncr5380_scsi_connection;

	/* remember the last SCSI cycle called out: */
	ncr5380->tme_ncr5380_last_scsi_control = scsi_control;
	ncr5380->tme_ncr5380_last_scsi_data = scsi_data;
	ncr5380->tme_ncr5380_last_scsi_events = scsi_events;
	ncr5380->tme_ncr5380_last_scsi_actions = scsi_actions;

	/* remember the DMA residual: */
	ncr5380->tme_ncr5380_dma_resid = (scsi_dma != NULL
					  ? scsi_dma->tme_scsi_dma_resid
					  : 0);

	/* unlock the mutex: */
	tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);
      
	/* do the callout: */
	rc = (conn_scsi != NULL
	      ? ((*conn_scsi->tme_scsi_connection_cycle)
		 (conn_scsi,
		  scsi_control,
		  scsi_data,
		  scsi_events,
		  scsi_actions,
		  scsi_dma))
	      : TME_OK);
	
	/* lock the mutex: */
	tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);
      }

      /* if at least one callout failed: */
      if (rc != TME_OK) {

	/* poison the last SCSI cycle called out, so we'll call out
	   again later.  we never call out a cycle with both
	   TME_SCSI_EVENT_NONE and TME_SCSI_ACTION_NONE, so this is
	   sufficient: */
	ncr5380->tme_ncr5380_last_scsi_events = TME_SCSI_EVENT_NONE;
	ncr5380->tme_ncr5380_last_scsi_actions = TME_SCSI_ACTION_NONE;

	/* remember that this callout should be attempted again at
           some later time: */
	later_callouts |= TME_NCR5380_CALLOUT_SCSI_CYCLE;
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  ncr5380->tme_ncr5380_callout_flags = later_callouts;
}

/* this is called for an event on the SCSI bus: */
static int
_tme_ncr5380_scsi_cycle(struct tme_scsi_connection *conn_scsi,
			tme_scsi_control_t scsi_control,
			tme_scsi_data_t scsi_data,
			tme_uint32_t scsi_events_triggered,
			tme_uint32_t scsi_actions_taken,
			const struct tme_scsi_dma *scsi_dma)
{
  struct tme_ncr5380 *ncr5380;
  unsigned long count;
  tme_uint8_t icr_old;
  tme_uint8_t icr_new;
  tme_uint8_t csb_old;
  tme_uint8_t csb_new;
  tme_uint8_t bsr_old;
  tme_uint8_t bsr_new;
  tme_scsi_data_t ids;
  int new_callouts;
  
  /* recover our data structure: */
  ncr5380 = conn_scsi->tme_scsi_connection.tme_connection_element->tme_element_private;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

  /* we no longer have any events or actions pending in the SCSI bus: */
  ncr5380->tme_ncr5380_last_scsi_events = TME_SCSI_EVENT_NONE;
  ncr5380->tme_ncr5380_last_scsi_actions = TME_SCSI_ACTION_NONE;

  /* get old register values: */
  icr_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR];
  csb_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB];
  bsr_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR];

  /* start some new register values: */
  icr_new = icr_old;
  csb_new = 0;
  bsr_new
    = (bsr_old
       & ~(TME_NCR5380_BSR_ACK
	   | TME_NCR5380_BSR_ATN
	   | TME_NCR5380_BSR_SPER));

  /* update the Current SCSI Bus (CSB) status and Bus Status Register with the
     state of the SCSI control signals: */
#define _TME_NCR5380_X_CONTROL(reg, _reg, _control)\
do {						\
  if (scsi_control & (_control)) {		\
    reg |= (_reg);				\
  }						\
} while (/* CONSTCOND */ 0)
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_REQ, TME_SCSI_SIGNAL_REQ);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_MSG, TME_SCSI_SIGNAL_MSG);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_C_D, TME_SCSI_SIGNAL_C_D);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_I_O, TME_SCSI_SIGNAL_I_O);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_DBP, TME_SCSI_SIGNAL_DBP);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_BSY, TME_SCSI_SIGNAL_BSY);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_RST, TME_SCSI_SIGNAL_RST);
  _TME_NCR5380_X_CONTROL(csb_new, TME_NCR5380_CSB_SEL, TME_SCSI_SIGNAL_SEL);
  _TME_NCR5380_X_CONTROL(bsr_new, TME_NCR5380_BSR_ACK, TME_SCSI_SIGNAL_ACK);
  _TME_NCR5380_X_CONTROL(bsr_new, TME_NCR5380_BSR_ATN, TME_SCSI_SIGNAL_ATN);
#undef _TME_NCR5380_X_CONTROL

  /* XXX set SPER in BSR if there is a parity error: */

  /* if we are being selected or reselected: */
  ids = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_SER];
  if (TME_SCSI_ID_SELECTED(ids, scsi_control, scsi_data)
      || TME_SCSI_ID_RESELECTED(ids, scsi_control, scsi_data)) {

    /* call out an interrupt: */
    bsr_new |= TME_NCR5380_BSR_INT;
    new_callouts |= TME_NCR5380_CALLOUT_INT;
  }

  /* if our arbitration is in progress, set AIP in the ICR: */
  if (scsi_actions_taken & TME_SCSI_ACTION_ARBITRATE_HALF) {
    icr_new |= TME_NCR5380_ICR_AIP;
  }

  /* if arbitration is in progress, but SEL is set on the bus, and
     we're not the ones asserting it, arbitration has been lost,
     so set LA: */
  if ((icr_new & TME_NCR5380_ICR_AIP)
      && (scsi_control & TME_SCSI_SIGNAL_SEL)
      && !(icr_new & TME_NCR5380_ICR_SEL)) {
    icr_new |= TME_NCR5380_ICR_LA;
  }

  /* if DMA was running: */
  if (scsi_actions_taken
      & (TME_SCSI_ACTION_DMA_INITIATOR
	 | TME_SCSI_ACTION_DMA_TARGET)) {

    /* get the count of bytes transferred: */
    count = ncr5380->tme_ncr5380_dma_resid - scsi_dma->tme_scsi_dma_resid;

    /* update our DMA address register: */
    ncr5380->tme_ncr5380_dma_address += count;
    
    /* if we were doing a DMA read into our internal DMA buffer, we
       need to flush it: */
    if ((scsi_dma->tme_scsi_dma_in - count)
	== &ncr5380->tme_ncr5380_int_dma) {
      new_callouts |= TME_NCR5380_CALLOUT_FLUSH_INT_DMA;
    }
  }

  /* set new register values: */
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_CSD, scsi_data);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_ICR, icr_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_CSB, csb_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_BSR, bsr_new);

  /* run the update function: */
  new_callouts |= _tme_ncr5380_update(ncr5380);

  /* make any new callouts: */
  _tme_ncr5380_callout(ncr5380, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);

  return (TME_OK);
}

/* the ncr5380 bus signal handler: */
static int
_tme_ncr5380_signal(void *_ncr5380, 
		   unsigned int signal)
{
  struct tme_ncr5380 *ncr5380;
  int new_callouts;
  unsigned int level;

  /* recover our data structure: */
  ncr5380 = (struct tme_ncr5380 *) _ncr5380;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

  /* take out the signal level: */
  level = signal & TME_BUS_SIGNAL_LEVEL_MASK;
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* dispatch on the generic bus signals: */
  switch (signal) {

  case TME_BUS_SIGNAL_RESET:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      new_callouts |= _tme_ncr5380_reset(ncr5380, FALSE);
    }
    new_callouts |= _tme_ncr5380_update(ncr5380);
    break;

  case TME_BUS_SIGNAL_DACK:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      new_callouts |= TME_NCR5380_CALLOUT_SCSI_CYCLE;
    }
    break;

  default:
    break;
  }

  /* make any new callouts: */
  _tme_ncr5380_callout(ncr5380, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the NCR 5380 bus cycle handler: */
static int
_tme_ncr5380_bus_cycle(void *_ncr5380,
		       struct tme_bus_cycle *cycle_init)
{
  struct tme_ncr5380 *ncr5380;
  tme_uint8_t icr_old;
  tme_uint8_t icr_new;
  tme_uint8_t mr2_old;
  tme_uint8_t mr2_new;
  tme_uint8_t bsr_old;
  tme_uint8_t bsr_new;
  int new_callouts;
  tme_bus_addr32_t address;
  tme_uint8_t cycle_size;

  /* recover our data structure: */
  ncr5380 = (struct tme_ncr5380 *) _ncr5380;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* get the address and cycle size: */
  address = cycle_init->tme_bus_cycle_address;
  cycle_size = cycle_init->tme_bus_cycle_size;

  /* lock the mutex: */
  tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

  /* get old register values: */
  icr_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR];
  mr2_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2];
  bsr_old = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR];

  /* run the bus cycle.  if this is a read cycle to a read-only register,
     read from the read-only register section of the register array: */
  assert (cycle_init->tme_bus_cycle_size = sizeof(tme_uint8_t));
  tme_bus_cycle_xfer_memory(cycle_init,
			    &ncr5380->tme_ncr5380_regs[0]
			    + ((cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ
				&& (TME_NCR5380_REGS_RO & TME_BIT(address)))
			       ? TME_NCR5380_SIZ_REGS
			       : 0),
			    TME_NCR5380_SIZ_REGS - 1);

  /* get new register values and temporarily put back the old values: */
  icr_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR];
  mr2_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2];
  bsr_new = ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR];
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_ICR] = icr_old;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_MR2] = mr2_old;
  ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_BSR] = bsr_old;

#define _TME_NCR5380_REG_IS(reg) TME_RANGES_OVERLAP(address, address + cycle_size - 1, TME_NCR5380_REG_INDEX(reg), TME_NCR5380_REG_INDEX(reg) + sizeof(tme_uint8_t) - 1)

  /* if this was a write: */
  if (cycle_init->tme_bus_cycle_type
      == TME_BUS_CYCLE_WRITE) {

    /* replace the DIFF and TEST write-only bits in the ICR with the
       old LA and AIP read-only bits: */
    icr_new = ((icr_new
		& ~(TME_NCR5380_ICR_DIFF
		    | TME_NCR5380_ICR_TEST))
	       | (icr_old
		  & (TME_NCR5380_ICR_DIFF
		     | TME_NCR5380_ICR_TEST)));

    /* if BSY is now set in MR2: */
    if (!(mr2_old & TME_NCR5380_MR2_BSY)
	&& (mr2_new & TME_NCR5380_MR2_BSY)) {

      /* "When this bit goes active the lower 6 bits of the ICR are
         reset and all signals removed from the SCSI bus. This is used
         to check for valid TARGET connection." */
      icr_new &= ~(TME_NCR5380_ICR_DBUS
		   | TME_NCR5380_ICR_ATN
		   | TME_NCR5380_ICR_SEL
		   | TME_NCR5380_ICR_BSY
		   | TME_NCR5380_ICR_ACK
		   | TME_NCR5380_ICR_LA);
    }

    /* if this was a write of the SDS register: */
    if (_TME_NCR5380_REG_IS(TME_NCR5380_REG_SDS)) {

      /* if BSY is set, and DMA isn't already running, start DMA: */
      if ((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB]
	   & TME_NCR5380_CSB_BSY)
	  && !ncr5380->tme_ncr5380_dma_running) {
	tme_log(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		100, TME_OK,
		(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		 "SDS written, DMA now running"));
	ncr5380->tme_ncr5380_dma_running = TRUE;
	ncr5380->tme_ncr5380_dma_address = 0;

	/* if we're sending on the SCSI bus as an initiator, we
           prefetch: */
	ncr5380->tme_ncr5380_dma_prefetch
	  = ((mr2_new & TME_NCR5380_MR2_TARG)
	     ? 0
	     : 1);
      }
    }

    /* if this was a write of the SDT register: */
    if (_TME_NCR5380_REG_IS(TME_NCR5380_REG_SDT)) {

      /* XXX what happens if TARG is not set? */
      assert ((mr2_new & TME_NCR5380_MR2_TARG) != 0);
      
      /* if BSY is set, and DMA isn't already running, start DMA: */
      if ((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB]
	   & TME_NCR5380_CSB_BSY)
	  && !ncr5380->tme_ncr5380_dma_running) {
	tme_log(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		100, TME_OK,
		(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		 "SDT written, DMA now running"));
	ncr5380->tme_ncr5380_dma_running = TRUE;
	ncr5380->tme_ncr5380_dma_address = 0;
	ncr5380->tme_ncr5380_dma_prefetch = 0;
      }
    }

    /* if this was a write of the SDI register: */
    if (_TME_NCR5380_REG_IS(TME_NCR5380_REG_SDI)) {

      /* XXX what happens if TARG is set? */
      assert ((mr2_new & TME_NCR5380_MR2_TARG) == 0);
      
      /* if BSY is set, and DMA isn't already running, start DMA: */
      if ((ncr5380->tme_ncr5380_regs[TME_NCR5380_REG_CSB]
	   & TME_NCR5380_CSB_BSY)
	  && !ncr5380->tme_ncr5380_dma_running) {
	tme_log(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		100, TME_OK,
		(&ncr5380->tme_ncr5380_element->tme_element_log_handle,
		 "SDI written, DMA now running"));
	ncr5380->tme_ncr5380_dma_running = TRUE;
	ncr5380->tme_ncr5380_dma_address = 0;
	ncr5380->tme_ncr5380_dma_prefetch = 0;
      }
    }

  }

  /* otherwise, this was a read: */
  else {

    /* if this was a read of the RPI register: */
    if (_TME_NCR5380_REG_IS(TME_NCR5380_REG_RPI)) {

      /* "Reading this register resets the SCSI parity, Busy Loss and
         Interrupt Request latches." */
      bsr_new &= ~(TME_NCR5380_BSR_SPER
		   | TME_NCR5380_BSR_BSY
		   | TME_NCR5380_BSR_INT);
      new_callouts |= TME_NCR5380_CALLOUT_INT;
    }
  }

#undef _TME_NCR5380_REG_IS

  /* set new register values: */
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_ICR, icr_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_MR2, mr2_new);
  TME_NCR5380_REG_PUT(ncr5380, TME_NCR5380_REG_BSR, bsr_new);

  /* update: */
  new_callouts |= _tme_ncr5380_update(ncr5380);

  /* make any new callouts: */
  _tme_ncr5380_callout(ncr5380, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the NCR 5380 TLB filler: */
static int
_tme_ncr5380_tlb_fill(void *_ncr5380, 
		      struct tme_bus_tlb *tlb, 
		      tme_bus_addr_t address,
		      unsigned int cycles)
{
  struct tme_ncr5380 *ncr5380;

  /* recover our data structure: */
  ncr5380 = (struct tme_ncr5380 *) _ncr5380;

  /* the address must be within range: */
  assert(address < TME_NCR5380_SIZ_REGS);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* if this is a read: */
  if (cycles & TME_BUS_CYCLE_READ) {
    
    /* this TLB entry covers this register only: */
    tlb->tme_bus_tlb_addr_first = address;
    tlb->tme_bus_tlb_addr_last = address;

    /* allow only reading: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ;
  }

  /* otherwise, we're writing: */
  else {

    /* this TLB entry covers all registers: */
    tlb->tme_bus_tlb_addr_first = 0;
    tlb->tme_bus_tlb_addr_last = TME_NCR5380_SIZ_REGS - 1;

    /* allow only writing: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_WRITE;
  }

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle = _tme_ncr5380_bus_cycle;

  /* our bus cycle handler private data: */
  tlb->tme_bus_tlb_cycle_private = ncr5380;

  return (TME_OK);
}

/* this makes a new bus connection: */
static int
_tme_ncr5380_connection_make_bus(struct tme_connection *conn,
				 unsigned int state)
{
  struct tme_ncr5380 *ncr5380;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* recover our data structure: */
  ncr5380 = conn->tme_connection_element->tme_element_private;

  /* call the bus device connection maker: */
  rc = tme_bus_device_connection_make(conn, state);

  /* if the full connection was successful, and we don't have a TLB
     set yet, allocate it: */
  if (rc == TME_OK
      && state == TME_CONNECTION_FULL
      && !ncr5380->tme_ncr5380_dma_tlb_added) {

    /* get our bus connection: */
    conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					      ncr5380->tme_ncr5380_device.tme_bus_device_connection,
					      &ncr5380->tme_ncr5380_device.tme_bus_device_connection_rwlock);

    /* allocate the TLB set: */
    rc = tme_bus_device_tlb_set_add(&ncr5380->tme_ncr5380_device,
				    1, 
				    &ncr5380->tme_ncr5380_dma_tlb);
    assert (rc == TME_OK);
    ncr5380->tme_ncr5380_dma_tlb_added = TRUE;
  }

  return (rc);
}

/* this makes a new SCSI connection: */
static int
_tme_ncr5380_connection_make_scsi(struct tme_connection *conn,
				  unsigned int state)
{
  struct tme_ncr5380 *ncr5380;
  struct tme_scsi_connection *conn_scsi;
  struct tme_scsi_connection *conn_scsi_other;

  /* recover our data structures: */
  ncr5380 = conn->tme_connection_element->tme_element_private;
  conn_scsi = (struct tme_scsi_connection *) conn;
  conn_scsi_other = (struct tme_scsi_connection *) conn->tme_connection_other;

  /* both sides must be SCSI connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SCSI);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SCSI);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&ncr5380->tme_ncr5380_mutex);

    /* save our connection: */
    ncr5380->tme_ncr5380_scsi_connection = conn_scsi_other;

    /* call out a cycle that asserts no signals and runs the
       wait-change action: */
    ncr5380->tme_ncr5380_last_scsi_events = TME_SCSI_EVENT_NONE;
    ncr5380->tme_ncr5380_scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
    ncr5380->tme_ncr5380_scsi_actions = TME_SCSI_ACTION_NONE;
    ncr5380->tme_ncr5380_scsi_control = 0;
    ncr5380->tme_ncr5380_scsi_data = 0;
    _tme_ncr5380_callout(ncr5380, TME_NCR5380_CALLOUT_SCSI_CYCLE);

    /* unlock our mutex: */
    tme_mutex_unlock(&ncr5380->tme_ncr5380_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_ncr5380_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a NCR 5380: */
static int
_tme_ncr5380_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_ncr5380 *ncr5380;
  struct tme_scsi_connection *conn_scsi;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  ncr5380 = (struct tme_ncr5380 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* since we need to allocate a TLB set when we make our bus
     connection, make sure any generic bus device connection sides use
     our connection maker: */
  for (conn = *_conns;
       conn != NULL;
       conn = conn->tme_connection_next) {
    if ((conn->tme_connection_type
	 == TME_CONNECTION_BUS_GENERIC)
	&& (conn->tme_connection_make
	    == tme_bus_device_connection_make)) {
      conn->tme_connection_make
	= _tme_ncr5380_connection_make_bus;
    }
  }

  /* if we don't have a SCSI connection, make one: */
  if (ncr5380->tme_ncr5380_scsi_connection == NULL) {

    /* allocate the new SCSI connection: */
    conn_scsi = tme_new0(struct tme_scsi_connection, 1);
    conn = &conn_scsi->tme_scsi_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SCSI;
    conn->tme_connection_score = tme_scsi_connection_score;
    conn->tme_connection_make = _tme_ncr5380_connection_make_scsi;
    conn->tme_connection_break = _tme_ncr5380_connection_break;

    /* fill in the SCSI connection: */
    conn_scsi->tme_scsi_connection_cycle = _tme_ncr5380_scsi_cycle;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new NCR 5380 function: */
TME_ELEMENT_NEW_DECL(tme_ic_ncr5380) {
  struct tme_ncr5380 *ncr5380;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  for (;;) {

    if (0) {
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

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the NCR 5380 structure: */
  ncr5380 = tme_new0(struct tme_ncr5380, 1);
  ncr5380->tme_ncr5380_element = element;
  tme_mutex_init(&ncr5380->tme_ncr5380_mutex);

  /* initialize our simple bus device descriptor: */
  ncr5380->tme_ncr5380_device.tme_bus_device_element = element;
  ncr5380->tme_ncr5380_device.tme_bus_device_tlb_fill = _tme_ncr5380_tlb_fill;
  ncr5380->tme_ncr5380_device.tme_bus_device_address_last = TME_NCR5380_SIZ_REGS - 1;
  ncr5380->tme_ncr5380_device.tme_bus_device_signal = _tme_ncr5380_signal;

  /* fill the element: */
  element->tme_element_private = ncr5380;
  element->tme_element_connections_new = _tme_ncr5380_connections_new;

  return (TME_OK);
}
