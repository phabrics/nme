/* $Id: am7990.c,v 1.4 2010/06/05 14:33:45 fredette Exp $ */

/* ic/am7990.c - implementation of Am7990 emulation: */

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

#include <tme/common.h>
_TME_RCSID("$Id: am7990.c,v 1.4 2010/06/05 14:33:45 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/ethernet.h>

/* macros: */

/* CSR0: */
#define TME_AM7990_CSR0_ERR			TME_BIT(15)
#define TME_AM7990_CSR0_BABL			TME_BIT(14)
#define TME_AM7990_CSR0_CERR			TME_BIT(13)
#define TME_AM7990_CSR0_MISS			TME_BIT(12)
#define TME_AM7990_CSR0_MERR			TME_BIT(11)
#define TME_AM7990_CSR0_RINT			TME_BIT(10)
#define TME_AM7990_CSR0_TINT			TME_BIT(9)
#define TME_AM7990_CSR0_IDON			TME_BIT(8)
#define TME_AM7990_CSR0_INTR			TME_BIT(7)
#define TME_AM7990_CSR0_INEA			TME_BIT(6)
#define TME_AM7990_CSR0_RXON			TME_BIT(5)
#define TME_AM7990_CSR0_TXON			TME_BIT(4)
#define TME_AM7990_CSR0_TDMD			TME_BIT(3)
#define TME_AM7990_CSR0_STOP			TME_BIT(2)
#define TME_AM7990_CSR0_STRT			TME_BIT(1)
#define TME_AM7990_CSR0_INIT			TME_BIT(0)
#define TME_AM7990_CSR0_READ_ONLY	\
  (TME_AM7990_CSR0_ERR			\
   | TME_AM7990_CSR0_INTR		\
   | TME_AM7990_CSR0_RXON		\
   | TME_AM7990_CSR0_TXON)
#define TME_AM7990_CSR0_WRITE_ONE_TO_CLEAR	\
  (TME_AM7990_CSR0_BABL			\
   | TME_AM7990_CSR0_CERR		\
   | TME_AM7990_CSR0_MISS		\
   | TME_AM7990_CSR0_MERR		\
   | TME_AM7990_CSR0_RINT		\
   | TME_AM7990_CSR0_TINT		\
   | TME_AM7990_CSR0_IDON)
#define TME_AM7990_CSR0_WRITE_ONE_ONLY	\
  (TME_AM7990_CSR0_TDMD			\
   | TME_AM7990_CSR0_STOP		\
   | TME_AM7990_CSR0_STRT		\
   | TME_AM7990_CSR0_INIT)

/* CSR3: */
#define TME_AM7990_CSR3_BSWAP			TME_BIT(2)
#define TME_AM7990_CSR3_ACON			TME_BIT(1)
#define TME_AM7990_CSR3_BCON			TME_BIT(0)

/* Mode: */
#define TME_AM7990_MODE_PROM			TME_BIT(15)
#define TME_AM7990_MODE_EMBA			TME_BIT(7)
#define TME_AM7990_MODE_INTL			TME_BIT(6)
#define TME_AM7990_MODE_DRTY			TME_BIT(5)
#define TME_AM7990_MODE_COLL			TME_BIT(4)
#define TME_AM7990_MODE_DTCR			TME_BIT(3)
#define TME_AM7990_MODE_LOOP			TME_BIT(2)
#define TME_AM7990_MODE_DTX			TME_BIT(1)
#define TME_AM7990_MODE_DRX			TME_BIT(0)

/* a Descriptor Ring Pointer: */
#define TME_AM7990_DRP_XDRA			(0x00fffff8)
#define TME_AM7990_DRP_XLEN_LOG2		(0xe0000000)

/* a descriptor table entry: */
#define TME_AM7990_DTE_OFFSET_XMD0		(0 * sizeof(tme_uint16_t))
#define TME_AM7990_DTE_OFFSET_XMD1		(1 * sizeof(tme_uint16_t))
#define TME_AM7990_DTE_OFFSET_XMD2		(2 * sizeof(tme_uint16_t))
#define TME_AM7990_DTE_OFFSET_XMD3		(3 * sizeof(tme_uint16_t))
#define TME_AM7990_DTE_SIZE			(4 * sizeof(tme_uint16_t))

/* common parts of TMD1 and RMD1: */
#define TME_AM7990_XMD1_OWN			TME_BIT(15)
#define TME_AM7990_XMD1_ERR			TME_BIT(14)
#define TME_AM7990_XMD1_STP			TME_BIT(9)
#define TME_AM7990_XMD1_ENP			TME_BIT(8)
#define TME_AM7990_XMD1_HADR			(0x00ff)

/* common parts of TMD2 and RMD2: */
#define TME_AM7990_XMD2_BCNT			(0x0fff)

/* TMD1: */
#define TME_AM7990_TMD1_ADD_FCS			TME_BIT(13)

/* TMD3: */
#define TME_AM7990_TMD3_BUFF			TME_BIT(15)

/* RMD1: */
#define TME_AM7990_RMD1_BUFF			TME_BIT(10)

/* the callout flags: */
#define TME_AM7990_CALLOUTS_RUNNING		TME_BIT(0)
#define TME_AM7990_CALLOUTS_MASK		(-2)
#define  TME_AM7990_CALLOUT_RECEIVE		TME_BIT(1)
#define  TME_AM7990_CALLOUT_TRANSMIT_SCAN	TME_BIT(2)
#define  TME_AM7990_CALLOUT_CONFIG		TME_BIT(3)
#define  TME_AM7990_CALLOUT_DMA_READ		TME_BIT(4)
#define  TME_AM7990_CALLOUT_DMA_WRITE		TME_BIT(5)

/* the default locks: */
#define TME_AM7990_LOCKS_DEFAULT		(0)

/* the size of the TLB entry hash: */
#define TME_AM7990_TLB_HASH_SIZE 		(512)

/* structures: */

/* the chip: */
struct tme_am7990 {

  /* our simple bus device header: */
  struct tme_bus_device tme_am7990_device;
#define tme_am7990_element tme_am7990_device.tme_bus_device_element

  /* the Ethernet connection: */
  struct tme_ethernet_connection *tme_am7990_eth_connection;

  /* the mutex protecting the chip: */
  tme_mutex_t tme_am7990_mutex;

  /* the callout flags: */
  int tme_am7990_callout_flags;

  /* our DMA TLB hash: */
  struct tme_bus_tlb tme_am7990_tlb_hash[TME_AM7990_TLB_HASH_SIZE];
  int tme_am7990_tlb_hash_added;

  /* our bus addresses: */
  tme_bus_addr32_t tme_am7990_offset_rap;
  tme_bus_addr32_t tme_am7990_offset_rdp;

  /* registers: */
  tme_uint16_t tme_am7990_rap;
  tme_uint16_t tme_am7990_csrs[4];
#define tme_am7990_csr0 tme_am7990_csrs[0]

  /* the initialization block: */
  tme_uint16_t tme_am7990_init[12];
#define tme_am7990_mode		tme_am7990_init[0]
#define tme_am7990_padr		tme_am7990_init[1]
#define tme_am7990_ladrf	tme_am7990_init[4]
#define tme_am7990_rdra		tme_am7990_init[8]
#define tme_am7990_rlen_rdra	tme_am7990_init[9]
#define tme_am7990_tdra		tme_am7990_init[10]
#define tme_am7990_tlen_tdra	tme_am7990_init[11]

  /* the transmit ring: */
  unsigned int tme_am7990_transmit_dte_index_mask;
  tme_uint32_t tme_am7990_transmit_dte_address;

  /* the current transmit DTE index, and any previously read TMD1
     value: */
  unsigned int tme_am7990_transmit_dte_index;
  tme_uint16_t tme_am7990_transmit_dte_tmd1;

  /* the receive buffer: */
  tme_uint8_t tme_am7990_receive_buffer[TME_ETHERNET_FRAME_MAX];
  unsigned int tme_am7990_receive_buffer_length;

  /* the receive ring: */
  unsigned int tme_am7990_receive_dte_index_mask;
  tme_uint32_t tme_am7990_receive_dte_address;

  /* the current receive DTE index: */
  unsigned int tme_am7990_receive_dte_index;

  /* this is nonzero if the interrupt is asserted: */
  int tme_am7990_int_asserted;

  /* the input and output Ethernet controls: */
  unsigned int tme_am7990_ether_ctrl_out;
  unsigned int tme_am7990_ether_ctrl_in;
};

/* prototypes: */
static int _tme_am7990_transmit _TME_P((struct tme_ethernet_connection *,
					tme_ethernet_fid_t *,
					struct tme_ethernet_frame_chunk *,
					unsigned int));

/* this resets the am7990: */
static void
_tme_am7990_reset(struct tme_am7990 *am7990)
{

  tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
	  100, TME_OK,
	  (&am7990->tme_am7990_element->tme_element_log_handle,
	   "reset"));

  /* clear all pending callouts: */
  am7990->tme_am7990_callout_flags &= TME_AM7990_CALLOUTS_MASK;

  /* reset CSR0: */
  am7990->tme_am7990_csr0 = TME_AM7990_CSR0_STOP;

  /* reset CSR3: */
  am7990->tme_am7990_csrs[3] = 0;

  /* "EMBA is cleared by activation of the RESET pin or setting the
     STOP bit." */
  am7990->tme_am7990_mode &= ~TME_AM7990_MODE_EMBA;
}

