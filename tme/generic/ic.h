/* $Id: ic.h,v 1.5 2009/08/29 21:19:57 fredette Exp $ */

/* tme/generic/ic/ic-impl.h - header file for generic IC support: */

/*
 * Copyright (c) 2002, 2003 Matt Fredette
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

#ifndef _TME_GENERIC_IC_H
#define _TME_GENERIC_IC_H

#include <tme/common.h>
_TME_RCSID("$Id: ic.h,v 1.5 2009/08/29 21:19:57 fredette Exp $");

/* includes: */
#include <tme/generic/bus.h>

/* macros: */

/* the maximum size, in bytes, of an IC's register file: */
#define TME_IC_REGS_SIZE	(1024 * sizeof(tme_uint32_t))

/* in many cases it can be useful to think of an IC's register file as
   addressable memory on a bus, with a given port size and endianness.

   we assume that each register has a bus address that is naturally
   aligned to the register's size.  we assume that no registers are
   larger than the port size.

   the basic unit of the register file becomes registers that are
   exactly the port size.  increasing bus addresses that are a
   multiple of the port size always map to increasing "basic device"
   register numbers, regardless of the device's endianness.

   so, registers that are exactly the port size are always easy to deal
   with.  their tme_ic_ireg_uintN positions are simply their bus
   addresses divided by the port size.

   the tme_ic_ireg_uintN indices of registers that are smaller than
   the port size need to take endianness into account.  the
   tme_ic_ireg_uintN convention holds that a less significant part of
   a larger register is identified by a smaller index than a more
   significant part of the same larger register - basically a
   little-endian convention.

   so when you have a bus address for some smaller part of a basic
   device register, the basic device register is still identified as
   the bus address divided by the port size.  then, the index of the
   specific smaller part of that register depends on the (bus address
   % port size) bits.

   if the device is a little-endian device, smaller values of this
   expression mean less significant parts of the larger register,
   which matches the tme_ic_ireg_uintN convention, so they can be used
   directly.

   if the device is a big-endian device, smaller values of this expression
   mean *more* significant parts of the larger register, which is the
   opposite of the tme_ic_ireg_uintN convention, so the bits have to
   be flipped before they can be used.

   note that this is all independant of the endianness of the
   *emulator host*.  that endianness has already been taken into
   consideration in the definition of the tme_ic_ireg_uintN macros
   below.  */
#if TME_ENDIAN_LITTLE != 0 || TME_ENDIAN_BIG != 1
#error "TME_ENDIAN_ definitions broken"
#endif /* TME_ENDIAN_LITTLE != 0 || TME_ENDIAN_BIG != 1 */
#define _TME_IC_BUS_IREG(portsz_lg2, endian, addr, regsz)	\
  (((addr) ^ ((endian) * ((1 << (portsz_lg2)) - 1))) / (regsz))
#define TME_IC_BUS_IREG8(portsz_lg2, endian, addr) _TME_IC_BUS_IREG(portsz_lg2, endian, addr, sizeof(tme_uint8_t))
#define TME_IC_BUS_IREG16(portsz_lg2, endian, addr) _TME_IC_BUS_IREG(portsz_lg2, endian, addr, sizeof(tme_uint16_t))
#define TME_IC_BUS_IREG32(portsz_lg2, endian, addr) _TME_IC_BUS_IREG(portsz_lg2, endian, addr, sizeof(tme_uint32_t))

/* this can be used to recover the bus address of a register, given an
   actual tme_ic_iregs member.  even so, it should optimize down to a
   constant: */
#define TME_IC_IREG_BUS(portsz_lg2, endian, reg, _struct, ic_member)	\
(									\
 /* get the raw byte offset of the register in the file: */		\
 (((unsigned long) &(((struct _struct *) 0)->reg))			\
  - ((unsigned long) &(((struct _struct *) 0)->ic_member tme_ic_iregs)))\
 /* if the device endianness matches the endianness of the emulator	\
    host, the raw byte offset of the register is the same as the	\
    device address.  otherwise, we have to flip the non-alignment bits	\
    covered by the port size in the raw address.			\
									\
    this macro *does* have to be aware of the endianness of the		\
    emulator host, because the raw byte offset we calculate above is	\
    "beyond the control" of the tme_ic_ireg_uintN macros: */		\
 ^ (((endian) ^ TME_ENDIAN_NATIVE)					\
    * ((((1 << (portsz_lg2)) - 1) * sizeof(((struct _struct *) 0)->reg))\
       & ((1 << (portsz_lg2)) - 1))))

