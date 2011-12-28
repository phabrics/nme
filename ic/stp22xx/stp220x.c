/* $Id: stp220x.c,v 1.5 2010/06/05 18:59:29 fredette Exp $ */

/* ic/stp220x.c - emulation of the uniprocessor system controller
   (STP2200) and the dual-processor system controller (STP2202): */

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
_TME_RCSID("$Id: stp220x.c,v 1.5 2010/06/05 18:59:29 fredette Exp $");

/* includes: */
#include "stp22xx-impl.h"

/* macros: */

/* MIDs: */
#define TME_STP220X_MID_CPU(n)			(n)
#define TME_STP220X_MID_IO			(31)

/* UPA ports: */
#define TME_STP220X_UPA_PORT_CPU(n)		(n)
#define TME_STP220X_UPA_PORT_IO			(2)
#define TME_STP220X_UPA_PORT_FFB		(3)
#define TME_STP220X_UPA_PORT_COUNT_MAX		(4)

/* the size of a UPA port: */
#define TME_STP220X_UPA_PORT_SIZE		(((tme_uint64_t) 1) << 33)

/* the number of SIMM groups: */
#define TME_STP220X_SIMM_GROUP_COUNT		(4)

/* registers: */
#define TME_STP220X_SC_CONTROL			(0x00)
#define TME_STP220X_SC_ID			(0x04)
#define TME_STP220X_SC_PERF(n)			(0x08 + (4 * (n)))
#define TME_STP220X_SC_PERFSHADOW		(0x10)
#define TME_STP220X_SC_PERF_CTRL		(0x20)
#define TME_STP220X_SC_DEBUG_PIN_CTRL		(0x30)
#define TME_STP220X_SC_UPA_CONFIG(port)		(0x40 + (8 * (port)))
#define TME_STP220X_SC_UPA_STATUS(port)		(0x44 + (8 * (port)))
#define TME_STP2200_SC_MC_CONTROL0		(0x60)
#define TME_STP2200_SC_MC_CONTROL1		(0x64)
#define TME_STP2202_SC_CC_DIAG			(0x70)
#define TME_STP2202_SC_CC_SNPVEC		(0x74)
#define TME_STP2202_SC_CC_FAULT			(0x78)
#define TME_STP2202_SC_CC_PROC_INDEX		(0x7c)
#define TME_STP2202_SC_MEMCTRL(n)		(0x80 + (4 * (n)))

/* this converts an SC UPA port config or status register address into
   a UPA port: */
#define TME_STP220X_SC_ADDRESS_UPA_PORT(address)	\
  (((address)						\
    - TME_STP220X_SC_UPA_CONFIG(0))			\
   / (TME_STP220X_SC_UPA_CONFIG(1)			\
      - TME_STP220X_SC_UPA_CONFIG(0)))

/* the control register: */
#define TME_STP220X_SC_CONTROL_POR		(((tme_uint32_t) 1) << 31)
#define TME_STP220X_SC_CONTROL_SOFT_POR		TME_BIT(30)
#define TME_STP220X_SC_CONTROL_SOFT_XIR		TME_BIT(29)
#define TME_STP220X_SC_CONTROL_B_POR		TME_BIT(28)
#define TME_STP220X_SC_CONTROL_B_XIR		TME_BIT(27)
#define TME_STP220X_SC_CONTROL_WAKEUP		TME_BIT(26)
#define TME_STP220X_SC_CONTROL_FATAL		TME_BIT(25)
#define TME_STP220X_SC_CONTROL_IAP		TME_BIT(23)
#define TME_STP220X_SC_CONTROL_EN_WKUP_POR	TME_BIT(22)
#define TME_STP220X_SC_CONTROL_MBZ	\
  (TME_BIT(24)				\
   | ((2 << 21) - (1 << 0)))

/* a UPA port configuration register: */
#define TME_STP220X_UPA_CONFIG_MD		(((tme_uint32_t) 1) << 31)
#define TME_STP220X_UPA_CONFIG_S_SLEEP		TME_BIT(30)
#define TME_STP220X_UPA_CONFIG_SPRQS		((2 << 27) - (1 << 24))
#define TME_STP220X_UPA_CONFIG_SPDQS		((2 << 23) - (1 << 18))
#define TME_STP220X_UPA_CONFIG_SPIQS		((2 << 17) - (1 << 16))
#define TME_STP220X_UPA_CONFIG_SQUEN		TME_BIT(15)
#define TME_STP220X_UPA_CONFIG_ONEREAD		TME_BIT(14)
#define TME_STP220X_UPA_CONFIG_MBZ		\
  (((2 << 29) - (1 << 28))			\
   | ((2 << 13) - (1 << 0)))

/* a UPA port status register: */
#define TME_STP220X_UPA_STATUS_FATAL		(((tme_uint32_t) 1) << 31)
#define TME_STP220X_UPA_STATUS_IADDR		TME_BIT(30)
#define TME_STP220X_UPA_STATUS_IPORT		TME_BIT(29)
#define TME_STP220X_UPA_STATUS_IPRTY		TME_BIT(28)
#define TME_STP220X_UPA_STATUS_MC0OF		TME_BIT(27)
#define TME_STP220X_UPA_STATUS_MC1OF		TME_BIT(26)
#define TME_STP220X_UPA_STATUS_MC0Q		((2 << 25) - (1 << 23))
#define TME_STP220X_UPA_STATUS_MC1Q		((2 << 22) - (1 << 20))
#define TME_STP220X_UPA_STATUS_MC1Q_0		TME_BIT(19)

/* this gives the log2 of the SIMM group size: */
#define TME_STP220X_SIMM_GROUP_SIZE_LOG2(stp220x) (TME_STP220X_IS_2200(stp220x) ? 28 : 29)

/* connections: */
#define TME_STP220X_CONN_UPA_PORT(n)		(n)
#define TME_STP220X_CONN_SIMM_GROUP(n)		TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_COUNT_MAX + (n))
#define TME_STP220X_CONN_EBUS			TME_STP220X_CONN_SIMM_GROUP(TME_STP220X_SIMM_GROUP_COUNT)
#define TME_STP220X_CONN_COUNT			(TME_STP220X_CONN_EBUS + 1)
#define TME_STP220X_CONN_NULL			(TME_STP220X_CONN_COUNT)

/* reset states: */
#define TME_STP220X_RESET_STATE_NEGATING	(TME_STP220X_UPA_PORT_COUNT_MAX)
#define TME_STP220X_RESET_STATE_ASSERTING	(TME_STP220X_UPA_PORT_COUNT_MAX * 2)

/* predicates: */
#define TME_STP220X_IS_2200(stp220x)		((stp220x)->tme_stp220x_is_2200)

/* types: */

/* the device: */
struct tme_stp220x {

  /* the common structure, and space for the connections: */
  struct tme_stp22xx tme_stp220x;
#define tme_stp220x_master_conn_index_pending tme_stp220x.tme_stp22xx_master_conn_index_pending
#define tme_stp220x_master_conn_index tme_stp220x.tme_stp22xx_master_conn_index
#define tme_stp220x_master_completion tme_stp220x.tme_stp22xx_master_completion
  union tme_stp22xx_conn __tme_stp220x_conns[TME_STP220X_CONN_COUNT];

  /* the last address in each connection: */
  tme_bus_addr64_t tme_stp220x_conn_address_last[TME_STP220X_CONN_COUNT];

  /* this is nonzero if this is an STP2200: */
  int tme_stp220x_is_2200;

  /* the reset state: */
  unsigned int tme_stp220x_reset_state;

  /* the reset bus signal: */
  tme_uint32_t tme_stp220x_reset_signal;

  /* the mask of connection indices requesting the bus: */
  tme_uint32_t tme_stp220x_brs;