/* this hashes an address into a TLB entry: */
static struct tme_bus_tlb *
_tme_am7990_tlb_hash(void *_am7990,
		     tme_bus_addr_t linear_address,
		     unsigned int cycles)
{
  struct tme_am7990 *am7990;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* return the TLB entry: */
  return (am7990->tme_am7990_tlb_hash
	  + ((((tme_bus_addr32_t) linear_address) >> 10) & (TME_AM7990_TLB_HASH_SIZE - 1)));
}

/* this locks the mutex: */
static void
_tme_am7990_lock(void *_am7990,
		 unsigned int locks)
{
  struct tme_am7990 *am7990;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* lock the mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);
}

/* this unlocks the mutex: */
static void
_tme_am7990_unlock(void *_am7990,
		   unsigned int locks)
{
  struct tme_am7990 *am7990;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* unlock the mutex: */
  tme_mutex_unlock(&am7990->tme_am7990_mutex);
}

/* this does a DMA: */
static int
_tme_am7990_dma(struct tme_am7990 *am7990,
		tme_uint32_t callout_flags,
		tme_uint32_t address,
		unsigned int count,
		tme_uint8_t *buffer)
{
  int rc;

  /* make the callout: */
  rc = ((callout_flags & TME_AM7990_CALLOUT_DMA_READ)
	? tme_bus_device_dma_read_16(&am7990->tme_am7990_device,
				     address,
				     count,
				     buffer,
				     TME_AM7990_LOCKS_DEFAULT)
	: tme_bus_device_dma_write_16(&am7990->tme_am7990_device,
				      address,
				      count,
				      buffer,
				      TME_AM7990_LOCKS_DEFAULT));

  /* if we got a bus error: */
  if (rc != TME_OK) {

    /* set MERR: */
    am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_MERR;
    
    /* we were cancelled: */
    return (TRUE);
  }

  /* we weren't cancelled: */
  return (FALSE);
}

/* this returns the address for a 16-bit DTE read or write: */
static inline tme_uint32_t
_tme_am7990_dte_address(struct tme_am7990 *am7990,
			tme_uint32_t callout_flags,
			tme_uint32_t dte_offset)
{
  tme_uint32_t dte_address;
  tme_uint32_t dte_index;
  tme_uint32_t dte_index_mask;

  /* get the DTE address, index, and index mask: */
  if (callout_flags & TME_AM7990_CALLOUT_RECEIVE) {
    dte_address = am7990->tme_am7990_receive_dte_address;
    dte_index = am7990->tme_am7990_receive_dte_index;
    dte_index_mask = am7990->tme_am7990_receive_dte_index_mask;
  }
  else {
    dte_address = am7990->tme_am7990_transmit_dte_address;
    dte_index = am7990->tme_am7990_transmit_dte_index;
    dte_index_mask = am7990->tme_am7990_transmit_dte_index_mask;
  }

  /* get the DTE address to read or write: */
  dte_address
    += (((dte_index * TME_AM7990_DTE_SIZE)
	 + dte_offset)
	& ((dte_index_mask * TME_AM7990_DTE_SIZE)
	   | (TME_AM7990_DTE_SIZE - 1)));
  assert ((dte_address % sizeof(tme_uint16_t)) == 0);

  return (dte_address);
}

/* this does a 16-bit DTE read: */
static tme_uint16_t
_tme_am7990_read(struct tme_am7990 *am7990,
		 tme_uint32_t callout_flags,
		 tme_uint32_t dte_offset,
		 int *_cancelled)
{
  tme_uint16_t value;

  /* do the DMA read: */
  *_cancelled = 
    _tme_am7990_dma(am7990,
		    callout_flags,
		    _tme_am7990_dte_address(am7990,
					    callout_flags,
					    dte_offset),
		    sizeof(value),
		    (tme_uint8_t *) &value);

  /* possibly byteswap the value read: */
  assert (am7990->tme_am7990_device.tme_bus_device_router != NULL);
  if (am7990->tme_am7990_device.tme_bus_device_router
      != (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
	  ? tme_bus_device_router_16el
	  : tme_bus_device_router_16eb)) {
    value = tme_bswap_u16(value);
  }

  return (value);
}
#define _tme_am7990_tx_read(am7990, dte_offset, _cancelled)		\
  _tme_am7990_read((am7990), TME_AM7990_CALLOUT_DMA_READ, (dte_offset), (_cancelled))
#define _tme_am7990_rx_read(am7990, dte_offset, _cancelled)		\
  _tme_am7990_read((am7990), TME_AM7990_CALLOUT_DMA_READ | TME_AM7990_CALLOUT_RECEIVE, (dte_offset), (_cancelled))

/* this does a 16-bit DTE write: */
static int
_tme_am7990_write(struct tme_am7990 *am7990,
		  tme_uint32_t callout_flags,
		  tme_uint32_t dte_offset,
		  tme_uint16_t value)
{

  /* possibly byteswap the value to write: */
  assert (am7990->tme_am7990_device.tme_bus_device_router != NULL);
  if (am7990->tme_am7990_device.tme_bus_device_router
      != (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
	  ? tme_bus_device_router_16el
	  : tme_bus_device_router_16eb)) {
    value = tme_bswap_u16(value);
  }

  /* do the DMA write: */
  return (_tme_am7990_dma(am7990,
			  callout_flags,
			  _tme_am7990_dte_address(am7990,
						  callout_flags,
						  dte_offset),
			  sizeof(value),
			  (tme_uint8_t *) &value));
}
#define _tme_am7990_tx_write(am7990, dte_offset, value)		\
  _tme_am7990_write((am7990), TME_AM7990_CALLOUT_DMA_WRITE, (dte_offset), (value))
#define _tme_am7990_rx_write(am7990, dte_offset, value)		\
  _tme_am7990_write((am7990), TME_AM7990_CALLOUT_DMA_WRITE | TME_AM7990_CALLOUT_RECEIVE, (dte_offset), (value))

