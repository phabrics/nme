/* $Id: z8530.c,v 1.20 2010/06/05 16:07:17 fredette Exp $ */

/* ic/z8530.c - implementation of Zilog 8530 emulation: */

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
_TME_RCSID("$Id: z8530.c,v 1.20 2010/06/05 16:07:17 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/serial.h>
#include <tme/ic/z8530.h>
#include "z8530reg.h"

/* macros: */

/* buffer sizes: */
#define TME_Z8530_BUFFER_SIZE_TX	(16)
#define TME_Z8530_BUFFER_SIZE_RX	(128)

/* channel callout flags: */
#define TME_Z8530_CALLOUT_CHECK		(0)
#define TME_Z8530_CALLOUT_RUNNING	TME_BIT(0)
#define TME_Z8530_CALLOUTS_MASK		(-2)
#define  TME_Z8530_CALLOUT_CTRL		TME_BIT(1)
#define  TME_Z8530_CALLOUT_CONFIG	TME_BIT(2)
#define  TME_Z8530_CALLOUT_READ		TME_BIT(3)
#define	 TME_Z8530_CALLOUT_INT		TME_BIT(4)

/* interrupt types.  note that the nonnegative values are chosen to
   correspond to the interrupt vector modification values for the
   given interrupt.  the nonnegative values correspond to the
   "status high/status low = 0" (TME_Z8530_WR9_INTVEC_STATUS = 0)
   case given in Table 5-6 in my SCC manual: */
#define TME_Z8530_INT_UNDEF		(-1)
#define TME_Z8530_INT_CHAN_TX		(0)
#define TME_Z8530_INT_CHAN_STATUS	(1)
#define TME_Z8530_INT_CHAN_RX		(2)
#define TME_Z8530_INT_CHAN_RX_SPCL	(3)
#define TME_Z8530_INT_CHAN_A		(4)
#define TME_Z8530_INT_CHAN_B		(0)

#define TME_Z8530_LOG_HANDLE(z) (&(z)->tme_z8530_element->tme_element_log_handle)

/* structures: */

/* one channel: */
struct tme_z8530_chan {

  /* the 16 write registers: */
  tme_uint8_t tme_z8530_chan_wrreg[16];

  /* the 16 read registers: */
  tme_uint8_t tme_z8530_chan_rdreg[16];

  /* the unlatched RR0 status bits and diff mask: */
  tme_uint8_t tme_z8530_chan_rr0_status_raw;
  tme_uint8_t tme_z8530_chan_rr0_status_diff_mask;

  /* the serial connected to this channel: */
  struct tme_serial_connection *tme_z8530_chan_connection;

  /* our transmitter buffer: */
  struct tme_serial_buffer tme_z8530_chan_buffer_tx;

  /* our receiver buffer: */
  struct tme_serial_buffer tme_z8530_chan_buffer_rx;

  /* the flags on this channel: */
  unsigned int tme_z8530_chan_flags;

  /* the callout flags on this channel: */
  int tme_z8530_chan_callout_flags;

#ifndef TME_NO_LOG
  tme_uint8_t tme_z8530_chan_last_read_reg;
  tme_uint8_t tme_z8530_chan_last_read_value;
#endif /* !TME_NO_LOG */
};

/* a channel connection: */ 
struct tme_z8530_connection {

  /* the generic serial connection: */
  struct tme_serial_connection tme_z8530_connection;

  /* the channel this connection is to: */
  struct tme_z8530_chan *tme_z8530_connection_chan;
};

/* the whole chip: */
struct tme_z8530 {

  /* our simple bus device header: */
  struct tme_bus_device tme_z8530_device;
#define tme_z8530_element tme_z8530_device.tme_bus_device_element

  /* our socket: */
  struct tme_z8530_socket tme_z8530_socket;
#define tme_z8530_address_chan_a tme_z8530_socket.tme_z8530_socket_address_chan_a
#define tme_z8530_address_chan_b tme_z8530_socket.tme_z8530_socket_address_chan_b
#define tme_z8530_offset_csr tme_z8530_socket.tme_z8530_socket_offset_csr
#define tme_z8530_offset_data tme_z8530_socket.tme_z8530_socket_offset_data
#define tme_z8530_port_least_lane tme_z8530_socket.tme_z8530_socket_port_least_lane
#define tme_z8530_pclk tme_z8530_socket.tme_z8530_socket_pclk

  /* the mutex protecting the chip: */
  tme_mutex_t tme_z8530_mutex;

  /* the two channels, and some common shared registers: */
  struct tme_z8530_chan tme_z8530_chan_a;
  struct tme_z8530_chan tme_z8530_chan_b;
#define tme_z8530_wr9 tme_z8530_chan_a.tme_z8530_chan_wrreg[9]
#define tme_z8530_rr3 tme_z8530_chan_a.tme_z8530_chan_rdreg[3]

  /* the IUS bits, which correspond exactly to bits in RR3: */
  tme_uint8_t tme_z8530_ius;
  
  /* the register pointer: */
  tme_uint8_t tme_z8530_register_pointer;

  /* if our interrupt line is currently asserted: */
  int tme_z8530_int_asserted;
};

/* the z8530 bus router: */
static const tme_bus_lane_t tme_z8530_router[TME_BUS_ROUTER_SIZE(TME_BUS8_LOG2)] = {
  
  /* [gen]  initiator port size: 8 bits
     [gen]  initiator port least lane: 0: */
  /* D7-D0 */	TME_BUS_LANE_ROUTE(0),
};

/* this resets one channel of a z8530: */
static void
_tme_z8530_channel_reset(struct tme_z8530 *z8530,
			 struct tme_z8530_chan *chan,
			 int hardware_reset)
{
  
  /* these reset values are taken from the SCC User's Manual: */
  chan->tme_z8530_chan_wrreg[0] = 0x00;
  chan->tme_z8530_chan_wrreg[1] = 0x00;
  chan->tme_z8530_chan_wrreg[2] = 0x00;
  chan->tme_z8530_chan_wrreg[3] = 0x00;
  chan->tme_z8530_chan_wrreg[4] = 0x04;
  chan->tme_z8530_chan_wrreg[5] = 0x00;
  chan->tme_z8530_chan_wrreg[6] = 0x00;
  chan->tme_z8530_chan_wrreg[7] = 0x00;
  chan->tme_z8530_chan_wrreg[8] = 0x00;
  z8530->tme_z8530_wr9 = (hardware_reset ? 0xc0 : 0x00);
  chan->tme_z8530_chan_wrreg[10] = 0x00;
  chan->tme_z8530_chan_wrreg[11] = 0x08;
  chan->tme_z8530_chan_wrreg[12] = 0x00;
  chan->tme_z8530_chan_wrreg[13] = 0x00;
  chan->tme_z8530_chan_wrreg[14] = (hardware_reset ? 0x30 : 0x20);
  chan->tme_z8530_chan_wrreg[15] = 0xf8;
  chan->tme_z8530_chan_rdreg[0] = 0x44;
  chan->tme_z8530_chan_rdreg[1] = 0x07;
  z8530->tme_z8530_rr3 = 0x00;
  chan->tme_z8530_chan_rdreg[10] = 0x00;

  /* the raw (unlatched) RR0 status bits: */
  chan->tme_z8530_chan_rr0_status_raw = 0;
  chan->tme_z8530_chan_rr0_status_diff_mask = 0;

  /* if this is a hardware reset: */
  if (hardware_reset) {

    /* clear the IUS bits: */
    z8530->tme_z8530_ius = 0;

    /* set the modified interrupt vector stored in RR2 of channel B: */
    z8530->tme_z8530_chan_b.tme_z8530_chan_rdreg[2] = (TME_Z8530_INT_CHAN_RX_SPCL << 1);
  }
}

/* this initializes one channel of a z8530: */
static void
_tme_z8530_channel_init(struct tme_z8530 *z8530,
			struct tme_z8530_chan *chan)
{

  /* allocate the Tx and Rx FIFOs: */  
  tme_serial_buffer_init(&chan->tme_z8530_chan_buffer_tx, 
			 TME_Z8530_BUFFER_SIZE_TX);
  tme_serial_buffer_init(&chan->tme_z8530_chan_buffer_rx, 
			 TME_Z8530_BUFFER_SIZE_RX);

  /* reset the channel: */
  _tme_z8530_channel_reset(z8530, chan, TRUE);
}

/* this returns TRUE iff the data at the top of the Rx FIFO has a
   Special Condition.  Special Conditions are: receiver overrun,
   framing error, end of frame, or, (if selected) parity error: */
static int
_tme_z8530_rx_fifo_special(struct tme_z8530_chan *chan)
{
  return ((chan->tme_z8530_chan_rdreg[0] & TME_Z8530_RR0_RX_AVAIL)
	  && (chan->tme_z8530_chan_rdreg[1]
	      & (TME_Z8530_RR1_END_OF_FRAME
		 | TME_Z8530_RR1_ERROR_FRAME
		 | TME_Z8530_RR1_ERROR_RX_OVERRUN
		 | ((chan->tme_z8530_chan_wrreg[1]
		     & TME_Z8530_WR1_PARITY_SPCL)
		    ? TME_Z8530_RR1_ERROR_PARITY
		    : 0))));
}

/* iff the data at the top of the Rx FIFO has a Special Condition
   exists, and the receive interrupt mode is either
   TME_Z8530_WR1_RX_INT_1ST_SPCL or TME_Z8530_WR1_RX_INT_SPCL, the
   data is held in the receive FIFO until an Error Reset command is
   issued: */
static int
_tme_z8530_rx_fifo_hold(struct tme_z8530_chan *chan)
{
  tme_uint8_t wr1;

  /* if a special condition exists and needs to be held in the
     receive FIFO: */
  wr1 = chan->tme_z8530_chan_wrreg[1];
  return (_tme_z8530_rx_fifo_special(chan)
	  && (((wr1 & TME_Z8530_WR1_RX_INT_MASK)
	       == TME_Z8530_WR1_RX_INT_1ST_SPCL)
	      || ((wr1 & TME_Z8530_WR1_RX_INT_MASK)
		  == TME_Z8530_WR1_RX_INT_SPCL)));
}

/* this attempts to refill the receive FIFO: */
static int
_tme_z8530_rx_fifo_refill(struct tme_z8530 *z8530,
			  struct tme_z8530_chan *chan)
{
  tme_uint8_t byte_buffer, byte;
  tme_serial_data_flags_t data_flags_buffer, data_flags;
  tme_uint8_t rr1, rr3_ip_rx;
  int ip_rx_set;
  int new_callouts;
  unsigned int rc;

  /* get the IP_RX bit for this channel: */
  rr3_ip_rx = ((chan == &z8530->tme_z8530_chan_a)
	       ? TME_Z8530_RR3_CHAN_A_IP_RX
	       : TME_Z8530_RR3_CHAN_B_IP_RX);

  /* if the data at the top of the Rx FIFO needs to be held, 
     there's nothing we can do: */
  if (_tme_z8530_rx_fifo_hold(chan)) {
    return (0);
  }

  /* assume we won't need any new callouts, and that the receive
     Interrupt Pending bit for this channel is not supposed to be set: */
  new_callouts = 0;
  ip_rx_set = FALSE;

  /* if the Rx buffer is currently full, after we copy out the
     next character we want to call out to read more data: */
  if (tme_serial_buffer_is_full(&chan->tme_z8530_chan_buffer_rx)) {
    new_callouts |= TME_Z8530_CALLOUT_READ;
  }
	
  /* get the next byte from our Rx buffer: */
  rc = tme_serial_buffer_copyout(&chan->tme_z8530_chan_buffer_rx,
				 &byte_buffer, 1,
				 &data_flags_buffer,
				 TME_SERIAL_COPY_NORMAL);
	
  /* if the Rx buffer was empty, the Rx FIFO is now empty: */
  if (rc == 0) {
    chan->tme_z8530_chan_rdreg[0] &= ~TME_Z8530_RR0_RX_AVAIL;
  }
	
  /* otherwise we have another byte for the Rx FIFO: */
  else {
    byte = byte_buffer;
    data_flags = data_flags_buffer;

    /* put the byte into RR8: */
    chan->tme_z8530_chan_rdreg[8] = byte;

    /* update RR1: */
    rr1 = chan->tme_z8530_chan_rdreg[1];
    if (data_flags & TME_SERIAL_DATA_BAD_FRAME) {
      rr1 |= TME_Z8530_RR1_ERROR_FRAME;
    }
    else {
      rr1 &= ~TME_Z8530_RR1_ERROR_FRAME;
    }
    if (data_flags & TME_SERIAL_DATA_OVERRUN) {
      rr1 |= TME_Z8530_RR1_ERROR_RX_OVERRUN;
    }
    if (data_flags & TME_SERIAL_DATA_BAD_PARITY) {
      rr1 |= TME_Z8530_RR1_ERROR_PARITY;
    }
    chan->tme_z8530_chan_rdreg[1] = rr1;
    
    /* see if the receive Interrupt Pending bit is supposed to be set: */
    switch (chan->tme_z8530_chan_wrreg[1] & TME_Z8530_WR1_RX_INT_MASK) {
    case TME_Z8530_WR1_RX_INT_DISABLE:
      break;
    case TME_Z8530_WR1_RX_INT_1ST_SPCL:
      ip_rx_set = (!(chan->tme_z8530_chan_rdreg[0] & TME_Z8530_RR0_RX_AVAIL)
		   || _tme_z8530_rx_fifo_special(chan));
      break;
    case TME_Z8530_WR1_RX_INT_ALL_SPCL:
      ip_rx_set = TRUE;
      break;
    case TME_Z8530_WR1_RX_INT_SPCL:
      ip_rx_set = _tme_z8530_rx_fifo_special(chan);
      break;
    }

    /* there is now a receive character available: */
    chan->tme_z8530_chan_rdreg[0] |= TME_Z8530_RR0_RX_AVAIL;
  }

  /* if the receive Interrupt Pending bit for this channel doesn't
     have the proper value, update it and callout an int: */
  if (!ip_rx_set != !(z8530->tme_z8530_rr3
		      & rr3_ip_rx)) {
    z8530->tme_z8530_rr3 ^= rr3_ip_rx;
    new_callouts |= TME_Z8530_CALLOUT_INT;
  }

  /* done: */
  return (new_callouts);
}

/* if an External/Status interrupt is not pending on this channel,
   this updates the status bits in RR0 with the current raw status,
   and requests an External/Status interrupt if one is needed: */
static int
_tme_z8530_rr0_update(struct tme_z8530 *z8530,
		      struct tme_z8530_chan *chan)
{
  tme_uint8_t rr0_status_new;
  tme_uint8_t rr0_status_diff_mask;
  tme_uint8_t wr15;
  tme_uint8_t rr3_ip_status;
  int need_ip_status;
  
  /* get the IP_STATUS bit for this channel: */
  rr3_ip_status = ((chan == &z8530->tme_z8530_chan_a)
		   ? TME_Z8530_RR3_CHAN_A_IP_STATUS
		   : TME_Z8530_RR3_CHAN_B_IP_STATUS);

  /* if an External/Status interrupt is already pending on this
     channel, return, and we need no new callouts: */
  if (z8530->tme_z8530_rr3 & rr3_ip_status) {
    return (0);
  }

  /* assume no IP_STATUS needed: */
  need_ip_status = FALSE;

  /* get the raw status and its interrupt mask: */
  rr0_status_new = chan->tme_z8530_chan_rr0_status_raw;
  rr0_status_diff_mask = chan->tme_z8530_chan_rr0_status_diff_mask;

  /* if interrupts are enabled: */
  if (chan->tme_z8530_chan_wrreg[1] & TME_Z8530_WR1_STATUS_INT_ENABLE) {
    wr15 = chan->tme_z8530_chan_wrreg[15];

    /* if CTS interrupts are enabled, and CTS has changed, we need a
       status interrupt.  similarly for DCD: */
    if (((wr15 & TME_Z8530_WR15_CTS_IE)
	 && (rr0_status_diff_mask & TME_Z8530_RR0_CTS))
	|| ((wr15 & TME_Z8530_WR15_DCD_IE)
	    && (rr0_status_diff_mask & TME_Z8530_RR0_DCD))) {
      need_ip_status = TRUE;
    }
    
    /* if break interrupts are enabled, and one or more break
       changes have happened: */
    if ((wr15 & TME_Z8530_WR15_BREAK_IE)
	&& (rr0_status_diff_mask & TME_Z8530_RR0_BREAK)) {

      /* we need a status interrupt: */
      need_ip_status = TRUE;

      /* make sure we interrupt for multiple transitions if multiple
	 transitions have happened.  i.e., if both the latched RR0 and
	 the raw status have the same value for BREAK, at least two
	 transitions happened "in between", so interrupt for the other
	 value first, and leave break set in the diff mask so another
	 interrupt for the actual current value will happen later: */
      if (((rr0_status_new
	    ^ chan->tme_z8530_chan_rdreg[0])
	   & TME_Z8530_RR0_BREAK) == 0) {
	rr0_status_new ^= TME_Z8530_RR0_BREAK;
      }
    }
  }

  /* update RR0: */
  chan->tme_z8530_chan_rdreg[0] =
    ((chan->tme_z8530_chan_rdreg[0]
      & ~rr0_status_diff_mask)
     | (rr0_status_new
	& rr0_status_diff_mask));

  /* update the status diff mask: */
  chan->tme_z8530_chan_rr0_status_diff_mask =
    (rr0_status_new
     ^ chan->tme_z8530_chan_rr0_status_raw);
  
  /* if this channel needs an External/Status interrupt: */
  if (need_ip_status) {
    z8530->tme_z8530_rr3 |= rr3_ip_status;
    return (TME_Z8530_CALLOUT_INT);
  }

  /* we don't need any callouts: */
  return (0);
}

/* this updates the modified interrupt vector stored in RR2 of
   channel B: */
static int
_tme_z8530_rr2_update(struct tme_z8530 *z8530,
		      tme_uint8_t rr3_ip)
{
  struct tme_z8530_chan *chan;
  int int_type;
  int vector;

  /* there can be at most one RR3 IP bit: */
  assert ((rr3_ip & (rr3_ip - 1)) == 0);

  /* get which channel this IP bit is for, and make sure the IP bit is
     shifted down to a TME_Z8530_RR3_CHAN_B_IP_ value: */
  if (rr3_ip <= TME_Z8530_RR3_CHAN_B_IP_RX) {    
    chan = &z8530->tme_z8530_chan_b;
    rr3_ip /= TME_Z8530_RR3_CHAN_B_IP_STATUS;
    int_type = TME_Z8530_INT_CHAN_B;
  }
  else {
    chan = &z8530->tme_z8530_chan_a;
    rr3_ip /= TME_Z8530_RR3_CHAN_A_IP_STATUS;
    int_type = TME_Z8530_INT_CHAN_A;
  }

  /* map that bit into an interrupt type: */
  switch (rr3_ip) {
  case TME_Z8530_RR3_CHAN_B_IP_RX:
    int_type
      |= (_tme_z8530_rx_fifo_special(chan)
	  ? TME_Z8530_INT_CHAN_RX_SPCL
	  : TME_Z8530_INT_CHAN_RX);
    break;
  case TME_Z8530_RR3_CHAN_B_IP_TX:
    int_type
      |= TME_Z8530_INT_CHAN_TX;
    break;
  case TME_Z8530_RR3_CHAN_B_IP_STATUS:
    int_type
      |= TME_Z8530_INT_CHAN_STATUS;
    break;
  default:
    /* "If no interrupts are pending, the status is V3,V2,V1 -011, or
       V6,V5,V4-110." */
    int_type = TME_Z8530_INT_CHAN_RX_SPCL;
    break;
  }

  /* come up with the modified interrupt vector.  iff
     TME_Z8530_WR9_INTVEC_STATUS, bits 4, 5, and 6 respectively take
     bits 2, 1, and 0 of the interrupt type, otherwise bits 1, 2, and
     3 respectively take bits 0, 1, and 2 of the interrupt type.  this
     strangness is found in Table 5-6 of my SCC manual: */
  vector = z8530->tme_z8530_chan_a.tme_z8530_chan_wrreg[2];
  if (z8530->tme_z8530_wr9 & TME_Z8530_WR9_INTVEC_STATUS) {
    vector
      = ((vector & 0x8f)
	 | ((vector << 6) & 0x40)
	 | ((vector << 4) & 0x20)
	 | ((vector << 3) & 0x10));
  }
  else {
    vector = (vector & 0xf1) | (int_type << 1);
  }

  /* save this modified interrupt vector in the channel B RR2: */
  z8530->tme_z8530_chan_b.tme_z8530_chan_rdreg[2] = vector;

  /* return the vector: */
  return (vector);
}

/* this updates RR2, and returns any pending interrupt with higher
   priority than the highest-priority interrupt currently under
   service, or zero if there is no such interrupt (or if interrupts
   are disabled): */
static int
_tme_z8530_int_pending(struct tme_z8530 *z8530)
{
  tme_uint8_t rr3;

  /* get the highest priority IP bit: */
  rr3 = z8530->tme_z8530_rr3;
  for (; (rr3 & (rr3 - 1)) != 0; ) {
    rr3 &= (rr3 - 1);
  }
  assert(rr3 <= TME_Z8530_RR3_CHAN_A_IP_RX);

  /* if the highest priority IP bit is currently under service: */
  if (rr3 <= z8530->tme_z8530_ius) {

    /* there is no interrupt pending: */
    rr3 = 0;
  }

  /* if interrupts are disabled: */
  if ((z8530->tme_z8530_wr9 & TME_Z8530_WR9_MIE) == 0) {

    /* there is no interrupt pending: */
    rr3 = 0;
  }

  /* update RR2: */
  _tme_z8530_rr2_update(z8530,
			rr3);

  return (rr3);
}

/* this does an interrupt acknowledge.  it returns TME_OK iff an
   interrupt was acknowledged, else it returns some error.  iff
   _vector is non-NULL this is a hardware interrupt acknowledge: */
static int
_tme_z8530_intack(struct tme_z8530 *z8530, int *_vector)
{
  tme_uint8_t rr3, wr9;
  int vector;

  /* if there is no pending interrupt with a higher priority than the
     highest-priority interrupt currently under service, or if the
     Master Interrupt Enable isn't set, the interrupt signal should
     not have been asserted: */
  rr3 = _tme_z8530_int_pending(z8530);
  if (rr3 == 0) {
    return (ENOENT);
  }

  /* if this is a hard interrupt acknowledge: */
  if (_vector != NULL) {

    /* if the chip's IEI pin is tied low: */
    if (z8530->tme_z8530_socket.tme_z8530_socket_flags
	& TME_Z8530_SOCKET_FLAG_IEI_TIED_LOW) {

      /* when IEI is low, we return without error, but we also don't
	 recognize the hard interrupt acknowledge cycle as selecting us
	 and we don't drive any vector: */
      *_vector = TME_BUS_INTERRUPT_VECTOR_UNDEF;
      return (TME_OK);
    }

    /* otherwise, we assume that the IEI pin is currently high, which
       means we always recognize the hard interrupt acknowledge cycle
       as selecting us.  this isn't a problem, even if there are
       multiple devices that can interrupt at our level, because we're
       not really driving a bus here, we're responding to a directed
       request for an interrupt vector.  we assume that if there
       really is an important priority system within our interrupt
       level, that it's enforced by the bus implementation: */
  }

  /* set the corresponding IUS bit: */
  z8530->tme_z8530_ius |= rr3;

  /* get the vector from the updated RR2 in channel B: */
  vector = z8530->tme_z8530_chan_b.tme_z8530_chan_rdreg[2];

  /* get the current WR9 value: */
  wr9 = z8530->tme_z8530_wr9;

  /* if this is a hard interrupt acknowledge: */
  if (_vector != NULL) {
  
    /* if we're not supposed to acknowledge hard interrupts with any
       vector, return no vector: */
    if (wr9 & TME_Z8530_WR9_NV) {
      *_vector = TME_BUS_INTERRUPT_VECTOR_UNDEF;
    }

    /* if we're supposed to acknowledge hard interrupts with the
       modified vector, return the modified vector: */
    else if (wr9 & TME_Z8530_WR9_VIS) {
      *_vector = vector;
    }

    /* otherwise, return the plain interrupt vector: */
    else {
      *_vector = z8530->tme_z8530_chan_a.tme_z8530_chan_wrreg[2];
    }
  }

  /* done: */
  return (TME_OK);
}

/* the z8530 callout function.  it must be called with the mutex locked: */
static void
_tme_z8530_callout(struct tme_z8530 *z8530, struct tme_z8530_chan *chan, int new_callouts)
{
  struct tme_serial_connection *conn_serial;
  struct tme_bus_connection *conn_bus;
  unsigned int ctrl;
  int int_asserted;
  tme_uint8_t buffer_input[32];
  unsigned int buffer_input_size;
  tme_serial_data_flags_t data_flags;
  struct tme_serial_config config;
  unsigned int chan_flags;
  int callouts;
  int later_callouts_a, later_callouts_b, *_later_callouts;
  int rc;
  
  /* add in any new callouts: */
  chan->tme_z8530_chan_callout_flags |= new_callouts;

  /* if this function is already running in another thread, return
     now.  the other thread will do our work: */
  if ((z8530->tme_z8530_chan_a.tme_z8530_chan_callout_flags
       | z8530->tme_z8530_chan_b.tme_z8530_chan_callout_flags)
      & TME_Z8530_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  z8530->tme_z8530_chan_a.tme_z8530_chan_callout_flags |= TME_Z8530_CALLOUT_RUNNING;
  z8530->tme_z8530_chan_b.tme_z8530_chan_callout_flags |= TME_Z8530_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts_a = later_callouts_b = 0;

  /* we assume we will not change our interrupt vector: */
  int_asserted = -1;

  /* loop while callouts are needed: */
  for (;;) {

    /* stop only when both channels need no callouts: */
    if ((callouts
	 = z8530->tme_z8530_chan_a.tme_z8530_chan_callout_flags)
	& TME_Z8530_CALLOUTS_MASK) {
      chan = &z8530->tme_z8530_chan_a;
      _later_callouts = &later_callouts_a;
    }
    else if ((callouts
	      = z8530->tme_z8530_chan_b.tme_z8530_chan_callout_flags)
	     & TME_Z8530_CALLOUTS_MASK) {
      chan = &z8530->tme_z8530_chan_b;
      _later_callouts = &later_callouts_b;
    }
    else {
      break;
    }

    /* clear the needed callouts: */
    chan->tme_z8530_chan_callout_flags = callouts & ~TME_Z8530_CALLOUTS_MASK;
      
    /* get this channel's connection: */
    conn_serial = chan->tme_z8530_chan_connection;

    /* get this channel's flags: */
    chan_flags = chan->tme_z8530_chan_flags;

    /* if we need to call out new control information: */
    if (callouts & TME_Z8530_CALLOUT_CTRL) {

      /* form the new ctrl: */
      ctrl = 0;
      if (chan->tme_z8530_chan_wrreg[5] & TME_Z8530_WR5_DTR) {
	ctrl |= TME_SERIAL_CTRL_DTR;
      }
      if (chan->tme_z8530_chan_wrreg[5] & TME_Z8530_WR5_RTS) {
	ctrl |= TME_SERIAL_CTRL_RTS;
      }
      if (chan->tme_z8530_chan_wrreg[5] & TME_Z8530_WR5_BREAK) {
	ctrl |= TME_SERIAL_CTRL_BREAK;
      }
      if (!tme_serial_buffer_is_empty(&chan->tme_z8530_chan_buffer_tx)) {
	ctrl |= TME_SERIAL_CTRL_OK_READ;
      }
      
      /* unlock the mutex: */
      tme_mutex_unlock(&z8530->tme_z8530_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_ctrl)
	       (conn_serial,
		ctrl))
	    : TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&z8530->tme_z8530_mutex);
      
      /* if the callout was unsuccessful, remember that at some
	 later time this callout should be attempted again: */
      if (rc != TME_OK) {
	*_later_callouts |= TME_Z8530_CALLOUT_CTRL;
      }
    }
      
    /* if we need to call out new config information: */
    if (callouts & TME_Z8530_CALLOUT_CONFIG) {

      /* form the new config: */
      memset(&config, 0, sizeof(config));

      /* the number of data bits per character.  note that we ignore
	 the transmit character size and only pay attention to the
	 receive character size: */
      switch (chan->tme_z8530_chan_wrreg[3] & TME_Z8530_WR3_RX_CSIZE_MASK) {
      case TME_Z8530_WR3_RX_CSIZE_5:
	config.tme_serial_config_bits_data = 5;
	break;
      case TME_Z8530_WR3_RX_CSIZE_7:
	config.tme_serial_config_bits_data = 7;
	break;
      case TME_Z8530_WR3_RX_CSIZE_6:  
	config.tme_serial_config_bits_data = 6;
	break;
      case TME_Z8530_WR3_RX_CSIZE_8:
	config.tme_serial_config_bits_data = 8;
	break;
      }

      /* the number of stop bits: */
      switch (chan->tme_z8530_chan_wrreg[4] & TME_Z8530_WR4_STOP_MASK) {
      case TME_Z8530_WR4_STOP_SYNC:
      case TME_Z8530_WR4_STOP_1_5BITS:
	abort();
      case TME_Z8530_WR4_STOP_1BIT:
	config.tme_serial_config_bits_stop = 1;
	break;
      case TME_Z8530_WR4_STOP_2BITS:
	config.tme_serial_config_bits_stop = 2;
	break;
      }
	
      /* the parity: */
      config.tme_serial_config_parity = 
	((chan->tme_z8530_chan_wrreg[4] & TME_Z8530_WR4_PARITY_ENABLE)
	 ? (chan->tme_z8530_chan_wrreg[4] & TME_Z8530_WR4_PARITY_EVEN
	    ? TME_SERIAL_PARITY_EVEN
	    : TME_SERIAL_PARITY_ODD)
	 : TME_SERIAL_PARITY_NONE);
      
      /* the baud rate: */
      /* XXX TBD */
      config.tme_serial_config_baud = 9600;
      
      /* flags: */
      /* XXX TBD: */
      config.tme_serial_config_flags = 0;
      
      /* unlock the mutex: */
      tme_mutex_unlock(&z8530->tme_z8530_mutex);
      
      /* do the callout: */
      rc = (conn_serial != NULL
	    ? ((*conn_serial->tme_serial_connection_config)
	       (conn_serial,
		&config))
	    : TME_OK);
      
      /* lock the mutex: */
      tme_mutex_lock(&z8530->tme_z8530_mutex);

      /* if the callout was unsuccessful, remember that at some
	 later time this callout should be attempted again: */
      if (rc != TME_OK) {
	*_later_callouts |= TME_Z8530_CALLOUT_CONFIG;
      }
    }

    /* if this channel's connection is readable: */
    if (callouts & TME_Z8530_CALLOUT_READ) {

      /* if the receive buffer is full, remember that at some later
	 time this callout should be attempted again: */
      if (tme_serial_buffer_is_full(&chan->tme_z8530_chan_buffer_rx)) {
	*_later_callouts |= TME_Z8530_CALLOUT_READ;
      }

      /* otherwise, continue to do the read: */
      else {

	/* get the minimum of the free space in the receive buffer and
	   the size of our stack buffer: */
	buffer_input_size = tme_serial_buffer_space_free(&chan->tme_z8530_chan_buffer_rx);
	buffer_input_size = TME_MIN(buffer_input_size, sizeof(buffer_input));

	/* unlock the mutex: */
	tme_mutex_unlock(&z8530->tme_z8530_mutex);

	/* do the read: */
	rc = (conn_serial != NULL
	      ? ((*conn_serial->tme_serial_connection_read)
		 (conn_serial,
		  buffer_input,
		  buffer_input_size,
		  &data_flags))
	      : 0);
	  
	/* lock the mutex: */
	tme_mutex_lock(&z8530->tme_z8530_mutex);
	
	/* if the read was successful: */
	if (rc > 0) {
	    
	  /* put the the characters into our Rx buffer: */
	  (void) tme_serial_buffer_copyin(&chan->tme_z8530_chan_buffer_rx,
					  buffer_input,
					  rc,
					  data_flags,
					  TME_SERIAL_COPY_NORMAL);

	  /* if the Rx FIFO is empty, refill it: */
	  if (!(chan->tme_z8530_chan_rdreg[0] & TME_Z8530_RR0_RX_AVAIL)) {
	    chan->tme_z8530_chan_callout_flags |= 
	      _tme_z8530_rx_fifo_refill(z8530, chan);
	  }

	  /* mark that we need to loop to callout to read more data: */
	  chan->tme_z8530_chan_callout_flags |= TME_Z8530_CALLOUT_READ;
	}

	/* otherwise, the read failed.  convention dictates that we
	   forget that the connection was readable, which we already
	   have done by clearing the CALLOUT_READ flag: */
      }
    }

    /* if we need to call out a possible change to our interrupt signal: */
    if (callouts & TME_Z8530_CALLOUT_INT) {
    
      /* if we thought we weren't checking the interrupt signal, we
	 are now.  assume that we are not supposed to be asserting an
	 interrupt: */
      if (int_asserted == -1) {
	int_asserted = FALSE;
      }

      /* if any pending interrupt has a higher priority than
	 the highest-priority interrupt currently under service,
	 and the Master Interrupt Enable is set,
	 we are supposed to be asserting an interrupt: */
      if (_tme_z8530_int_pending(z8530)) {
	int_asserted = TRUE;
      }
    }
  }

  /* if we checked the interrupt signal and we need to call out a
     change: */
  if (int_asserted != -1
      && !int_asserted != !z8530->tme_z8530_int_asserted) {

    /* unlock our mutex: */
    tme_mutex_unlock(&z8530->tme_z8530_mutex);
    
    /* get our bus connection: */
    conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
					      z8530->tme_z8530_device.tme_bus_device_connection,
					      &z8530->tme_z8530_device.tme_bus_device_connection_rwlock);
    
    /* call out the bus interrupt signal edge: */
    rc = (*conn_bus->tme_bus_signal)
      (conn_bus,
       TME_BUS_SIGNAL_INT_UNSPEC
       | TME_BUS_SIGNAL_EDGE
       | (int_asserted
	  ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	  : TME_BUS_SIGNAL_LEVEL_NEGATED));
    
    /* lock our mutex: */
    tme_mutex_lock(&z8530->tme_z8530_mutex);
    
    /* if this callout was successful, note the new state of the
       interrupt signal: */
    if (rc == TME_OK) {
      z8530->tme_z8530_int_asserted = int_asserted;
    }
    
    /* otherwise, remember that at some later time this callout
       should be attempted again: */
    else {
      later_callouts_a |= TME_Z8530_CALLOUT_INT;
      later_callouts_b |= TME_Z8530_CALLOUT_INT;
    }
  }

  /* put in any later callouts, and clear that callouts are running: */
  z8530->tme_z8530_chan_a.tme_z8530_chan_callout_flags = later_callouts_a;
  z8530->tme_z8530_chan_b.tme_z8530_chan_callout_flags = later_callouts_b;
}      