  /* the ebus EB_ADDR and EB_DATA registers: */
  unsigned int tme_stp220x_eb_addr;
  tme_uint8_t tme_stp220x_eb_data[sizeof(tme_uint32_t)];

  /* the registers: */
  tme_uint32_t tme_stp220x_sc_control;
  tme_uint32_t tme_stp220x_sc_perf[2];
  tme_uint32_t tme_stp220x_sc_perfshadow;
  tme_uint32_t tme_stp220x_sc_perf_ctrl;
  tme_uint32_t tme_stp220x_sc_upa_config[TME_STP220X_UPA_PORT_COUNT_MAX];
  tme_uint32_t tme_stp220x_sc_upa_status[TME_STP220X_UPA_PORT_COUNT_MAX];
  tme_uint32_t tme_stp2202_sc_cc_diag;
  tme_uint32_t tme_stp2202_sc_cc_fault;
  tme_uint32_t tme_stp2202_sc_cc_proc_index;
  tme_uint32_t tme_stp220x_sc_memctrl[0xe];
#define tme_stp2200_sc_mc_control tme_stp220x_sc_memctrl
#define tme_stp2202_sc_memctrl tme_stp220x_sc_memctrl
};

/* globals: */

/* the ebus subregion, decoded by the PAL block of the RIC chip
   (STP2210), for the PLL chip (either an mc12429 or an mc12439): */
static const struct tme_bus_subregion _tme_stp2210_mc124x9_subregion = {

  /* the first and last addresses, starting from zero, of this
     subregion: */
  0x4000,
  0x4003,

  /* there are no other subregions for this bus connection: */
  (const struct tme_bus_subregion *) NULL
};

/* this converts a connection index into a MID: */
static unsigned int
_tme_stp220x_conn_index_mid(const struct tme_stp220x *stp220x,
			    unsigned int conn_index)
{
  switch (conn_index) {
  default: assert (FALSE);
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(0)):
    return (TME_STP220X_MID_CPU(0));
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(1)):
    assert (!TME_STP220X_IS_2200(stp220x));
    return (TME_STP220X_MID_CPU(1));
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_IO):
    return (TME_STP220X_MID_IO);
  }
}

/* this converts a MID into a connection index: */
static unsigned int
_tme_stp220x_lookup_mid(const struct tme_stp220x *stp220x,
			tme_uint32_t mid)
{
  switch (mid) {
  case TME_STP220X_MID_CPU(0):
    return (TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(0)));
  case TME_STP220X_MID_CPU(1):
    return (TME_STP220X_IS_2200(stp220x)
	    ? TME_STP220X_CONN_NULL
	    : TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(1)));
  case TME_STP220X_MID_IO:
    return (TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_IO));
  default:
    return (TME_STP220X_CONN_NULL);
  }
}

/* this converts an address into a connection index and a
   connection-relative address: */
static unsigned int
_tme_stp220x_lookup_address(const struct tme_stp220x *stp220x,
			    tme_bus_addr64_t *_address,
			    tme_bus_addr64_t *_region_size_m1)
{
  tme_uint32_t address_32_64;
  tme_uint32_t address_0_31;
  unsigned int group_size_log2;
  unsigned int group;
  tme_uint32_t group_size_m1;
  unsigned int conn_index;
  tme_uint32_t upn;

  /* get bits 32..64 of the address: */
  address_32_64 = (*_address >> 32);

  /* if this address is in memory: */
  if (address_32_64 < 0x100) {

    /* get bits 0..31 of the address: */
    address_0_31 = *_address;

    /* get the SIMM group number: */
    group_size_log2 = TME_STP220X_SIMM_GROUP_SIZE_LOG2(stp220x);
    group = (address_0_31 >> group_size_log2) % TME_STP220X_SIMM_GROUP_COUNT;

    /* get the connection index for this SIMM group: */
    conn_index = TME_STP220X_CONN_SIMM_GROUP(group);

    /* get the size, minus one, of the SIMM group: */
    group_size_m1 = stp220x->tme_stp220x_conn_address_last[conn_index];

    /* return the truncated address within the SIMM group, the size of
       the SIMM group, and the connection index: */
    *_address = (address_0_31 & group_size_m1);
    *_region_size_m1 = group_size_m1;
    return (conn_index);
  }

  /* make the region size the UPA port size: */
  *_region_size_m1 = (TME_STP220X_UPA_PORT_SIZE - 1);

  /* if this address is in a UPA port: */
  if (__tme_predict_true(address_32_64 >= 0x1c0)) {

    /* make the address within the UPA port: */
    *_address %= TME_STP220X_UPA_PORT_SIZE;

    /* get the UPN: */
    upn = (address_32_64 - 0x1c0) >> 1;

    /* UPN 30 is the FFB: */
    if (upn == 30) {
      return (TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_FFB));
    }

    /* all other UPNs are either valid MIDs or are reserved: */
    return (_tme_stp220x_lookup_mid(stp220x, upn));
  }

  /* this address is in a reserved region: */
  return (TME_STP220X_CONN_NULL);
}

/* this busies an agent generic bus connection: */
#define _tme_stp220x_agent_busy_bus(stp220x, agent_conn_index) \
  tme_stp22xx_busy_bus(&(stp220x)->tme_stp220x, agent_conn_index)

/* this unbusies an agent generic bus connection: */
#define _tme_stp220x_agent_unbusy_bus(stp220x, agent_conn_bus) \
  tme_stp22xx_unbusy_bus(&(stp220x)->tme_stp220x, agent_conn_bus)

/* this busies a slave generic bus connection: */
#define _tme_stp220x_slave_busy_bus(stp220x, slave_conn_index) \
  tme_stp22xx_slave_busy_bus(&(stp220x)->tme_stp220x, slave_conn_index)

/* this unbusies a slave connection: */
#define _tme_stp220x_slave_unbusy(stp220x) \
  tme_stp22xx_slave_unbusy(&(stp220x)->tme_stp220x)

/* this busies a slave UPA connection: */
static struct tme_upa_bus_connection *
_tme_stp220x_slave_busy_upa(struct tme_stp220x *stp220x,
			    unsigned int slave_conn_index)
{
  struct tme_upa_bus_connection *slave_conn_upa;
  struct tme_bus_connection *slave_conn_bus;

  /* if the connection index is valid: */
  slave_conn_upa = NULL;
  if (__tme_predict_true(slave_conn_index != TME_STP220X_CONN_NULL)) {

    /* if there is a UPA slave at this connection index: */
    slave_conn_upa = stp220x->tme_stp220x.tme_stp22xx_conns[slave_conn_index].tme_stp22xx_conn_upa;
    if (__tme_predict_true(slave_conn_upa != NULL
			   && slave_conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_type == TME_CONNECTION_BUS_UPA)) {

      /* busy the connection to the UPA slave: */
      slave_conn_bus = _tme_stp220x_slave_busy_bus(stp220x, slave_conn_index);
      assert (slave_conn_bus == &slave_conn_upa->tme_upa_bus_connection);
      return ((struct tme_upa_bus_connection *) slave_conn_bus);
    }
  }

  return (NULL);
}

/* this enters an stp220x: */
#define _tme_stp220x_enter_bus(agent_conn_bus) \
  ((struct tme_stp220x *) tme_stp22xx_enter((struct tme_stp22xx *) (agent_conn_bus)->tme_bus_connection.tme_connection_element->tme_element_private))

/* these enter an stp220x as the bus master: */
#define _tme_stp220x_enter_master_bus(agent_conn_bus) \
  ((struct tme_stp220x *) tme_stp22xx_enter_master(agent_conn_bus))
#define _tme_stp220x_enter_master_upa(agent_conn_upa) \
  _tme_stp220x_enter_master_bus(&(agent_conn_upa)->tme_upa_bus_connection)

