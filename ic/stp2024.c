/* $Id: stp2024.c,v 1.3 2009/11/08 16:21:32 fredette Exp $ */

/* ic/stp2024.c - STP2024 (Aurora Personality Chip DMA) implementation: */

/*
 * Copyright (c) 2009 Matt Fredette
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
_TME_RCSID("$Id: stp2024.c,v 1.3 2009/11/08 16:21:32 fredette Exp $");

/* includes: */
#include <tme/element.h>
#undef TME_BUS_VERSION
#define TME_BUS_VERSION TME_X_VERSION(0, 0)
#include <tme/generic/bus.h>
#include <tme/completion.h>

/* macros: */

/* the device offsets: */
#define TME_STP2024_OFFSET_POWER_IDLE		(0xa000000)
#define TME_STP2024_OFFSET_POWER_FANCTL		(0xa000020)
#define TME_STP2024_OFFSET_POWER_CPOWER		(0xa000024)
#define TME_STP2024_OFFSET_POWER_BPORT		(0xa000030)
#define TME_STP2024_OFFSET_CODEC_REG0		(0xc000000)
#define TME_STP2024_OFFSET_CODEC_REG1		(0xc000004)
#define TME_STP2024_OFFSET_CODEC_REG2		(0xc000008)
#define TME_STP2024_OFFSET_CODEC_REG3		(0xc00000c)
#define TME_STP2024_OFFSET_DMA_CSR		(0xc000010)
#define TME_STP2024_OFFSET_DMA_CVA		(0xc000020)
#define TME_STP2024_OFFSET_DMA_CC		(0xc000024)
#define TME_STP2024_OFFSET_DMA_CNVA		(0xc000028)
#define TME_STP2024_OFFSET_DMA_CNC		(0xc00002c)
#define TME_STP2024_OFFSET_DMA_PVA		(0xc000030)
#define TME_STP2024_OFFSET_DMA_PC		(0xc000034)
#define TME_STP2024_OFFSET_DMA_PNVA		(0xc000038)
#define TME_STP2024_OFFSET_DMA_PNC		(0xc00003c)

/* the fan control register: */
#define TME_STP2024_POWER_FANCTL_LOW		(1 << 0)
#define TME_STP2024_POWER_FANCTL_ID		(0xe)

/* the DMA CSR: */
#define TME_STP2024_DMA_CSR_ID			(0xff << 24)
#define TME_STP2024_DMA_CSR_IP			(1 << 23)
#define TME_STP2024_DMA_CSR_PI			(1 << 22)
#define TME_STP2024_DMA_CSR_CI			(1 << 21)
#define TME_STP2024_DMA_CSR_EI			(1 << 20)
#define TME_STP2024_DMA_CSR_IE			(1 << 19)
#define TME_STP2024_DMA_CSR_PIE			(1 << 18)
#define TME_STP2024_DMA_CSR_CIE			(1 << 17)
#define TME_STP2024_DMA_CSR_EIE			(1 << 16)
#define TME_STP2024_DMA_CSR_PMI			(1 << 15)
#define TME_STP2024_DMA_CSR_PM			(1 << 14)
#define TME_STP2024_DMA_CSR_PD			(1 << 13)
#define TME_STP2024_DMA_CSR_PMIE		(1 << 12)
#define TME_STP2024_DMA_CSR_CM			(1 << 11)
#define TME_STP2024_DMA_CSR_CD			(1 << 10)
#define TME_STP2024_DMA_CSR_CMI			(1 << 9)
#define TME_STP2024_DMA_CSR_CMIE		(1 << 8)
#define TME_STP2024_DMA_CSR_PPAUSE		(1 << 7)
#define TME_STP2024_DMA_CSR_CPAUSE		(1 << 6)
#define TME_STP2024_DMA_CSR_PDN			(1 << 5)
#define TME_STP2024_DMA_CSR_LOOPBACK		(1 << 4)
#define TME_STP2024_DMA_CSR_PGO			(1 << 3)
#define TME_STP2024_DMA_CSR_CGO			(1 << 2)
						/* bit 1 unused */
#define TME_STP2024_DMA_CSR_RESET		(1 << 0)

/* the DMA CSR bits for the two pipelines: */
#define TME_STP2024_DMA_XCSR(pipeline)		\
  ((pipeline)					\
   ? (TME_STP2024_DMA_CSR_PI			\
      | TME_STP2024_DMA_CSR_PIE			\
      | TME_STP2024_DMA_CSR_PMI			\
      | TME_STP2024_DMA_CSR_PM			\
      | TME_STP2024_DMA_CSR_PD			\
      | TME_STP2024_DMA_CSR_PMIE		\
      | TME_STP2024_DMA_CSR_PPAUSE		\
      | TME_STP2024_DMA_CSR_PGO)		\
   : (TME_STP2024_DMA_CSR_CI			\
      | TME_STP2024_DMA_CSR_CIE			\
      | TME_STP2024_DMA_CSR_CMI			\
      | TME_STP2024_DMA_CSR_CM			\
      | TME_STP2024_DMA_CSR_CD			\
      | TME_STP2024_DMA_CSR_CMIE		\
      | TME_STP2024_DMA_CSR_CPAUSE		\
      | TME_STP2024_DMA_CSR_CGO))

