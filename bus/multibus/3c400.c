/* $Id: 3c400.c,v 1.11 2010/06/05 13:49:56 fredette Exp $ */

/* bus/multibus/3c400.c - implementation of the Multibus 3c400 emulation: */

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
_TME_RCSID("$Id: 3c400.c,v 1.11 2010/06/05 13:49:56 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/generic/ethernet.h>

/* macros: */

/* register offsets and sizes: */
#define TME_3C400_REG_CSR	(0)
#define TME_3C400_SIZ_CSR	(sizeof(tme_uint16_t))
#define TME_3C400_REG_BACKOFF	(2)
#define TME_3C400_SIZ_BACKOFF	(sizeof(tme_uint16_t))
#define TME_3C400_REG_AROM	(1024)
#define TME_3C400_SIZ_AROM	(TME_ETHERNET_ADDR_SIZE)
#define TME_3C400_REG_ARAM	(1536)
#define TME_3C400_SIZ_ARAM	(TME_ETHERNET_ADDR_SIZE)
#define TME_3C400_REG_TBUF	(2048)
#define TME_3C400_SIZ_BUF	(2048)
#define TME_3C400_REG_ABUF	(TME_3C400_REG_TBUF + TME_3C400_SIZ_BUF)
#define TME_3C400_REG_BBUF	(TME_3C400_REG_ABUF + TME_3C400_SIZ_BUF)
#define TME_3C400_SIZ_CARD	(TME_3C400_REG_BBUF + TME_3C400_SIZ_BUF)

/* the bits in the Control/Status Register.  software can set and
   clear bits covered by TME_3C400_CSR_INTPA.  software can set, but
   not clear, bits not covered by TME_3C400_CSR_INTPA: */
#define	TME_3C400_CSR_BBSW	(0x8000)	/* B buffer empty (belongs to card) */
#define	TME_3C400_CSR_ABSW	(0x4000)	/* A buffer empty (belongs to card) */
#define	TME_3C400_CSR_TBSW	(0x2000)	/* T buffer full (belongs to card) */
#define	TME_3C400_CSR_JAM	(0x1000)	/* Ethernet jammed (collision) */
#define	TME_3C400_CSR_AMSW	(0x0800)	/* address RAM belongs to ether */
#define	TME_3C400_CSR_RBBA	(0x0400)	/* B buffer received before A */
#define	TME_3C400_CSR_RESET	(0x0100)	/* reset the card */
#define	TME_3C400_CSR_INTPA	(0x00ff)	/* mask for interrupt and PA fields */
#define	 TME_3C400_CSR_BINT	(0x0080)	/* B buffer interrupt enable */
#define	 TME_3C400_CSR_AINT	(0x0040)	/* A buffer interrupt enable */
#define	 TME_3C400_CSR_TINT	(0x0020)	/* T buffer interrupt enable */
#define	 TME_3C400_CSR_JINT	(0x0010)	/* jam interrupt enable */
#define	 TME_3C400_CSR_PAMASK	(0x000f)	/* PA field */
#define	  TME_3C400_CSR_PA	(0x0007)	/* receive mine+broadcast-errors */
#define   TME_3C400_CSR_PROMISC	(0x0001)	/* receive all-errors */

/* the first 16 bits of all buffers are a status word: */
#define TME_3C400_SIZ_BUF_STATUS	(sizeof(tme_uint16_t))

/* the bits of a receive buffer status word: */
/* Frame Check Sequence (CRC) error */
#define TME_3C400_RBUF_FCSERR		(0x8000)
/* this packet was broadcast: */
#define TME_3C400_RBUF_BROADCAST	(0x4000)
/* this packet had a "range error": */
#define TME_3C400_RBUF_RGERR		(0x2000)
/* this packet matched our address: */
#define TME_3C400_RBUF_ADDRMATCH	(0x1000)
/* this packet had a framing error: */
#define TME_3C400_RBUF_FRERR		(0x0800)
/* the first byte after the frame in the buffer: */
#define TME_3C400_RBUF_DOFF_MASK	(0x07ff)

/* these get and put the CSR: */
#define TME_3C400_CSR_GET(_3c400)	\
  ((((tme_uint16_t) (_3c400)->tme_3c400_card[TME_3C400_REG_CSR + 0]) << 8)	\
   + (_3c400)->tme_3c400_card[TME_3C400_REG_CSR + 1])
#define TME_3C400_CSR_PUT(_3c400, csr)	\
  do {										\
    (_3c400)->tme_3c400_card[TME_3C400_REG_CSR + 0] = (csr) >> 8;		\
    (_3c400)->tme_3c400_card[TME_3C400_REG_CSR + 1] = (tme_uint8_t) (csr);	\
  } while (/* CONSTCOND */ 0)

/* the callout flags: */
#define TME_3C400_CALLOUT_CHECK		(0)
#define TME_3C400_CALLOUT_RUNNING	TME_BIT(0)
#define TME_3C400_CALLOUTS_MASK		(-2)
#define  TME_3C400_CALLOUT_CTRL		TME_BIT(1)
#define  TME_3C400_CALLOUT_CONFIG	TME_BIT(2)
#define  TME_3C400_CALLOUT_READ		TME_BIT(3)
#define	 TME_3C400_CALLOUT_INT		TME_BIT(4)

/* structures: */

/* the card: */
struct tme_3c400 {

  /* our simple bus device header: */
  struct tme_bus_device tme_3c400_device;
#define tme_3c400_element tme_3c400_device.tme_bus_device_element

  /* the mutex protecting the card: */
  tme_mutex_t tme_3c400_mutex;

  /* the rwlock protecting the card: */
  tme_rwlock_t tme_3c400_rwlock;

  /* the Ethernet connection: */
  struct tme_ethernet_connection *tme_3c400_eth_connection;

  /* the callout flags: */
  int tme_3c400_callout_flags;

  /* if our interrupt line is currently asserted: */
  int tme_3c400_int_asserted;

  /* it's easiest to just model the card as a chunk of memory: */
  tme_uint8_t tme_3c400_card[TME_3C400_SIZ_CARD];

#ifndef TME_NO_LOG
  tme_uint16_t tme_3c400_last_log_csr;
#endif /* !TME_NO_LOG */
};

/* this resets the card: */
static void
_tme_3c400_reset(struct tme_3c400 *_3c400)
{
  tme_uint16_t csr;

  /* the reset CSR value: */
  csr = 0;

  /* set the CSR: */
  TME_3C400_CSR_PUT(_3c400, csr);

  /* clear all pending callouts: */
  _3c400->tme_3c400_callout_flags &= TME_3C400_CALLOUTS_MASK;

  /* if the interrupt line is currently asserted, negate it: */
  if (_3c400->tme_3c400_int_asserted) {
    _3c400->tme_3c400_callout_flags |= TME_3C400_CALLOUT_INT;
  }
}

/* the _3c400 callout function.  it must be called with the mutex locked: */
static void
_tme_3c400_callout(struct tme_3c400 *_3c400, int new_callouts)
{
  struct tme_ethernet_connection *conn_eth;
  struct tme_bus_connection *conn_bus;
  tme_uint16_t csr, csr_rbba, recv_buffer;
  int callouts, later_callouts;
  unsigned int ctrl;
  struct tme_ethernet_config config;
  int rc;
  const tme_uint8_t *addrs[2];
  tme_ethernet_fid_t frame_id;
  tme_uint8_t *rbuf;
  tme_uint16_t status;
  struct tme_ethernet_frame_chunk *frame_chunk, frame_chunk_buffer;
  int int_asserted;
  
  /* add in any new callouts: */
  _3c400->tme_3c400_callout_flags |= new_callouts;

  /* if this function is already running in another thread, simply
     return now.  the other thread will do our work: */
  if (_3c400->tme_3c400_callout_flags & TME_3C400_CALLOUT_RUNNING) {
    return;
  }

  /* callouts are now running: */
  _3c400->tme_3c400_callout_flags |= TME_3C400_CALLOUT_RUNNING;

  /* assume that we won't need any later callouts: */
  later_callouts = 0;

  /* loop while callouts are needed: */
  for (; (callouts = _3c400->tme_3c400_callout_flags) & TME_3C400_CALLOUTS_MASK; ) {

    /* clear the needed callouts: */
    _3c400->tme_3c400_callout_flags = callouts & ~TME_3C400_CALLOUTS_MASK;
    callouts &= TME_3C400_CALLOUTS_MASK;

    /* get this card's connection: */
    conn_eth = _3c400->tme_3c400_eth_connection;

    /* if we need to call out new control information: */
    if (callouts & TME_3C400_CALLOUT_CTRL) {

      /* get the current CSR value: */
      csr = TME_3C400_CSR_GET(_3c400);

      /* form the new ctrl: */
      ctrl = 0;
      if (csr & TME_3C400_CSR_TBSW) {
	ctrl |= TME_ETHERNET_CTRL_OK_READ;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&_3c400->tme_3c400_mutex);
      
      /* do the callout: */
      rc = (conn_eth != NULL
	    ? ((*conn_eth->tme_ethernet_connection_ctrl)
	       (conn_eth,
		ctrl))
	    : TME_OK);
	
      /* lock the mutex: */
      tme_mutex_lock(&_3c400->tme_3c400_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_3C400_CALLOUT_CTRL;
      }
    }
      
    /* if we need to call out new config information: */
    if (callouts & TME_3C400_CALLOUT_CONFIG) {
      
      /* get the current CSR value: */
      csr = TME_3C400_CSR_GET(_3c400);

      /* form the new config: */
      memset(&config, 0, sizeof(config));
      
      /* our Ethernet address: */
      config.tme_ethernet_config_addr_count = 0;
      addrs[config.tme_ethernet_config_addr_count++]
	= tme_ethernet_addr_broadcast;
      if (csr & TME_3C400_CSR_AMSW) {
	addrs[config.tme_ethernet_config_addr_count++]
	  = &_3c400->tme_3c400_card[TME_3C400_REG_ARAM];
      }
      config.tme_ethernet_config_addrs = addrs;
      
      /* our config flags: */
      config.tme_ethernet_config_flags = TME_ETHERNET_CONFIG_NORMAL;
      switch (csr & TME_3C400_CSR_PAMASK) {
      case 0:
	config.tme_ethernet_config_addr_count = 0;
	break;
      case TME_3C400_CSR_PA:
	break;
      case TME_3C400_CSR_PROMISC:
	config.tme_ethernet_config_flags |= TME_ETHERNET_CONFIG_PROMISC;
	break;
      default: abort();
      }
      
      /* unlock the mutex: */
      tme_mutex_unlock(&_3c400->tme_3c400_mutex);
      
      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_config)
	       (conn_eth,
		&config)));
      
      /* lock the mutex: */
      tme_mutex_lock(&_3c400->tme_3c400_mutex);
      
      /* if the callout was unsuccessful, remember that at some later
	 time this callout should be attempted again: */
      if (rc != TME_OK) {
	later_callouts |= TME_3C400_CALLOUT_CONFIG;
      }
    }

    /* if the Ethernet is readable: */
    if (callouts & TME_3C400_CALLOUT_READ) {

      /* get the current CSR value: */
      csr = TME_3C400_CSR_GET(_3c400);

      /* try to find an empty buffer: */
      switch (csr & (TME_3C400_CSR_BBSW | TME_3C400_CSR_ABSW)) {
      default:
	/* both buffers are full: */
	recv_buffer = 0;
	csr_rbba = (csr & TME_3C400_CSR_RBBA);
	break;
      case TME_3C400_CSR_BBSW:
	/* the A buffer is full but the B buffer is empty: */
	recv_buffer = TME_3C400_CSR_BBSW;
	csr_rbba = 0;
	break;
      case TME_3C400_CSR_ABSW:
	/* the B buffer is full but the A buffer is empty: */
	recv_buffer = TME_3C400_CSR_ABSW;
	csr_rbba = TME_3C400_CSR_RBBA;
	break;
      case TME_3C400_CSR_ABSW | TME_3C400_CSR_BBSW:
	/* both buffers are empty: */
	recv_buffer = TME_3C400_CSR_ABSW;
	csr_rbba = 0;
	break;
      }

      /* unlock the mutex: */
      tme_mutex_unlock(&_3c400->tme_3c400_mutex);
      
      /* make a frame chunk to receive this frame.  remember that the
	 first two bytes of our card's buffers are a status word: */
      if (recv_buffer == 0) {
	rbuf = NULL;
	frame_chunk = NULL;
      }
      else {
	rbuf = 
	  &_3c400->tme_3c400_card[(recv_buffer == TME_3C400_CSR_ABSW
				   ? TME_3C400_REG_ABUF
				   : TME_3C400_REG_BBUF)];
	frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
	frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
	  = rbuf + TME_3C400_SIZ_BUF_STATUS;
	frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
	  = TME_3C400_SIZ_BUF;
	frame_chunk = &frame_chunk_buffer;
      }

      /* do the callout: */
      rc = (conn_eth == NULL
	    ? TME_OK
	    : ((*conn_eth->tme_ethernet_connection_read)
	       (conn_eth,
		&frame_id,
		frame_chunk,
		TME_ETHERNET_READ_NEXT)));
      
      /* lock the mutex: */
      tme_mutex_lock(&_3c400->tme_3c400_mutex);
      
      /* get the current CSR value: */
      csr = TME_3C400_CSR_GET(_3c400);

      /* if the read was successful: */
      if (rc > 0) {

	/* if this frame was received into a buffer: */
	if (recv_buffer != 0) {

	  /* form the status word for this buffer: */
	  status = 0;

	  /* the first thing in the frame is the destination address,
	     which we check against our address and the broadcast
	     address: */
	  if (memcmp(rbuf + TME_3C400_SIZ_BUF_STATUS,
		     &_3c400->tme_3c400_card[TME_3C400_REG_ARAM],
		     TME_ETHERNET_ADDR_SIZE) == 0) {
	    status |= TME_3C400_RBUF_ADDRMATCH;
	  }
	  else if (memcmp(rbuf + TME_3C400_SIZ_BUF_STATUS,
			  tme_ethernet_addr_broadcast,
		     TME_ETHERNET_ADDR_SIZE) == 0) {
	    status |= TME_3C400_RBUF_BROADCAST;
	  }
			  
	  /* put in the offset of the first free byte in the status.
	     make sure we present a packet that is at least as big
	     as the smallest Ethernet frame: */
	  rc = TME_MAX(rc, TME_ETHERNET_FRAME_MIN - TME_ETHERNET_CRC_SIZE);
	  status |= TME_3C400_SIZ_BUF_STATUS + rc;

	  /* put in the status: */
	  *((tme_uint16_t *) rbuf) = tme_htobe_u16(status);

	  /* update the CSR to reflect the new packet: */
	  csr = (csr & ~(recv_buffer | TME_3C400_CSR_RBBA)) | csr_rbba;
	  TME_3C400_CSR_PUT(_3c400, csr);

	  /* if interrupts are enabled on this buffer, mark that we need
	     to callout an interrupt: */
	  if (csr & (recv_buffer >> 8)) {
	    _3c400->tme_3c400_callout_flags |= TME_3C400_CALLOUT_INT;
	  }
	}

	/* mark that we need to loop to callout to read more frames: */
	_3c400->tme_3c400_callout_flags |= TME_3C400_CALLOUT_READ;
      }

      /* otherwise, the read failed.  convention dictates that we
	 forget that the connection was readable, which we already
	 have done by clearing the CALLOUT_READ flag: */
    }

    /* if we need to call out a possible change to our interrupt
       signal: */
    if (callouts & TME_3C400_CALLOUT_INT) {

      /* get the current CSR value: */
      csr = TME_3C400_CSR_GET(_3c400);

      /* see if the interrupt signal should be asserted or negated: */
      int_asserted = ((~(csr & (TME_3C400_CSR_BBSW
				| TME_3C400_CSR_ABSW
				| TME_3C400_CSR_TBSW)))
		      & ((csr & (TME_3C400_CSR_BINT
				 | TME_3C400_CSR_AINT
				 | TME_3C400_CSR_TINT)) << 8));

      /* if the interrupt signal doesn't already have the right state: */
      if (!int_asserted != !_3c400->tme_3c400_int_asserted) {

	/* unlock our mutex: */
	tme_mutex_unlock(&_3c400->tme_3c400_mutex);
	
	/* get our bus connection: */
	conn_bus = tme_memory_atomic_pointer_read(struct tme_bus_connection *,
						  _3c400->tme_3c400_device.tme_bus_device_connection,
						  &_3c400->tme_3c400_device.tme_bus_device_connection_rwlock);
	
	/* call out the bus interrupt signal edge: */
	rc = (*conn_bus->tme_bus_signal)
	  (conn_bus,
	   TME_BUS_SIGNAL_INT_UNSPEC
	   | (int_asserted
	      ? TME_BUS_SIGNAL_LEVEL_ASSERTED
	      : TME_BUS_SIGNAL_LEVEL_NEGATED));

	/* lock our mutex: */
	tme_mutex_lock(&_3c400->tme_3c400_mutex);
	
	/* if this callout was successful, note the new state of the
	   interrupt signal: */
	if (rc == TME_OK) {
	  _3c400->tme_3c400_int_asserted = int_asserted;
	}

	/* otherwise, remember that at some later time this callout
	   should be attempted again: */
	else {
	  later_callouts |= TME_3C400_CALLOUT_INT;
	}
      }
    }
  }
  
  /* put in any later callouts, and clear that callouts are running: */
  _3c400->tme_3c400_callout_flags = later_callouts;
}