/* this leaves an stp220x: */
#define _tme_stp220x_leave(stp220x) \
  tme_stp22xx_leave(&(stp220x)->tme_stp220x)

/* this validates an agent's completion: */
/* NB: agent_completion may be NULL: */
#define _tme_stp220x_completion_validate(stp220x, completion) \
  tme_stp22xx_completion_validate(&(stp220x)->tme_stp220x, completion)

/* this allocates a completion: */
#define _tme_stp220x_completion_alloc(stp220x, handler, arg) \
  tme_stp22xx_completion_alloc(&(stp220x)->tme_stp220x, handler, arg)

/* this completes a bus operation between master and slave: */
#define _tme_stp220x_complete_master tme_stp22xx_complete_master

/* this completes a reset assertion or negation: */
static void
_tme_stp220x_complete_reset(struct tme_stp22xx *stp22xx,
			    struct tme_completion *completion,
			    void *arg)
{
  struct tme_stp220x *stp220x;

  /* recover our data structure: */
  stp220x = (struct tme_stp220x *) stp22xx;

  /* update the reset state: */
  stp220x->tme_stp220x_reset_state--;

  /* unused: */
  completion = 0;
  arg = 0;
}

/* this completes a bus grant: */
#define _tme_stp220x_complete_bg tme_stp22xx_complete_bg

/* this calls out a bus signal to an agent: */
#define _tme_stp220x_callout_signal(stp220x, conn_index, signal, completion_handler) \
  tme_stp22xx_callout_signal(&(stp220x)->tme_stp220x, conn_index, signal, completion_handler)

/* the run function: */
static void
_tme_stp220x_run(struct tme_stp22xx *stp22xx)
{
  struct tme_stp220x *stp220x;
  unsigned int reset_state;
  unsigned int agent_conn_index;
  unsigned int master_conn_index;
  tme_uint32_t brs;

  /* recover our data structure: */
  stp220x = (struct tme_stp220x *) stp22xx;

  /* loop forever: */
  for (;;) {

    /* if we need to assert reset to another agent: */
    reset_state = stp220x->tme_stp220x_reset_state;
    if (reset_state > TME_STP220X_RESET_STATE_NEGATING) {

      /* assert reset to the next agent: */
      agent_conn_index = reset_state - (TME_STP220X_RESET_STATE_NEGATING + 1);
      _tme_stp220x_callout_signal(stp220x,
				  agent_conn_index,
				  (stp220x->tme_stp220x_reset_signal
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_ASSERTED),
				  _tme_stp220x_complete_reset);
      continue;
    }

    /* if there is a current master: */
    master_conn_index = stp220x->tme_stp220x_master_conn_index;
    if (master_conn_index != TME_STP220X_CONN_NULL) {

      /* if the current master is still requesting the bus: */
      if (stp220x->tme_stp220x_brs & (1 << master_conn_index)) { 

	/* stop now.  we can't do anything else until the current
	   master releases the bus: */
	break;
      }

      /* there is no current master: */
      stp220x->tme_stp220x_master_conn_index = TME_STP220X_CONN_NULL;

      /* negate bus grant to the former master: */
      _tme_stp220x_callout_signal(stp220x,
				  master_conn_index,
				  (TME_BUS_SIGNAL_BG
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_NEGATED),
				  tme_stp22xx_complete_nop);
      continue;
    }

    /* if we need to negate reset to another agent: */
    reset_state = stp220x->tme_stp220x_reset_state;
    if (reset_state) {

      /* negate reset to the next agent: */
      agent_conn_index = reset_state - 1;
      _tme_stp220x_callout_signal(stp220x,
				  agent_conn_index,
				  (stp220x->tme_stp220x_reset_signal
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_NEGATED),
				  _tme_stp220x_complete_reset);
      continue;
    }

    /* if one or more agents are requesting the bus: */
    brs = stp220x->tme_stp220x_brs;
    if (brs != 0) {

      /* get the connection index for the next master: */
      /* XXX FIXME - should we do something fair here? */
      for (master_conn_index = 0;
	   (brs & 1) == 0;
	   brs >>= 1, master_conn_index++);

      /* set the pending master: */
      stp220x->tme_stp220x_master_conn_index_pending = master_conn_index;

      /* assert bus grant to the current master: */
      _tme_stp220x_callout_signal(stp220x,
				  master_conn_index,
				  (TME_BUS_SIGNAL_BG
				   | TME_BUS_SIGNAL_EDGE
				   | TME_BUS_SIGNAL_LEVEL_ASSERTED),
				  _tme_stp220x_complete_bg);
      continue;
    }

    /* no other running is needed: */
    break;
  }
}

/* this does a reset: */
static void
_tme_stp220x_reset(struct tme_stp220x *stp220x,
		   tme_uint32_t resets)
{
  tme_uint32_t sc_control;
  tme_uint32_t reset;

  /* get the current status registers, so that the chosen reset may
     reset them: */
  sc_control = stp220x->tme_stp220x_sc_control;

  /* "If multiple resets occur simultaneously, the following order
     will be chosen for the reporting:" */

  if (resets & TME_STP220X_SC_CONTROL_POR) {
    sc_control = 0;
    reset = TME_STP220X_SC_CONTROL_POR;
  }
  else if (resets & TME_STP220X_SC_CONTROL_FATAL) {
    reset = TME_STP220X_SC_CONTROL_FATAL;
  }
  else if (resets & TME_STP220X_SC_CONTROL_B_POR) {
    sc_control = 0;
    reset = TME_STP220X_SC_CONTROL_B_POR;
  }
  else if (resets & TME_STP220X_SC_CONTROL_WAKEUP) {
    reset = TME_STP220X_SC_CONTROL_WAKEUP;
  }
  else if (resets & TME_STP220X_SC_CONTROL_SOFT_POR) {
    sc_control = 0;
    reset = TME_STP220X_SC_CONTROL_SOFT_POR;
  }
  else if (resets & TME_STP220X_SC_CONTROL_B_XIR) {
    reset = TME_STP220X_SC_CONTROL_B_XIR;
  }
  else {
    assert (resets == TME_STP220X_SC_CONTROL_SOFT_XIR);
    reset = TME_STP220X_SC_CONTROL_SOFT_XIR;
  }

  /* update the status registers: */
  stp220x->tme_stp220x_sc_control = sc_control | reset;

  /* if this is a type of power or fatal reset: */
  if (reset
      & (TME_STP220X_SC_CONTROL_FATAL
	 | TME_STP220X_SC_CONTROL_POR
	 | TME_STP220X_SC_CONTROL_B_POR
	 | TME_STP220X_SC_CONTROL_SOFT_POR)) {

    /* start asserting SYS_RESET_L: */
    stp220x->tme_stp220x_reset_state = TME_STP220X_RESET_STATE_ASSERTING;
    stp220x->tme_stp220x_reset_signal = TME_BUS_SIGNAL_RESET;
  }

  /* otherwise, if this is a wakeup: */
  else if (reset == TME_STP220X_SC_CONTROL_WAKEUP) {

    /* XXX WRITEME: */
    abort();
  }

  /* otherwise, this must be an XIR: */
  else {
    assert (reset == TME_STP220X_SC_CONTROL_B_XIR
	    || reset == TME_STP220X_SC_CONTROL_SOFT_XIR);

    /* start asserting UPA_XIR_L: */
    /* NB: we reuse TME_BUS_SIGNAL_HALT for this: */
    stp220x->tme_stp220x_reset_state = TME_STP220X_RESET_STATE_ASSERTING;
    stp220x->tme_stp220x_reset_signal = TME_BUS_SIGNAL_HALT;
  }
}

/* this handles a bus signal: */
static void
_tme_stp220x_signal(struct tme_bus_connection *agent_conn_bus,
		    unsigned int signal,
		    struct tme_completion *agent_completion)
{
  struct tme_stp220x *stp220x;
  tme_uint32_t level;
  tme_uint32_t agent_conn_index_mask;