/* connection types: */
#define TME_STP2024_CONN_NULL			(0)
#define TME_STP2024_CONN_SBUS			(1)
#define TME_STP2024_CONN_CODEC			(2)
#define TME_STP2024_CONN_COUNT			(3)

/* the codec divides its bus address space in half.  the first
   is for playback, the second is record: */
#define TME_STP2024_CODEC_ADDRESS_MASK		((0 - (tme_bus_addr_t) 1) >> 1)

#define TME_STP2024_BUS_TRANSITION		(1)

/* structures: */

/* the device: */
struct tme_stp2024 {

  /* backpointer to our element: */
  struct tme_element *tme_stp2024_element;

  /* the mutex protecting the device: */
  tme_mutex_t tme_stp2024_mutex;

  /* the bus connections: */
  struct tme_bus_connection *tme_stp2024_conns[TME_STP2024_CONN_COUNT];

  /* the fan control register: */
  tme_uint8_t tme_stp2024_power_fanctl;

  /* the DMA registers: */
  tme_uint32_t tme_stp2024_dma_csr;
  tme_uint32_t tme_stp2024_dma_xcsr[2];
  tme_uint32_t tme_stp2024_dma_xva[2];
  tme_uint32_t tme_stp2024_dma_xc[2];
  tme_uint32_t tme_stp2024_dma_xnva[2];
  tme_uint32_t tme_stp2024_dma_xnc[2];
  tme_bus_addr_t tme_stp2024_dma_codec_xaddress[2];

#if TME_STP2024_BUS_TRANSITION

  /* the codec register TLB token: */
  struct tme_token tme_stp2024_codec_cycle_tlb_token;

#endif /* TME_STP2024_BUS_TRANSITION */
};

/* an stp2024 internal bus connection: */
struct tme_stp2024_connection {

  /* the external bus connection: */
  struct tme_bus_connection tme_stp2024_connection;

  /* this is nonzero if a TME_CONNECTION_BUS_GENERIC chip connection
     is for the registers: */
  unsigned int tme_stp2024_connection_which;
};

/* globals: */

/* the audio bus subregion: */
static const struct tme_bus_subregion _tme_stp2024_subregion_audio = {

  /* the first and last addresses, starting from zero, of this
     subregion: */
  TME_STP2024_OFFSET_CODEC_REG0,
  TME_STP2024_OFFSET_DMA_PNC + sizeof(tme_uint32_t) - 1,

  /* there are no other subregions for this bus connection: */
  (const struct tme_bus_subregion *) NULL
};

/* the power-management bus subregion: */
static const struct tme_bus_subregion _tme_stp2024_subregion_power = {

  /* the first and last addresses, starting from zero, of this
     subregion: */
  TME_STP2024_OFFSET_POWER_IDLE,
  TME_STP2024_OFFSET_POWER_BPORT,

  /* the next subregion for this bus connection: */
  &_tme_stp2024_subregion_audio
};

#define _tme_stp2024_subregion_first _tme_stp2024_subregion_power

/* this advances a DMA pipeline: */
static int
_tme_stp2024_dma_advance(struct tme_stp2024 *stp2024,
			 tme_bus_addr_t codec_xaddress,
			 tme_uint32_t xcsr_empty)
{
  int pipeline;
  tme_uint32_t advance;

  /* get the pipeline: */
  pipeline = (codec_xaddress > TME_STP2024_CODEC_ADDRESS_MASK);
  codec_xaddress &= TME_STP2024_CODEC_ADDRESS_MASK;

  /* see how much the codec has advanced in this pipeline: */
  advance
    = (((tme_uint32_t) 
	(codec_xaddress
	 - stp2024->tme_stp2024_dma_codec_xaddress[pipeline]))
       & (tme_uint32_t) TME_STP2024_CODEC_ADDRESS_MASK);

  /* the codec can't advance more than the count of the current
     buffer; it's supposed to advance to that boundary before going
     beyond it to the next buffer: */
  assert (advance <= stp2024->tme_stp2024_dma_xc[pipeline]);

  /* advance the pipeline: */
  stp2024->tme_stp2024_dma_xva[pipeline] += advance;
  stp2024->tme_stp2024_dma_xc[pipeline] -= advance;

  /* while the count is zero: */
  for (; stp2024->tme_stp2024_dma_xc[pipeline] == 0; ) {

    /* if the next buffer is dirty: */
    if (stp2024->tme_stp2024_dma_xcsr[pipeline]
	& (TME_STP2024_DMA_CSR_PD
	   | TME_STP2024_DMA_CSR_CD)) {

      /* possibly mark this pipeline as empty: */
      stp2024->tme_stp2024_dma_xcsr[pipeline] |= xcsr_empty;
      break;
    }

    /* advance to the next buffer: */
    stp2024->tme_stp2024_dma_xva[pipeline]
      = stp2024->tme_stp2024_dma_xnva[pipeline];
    stp2024->tme_stp2024_dma_xc[pipeline]
      = stp2024->tme_stp2024_dma_xnc[pipeline];

    /* the next buffer is dirty: */
    stp2024->tme_stp2024_dma_xcsr[pipeline]
      |= (TME_STP2024_DMA_CSR_PD
	  | TME_STP2024_DMA_CSR_CD);
  }

  return (pipeline);
}