/* this initializes the am7990: */
static void
_tme_am7990_init(struct tme_am7990 *am7990)
{
  tme_uint32_t iadr;
  int cancelled;
  unsigned int init_i;
  tme_uint16_t csr0;
  tme_uint32_t drp;
  
  /* DMA in the initialization block: */
  iadr = am7990->tme_am7990_csrs[2];
  iadr = (iadr << 16) + am7990->tme_am7990_csrs[1];
  cancelled
    = _tme_am7990_dma(am7990,
		      TME_AM7990_CALLOUT_DMA_READ,
		      iadr,
		      sizeof(am7990->tme_am7990_init),
		      (tme_uint8_t *) am7990->tme_am7990_init);
  if (cancelled) {
    return;
  }

  /* possibly byteswap the 16-bit words of the initialization block: */
  assert (am7990->tme_am7990_device.tme_bus_device_router != NULL);
  if (am7990->tme_am7990_device.tme_bus_device_router
      != (TME_ENDIAN_NATIVE == TME_ENDIAN_LITTLE
	  ? tme_bus_device_router_16el
	  : tme_bus_device_router_16eb)) {
    for (init_i = 0; init_i < TME_ARRAY_ELS(am7990->tme_am7990_init); init_i++) {
      am7990->tme_am7990_init[init_i] = tme_bswap_u16(am7990->tme_am7990_init[init_i]);
    }
  }

  /* the least significant byte of PADR is actually the "first" byte
     of the Ethernet address as is is normally seen, but otherwise the
     16-bit words of PADR are in the correct order.  so we only need
     to swap those words into little-endian order: */
  (&am7990->tme_am7990_padr)[0] = tme_htole_u16((&am7990->tme_am7990_padr)[0]);
  (&am7990->tme_am7990_padr)[1] = tme_htole_u16((&am7990->tme_am7990_padr)[1]);
  (&am7990->tme_am7990_padr)[2] = tme_htole_u16((&am7990->tme_am7990_padr)[2]);

  tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
	  100, TME_OK,
	  (&am7990->tme_am7990_element->tme_element_log_handle,
	   "init CSR0 0x%04x IADR 0x%08x MODE 0x%04x",
	   (unsigned int) am7990->tme_am7990_csr0,
	   iadr,
	   (unsigned int) am7990->tme_am7990_mode));

  /* get CSR0: */
  csr0 = am7990->tme_am7990_csr0;

  /* set IDON: */
  csr0 |= TME_AM7990_CSR0_IDON;

  /* "RXON is cleared when IDON is set from setting the INIT bit and
     DRX = 1 in the MODE register" */
  if (am7990->tme_am7990_mode & TME_AM7990_MODE_DRX) {
    csr0 &= ~TME_AM7990_CSR0_RXON;
  }

  /* "TXON is cleared when IDON is set and DTX = 1 in the MODE
     register" */
  if (am7990->tme_am7990_mode & TME_AM7990_MODE_DTX) {
    csr0 &= ~TME_AM7990_CSR0_TXON;
  }

  /* clear STOP: */
  csr0 &= ~TME_AM7990_CSR0_STOP;

  /* update CSR0: */
  am7990->tme_am7990_csr0 = csr0;

  /* call out an Ethernet configuration update: */
  am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_CONFIG;

  /* if we're not in internal loopback mode, and the Ethernet
     connection is readable, call out an Ethernet receive: */
  if (!(am7990->tme_am7990_mode & TME_AM7990_MODE_INTL)
      && (am7990->tme_am7990_ether_ctrl_in & TME_ETHERNET_CTRL_OK_READ)) {
    am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_RECEIVE;
  }

  /* initialize for the receive ring: */
  drp = am7990->tme_am7990_rlen_rdra;
  drp = (drp << 16) | am7990->tme_am7990_rdra;
  am7990->tme_am7990_receive_dte_address = drp & TME_AM7990_DRP_XDRA;
  am7990->tme_am7990_receive_dte_index_mask = (1 << TME_FIELD_MASK_EXTRACTU(drp, TME_AM7990_DRP_XLEN_LOG2)) - 1;
  am7990->tme_am7990_receive_dte_index = 0;

  /* initialize for the transmit ring: */
  drp = am7990->tme_am7990_tlen_tdra;
  drp = (drp << 16) | am7990->tme_am7990_tdra;
  am7990->tme_am7990_transmit_dte_address = drp & TME_AM7990_DRP_XDRA;
  am7990->tme_am7990_transmit_dte_index_mask = (1 << TME_FIELD_MASK_EXTRACTU(drp, TME_AM7990_DRP_XLEN_LOG2)) - 1;
  am7990->tme_am7990_transmit_dte_index = 0;
  am7990->tme_am7990_transmit_dte_tmd1 = 0;
}

/* this starts the am7990: */
static void
_tme_am7990_start(struct tme_am7990 *am7990)
{
  tme_uint16_t csr0;

  tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
	  100, TME_OK,
	  (&am7990->tme_am7990_element->tme_element_log_handle,
	   "start CSR0 0x%04x MODE 0x%04x",
	   (unsigned int) am7990->tme_am7990_csr0,
	   (unsigned int) am7990->tme_am7990_mode));

  /* get CSR0: */
  csr0 = am7990->tme_am7990_csr0;

  /* "RXON is set when STRT is set if DRX = 0 in the MODE register" */
  if (!(am7990->tme_am7990_mode & TME_AM7990_MODE_DRX)) {
    csr0 |= TME_AM7990_CSR0_RXON;
  }

  /* "TXON is set when STRT is set if DTX = 0 in the MODE register" */
  if (!(am7990->tme_am7990_mode & TME_AM7990_MODE_DTX)) {
    csr0 |= TME_AM7990_CSR0_TXON;
  }

  /* clear STOP: */
  csr0 &= ~TME_AM7990_CSR0_STOP;

  /* update CSR0: */
  am7990->tme_am7990_csr0 = csr0;
}  

/* this scans for a start-of-packet transmit buffer owned by us: */
static void
_tme_am7990_transmit_scan(struct tme_am7990 *am7990)
{
  tme_uint16_t transmit_dte_tmd1;
  int cancelled;

  /* if the transmitter is not on, return now: */
  if (!(am7990->tme_am7990_csr0 & TME_AM7990_CSR0_TXON)) {
    return;
  }

  /* clear TDMD: */
  am7990->tme_am7990_csr0 &= ~TME_AM7990_CSR0_TDMD;

  /* loop forever: */
  for (;;) {

    /* if we own the current transmit buffer: */
    transmit_dte_tmd1 = am7990->tme_am7990_transmit_dte_tmd1;
    if (transmit_dte_tmd1 & TME_AM7990_XMD1_OWN) {

      /* if the current transmit buffer is also for the start of a
	 packet, return now: */
      if (transmit_dte_tmd1 & TME_AM7990_XMD1_STP) {
	return;
      }

      /* write TMD1 to clear OWN: */
      cancelled
	= _tme_am7990_tx_write(am7990,
			       TME_AM7990_DTE_OFFSET_XMD1,
			       (transmit_dte_tmd1
				& ~TME_AM7990_XMD1_OWN));
      if (cancelled) {
	return;
      }

      /* advance to the next transmit buffer and request a transmit
	 interrupt: */
      am7990->tme_am7990_transmit_dte_index
	= ((am7990->tme_am7990_transmit_dte_index
	    + 1)
	   & am7990->tme_am7990_transmit_dte_index_mask);
      am7990->tme_am7990_transmit_dte_tmd1 = 0;
      am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_TINT;
    }

    /* otherwise, we must not own the current transmit buffer: */
    else {

      /* read TMD1: */
      transmit_dte_tmd1
	= _tme_am7990_tx_read(am7990,
			      TME_AM7990_DTE_OFFSET_XMD1,
			      &cancelled);
      if (cancelled) {
	return;
      }
      am7990->tme_am7990_transmit_dte_tmd1 = transmit_dte_tmd1;

      /* if we still don't own the current transmit buffer, return now: */
      if (!(transmit_dte_tmd1 & TME_AM7990_XMD1_OWN)) {
	return;
      }
    }
  }
  /* NOTREACHED */
}