/* the z8530 bus cycle handler: */
static int
_tme_z8530_bus_cycle(void *_z8530, struct tme_bus_cycle *cycle_init)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_chan *chan;
  tme_bus_addr32_t address, z8530_address_last;
  int is_csr;
  tme_uint8_t buffer, value;
  struct tme_bus_cycle cycle_resp;
  unsigned int register_pointer;
  int new_callouts;
  unsigned int rr3_chan_shift;
  tme_uint8_t rr3_ip_tx;
  tme_uint8_t rr3_ip_rx;
  tme_uint8_t ius;

  /* recover our data structure: */
  z8530 = (struct tme_z8530 *) _z8530;

  /* the requested cycle must be within range: */
  z8530_address_last = z8530->tme_z8530_device.tme_bus_device_address_last;
  assert(cycle_init->tme_bus_cycle_address <= z8530_address_last);
  assert(cycle_init->tme_bus_cycle_size <= (z8530_address_last - cycle_init->tme_bus_cycle_address) + 1);

  /* see if this is channel A or channel B: */
  address = cycle_init->tme_bus_cycle_address;
  if ((z8530->tme_z8530_address_chan_b
       > z8530->tme_z8530_address_chan_a)
      ? (address >= z8530->tme_z8530_address_chan_b)
      : (address < z8530->tme_z8530_address_chan_a)) {
    chan = &z8530->tme_z8530_chan_b;
    address -= z8530->tme_z8530_address_chan_b;
    rr3_chan_shift = 0;
  }
  else {
    chan = &z8530->tme_z8530_chan_a;
    address -= z8530->tme_z8530_address_chan_a;
    rr3_chan_shift = 3;
  }

  /* see if this is a data or csr access: */
  is_csr = ((z8530->tme_z8530_offset_csr
	     > z8530->tme_z8530_offset_data)
	    ? (address >= z8530->tme_z8530_offset_csr)
	    : (address < z8530->tme_z8530_offset_data));

  /* lock the mutex: */
  tme_mutex_lock(&z8530->tme_z8530_mutex);

  /* if this is a csr access, use the current register pointer and
     reset it to zero, otherwise use 8 as the register pointer
     (corresponding to WR8 and RR8) and don't touch the current
     register pointer: */
  if (is_csr) {
    register_pointer = z8530->tme_z8530_register_pointer;
    z8530->tme_z8530_register_pointer = 0;
  }
  else {
    register_pointer = 8;
  }

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* run the bus cycle: */
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_z8530_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_READ;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(z8530->tme_z8530_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
    value = buffer;
    
    /* log this write: */
    tme_log(TME_Z8530_LOG_HANDLE(z8530), 100000, TME_OK,
	    (TME_Z8530_LOG_HANDLE(z8530),
	     "channel %c 0x%02x -> WR%d",
	     'A' + (chan == &z8530->tme_z8530_chan_b),
	     value,
	     register_pointer));

    /* dispatch on the register pointer: */
    switch (register_pointer) {

      /* WR0: */
    case 0:

      /* get the next register pointer: */
      z8530->tme_z8530_register_pointer = (value & TME_Z8530_WR0_REGISTER_POINTER);

      /* dispatch on the CRC reset code: */
      switch (value & TME_Z8530_WR0_CRC_RESET_MASK) {
      case TME_Z8530_WR0_CRC_RESET_NULL:
      case TME_Z8530_WR0_CRC_RESET_RX:
      case TME_Z8530_WR0_CRC_RESET_TX:
      case TME_Z8530_WR0_CRC_RESET_TX_EOM:
	break;
      }

      /* dispatch on the command code: */
      switch (value & TME_Z8530_WR0_CMD_MASK) {

      case TME_Z8530_WR0_CMD_NULL:
	break;

      case TME_Z8530_WR0_CMD_POINT_HI:
	z8530->tme_z8530_register_pointer |= 0x08;
	break;

      case TME_Z8530_WR0_CMD_RESET_STATUS:

	/* clear any Status interrupt and update RR0: */
	if (z8530->tme_z8530_rr3
	    & (TME_Z8530_RR3_CHAN_B_IP_STATUS
	       << rr3_chan_shift)) {
	  z8530->tme_z8530_rr3
	    &= ~(TME_Z8530_RR3_CHAN_B_IP_STATUS
		 << rr3_chan_shift);
	  new_callouts |= TME_Z8530_CALLOUT_INT;
	}
	new_callouts |= _tme_z8530_rr0_update(z8530, chan);
	break;

      case TME_Z8530_WR0_CMD_RESET_ERROR:

	/* clear any errors and refill the Rx FIFO: */
	chan->tme_z8530_chan_rdreg[1] &=
	  ~(TME_Z8530_RR1_END_OF_FRAME
	    | TME_Z8530_RR1_ERROR_FRAME
	    | TME_Z8530_RR1_ERROR_RX_OVERRUN
	    | TME_Z8530_RR1_ERROR_PARITY);
	new_callouts |= _tme_z8530_rx_fifo_refill(z8530, chan);
	break;

      case TME_Z8530_WR0_CMD_RESET_IUS:

	/* reset the highest IUS bit: */
	for (ius = z8530->tme_z8530_ius;
	     ius & (ius - 1);
	     ius &= (ius - 1));
	z8530->tme_z8530_ius ^= ius;
	
	/* always check for an interrupt callout: */
	new_callouts |= TME_Z8530_CALLOUT_INT;
	break;

      case TME_Z8530_WR0_CMD_RESET_TX:
	
	/* get the IP_TX bit for this channel: */
	rr3_ip_tx = ((chan == &z8530->tme_z8530_chan_a)
		     ? TME_Z8530_RR3_CHAN_A_IP_TX
		     : TME_Z8530_RR3_CHAN_B_IP_TX);

	/* if the IP_TX bit is set for this channel, clear it: */
	if (z8530->tme_z8530_rr3 & rr3_ip_tx) {
	  z8530->tme_z8530_rr3 &= ~rr3_ip_tx;
	  new_callouts |= TME_Z8530_CALLOUT_INT;
	}
	break;

      case TME_Z8530_WR0_CMD_INT_NEXT_RX:

	/* if the receive FIFO is not empty: */
	if (chan->tme_z8530_chan_rdreg[0]
	    & TME_Z8530_RR0_RX_AVAIL) {

	  /* get the IP_RX bit for this channel: */
	  rr3_ip_rx = ((chan == &z8530->tme_z8530_chan_a)
		       ? TME_Z8530_RR3_CHAN_A_IP_RX
		       : TME_Z8530_RR3_CHAN_B_IP_RX);

	  /* call out another receive interrupt: */
	  z8530->tme_z8530_rr3 |= rr3_ip_rx;
	  new_callouts |= TME_Z8530_CALLOUT_INT;
	}
	break;

      case TME_Z8530_WR0_CMD_SEND_ABORT:
	/* XXX TBD: */
	abort();
      }
      break;

      /* WR1: */
    case 1:
      chan->tme_z8530_chan_wrreg[1] = value;

      /* XXX since we fixed the RR3 initialization bug present through
	 revision 1.9, this may now be unnecessary, and might even
	 be incorrect for non-NetBSD systems: */
      /* NB: some serial drivers, like NetBSD's z8530tty.c (at least
	 as recently as rev 1.79), never issue
	 TME_Z8530_WR0_CMD_RESET_TX when they have no more data to
	 send in response to a Tx Empty interrupt.  instead, these
	 drivers simply clear TME_Z8530_WR1_TX_INT_ENABLE and expect
	 that to disable all further Tx Empty interrupts.
	 
	 I can't find anything in the SCC manual that says that the
	 transmit Interrupt Pending bit works this way.  is the SCC
	 manual wrong?  in any case, if TME_Z8530_WR1_TX_INT_ENABLE is
	 clear, clear the IP_TX bit for this channel: */
      if (!(value & TME_Z8530_WR1_TX_INT_ENABLE)) {

	/* get the IP_TX bit for this channel: */
	rr3_ip_tx = ((chan == &z8530->tme_z8530_chan_a)
		     ? TME_Z8530_RR3_CHAN_A_IP_TX
		     : TME_Z8530_RR3_CHAN_B_IP_TX);

	/* if the IP_TX bit is set for this channel, clear it: */
	if (z8530->tme_z8530_rr3 & rr3_ip_tx) {
	  z8530->tme_z8530_rr3 &= ~rr3_ip_tx;
	  new_callouts |= TME_Z8530_CALLOUT_INT;
	}
      }

      break;

      /* WR2: */
    case 2:
      z8530->tme_z8530_chan_a.tme_z8530_chan_wrreg[2] = value;
      break;

      /* WR3: */
      /* WR4: */
      /* WR5: */
    case 5:
      new_callouts |= TME_Z8530_CALLOUT_CTRL;
    case 3:
    case 4:
      chan->tme_z8530_chan_wrreg[register_pointer] = value;
      new_callouts |= TME_Z8530_CALLOUT_CONFIG;
      break;

      /* WR8: */
    case 8:

      /* if the transmit buffer was previously empty, it won't be now,
	 so clear any transmitter interrupt and call out the new
	 control: */
      if (tme_serial_buffer_is_empty(&chan->tme_z8530_chan_buffer_tx)) {
	if (z8530->tme_z8530_rr3
	    & (TME_Z8530_RR3_CHAN_B_IP_TX
	       << rr3_chan_shift)) {
	  z8530->tme_z8530_rr3
	    &= ~(TME_Z8530_RR3_CHAN_B_IP_TX
		 << rr3_chan_shift);
	  new_callouts |= TME_Z8530_CALLOUT_INT;
	}
	new_callouts |= TME_Z8530_CALLOUT_CTRL;
      }

      /* copy in the new character: */
      tme_serial_buffer_copyin(&chan->tme_z8530_chan_buffer_tx,
			       &buffer, 1,
			       TME_SERIAL_DATA_NORMAL,
			       TME_SERIAL_COPY_NORMAL);

      /* update RR0: */
      chan->tme_z8530_chan_rdreg[0] &= ~TME_Z8530_RR0_TX_EMPTY;

      /* update RR1: */
      chan->tme_z8530_chan_rdreg[1] &= ~TME_Z8530_RR1_ALL_SENT;
      break;

      /* WR9: */
    case 9:

      /* dispatch on the reset command: */
      switch (value & TME_Z8530_WR9_RESET_MASK) {
      case TME_Z8530_WR9_RESET_NULL:
	break;
      case TME_Z8530_WR9_RESET_CHAN_B:
	_tme_z8530_channel_reset(z8530, &z8530->tme_z8530_chan_b, FALSE);
	break;
      case TME_Z8530_WR9_RESET_CHAN_A:
	_tme_z8530_channel_reset(z8530, &z8530->tme_z8530_chan_a, FALSE);
	break;
      case TME_Z8530_WR9_RESET_CHIP:
	_tme_z8530_channel_reset(z8530, &z8530->tme_z8530_chan_a, TRUE);
	_tme_z8530_channel_reset(z8530, &z8530->tme_z8530_chan_b, TRUE);
	break;
      }

      /* we can't handle these bits yet: */
      if (value & (TME_Z8530_WR9_SOFT_INTACK
		   | TME_Z8530_WR9_DLC)) {
	abort();
      }

      /* save the register value: */
      z8530->tme_z8530_wr9 = value;

      /* this may have enabled interrupts: */
      new_callouts |= TME_Z8530_CALLOUT_INT;
      break;

      /* WR11: */
      /* WR12: */
      /* WR13: */
      /* WR14: */
    case 11:
    case 12:
    case 13:
    case 14:
      chan->tme_z8530_chan_wrreg[register_pointer] = value;
      new_callouts |= TME_Z8530_CALLOUT_CONFIG;
      break;

      /* WR6: */
      /* WR7: */
      /* WR10: */
      /* WR15: */
    case 6:
    case 7:
    case 10:
    case 15:
      chan->tme_z8530_chan_wrreg[register_pointer] = value;
      break;

      /* everything else: */
    default:
      abort();
    }
  }

  /* otherwise, this is a read: */
  else {
    assert(cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_READ);

    /* dispatch on the register pointer: */
    switch (register_pointer) {

      /* RR0: */
      /* RR4 (an image of RR0 on the 8530): */
    case 0:
    case 4:
      value = chan->tme_z8530_chan_rdreg[0];
      break;

      /* RR1: */
      /* RR5 (an image of RR1 on the 8530): */
    case 1:
    case 5:
      value = chan->tme_z8530_chan_rdreg[1];
      break;
      
      /* RR2: */
    case 2:
      if (chan == &z8530->tme_z8530_chan_a) {
	value = chan->tme_z8530_chan_wrreg[2];
      }
      else {
	value = chan->tme_z8530_chan_rdreg[2];
      }
      break;

      /* RR3: */
    case 3:
      value = ((chan == &z8530->tme_z8530_chan_a)
	       ? chan->tme_z8530_chan_rdreg[register_pointer]
	       : 0);
      break;

      /* RR8: */
    case 8:

      /* return whatever's in the Rx FIFO right now and
	 refill it: */
      value = chan->tme_z8530_chan_rdreg[8];
      new_callouts |= _tme_z8530_rx_fifo_refill(z8530, chan);
      break;

      /* RR10: */
      /* RR14 (an image of RR10 on the 8530): */
    case 10:
    case 14:
      value = 0;
      break;

      /* RR12: */
    case 12:
      /* these return the corresponding WR: */
      value = chan->tme_z8530_chan_wrreg[register_pointer];
      break;

      /* RR9 (an image of RR13 on the 8530): */
      /* RR13: */
    case 9:
    case 13:
      value = chan->tme_z8530_chan_wrreg[13];
      break;

      /* RR11 (an image of RR15 on the 8530): */
      /* RR15: */
    case 11:
    case 15:
      value = chan->tme_z8530_chan_wrreg[15];
      break;

      /* everything else: */
    default:
      abort();
    }

#ifndef TME_NO_LOG
    /* log this read: */
    if (chan->tme_z8530_chan_last_read_reg != register_pointer
	|| chan->tme_z8530_chan_last_read_value != value) {
      chan->tme_z8530_chan_last_read_reg = register_pointer;
      chan->tme_z8530_chan_last_read_value = value;
      tme_log(TME_Z8530_LOG_HANDLE(z8530), 100000, TME_OK,
	      (TME_Z8530_LOG_HANDLE(z8530),
	       "channel %c RR%d -> 0x%02x",
	       'A' + (chan == &z8530->tme_z8530_chan_b),
	       register_pointer,
	       value));
    }
#endif /* !TME_NO_LOG */

    /* run the bus cycle: */
    buffer = value;
    cycle_resp.tme_bus_cycle_buffer = &buffer;
    cycle_resp.tme_bus_cycle_lane_routing = tme_z8530_router;
    cycle_resp.tme_bus_cycle_address = 0;
    cycle_resp.tme_bus_cycle_buffer_increment = 1;
    cycle_resp.tme_bus_cycle_type = TME_BUS_CYCLE_WRITE;
    cycle_resp.tme_bus_cycle_size = sizeof(buffer);
    cycle_resp.tme_bus_cycle_port = 
      TME_BUS_CYCLE_PORT(z8530->tme_z8530_port_least_lane,
			 TME_BUS8_LOG2);
    tme_bus_cycle_xfer(cycle_init, &cycle_resp);
  }
    
  /* make any needed callouts: */
  _tme_z8530_callout(z8530, chan, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&z8530->tme_z8530_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this is called when the serial configuration changes: */
static int
_tme_z8530_config(struct tme_serial_connection *conn_serial, 
		  struct tme_serial_config *config)
{
  /* do nothing: */
  return (TME_OK);
}

/* this is called when control lines change: */
static int
_tme_z8530_ctrl(struct tme_serial_connection *conn_serial, 
		unsigned int ctrl)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_chan *chan;
  tme_uint8_t rr0_status_new;
  int new_callouts;

  /* recover our data structures: */
  z8530 = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;
  chan = ((struct tme_z8530_connection *) conn_serial)->tme_z8530_connection_chan;

  /* lock the mutex: */
  tme_mutex_lock(&z8530->tme_z8530_mutex);

  /* assume that we won't need to make any callouts: */
  new_callouts = 0;

  /* make new raw RR0 status bits, and update the mask of bits that
     have changed from the RR0 at the time of the last External/Status
     interrupt, with the exception of TME_Z8530_RR0_BREAK - break
     transitions are not aggregated so they are not missed while the
     RR0 status bits are latched: */
  rr0_status_new = 0;
  if (ctrl & TME_SERIAL_CTRL_DCD) {
    rr0_status_new |= TME_Z8530_RR0_DCD;
  }
  if (ctrl & TME_SERIAL_CTRL_CTS) {
    rr0_status_new |= TME_Z8530_RR0_CTS;
  }
  if (ctrl & TME_SERIAL_CTRL_BREAK) {
    rr0_status_new |= TME_Z8530_RR0_BREAK;
  }
  chan->tme_z8530_chan_rr0_status_raw = rr0_status_new;
  chan->tme_z8530_chan_rr0_status_diff_mask = 
    (((chan->tme_z8530_chan_rdreg[0]
       ^ rr0_status_new)
      & (TME_Z8530_RR0_DCD
	 | TME_Z8530_RR0_CTS
	 | TME_Z8530_RR0_BREAK))
     | (chan->tme_z8530_chan_rr0_status_diff_mask
	& TME_Z8530_RR0_BREAK));

  /* try to update RR0.  (if an External/Status interrupt is pending, 
     RR0 is latched): */
  new_callouts |= _tme_z8530_rr0_update(z8530, chan);

  /* if this channel is readable, call out to read data: */
  if (ctrl & TME_SERIAL_CTRL_OK_READ) {
    new_callouts |= TME_Z8530_CALLOUT_READ;
  }

  /* if needed, make callouts: */
  _tme_z8530_callout(z8530, chan, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&z8530->tme_z8530_mutex);

  return (TME_OK);
}

