/* $Id: ncr53c9x.c,v 1.5 2010/06/05 15:50:40 fredette Exp $ */

/* ic/ncr53c9x.c - implementation of NCR 53c9x emulation: */

/*
 * Copyright (c) 2005, 2006 Matt Fredette
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
_TME_RCSID("$Id: ncr53c9x.c,v 1.5 2010/06/05 15:50:40 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/scsi.h>
#include <sys/time.h>

/* TODO: */

/* _tme_ncr53c9x_reset() needs to know how to reset the device entirely,
   needs a set of flags or values to distinguish different reset types */

/* XXX FIXME - we need to call _tme_ncr53c9x_disconnect(ncr53c9x)
   when all commands that may disconnect from the bus do so, since normal
   command completion does not output a zero scsi_control.  for example,
   more error cases during selection probably need this */

/* macros: */

/* NCR 53c9x variants: */
#define TME_NCR53C9X_VARIANT_NULL	(0)
#define TME_NCR53C9X_VARIANT_ESP100	(1)
#define TME_NCR53C9X_VARIANT_ESP100A	(2)

/* most registers are not read-write: */
#define TME_NCR53C9X_REG_RO(x)		(x)
#define TME_NCR53C9X_REG_WO(x)		((x) + TME_NCR53C9X_SIZ_REGS)
#define TME_NCR53C9X_REG_RW(x)		TME_NCR53C9X_REG_RO(x)
#define TME_NCR53C9X_REG_INDEX(x)	((x) % TME_NCR53C9X_SIZ_REGS)

/* register offsets: */
#define TME_NCR53C9X_REG_CTC_LSB	TME_NCR53C9X_REG_RO(0x0)
#define TME_NCR53C9X_REG_CTC_MSB	TME_NCR53C9X_REG_RO(0x1)
#define TME_NCR53C9X_REG_STC_LSB	TME_NCR53C9X_REG_WO(0x0)
#define TME_NCR53C9X_REG_STC_MSB	TME_NCR53C9X_REG_WO(0x1)
#define TME_NCR53C9X_REG_FIFO		TME_NCR53C9X_REG_RW(0x2)
#define TME_NCR53C9X_REG_CMD		TME_NCR53C9X_REG_RW(0x3)
#define TME_NCR53C9X_REG_STAT		TME_NCR53C9X_REG_RO(0x4)
#define TME_NCR53C9X_REG_SDID		TME_NCR53C9X_REG_WO(0x4)
#define TME_NCR53C9X_REG_INST		TME_NCR53C9X_REG_RO(0x5)
#define TME_NCR53C9X_REG_TIMEOUT	TME_NCR53C9X_REG_WO(0x5)
#define TME_NCR53C9X_REG_IS		TME_NCR53C9X_REG_RO(0x6)
#define TME_NCR53C9X_REG_SYNCH_PERIOD	TME_NCR53C9X_REG_WO(0x6)
#define TME_NCR53C9X_REG_CFIS		TME_NCR53C9X_REG_RO(0x7)
#define TME_NCR53C9X_REG_SYNCH_OFFSET	TME_NCR53C9X_REG_WO(0x7)
#define TME_NCR53C9X_REG_CONTROL1	TME_NCR53C9X_REG_RW(0x8)
#define TME_NCR53C9X_REG_CLOCK_FACTOR	TME_NCR53C9X_REG_WO(0x9)
#define TME_NCR53C9X_REG_TEST		TME_NCR53C9X_REG_WO(0xa)
#define TME_NCR53C9X_REG_CONTROL2	TME_NCR53C9X_REG_RW(0xb)
#define TME_NCR53C9X_REG_CONTROL3	TME_NCR53C9X_REG_RW(0xc)
#define TME_NCR53C9X_REG_ALIGN		TME_NCR53C9X_REG_WO(0xf)
#define TME_NCR53C9X_SIZ_REGS		(0x10)

/* a few registers are read/write: */
#define TME_NCR53C9X_REGS_RW					\
  (TME_BIT(TME_NCR53C9X_REG_INDEX(TME_NCR53C9X_REG_FIFO))	\
   | TME_BIT(TME_NCR53C9X_REG_INDEX(TME_NCR53C9X_REG_CMD))	\
   | TME_BIT(TME_NCR53C9X_REG_INDEX(TME_NCR53C9X_REG_CONTROL1))	\
   | TME_BIT(TME_NCR53C9X_REG_INDEX(TME_NCR53C9X_REG_CONTROL2))	\
   | TME_BIT(TME_NCR53C9X_REG_INDEX(TME_NCR53C9X_REG_CONTROL3)))

/* fields in the Command Register: */
#define TME_NCR53C9X_CMD_MASK		(0x7f)
#define  TME_NCR53C9X_CMD_NOP		 (0x00)
#define  TME_NCR53C9X_CMD_CLEAR_FIFO	 (0x01)
#define  TME_NCR53C9X_CMD_RESET		 (0x02)
#define  TME_NCR53C9X_CMD_RESET_BUS	 (0x03)
#define  TME_NCR53C9X_CMD_DMA_STOP	 (0x04)
#define  TME_NCR53C9X_CMD_TRANSFER	 (0x10)
#define  TME_NCR53C9X_CMD_ICCS		 (0x11)
#define  TME_NCR53C9X_CMD_MSG_ACCEPTED	 (0x12)
#define  TME_NCR53C9X_CMD_TRANSFER_PAD	 (0x18)
#define  TME_NCR53C9X_CMD_ATN_SET	 (0x1a)
#define  TME_NCR53C9X_CMD_ATN_RESET	 (0x1b)
#define  TME_NCR53C9X_CMD_SEND_MSG	 (0x20)
#define  TME_NCR53C9X_CMD_SEND_STATUS	 (0x21)
#define  TME_NCR53C9X_CMD_SEND_DATA	 (0x22)
#define  TME_NCR53C9X_CMD_DISCONNECT_ST	 (0x23)
#define  TME_NCR53C9X_CMD_TERMINATE	 (0x24)
#define  TME_NCR53C9X_CMD_TCCS		 (0x25)
#define  TME_NCR53C9X_CMD_DISCONNECT	 (0x27)
#define  TME_NCR53C9X_CMD_RECV_MSG	 (0x28)
#define  TME_NCR53C9X_CMD_RECV_CMD	 (0x29)
#define  TME_NCR53C9X_CMD_RECV_DATA	 (0x2a)
#define  TME_NCR53C9X_CMD_RCS		 (0x2b)
#define  TME_NCR53C9X_CMD_RESELECT	 (0x40)
#define  TME_NCR53C9X_CMD_SELECT	 (0x41)
#define  TME_NCR53C9X_CMD_SELECT_ATN	 (0x42)
#define  TME_NCR53C9X_CMD_SELECT_ATN_STOP (0x43)
#define  TME_NCR53C9X_CMD_SELECT_ENABLE	 (0x44)
#define  TME_NCR53C9X_CMD_SELECT_DISABLE (0x45)
#define  TME_NCR53C9X_CMD_SELECT_ATN3	 (0x46)
#define  TME_NCR53C9X_CMD_RESELECT3	 (0x47)
#define TME_NCR53C9X_CMD_DMA		TME_BIT(7)

/* bits in the Status register: */
#define TME_NCR53C9X_STAT_I_O		TME_BIT(0)
#define TME_NCR53C9X_STAT_C_D		TME_BIT(1)
#define TME_NCR53C9X_STAT_MSG		TME_BIT(2)
#define TME_NCR53C9X_STAT_GCV		TME_BIT(3)
#define TME_NCR53C9X_STAT_CTZ		TME_BIT(4)
#define TME_NCR53C9X_STAT_PE		TME_BIT(5)
#define TME_NCR53C9X_STAT_IOE		TME_BIT(6)
#define TME_NCR53C9X_STAT_INT		TME_BIT(7)

/* bits in the SCSI Destination ID register: */
#define TME_NCR53C9X_SDID_DID		(0x07)

/* bits in the Interrupt Status register: */
#define TME_NCR53C9X_INST_SEL		TME_BIT(0)
#define TME_NCR53C9X_INST_SELA		TME_BIT(1)
#define TME_NCR53C9X_INST_RESEL		TME_BIT(2)
#define TME_NCR53C9X_INST_SO		TME_BIT(3)
#define TME_NCR53C9X_INST_SR		TME_BIT(4)
#define TME_NCR53C9X_INST_DIS		TME_BIT(5)
#define TME_NCR53C9X_INST_ICMD		TME_BIT(6)
#define TME_NCR53C9X_INST_SRST		TME_BIT(7)

/* bits in the Internal State register: */
#define TME_NCR53C9X_IS_SOF		TME_BIT(3)

/* bits in the Current FIFO/Internal State register: */
#define TME_NCR53C9X_CFIS_IS		(0xe0)
#define TME_NCR53C9X_CFIS_CF		(0x1f)

/* bits in Control register one: */
#define TME_NCR53C9X_CONTROL1_ID	(0x07)
#define TME_NCR53C9X_CONTROL1_STE	TME_BIT(3)
#define TME_NCR53C9X_CONTROL1_PERE	TME_BIT(4)
#define TME_NCR53C9X_CONTROL1_PTE	TME_BIT(5)
#define TME_NCR53C9X_CONTROL1_DISR	TME_BIT(6)
#define TME_NCR53C9X_CONTROL1_ETM	TME_BIT(7)

/* bits in the Test register: */
#define TME_NCR53C9X_TEST_FTM		TME_BIT(0)
#define TME_NCR53C9X_TEST_FIM		TME_BIT(1)
#define TME_NCR53C9X_TEST_FHI		TME_BIT(2)

/* bits in Control register two: */
#define TME_NCR53C9X_CONTROL2_PGDP	TME_BIT(0)
#define TME_NCR53C9X_CONTROL2_PGRP	TME_BIT(1)
#define TME_NCR53C9X_CONTROL2_ACDPE	TME_BIT(2)
#define TME_NCR53C9X_CONTROL2_S2FE	TME_BIT(3)
#define TME_NCR53C9X_CONTROL2_TSDR	TME_BIT(4)
#define TME_NCR53C9X_CONTROL2_SBO	TME_BIT(5)
#define TME_NCR53C9X_CONTROL2_LSP	TME_BIT(6)
#define TME_NCR53C9X_CONTROL2_DAE	TME_BIT(7)

/* bits in Control register three: */
#define TME_NCR53C9X_CONTROL3_BS8	TME_BIT(0)
#define TME_NCR53C9X_CONTROL3_MDM	TME_BIT(1)
#define TME_NCR53C9X_CONTROL3_LBTM	TME_BIT(2)

/* predicates: */
#define TME_NCR53C9X_HAS_CONTROL2_LSP(ncr53c9x) \
  (FALSE)
#define TME_NCR53C9X_HAS_CONTROL2_DAE(ncr53c9x) \
  (FALSE)

/* major modes: */
#define TME_NCR53C9X_MODE_IDLE		(0)
#define TME_NCR53C9X_MODE_INITIATOR	(1)
#define TME_NCR53C9X_MODE_TARGET	(2)

/* special command sequence numbers: */
#define TME_NCR53C9X_CMD_SEQUENCE_DONE	(0x100)
#define TME_NCR53C9X_CMD_SEQUENCE_UNDEF	(0x101)

/* reset types: */
#define TME_NCR53C9X_RESET_FLAG_CMD	TME_BIT(0)
#define TME_NCR53C9X_RESET_WHICH	TME_BIT(1)
#define TME_NCR53C9X_RESET_DEVICE	(0 * TME_NCR53C9X_RESET_WHICH)
#define TME_NCR53C9X_RESET_BUS		(1 * TME_NCR53C9X_RESET_WHICH)

/* this value must match none of the SCSI bus phases: */
#define _TME_SCSI_PHASE_UNDEF			(TME_SCSI_PHASE_MESSAGE_IN * 2)

/* the callout flags: */
#define TME_NCR53C9X_CALLOUTS_RUNNING		TME_BIT(0)
#define TME_NCR53C9X_CALLOUTS_MASK		(-2)
#define TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA	(0x3 << 2)
#define TME_NCR53C9X_CALLOUT_TERMINAL_DMA	(0x3 << 4)
#define TME_NCR53C9X_CALLOUT_INT		(0x3 << 6)
#define TME_NCR53C9X_CALLOUT_SCSI_CYCLE		(0x3 << 8)
#define TME_NCR53C9X_CALLOUT_RUNNING(x)		((x) & ((x) << 1))

#if 1
#define TME_NCR53C9X_DEBUG
#endif

/* structures: */

/* the IC: */
struct tme_ncr53c9x {

  /* our simple bus device header: */
  struct tme_bus_device tme_ncr53c9x_device;
#define tme_ncr53c9x_element tme_ncr53c9x_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_ncr53c9x_mutex;

  /* the SCSI bus connection: */
  struct tme_scsi_connection *tme_ncr53c9x_scsi_connection;

  /* the callout flags: */
  int tme_ncr53c9x_callout_flags;

  /* the variant we are emulating: */
  unsigned int tme_ncr53c9x_variant;

  /* it's easiest to just model the registers as a chunk of memory.
     we have twice as much memory as address space, because most
     addresses read one register, and write another: */
  tme_uint8_t tme_ncr53c9x_regs[TME_NCR53C9X_SIZ_REGS * 2];

  /* the current major mode: */
  unsigned int tme_ncr53c9x_mode;

  /* the desired output SCSI cycle: */
  tme_scsi_control_t tme_ncr53c9x_out_scsi_control;
  tme_scsi_data_t tme_ncr53c9x_out_scsi_data;
  tme_uint32_t tme_ncr53c9x_out_scsi_events;
  tme_uint32_t tme_ncr53c9x_out_scsi_actions;

  /* the active output SCSI cycle: */
  tme_scsi_control_t tme_ncr53c9x_active_scsi_control;
  tme_scsi_data_t tme_ncr53c9x_active_scsi_data;
  tme_uint32_t tme_ncr53c9x_active_scsi_events;
  tme_uint32_t tme_ncr53c9x_active_scsi_actions;
  tme_uint32_t tme_ncr53c9x_active_scsi_cycle_marker;
  unsigned long tme_ncr53c9x_active_scsi_dma_resid;

  /* the last input SCSI cycle: */
  tme_scsi_control_t tme_ncr53c9x_in_scsi_control;
  tme_scsi_data_t tme_ncr53c9x_in_scsi_data;
  tme_uint32_t tme_ncr53c9x_in_scsi_events;
  tme_uint32_t tme_ncr53c9x_in_scsi_actions;

  /* if our interrupt line is currently asserted: */
  int tme_ncr53c9x_last_int_asserted;

  /* the command FIFO: */
  unsigned int tme_ncr53c9x_fifo_cmd_head;
  unsigned int tme_ncr53c9x_fifo_cmd_tail;
  tme_uint8_t tme_ncr53c9x_fifo_cmd[3];

  /* the data FIFO: */
  unsigned int tme_ncr53c9x_fifo_data_head;
  unsigned int tme_ncr53c9x_fifo_data_tail;
  tme_uint8_t tme_ncr53c9x_fifo_data[16];

  /* the status FIFO: */
  unsigned int tme_ncr53c9x_fifo_status_head;
  unsigned int tme_ncr53c9x_fifo_status_tail;
  struct {
    tme_uint8_t tme_ncr53c9x_status_stat;
    tme_uint8_t tme_ncr53c9x_status_is;
    tme_uint8_t tme_ncr53c9x_status_inst;
  } tme_ncr53c9x_fifo_status[3];

  /* the sequence number of the current command: */
  unsigned int tme_ncr53c9x_cmd_sequence;

  /* this is nonzero if DMA is running: */
  int tme_ncr53c9x_dma_running;

  /* our DMA TLB set: */
  struct tme_bus_tlb tme_ncr53c9x_dma_tlb;
  int tme_ncr53c9x_dma_tlb_added;

  /* our DMA pseudoaddress: */
  tme_bus_addr32_t tme_ncr53c9x_dma_address;

  /* if this is nonzero, a SCSI reset is detected: */
  int tme_ncr53c9x_detected_scsi_reset;

  /* the command sequence label for a SCSI BSY signal lost handler: */
  unsigned int tme_ncr53c9x_cmd_sequence_bsy_lost;

  /* the command sequence label for a SCSI bus phase mismatch handler,
     and the latched SCSI bus phase: */
  unsigned int tme_ncr53c9x_cmd_sequence_phase_mismatch;
  tme_scsi_control_t tme_ncr53c9x_latched_phase;

  /* the command sequence label for a timeout handler, the timeout
     length, and the timeout absolute time: */
  unsigned int tme_ncr53c9x_cmd_sequence_timeout;
  struct timeval tme_ncr53c9x_timeout_length;
  struct timeval tme_ncr53c9x_timeout_time;

  /* the command sequence SCSI bus transfer residual: */
  unsigned long tme_ncr53c9x_transfer_resid;

  /* if this is nonzero, this is the state in a machine that detects
     the actual SCSI bus transfer residual, based on the SCSI bus
     phase and the data being transferred: */
  tme_uint32_t tme_ncr53c9x_transfer_resid_detect_state;

  /* the timeout thread condition: */
  tme_cond_t tme_ncr53c9x_timeout_cond;
};

/* this locks the mutex: */
static void
_tme_ncr53c9x_lock(void *_ncr53c9x,
		   unsigned int locks)
{
  struct tme_ncr53c9x *ncr53c9x;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* lock the mutex: */
  tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);
}

/* this unlocks the mutex: */
static void
_tme_ncr53c9x_unlock(void *_ncr53c9x,
		     unsigned int locks)
{
  struct tme_ncr53c9x *ncr53c9x;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
}

/* this hashes an address into a TLB entry: */
static struct tme_bus_tlb *
_tme_ncr53c9x_tlb_hash(void *_ncr53c9x,
		     tme_bus_addr_t linear_address,
		     unsigned int cycles)
{
  struct tme_ncr53c9x *ncr53c9x;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* return the TLB entry: */
  return (&ncr53c9x->tme_ncr53c9x_dma_tlb);
}

