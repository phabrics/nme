/* $Id: nec765.c,v 1.3 2009/08/29 21:12:47 fredette Exp $ */

/* ic/nec765.c - NEC 765 (and Intel 8207x) implementation: */

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
_TME_RCSID("$Id: nec765.c,v 1.3 2009/08/29 21:12:47 fredette Exp $");

/* includes: */
#undef TME_NEC765_VERSION
#define TME_NEC765_VERSION TME_X_VERSION(0, 0)
#include <tme/ic/nec765.h>
#include <tme/generic/bus-device.h>

/* macros: */

/* the different parts: */
#define TME_NEC765_PART_NEC765		(0)
#define TME_NEC765_PART_I82072		(1)
#define TME_NEC765_PART_I82077		(2)

/* register offsets.  to keep things simple, all of the register
   offsets are for the i82077: */
#define TME_NEC765_I82077_REG_STATUS_A	(0)
#define TME_NEC765_I82077_REG_STATUS_B	(1)
#define TME_NEC765_I82077_REG_DOR	(2)
#define TME_NEC765_I82077_REG_TCR	(3)
#define TME_NEC765_REG_MSR		(4)	/* read-only */
#define TME_NEC765_REG_DRS		(4)	/* write-only */
#define TME_NEC765_REG_FIFO		(5)
#define TME_NEC765_SIZ_REG_NEC765	(2)
#define TME_NEC765_SIZ_REG_I82072	(4)
#define TME_NEC765_SIZ_REG_I82077	(8)

/* the DOR register: */
#define TME_NEC765_I82077_DOR_REST	TME_BIT(2)
#define TME_NEC765_I82077_DOR_DMA	TME_BIT(3)

/* the MSR register: */
#define TME_NEC765_MSR_ACTIVE_A		TME_BIT(0)	/* drive A is active */
#define TME_NEC765_MSR_ACTIVE_B		TME_BIT(1)	/* drive B is active */
#define TME_NEC765_MSR_CB		TME_BIT(4)	/* controller is busy */
#define TME_NEC765_MSR_NON_DMA		TME_BIT(5)	/* non-DMA mode */
#define TME_NEC765_MSR_DIO		TME_BIT(6)	/* data register is input (CPU should read) */
#define TME_NEC765_MSR_RQM		TME_BIT(7)	/* request for master */

/* the DRS register: */
#define TME_NEC765_I82072_DRS_MODE_MASK	(0x0f)
#define  TME_NEC765_I82072_DRS_MODE_500KBPS	(0x00)
#define  TME_NEC765_I82072_DRS_MODE_300KBPS	(0x01)
#define  TME_NEC765_I82072_DRS_MODE_250KBPS	(0x02)
#define  TME_NEC765_I82072_DRS_MODE_125KBPS	(0x03)
#define TME_NEC765_I82072_DRS_PLL	TME_BIT(5)
#define TME_NEC765_I82072_DRS_POWER	TME_BIT(6)
#define TME_NEC765_I82072_DRS_RESET	TME_BIT(7)	/* soft reset */

/* the ST0 register: */
#define TME_NEC765_ST0_UC		TME_BIT(4)	/* unit check */
#define TME_NEC765_ST0_NR		TME_BIT(3)	/* unit check */
#define TME_NEC765_ST0_IC_MASK		(0xc0)
#define  TME_NEC765_ST0_IC_NORMAL	 (0x00)
#define  TME_NEC765_ST0_IC_ABNORMAL	 (0x40)
#define  TME_NEC765_ST0_IC_INVALID	 (0x80)
#define  TME_NEC765_ST0_IC_ABNORMAL_POLL (0xc0)

/* the ST3 register: */
#define TME_NEC765_ST3_RDY		TME_BIT(5)

/* commands: */
#define _TME_NEC765_CMD(nec765, mask, cmd) (((nec765)->tme_nec765_data_fifo[0] & (mask)) == (cmd))
#define TME_NEC765_CMD_TRACK_READ(nec765)	_TME_NEC765_CMD(nec765, 0x9f, 0x02)
#define TME_NEC765_CMD_FIX_DRIVE_DATA(nec765)	_TME_NEC765_CMD(nec765, 0xff, 0x03)
#define TME_NEC765_CMD_CALIBRATE(nec765)	_TME_NEC765_CMD(nec765, 0xff, 0x07)
#define TME_NEC765_CMD_CHECK_INTS(nec765)	_TME_NEC765_CMD(nec765, 0xff, 0x08)
#define TME_NEC765_CMD_I82072_CONFIG(nec765)	_TME_NEC765_CMD(nec765, 0xff, 0x13)
/* XXX FIXME - is this right? */
#define TME_NEC765_CMD_I82072_DUMPREG(nec765)	_TME_NEC765_CMD(nec765, 0xff, 0x0e)

