/* $Id: stp222x-main.c,v 1.4 2009/09/07 15:41:07 fredette Exp $ */

/* ic/stp222x-main.c - main emulation of the UPA to SBus interface
   controller (STP2220) and the UPA to PCI interface controller
   (STP2222): */

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
_TME_RCSID("$Id: stp222x-main.c,v 1.4 2009/09/07 15:41:07 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include "stp222x-impl.h"

/* macros: */

/* common register offsets: */
#define TME_STP222X_REG00_UPA_PORT_ID		(0x0000)
#define TME_STP222X_REG00_UPA_PORT_CONFIG	(0x0008)
#define TME_STP222X_REG00_CSR			(0x0010)
						/* 0x0018 unused */
#define TME_STP222X_REG00_ECC_CONTROL		(0x0020)
						/* 0x0028 unused */
#define TME_STP222X_REG00_UE_AFSR		(0x0030)
#define TME_STP222X_REG00_UE_AFAR		(0x0038)
#define TME_STP222X_REG00_CE_AFSR		(0x0040)
#define TME_STP222X_REG00_CE_AFAR		(0x0048)
#define TME_STP222X_REG01_PM_CR			(0x0100)
#define TME_STP222X_REG01_PM_COUNT		(0x0108)

/* the control/status register: */
#define TME_STP222X_CSR_IMPL			((((tme_uint64_t) 2) << 63) - (((tme_uint64_t) 1) << 60))
#define TME_STP222X_CSR_VER			((((tme_uint64_t) 2) << 59) - (((tme_uint64_t) 1) << 56))
#define TME_STP222X_CSR_MID			((((tme_uint64_t) 2) << 55) - (((tme_uint64_t) 1) << 51))
#define TME_STP222X_CSR_IGN			((((tme_uint64_t) 2) << 50) - (((tme_uint64_t) 1) << 46))
#define TME_STP222X_CSR_APCKEN			(1 << 3)
#define TME_STP222X_CSR_APERR			(1 << 2)
#define TME_STP222X_CSR_IAP			(1 << 1)
#define TME_STP222X_CSR_MODE			(1 << 0)

/* stp2220-specific registers: */
#define TME_STP2220_REG20_SBUS_CSR		(0x2000)
						/* 0x2028 unused */
#define TME_STP2220_REG20_SBUS_AFSR		(0x2010)
#define TME_STP2220_REG20_SBUS_AFAR		(0x2018)
#define TME_STP2220_REG20_SBUS_CONFIG(n)	(0x2020 + (TME_STP222X_REG_SIZE * (n)))

/* the stp2220 slot size: */
#define TME_STP2220_SLOT_SIZE			(256 * 1024 * 1024)

/* this gives the raw stp2220 address for a slot and offset: */
#define TME_STP2220_SBUS_ADDRESS(slot, offset)	(((slot) * TME_STP2220_SLOT_SIZE) + (offset))

/* reset states: */
#define TME_STP222X_RESET_STATE_NEGATED		(0)
#define TME_STP222X_RESET_STATE_NEGATING	(TME_STP222X_RESET_STATE_NEGATED + TME_STP222X_CONN_NULL)
#define TME_STP222X_RESET_STATE_ASSERTED	(TME_STP222X_RESET_STATE_NEGATING + 1)
#define TME_STP222X_RESET_STATE_ASSERTING	(TME_STP222X_RESET_STATE_ASSERTED + TME_STP222X_CONN_NULL)

/* globals: */

/* the simple 64-bit register bus router: */
static const tme_bus_lane_t _tme_stp222x_bus_router_regs[sizeof(tme_uint64_t)] = {
  TME_BUS_LANE_ROUTE(0),
  TME_BUS_LANE_ROUTE(1),
  TME_BUS_LANE_ROUTE(2),
  TME_BUS_LANE_ROUTE(3),
  TME_BUS_LANE_ROUTE(4),
  TME_BUS_LANE_ROUTE(5),
  TME_BUS_LANE_ROUTE(6),
  TME_BUS_LANE_ROUTE(7)
};

/* the STP2220 mapping from obio slot and offset to IDI: */
static const struct {
  tme_uint32_t _tme_stp2220_obio_slot;
  tme_uint32_t _tme_stp2220_obio_offset;
  tme_uint32_t _tme_stp2220_obio_idi;
} _tme_stp2220_obios[] = {
  { TME_STP2220_SLOT_AUDIO,	0x0000000, TME_STP2220_IDI_AUDIO, },
  { TME_STP2220_SLOT_MACIO,	0x8800000, TME_STP222X_IDI_SCSI },
  { TME_STP2220_SLOT_MACIO,	0x8c00000, TME_STP222X_IDI_ETHER },
  { TME_STP2220_SLOT_MACIO,	0xc800000, TME_STP222X_IDI_BPP },
  { TME_STP2220_SLOT_SLAVIO,	0x1100000, TME_STP2220_IDI_ZS0_ZS1 },
  { TME_STP2220_SLOT_SLAVIO,	0x1000000, TME_STP2220_IDI_ZS0_ZS1 },
  { TME_STP2220_SLOT_SLAVIO,	0x1400000, TME_STP2220_IDI_FD },
};

/* this converts an I/O connection into a connection index: */
static tme_uint32_t
_tme_stp222x_io_conn_index(const struct tme_stp222x *stp222x,
			   const struct tme_bus_connection *io_conn_bus)
{
  tme_uint32_t connid;
  tme_uint32_t conn_index;

  /* this must be an I/O connection: */
  assert (io_conn_bus
	  != stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_bus);

  /* get the connection id: */
  connid = io_conn_bus->tme_bus_connection.tme_connection_id;

  /* if this is a card connection: */
  if ((connid
       & TME_STP222X_CONNID_TYPE)
      == TME_STP222X_CONNID_TYPE_CARD) {

    /* if this is an stp2220: */
    if (TME_STP222X_IS_2220(stp222x)) {

      /* assume that this is the first master connection for this
	 card, which uses the card's primary master connection
	 index: */
      conn_index = TME_FIELD_MASK_EXTRACTU(connid, TME_STP2220_CONNID_CARD_WHICH);

      /* if this is an alternate connection for the card: */
      if (connid & TME_STP2220_CONNID_CARD_ALTERNATE) {

	/* starting from the first alternate master connection index
	   for this card and moving forward, search all remaining
	   connection indices: */
	conn_index
	  = (TME_STP2220_SLOTS_CARD
	     + (connid & TME_STP2220_CONNID_CARD_WHICH));
	for (; stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != io_conn_bus; ) {
	  conn_index++;
	  assert (conn_index < stp222x->tme_stp222x_slave_conn_index_next);
	}
      }
    }

    /* otherwise, this is an stp2222: */
    else {
      abort();
    }
  }

  /* otherwise, this is an obio connection id: */
  else {

    /* assume that this is an obio short connection id, and get the
       connection index: */
    conn_index = TME_FIELD_MASK_EXTRACTU(connid, TME_STP222X_CONNID_OBIO_SHORT_CONN_WHICH);

    /* if this is an obio long connection id: */
    if ((connid
	 & TME_STP222X_CONNID_OBIO_TYPE)
	== TME_STP222X_CONNID_OBIO_TYPE_LONG) {

      /* search all connection indices: */
      conn_index = 0;
      for (; stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != io_conn_bus; ) {
	conn_index++;
	assert (conn_index < stp222x->tme_stp222x_slave_conn_index_next);
      }
    }
  }

  assert (stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus == io_conn_bus);
  return (conn_index);
}

/* this converts an I/O connection and interrupt signal into an
   IDI: */
static tme_uint32_t
_tme_stp222x_io_idi(const struct tme_stp222x *stp222x,
		    const struct tme_bus_connection *io_conn_bus,
		    unsigned int signal_int)
{
  tme_uint32_t connid;
  tme_uint32_t idi;
  tme_uint32_t idi_addend;

  /* this must be an I/O connection: */
  assert (io_conn_bus
	  != stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_bus);

  /* get the connection id: */
  connid = io_conn_bus->tme_bus_connection.tme_connection_id;

  /* if this is a card connection: */
  if ((connid
       & TME_STP222X_CONNID_TYPE)
      == TME_STP222X_CONNID_TYPE_CARD) {

    /* if this is an stp2220, bits 1..2 of a card connection id are
       the card (slot) number, which are bits 3..4 of the connection's
       base IDI.  otherwise, this is an stp2222, and bits 1..3 of a
       card connection id are the bus and slot number, which are bits
       2..4 of the connection's base IDI.  we take advantage of this
       to quickly make the base IDI: */
#if TME_STP222X_CONNID_TYPE != (0x1 << 0)
#error "TME_STP222X_CONNID_TYPE changed"
#endif
#if TME_STP222X_CONNID_TYPE_CARD != 0
#error "TME_STP222X_CONNID_TYPE_CARD changed"
#endif
#if (TME_STP2222_CONNID_BUS_WHICH + TME_STP2222_CONNID_DEVICE_WHICH) != (0x7 << 1)
#error "TME_STP2222_CONNID_BUS_WHICH or TME_STP2222_CONNID_DEVICE_WHICH changed"
#endif
#if TME_STP2220_CONNID_CARD_WHICH != (0x3 << 1)
#error "TME_STP2220_CONN_CARD_WHICH changed"
#endif
    idi = connid << 1;
    if (TME_STP222X_IS_2220(stp222x)) {
      idi <<= 1;
    }
    idi %= TME_STP222X_IDI0_OBIO;

    /* the interrupt signal can't be unspecified: */
    assert (signal_int != TME_BUS_SIGNAL_INT_UNSPEC);

    /* convert the interrupt signal into an IDI addend: */
    idi_addend = TME_BUS_SIGNAL_INDEX_INT(signal_int);

    /* the IDI addend must be within range: */
    assert (idi_addend
	    < (TME_STP222X_IS_2220(stp222x)
	       ? TME_SBUS_SLOT_INTS
	       : TME_PCI_SLOT_INTS));
    
    /* add in the IDI addend: */
    idi += idi_addend;
  }

  /* otherwise, this is an obio connection: */
  else {

    /* this must be a obio short connection id: */
    assert ((connid
	     & TME_STP222X_CONNID_OBIO_TYPE)
	    == TME_STP222X_CONNID_OBIO_TYPE_SHORT);

    /* get the connection's base IDI: */
    idi = TME_FIELD_MASK_EXTRACTU(connid, TME_STP222X_CONNID_OBIO_SHORT_IDI);

    /* if the interrupt signal is specified: */
    if (signal_int != TME_BUS_SIGNAL_INT_UNSPEC) {

      /* the audio connection is the only obio connection allowed to
	 specify its interrupt signal - zero is used for record, one
	 for playback: */
      assert (signal_int == TME_BUS_SIGNAL_INT(0)
	      || signal_int == TME_BUS_SIGNAL_INT(1));
      if (!TME_STP222X_IS_2220(stp222x)) {
	assert (idi == TME_STP2220_IDI_AUDIO);

	/* nothing to do */
      }
      else {
#if TME_STP2222_IDI_AUDIO_PLAYBACK != (TME_STP2222_IDI_AUDIO_RECORD + 1)
#error "TME_STP2222_IDI_AUDIO_ values changed"
#endif
	assert (idi == TME_STP2222_IDI_AUDIO_RECORD);
	idi += TME_BUS_SIGNAL_INDEX_INT(signal_int);
      }
    }
  }

  return (idi);
}

/* this converts an address into an address space: */
static tme_uint32_t
_tme_stp222x_lookup_address(const struct tme_stp222x *stp222x,
			    tme_bus_addr64_t address,
			    tme_bus_addr32_t *_region_size_m1)
{
  tme_bus_addr32_t region_size_m1;
  unsigned int aspace_i;
  tme_uint32_t address_16_47;

  /* if this is an stp2220: */
  if (TME_STP222X_IS_2220(stp222x)) {

    /* only the upper half of our UPA address space is mapped to the
       SBus address space: */
    region_size_m1 = 0xffffffff;
    aspace_i
      = (address <= region_size_m1
	 ? TME_STP222X_ASPACE_NULL
	 : TME_STP2220_ASPACE_SBUS);
  }

  /* otherwise, this is an stp2222: */
  else {

    /* get bits 16..47 of the UPA address: */
    address_16_47 = (address >> 16);
    assert (address_16_47 == (address >> 16));

    /* if this is a PCI bus memory space: */
    if (address_16_47 >= 0x10000) {
      assert (address_16_47 <= 0x1ffff);
      region_size_m1 = 0x7fffffff;
      aspace_i
	= TME_STP2222_ASPACE_PCI_MEMORY(((address_16_47 & 0x8000) == 0)
					== !TME_STP2222_BOOT_BUS(stp222x));
    }

    /* otherwise, if this is a PCI bus I/O space: */
    else if ((address_16_47 | 1) == 0x00201) {
      region_size_m1 = 0xffff;
      aspace_i = TME_STP2222_ASPACE_PCI_IO(address_16_47 & 1);
    }

    /* otherwise, if this is PCI configuration space: */
    else if (address_16_47 == 0x00100) {
      region_size_m1 = 0xffffff;
      aspace_i = TME_STP2222_ASPACE_PCI_CONFIGURATION;
    }

    /* otherwise, this address is in the register space: */
    else {
      region_size_m1 = 0xffff;
      aspace_i = TME_STP222X_ASPACE_NULL;
    }
  }

  /* return the region size and address space: */
  *_region_size_m1 = region_size_m1;
  return (aspace_i);
}

/* the 0x00 register group register handler: */
static void
_tme_stp222x_reg00_regs(struct tme_stp222x *stp222x,
			struct tme_stp222x_reg *reg)
{
  
  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {
    
    /* dispatch on the register: */
    switch (TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address)) {
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UPA_PORT_CONFIG):
      stp222x->tme_stp222x_upa_port_config
	= (reg->tme_stp222x_reg_value
	   & TME_UPA_PORT_CONFIG_SCIQ0);
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CSR):
      stp222x->tme_stp222x_csr
	= ((stp222x->tme_stp222x_csr | ~TME_STP222X_CSR_APCKEN)
	   & (reg->tme_stp222x_reg_value ^ TME_STP222X_CSR_APCKEN));
      tme_stp222x_mdu_ign_update(stp222x,
				 TME_FIELD_MASK_EXTRACTU(stp222x->tme_stp222x_csr,
							 TME_STP222X_CSR_IGN));
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_ECC_CONTROL):
      stp222x->tme_stp222x_ecc_control = (reg->tme_stp222x_reg_value >> 61);
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UPA_PORT_ID):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UE_AFSR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UE_AFAR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CE_AFSR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CE_AFAR):
      break;
    default:
      return;
    }
  }

  /* otherwise, this is a read: */
  else {

    /* dispatch on the register: */
    switch (TME_STP222X_REGGROUP_INDEX(reg->tme_stp222x_reg_address)) {
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UPA_PORT_ID):
      reg->tme_stp222x_reg_value
	= (TME_UPA_PORT_ID_COOKIE
	   + !TME_UPA_PORT_ID_ECC_NOT_VALID
	   + !TME_UPA_PORT_ID_ONEREAD
	   + !TME_UPA_PORT_ID_PINT_RDQ
	   + (8 * _TME_FIELD_MASK_FACTOR(TME_UPA_PORT_ID_PREQ_DQ))
	   + (2 * _TME_FIELD_MASK_FACTOR(TME_UPA_PORT_ID_PREQ_RQ))
	   + ((TME_UPA_UPACAP_MASTER
	       + !TME_UPA_UPACAP_CACHEMASTER
	       + !TME_UPA_UPACAP_SLAVE_INT_L
	       + TME_UPA_UPACAP_INTERRUPTMASTER
	       + !TME_UPA_UPACAP_HANDLERSLAVE)
	      * _TME_FIELD_MASK_FACTOR(TME_UPA_PORT_ID_UPACAP))
	   + ((TME_STP222X_IS_2220(stp222x)
	       ? 0xef07
	       : 0x1954)
	      * _TME_FIELD_MASK_FACTOR(TME_UPA_PORT_ID_ID)));
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UPA_PORT_CONFIG):
      reg->tme_stp222x_reg_value = stp222x->tme_stp222x_upa_port_config;
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CSR):
      reg->tme_stp222x_reg_value
	= ((stp222x->tme_stp222x_csr
	    & (TME_STP222X_CSR_MID
	       | TME_STP222X_CSR_IGN
	       | TME_STP222X_CSR_APCKEN
	       | TME_STP222X_CSR_APERR
	       | TME_STP222X_CSR_IAP
	       | TME_STP222X_CSR_MODE))
	   + (TME_STP222X_IS_2220(stp222x)
	      ? ((0x0 * _TME_FIELD_MASK_FACTOR(TME_STP222X_CSR_IMPL))
		 + (0x1 * _TME_FIELD_MASK_FACTOR(TME_STP222X_CSR_VER)))
	      : ((0x0 * _TME_FIELD_MASK_FACTOR(TME_STP222X_CSR_IMPL))
		 + (0x0 * _TME_FIELD_MASK_FACTOR(TME_STP222X_CSR_VER)))));
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_ECC_CONTROL):
      reg->tme_stp222x_reg_value
	= (((tme_uint64_t) stp222x->tme_stp222x_ecc_control)
	   << 61);
      break;
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UE_AFSR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_UE_AFAR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CE_AFSR):
    case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG00_CE_AFAR):
      reg->tme_stp222x_reg_value = 0;
      break;
    default:
      return;
    }
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* the stp2220 SBus register handler: */
static void
_tme_stp2220_sbus_regs(struct tme_stp222x *stp222x,
		       struct tme_stp222x_reg *reg)
{
  tme_uint64_t mask_w;
  tme_uint64_t mask_ro;
  tme_uint64_t zero;
  tme_uint64_t *_value;