/* the _3c400 bus cycle handler: */
static int
_tme_3c400_bus_cycle(void *__3c400, struct tme_bus_cycle *cycle_init)
{
  struct tme_3c400 *_3c400;
  tme_uint16_t csr_old, csr_new, csr_diff;
  int new_callouts;

  /* recover our data structure: */
  _3c400 = (struct tme_3c400 *) __3c400;

  /* assume we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&_3c400->tme_3c400_mutex);

  /* get the changed CSR value - there are bits that software can only
     set, and not clear: */
  csr_old = TME_3C400_CSR_GET(_3c400);

  /* unless this address falls within the address ROM, run the cycle: */
  if ((cycle_init->tme_bus_cycle_address 
       < TME_3C400_REG_AROM)
      || (cycle_init->tme_bus_cycle_address
	  >= TME_3C400_REG_ARAM)) {
    tme_bus_cycle_xfer_memory(cycle_init, 
			      _3c400->tme_3c400_card,
			      _3c400->tme_3c400_device.tme_bus_device_address_last);
  }

  /* get the new CSR value, and put back any bits that software
     cannot clear: */
  csr_new = TME_3C400_CSR_GET(_3c400);
  csr_new |= (csr_old & ~TME_3C400_CSR_INTPA);

  /* get the set of bits that has changed: */
  csr_diff = (csr_old ^ csr_new);

  /* if this is a reset: */
  if (csr_diff & TME_3C400_CSR_RESET) {
    _tme_3c400_reset(_3c400);
  }

  /* otherwise: */
  else {

    /* if the transmit buffer now belongs to the card, call out
       that we are now readable: */
    if (csr_diff & TME_3C400_CSR_TBSW) {
      new_callouts |= TME_3C400_CALLOUT_CTRL;
    }

    /* if the address RAM now belongs to the card, or if the address
       filter configuration has changed, call out the config change: */
    if ((csr_diff & TME_3C400_CSR_AMSW)
	|| (csr_diff & TME_3C400_CSR_PAMASK) != 0) {
      new_callouts |= TME_3C400_CALLOUT_CONFIG;
    }

    /* if any interrupt enable status bits have changed,
       call out the interrupt signal change: */
    if (csr_diff & (TME_3C400_CSR_BINT
		    | TME_3C400_CSR_AINT
		    | TME_3C400_CSR_TINT)) {
      new_callouts |= TME_3C400_CALLOUT_INT;
    }

    /* set the current CSR value: */
    TME_3C400_CSR_PUT(_3c400, csr_new);

#ifndef TME_NO_LOG
    if (csr_new != _3c400->tme_3c400_last_log_csr) {
      _3c400->tme_3c400_last_log_csr = csr_new;
      tme_log(&_3c400->tme_3c400_element->tme_element_log_handle,
	      1000, TME_OK,
	      (&_3c400->tme_3c400_element->tme_element_log_handle,
	       "csr now 0x%04x",
	       csr_new));
    }
#endif /* !TME_NO_LOG */
  }

  /* make any new callouts: */
  _tme_3c400_callout(_3c400, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&_3c400->tme_3c400_mutex);

  /* no faults: */
  return (TME_OK);
}