  /* enter: */
  stp220x = _tme_stp220x_enter_bus(agent_conn_bus);

  /* get the level.  this must be an edge: */
  assert (signal & TME_BUS_SIGNAL_EDGE);
  level = signal - TME_BUS_SIGNAL_EDGE;
  signal = TME_BUS_SIGNAL_WHICH(signal);
  level ^= signal;

  /* get the agent's connection index mask: */
  agent_conn_index_mask = (1 << agent_conn_bus->tme_bus_connection.tme_connection_id);

  /* dispatch on the signal: */
  switch (signal) {

    /* the bus request signal: */
  case TME_BUS_SIGNAL_BR:

    /* mark this caller's bus request line as either asserted or
       negated: */
    assert (level == TME_BUS_SIGNAL_LEVEL_NEGATED
	    || level == TME_BUS_SIGNAL_LEVEL_ASSERTED);
    stp220x->tme_stp220x_brs 
      = ((stp220x->tme_stp220x_brs
	  | agent_conn_index_mask)
	 & (level == TME_BUS_SIGNAL_LEVEL_ASSERTED
	    ? ~0
	    : ~agent_conn_index_mask));
    break;

  default:
    assert (FALSE);
    break;
  }

  /* leave: */
  _tme_stp220x_completion_validate(stp220x, agent_completion);
  _tme_stp220x_leave(stp220x);
}

/* this handles a bus cycle: */
static void
_tme_stp220x_cycle(struct tme_bus_connection *master_conn_bus,
		   struct tme_bus_cycle *master_cycle,
		   tme_uint32_t *_master_fast_cycle_types,
		   struct tme_completion *master_completion)
{
  struct tme_stp220x *stp220x;
  tme_bus_addr64_t slave_address;
  unsigned int slave_conn_index;
  tme_bus_addr64_t region_size_m1;

  /* enter: */
  stp220x = _tme_stp220x_enter_master_bus(master_conn_bus);

  /* start this cycle: */
  assert (stp220x->tme_stp220x_master_completion == NULL);
  stp220x->tme_stp220x_master_completion = &master_completion;

  /* convert the master's address into a slave connection index and
     address: */
  slave_address = master_cycle->tme_bus_cycle_address;
  slave_conn_index = _tme_stp220x_lookup_address(stp220x, &slave_address, &region_size_m1);
  master_cycle->tme_bus_cycle_address = slave_address;

  /* run the slave bus cycle: */
  tme_stp22xx_slave_cycle(master_conn_bus,
			  slave_conn_index,
			  master_cycle,
			  _master_fast_cycle_types,
			  &master_completion);
 
  /* leave: */
  _tme_stp220x_leave(stp220x);
}

/* this delivers an interrupt: */
static void
_tme_stp220x_interrupt(struct tme_upa_bus_connection *master_conn_upa,
		       tme_uint32_t slave_mid,
		       const tme_uint64_t *data,
		       struct tme_completion *master_completion)
{
  struct tme_stp220x *stp220x;
  unsigned int slave_conn_index;
  struct tme_upa_bus_connection *slave_conn_upa;
  struct tme_completion *completion;
  struct tme_upa_bus_connection *slave_conn_upa_other;

  /* enter: */
  stp220x = _tme_stp220x_enter_master_upa(master_conn_upa);

  /* convert the MID into a connection index: */
  slave_conn_index = _tme_stp220x_lookup_mid(stp220x, slave_mid);

  /* busy the connection to the slave: */
  slave_conn_upa = _tme_stp220x_slave_busy_upa(stp220x, slave_conn_index);

  /* if this slave does not exist: */
  if (__tme_predict_false(slave_conn_upa == NULL)) {

    /* complete with an error: */
    master_completion->tme_completion_error = ENOENT;
    _tme_stp220x_completion_validate(stp220x, master_completion);
  }

  /* otherwise, if this master is trying to interrupt itself: */
  else if (__tme_predict_false(slave_conn_upa == master_conn_upa)) {

    /* unbusy the connection to the slave: */
    _tme_stp220x_slave_unbusy(stp220x);

    /* complete with an error: */
    master_completion->tme_completion_error = EIO;
    _tme_stp220x_completion_validate(stp220x, master_completion);
  }

  /* otherwise, we can deliver this interrupt: */
  else {

    /* start this cycle: */
    assert (stp220x->tme_stp220x_master_completion == NULL);
    stp220x->tme_stp220x_master_completion = &master_completion;

    /* when we complete, we will will complete for the master: */
    completion = _tme_stp220x_completion_alloc(stp220x,
					       _tme_stp220x_complete_master,
					       &master_completion);

    /* leave: */
    _tme_stp220x_leave(stp220x);

    /* deliver this interrupt: */
    slave_conn_upa_other = (struct tme_upa_bus_connection *) slave_conn_upa->tme_upa_bus_connection.tme_bus_connection.tme_connection_other;
    (*slave_conn_upa_other->tme_upa_bus_interrupt)
      (slave_conn_upa_other,
       master_conn_upa->tme_upa_bus_connection_mid,
       data,
       completion);

    /* reenter: */
    stp220x = _tme_stp220x_enter_bus(&master_conn_upa->tme_upa_bus_connection);
  }

  /* leave: */
  _tme_stp220x_leave(stp220x);
}

/* this fills a TLB entry: */
static void
_tme_stp220x_tlb_fill(struct tme_bus_connection *agent_conn_bus,
		      struct tme_bus_tlb *tlb,
		      tme_bus_addr_t agent_address_wider,
		      unsigned int cycle_type)
{
  struct tme_stp220x *stp220x;
  tme_bus_addr64_t slave_address;
  unsigned int slave_conn_index;
  tme_bus_addr64_t region_size_m1;
  tme_bus_addr64_t agent_address;
  struct tme_bus_tlb tlb_mapping;

  /* enter: */
  stp220x = _tme_stp220x_enter_bus(agent_conn_bus);

  /* convert the agent's address into a slave connection index and
     address: */
  slave_address = agent_address_wider;
  slave_conn_index = _tme_stp220x_lookup_address(stp220x, &slave_address, &region_size_m1);

  /* fill the TLB entry: */
  tme_stp22xx_tlb_fill(agent_conn_bus,
		       tlb,
		       slave_conn_index,
		       slave_address,
		       cycle_type);

  /* leave: */
  _tme_stp220x_leave(stp220x);

  /* map the filled TLB entry: */
  agent_address = ~region_size_m1;
  agent_address &= agent_address_wider;
  tlb_mapping.tme_bus_tlb_addr_first = agent_address;
  agent_address |= region_size_m1;
  tlb_mapping.tme_bus_tlb_addr_last = agent_address;
#if TME_STP22XX_BUS_TRANSITION
  tlb_mapping.tme_bus_tlb_cycles_ok = (TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE);
#endif /* TME_STP22XX_BUS_TRANSITION */
  tme_bus_tlb_map(tlb, slave_address, &tlb_mapping, agent_address_wider);
}

/* this handles an ebus cycle: */
static void
_tme_stp220x_ebus_cycle(struct tme_bus_connection *ebus_conn_bus,
			struct tme_bus_cycle *cycle,
			tme_uint32_t *_master_fast_cycle_types,
			struct tme_completion *completion)
{
  struct tme_stp220x *stp220x;
  unsigned int upa_port;
  tme_bus_addr32_t address;
  tme_uint8_t value8;
  tme_uint32_t value32;

  /* enter: */
  stp220x = _tme_stp220x_enter_bus(ebus_conn_bus);

  /* get the address: */
  address = cycle->tme_bus_cycle_address;