#define TME_NCR53C9X_DEBUG_REG_READ	(0)
#define TME_NCR53C9X_DEBUG_REG_WRITE	(1)
#define TME_NCR53C9X_DEBUG_REG_PUT	(2)
#ifdef TME_NCR53C9X_DEBUG
static void
_tme_ncr53c9x_debug_reg(struct tme_ncr53c9x *ncr53c9x,
			unsigned int reg,
			unsigned int why,
			tme_uint8_t val_new)
{
  const char *why_name;
  const char *reg_name;

  switch (why) {
  case TME_NCR53C9X_DEBUG_REG_READ: 
    why_name = "rd"; 
    break;
  case TME_NCR53C9X_DEBUG_REG_WRITE: 
    why_name = "wr"; 
    break;
  default: assert (FALSE);
  case TME_NCR53C9X_DEBUG_REG_PUT:
    if (ncr53c9x->tme_ncr53c9x_regs[reg] == val_new) {
      return;
    }
    why_name = "<-";
    break;
  }

  switch (reg) {
  case TME_NCR53C9X_REG_CTC_LSB: reg_name = "CTC.LSB"; break;
  case TME_NCR53C9X_REG_CTC_MSB: reg_name = "CTC.MSB"; break;
  case TME_NCR53C9X_REG_STC_LSB: reg_name = "STC.LSB"; break;
  case TME_NCR53C9X_REG_STC_MSB: reg_name = "STC.MSB"; break;
  case TME_NCR53C9X_REG_FIFO: reg_name = "FIFO"; break;
  case TME_NCR53C9X_REG_CMD: reg_name = "CMD"; break;
  case TME_NCR53C9X_REG_STAT: reg_name = "STAT"; break;
  case TME_NCR53C9X_REG_SDID: reg_name = "SDID"; break;
  case TME_NCR53C9X_REG_INST: reg_name = "INST"; break;
  case TME_NCR53C9X_REG_TIMEOUT: reg_name = "TIMEOUT"; break;
  case TME_NCR53C9X_REG_IS: reg_name = "IS"; break;
  case TME_NCR53C9X_REG_SYNCH_PERIOD: reg_name = "SYNCH_PERIOD"; break;
  case TME_NCR53C9X_REG_CFIS: reg_name = "CFIS"; break;
  case TME_NCR53C9X_REG_SYNCH_OFFSET: reg_name = "SYNCH_OFFSET"; break;
  case TME_NCR53C9X_REG_CONTROL1: reg_name = "CONTROL1"; break;
  case TME_NCR53C9X_REG_CLOCK_FACTOR: reg_name = "CLOCK_FACTOR"; break;
  case TME_NCR53C9X_REG_TEST: reg_name = "TEST"; break;
  case TME_NCR53C9X_REG_CONTROL2: reg_name = "CONTROL2"; break;
  case TME_NCR53C9X_REG_CONTROL3: reg_name = "CONTROL3"; break;
  case TME_NCR53C9X_REG_ALIGN: reg_name = "ALIGN"; break;
  default: reg_name = "???"; break;
  }
  tme_log(&ncr53c9x->tme_ncr53c9x_element->tme_element_log_handle,
	  100, TME_OK,
	  (&ncr53c9x->tme_ncr53c9x_element->tme_element_log_handle,
	   "%s (0x%02x) %s 0x%02x",
	   reg_name,
	   (reg % TME_NCR53C9X_SIZ_REGS),
	   why_name,
	   val_new));
}
#define TME_NCR53C9X_DEBUG_BP(x) _TME_CONCAT(_tme_ncr53c9x_debug_bp_,x)()
#define _TME_NCR53C9X_DEBUG_BP(x) static void _TME_CONCAT(_tme_ncr53c9x_debug_bp_,x)(void) { }
_TME_NCR53C9X_DEBUG_BP(read_inst)
_TME_NCR53C9X_DEBUG_BP(read_fifo)
_TME_NCR53C9X_DEBUG_BP(write_fifo)
_TME_NCR53C9X_DEBUG_BP(write_cmd)
_TME_NCR53C9X_DEBUG_BP(dma_terminal)
_TME_NCR53C9X_DEBUG_BP(dma_scsi)
#else  /* !TME_NCR53C9X_DEBUG */
#define _tme_ncr53c9x_debug_reg(n, r, w, v) do { } while (/* CONSTCOND */ 0 && (n) && (r) && (w) && (v))
#define TME_NCR53C9X_DEBUG_BP(x) do { } while (/* CONSTCOND */ 0)
#endif /* !TME_NCR53C9X_DEBUG */
#define TME_NCR53C9X_REG_PUT(n, r, v)	\
  do {					\
    _tme_ncr53c9x_debug_reg((n), (r), TME_NCR53C9X_DEBUG_REG_PUT, (v));\
    (n)->tme_ncr53c9x_regs[(r)] = (v);	\
  } while (/* CONSTCOND */ 0)

/* this puts the current SCSI bus phase into a STAT register value: */
static inline tme_uint8_t
_tme_ncr53c9x_scsi_phase_stat(struct tme_ncr53c9x *ncr53c9x,
			      tme_uint8_t reg_stat)
{
  tme_scsi_control_t scsi_control;

  /* get the current SCSI bus control signals: */
  scsi_control = ncr53c9x->tme_ncr53c9x_in_scsi_control;

  /* clear the SCSI bus phase bits in the STAT register value: */
  reg_stat
    &= ~(TME_NCR53C9X_STAT_I_O
	 | TME_NCR53C9X_STAT_C_D
	 | TME_NCR53C9X_STAT_MSG);

  /* copy the SCSI bus control signals for the bus phase into the STAT
     register value: */
#define _TME_NCR53C9X_X_CONTROL(reg, _reg, _control)\
  do {						\
    if (scsi_control & (_control)) {		\
      reg |= (_reg);				\
    }						\
  } while (/* CONSTCOND */ 0)
  _TME_NCR53C9X_X_CONTROL(reg_stat, TME_NCR53C9X_STAT_MSG, TME_SCSI_SIGNAL_MSG);
  _TME_NCR53C9X_X_CONTROL(reg_stat, TME_NCR53C9X_STAT_C_D, TME_SCSI_SIGNAL_C_D);
  _TME_NCR53C9X_X_CONTROL(reg_stat, TME_NCR53C9X_STAT_I_O, TME_SCSI_SIGNAL_I_O);
#undef _TME_NCR53C9X_X_CONTROL

  return (reg_stat);
}

/* this clears the command FIFO: */
static inline void
_tme_ncr53c9x_fifo_cmd_clear(struct tme_ncr53c9x *ncr53c9x)
{
  unsigned int fifo_head;

  /* write a NOP command (which must be a zero) at the head of the
     command FIFO: */
#if TME_NCR53C9X_CMD_NOP != 0
#error "TME_NCR53C9X_CMD_NOP must be zero"
#endif
  fifo_head = ncr53c9x->tme_ncr53c9x_fifo_cmd_head;
  ncr53c9x->tme_ncr53c9x_fifo_cmd[fifo_head] = TME_NCR53C9X_CMD_NOP;

  /* set the tail of the command FIFO equal to the head: */
  ncr53c9x->tme_ncr53c9x_fifo_cmd_tail = fifo_head;

  /* start the NOP command at the beginning of its sequence: */
  ncr53c9x->tme_ncr53c9x_cmd_sequence = 0;
}

/* this updates the data FIFO: */
static void
_tme_ncr53c9x_fifo_data_update(struct tme_ncr53c9x *ncr53c9x)
{
  unsigned int fifo_head;
  unsigned int fifo_tail;
  unsigned int fifo_count;

  /* get the number of bytes in the FIFO: */
  fifo_head = ncr53c9x->tme_ncr53c9x_fifo_data_head;
  fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;
  if (fifo_head >= fifo_tail) {
    fifo_count = fifo_head - fifo_tail;
  }
  else {
    fifo_count = TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data) - (fifo_tail - fifo_head);
  }
    
  /* update the Current FIFO register: */
  assert (fifo_count <= TME_FIELD_MASK_EXTRACTU(TME_NCR53C9X_CFIS_CF, TME_NCR53C9X_CFIS_CF));
  TME_FIELD_MASK_DEPOSITU(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CFIS],
			  TME_NCR53C9X_CFIS_CF,
			  fifo_count);
}

/* this clears the data FIFO: */
static inline void
_tme_ncr53c9x_fifo_data_clear(struct tme_ncr53c9x *ncr53c9x)
{
  unsigned int fifo_tail;

  /* set the head of the data FIFO equal to the tail: */
  fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;
  ncr53c9x->tme_ncr53c9x_fifo_data_head = fifo_tail;

  /* put a zero at the tail of the data FIFO: */
  ncr53c9x->tme_ncr53c9x_fifo_data[fifo_tail] = 0;

  /* update the Current FIFO register: */
  _tme_ncr53c9x_fifo_data_update(ncr53c9x);
}

/* this adds the given INST register value, along with the values
   currently in the STAT and IS register images, to the status FIFO,
   and calls out an interrupt: */
static void
_tme_ncr53c9x_fifo_status_add(struct tme_ncr53c9x *ncr53c9x,
			      tme_uint8_t reg_inst)
{
  tme_uint8_t reg_stat;
  unsigned int fifo_head;

  /* get the head of the status FIFO: */
  fifo_head = ncr53c9x->tme_ncr53c9x_fifo_status_head;

  /* get the accumulated STAT image at the head of the status FIFO and
     set the INT bit: */
  /* NB: the accumulated IOE, PE, and GCV bits may have been set by
     the active command (or even by some earlier command, especially
     in the IOE case).  the CTZ bit is always live.  the SCSI bus
     phase bits are either live, or latched here: */
  reg_stat
    = (ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_stat
       | TME_NCR53C9X_STAT_INT);

  /* if the SCSI bus phase bits are latched in the STAT register: */
  if (TME_NCR53C9X_HAS_CONTROL2_LSP(ncr53c9x)
      && (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL2] & TME_NCR53C9X_CONTROL2_LSP)) {

    /* latch the current SCSI bus phase: */
    reg_stat = _tme_ncr53c9x_scsi_phase_stat(ncr53c9x, reg_stat);
  }

  /* finish the STAT and INST values at the head of the status FIFO.
     the IS value may have already been set by the active command: */
  /* NB: we also accumulate values in the INST register: */
  ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_stat = reg_stat;
  ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_inst |= reg_inst;

  /* if the status FIFO is not full: */
  fifo_head++;
  if (fifo_head == TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_status)) {
    fifo_head = 0;
  }
  if (fifo_head != ncr53c9x->tme_ncr53c9x_fifo_status_tail) {

    /* zero STAT, IS, and INST for the next status: */
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_stat = 0;
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_is = 0;
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_inst = 0;

    /* advance the head of the status FIFO: */
    ncr53c9x->tme_ncr53c9x_fifo_status_head = fifo_head;
  }

  /* call out an interrupt change: */
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_INT;
}

/* this returns the current timeout in the SCSI Timeout register, in
   milliseconds: */
static unsigned int
_tme_ncr53c9x_stimreg_msec(struct tme_ncr53c9x *ncr53c9x)
{
  /* XXX WRITEME: */
  ncr53c9x = 0;
  return (25);
}

/* this returns nonzero if the current SCSI data transfer is
   transferring in from the SCSI bus: */
static inline tme_scsi_control_t
_tme_ncr53c9x_transfer_input(const struct tme_ncr53c9x *ncr53c9x)
{
  tme_scsi_control_t scsi_control;

  /* if we're in initiator mode: */
  if (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_INITIATOR) {

    /* the bus cycle type is derived from the latched SCSI bus phase: */
    scsi_control = ncr53c9x->tme_ncr53c9x_latched_phase;
    assert (scsi_control != _TME_SCSI_PHASE_UNDEF);
  }

  /* otherwise, we must be in target mode: */
  else {
    assert (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_TARGET);

    /* the bus cycle type is derived from the output SCSI bus phase,
       and we flip the SCSI I/O control signal: */
    scsi_control = (ncr53c9x->tme_ncr53c9x_out_scsi_control ^ TME_SCSI_SIGNAL_I_O);
  }

  /* the current SCSI data transfer is transferring in from the SCSI
     bus if the SCSI I/O control signal is asserted: */
  return (scsi_control & TME_SCSI_SIGNAL_I_O);
}

/* this returns the CTC value: */
static inline tme_uint32_t
_tme_ncr53c9x_ctc_read(const struct tme_ncr53c9x *ncr53c9x)
{
  tme_uint32_t reg_ctc;

  /* get the raw value of the CTC register: */
  reg_ctc = ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CTC_MSB];
  reg_ctc = (reg_ctc << 8) + ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CTC_LSB];

  /* if the CTC register is zero, and it didn't count down to zero,
     the actual CTC value is 65536: */
  if (reg_ctc == 0
      && !(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STAT]
	   & TME_NCR53C9X_STAT_CTZ)) {
    reg_ctc = 0x10000;
  }

  return (reg_ctc);
}

/* this writes the CTC value: */
static inline void
_tme_ncr53c9x_ctc_write(struct tme_ncr53c9x *ncr53c9x, tme_uint32_t reg_ctc)
{

  /* the CTC register must fit into 16 bits: */
  assert (reg_ctc < 0x10000);
  
  /* if the CTC register counted down to zero, set CTZ: */
  if (reg_ctc == 0) {
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_STAT,
			 (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STAT]
			  | TME_NCR53C9X_STAT_CTZ));
  }

  /* put the raw value of the CTC register: */
  TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CTC_LSB, reg_ctc);
  TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CTC_MSB, (reg_ctc >> 8));
}

/* this returns the SCSI bus transfer count: */
static tme_uint32_t
_tme_ncr53c9x_transfer_count(const struct tme_ncr53c9x *ncr53c9x)
{
  tme_uint32_t reg_ctc;
  tme_uint32_t fifo_count;
  tme_uint32_t count;

  /* get the count of bytes already in the data FIFO: */
  fifo_count = TME_FIELD_MASK_EXTRACTU(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CFIS],
				       TME_NCR53C9X_CFIS_CF);

  /* if DMA is running: */
  if (ncr53c9x->tme_ncr53c9x_dma_running) {

    /* if there are bytes already in the data FIFO: */
    if (fifo_count != 0) {
      /* XXX FIXME - should we log a warning here? */
    }

    /* get the value of the CTC register: */
    reg_ctc = _tme_ncr53c9x_ctc_read(ncr53c9x);

    /* if the current SCSI data transfer is transferring in from the
       SCSI bus: */
    if (_tme_ncr53c9x_transfer_input(ncr53c9x)) {

      /* subtract the number of bytes already in the data FIFO from
	 the CTC value to get the SCSI bus transfer count: */
      if (reg_ctc < fifo_count) {
	count = 0;
      }
      else {
	count = reg_ctc - fifo_count;
      }
    }

    /* otherwise, the current SCSI data transfer is transferring out to
       the SCSI bus: */
    else {

      /* add the number of bytes already in the data FIFO to the CTC
	 value to get the maximum SCSI bus transfer count: */
      count = reg_ctc + fifo_count;
    }
  }

  /* otherwise, DMA is not running: */
  else {

    /* if the current SCSI data transfer is transferring in from the
       SCSI bus: */
    if (_tme_ncr53c9x_transfer_input(ncr53c9x)) {

      /* XXX FIXME - the AMD 53c(F?)9x documentation is all I can
	 find, and it's not well-written.  part of the 53cF94
	 documentation for the Information Transfer command reads:

	 "Upon receipt of the last byte during Msg In phase, ACK will
	 remain asserted to prevent the Target from issuing any
	 additional bytes, while the Initiator decides to accept/
	 reject the message.  If non-DMA commands are used, the last
	 byte signals the FIFO is empty."

	 this last sentence is very confusing, but it could be
	 interpreted to mean that a non-DMA transfer in from the SCSI
	 bus will transfer the number of bytes already in the data
	 FIFO (*overwriting* those bytes in the FIFO), plus one
	 additional byte, and then the command is complete.

	 this sounds very strange, so it's probably wrong.  for now,
	 we assume that a non-DMA transfer in from the SCSI bus simply
	 transfers one byte, and hope that all users only do non-DMA
	 input transfers when the FIFO is empty: */
      /* XXX FIXME - what "the last byte signals the FIFO
	 is empty" probably means is: after you read out what you know
	 to be the last byte of a message (because you know how SCSI
	 messages are structured), you can assume that the FIFO is 
	 empty. */
      
      /* transfer a single byte: */
      count = 1;
    }

    /* otherwise, the current SCSI data transfer is transferring out to
       the SCSI bus: */
    else {
      
      /* transfer all of the bytes in the data FIFO: */
      count = fifo_count;
    }
  }

  return (count);
}

/* this finishes a command: */
static void
_tme_ncr53c9x_cmd_done(struct tme_ncr53c9x *ncr53c9x)
{

  /* stop driving the SCSI data lines, and just wait for the SCSI bus
     to change without taking any actions: */
  ncr53c9x->tme_ncr53c9x_out_scsi_data = 0;
  ncr53c9x->tme_ncr53c9x_out_scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
  ncr53c9x->tme_ncr53c9x_out_scsi_actions = TME_SCSI_ACTION_NONE;

  /* call out a SCSI bus cycle: */
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;

  /* if DMA is running: */
  if (ncr53c9x->tme_ncr53c9x_dma_running) {

    /* call out the terminal DMA address: */
    ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_TERMINAL_DMA;
  }
}

/* this disconnects from the the SCSI bus: */
static void
_tme_ncr53c9x_disconnect(struct tme_ncr53c9x *ncr53c9x)
{

  /* stop driving the SCSI control lines: */
  ncr53c9x->tme_ncr53c9x_out_scsi_control = 0;

  /* finish any current command.  this will also call out a SCSI bus
     cycle: */
  _tme_ncr53c9x_cmd_done(ncr53c9x);

  /* return to the idle mode: */
  ncr53c9x->tme_ncr53c9x_mode = TME_NCR53C9X_MODE_IDLE;
}