/* this is called to read serial data (from the z8530 perspective, to transmit it): */
static int
_tme_z8530_read(struct tme_serial_connection *conn_serial, 
		tme_uint8_t *data, unsigned int count,
		tme_serial_data_flags_t *_data_flags)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_chan *chan;
  tme_uint8_t rr3_ip_tx;
  int new_callouts;
  int rc;

  /* assume that we won't need to make any callouts: */
  new_callouts = 0;

  /* recover our data structures: */
  z8530 = conn_serial->tme_serial_connection.tme_connection_element->tme_element_private;
  chan = ((struct tme_z8530_connection *) conn_serial)->tme_z8530_connection_chan;

  /* lock our mutex: */
  tme_mutex_lock(&z8530->tme_z8530_mutex);

  /* copy out data from the Tx FIFO: */
  rc = tme_serial_buffer_copyout(&chan->tme_z8530_chan_buffer_tx,
				 data, count,
				 _data_flags,
				 TME_SERIAL_COPY_NORMAL);

  /* if the Tx buffer is now empty: */
  if (tme_serial_buffer_is_empty(&chan->tme_z8530_chan_buffer_tx)) {

    /* update RR0: */
    chan->tme_z8530_chan_rdreg[0] |= TME_Z8530_RR0_TX_EMPTY;

    /* update RR1: */
    chan->tme_z8530_chan_rdreg[1] |= TME_Z8530_RR1_ALL_SENT;

    /* if Tx interrupts are enabled: */
    if (chan->tme_z8530_chan_wrreg[1] & TME_Z8530_WR1_TX_INT_ENABLE) {

      /* get the IP_TX bit for this channel: */
      rr3_ip_tx = ((chan == &z8530->tme_z8530_chan_a)
		   ? TME_Z8530_RR3_CHAN_A_IP_TX
		   : TME_Z8530_RR3_CHAN_B_IP_TX);

      /* if the Tx buffer is newly empty, flag this channel for a Tx
	 interrupt: */
      if (!(z8530->tme_z8530_rr3 & rr3_ip_tx)) {
	z8530->tme_z8530_rr3 |= rr3_ip_tx;
	new_callouts |= TME_Z8530_CALLOUT_INT;
      }
    }

    /* call out a new control to clear OK_READ: */
    new_callouts |= TME_Z8530_CALLOUT_CTRL;
  }

  /* make any needed callouts: */
  _tme_z8530_callout(z8530, chan, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&z8530->tme_z8530_mutex);

  /* done: */
  return (rc);
}