/* this receives a frame: */
static void
_tme_am7990_receive(struct tme_am7990 *am7990)
{
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  struct tme_ethernet_connection *conn_eth;
  tme_ethernet_fid_t frame_id;
  int resid;
  tme_uint8_t *frame_chunk_bytes;
  unsigned int frame_chunk_bytes_count;
  struct tme_ethernet_header *ether_header;
  tme_uint32_t crc32;
  tme_uint16_t receive_dte_rmd0;
  tme_uint16_t receive_dte_rmd1;
  tme_uint16_t receive_dte_rmd2;
  tme_uint16_t receive_dte_rmd1_next;
  tme_uint32_t receive_buffer_address;
  tme_uint16_t receive_buffer_count;
  int cancelled;
  int stop;

  /* assume that there is no loopback packet: */
  resid = 0;

  /* if we're in loopback mode: */
  if (am7990->tme_am7990_mode & TME_AM7990_MODE_LOOP) {

    /* get and clear any loopback packet length: */
    resid = am7990->tme_am7990_receive_buffer_length;
    am7990->tme_am7990_receive_buffer_length = 0;

    /* "The C-LANCE will not receive any packets externally when it is
       in internal loopback mode." */
    if (resid == 0
	&& (am7990->tme_am7990_mode & TME_AM7990_MODE_INTL)) {
      return;
    }
  }

  /* if there isn't any loopback packet, and the Ethernet connection
     is not readable, return now: */
  if (resid == 0
      && !(am7990->tme_am7990_ether_ctrl_in & TME_ETHERNET_CTRL_OK_READ)) {
    return;
  }

  /* receive the packet into the internal buffer: */
  frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
  frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count = sizeof(am7990->tme_am7990_receive_buffer);
  frame_chunk_buffer.tme_ethernet_frame_chunk_bytes = &am7990->tme_am7990_receive_buffer[0];

  /* if there isn't any loopback packet: */
  if (resid == 0) {

    /* assume that the Ethernet read will fail: */
    am7990->tme_am7990_ether_ctrl_in &= ~TME_ETHERNET_CTRL_OK_READ;

    /* get the Ethernet connection: */
    conn_eth = am7990->tme_am7990_eth_connection;

    /* unlock the mutex: */
    tme_mutex_unlock(&am7990->tme_am7990_mutex);
  
    /* do the callout: */
    resid = (conn_eth == NULL
	     ? 0
	     : ((*conn_eth->tme_ethernet_connection_read)
		(conn_eth,
		 &frame_id,
		 &frame_chunk_buffer,
		 TME_ETHERNET_READ_NEXT)));
    
    /* lock the mutex: */
    tme_mutex_lock(&am7990->tme_am7990_mutex);

    /* if the read failed, return now: */
    if (resid <= 0) {
      return;
    }

    /* assume that the Ethernet connection is still readable: */
    am7990->tme_am7990_ether_ctrl_in |= TME_ETHERNET_CTRL_OK_READ;

    /* append four dummy CRC bytes: */
    resid = TME_MIN((unsigned int) resid + TME_ETHERNET_CRC_SIZE,
		    sizeof(am7990->tme_am7990_receive_buffer));
  }

  /* assume that we should callout another receive after this one: */
  am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_RECEIVE;

  /* if the receiver is not on, return now: */
  if (!(am7990->tme_am7990_csr0 & TME_AM7990_CSR0_RXON)) {
    return;
  }

  /* if this packet is not addressed to our physical address: */
  ether_header = (struct tme_ethernet_header *) &am7990->tme_am7990_receive_buffer[0];
  if (memcmp(&ether_header->tme_ethernet_header_dst[0],
	     &am7990->tme_am7990_padr,
	     TME_ETHERNET_ADDR_SIZE) != 0) {

    /* if this is not a multicast packet, return now: */
    if (!(ether_header->tme_ethernet_header_dst[0] & 0x1)) {
      return;
    }

    /* if this is not a broadcast packet: */
    if (memcmp(&ether_header->tme_ethernet_header_dst[0],
	       &tme_ethernet_addr_broadcast[0],
	       TME_ETHERNET_ADDR_SIZE) != 0) {

      /* calculate the CRC of the destination address: */
      crc32 = tme_ethernet_crc32_el(&ether_header->tme_ethernet_header_dst[0],
				    TME_ETHERNET_ADDR_SIZE);
    
      /* the am7990 only uses the most significant six bits of the CRC: */
      crc32 >>= (32 - 6);

      /* if the bit in the logical address filter is clear, return now: */
      if (((&am7990->tme_am7990_ladrf)[crc32 / (8 * sizeof(am7990->tme_am7990_ladrf))]
	   & TME_BIT(crc32 % (8 * sizeof(am7990->tme_am7990_ladrf)))) == 0) {
	return;
      }
    }
  }

  /* read RMD1: */
  receive_dte_rmd1
    = _tme_am7990_rx_read(am7990,
			  TME_AM7990_DTE_OFFSET_XMD1,
			  &cancelled);
  if (cancelled) {
    return;
  }

  /* if we don't own this receive buffer: */
  if (!(receive_dte_rmd1 & TME_AM7990_XMD1_OWN)) {

    if ((am7990->tme_am7990_csr0 & TME_AM7990_CSR0_MISS) == 0) {
      tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
	      500, TME_OK,
	      (&am7990->tme_am7990_element->tme_element_log_handle,
	       "receive MISS"));
    }

    /* set MISS in CSR0 and return now: */
    am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_MISS;
    return;
  }

  /* set STP in RMD1: */
  receive_dte_rmd1 |= TME_AM7990_XMD1_STP;

  /* while we have bytes to write: */
  frame_chunk_bytes = &am7990->tme_am7990_receive_buffer[0];
  frame_chunk_bytes_count = resid;
  do {

    /* read RMD0 and RMD2: */
    receive_dte_rmd0
      = _tme_am7990_rx_read(am7990,
			    TME_AM7990_DTE_OFFSET_XMD0,
			    &cancelled);
    if (cancelled) {
      break;
    }
    receive_dte_rmd2
      = _tme_am7990_rx_read(am7990,
			    TME_AM7990_DTE_OFFSET_XMD2,
			    &cancelled);
    if (cancelled) {
      break;
    }

    /* get the receive buffer address and count: */
    receive_buffer_address = (receive_dte_rmd1 & TME_AM7990_XMD1_HADR);
    receive_buffer_address = (receive_buffer_address << 16) + receive_dte_rmd0;
    receive_buffer_count = (0 - receive_dte_rmd2) & TME_AM7990_XMD2_BCNT;

    /* get the count of bytes to write in this iteration: */
    receive_buffer_count = TME_MIN(frame_chunk_bytes_count, receive_buffer_count);

    /* do the write: */
    cancelled 
      = _tme_am7990_dma(am7990,
			(TME_AM7990_CALLOUT_DMA_WRITE
			 | TME_AM7990_CALLOUT_RECEIVE),
			receive_buffer_address,
			receive_buffer_count,
			frame_chunk_bytes);
    if (cancelled) {
      break;
    }

    /* advance: */
    frame_chunk_bytes += receive_buffer_count;
    frame_chunk_bytes_count -= receive_buffer_count;

    /* if this is the last buffer for this packet: */
    if (frame_chunk_bytes_count == 0) {

      /* write RMD3 to set MCNT: */
      cancelled
	= _tme_am7990_rx_write(am7990,
			       TME_AM7990_DTE_OFFSET_XMD3,
			       (unsigned int) resid);
      if (cancelled) {
	break;
      }

      /* set ENP in RMD1: */
      receive_dte_rmd1 |= TME_AM7990_XMD1_ENP;

      /* we don't read RMD1 for the next receive buffer: */
      receive_dte_rmd1_next = 0;

      /* stop now: */
      stop = TRUE;
    }

    /* otherwise, this is not the last buffer for this packet: */
    else {

      /* read RMD1 for the next transmit buffer: */
      receive_dte_rmd1_next
	= _tme_am7990_rx_read(am7990,
			      (TME_AM7990_DTE_SIZE
			       + TME_AM7990_DTE_OFFSET_XMD1),
			      &cancelled);
      if (cancelled) {
	break;
      }

      /* if we don't own the next buffer: */
      stop = !(receive_dte_rmd1_next & TME_AM7990_XMD1_OWN);
      if (stop) {

	tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
		500, TME_OK,
		(&am7990->tme_am7990_element->tme_element_log_handle,
		 "receive BUFF"));

	/* set BUFF and ERR in RMD1: */
	receive_dte_rmd1 |= TME_AM7990_RMD1_BUFF | TME_AM7990_XMD1_ERR;
      }
    }

    /* write RMD1 to clear OWN: */
    cancelled
      = _tme_am7990_rx_write(am7990,
			     TME_AM7990_DTE_OFFSET_XMD1,
			     (receive_dte_rmd1
			      & ~TME_AM7990_XMD1_OWN));
    if (cancelled) {
      break;
    }

    /* advance to the next receive buffer: */
    am7990->tme_am7990_receive_dte_index
      = ((am7990->tme_am7990_receive_dte_index
	  + 1)
	 & am7990->tme_am7990_receive_dte_index_mask);
    receive_dte_rmd1 = receive_dte_rmd1_next;
  } while (!stop);

  /* if nothing was cancelled: */
  if (!cancelled) {
      
    /* generate a receive interrupt: */
    am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_RINT;
  }
}