  /* get any UPA port number: */
  upa_port = TME_STP220X_SC_ADDRESS_UPA_PORT(stp220x->tme_stp220x_eb_addr);

  /* if this is a write: */
  if (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* get value being written: */
    tme_bus_cycle_xfer_memory(cycle,
			      &value8 - address,
			      address);

    /* if this is a write to EB_ADDR: */
    if (address < sizeof(tme_uint32_t)) {

      /* update EB_ADDR: */
      stp220x->tme_stp220x_eb_addr = value8;
    }

    /* otherwise, if this is a write to EB_DATA: */
    else if (address < (2 * sizeof(tme_uint32_t))) {

      /* update EB_DATA: */
      stp220x->tme_stp220x_eb_data[address % sizeof(tme_uint32_t)] = value8;

      /* if this was a write to EB_DATA bits 0..7: */
      if (address == ((sizeof(tme_uint32_t) * 2) - 1)) {

	/* get the EB_DATA value: */
	value32 = stp220x->tme_stp220x_eb_data[0];
	value32 = (value32 << 8) + stp220x->tme_stp220x_eb_data[1];
	value32 = (value32 << 8) + stp220x->tme_stp220x_eb_data[2];
	value32 = (value32 << 8) + stp220x->tme_stp220x_eb_data[3];

	/* dispatch on EB_ADDR: */
	switch (stp220x->tme_stp220x_eb_addr) {
	default:
	/* case TME_STP220X_SC_DEBUG_PIN_CTRL: */
	/* case TME_STP2202_SC_CC_SNPVEC: */
	  abort();
	  break;
	case TME_STP220X_SC_CONTROL:

	  /* clear any R/W1C bits: */
	  stp220x->tme_stp220x_sc_control
	    = (stp220x->tme_stp220x_sc_control
	       & ~(value32
		   & (TME_STP220X_SC_CONTROL_POR
		      | TME_STP220X_SC_CONTROL_B_POR
		      | TME_STP220X_SC_CONTROL_B_XIR
		      | TME_STP220X_SC_CONTROL_WAKEUP
		      | TME_STP220X_SC_CONTROL_FATAL)));

	  /* we don't support these bits: */
	  if (value32
	      & (TME_STP220X_SC_CONTROL_IAP
		 | TME_STP220X_SC_CONTROL_EN_WKUP_POR)) {
	    abort();
	  }

	  /* update the R/W bits: */
	  value32
	    = ((stp220x->tme_stp220x_sc_control
		^ value32)
	       & (TME_STP220X_SC_CONTROL_SOFT_POR
		  | TME_STP220X_SC_CONTROL_SOFT_XIR
		  ));
	  stp220x->tme_stp220x_sc_control ^= value32;

	  /* handle any soft resets: */
	  value32 &= stp220x->tme_stp220x_sc_control;
	  value32
	    &= (TME_STP220X_SC_CONTROL_SOFT_POR
		| TME_STP220X_SC_CONTROL_SOFT_XIR);
	  if (value32 != 0) {
	    _tme_stp220x_reset(stp220x, value32);
	  }
	  break;
	case TME_STP220X_SC_ID:
	case TME_STP220X_SC_PERF(0):
	case TME_STP220X_SC_PERF(1):
	case TME_STP220X_SC_PERFSHADOW:
	  break;
	case TME_STP220X_SC_PERF_CTRL:
	  if (value32 & (1 << 15)) {
	    stp220x->tme_stp220x_sc_perf[1] = 0;
	  }
	  if (value32 & (1 << 7)) {
	    stp220x->tme_stp220x_sc_perf[0] = 0;
	  }
	  stp220x->tme_stp220x_sc_perf_ctrl = value32;
	  break;
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_CPU(0)):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_CPU(1)):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_IO):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_FFB):
	  if (upa_port == TME_STP220X_UPA_PORT_CPU(0)
	      || upa_port == TME_STP220X_UPA_PORT_CPU(1)) {
	    value32 &= ~TME_STP220X_UPA_CONFIG_SPDQS;
	  }
	  else {
	    value32
	      &= ~(TME_STP220X_UPA_CONFIG_S_SLEEP
		   | TME_STP220X_UPA_CONFIG_SPIQS);
	  }
	  if (upa_port == TME_STP220X_UPA_PORT_FFB) {
	    value32
	      |= (TME_STP220X_UPA_CONFIG_MD
		  | TME_STP220X_UPA_CONFIG_ONEREAD);
	  }
	  if ((value32 & TME_STP220X_UPA_CONFIG_SQUEN) == 0) {
	    value32
	      = ((value32
		  & ~(TME_STP220X_UPA_CONFIG_SPRQS
		      | TME_STP220X_UPA_CONFIG_SPIQS))
		 | (stp220x->tme_stp220x_sc_upa_config[upa_port]
		    & (TME_STP220X_UPA_CONFIG_SPRQS
		       | TME_STP220X_UPA_CONFIG_SPIQS)));
	  }
	  value32
	    &= ~(TME_STP220X_UPA_CONFIG_SQUEN
		 | TME_STP220X_UPA_CONFIG_MBZ);
	  stp220x->tme_stp220x_sc_upa_config[upa_port] = value32;
	  break;
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_CPU(0)):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_CPU(1)):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_IO):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_FFB):
	  stp220x->tme_stp220x_sc_upa_status[upa_port]
	    &= ~(value32
		 & (TME_STP220X_UPA_STATUS_FATAL
		    | TME_STP220X_UPA_STATUS_IADDR
		    | TME_STP220X_UPA_STATUS_IPORT
		    | TME_STP220X_UPA_STATUS_IPRTY
		    | TME_STP220X_UPA_STATUS_MC0OF
		    | TME_STP220X_UPA_STATUS_MC1OF));
	  break;
	case TME_STP2200_SC_MC_CONTROL0:
	  stp220x->tme_stp2200_sc_mc_control[0]
	    = (value32
	       & ~((((tme_uint32_t) 2) << 30) - (1 << 16)));
	  break;
	case TME_STP2200_SC_MC_CONTROL1:
	  stp220x->tme_stp2200_sc_mc_control[1]
	    = (value32
	       & ~((((tme_uint32_t) 2) << 31) - (1 << 13)));
	  break;
	case TME_STP2202_SC_MEMCTRL(0x0):
	case TME_STP2202_SC_MEMCTRL(0x1):
	case TME_STP2202_SC_MEMCTRL(0x2):
	case TME_STP2202_SC_MEMCTRL(0x3):
	case TME_STP2202_SC_MEMCTRL(0x4):
	case TME_STP2202_SC_MEMCTRL(0x5):
	case TME_STP2202_SC_MEMCTRL(0x6):
	case TME_STP2202_SC_MEMCTRL(0x7):
	case TME_STP2202_SC_MEMCTRL(0x8):
	case TME_STP2202_SC_MEMCTRL(0x9):
	case TME_STP2202_SC_MEMCTRL(0xa):
	case TME_STP2202_SC_MEMCTRL(0xb):
	case TME_STP2202_SC_MEMCTRL(0xc):
	case TME_STP2202_SC_MEMCTRL(0xd):
	  stp220x->tme_stp2202_sc_memctrl
	    [(stp220x->tme_stp220x_eb_addr
	      - TME_STP2202_SC_MEMCTRL(0x0))
	     / (TME_STP2202_SC_MEMCTRL(0x1)
		- TME_STP2202_SC_MEMCTRL(0x0))]
	    = value32;
	  break;
	case TME_STP2202_SC_CC_DIAG:
	  stp220x->tme_stp2202_sc_cc_diag
	    = (value32
	       & ~((2 << 14) - (1 << 0)));
	  break;
	case TME_STP2202_SC_CC_FAULT:
	  stp220x->tme_stp2202_sc_cc_fault
	    = ((stp220x->tme_stp2202_sc_cc_fault
		| ((2 << 27) - (1 << 13)))	/* FLTIndex */
	       & (value32
		  ^ ((1 << 31)		/* PERR0 */
		     | (1 << 30)	/* CERR0 */
		     | (1 << 29)	/* PERR1 */
		     | (1 << 28))));	/* CERR1 */
	case TME_STP2202_SC_CC_PROC_INDEX:
	  stp220x->tme_stp2202_sc_cc_proc_index = value32;
	  break;
	}
      }
    }

    /* otherwise, this must be a write to the PLL chip: */
    else {
      assert (address >= _tme_stp2210_mc124x9_subregion.tme_bus_subregion_address_first
	      && address <= _tme_stp2210_mc124x9_subregion.tme_bus_subregion_address_last);

      /* we can ignore all writes to the PLL chip, except for a write
	 to the PLL serial load and reset address, which is the first
	 address: */
      if (address
	  == (tme_bus_addr32_t) _tme_stp2210_mc124x9_subregion.tme_bus_subregion_address_first) {

	/* when the PLL serial load and reset address is written, the
	   STP2210 asserts B_POR to the STP220x: */
	_tme_stp220x_reset(stp220x, TME_STP220X_SC_CONTROL_B_POR);
      }
    }
  }

  /* otherwise, this must be a read: */
  else {
    assert (cycle->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* if this is a read of EB_ADDR: */
    if (address < sizeof(tme_uint32_t)) {

      /* return EB_ADDR: */
      value8 = stp220x->tme_stp220x_eb_addr;
    }

    /* otherwise, if this is a read of EB_DATA: */
    else if (address < (2 * sizeof(tme_uint32_t))) {

      /* if this is a read of EB_DATA bits 24..31: */
      if (address == sizeof(tme_uint32_t)) {

	/* dispatch on EB_ADDR: */
	switch (stp220x->tme_stp220x_eb_addr) {
	default:
        /* case TME_STP220X_SC_DEBUG_PIN_CTRL: */
	/* case TME_STP2202_SC_CC_SNPVEC: */
	  abort();
	case TME_STP220X_SC_CONTROL:
	  value32 = stp220x->tme_stp220x_sc_control;
	  break;
	case TME_STP220X_SC_ID:
	  value32
	    = (((TME_STP220X_IS_2200(stp220x)
		 ? 0x3340
		 : 0xacf1) << 16)
	       + ((TME_STP220X_IS_2200(stp220x)
		   ? 3
		   : 4) << 12)
	       + (1 << 4)
	       + (1 << 0));
	  break;
	case TME_STP220X_SC_PERF(0):
	  value32 = stp220x->tme_stp220x_sc_perf[0];
	  stp220x->tme_stp220x_sc_perfshadow = stp220x->tme_stp220x_sc_perf[1];
	  break;
	case TME_STP220X_SC_PERF(1):
	  value32 = stp220x->tme_stp220x_sc_perf[1];
	  stp220x->tme_stp220x_sc_perfshadow = stp220x->tme_stp220x_sc_perf[0];
	  break;
	case TME_STP220X_SC_PERFSHADOW:
	  value32 = stp220x->tme_stp220x_sc_perfshadow;
	  break;
	case TME_STP220X_SC_PERF_CTRL:
	  value32 = stp220x->tme_stp220x_sc_perf_ctrl;
	  break;
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_CPU(0)):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_CPU(1)):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_IO):
	case TME_STP220X_SC_UPA_CONFIG(TME_STP220X_UPA_PORT_FFB):
	  value32 = stp220x->tme_stp220x_sc_upa_config[upa_port];
	  break;
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_CPU(0)):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_CPU(1)):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_IO):
	case TME_STP220X_SC_UPA_STATUS(TME_STP220X_UPA_PORT_FFB):
	  value32 = stp220x->tme_stp220x_sc_upa_status[upa_port];
	  break;
	case TME_STP2200_SC_MC_CONTROL0:
	  value32 = stp220x->tme_stp2200_sc_mc_control[0];
	  break;
	case TME_STP2200_SC_MC_CONTROL1:
	  value32 = stp220x->tme_stp2200_sc_mc_control[1];
	  break;
	case TME_STP2202_SC_CC_DIAG:
	  value32 = stp220x->tme_stp2202_sc_cc_diag;
	  break;
	case TME_STP2202_SC_CC_FAULT:
	  value32 = stp220x->tme_stp2202_sc_cc_fault;
	  break;
	case TME_STP2202_SC_CC_PROC_INDEX:
	  value32 = stp220x->tme_stp2202_sc_cc_proc_index;
	  break;
	case TME_STP2202_SC_MEMCTRL(0x0):
	case TME_STP2202_SC_MEMCTRL(0x1):
	case TME_STP2202_SC_MEMCTRL(0x2):
	case TME_STP2202_SC_MEMCTRL(0x3):
	case TME_STP2202_SC_MEMCTRL(0x4):
	case TME_STP2202_SC_MEMCTRL(0x5):
	case TME_STP2202_SC_MEMCTRL(0x6):
	case TME_STP2202_SC_MEMCTRL(0x7):
	case TME_STP2202_SC_MEMCTRL(0x8):
	case TME_STP2202_SC_MEMCTRL(0x9):
	case TME_STP2202_SC_MEMCTRL(0xa):
	case TME_STP2202_SC_MEMCTRL(0xb):
	case TME_STP2202_SC_MEMCTRL(0xc):
	case TME_STP2202_SC_MEMCTRL(0xd):
	  value32
	    = (stp220x->tme_stp2202_sc_memctrl
	       [(stp220x->tme_stp220x_eb_addr
		 - TME_STP2202_SC_MEMCTRL(0x0))
		/ (TME_STP2202_SC_MEMCTRL(0x1)
		   - TME_STP2202_SC_MEMCTRL(0x0))]);
	  break;
	}

	/* update the EB_DATA value: */
	stp220x->tme_stp220x_eb_data[3] = value32;
	value32 >>= 8;
	stp220x->tme_stp220x_eb_data[2] = value32;
	value32 >>= 8;
	stp220x->tme_stp220x_eb_data[1] = value32;
	value32 >>= 8;
	stp220x->tme_stp220x_eb_data[0] = value32;
      }

      /* return the EB_DATA byte: */
      value8 = stp220x->tme_stp220x_eb_data[address % sizeof(tme_uint32_t)];
    }
     
    /* return the value: */
    tme_bus_cycle_xfer_memory(cycle,
			      &value8 - address,
			      address);
  }

  /* leave: */
  completion->tme_completion_error = TME_OK;
  _tme_stp220x_completion_validate(stp220x, completion);
  _tme_stp220x_leave(stp220x);

  /* ebus cycles can never be fast: */
  *_master_fast_cycle_types = 0;
}