/* this is called when a device changes its configuration: */
static int
_tme_3c400_config(struct tme_ethernet_connection *conn_eth, 
		  struct tme_ethernet_config *config)
{
  /* we don't care when other devices on the Ethernet
     reconfigure themselves: */
  return (TME_OK);
}

/* this is called when control lines change: */
static int
_tme_3c400_ctrl(struct tme_ethernet_connection *conn_eth, 
		unsigned int ctrl)
{
  struct tme_3c400 *_3c400;
  int new_callouts;

  /* recover our data structures: */
  _3c400 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock the mutex: */
  tme_mutex_lock(&_3c400->tme_3c400_mutex);

  /* if this connection is readable, call out a read: */
  if (ctrl & TME_ETHERNET_CTRL_OK_READ) {
    new_callouts |= TME_3C400_CALLOUT_READ;
  }

  /* make any new callouts: */
  _tme_3c400_callout(_3c400, new_callouts);

  /* unlock the mutex: */
  tme_mutex_unlock(&_3c400->tme_3c400_mutex);

  return (TME_OK);
}

/* this is called to read frames (from the 3c400 perspective, to transmit them): */
static int
_tme_3c400_read(struct tme_ethernet_connection *conn_eth, 
		tme_ethernet_fid_t *_frame_id,
		struct tme_ethernet_frame_chunk *frame_chunks,
		unsigned int flags)
{
  struct tme_3c400 *_3c400;
  struct tme_ethernet_frame_chunk frame_chunk_buffer;
  tme_uint16_t csr, count;
  int new_callouts;
  int rc;

  /* recover our data structures: */
  _3c400 = conn_eth->tme_ethernet_connection.tme_connection_element->tme_element_private;

  /* assume that we won't need any new callouts: */
  new_callouts = 0;

  /* lock our mutex: */
  tme_mutex_lock(&_3c400->tme_3c400_mutex);

  /* get the current CSR value: */
  csr = TME_3C400_CSR_GET(_3c400);

  /* if the transmit buffer is full: */
  if (csr & TME_3C400_CSR_TBSW) {

    /* get the count of bytes in the frame: */
    count = (TME_3C400_SIZ_BUF
	     - (tme_betoh_u16(*((tme_uint16_t *) 
				&_3c400->tme_3c400_card[TME_3C400_REG_TBUF]))
		& TME_3C400_RBUF_DOFF_MASK));

    /* form the single frame chunk: */
    frame_chunk_buffer.tme_ethernet_frame_chunk_next = NULL;
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes
      = (&_3c400->tme_3c400_card[TME_3C400_REG_TBUF]
	 + TME_3C400_SIZ_BUF
	 - count);
    frame_chunk_buffer.tme_ethernet_frame_chunk_bytes_count
      = count;

    /* copy out the frame: */
    count = tme_ethernet_chunks_copy(frame_chunks, &frame_chunk_buffer);

    /* if this isn't a peek: */
    if (!(flags & TME_ETHERNET_READ_PEEK)) {

      /* mark the transmit buffer as empty: */
      csr &= ~TME_3C400_CSR_TBSW;
      TME_3C400_CSR_PUT(_3c400, csr);
      
      /* if transmit buffer interrupts are enabled, call out
	 an interrupt: */
      if (csr & TME_3C400_CSR_TINT) {
	new_callouts |= TME_3C400_CALLOUT_INT;
      }
    }

    /* success: */
    rc = count;
  }

  /* if the transmit buffer is empty, return an error: */
  else {
    rc = -ENOENT;
  }

  /* make any new callouts: */
  _tme_3c400_callout(_3c400, new_callouts);

  /* unlock our mutex: */
  tme_mutex_unlock(&_3c400->tme_3c400_mutex);

  /* done: */
  return (rc);
}