/* the am7990 callout function.  it must be called with the mutex locked: */
static void
_tme_am7990_callout(struct tme_am7990 *am7990)
{
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection conn_eth_buffer;
  struct tme_bus_connection *conn_bus;
  unsigned int ctrl;
  struct tme_ethernet_config config;
  const tme_uint8_t *config_addrs[2];
  int again;
  int rc;
  int int_asserted;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (am7990->tme_am7990_callout_flags & TME_AM7990_CALLOUTS_RUNNING) {
    return;
  }

  /* callouts are now running: */
  am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUTS_RUNNING;

  /* loop while we have work to do: */
  do {
    again = FALSE;

    /* if we need to do a transmit scan: */
    if (am7990->tme_am7990_callout_flags & TME_AM7990_CALLOUT_TRANSMIT_SCAN) {
      again = TRUE;

      /* clear the callout flag: */
      am7990->tme_am7990_callout_flags &= ~TME_AM7990_CALLOUT_TRANSMIT_SCAN;

      /* do the transmit scan: */
      _tme_am7990_transmit_scan(am7990);
    }

    /* form our ctrl: */
    ctrl = 0;

    /* if the transmitter is on and the next transmit buffer is for
       the start of a packet and is owned by us, we are readable: */
    if ((am7990->tme_am7990_csr0
	 & TME_AM7990_CSR0_TXON)
	&& ((am7990->tme_am7990_transmit_dte_tmd1
	     & (TME_AM7990_XMD1_OWN
		| TME_AM7990_XMD1_STP))
	    == (TME_AM7990_XMD1_OWN
		| TME_AM7990_XMD1_STP))) {
      ctrl |= TME_ETHERNET_CTRL_OK_READ;
    }

    /* if we are readable and in loopback mode: */
    if ((ctrl & TME_ETHERNET_CTRL_OK_READ)
	&& (am7990->tme_am7990_mode & TME_AM7990_MODE_LOOP)) {
      again = TRUE;

      /* unlock the mutex: */
      tme_mutex_unlock(&am7990->tme_am7990_mutex);

      /* call out a loopback transmit: */
      conn_eth_buffer.tme_ethernet_connection.tme_connection_element = am7990->tme_am7990_element;
      _tme_am7990_transmit(&conn_eth_buffer,
			   NULL,
			   NULL,
			   0);

      /* lock the mutex: */
      tme_mutex_lock(&am7990->tme_am7990_mutex);

      /* don't call out any new control information for now: */
      ctrl = am7990->tme_am7990_ether_ctrl_out;
    }

    /* if we need to call out new control information: */
    if (am7990->tme_am7990_ether_ctrl_out != ctrl) {
      again = TRUE;

      /* note the new state of the called-out control information: */
      am7990->tme_am7990_ether_ctrl_out = ctrl;

      /* get this card's connection: */
      conn_eth = am7990->tme_am7990_eth_connection;

      /* unlock the mutex: */
      tme_mutex_unlock(&am7990->tme_am7990_mutex);
      
      /* do the callout: */
      rc = (conn_eth != NULL
	    ? ((*conn_eth->tme_ethernet_connection_ctrl)
	       (conn_eth,
		ctrl))
	    : TME_OK);
      assert (rc == TME_OK);

      /* lock the mutex: */
      tme_mutex_lock(&am7990->tme_am7990_mutex);
    }

    /* if we need to call out new config information: */
    if (am7990->tme_am7990_callout_flags & TME_AM7990_CALLOUT_CONFIG) {
      again = TRUE;

      /* clear the callout flag: */
      am7990->tme_am7990_callout_flags &= ~TME_AM7990_CALLOUT_CONFIG;
      
      /* form the new config: */
      memset(&config, 0, sizeof(config));

      /* if we're in promiscuous mode or any bits the logical address
	 filter are nonzero: */
      if ((am7990->tme_am7990_mode & TME_AM7990_MODE_PROM)
	  || (&am7990->tme_am7990_ladrf)[0]
	  || (&am7990->tme_am7990_ladrf)[1]
	  || (&am7990->tme_am7990_ladrf)[2]
	  || (&am7990->tme_am7990_ladrf)[3]) {

	/* we have to be in promiscuous mode: */
	config.tme_ethernet_config_flags |= TME_ETHERNET_CONFIG_PROMISC;
      }

      /* otherwise, we only need to see packets addressed to our
	 physical address, and broadcast packets: */
      else {
      
	/* our Ethernet addresses: */
	config.tme_ethernet_config_addr_count = 2;
	config_addrs[0] = (tme_uint8_t *) &am7990->tme_am7990_padr;
	config_addrs[1] = &tme_ethernet_addr_broadcast[0];
	config.tme_ethernet_config_addrs = config_addrs;
      }

      /* get this card's connection: */
      conn_eth = am7990->tme_am7990_eth_connection;

      /* unlock the mutex: */
      tme_mutex_unlock(&am7990->tme_am7990_mutex);
      
      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_config)
	       (conn_eth,
		&config)));
      assert (rc == TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&am7990->tme_am7990_mutex);
    }

    /* if we need to receive a frame: */
    if (am7990->tme_am7990_callout_flags & TME_AM7990_CALLOUT_RECEIVE) {
      again = TRUE;

      /* clear the callout flag: */
      am7990->tme_am7990_callout_flags &= ~TME_AM7990_CALLOUT_RECEIVE;

      /* do the receive: */
      _tme_am7990_receive(am7990);
    }

    /* if we need to call out an interrupt: */
    if (am7990->tme_am7990_csr0
	& (TME_AM7990_CSR0_TINT
	   | TME_AM7990_CSR0_BABL
	   | TME_AM7990_CSR0_MISS
	   | TME_AM7990_CSR0_MERR
	   | TME_AM7990_CSR0_RINT
	   | TME_AM7990_CSR0_TINT
	   | TME_AM7990_CSR0_IDON)) {
      am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_INTR;
    }
    else {
      am7990->tme_am7990_csr0 &= ~TME_AM7990_CSR0_INTR;
    }
    int_asserted
      = ((am7990->tme_am7990_csr0
	  & (TME_AM7990_CSR0_INTR
	     | TME_AM7990_CSR0_INEA))
	 == (TME_AM7990_CSR0_INTR
	     | TME_AM7990_CSR0_INEA));
    if (!!am7990->tme_am7990_int_asserted != int_asserted) {
      again = TRUE;

      /* note the new state of the interrupt signal: */
      am7990->tme_am7990_int_asserted = int_asserted;

      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						am7990->tme_am7990_device.tme_bus_device_connection,
						&am7990->tme_am7990_device.tme_bus_device_connection_rwlock);

      /* unlock our mutex: */
      tme_mutex_unlock(&am7990->tme_am7990_mutex);
      
      /* call out the bus interrupt signal edge: */
      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 TME_BUS_SIGNAL_INT_UNSPEC
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));
      assert (rc == TME_OK);
      
      /* lock our mutex: */
      tme_mutex_lock(&am7990->tme_am7990_mutex);
    }
  } while (again);

  /* clear that callouts are running: */
  am7990->tme_am7990_callout_flags &= ~TME_AM7990_CALLOUTS_RUNNING;
}