/* this fills a TLB entry for the ebus connection: */
static void
_tme_stp220x_ebus_tlb_fill(struct tme_bus_connection *ebus_conn_bus,
			   struct tme_bus_tlb *tlb,
			   tme_bus_addr_t address_wider,
			   unsigned int cycle_type)
{

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers only this address: */
  /* XXX FIXME - this is because we're lazy and don't want to write
     the bus cycle handler to size cycles down to a byte: */
  tlb->tme_bus_tlb_addr_first = address_wider;
  tlb->tme_bus_tlb_addr_last = address_wider;
}

#if TME_STP22XX_BUS_TRANSITION

/* this is the bus signal transition glue: */
static int
_tme_stp220x_signal_transition(struct tme_bus_connection *agent_conn_bus,
			       unsigned int signal)
{
  struct tme_completion completion_buffer;
  tme_completion_init(&completion_buffer);
  _tme_stp220x_signal(agent_conn_bus,
		      signal,
		      &completion_buffer);
  return (TME_OK);
}
#define _tme_stp220x_signal _tme_stp220x_signal_transition

/* this is the bus cycle transition glue: */
static int
_tme_stp220x_cycle_transition(void *_master_conn_bus,
			      struct tme_bus_cycle *master_cycle)
{
  struct tme_completion completion_buffer;
  tme_uint32_t master_fast_cycle_types;
  tme_completion_init(&completion_buffer);
  _tme_stp220x_cycle((struct tme_bus_connection *) _master_conn_bus,
		     master_cycle,
		     &master_fast_cycle_types,
		     &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

/* this is the bus TLB fill transition glue: */
static int
_tme_stp220x_tlb_fill_transition(struct tme_bus_connection *agent_conn_bus,
				 struct tme_bus_tlb *tlb,
				 tme_bus_addr_t agent_address_wider,
				 unsigned int cycle_type)
{
  _tme_stp220x_tlb_fill(agent_conn_bus,
			tlb,
			agent_address_wider,
			cycle_type);

  /* we always handle any slow cycles: */
  tlb->tme_bus_tlb_cycles_ok |= cycle_type;
  tlb->tme_bus_tlb_addr_offset = 0;
  tlb->tme_bus_tlb_addr_shift = 0;
  tlb->tme_bus_tlb_cycle = _tme_stp220x_cycle_transition;
  tlb->tme_bus_tlb_cycle_private = agent_conn_bus;
  assert (tlb->tme_bus_tlb_fault_handler_count == 0);

  return (TME_OK);
}
#define _tme_stp220x_tlb_fill _tme_stp220x_tlb_fill_transition

/* this is the ebus cycle transition glue: */
static int
_tme_stp220x_ebus_cycle_transition(void *_ebus_conn_bus,
				   struct tme_bus_cycle *cycle)
{
  struct tme_completion completion_buffer;
  tme_uint32_t master_fast_cycle_types;
  tme_completion_init(&completion_buffer);
  _tme_stp220x_ebus_cycle((struct tme_bus_connection *) _ebus_conn_bus,
			  cycle,
			  &master_fast_cycle_types,
			  &completion_buffer);
  return (completion_buffer.tme_completion_error);
}

/* this is the ebus TLB fill transition glue: */
static int
_tme_stp220x_ebus_tlb_fill_transition(struct tme_bus_connection *ebus_conn_bus,
				      struct tme_bus_tlb *tlb,
				      tme_bus_addr_t address_wider,
				      unsigned int cycle_type)
{
  _tme_stp220x_ebus_tlb_fill(ebus_conn_bus,
			     tlb,
			     address_wider,
			     cycle_type);

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = ebus_conn_bus;
  tlb->tme_bus_tlb_cycle = _tme_stp220x_ebus_cycle_transition;

  return (TME_OK);
}
#define _tme_stp220x_ebus_tlb_fill _tme_stp220x_ebus_tlb_fill_transition

#endif /* TME_STP22XX_BUS_TRANSITION */

/* our command function: */
static int
_tme_stp220x_command(struct tme_element *element,
		     const char * const * args,
		     char **_output)
{
  struct tme_bus_connection dummy_conn_bus;
  struct tme_stp220x *stp220x;
  int do_reset;

  /* enter: */
  dummy_conn_bus.tme_bus_connection.tme_connection_element = element;
  stp220x = _tme_stp220x_enter_bus(&dummy_conn_bus);

  /* assume no reset: */
  do_reset = FALSE;

  /* the "power" command: */
  if (TME_ARG_IS(args[1], "power")
      && args[2] == NULL) {
    _tme_stp220x_reset(stp220x, TME_STP220X_SC_CONTROL_POR);
  }

  /* the "reset" command: */
  else if (TME_ARG_IS(args[1], "reset")
	   && args[2] == NULL) {
    _tme_stp220x_reset(stp220x, TME_STP220X_SC_CONTROL_B_XIR);
  }

  /* any other command: */
  else {
    if (args[1] != NULL) {
      tme_output_append_error(_output,
			      "%s '%s', ",
			      _("unknown command"),
			      args[1]);
    }
    tme_output_append_error(_output,
			    _("available %s commands: %s %s"),
			    args[0],
			    "power",
			    "reset");
    return (EINVAL);
  }

  /* leave: */
  _tme_stp220x_leave(stp220x);
  return (TME_OK);
}

/* the connection scorer: */
static int
_tme_stp220x_connection_score(struct tme_connection *conn,
			      unsigned int *_score)
{
  struct tme_bus_connection *conn_bus;
  struct tme_stp220x *stp220x;
  struct tme_upa_bus_connection *conn_upa_other;
  struct tme_bus_connection *conn_bus_other;
  unsigned int score;

  /* recover the bus connection: */
  conn_bus = (struct tme_bus_connection *) conn;

  /* enter: */
  stp220x = _tme_stp220x_enter_bus(conn_bus);

  /* assume that this connection is useless: */
  score = 0;

  /* dispatch on the connection type: */
  conn_upa_other = (struct tme_upa_bus_connection *) conn->tme_connection_other;
  conn_bus_other = (struct tme_bus_connection *) conn->tme_connection_other;
  switch (conn->tme_connection_type) {

    /* this must be a UPA agent, and not another controller: */
  case TME_CONNECTION_BUS_UPA:
    if (conn_upa_other->tme_upa_bus_connection.tme_bus_tlb_set_add == NULL
	&& conn_upa_other->tme_upa_bus_interrupt != NULL) {
      score = 10;
    }
    break;

    /* the ebus connection must be to a bus, not an agent.  FFB and
       memory connections must be to agents, not buses: */
  case TME_CONNECTION_BUS_GENERIC:
    if ((conn_bus_other->tme_bus_tlb_set_add != NULL)
	== (conn->tme_connection_id == TME_STP220X_CONN_EBUS)) {
      score = 1;
    }
    break;

  default: abort();
  }

  /* leave: */
  _tme_stp220x_leave(stp220x);
  *_score = score;
  return (TME_OK);
}

/* this makes a new connection: */
static int
_tme_stp220x_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_upa_bus_connection *conn_upa;
  struct tme_bus_connection *conn_bus;
  struct tme_stp220x *stp220x;
  unsigned int conn_index;
  const struct tme_bus_connection *conn_bus_other;
  tme_bus_addr64_t slave_address_last;

  /* ignore a half-connection: */
  if (state == TME_CONNECTION_HALF) {
    return (TME_OK);
  }

  /* recover the bus connection: */
  conn_upa = (struct tme_upa_bus_connection *) conn;
  conn_bus = (struct tme_bus_connection *) conn;

  /* enter: */
  stp220x = _tme_stp220x_enter_bus(conn_bus);

  /* dispatch on the connection index: */
  conn_index = conn->tme_connection_id;
  switch (conn_index) {
  default: assert (FALSE);
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(0)):
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(1)):
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_IO):
    stp220x->tme_stp220x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_upa = conn_upa;
    break;
  case TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_FFB):
  case TME_STP220X_CONN_SIMM_GROUP(0):
  case TME_STP220X_CONN_SIMM_GROUP(1):
  case TME_STP220X_CONN_SIMM_GROUP(2):
  case TME_STP220X_CONN_SIMM_GROUP(3):