  /* assume that all bits of the register are writable: */
  mask_w = 0 - (tme_uint64_t) 1;

  /* assume that all read-only bits of the register are zero: */
  mask_ro = 0;

  /* make a dummy zero register: */
  zero = 0;

  /* if this is the SBus control/status register: */
  if (reg->tme_stp222x_reg_address == TME_STP2220_REG20_SBUS_CSR) {
    _value = &stp222x->tme_stp2220_sbus_csr;
    mask_w
      = (TME_BIT(10)		/* shortens PIO access latency */
	 + TME_BIT(8)		/* enables interrupts for SBus errors */
	 + TME_BIT(5)		/* enables DVMA for slot 15 */
	 + TME_BIT(4)		/* enables DVMA for slot 14 */
	 + 0xf);		/* enables DVMA for slots 0..3 */
  }

  /* otherwise, if this is the SBus AFSR or AFAR: */
  else if (reg->tme_stp222x_reg_address == TME_STP2220_REG20_SBUS_AFSR
	   || reg->tme_stp222x_reg_address == TME_STP2220_REG20_SBUS_AFAR) {
    _value = &zero;
  }

  /* otherwise, if this is an SBus slot configuration register: */
  else if ((reg->tme_stp222x_reg_address
	    >= TME_STP2220_REG20_SBUS_CONFIG(0))
	   && (reg->tme_stp222x_reg_address
	       <= TME_STP2220_REG20_SBUS_CONFIG(TME_STP2220_SLOTS_CARD + TME_STP2220_SLOTS_OBIO - 1))) {
    _value
      = (&stp222x->tme_stp2220_sbus_config_card
	 [(reg->tme_stp222x_reg_address
	   - TME_STP2220_REG20_SBUS_CONFIG(0))
	  / TME_STP222X_REG_SIZE]);
    mask_w
      = (TME_BIT(14)		/* enables extended transfers */
	 + TME_BIT(4)		/* enables 64-byte bursts */
	 + TME_BIT(3)		/* enables 32-byte bursts */
	 + TME_BIT(2)		/* enables 16-byte bursts */
	 + TME_BIT(1)		/* enables 8-byte bursts */
	 );
  }

