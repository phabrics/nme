/* $Id: z8530reg.h,v 1.3 2003/10/25 17:07:58 fredette Exp $ */

/* ic/z8530reg.h - register definitions for Zilog 8530 emulation: */

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

#ifndef _TME_Z8530REG_H
#define _TME_Z8530REG_H

#include <tme/common.h>
_TME_RCSID("$Id: z8530reg.h,v 1.3 2003/10/25 17:07:58 fredette Exp $");

/* macros: */

/* WR0: */
#define TME_Z8530_WR0_CRC_RESET_MASK	(0xc0)
#define  TME_Z8530_WR0_CRC_RESET_NULL	(0x00)
#define  TME_Z8530_WR0_CRC_RESET_RX	(0x40)
#define  TME_Z8530_WR0_CRC_RESET_TX	(0x80)
#define  TME_Z8530_WR0_CRC_RESET_TX_EOM	(0xc0)
#define TME_Z8530_WR0_CMD_MASK		(0x38)
#define  TME_Z8530_WR0_CMD_NULL		(0x00)
#define  TME_Z8530_WR0_CMD_POINT_HI	(0x08)
#define  TME_Z8530_WR0_CMD_RESET_STATUS	(0x10)
#define  TME_Z8530_WR0_CMD_SEND_ABORT	(0x18)
#define  TME_Z8530_WR0_CMD_INT_NEXT_RX	(0x20)
#define  TME_Z8530_WR0_CMD_RESET_TX	(0x28)
#define  TME_Z8530_WR0_CMD_RESET_ERROR	(0x30)
#define  TME_Z8530_WR0_CMD_RESET_IUS	(0x38)
#define TME_Z8530_WR0_REGISTER_POINTER	(0x07)

/* WR1: */
/* for now we ignore the DMA-related bits: */
#define TME_Z8530_WR1_RX_INT_MASK	(0x18)
#define  TME_Z8530_WR1_RX_INT_DISABLE	(0x00)
#define  TME_Z8530_WR1_RX_INT_1ST_SPCL	(0x08)
#define  TME_Z8530_WR1_RX_INT_ALL_SPCL	(0x10)
#define  TME_Z8530_WR1_RX_INT_SPCL	(0x18)
#define TME_Z8530_WR1_PARITY_SPCL	(0x04)
#define TME_Z8530_WR1_TX_INT_ENABLE	(0x02)
#define TME_Z8530_WR1_STATUS_INT_ENABLE	(0x01)

/* WR3: */
#define TME_Z8530_WR3_RX_CSIZE_MASK	(0xc0)
#define  TME_Z8530_WR3_RX_CSIZE_5	(0x00)
#define  TME_Z8530_WR3_RX_CSIZE_7	(0x40)
#define  TME_Z8530_WR3_RX_CSIZE_6	(0x80)
#define  TME_Z8530_WR3_RX_CSIZE_8	(0xc0)
#define TME_Z8530_WR3_AUTO_ENABLES	(0x20)
#define TME_Z8530_WR3_HUNT		(0x10)
#define TME_Z8530_WR3_RX_CRC_ENABLE	(0x08)
#define TME_Z8530_WR3_ADDRESS_SEARCH	(0x04)
#define TME_Z8530_WR3_SYNC_LOAD_INHIB	(0x02)
#define TME_Z8530_WR3_RX_ENABLE		(0x01)

/* WR4: */
#define TME_Z8530_WR4_CLOCK_X_MASK	(0xc0)
#define  TME_Z8530_WR4_CLOCK_X1		(0x00)
#define  TME_Z8530_WR4_CLOCK_X16	(0x40)
#define  TME_Z8530_WR4_CLOCK_X32	(0x80)
#define  TME_Z8530_WR4_CLOCK_X64	(0xc0)
#define TME_Z8530_WR4_SYNC_MASK		(0x30)
#define  TME_Z8530_WR4_SYNC_8BIT	(0x00)
#define  TME_Z8530_WR4_SYNC_16BIT	(0x10)
#define  TME_Z8530_WR4_SYNC_SDLC	(0x20)
#define  TME_Z8530_WR4_SYNC_EXTERNAL	(0x30)
#define TME_Z8530_WR4_STOP_MASK		(0x0c)
#define  TME_Z8530_WR4_STOP_SYNC	(0x00)
#define  TME_Z8530_WR4_STOP_1BIT	(0x04)
#define  TME_Z8530_WR4_STOP_1_5BITS	(0x08)
#define  TME_Z8530_WR4_STOP_2BITS	(0x0c)
#define TME_Z8530_WR4_PARITY_EVEN	(0x02)
#define TME_Z8530_WR4_PARITY_ENABLE	(0x01)