#if TME_STP220X_SIMM_GROUP_COUNT != 4
#error "TME_STP220X_SIMM_GROUP_COUNT changed"
#endif
    conn_bus_other = (struct tme_bus_connection *) conn_bus->tme_bus_connection.tme_connection_other;
    slave_address_last = conn_bus_other->tme_bus_subregions.tme_bus_subregion_address_last;
    stp220x->tme_stp220x_conn_address_last[conn_index] = slave_address_last;
    assert (conn_bus_other->tme_bus_subregions.tme_bus_subregion_address_first == 0);
    assert (slave_address_last > 0
	    && (((slave_address_last + 1) & slave_address_last) == 0)
	    && (conn_index == TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_FFB)
		|| slave_address_last < (((tme_uint32_t) 1) << TME_STP220X_SIMM_GROUP_SIZE_LOG2(stp220x))));
    assert (conn_bus_other->tme_bus_subregions.tme_bus_subregion_next == NULL);
  case TME_STP220X_CONN_EBUS:
    stp220x->tme_stp220x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus = conn_bus;
    break;
  }

  /* leave: */
  _tme_stp220x_leave(stp220x);
  return (TME_OK);
}

/* this breaks a connection: */
static int 
_tme_stp220x_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes new connection sides: */
static int
_tme_stp220x_connections_new(struct tme_element *element,
			     const char * const *args,
			     struct tme_connection **_conns,
			     char **_output)
{
  int rc;
  struct tme_stp220x *stp220x;
  struct tme_upa_bus_connection *conn_upa;
  struct tme_bus_connection *conn_bus;
  struct tme_connection *conn;
  tme_bus_addr_t address;
  unsigned int conn_index;
  tme_bus_addr64_t region_size_m1;

  /* assume that we will succeed: */
  rc = TME_OK;

  /* recover our data structure: */
  stp220x = (struct tme_stp220x *) element->tme_element_private;

  /* lock the mutex: */
  tme_mutex_lock(&stp220x->tme_stp220x.tme_stp22xx_mutex);

  /* if we have no arguments, this is the ebus connection: */
  if (args[1] == NULL) {

    /* if an ebus connection has already been made: */
    if (stp220x->tme_stp220x.tme_stp22xx_conns[TME_STP220X_CONN_EBUS].tme_stp22xx_conn_bus != NULL) {
      rc = EEXIST;
    }

    /* otherwise, the ebus connection hasn't already been made: */
    else {

      /* create our side of a generic bus connection: */
      conn_bus = tme_new0(struct tme_bus_connection, 1);
      conn_bus->tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_GENERIC;

      /* fill in the generic bus connection: */
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_first = 0;
      conn_bus->tme_bus_subregions.tme_bus_subregion_address_last = (sizeof(tme_uint32_t) * 2) - 1;
      conn_bus->tme_bus_subregions.tme_bus_subregion_next = &_tme_stp2210_mc124x9_subregion;
      conn_bus->tme_bus_signals_add = NULL;
      conn_bus->tme_bus_signal = NULL;
      conn_bus->tme_bus_intack = NULL;
      conn_bus->tme_bus_tlb_set_add = NULL;
      conn_bus->tme_bus_tlb_fill = _tme_stp220x_ebus_tlb_fill;

      /* fill in the generic connection: */
      conn = &conn_bus->tme_bus_connection;
      conn->tme_connection_id = TME_STP220X_CONN_EBUS;
      conn->tme_connection_score = _tme_stp220x_connection_score;
      conn->tme_connection_make = _tme_stp220x_connection_make;
      conn->tme_connection_break = _tme_stp220x_connection_break;

      /* add in this connection side possibility: */
      conn->tme_connection_next = *_conns;
      *_conns = conn;
    }
  }

  /* otherwise, if this is a UPA bus connection: */
  else if (TME_ARG_IS(args[1], "addr")
	   && ((address
		= tme_bus_addr_parse(args[2], 0 - (tme_uint64_t) 1))
	       != (0 - (tme_uint64_t) 1))
	   && args[3] == NULL) {

    /* convert the address into a connection index and address: */
    conn_index = _tme_stp220x_lookup_address(stp220x, &address, &region_size_m1);

    /* if this connection is reserved: */
    if (conn_index == TME_STP220X_CONN_NULL) {
      rc = EINVAL;
    }

    /* otherwise, if this connection is already made: */
    else if (stp220x->tme_stp220x.tme_stp22xx_conns[conn_index].tme_stp22xx_conn_bus != NULL) {
      rc = EEXIST;
    }

    /* otherwise, this connection hasn't been made yet: */
    else {

      /* if this is a CPU or IO connection: */
      if (conn_index == TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_IO)
	  || conn_index == TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(0))
	  || conn_index == TME_STP220X_CONN_UPA_PORT(TME_STP220X_UPA_PORT_CPU(1))) {

	/* create our side of a UPA connection: */
	conn_upa = tme_new0(struct tme_upa_bus_connection, 1);
	conn_upa->tme_upa_bus_interrupt = _tme_stp220x_interrupt;
	conn_upa->tme_upa_bus_connection_mid = _tme_stp220x_conn_index_mid(stp220x, conn_index);
	conn_bus = &conn_upa->tme_upa_bus_connection;
	conn_bus->tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_UPA;
      }

      /* otherwise, this is an FFB or memory connection: */
      else {

	/* create our side of a generic bus connection: */
	conn_bus = tme_new0(struct tme_bus_connection, 1);
	conn_bus->tme_bus_connection.tme_connection_type = TME_CONNECTION_BUS_GENERIC;
      }

      /* fill in the generic bus connection: */
      conn_bus->tme_bus_signals_add = NULL;
      conn_bus->tme_bus_signal = _tme_stp220x_signal;
      conn_bus->tme_bus_intack = NULL;
      conn_bus->tme_bus_tlb_set_add = tme_stp22xx_tlb_set_add;
      conn_bus->tme_bus_tlb_fill = _tme_stp220x_tlb_fill;

      /* fill in the generic connection: */
      conn = &conn_bus->tme_bus_connection;
      conn->tme_connection_id = conn_index;
      conn->tme_connection_score = _tme_stp220x_connection_score;
      conn->tme_connection_make = _tme_stp220x_connection_make;
      conn->tme_connection_break = _tme_stp220x_connection_break;

      /* add in this connection side possibility: */
      conn->tme_connection_next = *_conns;
      *_conns = conn;
    }
  }

  /* otherwise, the arguments are unknown: */
  else {
    tme_output_append_error(_output, 
			    "%s %s [ addr %s ]",
			    _("usage:"),
			    args[0],
			    _("BUS-ADDRESS"));
    rc = EINVAL;
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&stp220x->tme_stp220x.tme_stp22xx_mutex);

  return (rc);
}