  /* otherwise, this is an unknown register: */
  else {
    return;
  }

  /* if this is a write: */
  if (reg->tme_stp222x_reg_write) {
    *_value = reg->tme_stp222x_reg_value & mask_w;
  }
  
  /* otherwise, this is a read: */
  else {
    reg->tme_stp222x_reg_value = *_value | mask_ro;
  }

  /* this register access has been completed: */
  reg->tme_stp222x_reg_completed = TRUE;
}

/* this completes a bus operation between master and slave: */
#define _tme_stp222x_complete_master tme_stp22xx_complete_master

/* this completes a reset assertion or negation: */
static void
_tme_stp222x_complete_reset(struct tme_stp22xx *stp22xx,
			    struct tme_completion *completion,
			    void *arg)
{
  struct tme_stp222x *stp222x;
  tme_uint32_t reset_state;

  /* recover our data structure: */
  stp222x = (struct tme_stp222x *) stp22xx;

  /* get the next reset state: */
  reset_state = stp222x->tme_stp222x_reset_state - 1;

  /* if we have finished asserting RESET on the I/O bus, and
     the UPA bus reset signal is negated: */
  if (reset_state == TME_STP222X_RESET_STATE_ASSERTED
      && stp222x->tme_stp222x_reset_level == TME_BUS_SIGNAL_LEVEL_NEGATED) {

    /* start negating RESET on the I/O bus: */
    reset_state = TME_STP222X_RESET_STATE_NEGATING;
  }

  /* update the reset state: */
  stp222x->tme_stp222x_reset_state = reset_state;

  /* unused: */
  completion = 0;
  arg = 0;
}

/* this completes a bus request assertion or negation: */
static void
_tme_stp222x_complete_br(struct tme_stp22xx *stp22xx,
			 struct tme_completion *completion,
			 void *arg)
{
  struct tme_stp222x *stp222x;

  /* recover our data structure: */
  stp222x = (struct tme_stp222x *) stp22xx;

  /* our bus request has been flipped: */
  stp222x->tme_stp222x_br = !stp222x->tme_stp222x_br;

  /* unused: */
  completion = 0;
  arg = 0;
}

/* this completes a bus grant: */
#define _tme_stp222x_complete_bg tme_stp22xx_complete_bg

/* this calls out a bus signal to a connection: */
#define _tme_stp222x_callout_signal(stp222x, conn_index, signal, completion_handler) \
  tme_stp22xx_callout_signal(&(stp222x)->tme_stp222x, conn_index, signal, completion_handler)