/* the am7990 bus signal handler: */
static int
_tme_am7990_signal(void *_am7990, 
		   unsigned int signal)
{
  struct tme_am7990 *am7990;
  unsigned int level;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* lock the mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);

  /* take out the signal level: */
  level = signal & TME_BUS_SIGNAL_LEVEL_MASK;
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* dispatch on the generic bus signals: */
  switch (signal) {
  case TME_BUS_SIGNAL_RESET:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      _tme_am7990_reset(am7990);
    }
    break;
  default:
    signal = TME_BUS_SIGNAL_IGNORE;
    break;
  }

  /* if we didn't ignore this bus signal: */
  if (signal != TME_BUS_SIGNAL_IGNORE) {

    /* make any new callouts: */
    _tme_am7990_callout(am7990);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&am7990->tme_am7990_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this is called when a device changes its configuration: */
static int
_tme_am7990_config(struct tme_ethernet_connection *conn_eth, 
		  struct tme_ethernet_config *config)
{
  /* we don't care when other devices on the Ethernet
     reconfigure themselves: */
  return (TME_OK);
}

/* this is called when control lines change: */
static int
_tme_am7990_ctrl(struct tme_ethernet_connection *conn_eth, 
		 unsigned int ctrl)
{
  struct tme_am7990 *am7990;

  /* recover our data structures: */
  am7990 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);

  /* note the new state of the controls: */
  am7990->tme_am7990_ether_ctrl_in = ctrl;

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_ETHERNET_CTRL_OK_READ) {
    am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_RECEIVE;
  }

  /* make any new callouts: */
  _tme_am7990_callout(am7990);

  /* unlock the mutex: */
  tme_mutex_unlock(&am7990->tme_am7990_mutex);

  return (TME_OK);
}

/* this transmits a frame: */
static int
_tme_am7990_transmit(struct tme_ethernet_connection *conn_eth, 
		     tme_ethernet_fid_t *_frame_id,
		     struct tme_ethernet_frame_chunk *frame_chunk,
		     unsigned int flags)
{
  struct tme_am7990 *am7990;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  tme_uint8_t frame_chunk_bytes_buffer[32];
  tme_uint8_t *frame_chunk_bytes;
  unsigned int frame_chunk_bytes_count;
  tme_uint16_t transmit_dte_tmd0;
  tme_uint16_t transmit_dte_tmd1;
  tme_uint16_t transmit_dte_tmd2;
  tme_uint16_t transmit_dte_tmd1_next;
  tme_uint32_t transmit_buffer_address;
  tme_uint16_t transmit_buffer_count;
  unsigned int count;
  int rc;
  int add_fcs;
  int stop;
  int cancelled;

  /* recover our data structures: */
  am7990 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* lock our mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);

  /* assume that we will have no packet to transmit: */
  rc = 0;

  /* if the transmitter is on, and the current transmit buffer is for
     the start of a packet and is owned by us: */
  if ((am7990->tme_am7990_csr0
       & TME_AM7990_CSR0_TXON)
      && ((am7990->tme_am7990_transmit_dte_tmd1
	   & (TME_AM7990_XMD1_OWN
	      | TME_AM7990_XMD1_STP))
	  == (TME_AM7990_XMD1_OWN
	      | TME_AM7990_XMD1_STP))) {
	
    /* see if we will add the FCS to this transmitted packet.
       "ADD_FCS is only valid when STP=1." */
    add_fcs =
      (!(am7990->tme_am7990_mode
	 & TME_AM7990_MODE_DTCR)
       || (am7990->tme_am7990_transmit_dte_tmd1
	   & TME_AM7990_TMD1_ADD_FCS));

    /* if we're in loopback mode, transmit this packet directly into
       the receive buffer: */
    if (am7990->tme_am7990_mode & TME_AM7990_MODE_LOOP) {
      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes = &am7990->tme_am7990_receive_buffer[0];
      frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count = sizeof(am7990->tme_am7990_receive_buffer);
      frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
      frame_chunk = &frame_chunk_buffer;
    }

    do {

      /* recover the previously read TMD1: */
      transmit_dte_tmd1 = am7990->tme_am7990_transmit_dte_tmd1;

      /* read TMD0 and TMD2: */
      transmit_dte_tmd0
	= _tme_am7990_tx_read(am7990,
			      TME_AM7990_DTE_OFFSET_XMD0,
			      &cancelled);
      if (cancelled) {
	break;
      }
      transmit_dte_tmd2
	= _tme_am7990_tx_read(am7990,
			      TME_AM7990_DTE_OFFSET_XMD2,
			      &cancelled);
      if (cancelled) {
	break;
      }

      /* get the transmit buffer address and count: */
      transmit_buffer_address = (transmit_dte_tmd1 & TME_AM7990_XMD1_HADR);
      transmit_buffer_address = (transmit_buffer_address << 16) + transmit_dte_tmd0;
      transmit_buffer_count = (0 - transmit_dte_tmd2) & TME_AM7990_XMD2_BCNT;

      /* read this transmit buffer into the frame chunks: */
      frame_chunk_bytes = NULL;
      frame_chunk_bytes_count = 0;
      for (; transmit_buffer_count > 0; ) {

	/* if we have exhausted the previous chunk: */
	if (frame_chunk_bytes_count == 0) {
	  
	  /* advance to the next chunk: */
	  if (frame_chunk == NULL) {
	    frame_chunk_bytes = &frame_chunk_bytes_buffer[0];
	    frame_chunk_bytes_count = sizeof(frame_chunk_bytes_buffer);
	  }
	  else {
	    frame_chunk_bytes = frame_chunk->tme_ethernet_frame_chunk_bytes;
	    frame_chunk_bytes_count = frame_chunk->tme_ethernet_frame_chunk_bytes_count;
	    frame_chunk = frame_chunk->tme_ethernet_frame_chunk_next;
	    continue;
	  }
	}
	    
	/* get the count of bytes to read in this iteration: */
	count = TME_MIN(frame_chunk_bytes_count, transmit_buffer_count);

	/* do the read: */
	cancelled 
	  = _tme_am7990_dma(am7990,
			    TME_AM7990_CALLOUT_DMA_READ,
			    transmit_buffer_address,
			    count,
			    frame_chunk_bytes);
	if (cancelled) {
	  break;
	}

	/* advance: */
	frame_chunk_bytes += count;
	frame_chunk_bytes_count -= count;
	transmit_buffer_address += count;
	transmit_buffer_count -= count;
	rc += count;
      }
      if (cancelled) {
	break;
      }

      /* if this is the last transmit buffer in the packet: */
      if (transmit_dte_tmd1 & TME_AM7990_XMD1_ENP) {

	/* we can't suppress CRC generation: */
	if (!add_fcs) {
	  abort();
	}

	/* clear TMD1 for the next transmit buffer: */
	transmit_dte_tmd1_next = 0;

	/* this is the last transmit buffer: */
	stop = TRUE;
      }

      /* otherwise, this is not the last transmit buffer in the packet: */
      else {
	
	/* read TMD1 for the next transmit buffer: */
	transmit_dte_tmd1_next
	  = _tme_am7990_tx_read(am7990,
				(TME_AM7990_DTE_SIZE
				 + TME_AM7990_DTE_OFFSET_XMD1),
				&cancelled);
	if (cancelled) {
	  break;
	}

	/* if we don't own the next transmit buffer: */
	stop = !(transmit_dte_tmd1_next & TME_AM7990_XMD1_OWN);
	if (stop) {

	  tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
		  500, TME_OK,
		  (&am7990->tme_am7990_element->tme_element_log_handle,
		   "transmit BUFF"));

	  /* write a TMD3 with BUFF set: */
	  cancelled
	    = _tme_am7990_tx_write(am7990,
				   TME_AM7990_DTE_OFFSET_XMD3, 
				   TME_AM7990_TMD3_BUFF);
	  if (cancelled) {
	    break;
	  }

	  /* turn off the transmitter: */
	  am7990->tme_am7990_csr0 &= ~TME_AM7990_CSR0_TXON;
	}
      }

      /* write TMD1 to clear OWN: */
      cancelled
	= _tme_am7990_tx_write(am7990,
			       TME_AM7990_DTE_OFFSET_XMD1,
			       (transmit_dte_tmd1
				& ~TME_AM7990_XMD1_OWN));
      if (cancelled) {
	break;
      }

      /* advance to the next transmit buffer: */
      am7990->tme_am7990_transmit_dte_index
	= ((am7990->tme_am7990_transmit_dte_index
	    + 1)
	   & am7990->tme_am7990_transmit_dte_index_mask);
      am7990->tme_am7990_transmit_dte_tmd1 = transmit_dte_tmd1_next;
    } while (!stop);

    /* if nothing was cancelled: */
    if (!cancelled) {
      
      /* generate a transmit interrupt: */
      am7990->tme_am7990_csr0 |= TME_AM7990_CSR0_TINT;
    }
  }

  /* if we're in loopback mode: */
  if (am7990->tme_am7990_mode & TME_AM7990_MODE_LOOP) {

    /* if we have a loopback packet: */
    if (rc > 0) {

      /* append the CRC bytes: */
      if ((rc + sizeof(tme_uint32_t)) > sizeof(am7990->tme_am7990_receive_buffer)) {
	abort();
      }
      rc += sizeof(tme_uint32_t);
      
      /* set the receive buffer length: */
      am7990->tme_am7990_receive_buffer_length = rc;
    }

    /* if we have a loopback packet, call out a receive: */
    if (am7990->tme_am7990_receive_buffer_length > 0) {
      am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_RECEIVE;
    }
  }

  /* otherwise, if we're not returning a packet: */
  else if (rc <= 0) {

    /* we aren't considered readable any more: */
    am7990->tme_am7990_ether_ctrl_out &= ~TME_ETHERNET_CTRL_OK_READ;
  }

  /* start a scan for the first buffer in the next packet to transmit: */
  am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_TRANSMIT_SCAN;

  /* make any new callouts: */
  _tme_am7990_callout(am7990);

  /* unlock our mutex: */
  tme_mutex_unlock(&am7990->tme_am7990_mutex);

  /* done: */
  return (rc);

  /* unused: */
  _frame_id = 0;
  flags = 0;
}

