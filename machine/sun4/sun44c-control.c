/* $Id: sun44c-control.c,v 1.3 2007/03/29 01:17:49 fredette Exp $ */

/* machine/sun4/sun4-control.c - implementation of Sun 4/4c control space emulation: */

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
_TME_RCSID("$Id: sun44c-control.c,v 1.3 2007/03/29 01:17:49 fredette Exp $");

/* includes: */
#include "sun4-impl.h"

/* the bus cycle handler for control spaces: */
int
_tme_sun44c_control_cycle_handler(void *_sun4_asi, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun4_asi *sun4_asi;
  struct tme_sun4 *sun4;
  struct tme_bus_cycle cycle_resp;
  tme_uint32_t asi;
  tme_uint32_t address;
  tme_uint8_t *clear_data8;
  tme_uint32_t *clear_data32;
  tme_uint32_t value32;
  tme_uint8_t enable_old, enable_new;
  int rc;

  /* recover our sun4 and asi: */
  sun4_asi = (struct tme_sun4_asi *) _sun4_asi;
  sun4 = sun4_asi->tme_sun4_asi_sun4;
  asi = sun4_asi - &sun4->tme_sun4_asis[0];

  /* get the address: */
  address = cycle_init->tme_bus_cycle_address;

  /* dispatch on the ASI and address, to get a value for our port: */
  value32 = 0;
  clear_data8 = NULL;
  clear_data32 = NULL;
  switch (asi) {

  case TME_SUN44C_ASI_SEGMAP:
    value32 = tme_sun_mmu_segmap_get(sun4->tme_sun44c_mmu, sun4->tme_sun44c_context, address);
    break;

  case TME_SUN44C_ASI_PGMAP:
    rc = _tme_sun44c_mmu_pte_get(sun4, address, &value32);
    assert(rc == TME_OK);
    break;
      
  case TME_SUN4C_ASI_HW_FLUSH_SEG: /* TME_SUN4_ASI_COPY */
    if (TME_SUN4_IS_SUN4C(sun4)) {
      if (cycle_init->tme_bus_cycle_type != TME_BUS_CYCLE_WRITE) {
	abort();
      }
    }
    else {
      abort();
    }
    break;

  case TME_SUN4C_ASI_HW_FLUSH_PG: /* TME_SUN4_ASI_REGMAP */
    if (TME_SUN4_IS_SUN4C(sun4)) {
      if (cycle_init->tme_bus_cycle_type != TME_BUS_CYCLE_WRITE) {
	abort();
      }
    }
    else {
      abort();
    }
    break;

  case TME_SUN4C_ASI_HW_FLUSH_ALL: /* TME_SUN4_ASI_FLUSH_USER */
  case TME_SUN4C_ASI_HW_FLUSH_CONTEXT: /* TME_SUN4_ASI_FLUSH_REG */
  case TME_SUN44C_ASI_FLUSH_CONTEXT:
  case TME_SUN44C_ASI_FLUSH_SEG:
  case TME_SUN44C_ASI_FLUSH_PG:
    if (cycle_init->tme_bus_cycle_type != TME_BUS_CYCLE_WRITE) {
      abort();
    }
    break;

  case TME_SUN4_32_ASI_CONTROL:

    /* dispatch on a smaller register number: */
    switch (TME_SUN44C_CONTROL_REG(address)) {
    
    case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_IDPROM):
      abort();
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CONTEXT):
      value32 = sun4->tme_sun44c_context;
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_ENABLE):
      value32 = sun4->tme_sun44c_enable;
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_UDVMA):
      value32 = sun4->tme_sun4_udvma;
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN4C_CONTROL_SYNC_ERR): /* TME_SUN4_CONTROL_BUSERR */
      if (TME_SUN4_IS_SUN4C(sun4)) {
	switch (address) {
	case TME_SUN4C_CONTROL_SYNC_ERR:
	  clear_data32 = &sun4->tme_sun4c_sync_err;
	  break;
	case TME_SUN4C_CONTROL_SYNC_VADDR:
	  /* XXX FIXME - SunOS' include/sun4c/buserr.h doesn't say if
	     the synchronous address register is cleared when read: */
	  value32 = sun4->tme_sun4c_sync_vaddr;
	  break;
	case TME_SUN4C_CONTROL_ASYNC_ERR:
	  clear_data32 = &sun4->tme_sun4c_async_err;
	  break;
	case TME_SUN4C_CONTROL_ASYNC_VADDR:
	  clear_data32 = &sun4->tme_sun4c_async_vaddr;
	  break;
	case TME_SUN4C_CONTROL_ASYNC_DATA_LO:
	  clear_data32 = &sun4->tme_sun4c_async_data_lo;
	  break;
	case TME_SUN4C_CONTROL_ASYNC_DATA_HI:
	  clear_data32 = &sun4->tme_sun4c_async_data_hi;
	  break;
	default:
	  abort();
	}
	if (clear_data32 != NULL) {
	  value32 = *clear_data32;
	}
      }
      else {
	value32 = sun4->tme_sun4_buserr;
	clear_data8 = &sun4->tme_sun4_buserr;
      }
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_DIAG):
      value32 = sun4->tme_sun4_diag;
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CACHE_TAGS):
    case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CACHE_DATA):
      return (_tme_sun44c_cache_cycle_control(sun4, cycle_init));
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_UDVMA_MAP):
      abort();
      break;

    case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_VME_INTVEC):
      abort();
      break;

    default:
      abort();
    }
    break;

  default:
    abort();
  }
     
  /* run the bus cycle: */
  cycle_resp.tme_bus_cycle_buffer
    = (((tme_uint8_t *) &value32)
       + (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
	  ? (sizeof(value32)
	     - cycle_init->tme_bus_cycle_size)
	  : ((unsigned int) cycle_init->tme_bus_cycle_size
	     - 1)));
  cycle_resp.tme_bus_cycle_buffer_increment
    = (TME_ENDIAN_NATIVE == TME_ENDIAN_BIG
       ? 1
       : -1);
  cycle_resp.tme_bus_cycle_lane_routing = cycle_init->tme_bus_cycle_lane_routing;
  cycle_resp.tme_bus_cycle_address = 0;
  cycle_resp.tme_bus_cycle_type = (cycle_init->tme_bus_cycle_type
				   ^ (TME_BUS_CYCLE_WRITE
				      | TME_BUS_CYCLE_READ));
  cycle_resp.tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, TME_BUS32_LOG2);
  tme_bus_cycle_xfer(cycle_init, &cycle_resp);

  /* whenever certain registers are read and/or written, they are cleared: */
  if (clear_data8 != NULL) {
    *clear_data8 = 0;
  }
  if (clear_data32 != NULL) {
    *clear_data32 = 0;
  }

  /* only these registers need action taken when they're written: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* dispatch on the ASI and address: */
    switch (asi) {

    case TME_SUN44C_ASI_SEGMAP:
      value32 &= (sun4->tme_sun44c_mmu_pmegs - 1);
      tme_sun_mmu_segmap_set(sun4->tme_sun44c_mmu, sun4->tme_sun44c_context, address, value32);
      break;

    case TME_SUN44C_ASI_PGMAP:
      rc = _tme_sun44c_mmu_pte_set(sun4, address, value32);
      assert (rc == TME_OK);
      break;
      
    case TME_SUN4C_ASI_HW_FLUSH_SEG: /* TME_SUN4_ASI_COPY */
      if (TME_SUN4_IS_SUN4C(sun4)) {
	if (__tme_predict_false((sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0)) {
	  _tme_sun44c_cache_cycle_flush(sun4, asi, address);
	}
      }
      else {
	abort();
      }
      break;

    case TME_SUN4C_ASI_HW_FLUSH_PG: /* TME_SUN4_ASI_REGMAP */
      if (TME_SUN4_IS_SUN4C(sun4)) {
	if (__tme_predict_false((sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0)) {
	  _tme_sun44c_cache_cycle_flush(sun4, asi, address);
	}
      }
      else {
	abort();
      }
      break;

    case TME_SUN4C_ASI_HW_FLUSH_ALL: /* TME_SUN4_ASI_FLUSH_USER */
    case TME_SUN4C_ASI_HW_FLUSH_CONTEXT: /* TME_SUN4_ASI_FLUSH_REG */
    case TME_SUN44C_ASI_FLUSH_CONTEXT:
    case TME_SUN44C_ASI_FLUSH_SEG:
    case TME_SUN44C_ASI_FLUSH_PG:
      if (__tme_predict_false((sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0)) {
	_tme_sun44c_cache_cycle_flush(sun4, asi, address);
      }
      break;

    case TME_SUN4_32_ASI_CONTROL:

      /* dispatch on a smaller register number: */
      switch (TME_SUN44C_CONTROL_REG(address)) {
    
      case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CONTEXT):
	sun4->tme_sun44c_context = value32;
	_tme_sun44c_mmu_context_set(sun4);
	break;

      case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_ENABLE):

	/* get the old and initial new values for the enable register: */
	enable_old = sun4->tme_sun44c_enable;
	enable_new = value32;

	/* fix up the new value for the enable register: */
	if (TME_SUN4_IS_SUN4C(sun4)) {

	  /* if this is a software reset, clear all other bits in the
	     enable register: */
	  if (enable_new & TME_SUN4C_ENA_RESET_SW) {
	    enable_new = TME_SUN4C_ENA_RESET_SW;
	  }
	}
	else {
	  /* XXX FIXME - we need to handle the "monitor" bit here: */
	  abort();
	  enable_new = (enable_old & TME_SUN4_ENA_DIAG) | (enable_new & ~TME_SUN4_ENA_DIAG);
	}

	/* set the new value of the enable register: */
	sun4->tme_sun44c_enable = enable_new;

	/* if we're changing to or from boot state: */
	if ((enable_old ^ enable_new) & TME_SUN44C_ENA_NOTBOOT) {

	  /* make a pseudo-context change: */
	  _tme_sun44c_mmu_context_set(sun4);

	  /* set the FPU strictness: */
	  ((*sun4->tme_sun4_sparc->tme_sparc_bus_fpu_strict)
	   (sun4->tme_sun4_sparc, !(enable_new & TME_SUN44C_ENA_NOTBOOT)));
	}

	/* if we're changing the cache enable, call out the change: */
	if ((enable_old ^ enable_new) & TME_SUN44C_ENA_CACHE) {
	  _tme_sun44c_cache_enable_change(sun4);
	}

	/* if we're enabling or disabling system DVMA, call out the change: */
	if ((enable_old ^ enable_new) & TME_SUN44C_ENA_SDVMA) {
	  _tme_sun44c_mmu_sdvma_change(sun4);
	}

	/* sun4c-specific enable register changes: */
	if (TME_SUN4_IS_SUN4C(sun4)) {

	  /* if this is a software reset: */
	  if (enable_new & TME_SUN4C_ENA_RESET_SW) {
	    return (_tme_sun4_reset(sun4, TRUE));
	  }
	}

	/* sun4-specific enable register changes: */
	else {
	  abort();
	}
	break;

      case TME_SUN44C_CONTROL_REG(TME_SUN4C_CONTROL_SYNC_ERR): /* TME_SUN4_CONTROL_BUSERR */
	if (TME_SUN4_IS_SUN4C(sun4)) {
	  switch (address) {
	  case TME_SUN4C_CONTROL_SYNC_ERR:
	    sun4->tme_sun4c_sync_err = value32;
	    break;
	  case TME_SUN4C_CONTROL_SYNC_VADDR:
	    sun4->tme_sun4c_sync_vaddr = value32;
	    break;
	  case TME_SUN4C_CONTROL_ASYNC_ERR:
	    sun4->tme_sun4c_async_err = value32;
	    break;
	  case TME_SUN4C_CONTROL_ASYNC_VADDR:
	    sun4->tme_sun4c_async_vaddr = value32;
	    break;
	  case TME_SUN4C_CONTROL_ASYNC_DATA_LO:
	    sun4->tme_sun4c_async_data_lo = value32;
	    break;
	  case TME_SUN4C_CONTROL_ASYNC_DATA_HI:
	    sun4->tme_sun4c_async_data_hi = value32;
	    break;
	  default:
	    abort();
	  }
	}
	else {
	  sun4->tme_sun4_buserr = value32;
	}
	break;

      case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_UDVMA):
	if (sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) {
	  abort();
	}
	sun4->tme_sun4_udvma = value32;
	break;

      case TME_SUN44C_CONTROL_REG(TME_SUN4_CONTROL_DIAG):
	sun4->tme_sun4_diag = value32;
	/* TBD */
	break;

      case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CACHE_TAGS):
      case TME_SUN44C_CONTROL_REG(TME_SUN44C_CONTROL_CACHE_DATA):
	assert(FALSE);

      default:
	break;
      }
      break;

    default:
      break;
    }
  }

  return (TME_OK);
}