/* the z8530 TLB filler: */
static int
_tme_z8530_tlb_fill(void *_z8530, struct tme_bus_tlb *tlb, 
		     tme_bus_addr_t address, unsigned int cycles)
{
  struct tme_z8530 *z8530;
  tme_bus_addr32_t z8530_address_last;

  /* recover our data structure: */
  z8530 = (struct tme_z8530 *) _z8530;

  /* the address must be within range: */
  z8530_address_last = z8530->tme_z8530_device.tme_bus_device_address_last;
  assert(address <= z8530_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = z8530_address_last;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = z8530;
  tlb->tme_bus_tlb_cycle = _tme_z8530_bus_cycle;

  return (TME_OK);
}

/* this handles a hardware interrupt acknowledge: */
static int
_tme_z8530_hard_intack(void *_z8530, unsigned int ipl, int *_vector)
{
  struct tme_z8530 *z8530;
  int rc;

  /* recover our data structure: */
  z8530 = (struct tme_z8530 *) _z8530;

  /* lock our mutex: */
  tme_mutex_lock(&z8530->tme_z8530_mutex);

  /* acknowledge the interrupt: */
  rc = _tme_z8530_intack(z8530, _vector);

  /* we always call out an interrupt check: */
  _tme_z8530_callout(z8530, &z8530->tme_z8530_chan_a, TME_Z8530_CALLOUT_INT);

  /* unlock our mutex: */
  tme_mutex_unlock(&z8530->tme_z8530_mutex);

  /* done: */
  return (rc);
}

/* this scores a serial connection: */
static int
_tme_z8530_connection_score(struct tme_connection *conn, unsigned int *_score)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_connection *conn_z8530;
  struct tme_serial_connection *conn_serial_other;

  /* recover our data structures: */
  z8530 = conn->tme_connection_element->tme_element_private;
  conn_z8530 = (struct tme_z8530_connection *) conn;
  conn_serial_other = (struct tme_serial_connection *) conn->tme_connection_other;

  /* both sides must be serial connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SERIAL);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SERIAL);

  /* this channel must be free: */
  assert(conn_z8530->tme_z8530_connection_chan->tme_z8530_chan_connection == NULL);

  /* we're lax on checking the members of the serial connection, 
     and just assume this connection is fine: */
  *_score = 1;
  return (TME_OK);
}