/* this enters: */
static struct tme_stp2024 *
_tme_stp2024_enter(struct tme_stp2024 *stp2024)
{

  /* lock the mutex: */
  tme_mutex_lock(&stp2024->tme_stp2024_mutex);

  return (stp2024);
}
#define _tme_stp2024_enter_bus(conn_bus) \
  ((struct tme_stp2024 *) _tme_stp2024_enter((struct tme_stp2024 *) (conn_bus)->tme_bus_connection.tme_connection_element->tme_element_private))

/* this leaves: */
#define _tme_stp2024_leave(stp2024) tme_mutex_unlock(&(stp2024)->tme_stp2024_mutex)

/* this busies a generic bus connection: */
#define _tme_stp2024_busy_bus(stp2024, conn_index) \
  ((stp2024)->tme_stp2024_conns[conn_index])

/* this unbusies a generic bus connection: */
#define _tme_stp2024_unbusy_bus(stp2024, conn_bus) \
  do { } while (/* CONSTCOND */ 0 && (stp2024)->tme_stp2024_element && (conn_bus)->tme_bus_connection.tme_connection_element)

/* the stp2024 SBus cycle handler: */
static void
_tme_stp2024_cycle_sbus(struct tme_bus_connection *master_conn_bus,
			struct tme_bus_cycle *master_cycle,
			tme_uint32_t *_master_fast_cycle_types,
			struct tme_completion *master_completion)
{
  struct tme_stp2024 *stp2024;
  tme_bus_addr32_t reg;
  struct tme_bus_connection *codec_conn_bus;
  struct tme_bus_connection *codec_conn_bus_other;
#if TME_STP2024_BUS_TRANSITION
  struct tme_bus_tlb tlb_local;
  int rc;
#endif /* TME_STP2024_BUS_TRANSITION */
  tme_uint32_t dma_csr;
  int pipeline;
  tme_uint32_t value32;
  tme_uint8_t value8;

  /* enter: */
  stp2024 = _tme_stp2024_enter_bus(master_conn_bus);

  /* get the register: */
  reg = master_cycle->tme_bus_cycle_address;

  /* if this is an eight-bit cycle to the codec: */
  if (master_cycle->tme_bus_cycle_size == sizeof(tme_uint8_t)
      && (reg >= TME_STP2024_OFFSET_CODEC_REG0
	  && reg <= TME_STP2024_OFFSET_CODEC_REG3)) {

    /* convert the cycle address for the codec: */
    master_cycle->tme_bus_cycle_address
      = ((reg - TME_STP2024_OFFSET_CODEC_REG0)
	 / sizeof(tme_uint32_t));

    /* busy the codec connection: */
    codec_conn_bus = _tme_stp2024_busy_bus(stp2024, TME_STP2024_CONN_CODEC);
    codec_conn_bus_other = (struct tme_bus_connection *) codec_conn_bus->tme_bus_connection.tme_connection_other;

    /* leave: */
    _tme_stp2024_leave(stp2024);

#if TME_STP2024_BUS_TRANSITION

    /* fill a TLB entry for this address: */
    tlb_local.tme_bus_tlb_token = &stp2024->tme_stp2024_codec_cycle_tlb_token;
    rc = ((*codec_conn_bus_other->tme_bus_tlb_fill)
	  (codec_conn_bus_other,
	   &tlb_local,
	   master_cycle->tme_bus_cycle_address,
	   master_cycle->tme_bus_cycle_type));
    assert (rc == TME_OK);

#endif /* TME_STP2024_BUS_TRANSITION */

    /* pass this cycle to the codec: */
#if TME_STP2024_BUS_TRANSITION
    rc = ((*tlb_local.tme_bus_tlb_cycle)
	  (tlb_local.tme_bus_tlb_cycle_private,
	   master_cycle));
    master_completion->tme_completion_error = rc;
    tme_completion_validate(master_completion);
#else  /* !TME_STP2024_BUS_TRANSITION */
#error WRITEME
#endif /* !TME_STP2024_BUS_TRANSITION */

    /* reenter: */
    _tme_stp2024_enter(stp2024);
  }