/* the _3c400 TLB filler: */
static int
_tme_3c400_tlb_fill(void *__3c400, struct tme_bus_tlb *tlb, 
		    tme_bus_addr_t address_wider, unsigned int cycles)
{
  struct tme_3c400 *_3c400;
  tme_bus_addr32_t address;

  /* recover our data structure: */
  _3c400 = (struct tme_3c400 *) __3c400;

  /* get the normal-width address: */
  address = address_wider;
  assert (address == address_wider);

  /* the address must be within range: */
  assert(address < TME_3C400_SIZ_CARD);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* if the address falls from the CSR to the address ROM: */
  if (TME_3C400_REG_CSR <= address 
      && address < TME_3C400_REG_AROM) {

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_3C400_REG_CSR;
    tlb->tme_bus_tlb_addr_last = TME_3C400_REG_AROM - 1;
  }

  /* if this address falls from the address ROM to the address RAM: */
  else if (TME_3C400_REG_AROM <= address 
	   && address < TME_3C400_REG_ARAM) {

    /* this TLB entry covers this range: */
    tlb->tme_bus_tlb_addr_first = TME_3C400_REG_AROM;
    tlb->tme_bus_tlb_addr_last = TME_3C400_REG_ARAM - 1;
  }

  /* anything else covers the remainder of the device: */
  else {

    /* this TLB entry can cover from the address RAM to the end of the card: */
    tlb->tme_bus_tlb_addr_first = TME_3C400_REG_ARAM;
    tlb->tme_bus_tlb_addr_last = TME_3C400_SIZ_CARD - 1;

    /* this TLB entry allows fast writing: */
    tlb->tme_bus_tlb_emulator_off_write = &_3c400->tme_3c400_card[0];
  }

  /* all address ranges allow fast reading: */
  tlb->tme_bus_tlb_emulator_off_read = &_3c400->tme_3c400_card[0];
  tlb->tme_bus_tlb_rwlock = &_3c400->tme_3c400_rwlock;

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = _3c400;
  tlb->tme_bus_tlb_cycle = _tme_3c400_bus_cycle;

  return (TME_OK);
}