/* this makes a new serial connection: */
static int
_tme_z8530_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_connection *conn_z8530;
  struct tme_serial_connection *conn_serial_other;

  /* recover our data structures: */
  z8530 = conn->tme_connection_element->tme_element_private;
  conn_z8530 = (struct tme_z8530_connection *) conn;
  conn_serial_other = (struct tme_serial_connection *) conn->tme_connection_other;

  /* both sides must be serial connections: */
  assert(conn->tme_connection_type == TME_CONNECTION_SERIAL);
  assert(conn->tme_connection_other->tme_connection_type == TME_CONNECTION_SERIAL);

  /* we're always set up to answer calls across the connection, so we
     only have to do work when the connection has gone full, namely
     taking the other side of the connection: */
  if (state == TME_CONNECTION_FULL) {

    /* save our connection: */
    conn_z8530->tme_z8530_connection_chan->tme_z8530_chan_connection = conn_serial_other;
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_z8530_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a z8530: */
static int
_tme_z8530_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_z8530 *z8530;
  struct tme_z8530_chan *chan;
  struct tme_z8530_connection *conn_z8530;
  struct tme_serial_connection *conn_serial;
  struct tme_connection *conn;

  /* recover our data structure: */
  z8530 = (struct tme_z8530 *) element->tme_element_private;

  /* if the args are "channel A" or "channel B", they're for us: */
  if (TME_ARG_IS(args[1], "channel")) {
    
    /* if this is for channel A, but it already has a connection, 
       we can't offer another connection.  similarly for channel B: */
    if (TME_ARG_IS(args[2], "A")) {
      chan = &z8530->tme_z8530_chan_a;
    }
    else if (TME_ARG_IS(args[2], "B")) {
      chan = &z8530->tme_z8530_chan_b;
    }
    else {
      tme_output_append_error(_output, 
			      "%s channel '%s', %s %s channel { A | B }",
			      _("bad"),
			      args[2],
			      _("usage:"),
			      args[0]);
      return (EINVAL);
    }
    if (chan->tme_z8530_chan_connection != NULL) {
      tme_output_append_error(_output,
			      "%s %s",
			      _("channel"),
			      args[2]);
      return (EISCONN);
    }

    /* allocate the new serial connection: */
    conn_z8530 = tme_new0(struct tme_z8530_connection, 1);
    conn_serial = &conn_z8530->tme_z8530_connection;
    conn = &conn_serial->tme_serial_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_SERIAL;
    conn->tme_connection_score = _tme_z8530_connection_score;
    conn->tme_connection_make = _tme_z8530_connection_make;
    conn->tme_connection_break = _tme_z8530_connection_break;

    /* fill in the serial connection: */
    conn_serial->tme_serial_connection_config = _tme_z8530_config;
    conn_serial->tme_serial_connection_ctrl = _tme_z8530_ctrl;
    conn_serial->tme_serial_connection_read = _tme_z8530_read;

    /* fill in the z8530 connection: */
    conn_z8530->tme_z8530_connection_chan = chan;

    /* return the connection side possibility: */
    *_conns = conn;
    return (TME_OK);
  }
  
  /* otherwise, make the generic bus device connection side: */
  return (tme_bus_device_connections_new(element, args, _conns, _output));
}