/* this creates a new stp220x element: */
static int
_tme_stp220x_new(struct tme_element *element,
		 const char * const *args,
		 const void *extra,
		 char **_output,
		 unsigned int is_2200)
{
  struct tme_stp220x *stp220x;
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
			      _("%s unexpected, "),
			      args[arg_i]);
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

  /* start the stp220x structure: */
  stp220x = tme_new0(struct tme_stp220x, 1);
  stp220x->tme_stp220x.tme_stp22xx_element = element;
  stp220x->tme_stp220x.tme_stp22xx_run = _tme_stp220x_run;
  tme_stp22xx_init(&stp220x->tme_stp220x,
		   sizeof(struct tme_stp220x),
		   TME_STP220X_CONN_NULL);

  /* set the type: */
  stp220x->tme_stp220x_is_2200 = is_2200;
  if (TME_STP220X_IS_2200(stp220x) != is_2200) {
    tme_free(stp220x);
    return (ENXIO);
  }

  /* initialize the last address in each connection to all-bits-one: */
  memset (stp220x->tme_stp220x_conn_address_last,
	  0xff,
	  sizeof(stp220x->tme_stp220x_conn_address_last));

  /* fill the element: */
  element->tme_element_private = stp220x;
  element->tme_element_connections_new = _tme_stp220x_connections_new;
  element->tme_element_command = _tme_stp220x_command;

  return (TME_OK);
}

/* this creates a new stp2200 element: */
TME_ELEMENT_X_NEW_DECL(tme_ic_,stp22xx,stp2200) {
  return (_tme_stp220x_new(element, args, extra, _output, TRUE));
}