/* the run function: */
static void
_tme_stp222x_run(struct tme_stp22xx *stp22xx)
{
  struct tme_stp222x *stp222x;
  unsigned int reset_state;
  unsigned int io_conn_index;
  unsigned int master_conn_index;
  tme_uint32_t io_brs;

  /* recover our data structure: */
  stp222x = (struct tme_stp222x *) stp22xx;

  /* loop forever: */
  for (;;) {

    /* if we need to assert reset to another I/O connection: */
    reset_state = stp222x->tme_stp222x_reset_state;
    if (reset_state > TME_STP222X_RESET_STATE_ASSERTED) {

      /* assert reset to the next I/O connection: */
      io_conn_index = reset_state - (TME_STP222X_RESET_STATE_ASSERTED + 1);
      _tme_stp222x_callout_signal(stp222x,
				  io_conn_index,
				  (TME_BUS_SIGNAL_RESET
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_ASSERTED),
				  _tme_stp222x_complete_reset);
      continue;
    }

    /* if there is a current master: */
    master_conn_index = stp222x->tme_stp222x_master_conn_index;
    if (master_conn_index != TME_STP222X_CONN_NULL) {
      assert (master_conn_index < TME_STP222X_CONN_SLAVE0);

      /* if the current master is still requesting the bus: */
      if (stp222x->tme_stp222x_io_brs & (1 << master_conn_index)) { 

	/* stop now.  we can't do anything else until the current
	   master releases the bus: */
	break;
      }

      /* there should be no master completion pending: */
      assert (stp222x->tme_stp222x.tme_stp22xx_master_completion == NULL);

      /* there is no current master: */
      stp222x->tme_stp222x_master_conn_index = TME_STP222X_CONN_NULL;

      /* negate bus grant to the former master: */
      _tme_stp222x_callout_signal(stp222x,
				  master_conn_index,
				  (TME_BUS_SIGNAL_BG
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_NEGATED),
				  tme_stp22xx_complete_nop);
      continue;
    }

    /* we need the UPA bus if an I/O connection is requesting the bus,
       or if we have an interrupt to dispatch, or if we have a
       streaming cache to flush.  if our UPA bus request line doesn't
       reflect whether we need the UPA bus: */
    if (!stp222x->tme_stp222x_br
	== (stp222x->tme_stp222x_io_brs != 0
#if TME_STP222X_MDU_BUFFER_COUNT != 2
#error "TME_STP222X_MDU_BUFFER_COUNT changed"
#endif
	    || (stp222x->tme_stp222x_mdu_dispatch_imr[0] != 0 /* !TME_STP222X_MDU_IMR_V */
		&& stp222x->tme_stp222x_mdu_dispatch_state[0] == 0) /* TME_STP222X_MDU_DISPATCH_NOW */
	    || (stp222x->tme_stp222x_mdu_dispatch_imr[1] != 0 /* !TME_STP222X_MDU_IMR_V */
		&& stp222x->tme_stp222x_mdu_dispatch_state[1] == 0) /* TME_STP222X_MDU_DISPATCH_NOW */
#if TME_STP222X_STC_COUNT != 2
#error "TME_STP222X_STC_COUNT changed"
#endif
	    || stp222x->tme_stp222x_stcs[0].tme_stp222x_stc_pgflush
	    || stp222x->tme_stp222x_stcs[1].tme_stp222x_stc_pgflush
	    )) {

      /* assert or negate our bus request on the UPA bus: */
      _tme_stp222x_callout_signal(stp222x,
				  TME_STP222X_CONN_UPA,
				  (TME_BUS_SIGNAL_BR
				   | TME_BUS_SIGNAL_EDGE
				   | (stp222x->tme_stp222x_br
				      ? TME_BUS_SIGNAL_LEVEL_NEGATED
				      : TME_BUS_SIGNAL_LEVEL_ASSERTED)),
				  _tme_stp222x_complete_br);
      continue;
    }

    /* if we need to negate reset to another I/O connection: */
    reset_state = stp222x->tme_stp222x_reset_state;
    if (reset_state > TME_STP222X_RESET_STATE_NEGATED
	&& reset_state < TME_STP222X_RESET_STATE_ASSERTED) {

      /* negate reset to the next I/O connection: */
      io_conn_index = reset_state - (TME_STP222X_RESET_STATE_NEGATED + 1);
      _tme_stp222x_callout_signal(stp222x,
				  io_conn_index,
				  (TME_BUS_SIGNAL_RESET
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_NEGATED),
				  _tme_stp222x_complete_reset);
      continue;
    }

    /* if we don't own the UPA bus: */
    if (!stp222x->tme_stp222x_bg) {

      /* stop now.  we can't do anything else until we own the UPA
	 bus: */
      break;
    }

    /* dispatch any interrupt: */
    if (tme_stp222x_mdu_dispatch(stp222x)) {
      continue;
    }

    /* flush any streaming cache: */
    if (tme_stp222x_stc_flush(stp222x)) {
      continue;
    }

    /* if one or more I/O connections are requesting the bus: */
    io_brs = stp222x->tme_stp222x_io_brs;
    if (io_brs != 0) {

      /* get the connection index for the next master: */
      /* XXX FIXME - should we do something fair here? */
      for (master_conn_index = 0;
	   (io_brs & 1) == 0;
	   io_brs >>= 1, master_conn_index++);

      /* set the pending master: */
      stp222x->tme_stp222x_master_conn_index_pending = master_conn_index;

      /* assert bus grant to the current master: */
      _tme_stp222x_callout_signal(stp222x,
				  master_conn_index,
				  (TME_BUS_SIGNAL_BG
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_ASSERTED),
				  _tme_stp222x_complete_bg);
      continue;
    }

    /* no other callouts are needed: */
    break;
  }
}

/* this handles a bus signal: */
static void
_tme_stp222x_signal(struct tme_bus_connection *conn_bus,
		    unsigned int signal,
		    struct tme_completion *completion)
{
  struct tme_stp222x *stp222x;
  tme_uint32_t level;
  tme_uint32_t io_conn_index;
  tme_uint32_t br_mask;
  tme_uint32_t idi;

  /* enter: */
  stp222x = tme_stp222x_enter_bus(conn_bus);

  /* get the level.  this must be an edge: */
  assert (signal & TME_BUS_SIGNAL_EDGE);
  level = signal - TME_BUS_SIGNAL_EDGE;
  signal = TME_BUS_SIGNAL_WHICH(signal);
  level ^= signal;

  /* if this bus signal is on the UPA connection: */
  if (conn_bus == stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_bus) {

    /* if this is the bus grant signal: */
    if (signal == TME_BUS_SIGNAL_BG) {

      /* mark our bus grant as either asserted or negated: */
      assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED
	      || level == TME_BUS_SIGNAL_LEVEL_ASSERTED);
      stp222x->tme_stp222x_bg = (level == TME_BUS_SIGNAL_LEVEL_ASSERTED);
    }

    /* otherwise, this must be reset signal: */
    else {
      assert (signal == TME_BUS_SIGNAL_RESET);

      /* update the level of the UPA reset signal: */
      stp222x->tme_stp222x_reset_level = level;

      /* if the UPA reset signal is asserted: */
      if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {

	/* start asserting RESET on the I/O bus: */
	stp222x->tme_stp222x_reset_state = TME_STP222X_RESET_STATE_ASSERTING;
      }

      /* otherwise, the UPA reset signal must be negated: */
      else {
	assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED);

	/* if RESET is currently asserted on the I/O bus, start
	   negating it: */
	assert (stp222x->tme_stp222x_reset_state >= TME_STP222X_RESET_STATE_ASSERTED);
	if (stp222x->tme_stp222x_reset_state == TME_STP222X_RESET_STATE_ASSERTED) {
	  stp222x->tme_stp222x_reset_state = TME_STP222X_RESET_STATE_NEGATING;
	}
      }
    }
  }

  /* otherwise, this bus signal is on an I/O connection: */
  else {

    /* if this is the bus request signal: */
    if (signal == TME_BUS_SIGNAL_BR) {

      /* get this I/O connection's bus request mask: */
      io_conn_index = _tme_stp222x_io_conn_index(stp222x, conn_bus);
      assert (io_conn_index < TME_STP222X_CONN_SLAVE0);
      br_mask = (1 << io_conn_index);

      /* mark this caller's bus request line as either asserted or
	 negated: */
      assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED
	      || level == TME_BUS_SIGNAL_LEVEL_ASSERTED);
      stp222x->tme_stp222x_io_brs
	= ((stp222x->tme_stp222x_io_brs
	    | br_mask)
	   & (level == TME_BUS_SIGNAL_LEVEL_ASSERTED
	      ? 0
	      : ~br_mask));
    }

    /* otherwise, this must be an interrupt signal: */
    else {
      assert (signal == TME_BUS_SIGNAL_INT_UNSPEC
	      || TME_BUS_SIGNAL_IS_INT(signal));

      /* get the IDI: */
      idi = _tme_stp222x_io_idi(stp222x, conn_bus, signal);

      /* update the interrupt concentrator: */
      tme_stp222x_mdu_intcon(stp222x, idi, level);
    }
  }

  /* leave: */
  tme_stp222x_completion_validate(stp222x, completion);
  tme_stp222x_leave(stp222x);
}