/* this finishes a command: */
#define TME_NEC765_CMD_DONE(nec765)			\
  do {							\
    (nec765)->tme_nec765_data_fifo_head = 0;		\
    (nec765)->tme_nec765_status_fifo_tail = 0;		\
    (nec765)->tme_nec765_status_fifo_head = 0;		\
  } while (/* CONSTCOND */ 0)

/* this sets a status byte: */
#define TME_NEC765_STATUS(nec765, i, status)		\
  do {							\
    if ((i) == 0) {					\
      TME_NEC765_CMD_DONE(nec765);			\
    }							\
    (nec765)->tme_nec765_status_fifo[(i)] = (status);	\
    (nec765)->tme_nec765_status_fifo_head = (i) + 1;	\
  } while (/* CONSTCOND */ 0)

#define TME_NEC765_LOG_HANDLE(nec765) (&(nec765)->tme_nec765_element->tme_element_log_handle)

/* structures: */

/* the card: */
struct tme_nec765 {

  /* our simple bus device header: */
  struct tme_bus_device tme_nec765_device;
#define tme_nec765_element tme_nec765_device.tme_bus_device_element

  /* our socket: */
  struct tme_nec765_socket tme_nec765_socket;
#define tme_nec765_addr_shift tme_nec765_socket.tme_nec765_socket_addr_shift
#define tme_nec765_port_least_lane tme_nec765_socket.tme_nec765_socket_port_least_lane

  /* the mutex protecting the card: */
  tme_mutex_t tme_nec765_mutex;

  /* the part emulated: */
  unsigned int tme_nec765_part;
#define TME_NEC765_IS_NEC765(nec765)	((nec765)->tme_nec765_part == TME_NEC765_PART_NEC765)
#define TME_NEC765_IS_I82072(nec765)	((nec765)->tme_nec765_part == TME_NEC765_PART_I82072)
#define TME_NEC765_IS_I82077(nec765)	((nec765)->tme_nec765_part == TME_NEC765_PART_I82077)

  /* if the interrupt is currently asserted: */
  int tme_nec765_int_asserted;

  /* the data FIFO: */
  tme_uint8_t tme_nec765_data_fifo[512];
  unsigned int tme_nec765_data_fifo_head;

  /* the status FIFO: */
  tme_uint8_t tme_nec765_status_fifo[16];
  unsigned int tme_nec765_status_fifo_head;
  unsigned int tme_nec765_status_fifo_tail;

  /* the i82077 DOR: */
  tme_uint8_t tme_nec765_i82077_dor;
};

/* this resets the NEC765: */
static void
_tme_nec765_reset(struct tme_nec765 *nec765, int hard)
{

  /* reset the data FIFO: */
  nec765->tme_nec765_data_fifo_head = 0;

  /* reset the status FIFO: */
  nec765->tme_nec765_status_fifo_head = 0;
  nec765->tme_nec765_status_fifo_tail = 0;
}

/* the nec765 bus cycle handler: */
static int
_tme_nec765_bus_cycle(void *_nec765, 
		      struct tme_bus_cycle *cycle_init)
{
  struct tme_nec765 *nec765;
  tme_uint32_t address;
  tme_uint32_t reg_offset;
  tme_uint8_t value;
  unsigned int fifo_head;
  unsigned int fifo_tail;
  const char *reg;

  /* recover our data structure: */
  nec765 = (struct tme_nec765 *) _nec765;

  /* this access must be for eight bits: */
  assert (cycle_init->tme_bus_cycle_size == 1);

  /* get the address: */
  address = cycle_init->tme_bus_cycle_address;

  /* lock the mutex: */
  tme_mutex_lock(&nec765->tme_nec765_mutex);

  /* set the register offset: */
  reg_offset
    = (TME_NEC765_IS_I82077(nec765)
       ? 0
       : TME_NEC765_REG_MSR);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* get value being written: */
    tme_bus_cycle_xfer_memory(cycle_init, 
			      &value - address,
			      address);