/* this makes a new Ethernet connection: */
static int
_tme_3c400_connection_make(struct tme_connection *conn, unsigned int state)
{
  struct tme_3c400 *_3c400;
  struct tme_ethernet_connection *conn_eth;
  struct tme_ethernet_connection *conn_eth_other;

  /* recover our data structures: */
  _3c400 = conn->tme_connection_element->tme_element_private;
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
    tme_mutex_lock(&_3c400->tme_3c400_mutex);

    /* save our connection: */
    _3c400->tme_3c400_eth_connection = conn_eth_other;

    /* unlock our mutex: */
    tme_mutex_unlock(&_3c400->tme_3c400_mutex);
  }

  return (TME_OK);
}

/* this breaks a connection: */
static int
_tme_3c400_connection_break(struct tme_connection *conn, unsigned int state)
{
  abort();
}

/* this makes a new connection side for a 3c400: */
static int
_tme_3c400_connections_new(struct tme_element *element,
			   const char * const *args,
			   struct tme_connection **_conns,
			   char **_output)
{
  struct tme_3c400 *_3c400;
  struct tme_ethernet_connection *conn_eth;
  struct tme_connection *conn;
  int rc;

  /* recover our data structure: */
  _3c400 = (struct tme_3c400 *) element->tme_element_private;

  /* make the generic bus device connection side: */
  rc = tme_bus_device_connections_new(element, args, _conns, _output);
  if (rc != TME_OK) {
    return (rc);
  }

  /* if we don't have an Ethernet connection, make one: */
  if (_3c400->tme_3c400_eth_connection == NULL) {

    /* allocate the new Ethernet connection: */
    conn_eth = tme_new0(struct tme_ethernet_connection, 1);
    conn = &conn_eth->tme_ethernet_connection;
    
    /* fill in the generic connection: */
    conn->tme_connection_next = *_conns;
    conn->tme_connection_type = TME_CONNECTION_ETHERNET;
    conn->tme_connection_score = tme_ethernet_connection_score;
    conn->tme_connection_make = _tme_3c400_connection_make;
    conn->tme_connection_break = _tme_3c400_connection_break;

    /* fill in the Ethernet connection: */
    conn_eth->tme_ethernet_connection_config = _tme_3c400_config;
    conn_eth->tme_ethernet_connection_ctrl = _tme_3c400_ctrl;
    conn_eth->tme_ethernet_connection_read = _tme_3c400_read;

    /* return the connection side possibility: */
    *_conns = conn;
  }

  /* done: */
  return (TME_OK);
}