/* the new z8530 function: */
TME_ELEMENT_NEW_DECL(tme_ic_z8530) {
  const struct tme_z8530_socket *socket;
  struct tme_z8530 *z8530;
  struct tme_z8530_socket socket_real;
  tme_bus_addr_t address_mask;

  /* dispatch on our socket version: */
  socket = (const struct tme_z8530_socket *) extra;
  if (socket == NULL) {
    tme_output_append_error(_output, _("need an ic socket"));
    return (ENXIO);
  }
  switch (socket->tme_z8530_socket_version) {
  case TME_Z8530_SOCKET_0:
    socket_real = *socket;
    break;
  default: 
    tme_output_append_error(_output, _("socket type"));
    return (EOPNOTSUPP);
  }
    
  /* we take no arguments: */
  if (args[1] != NULL) {
    tme_output_append_error(_output,
			    "%s %s, %s %s",
			    args[1],
			    _("unexpected"),
			    _("usage:"),
			    args[0]);
    return (EINVAL);
  }

  /* start the z8530 structure: */
  z8530 = tme_new0(struct tme_z8530, 1);
  z8530->tme_z8530_socket = socket_real;
  tme_mutex_init(&z8530->tme_z8530_mutex);
  _tme_z8530_channel_init(z8530, &z8530->tme_z8530_chan_a);
  _tme_z8530_channel_init(z8530, &z8530->tme_z8530_chan_b);

  /* figure our address mask, up to the nearest power of two: */
  address_mask = TME_MAX(z8530->tme_z8530_address_chan_a,
			 z8530->tme_z8530_address_chan_b);
  address_mask += TME_MAX(z8530->tme_z8530_offset_csr,
			  z8530->tme_z8530_offset_data);
  if (address_mask & (address_mask - 1)) {
    for (; address_mask & (address_mask - 1); address_mask &= (address_mask - 1));
    address_mask <<= 1;
  }
  address_mask -= 1;

  /* initialize our simple bus device descriptor: */
  z8530->tme_z8530_device.tme_bus_device_element = element;
  z8530->tme_z8530_device.tme_bus_device_tlb_fill = _tme_z8530_tlb_fill;
  z8530->tme_z8530_device.tme_bus_device_intack = _tme_z8530_hard_intack;
  z8530->tme_z8530_device.tme_bus_device_address_last = address_mask;

  /* fill the element: */
  element->tme_element_private = z8530;
  element->tme_element_connections_new = _tme_z8530_connections_new;

  return (TME_OK);
}