/* this resets the NCR 53c9x: */
static void
_tme_ncr53c9x_reset(struct tme_ncr53c9x *ncr53c9x,
		    unsigned int reset_type)
{
  unsigned int fifo_tail;
  tme_uint8_t value;

  /* if this is a device reset: */
  if ((reset_type & ~TME_NCR53C9X_RESET_FLAG_CMD) == TME_NCR53C9X_RESET_DEVICE) {

    /* "The Reset Device Command immediately stops any device operation
       and resets all the functions of the device.  It returns the device
       to the disconnected state and it also generates a hard reset."

       NB: I think the above is in error, and the Reset Device command
       actually generates a soft reset.  if the above is not in error,
       then the documentation describes no way to cause a soft reset: */

    /* "[STCREG] retains its programmed value until it is overwritten
       and is not affected by hardware or software reset." */

    /* "[FFREG] is reset to zero by hardware or software reset." */
    _tme_ncr53c9x_fifo_data_clear(ncr53c9x);
    
    /* "[PE in STATREG] will be cleared by reading the Interrupt
       Status Register or by a hard or soft reset."

       "[INT in STATREG] will be cleared by a hardware or software
       reset."

       "[IOE in STATREG] will be cleared by reading the Interrupt
       Status Register or by a hard or soft reset."

       "The GCV bit [in STATREG] is cleared by reading the Interrupt
       Status Register (INSTREG) or by a hard or soft reset." */

    /* get the tail of the status FIFO: */
    fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_status_tail;

    /* set the head of the status FIFO equal to the tail: */
    ncr53c9x->tme_ncr53c9x_fifo_status_head = fifo_tail;

    /* clear STAT, IS, and INST: */
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_stat = 0;
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_is = 0;
    ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_inst = 0;

    /* call out an interrupt change: */
    ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_INT;

    /* "The DID 2:0 bits are not affected by reset." */

    /* "The STPREG defaults to five after a hard or soft reset." */
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_SYNCH_PERIOD, 5);

    /* "The SOFREG is set to zero after a hard or soft reset." */
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_SYNCH_OFFSET, 0);

    /* "The ETM bit [in CONTROL1] is reset to zero by a hard or soft
       reset."

       "The DISR bit [in CONTROL1] is reset to zero by a hard or soft
       reset."

       "[The CID bits in CONTROL1] are not affected by hard or soft
       reset."

       "The PTE bit [in CONTROL1] is reset to zero by a hard or soft
       reset."
       
       "To reset [the STE bit in CONTROL1] and to resume normal
       operation the device must be issued a hard or soft reset." */
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CONTROL1, 
			 (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL1]
			  & ~(TME_NCR53C9X_CONTROL1_ETM
			      | TME_NCR53C9X_CONTROL1_DISR
			      | TME_NCR53C9X_CONTROL1_PTE
			      | TME_NCR53C9X_CONTROL1_STE)));

    /* "The CLKF 2:0 bits will default to a value of 2 by a hard or soft reset." */
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CLOCK_FACTOR, 2);

    /* "The LSP bit [in CONTROL2] is reset by a hard or soft reset."

       "The DAE bit [in CONTROL2] is reset to zero by a hard or soft
       reset." */
    value = ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL2];
    if (TME_NCR53C9X_HAS_CONTROL2_LSP(ncr53c9x)) {
      value &= ~TME_NCR53C9X_CONTROL2_LSP;
    }
    if (TME_NCR53C9X_HAS_CONTROL2_DAE(ncr53c9x)) {
      value &= ~TME_NCR53C9X_CONTROL2_DAE;
    }
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CONTROL2, value);

    /* "The LBTM bit [in CONTROL3] is reset by hard or soft reset." */
    TME_NCR53C9X_REG_PUT(ncr53c9x, TME_NCR53C9X_REG_CONTROL3, 
			 (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL3]
			  & ~(TME_NCR53C9X_CONTROL3_LBTM)));

    /* make the Reset Device command the finished active command, and
       drop all other commands in the command queue: */
    ncr53c9x->tme_ncr53c9x_fifo_cmd[ncr53c9x->tme_ncr53c9x_fifo_cmd_tail] = TME_NCR53C9X_CMD_RESET;
    ncr53c9x->tme_ncr53c9x_cmd_sequence = TME_NCR53C9X_CMD_SEQUENCE_DONE;
    ncr53c9x->tme_ncr53c9x_fifo_cmd_head = ncr53c9x->tme_ncr53c9x_fifo_cmd_tail;
  }

  /* otherwise, this is a SCSI bus reset: */
  else {

    /* "A SCSI bus reset during any target command will cause the device
       to abort the command sequence, flag a SCSI bus reset interrupt
       (if the interrupt is enabled) and disconnect from the SCSI bus."  */

    /* we assume that the command FIFO should be cleared: */
    _tme_ncr53c9x_fifo_cmd_clear(ncr53c9x);
  }

  /* disconnect from the SCSI bus: */
  _tme_ncr53c9x_disconnect(ncr53c9x);
}

/* command sequence macros and functions: */

/* this labels an unknown default point in the sequence: */
#define _TME_NCR53C9X_CS_DEFAULT					\
  default: assert(FALSE)

/* this labels a point in the sequence: */
#define _TME_NCR53C9X_CS(x)						\
  cmd_sequence = (x);							\
  /* FALLTHROUGH */							\
  case (x)

/* this simply waits in the sequence: */
#define _TME_NCR53C9X_CS_WAIT						\
  if (TRUE)								\
    break

/* this does a goto in the sequence: */
#define _TME_NCR53C9X_CS_GOTO(x)					\
  cmd_sequence = (x);							\
  _TME_NCR53C9X_CS_WAIT

/* this finishes the sequence: */
#define _TME_NCR53C9X_CS_DONE						\
  _TME_NCR53C9X_CS_GOTO(TME_NCR53C9X_CMD_SEQUENCE_DONE)

/* this sets the IS register: */
#define _TME_NCR53C9X_CS_IS(reg_is)					\
  ncr53c9x->tme_ncr53c9x_fifo_status[ncr53c9x->tme_ncr53c9x_fifo_status_head].tme_ncr53c9x_status_is = (reg_is)

/* this generates an interrupt: */
#define _TME_NCR53C9X_CS_INT(reg_inst)					\
  _tme_ncr53c9x_fifo_status_add(ncr53c9x, (reg_inst))

/* this clears the command FIFO: */
#define _TME_NCR53C9X_CS_FIFO_CMD_CLEAR					\
  _tme_ncr53c9x_fifo_cmd_clear(ncr53c9x)

/* this checks the current major mode: */
/* "If the device is not in the initiator mode and an initiator
   command is received the device will ignore the command, generate an
   illegal command interrupt and clear the Command Register (CMDREG)
   03H." */
#define _TME_NCR53C9X_CS_MODE(mode)					\
  if (ncr53c9x->tme_ncr53c9x_mode != (mode)) {				\
    _TME_NCR53C9X_CS_FIFO_CMD_CLEAR;					\
    _TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_ICMD);			\
    _TME_NCR53C9X_CS_DONE;						\
  }									\
  do { } while (/* CONSTCOND */ 0)

/* this drives the SCSI bus: */
#define _TME_NCR53C9X_CS_SCSI_OUT(control)				\
  ncr53c9x->tme_ncr53c9x_out_scsi_control = (control);			\
  ncr53c9x->tme_ncr53c9x_out_scsi_data = 0;				\
  ncr53c9x->tme_ncr53c9x_out_scsi_events = TME_SCSI_EVENT_BUS_CHANGE;	\
  ncr53c9x->tme_ncr53c9x_out_scsi_actions = TME_SCSI_ACTION_NONE;	\
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE

/* this waits for us to drive the SCSI bus, and then waits for a value
   on the SCSI bus: */
#define _TME_NCR53C9X_CS_WAIT_SCSI(control_mask, control)		\
  if ((((ncr53c9x)->tme_ncr53c9x_callout_flags				\
	& TME_NCR53C9X_CALLOUT_SCSI_CYCLE) != 0)			\
      || ((ncr53c9x->tme_ncr53c9x_in_scsi_control & (control_mask))	\
	    != (control)))						\
    break
#define _TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(control)			\
  _TME_NCR53C9X_CS_WAIT_SCSI(control, control)
#define _TME_NCR53C9X_CS_WAIT_SCSI_NEGATED(control)			\
  _TME_NCR53C9X_CS_WAIT_SCSI(control, 0)

/* this selects on the SCSI bus: */
#define _TME_NCR53C9X_CS_SELECT(action)					\
  ncr53c9x->tme_ncr53c9x_out_scsi_data = 0;				\
  ncr53c9x->tme_ncr53c9x_out_scsi_events = TME_SCSI_EVENT_BUS_FREE;	\
  ncr53c9x->tme_ncr53c9x_out_scsi_actions				\
    = ((action)								\
       | TME_SCSI_ACTION_ID_SELF(TME_FIELD_MASK_EXTRACTU(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL1], \
							 TME_NCR53C9X_CONTROL1_ID)) \
       | TME_SCSI_ACTION_ID_OTHER(TME_FIELD_MASK_EXTRACTU(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_SDID], \
							  TME_NCR53C9X_SDID_DID))); \
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE

/* this waits for us to select on the SCSI bus: */
#define _TME_NCR53C9X_CS_WAIT_SELECT					\
  if ((((ncr53c9x)->tme_ncr53c9x_callout_flags				\
	& TME_NCR53C9X_CALLOUT_SCSI_CYCLE) != 0)			\
      || (ncr53c9x->tme_ncr53c9x_in_scsi_actions 			\
	  & (TME_SCSI_ACTION_SELECT					\
	     | TME_SCSI_ACTION_SELECT_WITH_ATN				\
	     | TME_SCSI_ACTION_RESELECT)) == 0)				\
    break

/* this latches the current time, plus a timeout in milliseconds: */
static void inline
_tme_ncr53c9x_cs_timeout(struct tme_ncr53c9x *ncr53c9x, unsigned int msec, unsigned int label)
{

  /* save the timeout length: */
  ncr53c9x->tme_ncr53c9x_timeout_length.tv_sec = (msec / 1000);
  ncr53c9x->tme_ncr53c9x_timeout_length.tv_usec = (msec % 1000) * 1000;

  /* get the current time: */
  gettimeofday(&ncr53c9x->tme_ncr53c9x_timeout_time, NULL);

  /* add the timeout length to get the timeout time: */
  ncr53c9x->tme_ncr53c9x_timeout_time.tv_sec += ncr53c9x->tme_ncr53c9x_timeout_length.tv_sec;
  ncr53c9x->tme_ncr53c9x_timeout_time.tv_usec += ncr53c9x->tme_ncr53c9x_timeout_length.tv_usec;
  if (ncr53c9x->tme_ncr53c9x_timeout_time.tv_usec >= 1000000) {
    ncr53c9x->tme_ncr53c9x_timeout_time.tv_usec -= 1000000;
    ncr53c9x->tme_ncr53c9x_timeout_time.tv_sec++;
  }

  /* set the timeout label: */
  ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout = label;

  /* signal the timeout thread: */
  tme_cond_notify(&ncr53c9x->tme_ncr53c9x_timeout_cond, FALSE);
}
#define _TME_NCR53C9X_CS_TIMEOUT(msec, label) _tme_ncr53c9x_cs_timeout(ncr53c9x, (msec), (label))
#define _TME_NCR53C9X_CS_TIMEOUT_CANCEL					\
  assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout != TME_NCR53C9X_CMD_SEQUENCE_UNDEF); \
  ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout = TME_NCR53C9X_CMD_SEQUENCE_UNDEF

/* this monitors the SCSI BSY signal: */
static unsigned int
_tme_ncr53c9x_cs_monitor_bsy(struct tme_ncr53c9x *ncr53c9x)
{
  tme_scsi_control_t scsi_control;
  unsigned int cmd_sequence_goto;

  /* get the current SCSI bus state: */
  scsi_control = ncr53c9x->tme_ncr53c9x_in_scsi_control;

  /* if BSY is negated: */
  if ((scsi_control & TME_SCSI_SIGNAL_BSY) == 0) {

    /* if an initiator DMA SCSI cycle is running: */
    if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE) != 0
	&& (ncr53c9x->tme_ncr53c9x_out_scsi_actions
	    & (TME_SCSI_ACTION_DMA_INITIATOR
	       | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)) != 0) {

      /* cancel the SCSI cycle: */
      ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
    }

    /* if the BSY lost label for this command sequence is the
       done sequence number: */
    if (ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost == TME_NCR53C9X_CMD_SEQUENCE_DONE) {

      /* issue a Disconnected interrupt: */
      _TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_DIS);

      /* force a disconnect: */
      _tme_ncr53c9x_disconnect(ncr53c9x);
    }

    /* goto the BSY lost label for the command sequence, and
       cancel the BSY lost label, and any phase mismatch label: */
    cmd_sequence_goto = ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost;
    ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;
    ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;
    return (cmd_sequence_goto);
  }

  return (TME_NCR53C9X_CMD_SEQUENCE_UNDEF);
}
#define _TME_NCR53C9X_CS_MONITOR_BSY(label)				\
  assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost == TME_NCR53C9X_CMD_SEQUENCE_UNDEF);\
  assert (label != TME_NCR53C9X_CMD_SEQUENCE_UNDEF);			\
  ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost = (label);		\
  cmd_sequence_goto = _tme_ncr53c9x_cs_monitor_bsy(ncr53c9x);		\
  if (cmd_sequence_goto != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {		\
    _TME_NCR53C9X_CS_GOTO(cmd_sequence_goto);				\
  }									\
  do { } while (/* CONSTCOND */ 0)

/* this monitors the SCSI bus phase: */
static unsigned int
_tme_ncr53c9x_cs_monitor_phase(struct tme_ncr53c9x *ncr53c9x, int force_monitor)
{
  tme_scsi_control_t scsi_control;
  unsigned int cmd_sequence_goto;

  /* get the current SCSI bus state: */
  scsi_control = ncr53c9x->tme_ncr53c9x_in_scsi_control;

  /* BSY must be asserted: */
  assert ((scsi_control & TME_SCSI_SIGNAL_BSY) != 0);

  /* if the command sequence SCSI bus transfer residual is nonzero,
     or if we're forcing monitoring even if the residual is zero,
     REQ is asserted, and the phase is mismatched: */
  if ((ncr53c9x->tme_ncr53c9x_transfer_resid != 0
       || force_monitor)
      && (scsi_control & TME_SCSI_SIGNAL_REQ) != 0
      && TME_SCSI_PHASE(scsi_control) != ncr53c9x->tme_ncr53c9x_latched_phase) {

    /* if an initiator DMA SCSI cycle is running: */
    if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE) != 0
	&& (ncr53c9x->tme_ncr53c9x_out_scsi_actions
	    & (TME_SCSI_ACTION_DMA_INITIATOR
	       | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)) != 0) {

      /* cancel the SCSI cycle: */
      ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
    }

    /* goto the phase mismatch label for the command sequence,
       and cancel the phase latch: */
    cmd_sequence_goto = ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch;
    ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;
    return (cmd_sequence_goto);
  }

  return (TME_NCR53C9X_CMD_SEQUENCE_UNDEF);
}
#define _TME_NCR53C9X_CS_LATCH_PHASE(phase)				\
  assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch == TME_NCR53C9X_CMD_SEQUENCE_UNDEF);\
  ncr53c9x->tme_ncr53c9x_latched_phase = (phase)
#define _TME_NCR53C9X_CS_MONITOR_PHASE(label)				\
  assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch == TME_NCR53C9X_CMD_SEQUENCE_UNDEF);\
  assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost != TME_NCR53C9X_CMD_SEQUENCE_UNDEF);\
  assert (label != TME_NCR53C9X_CMD_SEQUENCE_UNDEF);			\
  ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch = (label);		\
  cmd_sequence_goto = _tme_ncr53c9x_cs_monitor_phase(ncr53c9x, TRUE);	\
  if (cmd_sequence_goto != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {		\
    _TME_NCR53C9X_CS_GOTO(cmd_sequence_goto);				\
  }									\
  do { } while (/* CONSTCOND */ 0)
#define _TME_NCR53C9X_CS_UNMONITOR_PHASE				\
  ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch = TME_NCR53C9X_CMD_SEQUENCE_UNDEF

/* this starts a data transfer: */
#define _TME_NCR53C9X_CS_TRANSFER(action, count)			\
  assert (ncr53c9x->tme_ncr53c9x_transfer_resid == 0);			\
  ncr53c9x->tme_ncr53c9x_out_scsi_events = TME_SCSI_EVENT_NONE;		\
  ncr53c9x->tme_ncr53c9x_out_scsi_actions = (action);			\
  ncr53c9x->tme_ncr53c9x_transfer_resid = (count);			\
  assert (ncr53c9x->tme_ncr53c9x_transfer_resid > 0);			\
  ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state = 0;		\
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE

/* this waits for a data transfer to finish: */
#define _TME_NCR53C9X_CS_WAIT_TRANSFER					\
  if (ncr53c9x->tme_ncr53c9x_transfer_resid != 0) 			\
    break

/* if this is a DMA command, this copies the Start Transfer Count
   register into the Current Transfer Count register: */
static void
_tme_ncr53c9x_cs_dma_setup(struct tme_ncr53c9x *ncr53c9x,
			   unsigned int cmd)
{
 
  /* if this command is a DMA command: */
  if ((cmd & TME_NCR53C9X_CMD_DMA) != 0) {

    /* copy the Start Transfer Count register into the Current
       Transfer Count register: */
    TME_NCR53C9X_REG_PUT(ncr53c9x, 
			 TME_NCR53C9X_REG_CTC_LSB,
			 ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STC_LSB]);
    TME_NCR53C9X_REG_PUT(ncr53c9x, 
			 TME_NCR53C9X_REG_CTC_MSB,
			 ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STC_MSB]);

    /* clear CTZ: */
    TME_NCR53C9X_REG_PUT(ncr53c9x, 
			 TME_NCR53C9X_REG_STAT,
			 (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STAT]
			  & ~TME_NCR53C9X_STAT_CTZ));

    /* zero the DMA address: */
    ncr53c9x->tme_ncr53c9x_dma_address = 0;

    /* DMA is running: */
    ncr53c9x->tme_ncr53c9x_dma_running = TRUE;
  }
}
#define _TME_NCR53C9X_CS_DMA_SETUP _tme_ncr53c9x_cs_dma_setup(ncr53c9x, cmd)