/* the am7990 descriptor polling thread: */
static void
_tme_am7990_poll_th(struct tme_am7990 *am7990)
{

  /* lock our mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);

  /* loop forever: */
  for (;;) {

    /* call out a transmit scan: */
    am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_TRANSMIT_SCAN;

    /* make any callouts: */
    _tme_am7990_callout(am7990);

    /* unlock our mutex: */
    tme_mutex_unlock(&am7990->tme_am7990_mutex);

    /* "[T]he C-LANCE will automatically poll the transmit ring in the
       memory once it has started ..  every 1.6ms" */
    tme_thread_sleep_yield(0, 16000);

    /* lock our mutex: */
    tme_mutex_unlock(&am7990->tme_am7990_mutex);
  }
  /* NOTREACHED */
}

/* the am7990 bus cycle handler: */
static int
_tme_am7990_bus_cycle(void *_am7990, struct tme_bus_cycle *cycle_init)
{
  struct tme_am7990 *am7990;
  tme_bus_addr32_t address, am7990_address_last;
  tme_uint16_t value;
  tme_uint32_t reg;
  tme_uint16_t csr0_old;
  tme_uint16_t csr0_new;
  tme_uint8_t port_init;
  unsigned int routing_index;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* the address must be within range: */
  am7990_address_last = am7990->tme_am7990_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= am7990_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (am7990_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* get the register being accessed: */
  address = cycle_init->tme_bus_cycle_address;
  reg = (address & ((am7990_address_last + 1) / 2));

  /* save the initiator's port description: */
  port_init = cycle_init->tme_bus_cycle_port;
  
  /* lock the mutex: */
  tme_mutex_lock(&am7990->tme_am7990_mutex);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS16_LOG2);

    /* if this was a write to the RAP: */
    if (reg == am7990->tme_am7990_offset_rap) {

      /* set the new RAP: */
      am7990->tme_am7990_rap = value % TME_ARRAY_ELS(am7990->tme_am7990_csrs);
    }

    /* otherwise, this was a write to the RDP: */
    else {
      assert (reg == am7990->tme_am7990_offset_rdp);

      /* if this is a write to CSR0: */
      if (am7990->tme_am7990_rap == 0) {

	/* get the previous value of CSR0: */
	csr0_old = am7990->tme_am7990_csr0;

	/* handle the read-only, write-1-to-clear, and write-1-only
           bits: */
	csr0_new = ((value & ~TME_AM7990_CSR0_READ_ONLY)
		    | (csr0_old
		       & (TME_AM7990_CSR0_READ_ONLY
			  | TME_AM7990_CSR0_WRITE_ONE_ONLY)));
	csr0_new = ((csr0_new ^ TME_AM7990_CSR0_WRITE_ONE_TO_CLEAR)
		    & (csr0_old | ~TME_AM7990_CSR0_WRITE_ONE_TO_CLEAR));

	/* update ERR: */
	csr0_new &= ~TME_AM7990_CSR0_ERR;
	if (csr0_new
	    & (TME_AM7990_CSR0_BABL
	       | TME_AM7990_CSR0_CERR
	       | TME_AM7990_CSR0_MISS
	       | TME_AM7990_CSR0_MERR)) {
	  csr0_new |= TME_AM7990_CSR0_ERR;
	}

	/* set the initial new CSR0 value: */
	am7990->tme_am7990_csr0 = csr0_new;

	/* "If STRT, INIT and STOP are all set together, STOP will
           override the other bits and only STOP will be set." */
	if (value & TME_AM7990_CSR0_STOP) {

	  /* reset the am7990: */
	  _tme_am7990_reset(am7990);
	}

	/* otherwise, STOP is not set in the new CSR0 value: */
	else {

	  /* if INIT is set in the CSR0 value written, and STOP is set
             in the old CSR0 value: */
	  if ((value & TME_AM7990_CSR0_INIT)
	      && (csr0_old & TME_AM7990_CSR0_STOP)) {

	    /* run the initialization procedure: */
	    _tme_am7990_init(am7990);
	  }

	  /* if STRT is set in the CSR0 value written, and STRT is not
	     set in the old CSR0 value: */
	  if ((value & TME_AM7990_CSR0_STRT)
	      && !(csr0_old & TME_AM7990_CSR0_STRT)) {

	    /* start the am7990: */
	    _tme_am7990_start(am7990);
	  }

	  /* if TDMD is set in the CSR0 value written, call out a
             transmit scan: */
	  if (value & TME_AM7990_CSR0_TDMD) {
	    am7990->tme_am7990_callout_flags |= TME_AM7990_CALLOUT_TRANSMIT_SCAN;
	  }
	}
      }

      /* otherwise, this is writing another CSR: */
      else {

	tme_log(&am7990->tme_am7990_element->tme_element_log_handle,
		100, TME_OK,
		(&am7990->tme_am7990_element->tme_element_log_handle,
		 "CSR%u <- 0x%04x",
		 am7990->tme_am7990_rap,
		 value));

	am7990->tme_am7990_csrs[am7990->tme_am7990_rap] = value;
      }
    }
  }
	    
  /* otherwise, this is a read: */
  else {
    assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* get the value to read: */
    if (reg == am7990->tme_am7990_offset_rap) {
      value = am7990->tme_am7990_rap;
    }
    else {
      assert (reg == am7990->tme_am7990_offset_rdp);
      value = am7990->tme_am7990_csrs[am7990->tme_am7990_rap];
    }

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init, 
			   &value,
			   TME_BUS16_LOG2);
  }

  /* NB: the generic memory implementation doesn't know its own port
     size or endianness, and instead just follows the endianness of
     whatever reads and writes it (see tme_bus_cycle_xfer_memory()).
     this is sort of a fragile system, but does allow the same generic
     memory implementation to work with all kinds of CPU emulations.

     the fragility shows with a chip like the am7990.  for all of its
     non-Ethernet data bus mastering, the real chip doesn't need to
     know memory's endianness; it just reads a 16-bit quantity from a
     16-bit aligned address and expects that the most-significant
     eight bits will be on D8..D15.  but since the memory doesn't know
     its own endianness, we have to use a bus routing that matches the
     endianness of the bus routing used by the CPU that wrote the
     memory.

     fortunately, we can learn the endianness of the CPU when it reads
     or writes us.  we do this here: */
  if (am7990->tme_am7990_device.tme_bus_device_router == NULL) {

    /* get the index into the bus routing used by the initiator for
       this cycle.  if the initiator's least lane is zero, then this
       is also a lane number: */
    routing_index
      = (TME_BUS_CYCLE_PORT_LANE_LEAST(cycle_init->tme_bus_cycle_port)
	 - TME_BUS_CYCLE_PORT_LANE_LEAST(port_init));

    /* if the initiator routed a byte from a lower address to the byte
       lane of lesser significance, then we assume that memory is
       little-endian: */
    am7990->tme_am7990_device.tme_bus_device_router
      = ((cycle_init->tme_bus_cycle_lane_routing[routing_index + 0]
	  < cycle_init->tme_bus_cycle_lane_routing[routing_index + 1])
	 ? tme_bus_device_router_16el
	 : tme_bus_device_router_16eb);
  }

  /* make any new callouts: */
  _tme_am7990_callout(am7990);

  /* unlock the mutex: */
  tme_mutex_unlock(&am7990->tme_am7990_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the am7990 TLB filler: */
static int
_tme_am7990_tlb_fill(void *_am7990,
		     struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address,
		     unsigned int cycles)
{
  struct tme_am7990 *am7990;
  tme_bus_addr32_t am7990_address_last;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) _am7990;

  /* the address must be within range: */
  am7990_address_last = am7990->tme_am7990_device.tme_bus_device_address_last;
  assert(address <= am7990_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = am7990_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = am7990;
  tlb->tme_bus_tlb_cycle = _tme_am7990_bus_cycle;

  return (TME_OK);
}

/* this makes a new Ethernet connection: */
static int
_tme_am7990_connection_make_eth(struct tme_connection *conn, unsigned int state)
{
  struct tme_am7990 *am7990;
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection *conn_eth_other;

  /* recover our data structures: */
  am7990 = conn->tme_connection_element->tme_element_private;
  conn_eth = (struct tme_ethernet_connection *) conn;
  conn_eth_other = (struct tme_ethernet_connection *) conn->tme_connection_other;

  /* both sides must be Ethernet connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_ETHERNET);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_ETHERNET);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* lock our mutex: */
    tme_mutex_lock(&am7990->tme_am7990_mutex);

    /* save our connection: */
    am7990->tme_am7990_eth_connection = conn_eth_other;

    /* unlock our mutex: */
    tme_mutex_unlock(&am7990->tme_am7990_mutex);
  }

  return (TME_OK);
}