/* WR5: */
#define TME_Z8530_WR5_DTR		(0x80)
#define TME_Z8530_WR5_TX_CSIZE_MASK	(0x60)
#define  TME_Z8530_WR5_TX_CSIZE_5	(0x00)
#define  TME_Z8530_WR5_TX_CSIZE_7	(0x20)
#define  TME_Z8530_WR5_TX_CSIZE_6	(0x40)
#define  TME_Z8530_WR5_TX_CSIZE_8	(0x60)
#define TME_Z8530_WR5_BREAK		(0x10)
#define TME_Z8530_WR5_TX_ENABLE		(0x08)
#define TME_Z8530_WR5_SDLC_CRC16	(0x04)
#define TME_Z8530_WR5_RTS		(0x02)
#define TME_Z8530_WR5_TX_CRC_ENABLE	(0x01)

/* WR9: */
#define TME_Z8530_WR9_RESET_MASK	(0xc0)
#define  TME_Z8530_WR9_RESET_NULL	(0x00)
#define  TME_Z8530_WR9_RESET_CHAN_B	(0x40)
#define  TME_Z8530_WR9_RESET_CHAN_A	(0x80)
#define  TME_Z8530_WR9_RESET_CHIP	(0xc0)
#define TME_Z8530_WR9_SOFT_INTACK	(0x20)
#define TME_Z8530_WR9_INTVEC_STATUS	(0x10)
#define TME_Z8530_WR9_MIE		(0x08)
#define TME_Z8530_WR9_DLC		(0x04)
#define TME_Z8530_WR9_NV		(0x02)
#define TME_Z8530_WR9_VIS		(0x01)

/* WR15: */
#define TME_Z8530_WR15_BREAK_IE		(0x80)
#define TME_Z8530_WR15_TX_EOM		(0x40)
#define TME_Z8530_WR15_CTS_IE		(0x20)
#define TME_Z8530_WR15_SYNC_HUNT_IE	(0x10)
#define TME_Z8530_WR15_DCD_IE		(0x08)
#define TME_Z8530_WR15_SDLC_FIFO_ENABLE	(0x04)
#define TME_Z8530_WR15_ZERO_COUNT_IE	(0x02)

/* RR0: */
#define TME_Z8530_RR0_BREAK		(0x80)
#define TME_Z8530_RR0_TX_UNDER_EOM	(0x40)
#define TME_Z8530_RR0_CTS		(0x20)
#define TME_Z8530_RR0_SYNC_HUNT		(0x10)
#define TME_Z8530_RR0_DCD		(0x08)
#define TME_Z8530_RR0_TX_EMPTY		(0x04)
#define TME_Z8530_RR0_ZERO_COUNT	(0x02)
#define TME_Z8530_RR0_RX_AVAIL		(0x01)

/* RR1: */
#define TME_Z8530_RR1_END_OF_FRAME	(0x80)
#define TME_Z8530_RR1_ERROR_FRAME	(0x40)
#define TME_Z8530_RR1_ERROR_RX_OVERRUN	(0x20)
#define TME_Z8530_RR1_ERROR_PARITY	(0x10)
#define TME_Z8530_RR1_RESID_0		(0x08)
#define TME_Z8530_RR1_RESID_1		(0x04)
#define TME_Z8530_RR1_RESID_2		(0x02)
#define TME_Z8530_RR1_ALL_SENT		(0x01)

/* RR3: */
#define TME_Z8530_RR3_CHAN_A_IP_RX	(0x20)
#define TME_Z8530_RR3_CHAN_A_IP_TX	(0x10)
#define TME_Z8530_RR3_CHAN_A_IP_STATUS	(0x08)
#define TME_Z8530_RR3_CHAN_B_IP_RX	(0x04)
#define TME_Z8530_RR3_CHAN_B_IP_TX	(0x02)
#define TME_Z8530_RR3_CHAN_B_IP_STATUS	(0x01)

#endif /* !_TME_Z8530REG_H */