/* this is the NCR 53c9x update function: */
static void
_tme_ncr53c9x_update(struct tme_ncr53c9x *ncr53c9x)
{
  unsigned int fifo_tail;
  unsigned int cmd;
  unsigned int cmd_sequence;
  unsigned int cmd_sequence_goto;
  tme_scsi_control_t scsi_control;
  tme_scsi_data_t scsi_data;
  tme_scsi_control_t phase;
  tme_scsi_data_t ids;
  struct timeval now;
  tme_uint32_t count;

  /* loop forever: */
  for (;;) {

    /* get the command at the tail of the command FIFO: */
    fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_cmd_tail;
    cmd = ncr53c9x->tme_ncr53c9x_fifo_cmd[fifo_tail];
    cmd_sequence = ncr53c9x->tme_ncr53c9x_cmd_sequence;

    /* if the command sequence number is zero: */
    if (cmd_sequence == 0) {

      /* the SCSI data bus must not be driven, and we must be waiting
	 for any SCSI bus change, without taking any actions: */
      assert (ncr53c9x->tme_ncr53c9x_out_scsi_data == 0);
      assert (ncr53c9x->tme_ncr53c9x_out_scsi_events == TME_SCSI_EVENT_BUS_CHANGE);
      assert (ncr53c9x->tme_ncr53c9x_out_scsi_actions == TME_SCSI_ACTION_NONE);

      /* DMA must not be running yet: */
      assert (!ncr53c9x->tme_ncr53c9x_dma_running);

      /* clear the input SCSI events and actions: */
      ncr53c9x->tme_ncr53c9x_in_scsi_events = TME_SCSI_EVENT_NONE;
      ncr53c9x->tme_ncr53c9x_in_scsi_actions = TME_SCSI_ACTION_NONE;

      /* no data is being transferred over the SCSI bus: */
      ncr53c9x->tme_ncr53c9x_transfer_resid = 0;

      /* no timeout is set: */
      ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;

      /* no SCSI bus phase is latched: */
      ncr53c9x->tme_ncr53c9x_latched_phase = _TME_SCSI_PHASE_UNDEF;
      
      /* the SCSI bus phase is not being monitored: */
      ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;

      /* if we're in initiator mode: */
      if (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_INITIATOR) {
	
	/* monitor the SCSI BSY signal: */
	ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost = TME_NCR53C9X_CMD_SEQUENCE_DONE;
      }

      /* otherwise, we're not in initiator mode: */
      else {

	/* don't monitor the SCSI BSY signal: */
	ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;
      }
    }

    /* get the current SCSI bus state: */
    scsi_control = ncr53c9x->tme_ncr53c9x_in_scsi_control;
    scsi_data = ncr53c9x->tme_ncr53c9x_in_scsi_data;

    /* if RST is being asserted: */
    if ((scsi_control & TME_SCSI_SIGNAL_RST) != 0) {

      /* if this SCSI reset hasn't already been detected: */
      if (!ncr53c9x->tme_ncr53c9x_detected_scsi_reset) {

	/* this SCSI reset has been detected: */
	ncr53c9x->tme_ncr53c9x_detected_scsi_reset = TRUE;

	/* if SCSI reset reporting hasn't been disabled: */
	if (!(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL1]
	      & TME_NCR53C9X_CONTROL1_DISR)) {

	  /* issue a SCSI reset interrupt: */
	  _TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SRST);
	}

	/* if the active command isn't a bus reset: */
	if ((cmd & TME_NCR53C9X_CMD_MASK) != TME_NCR53C9X_CMD_RESET_BUS
	    || cmd_sequence == TME_NCR53C9X_CMD_SEQUENCE_DONE) {

	  /* do an external SCSI reset: */
	  _tme_ncr53c9x_reset(ncr53c9x, TME_NCR53C9X_RESET_BUS);

	  /* stop now: */
	  break;
	}
      }
    }

    /* otherwise, RST is not being asserted: */
    else {

      /* no SCSI reset is detected: */
      ncr53c9x->tme_ncr53c9x_detected_scsi_reset = FALSE;
    }

    /* if we are idle: */
    if (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_IDLE) {

      /* if we're not selecting or reselecting, and we are being
         selected or reselected: */
      ids = (1 << TME_FIELD_MASK_EXTRACTU(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL1],
					  TME_NCR53C9X_CONTROL1_ID));
      if ((ncr53c9x->tme_ncr53c9x_in_scsi_actions
	   & (TME_SCSI_ACTION_SELECT
	      | TME_SCSI_ACTION_SELECT_WITH_ATN
	      | TME_SCSI_ACTION_RESELECT)) == 0
	  && (TME_SCSI_ID_SELECTED(ids, scsi_control, scsi_data)
	      || TME_SCSI_ID_RESELECTED(ids, scsi_control, scsi_data))) {

	/* if selection and reselection are enabled: */
	/* XXX WRITEME: */
	abort();
      }
    }

    /* if the command at the tail of the command FIFO is not done: */
    if (cmd_sequence != TME_NCR53C9X_CMD_SEQUENCE_DONE) {

      /* if the SCSI BSY signal is monitored: */
      if (ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {

	/* we must be in initiator mode: */
	assert (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_INITIATOR);

	/* monitor the SCSI BSY signal: */
	cmd_sequence_goto = _tme_ncr53c9x_cs_monitor_bsy(ncr53c9x);
	if (cmd_sequence_goto != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {
	  cmd_sequence = cmd_sequence_goto;
	}
      }

      /* if the SCSI bus phase is monitored: */
      if (ncr53c9x->tme_ncr53c9x_cmd_sequence_phase_mismatch != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {

	/* we must be in initiator mode: */
	assert (ncr53c9x->tme_ncr53c9x_mode == TME_NCR53C9X_MODE_INITIATOR);

	/* the SCSI BSY signal must be monitored: */
	assert (ncr53c9x->tme_ncr53c9x_cmd_sequence_bsy_lost != TME_NCR53C9X_CMD_SEQUENCE_UNDEF);

	/* monitor the SCSI bus phase: */
	cmd_sequence_goto = _tme_ncr53c9x_cs_monitor_phase(ncr53c9x, FALSE);
	if (cmd_sequence_goto != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {
	  cmd_sequence = cmd_sequence_goto;
	}
      }

      /* if a timeout is set: */
      if (ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {

	/* get the current time: */
	gettimeofday(&now, NULL);

	/* if the timeout has expired: */
	if (now.tv_sec > ncr53c9x->tme_ncr53c9x_timeout_time.tv_sec
	    || (now.tv_sec == ncr53c9x->tme_ncr53c9x_timeout_time.tv_sec
		&& now.tv_usec >= ncr53c9x->tme_ncr53c9x_timeout_time.tv_usec)) {

	  /* goto the timeout label for the command sequence, and
             cancel the timeout: */
	  cmd_sequence = ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout;
	  ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout = TME_NCR53C9X_CMD_SEQUENCE_UNDEF;
	}
      }
    }

    /* if the command at the tail of the command FIFO is still not done: */
    if (cmd_sequence != TME_NCR53C9X_CMD_SEQUENCE_DONE) {

      /* dispatch on the current command: */
      switch (cmd & TME_NCR53C9X_CMD_MASK) {

	/* "The No Operation Command is used to perform no operation
	   and no interrupt is generated at the end of this command.
	   This command is issued after the Reset Device Command to
	   enable the Command Register.  A No Operation Command in the
	   DMA mode may be used to verify the contents of the Start
	   Transfer Count Register (STCREG) 00H 01H.  After the STCREG
	   is loaded with the transfer count and a No Operation
	   Command is issued, reading the Current Transfer Count
	   Register (CTCREG) 00H 01H will give the transfer count
	   value." */
      case TME_NCR53C9X_CMD_NOP:
	assert (cmd_sequence == 0);
	_TME_NCR53C9X_CS_DMA_SETUP;
	_TME_NCR53C9X_CS_DONE;

	/* "The Clear FIFO Command is used to initialize the FIFO to
	   the empty condition. The Current FIFO Register (CFISREG)
	   07H reflects the empty FIFO status and the bottom of the
	   FIFO is set to zero. No interrupt is generated at the end
	   of this command." */
      case TME_NCR53C9X_CMD_CLEAR_FIFO:
	assert (cmd_sequence == 0);
	_tme_ncr53c9x_fifo_data_clear(ncr53c9x);
	_TME_NCR53C9X_CS_DONE;

	/* these commands are handled elsewhere: */
      case TME_NCR53C9X_CMD_RESET: 
      case TME_NCR53C9X_CMD_DMA_STOP:
	assert (FALSE);
	/* FALLTHROUGH */

	/* an unknown command: */
      default:
	abort();

	/* "The Reset SCSI Bus Command is used to assert the RSTC
	   signal for approximately 25 ms. This command causes the
	   device to go to the disconnected state.  No interrupt is
	   generated upon command completion.  A SCSI reset interrupt
	   is however generated upon command completion if the
	   interrupt is not disabled in the Control Register One
	   (CNTLREG1) 08H." */
      case TME_NCR53C9X_CMD_RESET_BUS:
	switch (cmd_sequence) {
	  _TME_NCR53C9X_CS_DEFAULT;
	  _TME_NCR53C9X_CS(0):	_TME_NCR53C9X_CS_SCSI_OUT(TME_SCSI_SIGNAL_RST);
	  _TME_NCR53C9X_CS(1):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_RST);
				_TME_NCR53C9X_CS_TIMEOUT(25, 3);
	  _TME_NCR53C9X_CS(2):	_TME_NCR53C9X_CS_WAIT;
	  _TME_NCR53C9X_CS(3):	_TME_NCR53C9X_CS_SCSI_OUT(0);
				_tme_ncr53c9x_disconnect(ncr53c9x);
				_TME_NCR53C9X_CS_DONE;
	}
	break;

	/* "The Information Transfer command is used to transfer
	   information bytes over the SCSI bus. This command may be
	   issued during any SCSI Information Transfer phase.
	   Information transfer for synchronous data must use the
	   DMA mode." */
      case TME_NCR53C9X_CMD_TRANSFER:
	switch (cmd_sequence) {
	  _TME_NCR53C9X_CS_DEFAULT;
	  _TME_NCR53C9X_CS(0):	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_INITIATOR);
				_TME_NCR53C9X_CS_DMA_SETUP;
	  _TME_NCR53C9X_CS(1):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_LATCH_PHASE(TME_SCSI_PHASE(scsi_control));
				_TME_NCR53C9X_CS_MONITOR_PHASE(10);
				count = _tme_ncr53c9x_transfer_count(ncr53c9x);
				if (count == 0) {
				  _TME_NCR53C9X_CS_GOTO(6);
				}
				phase = ncr53c9x->tme_ncr53c9x_latched_phase;
				if (phase == TME_SCSI_PHASE_MESSAGE_OUT) {
				  count--;
				  if (count == 0) {
				    _TME_NCR53C9X_CS_GOTO(3);
				  }
				}
				_TME_NCR53C9X_CS_TRANSFER((phase == TME_SCSI_PHASE_MESSAGE_IN
							   ? TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK
							   : TME_SCSI_ACTION_DMA_INITIATOR),
							  count);
	  _TME_NCR53C9X_CS(2):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
				phase = ncr53c9x->tme_ncr53c9x_latched_phase;
				if (phase == TME_SCSI_PHASE_MESSAGE_IN) {
				  _TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SO);
				  _TME_NCR53C9X_CS_DONE;
				}
				_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				if (phase != TME_SCSI_PHASE_MESSAGE_OUT) {
				  _TME_NCR53C9X_CS_GOTO(6);
				}
	  _TME_NCR53C9X_CS(3):	_TME_NCR53C9X_CS_SCSI_OUT(ncr53c9x->tme_ncr53c9x_out_scsi_control & ~TME_SCSI_SIGNAL_ATN);
	  _TME_NCR53C9X_CS(4):	_TME_NCR53C9X_CS_WAIT_SCSI_NEGATED(TME_SCSI_SIGNAL_ATN);
				_TME_NCR53C9X_CS_TRANSFER(TME_SCSI_ACTION_DMA_INITIATOR, 1);
	  _TME_NCR53C9X_CS(5):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
				_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
	  _TME_NCR53C9X_CS(6):	_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR);
				_TME_NCR53C9X_CS_DONE;

	  /* "The target changes the SCSI bus phase before the
	     expected number of bytes are transferred. The device
	     clears the Command Register (CMDREG) 03H, and generates a
	     service interrupt when the target asserts REQ." */
	  /* NB: we have to clear the command FIFO last; once we do,
	     we can't CS_WAIT on anything or CS_GOTO, since our own
	     command will have been cleared: */
	  _TME_NCR53C9X_CS(10):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR);
				_TME_NCR53C9X_CS_FIFO_CMD_CLEAR;
				_TME_NCR53C9X_CS_DONE;	  
	}
	break;

	/* "The Initiator Command Complete Steps command is normally
	   issued when the SCSI bus is in the Status In phase.  One
	   Status byte followed by one Message byte is transferred if
	   this command completes normally.  After receiving the
	   message byte the device will keep the ACK signal asserted
	   to allow the initiator to examine the message and assert
	   the ATN signal if it is unacceptable." */
      case TME_NCR53C9X_CMD_ICCS:
	switch (cmd_sequence) {
	  _TME_NCR53C9X_CS_DEFAULT;
	  _TME_NCR53C9X_CS(0):	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_INITIATOR);
				_TME_NCR53C9X_CS_DMA_SETUP;
          _TME_NCR53C9X_CS(1):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_LATCH_PHASE(TME_SCSI_PHASE(scsi_control));
				_TME_NCR53C9X_CS_MONITOR_PHASE(10);
				_TME_NCR53C9X_CS_TRANSFER(TME_SCSI_ACTION_DMA_INITIATOR, 1);
          _TME_NCR53C9X_CS(2):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
				_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_UNMONITOR_PHASE;
				_TME_NCR53C9X_CS_LATCH_PHASE(TME_SCSI_PHASE_MESSAGE_IN);
          _TME_NCR53C9X_CS(3):	_TME_NCR53C9X_CS_MONITOR_PHASE(5);
				_TME_NCR53C9X_CS_TRANSFER(TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK, 1);
          _TME_NCR53C9X_CS(4):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
          _TME_NCR53C9X_CS(5):	_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SO);
				_TME_NCR53C9X_CS_DONE;

	  /* a phase mismatch: */
          _TME_NCR53C9X_CS(10):	_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR);
				_TME_NCR53C9X_CS_DONE;
	}
	break;

	/* "The Message Accepted Command is used to release the ACK
	   signal. This command is normally used to complete a Message
	   In handshake.  Upon execution of this command the device
	   generates a service request interrupt after REQ is asserted
	   by the target." */
	/* NB: the target usually disconnects instead of asserting REQ
	   again, because most Message In phases are at the end of
	   completed commands.  in this case, the default initiator
	   mode BSY monitoring will issue a disconnected interrupt: */
      case TME_NCR53C9X_CMD_MSG_ACCEPTED:
	switch (cmd_sequence) {
	  _TME_NCR53C9X_CS_DEFAULT;
          _TME_NCR53C9X_CS(0):	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_INITIATOR);
          _TME_NCR53C9X_CS(1):	_TME_NCR53C9X_CS_WAIT_SCSI_NEGATED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_SCSI_OUT(ncr53c9x->tme_ncr53c9x_out_scsi_control & ~TME_SCSI_SIGNAL_ACK);
          _TME_NCR53C9X_CS(2):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
	  			_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR);
				_TME_NCR53C9X_CS_DONE;
	}
	break;

	/* XXX FIXME - we don't support the Transfer Pad command yet: */
      case TME_NCR53C9X_CMD_TRANSFER_PAD:
	abort();

	/* "The Set ATN Command is used to drive the ATN signal active
	   on the SCSI bus.  An interrupt is not generated at the end
	   of this command." */
      case TME_NCR53C9X_CMD_ATN_SET:
	assert (cmd_sequence == 0);
	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_INITIATOR);
	_TME_NCR53C9X_CS_SCSI_OUT(ncr53c9x->tme_ncr53c9x_out_scsi_control | TME_SCSI_SIGNAL_ATN);
	_TME_NCR53C9X_CS_DONE;

	/* "The Reset ATN Command is used to deassert the ATN signal
	   on the SCSI bus. An interrupt is not generated at the end
	   of this command." */
      case TME_NCR53C9X_CMD_ATN_RESET:
	assert (cmd_sequence == 0);
	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_INITIATOR);
	_TME_NCR53C9X_CS_SCSI_OUT(ncr53c9x->tme_ncr53c9x_out_scsi_control & ~TME_SCSI_SIGNAL_ATN);
	_TME_NCR53C9X_CS_DONE;

	/* "The Select without ATN Steps Command is used by the
	   initiator to select a target.  When this command is issued
	   the device arbitrates for the control of the SCSI bus.
	   When the device wins arbitration, it selects the target
	   device and transfers the Command Descriptor Block (CDB).
	   Before issuing this command the SCSI Timeout Register
	   (STIMREG) 05H, the Control Register One (CNTLREG1) 08H and
	   the SCSI Destination ID Register (SDIDREG) 04H must be set
	   to the proper values.  If DMA is enabled, the Start
	   Transfer Count Register (STCREG) 00H 01H must be set to the
	   total length of the command.  If DMA is not enabled, the
	   data must be loaded into the FIFO before issuing this
	   command.  This command will be terminated early if the SCSI
	   Timeout Register times out or if the target does not go to
	   the Command Phase following the Selection Phase or if the
	   target exits the Command Phase early." */
      case TME_NCR53C9X_CMD_SELECT:
	/* FALLTHROUGH */

	/* "The Select with ATN Steps Command is used by the initiator
	   to select a target.  When this command is issued the device
	   arbitrates for the control of the SCSI bus.  When the
	   device wins arbitration, it selects the target device with
	   the ATN signal asserted and transfers the Command
	   Descriptor Block (CDB) and a one byte message.  Before
	   issuing this command the SCSI Timeout Register (STIMREG)
	   05H, the Control Register One (CNTLREG1) 08H and the SCSI
	   Destination ID Register (SDIDREG) 04H must be set to the
	   proper values.  If DMA is enabled, the Start Transfer Count
	   Register (STCREG) 00H 01H must be set to the total length
	   of the command.  If DMA is not enabled, the data must be
	   loaded into the FIFO before issuing this command." */
      case TME_NCR53C9X_CMD_SELECT_ATN:
	/* FALLTHROUGH */

	/* "The Select with ATN3 Steps Command is used by the
	   initiator to select a target.  This command is similar to
	   the Select with ATN Steps Command, except that it sends
	   exactly three message bytes.  When this command is issued
	   the device arbitrates for the control of the SCSI bus.
	   When the device wins arbitration, it selects the target
	   device with the ATN signal asserted and transfers the
	   Command Descriptor Block (CDB) and three message bytes.
	   Before issuing this command the SCSI Timeout Register
	   (STIMREG) 05H, the Control Register One (CNTLREG1) 08H and
	   the SCSI Destination ID Register (SDIDREG) 04H must be set
	   to the proper values.  If DMA is enabled, the Start
	   Transfer Count Register (STCREG) 00H 01H must be set to the
	   total length of the command.  If DMA is not enabled, the
	   data must be loaded into the FIFO before issuing this
	   command." */
      case TME_NCR53C9X_CMD_SELECT_ATN3:
	/* FALLTHROUGH */

	/* "The Select with ATN and Stop Steps Command is used by the
	   initiator to select a target.  When this command is issued
	   the device arbitrates for the control of the SCSI bus. When
	   the device wins arbitration, it selects the target device
	   with the ATN signal asserted and transfers the Command
	   Descriptor Block (CDB) and stops after one message byte is
	   sent, but the ATN signal is not deasserted at the end of
	   the command which allows the initiator to send other
	   messages after the ID message is sent out. Before issuing
	   this command the SCSI Timeout Register (STIMREG) 05H, the
	   Control Register One (CNTLREG1) 08H and the SCSI
	   Destination ID Register (SDIDREG) 04H must be set to the
	   proper values." */
      case TME_NCR53C9X_CMD_SELECT_ATN_STOP:
	/* FALLTHROUGH */

	switch (cmd_sequence) {
	  _TME_NCR53C9X_CS_DEFAULT;
          _TME_NCR53C9X_CS(0):	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_IDLE);
				_TME_NCR53C9X_CS_DMA_SETUP;
				_TME_NCR53C9X_CS_SELECT((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT
							? TME_SCSI_ACTION_SELECT
							: TME_SCSI_ACTION_SELECT_WITH_ATN);
          _TME_NCR53C9X_CS(1):	_TME_NCR53C9X_CS_WAIT_SELECT;
				_TME_NCR53C9X_CS_TIMEOUT(_tme_ncr53c9x_stimreg_msec(ncr53c9x), 30);
          _TME_NCR53C9X_CS(2):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_BSY);
				_TME_NCR53C9X_CS_TIMEOUT_CANCEL;
				_TME_NCR53C9X_CS_SCSI_OUT((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT
							  ? 0
							  : TME_SCSI_SIGNAL_ATN);
          _TME_NCR53C9X_CS(3):	_TME_NCR53C9X_CS_WAIT_SCSI_NEGATED(TME_SCSI_SIGNAL_SEL);
				ncr53c9x->tme_ncr53c9x_mode = TME_NCR53C9X_MODE_INITIATOR;
				_TME_NCR53C9X_CS_MONITOR_BSY(21);
				if ((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT) {
				  _TME_NCR53C9X_CS_GOTO(10);
				}
          _TME_NCR53C9X_CS(4):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				if (TME_SCSI_PHASE(scsi_control) != TME_SCSI_PHASE_MESSAGE_OUT) {
				  _TME_NCR53C9X_CS_IS(0);
				  _TME_NCR53C9X_CS_GOTO(16);
				}
				_TME_NCR53C9X_CS_LATCH_PHASE(TME_SCSI_PHASE_MESSAGE_OUT);
				_TME_NCR53C9X_CS_MONITOR_PHASE(20);
				count = _tme_ncr53c9x_transfer_count(ncr53c9x);
				if (count == 0) {
				  abort();
				}
				if ((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT_ATN) {
				  _TME_NCR53C9X_CS_GOTO(6);
				}
				else if ((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT_ATN3) {
				  if (count < 3) {
				    abort();
				  }
				  count = 2;
				}
				else {
				  assert ((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT_ATN_STOP);
				  count = 1;
				}
				_TME_NCR53C9X_CS_TRANSFER(TME_SCSI_ACTION_DMA_INITIATOR, count);
          _TME_NCR53C9X_CS(5):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
          _TME_NCR53C9X_CS(6):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				if ((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT_ATN_STOP) {
				  _TME_NCR53C9X_CS_IS(1);
				  _TME_NCR53C9X_CS_GOTO(16);
				}
				_TME_NCR53C9X_CS_SCSI_OUT(ncr53c9x->tme_ncr53c9x_out_scsi_control & ~TME_SCSI_SIGNAL_ATN);
          _TME_NCR53C9X_CS(7):	_TME_NCR53C9X_CS_WAIT_SCSI_NEGATED(TME_SCSI_SIGNAL_ATN);
				_TME_NCR53C9X_CS_TRANSFER(TME_SCSI_ACTION_DMA_INITIATOR, 1);
          _TME_NCR53C9X_CS(8):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
	  _TME_NCR53C9X_CS(10):	_TME_NCR53C9X_CS_WAIT_SCSI_ASSERTED(TME_SCSI_SIGNAL_REQ);
				_TME_NCR53C9X_CS_UNMONITOR_PHASE;
				_TME_NCR53C9X_CS_LATCH_PHASE(TME_SCSI_PHASE_COMMAND);
				_TME_NCR53C9X_CS_MONITOR_PHASE(21);
				count = _tme_ncr53c9x_transfer_count(ncr53c9x);
				if (count == 0) {
				  abort();
				}
				_TME_NCR53C9X_CS_TRANSFER(((cmd & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_SELECT_ATN_STOP
							   ? TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK
							   : TME_SCSI_ACTION_DMA_INITIATOR),
							  count);
				ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state = 1;
	  _TME_NCR53C9X_CS(11):	_TME_NCR53C9X_CS_WAIT_TRANSFER;
				_TME_NCR53C9X_CS_IS(4);
	  _TME_NCR53C9X_CS(16):	_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR | TME_NCR53C9X_INST_SO);
				_TME_NCR53C9X_CS_DONE;

	  /* "This command will be terminated early in the following situations:" */

	  /* "The target exits the Message Phase early" */
          _TME_NCR53C9X_CS(20):	_TME_NCR53C9X_CS_IS(2);
				_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR | TME_NCR53C9X_INST_SO);
				_TME_NCR53C9X_CS_DONE;

	  /* "The target does not go to the Command Phase following the
	     Selection Phase"

	     "The target exits the Command Phase early." */
          _TME_NCR53C9X_CS(21):	_TME_NCR53C9X_CS_IS(ncr53c9x->tme_ncr53c9x_transfer_resid == 0 ? 2 : 3);
				_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SR | TME_NCR53C9X_INST_SO);
				_TME_NCR53C9X_CS_DONE;

	  /* "The SCSI Timeout Register times out" */
          _TME_NCR53C9X_CS(30):	_tme_ncr53c9x_disconnect(ncr53c9x);
				_TME_NCR53C9X_CS_IS(0);
				_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_DIS);
				_TME_NCR53C9X_CS_DONE;
	}
	break;

	/* XXX FIXME - we don't support any of the Target mode commands yet: */
      case TME_NCR53C9X_CMD_SEND_MSG:
      case TME_NCR53C9X_CMD_SEND_STATUS:
      case TME_NCR53C9X_CMD_SEND_DATA:
      case TME_NCR53C9X_CMD_DISCONNECT_ST:
      case TME_NCR53C9X_CMD_TERMINATE:
      case TME_NCR53C9X_CMD_TCCS:
      case TME_NCR53C9X_CMD_DISCONNECT:
      case TME_NCR53C9X_CMD_RECV_MSG:
      case TME_NCR53C9X_CMD_RECV_CMD:
      case TME_NCR53C9X_CMD_RECV_DATA:
      case TME_NCR53C9X_CMD_RCS:
      case TME_NCR53C9X_CMD_RESELECT:
      case TME_NCR53C9X_CMD_RESELECT3:
	abort();

	/* XXX FIXME - since we don't support any of the Target mode
	   commands yet, there isn't much to do here: */
      case TME_NCR53C9X_CMD_SELECT_ENABLE:
	assert (cmd_sequence == 0);
	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_IDLE);
	_TME_NCR53C9X_CS_DONE;

	/* XXX FIXME - since we don't support any of the Target mode
	   commands yet, there isn't much to do here: */
      case TME_NCR53C9X_CMD_SELECT_DISABLE:
	assert (cmd_sequence == 0);
	_TME_NCR53C9X_CS_MODE(TME_NCR53C9X_MODE_IDLE);
	_TME_NCR53C9X_CS_INT(TME_NCR53C9X_INST_SO);
	_TME_NCR53C9X_CS_DONE;
      }
    }

    /* if the command at the tail of the command FIFO has finished its
       sequence: */
    if (cmd_sequence == TME_NCR53C9X_CMD_SEQUENCE_DONE) {

      /* if the command sequence was just now finished: */
      if (ncr53c9x->tme_ncr53c9x_cmd_sequence != TME_NCR53C9X_CMD_SEQUENCE_DONE) {

	/* finish the command: */
	_tme_ncr53c9x_cmd_done(ncr53c9x);

	/* mark the command sequence as finished: */
	ncr53c9x->tme_ncr53c9x_cmd_sequence = cmd_sequence;
      }

      /* if we still have to flush the FIFO to DMA, or call out our
	 terminal DMA address, we can't start running another command,
	 because it may need to use DMA too: */
      if (ncr53c9x->tme_ncr53c9x_callout_flags
	  & (TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA
	     | TME_NCR53C9X_CALLOUT_TERMINAL_DMA)) {

	/* stop now: */
	break;
      }
	
      /* if the command FIFO is empty: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_cmd_tail;
      if (fifo_tail == ncr53c9x->tme_ncr53c9x_fifo_cmd_head) {

	/* stop now: */
	break;
      }

      /* advance the tail of the command FIFO and reset the command
	 sequence: */
      fifo_tail++;
      if (fifo_tail == TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_cmd)) {
	fifo_tail = 0;
      }
      ncr53c9x->tme_ncr53c9x_fifo_cmd_tail = fifo_tail;
      cmd_sequence = 0;
    }

    /* otherwise, the command at the tail of the command FIFO has not
       finished its sequence: */
    else {
      
      /* if the command sequence number has not changed: */
      if (cmd_sequence == ncr53c9x->tme_ncr53c9x_cmd_sequence) {

	/* stop now: */
	break;
      }
    }

    /* update the command sequence number: */
    ncr53c9x->tme_ncr53c9x_cmd_sequence = cmd_sequence;
  }
}

