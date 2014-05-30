/* $Id: ncr89c105.c,v 1.2 2009/08/29 21:08:48 fredette Exp $ */

/* ic/ncr89c105.c - NCR89C105 (SLAVIO) implementation: */

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
_TME_RCSID("$Id: ncr89c105.c,v 1.2 2009/08/29 21:08:48 fredette Exp $");

/* includes: */
#undef TME_NEC765_VERSION
#define TME_NEC765_VERSION TME_X_VERSION(0, 0)
#include <tme/element.h>
#include <tme/ic/z8530.h>
#include <tme/ic/nec765.h>
#include <tme/machine/sun.h>
#include <tme/generic/bus-device.h>

/* macros: */

/* the device sections: */
#define TME_NCR89C105_SECTION_NULL		(0)
#define TME_NCR89C105_SECTION_AUXIO		(1)

/* structures: */

/* the device: */
struct tme_ncr89c105 {

  /* our simple bus device header: */
  struct tme_bus_device tme_ncr89c105_device;
#define tme_ncr89c105_element tme_ncr89c105_device.tme_bus_device_element

  /* the mutex protecting the device: */
  tme_mutex_t tme_ncr89c105_mutex;

  /* the section emulated: */
  unsigned int tme_ncr89c105_section;

  /* the auxio register: */
  tme_uint8_t tme_ncr89c105_auxio;
};

/* the ncr89c105 bus cycle handler: */
static int
_tme_ncr89c105_bus_cycle(void *_ncr89c105, 
			 struct tme_bus_cycle *cycle_init)
{
  struct tme_ncr89c105 *ncr89c105;
  unsigned int address;
  unsigned int cycle_size;
  tme_uint8_t value32;
  tme_uint8_t value8;

  /* recover our data structure: */
  ncr89c105 = (struct tme_ncr89c105 *) _ncr89c105;

  /* get the address and cycle size: */
  address = cycle_init->tme_bus_cycle_address;
  cycle_size = cycle_init->tme_bus_cycle_size;
  assert (cycle_size <= sizeof(value32));

  /* lock the mutex: */
  tme_mutex_lock(&ncr89c105->tme_ncr89c105_mutex);

  /* if this is a write: */
  if (cycle_init->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE) {

    /* get the value written: */
    tme_bus_cycle_xfer_memory(cycle_init,
			      ((cycle_size == sizeof(value8)
				? &value8
				: (tme_uint8_t *) &value32)
			       - address),
			      address);

    /* dispatch on the section: */
    switch (ncr89c105->tme_ncr89c105_section) {
    default: assert (FALSE);
    case TME_NCR89C105_SECTION_AUXIO:
      assert (cycle_size == sizeof(value8));
      ncr89c105->tme_ncr89c105_auxio = value8;
      break;
    }
  }

  /* otherwise, this is a read: */
  else {

    /* dispatch on the section: */
    switch (ncr89c105->tme_ncr89c105_section) {
    default: assert (FALSE);
    case TME_NCR89C105_SECTION_AUXIO:
      assert (cycle_size == sizeof(value8));
      value8 = ncr89c105->tme_ncr89c105_auxio;
      break;
    }

    /* return the value read: */
    tme_bus_cycle_xfer_memory(cycle_init, 
			      ((cycle_size == sizeof(value8)
				? &value8
				: (tme_uint8_t *) &value32)
			       - address),
			      address);
  }

  /* unlock the mutex: */
  tme_mutex_unlock(&ncr89c105->tme_ncr89c105_mutex);

  return (TME_OK);
}

/* the ncr89c105 TLB filler: */
static int
_tme_ncr89c105_tlb_fill(void *_ncr89c105,
			struct tme_bus_tlb *tlb, 
			tme_bus_addr_t address_wider,
			unsigned int cycles)
{
  struct tme_ncr89c105 *ncr89c105;

  /* recover our data structure: */
  ncr89c105 = (struct tme_ncr89c105 *) _ncr89c105;

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* this TLB entry covers only these addresses: */
  switch (ncr89c105->tme_ncr89c105_section) {
  default: assert (FALSE);
  case TME_NCR89C105_SECTION_AUXIO:
    tlb->tme_bus_tlb_addr_first = address_wider;
    tlb->tme_bus_tlb_addr_last = address_wider;
    break;
  }

  /* allow reading and writing: */
  tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = ncr89c105;
  tlb->tme_bus_tlb_cycle = _tme_ncr89c105_bus_cycle;

  return (TME_OK);
}

/* the new ncr89c105 section function: */
static int
_tme_ncr89c105_new(struct tme_element *element,
		   const char * const *args,
		   const void *extra,
		   char **_output,
		   unsigned int section)
{
  int usage;
  int arg_i;
  struct tme_ncr89c105 *ncr89c105;

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

  /* start the ncr89c105 structure: */
  ncr89c105 = tme_new0(struct tme_ncr89c105, 1);
  tme_mutex_init(&ncr89c105->tme_ncr89c105_mutex);
  ncr89c105->tme_ncr89c105_section = section;
  ncr89c105->tme_ncr89c105_element = element;

  /* initialize our simple bus device descriptor: */
  ncr89c105->tme_ncr89c105_device.tme_bus_device_tlb_fill = _tme_ncr89c105_tlb_fill;
  switch (section) {
  default: assert (FALSE);
  case TME_NCR89C105_SECTION_AUXIO:
    ncr89c105->tme_ncr89c105_device.tme_bus_device_address_last = sizeof(tme_uint8_t);
    break;
  }

  /* fill the element: */
  element->tme_element_private = ncr89c105;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (TME_OK);
}

/* this creates a new ncr89c105 z85c30: */
TME_ELEMENT_SUB_NEW_DECL(tme_ic_ncr89c105,z85c30) {
  struct tme_z8530_socket socket = TME_SUN_Z8530_SOCKET_INIT;
  const char *sub_args[2];

  /* create the z8530: */
  sub_args[0] = "tme/ic/z8530";
  sub_args[1] = NULL;
  return (tme_element_new(element, sub_args, &socket, _output));
}

/* this creates a new ncr89c105 fdtwo: */
TME_ELEMENT_SUB_NEW_DECL(tme_ic_ncr89c105,i82077) {
  struct tme_nec765_socket socket;
  const char *sub_args[2];

  /* create the nec765 socket: */
  memset (&socket, 0, sizeof(socket));
  socket.tme_nec765_socket_version = TME_NEC765_SOCKET_0;

  /* create the i82077: */
  sub_args[0] = "tme/ic/i82077";
  sub_args[1] = NULL;
  return (tme_element_new(element, sub_args, &socket, _output));
}

/* this creates a new ncr89c105 auxio: */
TME_ELEMENT_SUB_NEW_DECL(tme_ic_ncr89c105,auxio) {
  return (_tme_ncr89c105_new(element, args, extra, _output, TME_NCR89C105_SECTION_AUXIO));
}