/* this handles a bus cycle: */
static void
_tme_stp222x_cycle(struct tme_bus_connection *master_conn_bus,
		   struct tme_bus_cycle *master_cycle,
		   tme_uint32_t *_master_fast_cycle_types,
		   struct tme_completion *master_completion)
{
  struct tme_stp222x *stp222x;
  tme_bus_addr64_t slave_address;
  tme_uint32_t aspace_i;
  unsigned int slave_conn_index;
  tme_bus_addr32_t region_size_m1;
  struct tme_stp222x_reg reg;
  tme_uint32_t reggroup;

  /* enter: */
  stp222x = tme_stp222x_enter_master_bus(master_conn_bus);

  /* convert the master's address into an address space, slave
     connection index and address: */
  slave_address = master_cycle->tme_bus_cycle_address;
  aspace_i
    = _tme_stp222x_lookup_address(stp222x,
				  slave_address,
				  &region_size_m1);
  slave_address &= region_size_m1;
  slave_conn_index
    = (aspace_i == TME_STP222X_ASPACE_NULL
       ? TME_STP222X_CONN_NULL
       : tme_stp222x_aspace_lookup(stp222x,
				   aspace_i,
				   &slave_address));
  master_cycle->tme_bus_cycle_address = slave_address;

  /* dispatch on the address space: */
  switch (aspace_i) {

  case TME_STP2222_ASPACE_PCI_CONFIGURATION:
    assert (!TME_STP222X_IS_2220(stp222x));
#if 0
    master_cycle->tme_bus_cycle_type |= TME_PCI_CYCLE_CONFIGURATION;
#endif
    abort();

  case TME_STP2222_ASPACE_PCI_IO(0):
  case TME_STP2222_ASPACE_PCI_IO(1):
    assert (!TME_STP222X_IS_2220(stp222x));
#if 0
    master_cycle->tme_bus_cycle_type |= TME_PCI_CYCLE_IO;
#endif
    abort();

  case TME_STP2220_ASPACE_SBUS:
    assert (TME_STP222X_IS_2220(stp222x));
    break;

  default:
    assert (!TME_STP222X_IS_2220(stp222x));
    assert (aspace_i == TME_STP2222_ASPACE_PCI_MEMORY(0)
	    || aspace_i == TME_STP2222_ASPACE_PCI_MEMORY(1));
    abort();

  case TME_STP222X_ASPACE_NULL:

    /* if this isn't an aligned 64-bit access: */
    if (__tme_predict_false((slave_address % TME_STP222X_REG_SIZE) != 0
			    || master_cycle->tme_bus_cycle_size != TME_STP222X_REG_SIZE)) {
      abort();
    }

    /* NB: read the _tme_stp103x_bus_port() comment about bus routing
       information coming out of the CPU: */
    /* replace the bus cycle routing information: */
    assert (TME_STP222X_REG_SIZE == (1 << TME_BUS64_LOG2));
    master_cycle->tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS64_LOG2);
    master_cycle->tme_bus_cycle_lane_routing
      = (_tme_stp222x_bus_router_regs
	 - TME_BUS_ROUTER_INDEX(TME_BUS64_LOG2, TME_BUS64_LOG2, 0));

    /* set the register address, poisoning it if it's too big for the
       structure: */
    reg.tme_stp222x_reg_address = slave_address;
    if (reg.tme_stp222x_reg_address != slave_address) {
      reg.tme_stp222x_reg_address = 0xffff;
    }

    /* if this is a write: */
    reg.tme_stp222x_reg_write = (master_cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE);
    if (reg.tme_stp222x_reg_write) {

      /* get value being written: */
      tme_bus_cycle_xfer_memory(master_cycle,
				((tme_uint8_t *) &reg.tme_stp222x_reg_value) - slave_address,
				slave_address + sizeof(tme_uint64_t) - 1);
      reg.tme_stp222x_reg_value = tme_betoh_u64(reg.tme_stp222x_reg_value);
    }

    /* otherwise, this must be a read: */
    else if (__tme_predict_false(master_cycle->tme_bus_cycle_type != TME_BUS_CYCLE_READ)) {
      abort();
    }

    /* assume that this register access won't be completed: */
    reg.tme_stp222x_reg_completed = FALSE;

    /* dispatch on the register group: */
    reggroup = TME_STP222X_REGGROUP_WHICH(reg.tme_stp222x_reg_address);
    switch (reggroup) {

    case 0x00:
      _tme_stp222x_reg00_regs(stp222x, &reg);
      break;

    case 0x01:
      switch (TME_STP222X_REGGROUP_INDEX(reg.tme_stp222x_reg_address)) {
      case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG01_PM_CR):
	abort();
      case TME_STP222X_REGGROUP_INDEX(TME_STP222X_REG01_PM_COUNT):
	abort();
      default:
	break;
      }
      break;

    case 0x24:
    case 0x02:
      if (reggroup 
	  == (TME_STP222X_IS_2220(stp222x)
	      ? 0x24
	      : 0x02)) {
	tme_stp222x_iommu_regs(stp222x, &reg);
      }
      break;

    case 0x2c:
    case 0x0c:
    case 0x30:
    case 0x10:
    case 0x1a:
    case 0x60:
    case 0x80:
      tme_stp222x_mdu_regs_imr_retry(stp222x, &reg);
      break;

    case 0x20:
      if (TME_STP222X_IS_2220(stp222x)) {
	_tme_stp2220_sbus_regs(stp222x, &reg);
      }
      break;

    case 0x34:
    case 0x14:
    case 0x38:
    case 0x18:
      tme_stp222x_mdu_regs_clear(stp222x, &reg);
      break;

    case 0x3c:
    case 0x1c:
      if ((reggroup < 0x30) == !TME_STP222X_IS_2220(stp222x)) {
	tme_stp222x_timer_regs(stp222x, &reg);
      }
      break;

    case 0x28:
      tme_stp222x_stc_regs(stp222x, 0, &reg);
      break;

    case 0x40:
      abort();

    case 0x48:
      if (TME_STP222X_IS_2220(stp222x)) {
        tme_stp222x_mdu_regs_diag(stp222x, &reg);
      }
      else {
	tme_stp222x_stc_regs(stp222x, 1, &reg);
      }
      break;

    case 0xa0:
      abort();

    case 0x44:
    case 0x45:
    case 0x46:
    case 0xa4:
    case 0xa5:
    case 0xa6:
      if ((reggroup >= 0xa0) == !TME_STP222X_IS_2220(stp222x)) {
	tme_stp222x_iommu_regs_diag(stp222x, &reg);
      }
      break;

    case 0xa8:
      if (!TME_STP222X_IS_2220(stp222x)) {
	tme_stp222x_mdu_regs_diag(stp222x, &reg);
      }
      break;