/* the NCR 53c9x callout function.  it must be called with the mutex locked: */
static void
_tme_ncr53c9x_callout(struct tme_ncr53c9x *ncr53c9x)
{
  struct tme_scsi_connection *conn_scsi;
  struct tme_bus_connection *conn_bus;
  struct tme_bus_tlb *tlb;
  struct tme_bus_tlb tlb_local;
  tme_scsi_control_t scsi_control;
  tme_scsi_data_t scsi_data;
  tme_uint32_t scsi_events;
  tme_uint32_t scsi_actions;
  tme_scsi_control_t last_active_scsi_control;
  tme_scsi_data_t last_active_scsi_data;
  tme_uint32_t last_active_scsi_events;
  tme_uint32_t last_active_scsi_actions;
  struct tme_scsi_dma scsi_dma_buffer;
  struct tme_scsi_dma *scsi_dma;
  unsigned int fifo_head;
  unsigned int fifo_tail;
  int callouts_blocked;
  tme_bus_addr32_t address;
  tme_uint32_t reg_ctc;
  tme_uint32_t resid;
  unsigned int cycle_type;
  int rc;
  int int_asserted;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUTS_RUNNING) {
    return;
  }

  /* callouts are now running: */
  ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUTS_RUNNING;

  /* initially, no callouts are blocked: */
  callouts_blocked = 0;

  /* loop forever: */
  for (;;) {

    /* any callout, successful or not, will clear this bit.  if we get
       to the bottom of the loop and this bit is still set, there are
       no more (unblocked) callouts to make, so we can stop: */
    callouts_blocked |= TME_NCR53C9X_CALLOUTS_RUNNING;

    /* if the data FIFO needs to be DMA flushed: */
    if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA) != 0) {

      /* we are running the data FIFO DMA flush: */
      ncr53c9x->tme_ncr53c9x_callout_flags
	&= ((~TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA)
	    | TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA));

      /* get the head and the tail of the data FIFO: */
      fifo_head = ncr53c9x->tme_ncr53c9x_fifo_data_head;
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;

      /* DMA must be running: */
      assert (ncr53c9x->tme_ncr53c9x_dma_running);

      /* get the DMA address and the CTC register: */
      address = ncr53c9x->tme_ncr53c9x_dma_address;
      reg_ctc = _tme_ncr53c9x_ctc_read(ncr53c9x);

      /* transfer out of the data FIFO starting at the tail, until the
	 head, or the right end of the data FIFO, or after CTC bytes,
	 whichever comes first: */
      resid = ((fifo_head < fifo_tail
		? TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data)
		: fifo_head)
	       - fifo_tail);
      resid = TME_MIN(resid, reg_ctc);

      /* we must have data to transfer: */
      assert (resid > 0);

      /* call out the transfer: */
      /* NB that we don't care about bus errors here - there's no way
	 to signal them to the chip.  it's possible that other DMA
	 hardware feeding the chip may note the bus error, though (and
	 may possibly intercept our callout of the terminal DMA
	 address, to substitute the actual address where the bus error
	 occurred, so software can recover it): */
      (void) tme_bus_device_dma_write_16(&ncr53c9x->tme_ncr53c9x_device,
					 address,
					 resid,
					 &ncr53c9x->tme_ncr53c9x_fifo_data[fifo_tail],
					 0);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if this callout was aborted or changed: */
      if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA)
	  != TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA)) {

	/* XXX FIXME - should we log an error here?  something
	   (probably software) aborted a DMA, but we can't abort a DMA
	   once it's been called out, so we likely transferred data
	   (changing the state of the system) after we reported the
	   abort complete to software: */
	continue;
      }

      /* the FIFO tail, DMA address, the CTC register must not have changed: */
      assert (fifo_tail == ncr53c9x->tme_ncr53c9x_fifo_data_tail);
      assert (address == ncr53c9x->tme_ncr53c9x_dma_address);
      assert (reg_ctc == _tme_ncr53c9x_ctc_read(ncr53c9x));

      /* update the FIFO tail, DMA address and CTC register: */
      ncr53c9x->tme_ncr53c9x_fifo_data_tail
	= ((ncr53c9x->tme_ncr53c9x_fifo_data_tail
	    + resid)
	   % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data));
      _tme_ncr53c9x_fifo_data_update(ncr53c9x);
      ncr53c9x->tme_ncr53c9x_dma_address += resid;
      reg_ctc -= resid;
      _tme_ncr53c9x_ctc_write(ncr53c9x, reg_ctc);

      /* if the FIFO can't be flushed any more: */
      if (ncr53c9x->tme_ncr53c9x_fifo_data_tail == ncr53c9x->tme_ncr53c9x_fifo_data_head
	  || reg_ctc == 0) {

	/* we don't need to make this callout any more: */
	ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA;
      }
    }

    /* if we need to call out the terminal DMA address, and this
       callout isn't blocked: */
    if ((ncr53c9x->tme_ncr53c9x_callout_flags
	 & TME_NCR53C9X_CALLOUT_TERMINAL_DMA
	 & ~callouts_blocked) != 0) {
      TME_NCR53C9X_DEBUG_BP(dma_terminal);

      /* if the data FIFO needs to be DMA flushed, we have to do that
	 first, so this callout is blocked: */
      if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA) != 0) {
	callouts_blocked |= TME_NCR53C9X_CALLOUT_TERMINAL_DMA;
	continue;
      }

      /* we are running the terminal DMA address callout: */
      ncr53c9x->tme_ncr53c9x_callout_flags
	&= ((~TME_NCR53C9X_CALLOUT_TERMINAL_DMA)
	    | TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_TERMINAL_DMA));

      /* DMA must be running: */
      assert (ncr53c9x->tme_ncr53c9x_dma_running);

      /* get the terminal DMA address: */
      address = ncr53c9x->tme_ncr53c9x_dma_address;

      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection,
						&ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection_rwlock);						  

      /* unlock the mutex: */
      tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
      
      /* do the callout: */
      rc = (conn_bus != NULL
	    ? ((*conn_bus->tme_bus_tlb_fill)
	       (conn_bus,
		NULL,
		address,
		TME_BUS_CYCLE_UNDEF))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if this callout was aborted or changed: */
      if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_TERMINAL_DMA)
	  != TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_TERMINAL_DMA)) {

	/* XXX FIXME - should we log an error here?  something
	   (probably software) aborted a terminal DMA callout, but we
	   can't abort one once it's been called out, so we likely
	   effected the callout (changing the state of the system)
	   after we reported the abort complete to software: */
	continue;
      }

      /* DMA must still be running, and the terminal DMA address must
         not have changed: */
      assert (ncr53c9x->tme_ncr53c9x_dma_running);
      assert (ncr53c9x->tme_ncr53c9x_dma_address == address);

      /* if this callout failed: */
      if (rc != TME_OK) {

	/* block this callout until some other callout succeeds: */
	callouts_blocked |= TME_NCR53C9X_CALLOUT_TERMINAL_DMA;
	continue;
      }

      /* DMA is no longer running: */
      ncr53c9x->tme_ncr53c9x_dma_running = FALSE;

      /* we don't need to make this callout any more: */
      ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_TERMINAL_DMA;
    }

    /* if our interrupt signal has changed, and this callout isn't
       blocked: */
    fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_status_tail;
    int_asserted = ((ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_stat
		     & TME_NCR53C9X_STAT_INT) != 0);
    if (!int_asserted != !ncr53c9x->tme_ncr53c9x_last_int_asserted
	&& (callouts_blocked & TME_NCR53C9X_CALLOUT_INT) == 0) {

      /* if we need to assert the interrupt signal, and either the
	 data FIFO needs to be data flushed, or we need to call out
	 the terminal DMA address, we need to do those first, so this
	 callout is blocked: */
      if (int_asserted
	  && (ncr53c9x->tme_ncr53c9x_callout_flags
	      & (TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA
		 | TME_NCR53C9X_CALLOUT_TERMINAL_DMA)) != 0) {
	callouts_blocked |= TME_NCR53C9X_CALLOUT_INT;
	continue;
      }

      /* get our bus connection: */
      conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection,
						&ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection_rwlock);

      /* unlock our mutex: */
      tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
	
      /* call out the bus interrupt signal edge: */
      rc = (*conn_bus->tme_bus_signal)
	(conn_bus,
	 TME_BUS_SIGNAL_INT_UNSPEC
	 | (int_asserted
	    ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	    : TME_BUS_SIGNAL_LEVEL_NEGATED));

      /* lock our mutex: */
      tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if this callout failed: */
      if (rc != TME_OK) {

	/* block this callout until some other callout succeeds: */
	callouts_blocked |= TME_NCR53C9X_CALLOUT_INT;
	continue;
      }
	  
      /* note the new state of the interrupt signal: */
      ncr53c9x->tme_ncr53c9x_last_int_asserted = int_asserted;
    }

    /* assume that we won't make a SCSI cycle callout: */
    scsi_control = 0;
    scsi_data = 0;
    scsi_events = TME_SCSI_EVENT_NONE;
    scsi_actions = TME_SCSI_ACTION_NONE;
    scsi_dma = NULL;

    /* if we need to call out a SCSI cycle, and this callout isn't
       blocked: */
    if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE
	 & TME_NCR53C9X_CALLOUT_SCSI_CYCLE
	 & ~callouts_blocked) != 0) {

      /* we are running the SCSI cycle callout: */
      ncr53c9x->tme_ncr53c9x_callout_flags
	&= ((~TME_NCR53C9X_CALLOUT_SCSI_CYCLE)
	    | TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_SCSI_CYCLE));
      
      /* get our desired output SCSI cycle: */
      scsi_control = ncr53c9x->tme_ncr53c9x_out_scsi_control;
      scsi_data = ncr53c9x->tme_ncr53c9x_out_scsi_data;
      scsi_events = ncr53c9x->tme_ncr53c9x_out_scsi_events;
      scsi_actions = ncr53c9x->tme_ncr53c9x_out_scsi_actions;

      /* the desired output SCSI cycle must have some events or
         actions: */
      assert (scsi_events != TME_SCSI_EVENT_NONE
	      || scsi_actions != TME_SCSI_ACTION_NONE);

      /* if this is a SCSI DMA callout: */
      if (scsi_actions
	  & (TME_SCSI_ACTION_DMA_TARGET
	     | TME_SCSI_ACTION_DMA_INITIATOR
	     | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)) {
	TME_NCR53C9X_DEBUG_BP(dma_scsi);

	/* we must have no SCSI events: */
	assert (scsi_events == TME_SCSI_EVENT_NONE);

	/* the command sequence SCSI bus transfer residual must be
	   nonzero: */
	assert (ncr53c9x->tme_ncr53c9x_transfer_resid > 0);

	/* set the bus cycle type to TME_BUS_CYCLE_WRITE if the SCSI
	   DMA cycle will transfer in from the SCSI bus, else set it
	   to TME_BUS_CYCLE_READ: */
	cycle_type = (_tme_ncr53c9x_transfer_input(ncr53c9x)
		      ? TME_BUS_CYCLE_WRITE
		      : TME_BUS_CYCLE_READ);

	/* assume that DMA is not running: */
	tlb = NULL;

	/* if DMA is running: */
	if (ncr53c9x->tme_ncr53c9x_dma_running) {

	  /* get the DMA address: */
	  address = ncr53c9x->tme_ncr53c9x_dma_address;

	  /* the CTC register must be nonzero: */
	  assert (_tme_ncr53c9x_ctc_read(ncr53c9x) > 0);

	  /* get our TLB entry: */
	  tlb = &ncr53c9x->tme_ncr53c9x_dma_tlb;

	  /* busy this TLB entry: */
	  tme_bus_tlb_busy(tlb);

	  /* if the TLB entry is valid, covers this address and allows
	     the needed access: */
	  if (tme_bus_tlb_is_valid(tlb)
	      && address >= (tme_bus_addr32_t) tlb->tme_bus_tlb_addr_first
	      && address <= (tme_bus_addr32_t) tlb->tme_bus_tlb_addr_last
	      && (((cycle_type == TME_BUS_CYCLE_READ
		    ? tlb->tme_bus_tlb_emulator_off_read
		    : tlb->tme_bus_tlb_emulator_off_write) != TME_EMULATOR_OFF_UNDEF)
		  || (tlb->tme_bus_tlb_cycles_ok & cycle_type))) {

	    /* unbusy this TLB entry: */
	    tme_bus_tlb_unbusy(tlb);
	  }

	  /* otherwise, this TLB entry is invalid, or doesn't cover this
	     address, or doesn't allow the needed access: */
	  else {

	    /* unbusy this TLB entry for filling: */
	    tme_bus_tlb_unbusy_fill(tlb);

	    /* pass this TLB's token: */
	    tlb_local.tme_bus_tlb_token = tlb->tme_bus_tlb_token;

	    /* get our bus connection: */
	    conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						      ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection,
						      &ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection_rwlock);

	    /* unlock the mutex: */
	    tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
      
	    /* do the callout: */
	    rc = (conn_bus != NULL
		  ? ((*conn_bus->tme_bus_tlb_fill)
		     (conn_bus,
		      &tlb_local,
		      address,
		      cycle_type))
		  : EAGAIN);
	
	    /* lock the mutex: */
	    tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

	    /* unblock all callouts: */
	    callouts_blocked = 0;

	    /* if the SCSI cycle callout was aborted or changed, restart: */
	    if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE)
		!= TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_SCSI_CYCLE)) {
	      continue;
	    }

	    /* DMA must still be running, and the DMA address must
	       not have changed: */
	    assert (ncr53c9x->tme_ncr53c9x_dma_running);
	    assert (ncr53c9x->tme_ncr53c9x_dma_address == address);

	    /* if this callout failed: */
	    if (rc != TME_OK) {

	      /* block this callout until some other callout succeeds.
		 if there isn't an active SCSI cycle called out, we
		 may actually call out this SCSI cycle's control and
		 data lines, but instead of running DMA we will just
		 wait for a bus change: */
	      callouts_blocked |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
	      continue;
	    }
	  
	    /* copy the local TLB entry into the global TLB entry: */
	    *tlb = tlb_local;
	  }
	}

	/* start the SCSI DMA structure: */
	/* XXX parity? */
	scsi_dma = &scsi_dma_buffer;
	scsi_dma_buffer.tme_scsi_dma_flags = TME_SCSI_DMA_8BIT;
	scsi_dma_buffer.tme_scsi_dma_sync_offset = 0;
	scsi_dma_buffer.tme_scsi_dma_sync_period = 0;

	/* get the head and the tail of the data FIFO: */
	fifo_head = ncr53c9x->tme_ncr53c9x_fifo_data_head;
	fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;

	/* assume that new data will be transferred into the data
	   FIFO, either from the SCSI bus or from the data bus.  we
	   will transfer data into the data FIFO starting at the head,
	   until the tail, or until the right end of the data FIFO,
	   whichever comes first: */
	scsi_dma_buffer.tme_scsi_dma_resid
	  = (((fifo_head < fifo_tail)
	      ? fifo_tail
	      : TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data))
	     - fifo_head);

	/* note that we can't fill all positions of the data FIFO,
	   because we identify an empty data FIFO when the head and
	   the tail are equal: */
	if (((fifo_head + scsi_dma_buffer.tme_scsi_dma_resid)
	     % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data))
	    == fifo_tail) {
	  scsi_dma_buffer.tme_scsi_dma_resid--;
	}

	/* if the SCSI DMA cycle will transfer in from the SCSI bus: */
	if (cycle_type == TME_BUS_CYCLE_WRITE) {

	  /* assume that we're transferring into the data FIFO: */
	  scsi_dma_buffer.tme_scsi_dma_in = &ncr53c9x->tme_ncr53c9x_fifo_data[fifo_head];
#ifndef NDEBUG
	  scsi_dma_buffer.tme_scsi_dma_out = NULL;
#endif /* NDEBUG */

	  /* if DMA is running: */
	  if (ncr53c9x->tme_ncr53c9x_dma_running) {

	    /* if the data FIFO isn't empty: */
	    if (fifo_head != fifo_tail) {

	      /* don't transfer any data from the SCSI bus now, and
		 request that the data FIFO be flushed: */
	      /* NB: this isn't really necessary - we could just go
		 ahead and transfer more data from the SCSI bus into
		 the data FIFO, and flush it all out later.  but
		 instead we assume that if we let the data in the FIFO
		 flush to DMA now, after we advance the DMA address we
		 may be able to start doing truly direct DMAs: */
	      scsi_dma_buffer.tme_scsi_dma_resid = 0;
	      ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA;
	    }

	    /* otherwise, the data FIFO is empty: */
	    else {

	      /* if this TLB entry supports fast writing: */
	      if (tlb->tme_bus_tlb_emulator_off_write != TME_EMULATOR_OFF_UNDEF) {

		/* get the DMA address: */
		address = ncr53c9x->tme_ncr53c9x_dma_address;

		/* transfer starting from the fast writing address,
		   stopping at least at the end of this TLB entry: */
		/* XXX FIXME - this breaks volatile: */
		scsi_dma_buffer.tme_scsi_dma_in = (tme_uint8_t *) (tlb->tme_bus_tlb_emulator_off_write + address);
		scsi_dma_buffer.tme_scsi_dma_resid = (((tme_bus_addr32_t) tlb->tme_bus_tlb_addr_last) - address) + 1;
	      }

	      /* otherwise, this TLB entry doesn't support fast writing: */
	      else {

		/* we're already set up to transfer in to the data FIFO: */
	      }

	      /* don't transfer more than CTC bytes: */
	      reg_ctc = _tme_ncr53c9x_ctc_read(ncr53c9x);
	      if (reg_ctc < scsi_dma_buffer.tme_scsi_dma_resid) {
		scsi_dma_buffer.tme_scsi_dma_resid = reg_ctc;
	      }
	    }
	  }

	  /* otherwise, DMA is not running: */
	  else {

	    /* if we're not transferring any data now, the data FIFO
               is full: */
	    if (scsi_dma_buffer.tme_scsi_dma_resid == 0) {

	      /* XXX FIXME - what happens here?  for now, we assume
		 that we do nothing, and just wait for software to
		 read data out of the FIFO, to make room for more
		 data: */
	    }
	  }
	}

	/* otherwise, the SCSI DMA cycle will transfer out to the SCSI bus: */
	else {

#ifndef NDEBUG
	  /* we're not transferring in from the SCSI bus: */
	  scsi_dma_buffer.tme_scsi_dma_in = NULL;
#endif /* NDEBUG */

	  /* if DMA is not running: */
	  if (!ncr53c9x->tme_ncr53c9x_dma_running) {

	    /* unless the FIFO is not empty, which we check below, we
	       don't have any data to transfer now: */
	    scsi_dma_buffer.tme_scsi_dma_out = NULL;
	    scsi_dma_buffer.tme_scsi_dma_resid = 0;
	  }

	  /* otherwise, DMA is running.  if there isn't any data in
	     the FIFO that needs to be transferred first: */
	  else if (fifo_tail == fifo_head) {

	    /* get the DMA address and the CTC register: */
	    address = ncr53c9x->tme_ncr53c9x_dma_address;
	    reg_ctc = _tme_ncr53c9x_ctc_read(ncr53c9x);

	    /* if this TLB entry supports fast reading: */
	    if (tlb->tme_bus_tlb_emulator_off_read != TME_EMULATOR_OFF_UNDEF) {

	      /* transfer starting from the fast reading address, stopping
		 at least at the end of this TLB entry: */
	      /* XXX FIXME - this breaks volatile: */
	      scsi_dma_buffer.tme_scsi_dma_out = (const tme_uint8_t *) (tlb->tme_bus_tlb_emulator_off_read + address);
	      scsi_dma_buffer.tme_scsi_dma_resid = (((tme_bus_addr32_t) tlb->tme_bus_tlb_addr_last) - address) + 1;

	      /* don't transfer more than CTC bytes: */
	      if (reg_ctc < scsi_dma_buffer.tme_scsi_dma_resid) {
		scsi_dma_buffer.tme_scsi_dma_resid = reg_ctc;
	      }
	    }

	    /* otherwise, this TLB entry doesn't support fast reading: */
	    else {

	      /* transfer into the data FIFO starting at the head,
		 until the tail, or the right end of the data FIFO, or
		 after CTC bytes, or after the SCSI bus transfer
		 residual, whichever comes first: */
	      resid = TME_MIN(scsi_dma_buffer.tme_scsi_dma_resid, reg_ctc);
	      resid = TME_MIN(resid, ncr53c9x->tme_ncr53c9x_transfer_resid);

	      /* if we need to detect the actual SCSI bus transfer
		 residual based on the data transferred: */
	      if (ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state != 0) {

		/* we can't DMA more than one byte at a time before
		   we've detected the actual SCSI bus residual: */
		assert (resid > 0);
		resid = 1;
	      }

	      /* call out the transfer: */
	      /* NB that we don't care about bus errors here - there's
		 no way to signal them to the chip.  it's possible
		 that other DMA hardware feeding the chip may note the
		 bus error, though (and may possibly intercept our
		 callout of the terminal DMA address, to substitute
		 the actual address where the bus error occurred, so
		 software can recover it): */
	      (void) tme_bus_device_dma_read_16(&ncr53c9x->tme_ncr53c9x_device,
						address,
						resid,
						&ncr53c9x->tme_ncr53c9x_fifo_data[fifo_head],
						0);

	      /* unblock all callouts: */
	      callouts_blocked = 0;

	      /* if this callout was aborted or changed: */
	      if ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE)
		  != TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_SCSI_CYCLE)) {

		/* XXX FIXME - should we log an error here?  something
		   (probably software) aborted a DMA, but we can't
		   abort a DMA once it's been called out, so we likely
		   transferred data (changing the state of the system)
		   after we reported the abort complete to software: */
		continue;
	      }

	      /* the FIFO head, DMA address, the CTC register must not have changed: */
	      assert (fifo_head == ncr53c9x->tme_ncr53c9x_fifo_data_head);
	      assert (address == ncr53c9x->tme_ncr53c9x_dma_address);
	      assert (reg_ctc == _tme_ncr53c9x_ctc_read(ncr53c9x));

	      /* update the FIFO head, DMA address and CTC register: */
	      fifo_head = (fifo_head + resid) % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data);
	      ncr53c9x->tme_ncr53c9x_fifo_data_head = fifo_head;
	      _tme_ncr53c9x_fifo_data_update(ncr53c9x);
	      ncr53c9x->tme_ncr53c9x_dma_address += resid;
	      reg_ctc -= resid;
	      _tme_ncr53c9x_ctc_write(ncr53c9x, reg_ctc);

	      /* reload the data FIFO tail: */
	      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;

	      /* the data FIFO must not be empty: */
	      assert (fifo_head != fifo_tail);
	    }
	  }

	  /* if the data FIFO isn't empty: */
	  if (fifo_head != fifo_tail) {

	    /* transfer out of the data FIFO starting at the tail,
	       until the head, or the right end of the data FIFO,
	       whichever comes first: */
	    scsi_dma_buffer.tme_scsi_dma_out = &ncr53c9x->tme_ncr53c9x_fifo_data[fifo_tail];
	    scsi_dma_buffer.tme_scsi_dma_resid = ((fifo_head < fifo_tail
						   ? TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data)
						   : fifo_head)
						  - fifo_tail);
	  }

	  /* if we need to detect the SCSI bus transfer residual based
	     on the data being transferred: */
	  if (ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state != 0) {

	    /* try to detect the SCSI bus transfer residual: */
	    resid
	      = tme_scsi_phase_resid(((ncr53c9x->tme_ncr53c9x_mode
				       == TME_NCR53C9X_MODE_INITIATOR)
				      ? ncr53c9x->tme_ncr53c9x_latched_phase
				      : ncr53c9x->tme_ncr53c9x_out_scsi_control),
				     &ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state,
				     scsi_dma_buffer.tme_scsi_dma_out,
				     scsi_dma_buffer.tme_scsi_dma_resid);
	    if (resid != 0) {
	      ncr53c9x->tme_ncr53c9x_transfer_resid
		= TME_MIN(ncr53c9x->tme_ncr53c9x_transfer_resid, resid);
	    }
	  }
	}

	/* don't transfer more bytes over the SCSI bus than requested
           by the command sequence: */
	scsi_dma_buffer.tme_scsi_dma_resid
	  = TME_MIN(scsi_dma_buffer.tme_scsi_dma_resid,
		    ncr53c9x->tme_ncr53c9x_transfer_resid);

	/* if we're the initiator and we want to hold ACK on the last
	   byte in the transfer, but either we haven't detected which
	   is the last byte yet or it's not in this DMA, we don't want
	   to hold ACK on the last byte in this DMA: */
	if (((scsi_actions & TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)
	     == TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)
	    && (ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state != 0
		|| scsi_dma_buffer.tme_scsi_dma_resid < ncr53c9x->tme_ncr53c9x_transfer_resid)) {
	  scsi_actions
	    = ((scsi_actions & ~TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)
	       | TME_SCSI_ACTION_DMA_INITIATOR);
	}

	/* XXX FIXME - in initiator mode, ACK may have been left set
	   by a previous command.  clear it before running this DMA
	   cycle: */

	/* if we don't have a SCSI DMA buffer: */
	if (scsi_dma_buffer.tme_scsi_dma_resid == 0) {

	  /* call out a SCSI cycle that just waits for a bus change: */
	  scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
	  scsi_actions = TME_SCSI_ACTION_NONE;
	  scsi_data = 0;
	  scsi_dma = NULL;
	}
      }
    }

    /* otherwise, we either don't need to call out a SCSI cycle, or
       this callout is blocked.  if the callout isn't blocked, but no
       SCSI cycle is active: */
    else if ((callouts_blocked & TME_NCR53C9X_CALLOUT_SCSI_CYCLE) == 0
	     && ncr53c9x->tme_ncr53c9x_active_scsi_events == TME_SCSI_EVENT_NONE
	     && ncr53c9x->tme_ncr53c9x_active_scsi_actions == TME_SCSI_ACTION_NONE) {

      /* call out the control and data signals from the last active
	 SCSI cycle, and just wait for a SCSI bus change: */
      scsi_control = ncr53c9x->tme_ncr53c9x_active_scsi_control;
      scsi_data = ncr53c9x->tme_ncr53c9x_active_scsi_data;
      scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
      scsi_actions = TME_SCSI_ACTION_NONE;
    }

    /* if we need to call out a SCSI cycle: */
    if (scsi_events != TME_SCSI_EVENT_NONE
	|| scsi_actions != TME_SCSI_ACTION_NONE) {

      /* get this card's SCSI connection: */
      conn_scsi = ncr53c9x->tme_ncr53c9x_scsi_connection;

      /* remember the SCSI cycle that is active before the callout: */
      last_active_scsi_control = ncr53c9x->tme_ncr53c9x_active_scsi_control;
      last_active_scsi_data = ncr53c9x->tme_ncr53c9x_active_scsi_data;
      last_active_scsi_events = ncr53c9x->tme_ncr53c9x_active_scsi_events;
      last_active_scsi_actions = ncr53c9x->tme_ncr53c9x_active_scsi_actions;

      /* assume that the callout will succeed, and set this new SCSI
	 cycle as active: */
      /* NB: we *cannot* do this after the callout returns, because
	 while we are calling this cycle out, the finished cycle may
	 get called out back to us: */
      ncr53c9x->tme_ncr53c9x_active_scsi_control = scsi_control;
      ncr53c9x->tme_ncr53c9x_active_scsi_data = scsi_data;
      ncr53c9x->tme_ncr53c9x_active_scsi_events = scsi_events;
      ncr53c9x->tme_ncr53c9x_active_scsi_actions = scsi_actions;
      ncr53c9x->tme_ncr53c9x_active_scsi_dma_resid
	= (scsi_dma != NULL
	   ? scsi_dma->tme_scsi_dma_resid
	   : 0);

      /* flip the SCSI cycle marker, and add it to the actions: */
      ncr53c9x->tme_ncr53c9x_active_scsi_cycle_marker
	^= TME_SCSI_ACTION_CYCLE_MARKER;
      scsi_actions |= ncr53c9x->tme_ncr53c9x_active_scsi_cycle_marker;

      /* unlock the mutex: */
      tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
      
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
      tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

      /* unblock all callouts: */
      callouts_blocked = 0;

      /* if the SCSI cycle callout failed: */
      if (rc != TME_OK) {

	/* assume that the SCSI cycle that was active before we tried
	   to call out, is still active after the callout failure: */	   
	ncr53c9x->tme_ncr53c9x_active_scsi_control = last_active_scsi_control;
	ncr53c9x->tme_ncr53c9x_active_scsi_data = last_active_scsi_data;
	ncr53c9x->tme_ncr53c9x_active_scsi_events = last_active_scsi_events;
	ncr53c9x->tme_ncr53c9x_active_scsi_actions = last_active_scsi_actions;
	ncr53c9x->tme_ncr53c9x_active_scsi_cycle_marker
	  ^= TME_SCSI_ACTION_CYCLE_MARKER;

	/* block this callout until some other callout succeeds: */
	callouts_blocked |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
	continue;
      }

      /* if we called out the desired output SCSI cycle, and it isn't
	 a DMA cycle: */
      if (scsi_control == ncr53c9x->tme_ncr53c9x_out_scsi_control
	  && scsi_data == ncr53c9x->tme_ncr53c9x_out_scsi_data
	  && scsi_events == ncr53c9x->tme_ncr53c9x_out_scsi_events
	  && ((scsi_actions & ~TME_SCSI_ACTION_CYCLE_MARKER)
	      == ncr53c9x->tme_ncr53c9x_out_scsi_actions)
	  && scsi_dma == NULL) {

	/* we don't need to make this callout any more: */
	ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
      }
    }

    /* if no more (unblocked) callouts can run, we can stop: */
    if (callouts_blocked & TME_NCR53C9X_CALLOUTS_RUNNING) {
      break;
    }
  }
  
  /* clear that callouts are running: */
  ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUTS_RUNNING;
}

