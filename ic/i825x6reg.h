/* $Id: i825x6reg.h,v 1.2 2007/02/21 01:26:19 fredette Exp $ */

/* ic/i825x6reg.h - register definitions for Intel 82586/82596 emulation: */

/*
 * Copyright (c) 2004 Matt Fredette
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

#ifndef _TME_I825X6REG_H
#define _TME_I825X6REG_H

#include <tme/common.h>
_TME_RCSID("$Id: i825x6reg.h,v 1.2 2007/02/21 01:26:19 fredette Exp $");

/* macros: */

/* the SCP address: */
#define TME_I825X6_SCP_ADDRESS		(0x00fffff4)

/* the SCP: */
#define TME_I825X6_SCP_SYSBUS		(2)
#define  TME_I825X6_SCP_SYSBUS_MODE_MASK (0x06)
#define   TME_I825X6_SCP_SYSBUS_MODE_82586 (0x00)
#define   TME_I825X6_SCP_SYSBUS_MODE_32S   (0x02)
#define   TME_I825X6_SCP_SYSBUS_MODE_LINEAR (0x04)
#define  TME_I825X6_SCP_SYSBUS_BE	 (0x80)
#define TME_I825X6_SCP_ISCP_ADDRESS	(8)
#define TME_I825X6_SCP_SIZE		(12)

/* the ISCP: */
#define TME_I825X6_ISCP_BUSY		(0)
#define TME_I82586_ISCP_SCB_OFFSET	(2)
#define TME_I82586_ISCP_SCB_BASE	(4)
#define TME_I825X6_ISCP_SIZE		(8)

/* the common SCB offsets: */
#define TME_I825X6_SCB_STAT_CUS_RUS_T	(0)
#define TME_I825X6_SCB_ACK_CUC_R_RUC	(2)

/* the i82586 and 32-bit segmented i82596 Command Block List and Receive Frame Area SCB offsets: */
#define TME_I82586_SCB_CBL_OFFSET	(4)
#define TME_I82586_SCB_RFA_OFFSET	(6)

/* the i82586 error counter SCB offsets: */
#define TME_I82586_SCB_ERRORS_CRC	(8)
#define TME_I82586_SCB_ERRORS_ALIGN	(10)
#define TME_I82586_SCB_ERRORS_RESOURCE	(12)
#define TME_I82586_SCB_ERRORS_OVERRUN	(14)

/* the i82586 SCB size: */
#define TME_I82586_SCB_SIZE		(16)

/* the SCB Status bits: */
#define TME_I825X6_SCB_STAT_MASK	(0xf000)
#define  TME_I825X6_SCB_STAT_CX		 (0x8000)
#define  TME_I825X6_SCB_STAT_FR		 (0x4000)
#define  TME_I825X6_SCB_STAT_CNA	 (0x2000)
#define  TME_I825X6_SCB_STAT_RNR	 (0x1000)

/* the SCB Command Unit Status field: */
#define TME_I825X6_SCB_CUS_MASK		(0x0700)
#define  TME_I825X6_SCB_CUS_IDLE	 (0x0000)
#define  TME_I825X6_SCB_CUS_SUSPENDED	 (0x0100)
#define  TME_I825X6_SCB_CUS_ACTIVE	 (0x0200)

/* the SCB Receive Unit Status: */
#define TME_I82586_SCB_RUS_MASK	 	(0x0070)
#define  TME_I825X6_SCB_RUS_READY	 (0x0040)
#define  TME_I825X6_SCB_RUS_ERESOURCE	 (0x0020)
#define  TME_I825X6_SCB_RUS_SUSPENDED	 (0x0010)
#define  TME_I825X6_SCB_RUS_IDLE	 (0x0000)

/* the SCB Command Unit Command field: */
#define TME_I825X6_SCB_CUC_MASK		(0x0700)
#define  TME_I825X6_SCB_CUC_NOP		 (0x0000)
#define  TME_I825X6_SCB_CUC_START	 (0x0100)
#define  TME_I825X6_SCB_CUC_RESUME	 (0x0200)
#define  TME_I825X6_SCB_CUC_SUSPEND	 (0x0300)
#define  TME_I825X6_SCB_CUC_ABORT	 (0x0400)

/* the SCB Reset bit: */
#define TME_I825X6_SCB_RESET		(0x0080)

/* the SCB Receive Unit Command field: */
#define TME_I825X6_SCB_RUC_MASK		(0x0070)
#define  TME_I825X6_SCB_RUC_NOP		 (0x0000)
#define  TME_I825X6_SCB_RUC_START	 (0x0010)
#define  TME_I825X6_SCB_RUC_RESUME	 (0x0020)
#define  TME_I825X6_SCB_RUC_SUSPEND	 (0x0030)
#define  TME_I825X6_SCB_RUC_ABORT	 (0x0040)