    default:
      if (TME_STP222X_IS_2220(stp222x)
	  ? (reggroup >= 0x50
	     && reggroup <= 0x59)
	  : (reggroup >= 0xb0
	     && reggroup <= 0xb9)) {
	tme_stp222x_stc_regs_diag(stp222x, 0, &reg);
      }
      else if (!TME_STP222X_IS_2220(stp222x)
	       && reggroup >= 0xc0
	       && reggroup <= 0xc9) {
	tme_stp222x_stc_regs_diag(stp222x, 1, &reg);
      }
      break;
    }

    /* if this register access was not completed: */
    if (__tme_predict_false(!reg.tme_stp222x_reg_completed)) {

      tme_log(TME_STP222X_LOG_HANDLE(stp222x), 100, TME_OK,
	      (TME_STP222X_LOG_HANDLE(stp222x),
	       _("unknown[0x%04x] %s 0x%" TME_PRIx64),
	       reg.tme_stp222x_reg_address,
	       (reg.tme_stp222x_reg_write
		? "<-"
		: "->"),
	       reg.tme_stp222x_reg_value));
    }

    /* if this was a read: */
    if (!reg.tme_stp222x_reg_write) {

      /* return the value: */
      reg.tme_stp222x_reg_value = tme_htobe_u64(reg.tme_stp222x_reg_value);
      tme_bus_cycle_xfer_memory(master_cycle,
				((tme_uint8_t *) &reg.tme_stp222x_reg_value) - slave_address,
				slave_address + sizeof(tme_uint64_t) - 1);
    }

    /* complete the cycle: */
    master_completion->tme_completion_error = TME_OK;
    tme_stp222x_completion_validate(stp222x, master_completion);
    master_completion = NULL;
    *_master_fast_cycle_types = 0;
    break;
  }

  /* if we didn't complete the cycle ourselves: */
  if (master_completion != NULL) {

    /* start this cycle: */
    assert (stp222x->tme_stp222x_master_completion == NULL);
    stp222x->tme_stp222x_master_completion = &master_completion;

    /* run the slave bus cycle: */
    tme_stp22xx_slave_cycle(master_conn_bus,
			    slave_conn_index,
			    master_cycle,
			    _master_fast_cycle_types,
			    &master_completion);
  }
 
  /* leave: */
  tme_stp222x_leave(stp222x);
}

/* this receives an interrupt: */
static void
_tme_stp222x_interrupt(struct tme_upa_bus_connection *master_conn_upa,
		       tme_uint32_t slave_mid,
		       const tme_uint64_t *data,
		       struct tme_completion *master_completion)
{
  abort();
}

/* this fills a TLB entry: */
static void
_tme_stp222x_tlb_fill(struct tme_bus_connection *agent_conn_bus,
		      struct tme_bus_tlb *tlb,
		      tme_bus_addr_t agent_address_wider,
		      unsigned int cycle_type)
{
  struct tme_stp222x *stp222x;
  tme_bus_addr64_t slave_address;
  tme_uint32_t aspace_i;
  tme_uint32_t slave_conn_index;
  tme_bus_addr32_t region_size_m1;
  tme_bus_addr64_t agent_address;
  struct tme_bus_tlb tlb_mapping;

  /* enter: */
  stp222x = tme_stp222x_enter_bus(agent_conn_bus);

  /* convert the agent's address into an address space, slave
     connection index and address: */
  slave_address = agent_address_wider;
  aspace_i
    = _tme_stp222x_lookup_address(stp222x,
				  agent_address_wider,
				  &region_size_m1);
  slave_address &= region_size_m1;
  slave_conn_index
    = (aspace_i == TME_STP222X_ASPACE_NULL
       ? TME_STP222X_CONN_NULL
       : tme_stp222x_aspace_lookup(stp222x,
				   aspace_i,
				   &slave_address));

  /* fill this TLB entry: */
  tme_stp22xx_tlb_fill(agent_conn_bus,
		       tlb,
		       slave_conn_index,
		       slave_address,
		       cycle_type);

  /* leave: */
  tme_stp222x_leave(stp222x);

  /* map the filled TLB entry: */
  agent_address = ~ (tme_bus_addr64_t) region_size_m1;
  agent_address &= agent_address_wider;
  tlb_mapping.tme_bus_tlb_addr_first = agent_address;
  agent_address |= region_size_m1;
  tlb_mapping.tme_bus_tlb_addr_last = agent_address;
#if TME_STP22XX_BUS_TRANSITION
  tlb_mapping.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
#endif /* TME_STP22XX_BUS_TRANSITION */
  tme_bus_tlb_map(tlb, slave_address, &tlb_mapping, agent_address_wider);
}

#if TME_STP22XX_BUS_TRANSITION

/* this is the bus signal transition glue: */
static int
_tme_stp222x_signal_transition(struct tme_bus_connection *conn_bus,
			       unsigned int signal)
{
  struct tme_completion completion_buffer;
  tme_completion_init(&completion_buffer);
  _tme_stp222x_signal(conn_bus,
		      signal,
		      &completion_buffer);
  return (TME_OK);
}
#define _tme_stp222x_signal _tme_stp222x_signal_transition

/* this is the bus cycle transition glue: */
static int
_tme_stp222x_cycle_transition(void *_master_conn_bus,
			      struct tme_bus_cycle *master_cycle)
{
  struct tme_completion completion_buffer;
  struct tme_stp222x *stp222x;
  struct tme_bus_connection *master_conn_bus;
  tme_uint32_t master_fast_cycle_types;

  tme_completion_init(&completion_buffer);