/* this is called for an event on the SCSI bus: */
static int
_tme_ncr53c9x_scsi_cycle(struct tme_scsi_connection *conn_scsi,
			 tme_scsi_control_t scsi_control,
			 tme_scsi_data_t scsi_data,
			 tme_uint32_t scsi_events_triggered,
			 tme_uint32_t scsi_actions_taken,
			 const struct tme_scsi_dma *scsi_dma)
{
  struct tme_ncr53c9x *ncr53c9x;
  unsigned long transfer_count;
  unsigned long resid;
  const tme_uint8_t *transfer_buffer;
  unsigned int fifo_head;
  unsigned int fifo_tail;
  tme_uint32_t reg_ctc;
  unsigned int cycle_type;
  int callout_clear;
  
  /* recover our data structure: */
  ncr53c9x = conn_scsi->tme_scsi_connection.tme_connection_element->tme_element_private;

  /* assume that we won't clear the SCSI cycle callout flags: */
  callout_clear = FALSE;

  /* lock the mutex: */
  tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* if this input SCSI cycle follows our active output SCSI cycle: */
  if (((scsi_actions_taken 
	^ ncr53c9x->tme_ncr53c9x_active_scsi_cycle_marker)
       & TME_SCSI_ACTION_CYCLE_MARKER) == 0) {

    /* the active output SCSI cycle now has the same control and data
       lines, but is waiting on no events and will take no actions: */
    ncr53c9x->tme_ncr53c9x_active_scsi_events = TME_SCSI_EVENT_NONE;
    ncr53c9x->tme_ncr53c9x_active_scsi_actions = TME_SCSI_ACTION_NONE;

    /* assume that, if the SCSI cycle callout is still marked running,
       we can clear the callout flags: */
    callout_clear = TRUE;
  }
  scsi_actions_taken &= ~TME_SCSI_ACTION_CYCLE_MARKER;

  /* set the last input SCSI cycle.  we accumulate the events
     triggered and actions taken, filtering out bus change and bus
     free events: */
  ncr53c9x->tme_ncr53c9x_in_scsi_control = scsi_control;
  ncr53c9x->tme_ncr53c9x_in_scsi_data = scsi_data;
  ncr53c9x->tme_ncr53c9x_in_scsi_events
    |= (scsi_events_triggered
	& ~(TME_SCSI_EVENT_BUS_CHANGE
	    | TME_SCSI_EVENT_BUS_FREE));
  ncr53c9x->tme_ncr53c9x_in_scsi_actions |= scsi_actions_taken;

  /* if a SCSI selection with ATN cycle just finished: */
  if ((scsi_actions_taken
       & TME_SCSI_ACTION_SELECT_WITH_ATN)
      == TME_SCSI_ACTION_SELECT_WITH_ATN) {

    /* ATN has already been asserted on our behalf, and we have to
       make sure that we continue to assert it: */
    ncr53c9x->tme_ncr53c9x_out_scsi_control |= TME_SCSI_SIGNAL_ATN;
    ncr53c9x->tme_ncr53c9x_active_scsi_control |= TME_SCSI_SIGNAL_ATN;
  }

  /* if a SCSI DMA cycle just finished: */
  if (scsi_actions_taken
      & (TME_SCSI_ACTION_DMA_TARGET
	 | TME_SCSI_ACTION_DMA_INITIATOR
	 | TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)) {

    /* this input cycle must follow our active output SCSI cycle: */
    assert (ncr53c9x->tme_ncr53c9x_active_scsi_events == TME_SCSI_EVENT_NONE
	    && ncr53c9x->tme_ncr53c9x_active_scsi_actions == TME_SCSI_ACTION_NONE);

    /* get the count of bytes transferred: */
    transfer_count = ncr53c9x->tme_ncr53c9x_active_scsi_dma_resid - scsi_dma->tme_scsi_dma_resid;

    /* update the number of expected bytes in this transfer: */
    resid = ncr53c9x->tme_ncr53c9x_transfer_resid;
    assert (transfer_count <= resid);
    resid -= transfer_count;
    ncr53c9x->tme_ncr53c9x_transfer_resid = resid;

    /* if this SCSI DMA cycle has no more bytes to transfer: */
    if (resid == 0) {

      /* if a SCSI initiator DMA with hold ACK cycle just finished: */
      if ((scsi_actions_taken
	   & TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK)
	  == TME_SCSI_ACTION_DMA_INITIATOR_HOLD_ACK) {

	/* ACK has already been asserted on our behalf, and we have to
	   make sure that we continue to assert it: */
	ncr53c9x->tme_ncr53c9x_out_scsi_control |= TME_SCSI_SIGNAL_ACK;
	ncr53c9x->tme_ncr53c9x_active_scsi_control |= TME_SCSI_SIGNAL_ACK;
      }
    }

    /* otherwise, this SCSI DMA cycle has more bytes to transfer: */
    else {

      /* we can't clear the callout flags for this DMA SCSI cycle yet.
	 we probably just reached the end of our TLB entry, and we
	 need to advance in DMA and call out the DMA SCSI cycle again.
	 if something else has happened (like we're the initiator, and
	 the target has suddenly changed phases), we'll pick that up
	 in _tme_ncr53c9x_update(): */
      callout_clear = FALSE;
    }

    /* if the SCSI DMA cycle transferred in from the SCSI bus: */
    if (_tme_ncr53c9x_transfer_input(ncr53c9x)) {

      /* set the bus cycle type to TME_BUS_CYCLE_WRITE and get
	 a pointer to the bytes transferred: */
      cycle_type = TME_BUS_CYCLE_WRITE;
      transfer_buffer = scsi_dma->tme_scsi_dma_in - transfer_count;

      /* if we need to detect the SCSI bus transfer residual based
	 on the data being transferred: */
      if (ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state != 0) {

	/* try to detect the SCSI bus transfer residual: */
	resid
	  = tme_scsi_phase_resid(((ncr53c9x->tme_ncr53c9x_mode
				   == TME_NCR53C9X_MODE_INITIATOR)
				  ? ncr53c9x->tme_ncr53c9x_latched_phase
				  : ncr53c9x->tme_ncr53c9x_out_scsi_control),
				 &ncr53c9x->tme_ncr53c9x_transfer_resid_detect_state,
				 transfer_buffer,
				 transfer_count);
	if (resid != 0) {
	  ncr53c9x->tme_ncr53c9x_transfer_resid
	    = TME_MIN(ncr53c9x->tme_ncr53c9x_transfer_resid, resid);
	}
      }
    }

    /* otherwise, the SCSI DMA cycle transferred out to the SCSI bus: */
    else {

      /* set the bus cycle type to TME_BUS_CYCLE_READ and get
	 a pointer to the bytes transferred: */
      cycle_type = TME_BUS_CYCLE_READ;
      transfer_buffer = scsi_dma->tme_scsi_dma_out - transfer_count;
    }

    /* if this SCSI DMA cycle didn't transfer to or from the data
       FIFO: */
    if ((transfer_buffer
	 < &ncr53c9x->tme_ncr53c9x_fifo_data[0])
	|| (transfer_buffer
	    > &ncr53c9x->tme_ncr53c9x_fifo_data[TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data) - 1])) {

      /* the data FIFO must be empty: */
      assert (ncr53c9x->tme_ncr53c9x_fifo_data_head == ncr53c9x->tme_ncr53c9x_fifo_data_tail);

      /* DMA must be running: */
      assert (ncr53c9x->tme_ncr53c9x_dma_running);

      /* the transfer count can't be greater than the CTC register: */
      reg_ctc = _tme_ncr53c9x_ctc_read(ncr53c9x);
      assert (transfer_count <= reg_ctc);
      
      /* update the DMA address and CTC register: */
      ncr53c9x->tme_ncr53c9x_dma_address += transfer_count;
      reg_ctc -= transfer_count;
      _tme_ncr53c9x_ctc_write(ncr53c9x, reg_ctc);
    }

    /* otherwise, this SCSI DMA cycle transferred to or from the data
       FIFO: */
    else {

      /* if the SCSI DMA cycle transferred in from the SCSI bus: */
      if (cycle_type == TME_BUS_CYCLE_WRITE) {

	/* if the SCSI DMA cycle transferred into the head of the data
           FIFO: */
	fifo_head = ncr53c9x->tme_ncr53c9x_fifo_data_head;
	if (transfer_buffer == &ncr53c9x->tme_ncr53c9x_fifo_data[fifo_head]) {

	  /* update the data FIFO head: */
	  ncr53c9x->tme_ncr53c9x_fifo_data_head
	    = ((fifo_head + transfer_count)
	       % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data));
	  _tme_ncr53c9x_fifo_data_update(ncr53c9x);

	  /* if DMA is running: */
	  if (ncr53c9x->tme_ncr53c9x_dma_running) {

	    /* we need to flush the data FIFO to DMA: */
	    ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_FLUSH_FIFO_DMA;
	  }
	}
      }

      /* otherwise, the SCSI DMA cycle transferred out to the SCSI bus: */
      else {

	/* if the SCSI DMA cycle transferred out of the tail of the data FIFO: */
	fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;
	if (transfer_buffer == &ncr53c9x->tme_ncr53c9x_fifo_data[fifo_tail]) {

	  /* update the data FIFO tail: */
	  ncr53c9x->tme_ncr53c9x_fifo_data_tail
	    = ((fifo_tail + transfer_count)
	       % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data));
	  _tme_ncr53c9x_fifo_data_update(ncr53c9x);
	}
      }
    }
  }

  /* if we can clear a SCSI cycle callout marked as running, do it: */
  if (callout_clear
      && ((ncr53c9x->tme_ncr53c9x_callout_flags & TME_NCR53C9X_CALLOUT_SCSI_CYCLE)
	  == TME_NCR53C9X_CALLOUT_RUNNING(TME_NCR53C9X_CALLOUT_SCSI_CYCLE))) {
    ncr53c9x->tme_ncr53c9x_callout_flags &= ~TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
  }

  /* run the update function: */
  _tme_ncr53c9x_update(ncr53c9x);

  /* make any callouts: */
  _tme_ncr53c9x_callout(ncr53c9x);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);

  return (TME_OK);
}