  /* otherwise, this is not a codec cycle: */
  else {

    /* if this is an aligned 32-bit access: */
    if (master_cycle->tme_bus_cycle_size == sizeof(tme_uint32_t)
	&& (reg % sizeof(tme_uint32_t)) == 0) {

      /* make the DMA CSR value: */
      dma_csr
	= ((stp2024->tme_stp2024_dma_csr
	    & (TME_STP2024_DMA_XCSR(0)
	       | TME_STP2024_DMA_XCSR(1)))
	   | (stp2024->tme_stp2024_dma_xcsr[0]
	      & TME_STP2024_DMA_XCSR(0))
	   | (stp2024->tme_stp2024_dma_xcsr[1]
	      & TME_STP2024_DMA_XCSR(1)));

      /* assume that this is a pipeline-specific register: */
      pipeline = (reg >= TME_STP2024_OFFSET_DMA_PVA);

      /* if this is a write: */
      if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

	/* run the bus cycle: */
	tme_bus_cycle_xfer_reg(master_cycle, 
			       &value32,
			       TME_BUS32_LOG2);

	/* dispatch on the register: */
	switch (reg) {
	default: abort();

	case TME_STP2024_OFFSET_DMA_CSR:
	  dma_csr 
	    = ((dma_csr
		& (TME_STP2024_DMA_CSR_ID
		   | TME_STP2024_DMA_CSR_IP
		   | TME_STP2024_DMA_CSR_PM
		   | TME_STP2024_DMA_CSR_PD
		   | TME_STP2024_DMA_CSR_CM
		   | TME_STP2024_DMA_CSR_CD))
	       | ((dma_csr
		   & ~value32)
		  & (TME_STP2024_DMA_CSR_PI
		     | TME_STP2024_DMA_CSR_CI
		     | TME_STP2024_DMA_CSR_EI
		     | TME_STP2024_DMA_CSR_PMI
		     | TME_STP2024_DMA_CSR_CMI))
	       | (value32
		  & (TME_STP2024_DMA_CSR_IE
		     | TME_STP2024_DMA_CSR_PIE
		     | TME_STP2024_DMA_CSR_CIE
		     | TME_STP2024_DMA_CSR_EIE
		     | TME_STP2024_DMA_CSR_PMIE
		     | TME_STP2024_DMA_CSR_CMIE
		     | TME_STP2024_DMA_CSR_PPAUSE
		     | TME_STP2024_DMA_CSR_CPAUSE
		     | TME_STP2024_DMA_CSR_PDN
		     | TME_STP2024_DMA_CSR_LOOPBACK
		     | TME_STP2024_DMA_CSR_PGO
		     | TME_STP2024_DMA_CSR_CGO
		     | TME_STP2024_DMA_CSR_RESET)));
	  stp2024->tme_stp2024_dma_csr = dma_csr;
	  stp2024->tme_stp2024_dma_xcsr[0] = (dma_csr & TME_STP2024_DMA_XCSR(0));
	  stp2024->tme_stp2024_dma_xcsr[1] = (dma_csr & TME_STP2024_DMA_XCSR(1));
	  break;

	case TME_STP2024_OFFSET_DMA_CVA:
	case TME_STP2024_OFFSET_DMA_PVA:
	  stp2024->tme_stp2024_dma_xva[pipeline] = value32;
	  break;

	case TME_STP2024_OFFSET_DMA_CC:
	case TME_STP2024_OFFSET_DMA_PC:
	  stp2024->tme_stp2024_dma_xc[pipeline] = value32;
	  break;

	case TME_STP2024_OFFSET_DMA_CNVA:
	case TME_STP2024_OFFSET_DMA_PNVA:
	  stp2024->tme_stp2024_dma_xnva[pipeline] = value32;
	  break;

	case TME_STP2024_OFFSET_DMA_CNC:
	case TME_STP2024_OFFSET_DMA_PNC:
	  stp2024->tme_stp2024_dma_xnc[pipeline] = value32;
	  stp2024->tme_stp2024_dma_xcsr[pipeline]
	    &= ~(TME_STP2024_DMA_CSR_PD
		 | TME_STP2024_DMA_CSR_CD);
	  break;
	}
      }

      /* otherwise, this must be a read: */
      else {
	assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

	/* dispatch on the register: */
	switch (reg) {
	default: abort();

	case TME_STP2024_OFFSET_DMA_CSR:
	  value32 = dma_csr;
	  break;

	case TME_STP2024_OFFSET_DMA_CVA:
	case TME_STP2024_OFFSET_DMA_PVA:
	  value32 = stp2024->tme_stp2024_dma_xva[pipeline];
	  break;

	case TME_STP2024_OFFSET_DMA_CC:
	case TME_STP2024_OFFSET_DMA_PC:
	  value32 = stp2024->tme_stp2024_dma_xc[pipeline];
	  break;

	case TME_STP2024_OFFSET_DMA_CNVA:
	case TME_STP2024_OFFSET_DMA_PNVA:
	  value32 = stp2024->tme_stp2024_dma_xnva[pipeline];
	  break;

	case TME_STP2024_OFFSET_DMA_CNC:
	case TME_STP2024_OFFSET_DMA_PNC:
	  value32 = stp2024->tme_stp2024_dma_xnc[pipeline];
	  break;
	}

	/* run the bus cycle: */
	tme_bus_cycle_xfer_reg(master_cycle, 
			       &value32,
			       TME_BUS32_LOG2);
      }
    }