  master_conn_bus = (struct tme_bus_connection *) _master_conn_bus;
  stp222x = (struct tme_stp222x *) master_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  (master_conn_bus == stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_bus
   ? _tme_stp222x_cycle
   : tme_stp222x_iommu_cycle)
    (master_conn_bus,
     master_cycle,
     &master_fast_cycle_types,
     &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

/* the bus TLB fill transition glue: */
static int
_tme_stp222x_tlb_fill_transition(struct tme_bus_connection *agent_conn_bus,
				 struct tme_bus_tlb *tlb,
				 tme_bus_addr_t agent_address_wider,
				 unsigned int cycle_type)
{
  struct tme_stp222x *stp222x;

  stp222x = (struct tme_stp222x *) agent_conn_bus->tme_bus_connection.tme_connection_element->tme_element_private;
  (agent_conn_bus == stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_bus
   ? _tme_stp222x_tlb_fill
   : tme_stp222x_iommu_tlb_fill)
    (agent_conn_bus,
     tlb,
     agent_address_wider,
     cycle_type);

  /* we always handle any slow cycles: */
  tlb->tme_bus_tlb_cycles_ok |= cycle_type;
  tlb->tme_bus_tlb_addr_offset = 0;
  tlb->tme_bus_tlb_addr_shift = 0;
  tlb->tme_bus_tlb_cycle = _tme_stp222x_cycle_transition;
  tlb->tme_bus_tlb_cycle_private = agent_conn_bus;
  assert (tlb->tme_bus_tlb_fault_handler_count == 0);

  return (TME_OK);
}
#define _tme_stp222x_tlb_fill _tme_stp222x_tlb_fill_transition
#define tme_stp222x_iommu_tlb_fill _tme_stp222x_tlb_fill_transition

#endif /* TME_STP22XX_BUS_TRANSITION */

/* the connection scorer: */
static int
_tme_stp222x_connection_score(struct tme_connection *conn,
			      unsigned int *_score)
{
  struct tme_bus_connection *conn_bus;
  struct tme_stp222x *stp222x;
  unsigned int score;
  struct tme_upa_bus_connection *conn_upa_other;
  struct tme_bus_connection *conn_bus_other;

  /* recover the bus connection: */
  conn_bus = (struct tme_bus_connection *) conn;

  /* enter: */
  stp222x = tme_stp222x_enter_bus(conn_bus);

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_upa_other = (struct tme_upa_bus_connection *) conn->tme_connection_other;
  conn_bus_other = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {
  
    /* this must be a UPA controller, and not another agent: */
  case TME_CONNECTION_BUS_UPA:
    if (conn_upa_other->tme_upa_bus_connection.tme_bus_tlb_set_add != NULL
	&& conn_upa_other->tme_upa_bus_interrupt != NULL) {
      score = 10;
    }
    break;

    /* this must be a bus device: */
  case TME_CONNECTION_BUS_GENERIC:
    if (conn_bus_other->tme_bus_tlb_set_add == NULL) {
      score = 1;
    }
    break;

  default: abort();
  }

  /* leave: */
  tme_stp222x_leave(stp222x);
  *_score = score;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_stp222x_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_upa_bus_connection *conn_upa;
  struct tme_bus_connection *conn_bus;
  struct tme_stp222x *stp222x;
  unsigned int conn_index;
  tme_uint32_t slot;
  tme_bus_addr32_t offset;
  int slaveonly;
  tme_uint32_t connid;
  unsigned int obio_i;

  /* ignore a half-connection: */
  if (state == TME_CONNECTION_HALF) {
    return (TME_OK);
  }

  /* recover the bus connection: */
  conn_upa = (struct tme_upa_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn;

  /* enter: */
  stp222x = tme_stp222x_enter_bus(conn_bus);

  /* if this is the UPA bus connection: */
  if (conn->tme_connection_type == TME_CONNECTION_BUS_UPA) {

    /* save the UPA bus connection: */
    stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_upa = conn_upa;
  }

  /* otherwise, this is a device bus connection: */
  else {

    /* if this is an stp2220: */
    if (TME_STP222X_IS_2220(stp222x)) {

      /* split the temporary connection id into the slot, offset, and
	 slaveonly flag: */
      offset = conn->tme_connection_id;
      slot = (offset / TME_STP2220_SLOT_SIZE);
      offset %= TME_STP2220_SLOT_SIZE;
      slaveonly
	= (slot > TME_STP2220_SLOT_CARD(TME_STP2220_SLOTS_CARD - 1)
	   && slot < TME_STP2220_SLOT_OBIO(0));
      if (slaveonly) {
	slot ^= TME_STP2220_SLOT_CARD(TME_STP2220_SLOTS_CARD);
      }

      /* if this is a card: */
      if (slot >= TME_STP2220_SLOT_CARD(0)
	  && slot <= TME_STP2220_SLOT_CARD(TME_STP2220_SLOTS_CARD - 1)) {

	/* start the card connection ID: */
	connid = TME_STP222X_CONNID_TYPE_CARD;
	TME_FIELD_MASK_DEPOSITU(connid,
				TME_STP2220_CONNID_CARD_WHICH,
				(slot - TME_STP2220_SLOT_CARD(0)));
	TME_FIELD_MASK_DEPOSITU(connid,
				TME_STP2220_CONNID_CARD_OFFSET,
				offset);

	/* if this is a slave connection: */
	if (slaveonly) {

	  /* this connection will use the next slave connection index,
	     which is an alternate connection index: */
	  conn_index = stp222x->tme_stp222x_slave_conn_index_next++;
	  connid |= TME_STP2220_CONNID_CARD_ALTERNATE;
	}

	else {

	  /* assume that this is the first master connection for this
	     card, which will use the card's primary master connection
	     index: */
	  conn_index = TME_FIELD_MASK_EXTRACTU(connid, TME_STP2220_CONNID_CARD_WHICH);

	  /* if this is not the first master connection for this card: */
	  if (stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != NULL) {

	    /* this connection will use an alternate master connection
	       index: */
	    connid |= TME_STP2220_CONNID_CARD_ALTERNATE;

	    /* starting from the first alternate master connection
	       index for this card and moving forward, search the
	       remaining master connection indices for a free one: */
	    /* NB: this means if you have an SBus card with a lot of
	       different masters on it, you should put it in the last
	       card slot, to save the preferred alternate master
	       connection indices for the other card slots: */
	    conn_index
	      = (TME_STP2220_SLOTS_CARD
		 + (connid & TME_STP2220_CONNID_CARD_WHICH));
	    for (; stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != NULL;) {
	      if (++conn_index == TME_STP222X_CONN_SLAVE0) {
		abort();
	      }
	    }
	  }
	}
      }

      /* otherwise, this is an obio device: */
      else {

	/* if this is the slave I/O slot, force slaveonly: */
	if (slot == TME_STP2220_SLOT_SLAVIO) {
	  slaveonly = TRUE;
	}

	/* if this is a slave connection: */
	if (slaveonly) {

	  /* this connection will use the next slave connection index: */
	  conn_index = stp222x->tme_stp222x_slave_conn_index_next++;
	}

	/* otherwise, this is a master connection: */
	else {

	  /* starting from the last master connection index and
	     moving backwards, search for a free one: */
	  conn_index = TME_STP222X_CONN_SLAVE0 - 1;
	  for (; stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != NULL;) {
	    if (conn_index-- == 0) {
	      abort();
	    }
	  }
	}

	/* assume that this is not a known obio connection, and make
	   the obio long connection id: */
	connid
	  = (TME_STP222X_CONNID_TYPE_OBIO
	     | TME_STP222X_CONNID_OBIO_TYPE_LONG);
	TME_FIELD_MASK_DEPOSITU(connid,
				TME_STP222X_CONNID_OBIO_LONG_WHICH,
				(slot - TME_STP2220_SLOT_OBIO(0)));
	TME_FIELD_MASK_DEPOSITU(connid,
				TME_STP222X_CONNID_OBIO_LONG_OFFSET,
				offset);

	/* if this is a known obio connection: */
	for (obio_i = 0; obio_i < TME_ARRAY_ELS(_tme_stp2220_obios); obio_i++) {
	  if (_tme_stp2220_obios[obio_i]._tme_stp2220_obio_slot == slot
	      && _tme_stp2220_obios[obio_i]._tme_stp2220_obio_offset == offset) {

	    /* make the obio short connection id: */
	    connid
	      = (TME_STP222X_CONNID_TYPE_OBIO
		 | TME_STP222X_CONNID_OBIO_TYPE_SHORT);
	    TME_FIELD_MASK_DEPOSITU(connid,
				    TME_STP222X_CONNID_OBIO_SHORT_IDI,
				    _tme_stp2220_obios[obio_i]._tme_stp2220_obio_idi);
	    TME_FIELD_MASK_DEPOSITU(connid,
				    TME_STP222X_CONNID_OBIO_SHORT_CONN_WHICH,
				    conn_index);
	    break;
	  }
	}
      }

      /* save the SBus offset for this connection: */
      assert (conn_index < TME_STP222X_CONN_NULL);
      stp222x->tme_stp2220_conn_offset[conn_index] = TME_STP2220_SBUS_ADDRESS(slot, offset);
    }

    /* otherwise, this is an stp222x: */
    else {
      abort();
    }

    /* update this connection's id: */
    conn->tme_connection_id = connid;
      
    /* add this connection to the list: */
    assert (conn_index < TME_STP222X_CONN_NULL);
    if (conn->tme_connection_type == TME_CONNECTION_BUS_GENERIC) {
      assert (TME_STP222X_IS_2220(stp222x));
      stp222x->tme_stp222x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus = conn_bus;
    }
    else {
      assert (FALSE);
    }

    /* rebuild the address spaces: */
    if (tme_stp222x_aspaces_rebuild(stp222x)) {
      abort();
    }
  }

  /* leave: */
  tme_stp222x_leave(stp222x);
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_stp222x_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes new connection sides: */
static int
_tme_stp222x_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  int rc;
  struct tme_stp222x *stp222x;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  tme_bus_addr_t slot_wider;
  tme_bus_addr_t offset_wider;
  int slaveonly;

  /* assume that we will succeed: */
  rc = TME_OK;

  /* recover our data structure: */
  stp222x = (struct tme_stp222x *) element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&stp222x->tme_stp222x.tme_stp22xx_mutex);

  /* if we have no arguments, this is the UPA connection: */
  if (args[1] == NULL) {

    /* if we already have the UPA connection: */
    if (stp222x->tme_stp222x.tme_stp22xx_conns[TME_STP222X_CONN_UPA].tme_stp22xx_conn_upa != NULL) {
      rc = EEXIST;
    }

    /* otherwise, we don't have a UPA connection yet: */
    else {

      /* create a UPA connection: */
      conn_upa = tme_new0(struct tme_upa_bus_connection, 1);
      conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_UPA;
      conn_upa->tme_upa_bus_interrupt = _tme_stp222x_interrupt;

      /* fill in the generic bus connection: */
      conn_bus = &conn_upa->tme_upa_bus_connection;
      conn_bus->tme_bus_signals_add = NULL;
      conn_bus->tme_bus_signal = _tme_stp222x_signal;
      conn_bus->tme_bus_intack = NULL;
      conn_bus->tme_bus_tlb_set_add = NULL;
      conn_bus->tme_bus_tlb_fill = _tme_stp222x_tlb_fill;

      /* fill in the generic connection: */
      conn = &conn_bus->tme_bus_connection;
      conn->tme_connection_score = _tme_stp222x_connection_score;
      conn->tme_connection_make = _tme_stp222x_connection_make;
      conn->tme_connection_break = _tme_stp222x_connection_break;

      /* add in this connection side possibility: */
      conn->tme_connection_next = *_conns;
      *_conns = conn;
    }
  }

  /* otherwise, if this is an stp2220 and we have "slot" and "offset"
     arguments, and an optional "slaveonly" argument, this is an SBus
     connection: */
  else if (TME_STP222X_IS_2220(stp222x)
	   && TME_ARG_IS(args[1], "slot")
	   && args[2] != NULL
	   && TME_ARG_IS(args[3], "offset")
	   && args[4] != NULL
	   && (((slaveonly = TME_ARG_IS(args[5], "slaveonly"))
		&& args[6] == NULL)
	       || args[5] == NULL)) {

    /* convert the slot and base offset: */
    slot_wider = tme_bus_addr_parse(args[2], TME_STP2220_SLOT_OBIO(TME_STP2220_SLOTS_OBIO));
    offset_wider = tme_bus_addr_parse(args[4], TME_STP2220_SLOT_SIZE);

    /* if this is a bad slot: */
    if ((slot_wider < TME_STP2220_SLOT_CARD(0)
	 || slot_wider > TME_STP2220_SLOT_CARD(TME_STP2220_SLOTS_CARD - 1))
	&& (slot_wider < TME_STP2220_SLOT_OBIO(0)
	    || slot_wider > TME_STP2220_SLOT_OBIO(TME_STP2220_SLOTS_OBIO - 1))) {
      tme_output_append_error(_output, 
			      "%s %s",
			      _("bad slot"),
			      args[2]);
      rc = EINVAL;
    }

    /* if this is a bad offset: */
    else if (offset_wider >= TME_STP2220_SLOT_SIZE) {
      tme_output_append_error(_output, 
			      "%s %s",
			      _("bad offset"),
			      args[4]);
      rc = EINVAL;
    }

    /* otherwise, the arguments are ok: */
    else {

      /* create a generic bus connection: */
      conn_bus = tme_new0(struct tme_bus_connection, 1);
      conn_bus->tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_GENERIC;

      /* fill in the generic bus connection: */
      conn_bus->tme_bus_signals_add = NULL;
      conn_bus->tme_bus_signal = _tme_stp222x_signal;
      conn_bus->tme_bus_intack = NULL;
      conn_bus->tme_bus_tlb_set_add = tme_stp22xx_tlb_set_add;
      conn_bus->tme_bus_tlb_fill = tme_stp222x_iommu_tlb_fill;

      /* fill in the generic connection: */
      /* NB: this connection's temporary id is the SBus address, with
	 the slot exclusive-ORed with the maximum number of cards if
	 the "slaveonly" flag was given: */
      conn = &conn_bus->tme_bus_connection;
      conn->tme_connection_id
	= TME_STP2220_SBUS_ADDRESS((slot_wider
				    ^ (slaveonly
				       ? TME_STP2220_SLOT_CARD(TME_STP2220_SLOTS_CARD)
				       : 0)),
				   offset_wider);
      conn->tme_connection_score = _tme_stp222x_connection_score;
      conn->tme_connection_make = _tme_stp222x_connection_make;
      conn->tme_connection_break = _tme_stp222x_connection_break;

      /* add in this connection side possibility: */
      conn->tme_connection_next = *_conns;
      *_conns = conn;
    }
  }

  /* otherwise, the arguments are unknown: */
  else {
    if (TME_STP222X_IS_2220(stp222x)) {
      tme_output_append_error(_output, 
			      "%s %s [ slot %s offset %s [ slaveonly ] ]",
			      _("usage:"),
			      args[0],
			      _("SLOT"),
			      _("OFFSET"));
    }
    rc = EINVAL;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&stp222x->tme_stp222x.tme_stp22xx_mutex);

  return (rc);
}

/* this creates a new stp222x element: */
static int
_tme_stp222x_new(struct tme_element *element,
		 const char * const *args,
		 const void *extra,
		 char **_output,
		 unsigned int is_2220)
{
  struct tme_stp222x *stp222x;
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

  /* start the stp222x structure: */
  stp222x = tme_new0(struct tme_stp222x, 1);
  stp222x->tme_stp222x.tme_stp22xx_element = element;
  stp222x->tme_stp222x.tme_stp22xx_run = _tme_stp222x_run;
  tme_stp22xx_init(&stp222x->tme_stp222x,
		   sizeof(struct tme_stp222x),
		   TME_STP222X_CONN_NULL);

  /* set the type: */
  stp222x->tme_stp222x_is_2220 = is_2220;
  if (TME_STP222X_IS_2220(stp222x) != is_2220) {
    tme_free(stp222x);
    return (ENXIO);
  }

  /* the UPA bus reset signal is initially negated: */
  stp222x->tme_stp222x_reset_level = TME_BUS_SIGNAL_LEVEL_NEGATED;

  /* initialize the CSR: */
  stp222x->tme_stp222x_csr
    = (0x1f * _TME_FIELD_MASK_FACTOR(TME_STP222X_CSR_MID));

  /* initialize the miscellaneous registers: */
  stp222x->tme_stp222x_upa_port_config = 1;

  /* initialize the connections: */
  stp222x->tme_stp222x_slave_conn_index_next = TME_STP222X_CONN_SLAVE0;

  /* initialize the timers: */
  tme_stp222x_timer_init(stp222x, &stp222x->tme_stp222x_timers[0]);
  tme_stp222x_timer_init(stp222x, &stp222x->tme_stp222x_timers[1]);

  /* initialize the MDU: */
  tme_stp222x_mdu_init(stp222x);

  /* initialize the IOMMU: */
  tme_stp222x_iommu_init(stp222x);

  /* initialize the streaming caches: */
  tme_stp222x_stc_init(&stp222x->tme_stp222x_stcs[0]);
  tme_stp222x_stc_init(&stp222x->tme_stp222x_stcs[1]);

  /* fill the element: */
  element->tme_element_private = stp222x;
  element->tme_element_connections_new = _tme_stp222x_connections_new;

  return (TME_OK);
}

/* this creates a new stp2220 element: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,stp22xx,stp2220) {
  return (_tme_stp222x_new(element, args, extra, _output, TRUE));
}