/* the ncr53c9x timeout thread: */
static void
_tme_ncr53c9x_timeout_th(struct tme_ncr53c9x *ncr53c9x)
{

  /* lock our mutex: */
  tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* loop forever: */
  for (;;) {

    /* update: */
    _tme_ncr53c9x_update(ncr53c9x);

    /* make any callouts: */
    _tme_ncr53c9x_callout(ncr53c9x);

    /* if there is a timeout pending: */
    if (ncr53c9x->tme_ncr53c9x_cmd_sequence != TME_NCR53C9X_CMD_SEQUENCE_DONE
	&& ncr53c9x->tme_ncr53c9x_cmd_sequence_timeout != TME_NCR53C9X_CMD_SEQUENCE_UNDEF) {

      /* sleep, but wake up if we get another timeout: */
      tme_cond_sleep_yield(&ncr53c9x->tme_ncr53c9x_timeout_cond,
			   &ncr53c9x->tme_ncr53c9x_mutex,
			   &ncr53c9x->tme_ncr53c9x_timeout_length);
    }
    
    /* otherwise, there is no timeout pending: */
    else {

      /* wait on the condition: */
      tme_cond_wait_yield(&ncr53c9x->tme_ncr53c9x_timeout_cond,
			  &ncr53c9x->tme_ncr53c9x_mutex);
    }
  }
  /* NOTREACHED */
}