    /* dispatch on the register: */
    switch (reg_offset + address) {

      /* the NetBSD fdc establish_chip_type() writes register number
	 two to distinguish an i82077 from an i82072.  on an i82077,
	 this is a write of the DOR; on an i82072, this write gets
	 mapped to register number six here: */
    case TME_NEC765_REG_MSR + TME_NEC765_I82077_REG_DOR:

      /* alias this register number six to the DRS: */
      /* FALLTHROUGH */

    case TME_NEC765_REG_DRS:

      /* if we need to soft-reset the chip: */
      if (value & TME_NEC765_I82072_DRS_RESET) {
	_tme_nec765_reset(nec765, FALSE);
      }

      reg = "DRS";
      break;

    case TME_NEC765_REG_FIFO:

      /* add to the data FIFO: */
      fifo_head = nec765->tme_nec765_data_fifo_head;
      assert (fifo_head < sizeof(nec765->tme_nec765_data_fifo));
      nec765->tme_nec765_data_fifo[fifo_head] = value;
      fifo_head++;
      nec765->tme_nec765_data_fifo_head = fifo_head;

      /* if we're waiting for a command: */
      if (1) {

	/* the check interrupts command: */
	if (TME_NEC765_CMD_CHECK_INTS(nec765)) {

	  /* the check interrupts command is a one-byte command: */
	  if (fifo_head == sizeof(tme_uint8_t)) {

	    if (nec765->tme_nec765_int_asserted) {
	      TME_NEC765_STATUS(nec765, 0, TME_NEC765_ST0_IC_NORMAL);
	      nec765->tme_nec765_int_asserted = FALSE;
	    }
	    else {
	      TME_NEC765_STATUS(nec765, 0, TME_NEC765_ST0_IC_INVALID);
	    }
	    TME_NEC765_STATUS(nec765, 1, 0x00); /* current cylinder */
	  }
	}

	/* the fix drive data command: */
	else if (TME_NEC765_CMD_FIX_DRIVE_DATA(nec765)) {

	  /* the fix drive data command is three-byte command: */
	  if (fifo_head == (sizeof(tme_uint8_t) * 3)) {

	    /* no status: */
	    TME_NEC765_CMD_DONE(nec765);
	  }
	}

	/* the read track command: */
	else if (TME_NEC765_CMD_TRACK_READ(nec765)) {

	  /* the read track command is an eight-byte command: */
	  if (fifo_head == (sizeof(tme_uint8_t) * 8)) {

	    /* return a unit check, drive not ready ST0: */
	    TME_NEC765_STATUS(nec765, 0, (TME_NEC765_ST0_UC | TME_NEC765_ST0_NR));
	  }
	}

	/* the calibrate command: */
	else if (TME_NEC765_CMD_CALIBRATE(nec765)) {

	  /* the calibrate command is a two-byte command: */
	  if (fifo_head == (sizeof(tme_uint8_t) * 2)) {

	    /* return an ST3: */
	    TME_NEC765_STATUS(nec765, 0,
			      (TME_NEC765_IS_NEC765(nec765)
			       ? TME_NEC765_ST3_RDY
			       : !TME_NEC765_ST3_RDY));
	  }
	}

	/* the i82072 configure command: */
	else if (TME_NEC765_CMD_I82072_CONFIG(nec765)) {

	  /* the i82072 configure command is a four-byte command: */
	  if (fifo_head == (sizeof(tme_uint8_t) * 4)) {

	    /* no status: */
	    TME_NEC765_CMD_DONE(nec765);
	  }
	}

	/* the i82072 dump registers command: */
	else if (TME_NEC765_CMD_I82072_DUMPREG(nec765)) {

	  /* the dump registers command is a one-byte command: */
	  if (fifo_head == sizeof(tme_uint8_t)) {

	    /* XXX FIXME - this is supposed to return ten status
	       bytes, but we return none instead, which causes the
	       NetBSD fd.c to decide that there are no drives
	       attached: */
	    TME_NEC765_CMD_DONE(nec765);
	  }
	}

	/* otherwise, this is an unknown command: */
	else {
	  abort();
	}
      }

      reg = "FIFO";
      break;

    case TME_NEC765_I82077_REG_DOR:
      if ((value & TME_NEC765_I82077_DOR_REST) == 0) {
	abort();
      }
      nec765->tme_nec765_i82077_dor = value;
      reg = "DOR";
      break;

    default:
      abort();
    }
    tme_log(TME_NEC765_LOG_HANDLE(nec765), 1000, TME_OK,
	    (TME_NEC765_LOG_HANDLE(nec765),
	     _("%s <- 0x%02x"),
	     reg,
	     value));
  }

  /* otherwise, this must be a read: */
  else {
    assert (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register: */
    switch (reg_offset + address) {

      /* the NetBSD fdc establish_chip_type() reads register number
	 two to distinguish an i82077 from an i82072.  on an i82077,
	 this is a read of the DOR; on an i82072, this read gets
	 mapped to register number six here: */
    case TME_NEC765_REG_MSR + TME_NEC765_I82077_REG_DOR:

      /* alias this register number six to the MSR: */
      /* FALLTHROUGH */

    case TME_NEC765_REG_MSR:

      /* if we are requesting the master: */
      if (1) {

	value = TME_NEC765_MSR_RQM;

	/* if we still have status to return: */
	if (nec765->tme_nec765_status_fifo_tail
	    < nec765->tme_nec765_status_fifo_head) {
	  value |= TME_NEC765_MSR_DIO;
	}
      }

      reg = "MSR";
      break;

    case TME_NEC765_REG_FIFO:

      /* if we're returning status: */
      fifo_tail = nec765->tme_nec765_status_fifo_tail;
      if (fifo_tail
	  < nec765->tme_nec765_status_fifo_head) {
	value = nec765->tme_nec765_status_fifo[fifo_tail];
	nec765->tme_nec765_status_fifo_tail = fifo_tail + 1;
	reg = "FIFO (status)";
      }

      else {
	abort();
      }

      break;

    case TME_NEC765_I82077_REG_DOR:
      value = nec765->tme_nec765_i82077_dor;
      reg = "DOR";
      break;

    default:
      abort();
    }

    tme_log(TME_NEC765_LOG_HANDLE(nec765), 1000, TME_OK,
	    (TME_NEC765_LOG_HANDLE(nec765),
	     _("%s -> 0x%02x"),
	     reg,
	     value));

    /* return the value: */
    tme_bus_cycle_xfer_memory(cycle_init, 
			      &value - address,
			      address);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&nec765->tme_nec765_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the nec765 TLB filler: */
static int
_tme_nec765_tlb_fill(void *_nec765,
		     struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address,
		     unsigned int cycles)
{
  struct tme_nec765 *nec765;

  /* recover our data structures: */
  nec765 = (struct tme_nec765 *) _nec765;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers only this address: */
  /* XXX FIXME - this is because we're lazy and don't want to write
     the bus cycle handler to size cycles down to a byte: */
  tlb->tme_bus_tlb_addr_first = address;
  tlb->tme_bus_tlb_addr_last = address;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = nec765;
  tlb->tme_bus_tlb_cycle = _tme_nec765_bus_cycle;

  return (TME_OK);
}

/* the new nec765 function: */
static int
_tme_nec765_new(struct tme_element *element,
		const char * const *args,
		const void *extra,
		char **_output,
		unsigned int part)
{
  const struct tme_nec765_socket *socket;
  struct tme_nec765_socket socket_real;
  struct tme_nec765 *nec765;
  int arg_i;
  int usage;
  const char *type;

  /* dispatch on our socket version: */
  socket = (const struct tme_nec765_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_nec765_socket_version) {
  case TME_NEC765_SOCKET_0:
    socket_real = *socket;
    break;
  default: 
    tme_output_append_error(_output, _("socket type"));
    return (EOPNOTSUPP);
  }

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  type = NULL;
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

  /* start the nec765 structure: */
  nec765 = tme_new0(struct tme_nec765, 1);
  tme_mutex_init(&nec765->tme_nec765_mutex);
  nec765->tme_nec765_part = part;
  nec765->tme_nec765_socket = socket_real;
  nec765->tme_nec765_element = element;
  _tme_nec765_reset(nec765, TRUE);

  /* initialize our simple bus device descriptor: */
  nec765->tme_nec765_device.tme_bus_device_tlb_fill = _tme_nec765_tlb_fill;
  nec765->tme_nec765_device.tme_bus_device_address_last
    = (TME_NEC765_IS_NEC765(nec765)
       ? TME_NEC765_SIZ_REG_NEC765
       : TME_NEC765_IS_I82072(nec765)
       ? TME_NEC765_SIZ_REG_I82072
       : TME_NEC765_SIZ_REG_I82077) - 1;

  /* fill the element: */
  element->tme_element_private = nec765;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}

TME_ELEMENT_X_NEW_DECL(tme_ic_,nec765,i82072) {
  return (_tme_nec765_new(element, args, extra, _output, TME_NEC765_PART_I82072));
}

TME_ELEMENT_X_NEW_DECL(tme_ic_,nec765,i82077) {
  return (_tme_nec765_new(element, args, extra, _output, TME_NEC765_PART_I82077));
}