/* the i825x6 Command Block, Receive Frame Descriptor, and Receive
   Buffer Descriptor flags: */
#define  TME_I825X6_FLAG_EL		 (0x8000)
#define  TME_I825X6_FLAG_S		 (0x4000)
#define  TME_I825X6_FLAG_C		 (0x8000)
#define  TME_I825X6_FLAG_B		 (0x4000)
#define  TME_I825X6_FLAG_OK		 (0x2000)

/* the i825x6 Command Block: */
#define TME_I825X6_CB_C_B_OK_A		(0)
#define  TME_I825X6_CB_A		 (0x1000)
#define TME_I825X6_CB_EL_S_I_CMD	(2)
#define  TME_I825X6_CB_I		 (0x2000)
#define  TME_I825X6_CB_CMD_MASK		 (0x0007)
#define   TME_I825X6_CB_CMD_NOP		  (0x0000)
#define   TME_I825X6_CB_CMD_SETUP_IA	  (0x0001)
#define   TME_I825X6_CB_CMD_CONFIGURE	  (0x0002)
#define   TME_I825X6_CB_CMD_SETUP_MC	  (0x0003)
#define   TME_I825X6_CB_CMD_TRANSMIT	  (0x0004)
#define   TME_I825X6_CB_CMD_TDR		  (0x0005)
#define   TME_I825X6_CB_CMD_DUMP	  (0x0006)
#define   TME_I825X6_CB_CMD_DIAGNOSE	  (0x0007)

/* the i82586 and 32-bit segmented i82596 Command Block: */
#define TME_I82586_CB_LINK_OFFSET	(4)
#define TME_I82586_CB_X			(6)

/* the i82586 and 32-bit segmented i82596 Transmit Command Block Transmit Buffer field: */
#define TME_I82586_TCB_TBD_OFFSET	(6)

/* the i82586 Transmit Command Block: */
#define  TME_I825X6_TCB_STATUS_MASK	 (0x0ff0)
#define   TME_I825X6_TCB_STATUS_UNDERRUN  (0x0100)
#define TME_I82586_TCB_ADDR_DEST	(8)
#define TME_I82586_TCB_LENGTH		(14)
#define TME_I82585_TCB_SIZE		(16)

/* the i82586 and 32-bit segmented i82586 Transmit Buffer: */
#define TME_I82586_TBD_EOF_SIZE		(0)
#define  TME_I82586_TBD_EOF		 (0x8000)
#define  TME_I82586_TBD_SIZE_MASK	 (0x3fff)
#define TME_I82586_TBD_TBD_OFFSET	(2)
#define TME_I82586_TBD_TB_ADDRESS	(4)

/* the i82586 and 32-bit segmented i82596 TDR result field: */
#define TME_I82586_TDR_STATUS_OK	(0x8000)

/* the i825x6 Receive Frame Descriptor: */
#define TME_I825X6_RFD_C_B_OK_STATUS	(0)
#define  TME_I825X6_RFD_STATUS_RNR	 (0x0200)
#define TME_I825X6_RFD_EL_S_SF		(2)
#define  TME_I82596_RFD_SF		 (0x0008)

/* the i82586 and 32-bit segmented i82596 Receive Frame Descriptor: */
#define TME_I82586_RFD_LINK_OFFSET	(4)
#define TME_I82586_RFD_RBD_OFFSET	(6)

/* the i82586 Receive Frame Descriptor: */
#define TME_I82586_RFD_RBD_ETH_HEADER	(6)

/* the i825x6 Receive Buffer Descriptor: */
#define TME_I825X6_RBD_EOF_F_ACT_COUNT	(0)
#define  TME_I825X6_RBD_EOF		 (0x8000)
#define  TME_I825X6_RBD_F		 (0x4000)
#define  TME_I825X6_RBD_ACT_COUNT_MASK	 (0x3fff)

/* the i82586 and 32-bit segmented i82596 Receive Buffer Descriptor: */
#define TME_I82586_RBD_RBD_OFFSET	(2)
#define TME_I82586_RBD_RB_ADDRESS	(4)
#define TME_I82586_RBD_EL_P_SIZE	(8)
#define  TME_I825X6_RBD_EL		 (0x8000)
#define  TME_I82596_RBD_P		 (0x4000)
#define  TME_I825X6_RBD_SIZE_MASK	 (0x3fff)

#endif /* !_TME_I825X6REG_H */