/* the ncr53c9x bus signal handler: */
static int
_tme_ncr53c9x_signal(void *_ncr53c9x, 
		     unsigned int signal)
{
  struct tme_ncr53c9x *ncr53c9x;
  unsigned int level;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* take out the signal level: */
  level = signal & TME_BUS_SIGNAL_LEVEL_MASK;
  signal = TME_BUS_SIGNAL_WHICH(signal);

  /* lock the mutex: */
  tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* dispatch on the generic bus signals: */
  switch (signal) {

  case TME_BUS_SIGNAL_RESET:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      _tme_ncr53c9x_reset(ncr53c9x, TME_NCR53C9X_RESET_DEVICE);
    }
    break;

  case TME_BUS_SIGNAL_DACK:
    if (level == TME_BUS_SIGNAL_LEVEL_ASSERTED) {
      ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
    }
    break;

  default:
    signal = TME_BUS_SIGNAL_IGNORE;
    break;
  }

  /* if we didn't ignore this bus signal: */
  if (signal != TME_BUS_SIGNAL_IGNORE) {

    /* run the update function: */
    _tme_ncr53c9x_update(ncr53c9x);

    /* make any callouts: */
    _tme_ncr53c9x_callout(ncr53c9x);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the NCR 53c9x bus cycle handler: */
static int
_tme_ncr53c9x_bus_cycle(void *_ncr53c9x,
			struct tme_bus_cycle *cycle_init)
{
  struct tme_ncr53c9x *ncr53c9x;
  unsigned int address;
  tme_uint8_t value;
  unsigned int fifo_head;
  unsigned int fifo_tail;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* get the address: */
  address = cycle_init->tme_bus_cycle_address;

  /* lock the mutex: */
  tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* if this is a read: */
  if ((cycle_init->tme_bus_cycle_type & TME_BUS_CYCLE_READ) != 0) {

    /* dispatch on the register: */
    switch (address) {
      
      /* reads of these registers are simple: */
    case TME_NCR53C9X_REG_CTC_LSB:
    case TME_NCR53C9X_REG_CTC_MSB:
    case TME_NCR53C9X_REG_CFIS:
    case TME_NCR53C9X_REG_CONTROL1:
    case TME_NCR53C9X_REG_CONTROL3:
      value = ncr53c9x->tme_ncr53c9x_regs[address];
      break;

      /* "The data is latched until the Interrupt Status Register is
         read.  The phase bits will be latched only if latching is
         enabled in the Control Register 2, otherwise, it will
         indicate the current SCSI phase.  If command stacking is used,
         two interrupts might occur.  Reading this register will clear
         the status information for the first interrupt and update the
         Status Register for the second interrupt." */
    case TME_NCR53C9X_REG_STAT:

      /* get the STAT register value at the tail of the status FIFO: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_status_tail;
      value = ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_stat;

      /* if the SCSI bus phase bits are not latched: */
      if (!TME_NCR53C9X_HAS_CONTROL2_LSP(ncr53c9x)
	  || !(ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_CONTROL2] & TME_NCR53C9X_CONTROL2_LSP)) {

	/* put in the current SCSI bus phase: */
	value = _tme_ncr53c9x_scsi_phase_stat(ncr53c9x, value);
      }
      
      /* put in the CTZ bit: */
      value
	= ((value & ~TME_NCR53C9X_STAT_CTZ)
	   | (ncr53c9x->tme_ncr53c9x_regs[TME_NCR53C9X_REG_STAT]
	      & TME_NCR53C9X_STAT_CTZ));
      break;

      /* "The Internal State Register (ISREG) tracks the progress of a
         sequence-type command.  It is updated after each successful
         completion of an intermediate operation.  If an error occurs,
         the host can read this register to determine at where the
         command failed and take the necessary procedure for
         recovery.  Reading the Interrupt Status Register will clear
         this register.  ISREG Bit 3 SOF Synchronous Offset Flag The
         SOF is reset when the Synchronous Offset Register (SOFREG)
         has reached its maximum value.  Note: The SOF bit is active
         Low." */
    case TME_NCR53C9X_REG_IS:

      /* get the IS register value at the tail of the status FIFO: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_status_tail;
      value = ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_is;
      break;

      /* "The Interrupt Status Register (INSTREG) will indicate the
         reason for the interrupt.  This register is used with the
         Status Register (STATREG) and Internal Status Register
         (ISREG) to determine the reason for the interrupt.  Reading
         the INSTREG will clear all three registers." */
    case TME_NCR53C9X_REG_INST:
      TME_NCR53C9X_DEBUG_BP(read_inst);

      /* get the INST register value at the tail of the status FIFO: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_status_tail;
      value = ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_inst;

      /* if the status FIFO has another element in it: */
      if (fifo_tail != ncr53c9x->tme_ncr53c9x_fifo_status_head) {

	/* advance the tail of the status FIFO: */
	fifo_tail++;
	if (fifo_tail == TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_status)) {
	  fifo_tail = 0;
	}
	ncr53c9x->tme_ncr53c9x_fifo_status_tail = fifo_tail;
      }

      /* otherwise, the status FIFO is empty: */
      else {

	/* clear STAT, IS, and INST: */
	ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_stat = 0;
	ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_is = 0;
	ncr53c9x->tme_ncr53c9x_fifo_status[fifo_tail].tme_ncr53c9x_status_inst = 0;
      }

      /* call out an interrupt signal change: */
      ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_INT;
      break;
      
      /* a data FIFO read: */
    case TME_NCR53C9X_REG_FIFO:
      TME_NCR53C9X_DEBUG_BP(read_fifo);

      /* get the FIFO register value at the tail of the data FIFO: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_data_tail;
      value = ncr53c9x->tme_ncr53c9x_fifo_data[fifo_tail];

      /* if the data FIFO has another element in it: */
      if (fifo_tail != ncr53c9x->tme_ncr53c9x_fifo_data_head) {

	/* advance the tail of the data FIFO: */
	ncr53c9x->tme_ncr53c9x_fifo_data_tail = (fifo_tail + 1) % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data);
	_tme_ncr53c9x_fifo_data_update(ncr53c9x);
      }
      break;

      /* "Reading this register will return the command currently
         being executed (or the last command executed if there are no
         pending commands)." */
    case TME_NCR53C9X_REG_CMD:

      /* get the CMD register value at the tail of the command FIFO: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_cmd_tail;
      value = ncr53c9x->tme_ncr53c9x_fifo_cmd[fifo_tail];
      break;

      /* a read of Control register two depends on the variant: */
    case TME_NCR53C9X_REG_CONTROL2:
      switch (ncr53c9x->tme_ncr53c9x_variant) {
      default: assert(FALSE);

	/* the ESP100 doesn't have Control register two: */
      case TME_NCR53C9X_VARIANT_ESP100: value = 0xff; break;
      }
      break;

      /* an unknown register: */
    default: abort();
    }

    /* log the read: */
    _tme_ncr53c9x_debug_reg(ncr53c9x, address, TME_NCR53C9X_DEBUG_REG_READ, value);

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init,
			   &value,
			   TME_BUS8_LOG2);
  }

  /* otherwise, this is a write: */
  else {
    assert ((cycle_init->tme_bus_cycle_type & TME_BUS_CYCLE_WRITE) != 0);

    /* run the bus cycle: */
    tme_bus_cycle_xfer_reg(cycle_init,
			   &value,
			   TME_BUS8_LOG2);
    
    /* turn the address into a proper register number: */
    if ((TME_NCR53C9X_REGS_RW & TME_BIT(address)) != 0) {
      address = TME_NCR53C9X_REG_RW(address);
    }
    else {
      address = TME_NCR53C9X_REG_WO(address);
    }

    /* log the write: */
    _tme_ncr53c9x_debug_reg(ncr53c9x, address, TME_NCR53C9X_DEBUG_REG_WRITE, value);

    /* dispatch on the register: */
    switch (address) {

      /* writes of these registers are simple: */
    case TME_NCR53C9X_REG_STC_LSB:
    case TME_NCR53C9X_REG_STC_MSB:
    case TME_NCR53C9X_REG_SDID:
    case TME_NCR53C9X_REG_TIMEOUT:
    case TME_NCR53C9X_REG_SYNCH_PERIOD:
    case TME_NCR53C9X_REG_SYNCH_OFFSET:
    case TME_NCR53C9X_REG_CONTROL1:
    case TME_NCR53C9X_REG_CLOCK_FACTOR:
    case TME_NCR53C9X_REG_CONTROL3:
    case TME_NCR53C9X_REG_ALIGN:
      ncr53c9x->tme_ncr53c9x_regs[address] = value;
      break;

      /* a data FIFO write: */
    case TME_NCR53C9X_REG_FIFO:
      TME_NCR53C9X_DEBUG_BP(write_fifo);

      /* if the data FIFO is full: */
      fifo_head = ncr53c9x->tme_ncr53c9x_fifo_data_head;
      if (((fifo_head + 1) % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data))
	  == ncr53c9x->tme_ncr53c9x_fifo_data_tail) {

	/* set Illegal Operation in the STAT register value at the
           head of the status FIFO: */
	fifo_head = ncr53c9x->tme_ncr53c9x_fifo_status_head;
	ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_stat |= TME_NCR53C9X_STAT_IOE;
      }

      /* otherwise, the data FIFO is not full: */
      else {

	/* add this data to the data FIFO: */
	ncr53c9x->tme_ncr53c9x_fifo_data[fifo_head] = value;
	ncr53c9x->tme_ncr53c9x_fifo_data_head = (fifo_head + 1) % TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_data);
	_tme_ncr53c9x_fifo_data_update(ncr53c9x);

	/* since we may be running a non-DMA command that is waiting
	   for data to transfer, call out a SCSI cycle, because we now
	   have some data: */
	ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
      }
      break;

      /* "Commands to the device are issued by writing to this
         register.  This register is two deep which allows for command
         queuing.  The second command can be issued before the first
         one is completed.  The Reset command and the Stop DMA command
         are not queued and are executed immediately." */
    case TME_NCR53C9X_REG_CMD:

      /* if this is the Reset Device command: */
      if ((value & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_RESET) {

	/* "The Reset Device Command immediately stops any device
	   operation and resets all the functions of the device.  It
	   returns the device to the disconnected state and it also
	   generates a hard reset.  The Reset Device Command remains
	   on the top of the Command Register FIFO holding the device
	   in the reset state until the No Operation Command is
	   loaded. The No Operation command serves to enable the
	   Command Register."  */
	_tme_ncr53c9x_reset(ncr53c9x, TME_NCR53C9X_RESET_DEVICE | TME_NCR53C9X_RESET_FLAG_CMD);
      }

      /* if the active command is the Reset Device command, and this
	 is not a No Operation command, ignore it: */
      fifo_tail = ncr53c9x->tme_ncr53c9x_fifo_cmd_tail;
      if (((ncr53c9x->tme_ncr53c9x_fifo_cmd[fifo_tail] & TME_NCR53C9X_CMD_MASK)
	   == TME_NCR53C9X_CMD_RESET)
	  && ((value & TME_NCR53C9X_CMD_MASK) != TME_NCR53C9X_CMD_NOP)) {
	break;
      }

      /* if this is the DMA Stop command: */
      if ((value & TME_NCR53C9X_CMD_MASK) == TME_NCR53C9X_CMD_DMA_STOP) {

	/* XXX FIXME - we don't support any of the Target mode commands yet: */
	abort();
      }

      /* if the command FIFO is already full: */
      fifo_head = (ncr53c9x->tme_ncr53c9x_fifo_cmd_head + 1);
      if (fifo_head == TME_ARRAY_ELS(ncr53c9x->tme_ncr53c9x_fifo_cmd)) {
	fifo_head = 0;
      }
      if (fifo_head == fifo_tail) {

	/* set Illegal Operation in the STAT register value at the
           head of the status FIFO: */
	fifo_head = ncr53c9x->tme_ncr53c9x_fifo_status_head;
	ncr53c9x->tme_ncr53c9x_fifo_status[fifo_head].tme_ncr53c9x_status_stat |= TME_NCR53C9X_STAT_IOE;
      }

      /* otherwise, make this command pending: */
      else {
	TME_NCR53C9X_DEBUG_BP(write_cmd);

	/* add this command to the command FIFO: */
	ncr53c9x->tme_ncr53c9x_fifo_cmd[fifo_head] = value;
	ncr53c9x->tme_ncr53c9x_fifo_cmd_head = fifo_head;
      }
      break;

      /* a write of Control register two depends on the variant: */
    case TME_NCR53C9X_REG_CONTROL2:
      switch (ncr53c9x->tme_ncr53c9x_variant) {
      default: assert(FALSE);

	/* the ESP100 doesn't have Control register two: */
      case TME_NCR53C9X_VARIANT_ESP100: break;
      }
      break;

      /* an unknown register: */
    default: abort();
    }
  }
  
  /* run the update function: */
  _tme_ncr53c9x_update(ncr53c9x);

  /* make any callouts: */
  _tme_ncr53c9x_callout(ncr53c9x);

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);

  /* no faults: */
  return (TME_OK);
}

/* the NCR 53c9x TLB filler: */
static int
_tme_ncr53c9x_tlb_fill(void *_ncr53c9x, 
		       struct tme_bus_tlb *tlb, 
		       tme_bus_addr_t address,
		       unsigned int cycles)
{
  struct tme_ncr53c9x *ncr53c9x;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) _ncr53c9x;

  /* the address must be within range: */
  assert(address < TME_NCR53C9X_SIZ_REGS);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers all registers: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = TME_NCR53C9X_SIZ_REGS - 1;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle = _tme_ncr53c9x_bus_cycle;

  /* our bus cycle handler private data: */
  tlb->tme_bus_tlb_cycle_private = ncr53c9x;

  return (TME_OK);
}

/* this makes a new bus connection: */
static int
_tme_ncr53c9x_connection_make_bus(struct tme_connection *conn,
				  unsigned int state)
{
  struct tme_ncr53c9x *ncr53c9x;
  struct tme_bus_connection *conn_bus;
  int rc;

  /* recover our data structure: */
  ncr53c9x = conn->tme_connection_element->tme_element_private;

  /* call the bus device connection maker: */
  rc = tme_bus_device_connection_make(conn, state);

  /* if the full connection was successful, and we don't have a TLB
     set yet, allocate it: */
  if (rc == TME_OK
      && state == TME_CONNECTION_FULL
      && !ncr53c9x->tme_ncr53c9x_dma_tlb_added) {

    /* get our bus connection: */
    conn_bus
      = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
				       ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection,
				       &ncr53c9x->tme_ncr53c9x_device.tme_bus_device_connection_rwlock);

    /* allocate the TLB set: */
    rc = tme_bus_device_tlb_set_add(&ncr53c9x->tme_ncr53c9x_device,
				    1, 
				    &ncr53c9x->tme_ncr53c9x_dma_tlb);
    assert (rc == TME_OK);
    ncr53c9x->tme_ncr53c9x_dma_tlb_added = TRUE;
  }

  return (rc);
}

/* this makes a new SCSI connection: */
static int
_tme_ncr53c9x_connection_make_scsi(struct tme_connection *conn,
				   unsigned int state)
{
  struct tme_ncr53c9x *ncr53c9x;
  struct tme_scsi_connection *conn_scsi;
  struct tme_scsi_connection *conn_scsi_other;

  /* recover our data structures: */
  ncr53c9x = conn->tme_connection_element->tme_element_private;
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
    tme_mutex_lock(&ncr53c9x->tme_ncr53c9x_mutex);

    /* save our connection: */
    ncr53c9x->tme_ncr53c9x_scsi_connection = conn_scsi_other;

    /* call out a cycle that asserts no signals and runs the
       wait-change action: */
    ncr53c9x->tme_ncr53c9x_active_scsi_events = TME_SCSI_EVENT_NONE;
    ncr53c9x->tme_ncr53c9x_out_scsi_events = TME_SCSI_EVENT_BUS_CHANGE;
    ncr53c9x->tme_ncr53c9x_out_scsi_actions = TME_SCSI_ACTION_NONE;
    ncr53c9x->tme_ncr53c9x_out_scsi_control = 0;
    ncr53c9x->tme_ncr53c9x_out_scsi_data = 0;
    ncr53c9x->tme_ncr53c9x_callout_flags |= TME_NCR53C9X_CALLOUT_SCSI_CYCLE;
    _tme_ncr53c9x_callout(ncr53c9x);

    /* unlock our mutex: */
    tme_mutex_unlock(&ncr53c9x->tme_ncr53c9x_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_ncr53c9x_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a NCR 53c9x: */
static int
_tme_ncr53c9x_connections_new(struct tme_element *element,
			      const char * const *args,
			      struct tme_connection **_conns,
			      char **_output)
{
  struct tme_ncr53c9x *ncr53c9x;
  struct tme_scsi_connection *conn_scsi;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  ncr53c9x = (struct tme_ncr53c9x *) element->tme_element_private;

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
	= _tme_ncr53c9x_connection_make_bus;
    }
  }

  /* if we don't have a SCSI connection, make one: */
  if (ncr53c9x->tme_ncr53c9x_scsi_connection == NULL) {

    /* allocate the new SCSI connection: */
    conn_scsi = tme_new0(struct tme_scsi_connection, 1);
    conn = &conn_scsi->tme_scsi_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SCSI;
    conn->tme_connection_score = tme_scsi_connection_score;
    conn->tme_connection_make = _tme_ncr53c9x_connection_make_scsi;
    conn->tme_connection_break = _tme_ncr53c9x_connection_break;

    /* fill in the SCSI connection: */
    conn_scsi->tme_scsi_connection_cycle = _tme_ncr53c9x_scsi_cycle;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new NCR 53c9x function: */
TME_ELEMENT_NEW_DECL(tme_ic_ncr53c9x) {
  struct tme_ncr53c9x *ncr53c9x;
  unsigned int variant;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  arg_i = 1;
  variant = TME_NCR53C9X_VARIANT_NULL;
  for (;;) {

    if (TME_ARG_IS(args[arg_i + 0], "variant")) {
      if (args[arg_i + 1] == NULL) {
	tme_output_append_error(_output,
				"%s, ",
				_("missing variant"));
	usage = TRUE;
	break;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "esp100")) {
	variant = TME_NCR53C9X_VARIANT_ESP100;
      }
      else if (TME_ARG_IS(args[arg_i + 1], "esp100a")) {
	variant = TME_NCR53C9X_VARIANT_ESP100A;
      }
      else {
	tme_output_append_error(_output,
				"%s %s, ",
				_("bad variant"),
				args[arg_i + 1]);
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

  if (variant == TME_NCR53C9X_VARIANT_NULL) {
    tme_output_append_error(_output,
			    "%s, ",
			    _("missing variant"));
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output, 
			    "%s %s variant { esp100 | esp100a }",
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the NCR 53c9x structure: */
  ncr53c9x = tme_new0(struct tme_ncr53c9x, 1);
  ncr53c9x->tme_ncr53c9x_variant = variant;
  ncr53c9x->tme_ncr53c9x_element = element;
  tme_mutex_init(&ncr53c9x->tme_ncr53c9x_mutex);

  /* initialize our simple bus device descriptor: */
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_element = element;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_tlb_fill = _tme_ncr53c9x_tlb_fill;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_address_last = TME_NCR53C9X_SIZ_REGS - 1;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_signal = _tme_ncr53c9x_signal;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_lock = _tme_ncr53c9x_lock;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_unlock = _tme_ncr53c9x_unlock;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_tlb_hash = _tme_ncr53c9x_tlb_hash;
  ncr53c9x->tme_ncr53c9x_device.tme_bus_device_router = tme_bus_device_router_16eb;

  /* fill the element: */
  element->tme_element_private = ncr53c9x;
  element->tme_element_connections_new = _tme_ncr53c9x_connections_new;

  /* reset the device: */
  _tme_ncr53c9x_reset(ncr53c9x, TME_NCR53C9X_RESET_DEVICE);

  /* initialize the timeout thread condition: */
  tme_cond_init(&ncr53c9x->tme_ncr53c9x_timeout_cond);

  /* start the timeout thread: */
  tme_thread_create((tme_thread_t) _tme_ncr53c9x_timeout_th, ncr53c9x);

  return (TME_OK);
}
