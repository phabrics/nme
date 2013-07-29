/* $Id: sun2-control.c,v 1.5 2009/08/30 14:42:15 fredette Exp $ */

/* machine/sun2/sun2-control.c - implementation of Sun 2 emulation control space: */

/*
 * Copyright (c) 2003 Matt Fredette
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
_TME_RCSID("$Id: sun2-control.c,v 1.5 2009/08/30 14:42:15 fredette Exp $");

/* includes: */
#include "sun2-impl.h"

/* macros: */

/* the bus cycle handler for function code three space: */
int
_tme_sun2_control_cycle_handler(void *_sun2, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun2 *sun2;
  struct tme_bus_cycle cycle_resp;
  tme_bus_addr32_t reg, address, index;
  tme_uint32_t pte;
  int rc;

  /* recover our sun2: */
  sun2 = (struct tme_sun2 *) _sun2;

  /* get the register and address and index: */
  reg = cycle_init->tme_bus_cycle_address & (TME_SUN2_PAGE_SIZE - 1);
  reg = TME_MIN(reg, TME_SUN2_CONTROL_JUNK);
  address = cycle_init->tme_bus_cycle_address & ~(TME_SUN2_PAGE_SIZE - 1);
  index = address >> TME_SUN2_PAGE_SIZE_LOG2;

  /* this macro evaluates to TRUE whenever a register is maybe being
     accessed: */
#define _TME_SUN2_REG_ACCESSED(icreg)				\
  ((TME_SUN2_CONTROL_ADDRESS(icreg) == 0)			\
   ? (reg < sizeof(sun2->icreg))				\
   : TME_RANGES_OVERLAP(reg,					\
			reg					\
			+ cycle_init->tme_bus_cycle_size - 1,	\
			!TME_SUN2_CONTROL_ADDRESS(icreg) +	\
			TME_SUN2_CONTROL_ADDRESS(icreg),	\
			TME_SUN2_CONTROL_ADDRESS(icreg)		\
			+ sizeof(sun2->icreg) - 1))

  /* whenever the page map register is accessed, we need to fill it
     before running the cycle: */
  if (_TME_SUN2_REG_ACCESSED(tme_sun2_pgmap_hi)
      || _TME_SUN2_REG_ACCESSED(tme_sun2_pgmap_lo)) {

    /* get the PTE from the MMU: */
    rc = _tme_sun2_mmu_pte_get(sun2, address, &pte);
    assert(rc == TME_OK);
    sun2->tme_sun2_pgmap_hi = (pte >> 16);
    sun2->tme_sun2_pgmap_lo = (pte & 0xffff);
  }

  /* whenever the segment map register is accessed, we need to fill it
     before running the cycle: */
  if (_TME_SUN2_REG_ACCESSED(tme_sun2_segmap)
      && cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ) {
    sun2->tme_sun2_segmap = tme_sun_mmu_segmap_get(sun2->tme_sun2_mmu, sun2->tme_sun2_context_user, address);
  }

  /* whenever the IDPROM register is accessed, we need to fill it
     before running the cycle: */
  if (_TME_SUN2_REG_ACCESSED(tme_sun2_idprom)
      && index < sizeof(sun2->tme_sun2_idprom_contents)) {
    sun2->tme_sun2_idprom = sun2->tme_sun2_idprom_contents[index];
  }

  /* run the cycle: */
  TME_SUN2_CONTROL_BUS_CYCLE(sun2, reg, &cycle_resp);
  cycle_resp.tme_bus_cycle_type = (cycle_init->tme_bus_cycle_type
				   ^ (TME_BUS_CYCLE_WRITE
				      | TME_BUS_CYCLE_READ));
  cycle_resp.tme_bus_cycle_lane_routing = cycle_init->tme_bus_cycle_lane_routing;
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* whenever the bus error register is read or written, it is
     cleared: */
  if (_TME_SUN2_REG_ACCESSED(tme_sun2_buserr)) {
    sun2->tme_sun2_buserr = 0;
  }

  /* these registers only need action taken when they're written: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {
    
    /* the page map has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_pgmap_hi)
	|| _TME_SUN2_REG_ACCESSED(tme_sun2_pgmap_lo)) {
      pte = sun2->tme_sun2_pgmap_hi;
      pte = (pte << 16) | sun2->tme_sun2_pgmap_lo;
      rc = _tme_sun2_mmu_pte_set(sun2, address, pte);
      assert(rc == TME_OK);
    }

    /* the segment map has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_segmap)) {
	tme_sun_mmu_segmap_set(sun2->tme_sun2_mmu, sun2->tme_sun2_context_user, address, sun2->tme_sun2_segmap);
    }

    /* the system context register has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_context_system)) {
      _tme_sun2_mmu_context_system_set(sun2);
    }

    /* the system context register has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_context_user)) {
      _tme_sun2_mmu_context_user_set(sun2);
    }

    /* the diag register has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_diag)) {
      /* TBD */
    }

    /* the system enable register has been written: */
    if (_TME_SUN2_REG_ACCESSED(tme_sun2_enable)) {
      rc = _tme_sun2_ipl_check(sun2);
      assert(rc == TME_OK);
      /* in case TME_SUN2_ENA_NOTBOOT changed, make a pseudo-context
	 change: */
      _tme_sun2_mmu_context_user_set(sun2);
    }
  }

  return (TME_OK);
}

#if 1
#include <stdio.h>

/* this dumps out the sun2 state: */
void
tme_sun2_dump(struct tme_sun2 *sun2)
{
  
  /* dump out the page map register: */
  fprintf(stderr, "PGMAP = 0x%04x%04x\n",
	  sun2->tme_sun2_pgmap_hi,
	  sun2->tme_sun2_pgmap_lo);
  fprintf(stderr, "SEGMAP = 0x%02x\n", sun2->tme_sun2_segmap);
  fprintf(stderr, "\n");
  fprintf(stderr, "SCONTEXT = 0x%02x\n", sun2->tme_sun2_context_system);
  fprintf(stderr, " CONTEXT = 0x%02x\n", sun2->tme_sun2_context_user);
  fprintf(stderr, "\n");
  fprintf(stderr, "IDPROM = 0x%02x\n", sun2->tme_sun2_idprom);
  fprintf(stderr, "DIAG = 0x%02x\n", sun2->tme_sun2_diag);
  fprintf(stderr, "BUSERR = 0x%04x\n", sun2->tme_sun2_buserr);
  fprintf(stderr, "ENABLE = 0x%04x\n", sun2->tme_sun2_enable);
}
#endif /* 1 */