/* the new _3c400 function: */
TME_ELEMENT_SUB_NEW_DECL(tme_bus_multibus,3c400) {
  struct tme_3c400 *_3c400;
  tme_uint8_t arom[TME_ETHERNET_ADDR_SIZE];
  int arom_ok;
  int arg_i;
  int usage;

  /* check our arguments: */
  usage = 0;
  arom_ok = FALSE;
  arg_i = 1;
  for (;;) {

    /* our Ethernet address ROM: */
    if (TME_ARG_IS(args[arg_i], "ether")
	&& tme_ethernet_addr_parse(args[arg_i + 1], arom) == TME_OK) {
      arom_ok = TRUE;
      arg_i += 2;
    }

    /* if we ran out of arguments: */
    else if (args[arg_i] == NULL) {

      /* we must have been given our Ethernet address ROM: */
      if (!arom_ok) {
	usage = TRUE;
      }
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
			    "%s %s ether %s",
			    _("usage:"),
			    args[0],
			    _("ETHERNET-ADDRESS"));
    return (EINVAL);
  }

  /* start the _3c400 structure: */
  _3c400 = tme_new0(struct tme_3c400, 1);
  _3c400->tme_3c400_element = element;
  tme_mutex_init(&_3c400->tme_3c400_mutex);
  tme_rwlock_init(&_3c400->tme_3c400_rwlock);
  memcpy(_3c400->tme_3c400_card + TME_3C400_REG_AROM,
	 arom,
	 sizeof(arom));

  /* initialize our simple bus device descriptor: */
  assert((TME_3C400_SIZ_CARD & (TME_3C400_SIZ_CARD - 1)) == 0);
  _3c400->tme_3c400_device.tme_bus_device_element = element;
  _3c400->tme_3c400_device.tme_bus_device_tlb_fill = _tme_3c400_tlb_fill;
  _3c400->tme_3c400_device.tme_bus_device_address_last = TME_3C400_SIZ_CARD - 1;

  /* fill the element: */
  element->tme_element_private = _3c400;
  element->tme_element_connections_new = _tme_3c400_connections_new;

  return (TME_OK);
}