/* the bus cycle handler for the interrupt register: */
int
_tme_sun44c_intreg_cycle_control(void *_sun4, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun4 *sun4;
  tme_uint8_t ints_old;
  int rc;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) _sun4;

  /* get the current interrupt register value: */
  ints_old = sun4->tme_sun44c_ints;

  /* do the transfer: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    &sun4->tme_sun44c_ints,
			    sizeof(sun4->tme_sun44c_ints) - 1);

  /* assume that we will return no special code: */
  rc = TME_OK;

  /* if the interrupt register has been written: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* on a sun4c, disabling interrupts clears an NMI: */
    if (TME_SUN4_IS_SUN4C(sun4)
	&& (ints_old & TME_SUN44C_IREG_INTS_ENAB)
	&& !(sun4->tme_sun44c_ints & TME_SUN44C_IREG_INTS_ENAB)) {
      sun4->tme_sun4_int_signals[TME_SPARC_IPL_NMI / 8] &= ~TME_BIT(TME_SPARC_IPL_NMI % 8);
    }

#if 1 /* emulation speed problem */
    /* the simple loop that the SS2 PROM power-on self test uses to
       wait for an level 14 timer interrupt times out before we can
       deliver the interrupt.  this is either because of the large
       instruction burst that the CY7C601 emulation uses (many
       iterations of that loop run without yielding to other threads
       and checking for the interrupt), or because the CY7C601
       emulation on a particular host runs much faster than the CPU in
       a real SS2.

       since this problem is more or less tied to the machine-specific
       PROM, we deal with this problem here in the machine emulation
       instead of in the CPU emulation.

       whenever the interrupt register is written to open up the level
       14 interrupt, and the enable register is still in the boot state,
       we assume that this test is running, and we immediately cause
       the timer interrupt to be delivered and the CPU to check for it.

       assuming that this will be a problem for all sun4/4c, we do it
       for all models for now: */
    if ((sun4->tme_sun44c_enable & TME_SUN44C_ENA_NOTBOOT) == 0
	&& ((sun4->tme_sun44c_ints
	     & (TME_SUN44C_IREG_INTS_ENAB
		| TME_SUN44C_IREG_COUNTER_L14))
	    == (TME_SUN44C_IREG_INTS_ENAB
		| TME_SUN44C_IREG_COUNTER_L14))
	&& ((ints_old
	     & (TME_SUN44C_IREG_INTS_ENAB
		| TME_SUN44C_IREG_COUNTER_L14))
	    != (TME_SUN44C_IREG_INTS_ENAB
		| TME_SUN44C_IREG_COUNTER_L14))) {
      _tme_sun4_timer_int_force(sun4, &sun4->tme_sun4_timer_l14);
      rc = TME_BUS_CYCLE_SYNCHRONOUS_EVENT;
    }
#endif /* emulation speed problem */       

    /* if an interrupt is pending, get the CPU to check for it now: */
    if (_tme_sun4_ipl_check(sun4)) {
      rc = TME_BUS_CYCLE_SYNCHRONOUS_EVENT;
    }
  }

  return (rc);
}

/* the bus cycle handler for the auxiliary register: */
int
_tme_sun4c_auxreg_cycle_control(void *_sun4, struct tme_bus_cycle *cycle_init)
{
  struct tme_sun4 *sun4;

  /* recover our sun4: */
  sun4 = (struct tme_sun4 *) _sun4;

  /* do the transfer: */
  tme_bus_cycle_xfer_memory(cycle_init, 
			    &sun4->tme_sun4c4m_aux,
			    sizeof(sun4->tme_sun4c4m_aux) - 1);

  return (TME_OK);
}