/* this fills a bus cycle structure given an address: */
#define TME_IC_IREG_BUS_CYCLE(portsz_lg2, endian, ic, addr, cycle)	\
do {									\
									\
  /* calculate the address of the byte corresponding to the given	\
     address.  if the device endianness matches the endianness of the	\
     emulator host, the address maps directly regardless of port size.	\
     otherwise, we have to flip all address bits inside the port size	\
     to locate this byte: */						\
  (cycle)->tme_bus_cycle_buffer = 					\
    (((tme_uint8_t *) &(ic)->tme_ic_iregs)				\
     + ((addr)								\
	^ (((endian) ^ TME_ENDIAN_NATIVE)				\
	   * ((1 << (portsz_lg2)) - 1))));				\
									\
  /* we have no routing yet: */						\
  (cycle)->tme_bus_cycle_lane_routing = NULL;				\
									\
  /* set the address: */						\
  (cycle)->tme_bus_cycle_address = (addr);				\
									\
  /* if the device endianness matches the endianness of the emulator	\
     host, increasing bus cycle buffer addresses correspond to		\
     increasing bus addresses, otherwise decreasing bus cycle buffer	\
     addresses correspond to increasing bus addresses: */		\
  (cycle)->tme_bus_cycle_buffer_increment = 				\
    1 - (2 * ((endian) ^ TME_ENDIAN_NATIVE));				\
									\
  /* set this device's port, assuming that its least lane is zero: */	\
  (cycle)->tme_bus_cycle_port = TME_BUS_CYCLE_PORT(0, portsz_lg2);	\
} while (/* CONSTCOND */ 0)

/* an ic: */
struct tme_ic {

  /* the IC's addressable register file: */
  union {
#ifdef TME_HAVE_INT64_T
    tme_uint64_t	tme_ic_iregs_uint64s[TME_IC_REGS_SIZE >> 3];
    tme_int64_t		tme_ic_iregs_int64s[TME_IC_REGS_SIZE >> 3];
#endif /* TME_HAVE_INT64_T */
    tme_uint32_t	tme_ic_iregs_uint32s[TME_IC_REGS_SIZE >> 2];
    tme_int32_t		tme_ic_iregs_int32s[TME_IC_REGS_SIZE >> 2];
    tme_uint16_t	tme_ic_iregs_uint16s[TME_IC_REGS_SIZE >> 1];
    tme_int16_t		tme_ic_iregs_int16s[TME_IC_REGS_SIZE >> 1];
    tme_uint8_t		tme_ic_iregs_uint8s[TME_IC_REGS_SIZE];
    tme_int8_t		tme_ic_iregs_int8s[TME_IC_REGS_SIZE];
  } tme_ic_iregs;
#ifdef WORDS_BIGENDIAN
#ifdef TME_HAVE_INT64_T
#define tme_ic_ireg_uint64(x)	tme_ic_iregs.tme_ic_iregs_uint64s[x]
#define tme_ic_ireg_int64(x)	tme_ic_iregs.tme_ic_iregs_int64s[x]
#define tme_ic_ireg_uint32(x)	tme_ic_iregs.tme_ic_iregs_uint32s[(x) ^ 1]
#define tme_ic_ireg_int32(x)	tme_ic_iregs.tme_ic_iregs_int32s[(x) ^ 1]
#define tme_ic_ireg_uint16(x)	tme_ic_iregs.tme_ic_iregs_uint16s[(x) ^ 3]
#define tme_ic_ireg_int16(x)	tme_ic_iregs.tme_ic_iregs_int16s[(x) ^ 3]
#define tme_ic_ireg_uint8(x)	tme_ic_iregs.tme_ic_iregs_uint8s[(x) ^ 7]
#define tme_ic_ireg_int8(x)	tme_ic_iregs.tme_ic_iregs_int8s[(x) ^ 7]
#else  /* !TME_HAVE_INT64_T */
#define tme_ic_ireg_uint32(x)	tme_ic_iregs.tme_ic_iregs_uint32s[x]
#define tme_ic_ireg_int32(x)	tme_ic_iregs.tme_ic_iregs_int32s[x]
#define tme_ic_ireg_uint16(x)	tme_ic_iregs.tme_ic_iregs_uint16s[(x) ^ 1]
#define tme_ic_ireg_int16(x)	tme_ic_iregs.tme_ic_iregs_int16s[(x) ^ 1]
#define tme_ic_ireg_uint8(x)	tme_ic_iregs.tme_ic_iregs_uint8s[(x) ^ 3]
#define tme_ic_ireg_int8(x)	tme_ic_iregs.tme_ic_iregs_int8s[(x) ^ 3]
#endif /* !TME_HAVE_INT64_T */
#else  /* !WORDS_BIGENDIAN */
#ifdef TME_HAVE_INT64_T
#define tme_ic_ireg_uint64(x)	tme_ic_iregs.tme_ic_iregs_uint64s[x]
#define tme_ic_ireg_int64(x)	tme_ic_iregs.tme_ic_iregs_int64s[x]
#endif /* TME_HAVE_INT64_T */
#define tme_ic_ireg_uint32(x)	tme_ic_iregs.tme_ic_iregs_uint32s[x]
#define tme_ic_ireg_int32(x)	tme_ic_iregs.tme_ic_iregs_int32s[x]
#define tme_ic_ireg_uint16(x)	tme_ic_iregs.tme_ic_iregs_uint16s[x]
#define tme_ic_ireg_int16(x)	tme_ic_iregs.tme_ic_iregs_int16s[x]
#define tme_ic_ireg_uint8(x)	tme_ic_iregs.tme_ic_iregs_uint8s[x]
#define tme_ic_ireg_int8(x)	tme_ic_iregs.tme_ic_iregs_int8s[x]
#endif /* !WORDS_BIGENDIAN */
};

/* prototypes: */

#endif /* !_TME_GENERIC_IC_H */