    /* otherwise, if this is an eight-bit access: */
    else if (master_cycle->tme_bus_cycle_size == sizeof(tme_uint8_t)) {

      /* if this is a write: */
      if (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

	/* XXX WRITEME */
	abort();
      }

      /* otherwise, this must be a read: */
      else {
	assert (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

	/* dispatch on the register: */
	switch (reg) {
	default: abort();

	case TME_STP2024_OFFSET_POWER_FANCTL:
	  value8 = stp2024->tme_stp2024_power_fanctl;
	  break;
	}
      
	/* run the bus cycle: */
	tme_bus_cycle_xfer_reg(master_cycle, 
			       &value8,
			       TME_BUS8_LOG2);
      }
    }

    /* complete the cycle: */
    master_completion->tme_completion_error = TME_OK;
    tme_memory_barrier(master_completion, sizeof(*master_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_completion_validate(master_completion);
    *_master_fast_cycle_types = 0;
  }

  /* leave: */
  _tme_stp2024_leave(stp2024);
}

/* the stp2024 SBus TLB filler: */
static void
_tme_stp2024_tlb_fill_sbus(struct tme_bus_connection *master_conn_bus,
			   struct tme_bus_tlb *tlb, 
			   tme_bus_addr_t address_wider,
			   unsigned int cycles)
{
  struct tme_stp2024 *stp2024;
  tme_bus_addr32_t address;
  const struct tme_bus_subregion *subregion;

  /* recover our data structures: */
  stp2024 = master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;

  /* get our address: */
  address = address_wider;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* find the subregion: */
  subregion = &_tme_stp2024_subregion_first;
  do {
    if (address >= subregion->tme_bus_subregion_address_first
	&& address <= subregion->tme_bus_subregion_address_last) {
      break;
    }
    subregion = subregion->tme_bus_subregion_next;
  } while (subregion != NULL);
  assert (subregion != NULL);

  /* this TLB entry covers only this subregion: */
  tlb->tme_bus_tlb_addr_first = subregion->tme_bus_subregion_address_first;
  tlb->tme_bus_tlb_addr_last = subregion->tme_bus_subregion_address_last;
}

/* the stp2024 codec cycle handler: */
static void
_tme_stp2024_cycle_codec(struct tme_bus_connection *codec_conn_bus,
			 struct tme_bus_cycle *codec_cycle,
			 tme_uint32_t *_codec_fast_cycle_types,
			 struct tme_completion *codec_completion)
{
  struct tme_stp2024 *stp2024;
  int pipeline;

  /* enter: */
  stp2024 = _tme_stp2024_enter_bus(codec_conn_bus);

  /* advance the capture or playback pipeline: */
  pipeline
    = _tme_stp2024_dma_advance(stp2024,
			       codec_cycle->tme_bus_cycle_address,
			       (codec_cycle->tme_bus_cycle_size > 0
				? (TME_STP2024_DMA_CSR_PM
				   | TME_STP2024_DMA_CSR_CM)
				: 0));

  /* if this is an empty bus cycle: */
  if (codec_cycle->tme_bus_cycle_size == 0) {

    /* complete with success: */
    codec_completion->tme_completion_error = TME_OK;
    tme_memory_barrier(codec_completion, sizeof(*codec_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_completion_validate(codec_completion);
  }

  /* otherwise, this bus cycle is not empty: */

  /* if the pipeline is empty or paused: */
  else if (stp2024->tme_stp2024_dma_xcsr[pipeline]
	   & (TME_STP2024_DMA_CSR_PM
	      | TME_STP2024_DMA_CSR_CM
	      | TME_STP2024_DMA_CSR_PPAUSE
	      | TME_STP2024_DMA_CSR_CPAUSE)) {

    /* complete with EAGAIN: */
    codec_completion->tme_completion_error = EAGAIN;
    tme_memory_barrier(codec_completion, sizeof(*codec_completion), TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
    tme_completion_validate(codec_completion);
    *_codec_fast_cycle_types = 0;
  }

  /* otherwise, the pipeline is not empty: */
  else {

    /* convert the cycle address into an SBus address: */
    codec_cycle->tme_bus_cycle_address = stp2024->tme_stp2024_dma_xva[pipeline];

    /* XXX WRITEME: */
    abort();
  }
    
  /* leave: */
  _tme_stp2024_leave(stp2024);
}

/* the stp2024 TLB filler for the codec: */
static void
_tme_stp2024_tlb_fill_codec(struct tme_bus_connection *codec_conn_bus,
			    struct tme_bus_tlb *tlb, 
			    tme_bus_addr_t codec_xaddress,
			    unsigned int cycles)
{
  struct tme_stp2024 *stp2024;
  struct tme_bus_tlb tlb_mapping;
  int pipeline;
  tme_uint32_t lookahead;
  tme_uint32_t count;
  tme_bus_addr_t slave_address;
  struct tme_bus_connection *slave_conn_bus;
  struct tme_bus_connection *slave_conn_bus_other;
#if TME_STP2024_BUS_TRANSITION
  int rc;
#endif /* TME_STP2024_BUS_TRANSITION */

  /* enter: */
  stp2024 = _tme_stp2024_enter_bus(codec_conn_bus);

  /* save the raw codec address in the mapping structure: */
  tlb_mapping.tme_bus_tlb_addr_first = codec_xaddress;

  /* get the pipeline: */
  pipeline = (codec_xaddress > TME_STP2024_CODEC_ADDRESS_MASK);
  codec_xaddress &= TME_STP2024_CODEC_ADDRESS_MASK;

  /* lock our mutex: */
  tme_mutex_lock(&stp2024->tme_stp2024_mutex);

  /* see how much the codec wants to look ahead in the pipeline: */
  lookahead
    = (((tme_uint32_t) 
	(codec_xaddress
	 - stp2024->tme_stp2024_dma_codec_xaddress[pipeline]))
       & (tme_uint32_t) TME_STP2024_CODEC_ADDRESS_MASK);

  /* if the pipeline is not empty and not paused: */
  if ((stp2024->tme_stp2024_dma_xcsr[pipeline]
       & (TME_STP2024_DMA_CSR_PM
	  | TME_STP2024_DMA_CSR_CM
	  | TME_STP2024_DMA_CSR_PPAUSE
	  | TME_STP2024_DMA_CSR_CPAUSE)) == 0) {

    /* assume that the codec wants to look ahead in the first buffer: */
    count = stp2024->tme_stp2024_dma_xc[pipeline];
    slave_address = stp2024->tme_stp2024_dma_xva[pipeline];

    /* if the codec wants to look ahead past the first buffer: */
    if (lookahead >= count) {

      /* assume that the codec wants to look ahead in the second buffer: */
      lookahead -= count;
      count = stp2024->tme_stp2024_dma_xnc[pipeline];
      slave_address = stp2024->tme_stp2024_dma_xnva[pipeline];

      /* if the second buffer is dirty, or if the codec wants to look
	 ahead past the second buffer: */
      if ((stp2024->tme_stp2024_dma_xcsr[pipeline]
	   & (TME_STP2024_DMA_CSR_PD
	      | TME_STP2024_DMA_CSR_CD))
	  || lookahead >= count) {

	/* we can't let the codec look ahead: */
	count = 0;
      }
    }

    /* if we can let the codec look ahead: */
    if (count > 0) {

      /* if we have an SBus connection: */
      slave_conn_bus = _tme_stp2024_busy_bus(stp2024, TME_STP2024_CONN_SBUS);
      if (slave_conn_bus != NULL) {

	/* leave: */
	_tme_stp2024_leave(stp2024);

	/* advance in the buffer to the lookahead point: */
	slave_address += lookahead;
	count -= lookahead;

	/* fill this TLB entry: */
	slave_conn_bus_other = (struct tme_bus_connection *) slave_conn_bus->tme_bus_connection.tme_connection_other;
#if TME_STP2024_BUS_TRANSITION
	rc = 
#endif /* TME_STP2024_BUS_TRANSITION */
	(*slave_conn_bus_other->tme_bus_tlb_fill)
	  (slave_conn_bus_other,
	   tlb,
	   slave_address,
	   cycles);
#if TME_STP2024_BUS_TRANSITION
	assert (rc == TME_OK);
#endif /* TME_STP2024_BUS_TRANSITION */

	/* reenter: */
	_tme_stp2024_enter(stp2024);

	/* unbusy the SBus connection: */
	_tme_stp2024_unbusy_bus(stp2024, slave_conn_bus);
      }

      /* map the filled TLB entry: */
      tlb_mapping.tme_bus_tlb_addr_last
	= (tlb_mapping.tme_bus_tlb_addr_first
	   + (count - 1));
#if TME_STP2024_BUS_TRANSITION
      tlb_mapping.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
#endif /* TME_STP2024_BUS_TRANSITION */
      tme_bus_tlb_map(tlb, slave_address, &tlb_mapping, tlb_mapping.tme_bus_tlb_addr_first);

      /* leave: */
      _tme_stp2024_leave(stp2024);

      return;
    }
  }

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers only the raw codec address: */
  tlb->tme_bus_tlb_addr_first = tlb_mapping.tme_bus_tlb_addr_first;
  tlb->tme_bus_tlb_addr_last = tlb_mapping.tme_bus_tlb_addr_first;

  /* leave: */
  _tme_stp2024_leave(stp2024);
}

/* the stp2024 TLB adder for the codec DMA: */
static void
_tme_stp2024_tlb_set_add(struct tme_bus_connection *codec_conn_bus,
			 struct tme_bus_tlb_set_info *tlb_set_info,
			 struct tme_completion *codec_completion)
{

  /* if this TLB set provides a bus context register: */
  if (tlb_set_info->tme_bus_tlb_set_info_bus_context != NULL) {

    /* this bus only has one context: */
    *tlb_set_info->tme_bus_tlb_set_info_bus_context = 0;
    tlb_set_info->tme_bus_tlb_set_info_bus_context_max = 0;
  }

  /* XXX WRITEME - this should call through to the SBus: */
  codec_completion->tme_completion_error = TME_OK;
  tme_memory_barrier(0, 0, TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE);
  tme_completion_validate(codec_completion);

  /* unused: */
  codec_conn_bus = 0;
}

#if TME_STP2024_BUS_TRANSITION

/* this is the bus cycle transition glue: */
static int
_tme_stp2024_cycle_transition(void *_master_conn_bus,
			      struct tme_bus_cycle *master_cycle)
{
  struct tme_completion completion_buffer;
  struct tme_stp2024 *stp2024;
  struct tme_bus_connection *master_conn_bus;
  tme_uint32_t master_fast_cycle_types;

  tme_completion_init(&completion_buffer);

  master_conn_bus = (struct tme_bus_connection *) _master_conn_bus;
  stp2024 = (struct tme_stp2024 *) master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  (master_conn_bus == stp2024->tme_stp2024_conns[TME_STP2024_CONN_SBUS]
   ? _tme_stp2024_cycle_sbus
   : _tme_stp2024_cycle_codec)
    (master_conn_bus,
     master_cycle,
     &master_fast_cycle_types,
     &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

/* this is the TLB fill transition glue: */
static int
_tme_stp2024_tlb_fill_transition(struct tme_bus_connection *agent_conn_bus,
				 struct tme_bus_tlb *tlb,
				 tme_bus_addr_t address_wider,
				 unsigned int cycle_type)
{
  struct tme_stp2024 *stp2024;

  stp2024 = (struct tme_stp2024 *) agent_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  (agent_conn_bus == stp2024->tme_stp2024_conns[TME_STP2024_CONN_SBUS]
   ? _tme_stp2024_tlb_fill_sbus
   : _tme_stp2024_tlb_fill_codec)
    (agent_conn_bus,
     tlb,
     address_wider,
     cycle_type);

  /* we always handle any slow cycles: */
  tlb->tme_bus_tlb_cycles_ok |= cycle_type;
  tlb->tme_bus_tlb_addr_offset = 0;
  tlb->tme_bus_tlb_addr_shift = 0;
  tlb->tme_bus_tlb_cycle = _tme_stp2024_cycle_transition;
  tlb->tme_bus_tlb_cycle_private = agent_conn_bus;
  assert (tlb->tme_bus_tlb_fault_handler_count == 0);

  return (TME_OK);
}
#define _tme_stp2024_tlb_fill_sbus _tme_stp2024_tlb_fill_transition
#define _tme_stp2024_tlb_fill_codec _tme_stp2024_tlb_fill_transition

/* this is the bus TLB set add transition glue: */
static int
_tme_stp2024_tlb_set_add_transition(struct tme_bus_connection *codec_conn_bus,
				    struct tme_bus_tlb_set_info *tlb_set_info)
{
  struct tme_completion completion_buffer;
  tme_completion_init(&completion_buffer);
  _tme_stp2024_tlb_set_add(codec_conn_bus,
			   tlb_set_info,
			   &completion_buffer);
  return (completion_buffer.tme_completion_error);
}
#define _tme_stp2024_tlb_set_add _tme_stp2024_tlb_set_add_transition

#endif /* TME_STP2024_BUS_TRANSITION */

/* this scores a new connection: */
static int
_tme_stp2024_connection_score(struct tme_connection *conn,
			      unsigned int *_score)
{
  struct tme_stp2024 *stp2024;
  struct tme_stp2024_connection *conn_stp2024;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  stp2024 = conn->tme_connection_element->tme_element_private;
  conn_stp2024 = (struct tme_stp2024_connection *) conn;

  /* this is a generic bus connection, so just score it nonzero and
     return.  note that there's no good way to differentiate a
     connection to a bus from a connection to just another chip, so we
     always return a nonzero score here: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_stp2024_connection_make(struct tme_connection *conn,
			     unsigned int state)
{
  struct tme_stp2024 *stp2024;
  struct tme_stp2024_connection *conn_stp2024;
  struct tme_bus_connection *conn_bus;

  /* both sides must be generic bus connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC);
  assert(conn->tme_connection_other->tme_connection_type
	 == conn->tme_connection_type);

  /* recover our data structures: */
  stp2024 = conn->tme_connection_element->tme_element_private;
  conn_stp2024 = (struct tme_stp2024_connection *) conn;
  conn_bus = &conn_stp2024->tme_stp2024_connection;

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&stp2024->tme_stp2024_mutex);

    /* save our connection: */
    switch (conn_stp2024->tme_stp2024_connection_which) {
    default: assert(FALSE);
    case TME_STP2024_CONN_SBUS:
    case TME_STP2024_CONN_CODEC:
      stp2024->tme_stp2024_conns[conn_stp2024->tme_stp2024_connection_which]
	= (struct tme_bus_connection *) conn;
      break;
    }

    /* unlock our mutex: */
    tme_mutex_unlock(&stp2024->tme_stp2024_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_stp2024_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for an stp2024: */
static int
_tme_stp2024_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  struct tme_stp2024 *stp2024;
  struct tme_stp2024_connection *conn_stp2024;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  unsigned int conn_which;
  int usage;
  int rc;

  /* recover our data structure: */
  stp2024 = (struct tme_stp2024 *) element->tme_element_private;
  
  /* we don't bother locking the mutex simply to check if connections
     already exist: */

  /* check our arguments: */
  usage = FALSE;
  rc = 0;
  conn_which = TME_STP2024_CONN_NULL;

  /* if this is the SBus connection: */
  if (args[1] == NULL) {
    conn_which = TME_STP2024_CONN_SBUS;
  }

  /* otherwise, if this connection is for the codec: */
  else if (TME_ARG_IS(args[1], "codec")) {
    conn_which = TME_STP2024_CONN_CODEC;
  }

  /* otherwise, this is a bad argument: */
  else {
    tme_output_append_error(_output,
			    "%s %s, ",
			    args[1],
			    _("unexpected"));
    usage = TRUE;
  }

  /* if this is for a connection that already exists: */
  if (conn_which != TME_STP2024_CONN_NULL
      && stp2024->tme_stp2024_conns[conn_which] != NULL) {
    rc = EEXIST;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s [ codec ]",
			    _("usage:"),
			    args[0]);
    rc = EINVAL;
  }

  if (rc) {
    return (rc);
  }

  /* make a new connection: */
  conn_stp2024 = tme_new0(struct tme_stp2024_connection, 1);
  conn_bus = &conn_stp2024->tme_stp2024_connection;
  conn = &conn_bus->tme_bus_connection;

  /* fill in the generic connection: */
  conn->tme_connection_next = *_conns;
  conn->tme_connection_type = TME_CONNECTION_BUS_GENERIC;
  conn->tme_connection_score = _tme_stp2024_connection_score;
  conn->tme_connection_make = _tme_stp2024_connection_make;
  conn->tme_connection_break = _tme_stp2024_connection_break;

  /* fill in the generic bus connection: */
  switch (conn_which) {
  default: assert(FALSE);
  case TME_STP2024_CONN_CODEC:
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
    conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = 0 - (tme_bus_addr_t) 1;
    conn_bus->tme_bus_subregions.tme_bus_subregion_next = NULL;
    conn_bus->tme_bus_signals_add = NULL;
    conn_bus->tme_bus_tlb_set_add = _tme_stp2024_tlb_set_add;
    conn_bus->tme_bus_tlb_fill = _tme_stp2024_tlb_fill_codec;
    break;
  case TME_STP2024_CONN_SBUS:
    conn_bus->tme_bus_tlb_fill = _tme_stp2024_tlb_fill_sbus;
    conn_bus->tme_bus_subregions = _tme_stp2024_subregion_first;
    break;
  }

  /* fill in the internal information: */
  conn_stp2024->tme_stp2024_connection_which = conn_which;

  /* return the connection side possibility: */
  *_conns = conn;
  return (TME_OK);
}

/* the new stp2024 function: */
TME_ELEMENT_NEW_DECL(tme_ic_stp2024) {
  int usage;
  int arg_i;
  tme_bus_addr_t power_fanctl;
  struct tme_stp2024 *stp2024;

  /* check our arguments: */
  usage = FALSE;
  arg_i = 1;
  power_fanctl = TME_STP2024_POWER_FANCTL_ID + 1;
  for (;;) {

    if (TME_ARG_IS(args[arg_i], "id")) {
      power_fanctl
	= (tme_bus_addr_parse(args[arg_i + 1], TME_STP2024_POWER_FANCTL_ID + 1)
	   * _TME_FIELD_MASK_FACTOR(TME_STP2024_POWER_FANCTL_ID));
      if (power_fanctl > TME_STP2024_POWER_FANCTL_ID) {
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

  if (power_fanctl > TME_STP2024_POWER_FANCTL_ID) {
    usage = TRUE;
  }      

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s id %s",
			    _("usage:"),
			    args[0],
			    _("ID"));
    return (EINVAL);
  }

  /* start the stp2024 structure: */
  stp2024 = tme_new0(struct tme_stp2024, 1);
  tme_mutex_init(&stp2024->tme_stp2024_mutex);
  stp2024->tme_stp2024_element = element;
  stp2024->tme_stp2024_power_fanctl = power_fanctl;

  /* fill the element: */
  element->tme_element_private = stp2024;
  element->tme_element_connections_new = _tme_stp2024_connections_new;

  return (TME_OK);
}