/* this makes a new bus connection: */
static int
_tme_am7990_connection_make_bus(struct tme_connection *conn,
				unsigned int state)
{
  struct tme_am7990 *am7990;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* recover our data structure: */
  am7990 = conn->tme_connection_element->tme_element_private;

  /* call the bus device connection maker: */
  rc = tme_bus_device_connection_make(conn, state);

  /* if the full connection was successful, and we don't have a TLB
     hash yet, allocate it: */
  if (rc == TME_OK
      && state == TME_CONNECTION_FULL
      && !am7990->tme_am7990_tlb_hash_added) {

    /* get our bus connection: */
    conn_bus
      = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
				       am7990->tme_am7990_device.tme_bus_device_connection,
				       &am7990->tme_am7990_device.tme_bus_device_connection_rwlock);

    /* add the TLB set: */
    rc = tme_bus_device_tlb_set_add(&am7990->tme_am7990_device,
				    TME_AM7990_TLB_HASH_SIZE,
				    am7990->tme_am7990_tlb_hash);
    assert (rc == TME_OK);
    am7990->tme_am7990_tlb_hash_added = TRUE;
  }

  return (rc);
}

/* this breaks a connection: */
static int
_tme_am7990_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a am7990: */
static int
_tme_am7990_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_am7990 *am7990;
  struct tme_ethernet_connection *conn_eth;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  am7990 = (struct tme_am7990 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* since we need to allocate our TLB hash when we make our bus
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
	= _tme_am7990_connection_make_bus;
    }
  }

  /* if we don't have an Ethernet connection, make one: */
  if (am7990->tme_am7990_eth_connection == NULL) {

    /* allocate the new Ethernet connection: */
    conn_eth = tme_new0(struct tme_ethernet_connection, 1);
    conn = &conn_eth->tme_ethernet_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_ETHERNET;
    conn->tme_connection_score = tme_ethernet_connection_score;
    conn->tme_connection_make = _tme_am7990_connection_make_eth;
    conn->tme_connection_break = _tme_am7990_connection_break;

    /* fill in the Ethernet connection: */
    conn_eth->tme_ethernet_connection_config = _tme_am7990_config;
    conn_eth->tme_ethernet_connection_ctrl = _tme_am7990_ctrl;
    conn_eth->tme_ethernet_connection_read = _tme_am7990_transmit;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new am7990 function: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,am7990,am7990) {
  struct tme_am7990 *am7990;
  int arg_i;
  int usage;
  tme_uint32_t reg_size;

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

  /* start the am7990 structure: */
  am7990 = tme_new0(struct tme_am7990, 1);
  am7990->tme_am7990_element = element;
  tme_mutex_init(&am7990->tme_am7990_mutex);

  /* set the register offsets: */
  /* XXX FIXME - these should come from a socket structure: */
  am7990->tme_am7990_offset_rdp = 0;
  am7990->tme_am7990_offset_rap = sizeof(tme_uint16_t);

  /* get the total register size: */
  assert ((am7990->tme_am7990_offset_rdp & (am7990->tme_am7990_offset_rdp - 1)) == 0);
  assert ((am7990->tme_am7990_offset_rap & (am7990->tme_am7990_offset_rap - 1)) == 0);
  reg_size = TME_MAX(am7990->tme_am7990_offset_rdp, am7990->tme_am7990_offset_rap) * 2;

  /* initialize our simple bus device descriptor: */
  am7990->tme_am7990_device.tme_bus_device_element = element;
  am7990->tme_am7990_device.tme_bus_device_tlb_fill = _tme_am7990_tlb_fill;
  am7990->tme_am7990_device.tme_bus_device_address_last = reg_size - 1;
  am7990->tme_am7990_device.tme_bus_device_signal = _tme_am7990_signal;
  am7990->tme_am7990_device.tme_bus_device_lock = _tme_am7990_lock;
  am7990->tme_am7990_device.tme_bus_device_unlock = _tme_am7990_unlock;
  am7990->tme_am7990_device.tme_bus_device_tlb_hash = _tme_am7990_tlb_hash;

  /* fill the element: */
  element->tme_element_private = am7990;
  element->tme_element_connections_new = _tme_am7990_connections_new;

  /* start the descriptor polling thread: */
  tme_thread_create((tme_thread_t) _tme_am7990_poll_th, am7990);

  /* reset the am7990: */
  _tme_am7990_reset(am7990);

  return (TME_OK);
}
