/* $Id: posix-memory.c,v 1.7 2009/08/30 21:50:17 fredette Exp $ */

/* host/posix/posix-memory.c - implementation of memory on a POSIX system: */

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
_TME_RCSID("$Id: posix-memory.c,v 1.7 2009/08/30 21:50:17 fredette Exp $");

/* includes: */
#include <tme/generic/bus-device.h>
#include <tme/hash.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/stat.h>
#ifdef HAVE_MMAP
#include <sys/types.h>
#include <sys/mman.h>
#endif /* HAVE_MMAP */

/* macros: */
#define TME_POSIX_MEMORY_RAM		(0)
#define TME_POSIX_MEMORY_ROM		(1)
#define TME_POSIX_MEMORY_PERSISTENT	(2)

/* the minimum size of a cacheable RAM: */
#define TME_MEMORY_POSIX_CACHEABLE_SIZE_RAM	(1024 * 1024)

/* the minimum size of a cacheable ROM: */
#define TME_MEMORY_POSIX_CACHEABLE_SIZE_ROM	(64 * 1024)

/* the size of the writable TLB entry set: */
#define TME_MEMORY_POSIX_TLBS_SIZE	(631)

/* structures: */

/* a valids bitmask: */
struct tme_posix_memory_valids {

  /* valid bitmasks are kept on a linked list: */
  struct tme_posix_memory_valids *tme_posix_memory_valids_next;

  /* the log2 of the page size for this valids bitmask: */
  tme_uint32_t tme_posix_memory_valids_page_size_log2;

  /* the valids bitmask: */
  tme_shared tme_uint8_t tme_posix_memory_valids_bitmask[1];
};

/* the memory structure: */
struct tme_posix_memory {

  /* our simple bus device header: */
  struct tme_bus_device tme_posix_memory_device;

  /* the mutex protecting the device: */
  tme_mutex_t tme_posix_memory_mutex;

  /* our memory type: */
  unsigned int tme_posix_memory_type;

  /* the file descriptor to any backing file: */
  int tme_posix_memory_fd;

  /* this is nonzero if the backing file is mmapped: */
  int tme_posix_memory_mapped;

  /* our rwlock: */
  tme_rwlock_t tme_posix_memory_rwlock;

  /* our memory contents: */
  tme_uint8_t *tme_posix_memory_contents;

  /* our writable TLB entry set: */
  struct tme_token **tme_posix_memory_tlb_tokens;

  /* any valids bitmasks: */
  struct tme_posix_memory_valids *tme_posix_memory_valids;

  /* the current writable TLB size: */
  tme_uint32_t tme_posix_memory_tlb_size;

  /* our bus cacheable structure: */
  struct tme_bus_cacheable tme_posix_memory_cacheable;
};

/* the memory bus cycle handler: */
static int
_tme_posix_memory_bus_cycle(void *_memory, struct tme_bus_cycle *cycle)
{
  struct tme_posix_memory *memory;

  /* recover our data structure: */
  memory = (struct tme_posix_memory *) _memory;

  /* run the cycle: */
  tme_bus_cycle_xfer_memory(cycle, 
			    ((cycle->tme_bus_cycle_type == TME_BUS_CYCLE_WRITE
			      && memory->tme_posix_memory_type == TME_POSIX_MEMORY_ROM)
			     ? NULL
			     : memory->tme_posix_memory_contents),
			    memory->tme_posix_memory_device.tme_bus_device_address_last);

  /* no faults: */
  return (TME_OK);
}

/* this invalidates all outstanding TLBs.  it must be called with the
   mutex held: */
static void
_tme_posix_memory_tlbs_invalidate(struct tme_posix_memory *memory)
{
  signed long tlb_i;
  struct tme_token **tlb_tokens;
  struct tme_token *tlb_token;

  /* invalidate all writable TLBS: */
  tlb_i = TME_MEMORY_POSIX_TLBS_SIZE - 1;
  tlb_tokens = memory->tme_posix_memory_tlb_tokens;
  do {
    tlb_token = tlb_tokens[tlb_i];
    if (tlb_token != NULL) {
      tlb_tokens[tlb_i] = NULL;
      tme_token_invalidate(tlb_token);
    }
  } while (--tlb_i >= 0);
}

/* the memory TLB filler: */
static int
_tme_posix_memory_tlb_fill(void *_memory, struct tme_bus_tlb *tlb, 
			   tme_bus_addr_t address_wider,
			   unsigned int cycles)
{
  struct tme_posix_memory *memory;
  unsigned long address;
  unsigned long memory_address_last;
  struct tme_token *tlb_token;
  struct tme_token **_tlb_token;
  struct tme_token *tlb_token_other;
  struct tme_posix_memory_valids *valids;
  unsigned long page_index;
  tme_uint32_t tlb_size;

  /* recover our data structure: */
  memory = (struct tme_posix_memory *) _memory;

  /* get the normal-width address: */
  address = address_wider;
  assert(address == address_wider);

  /* the address must be within range: */
  memory_address_last = memory->tme_posix_memory_device.tme_bus_device_address_last;
  assert(memory_address_last == memory->tme_posix_memory_device.tme_bus_device_address_last);
  assert(address <= memory_address_last);

  /* initialize the TLB entry: */
  tme_bus_tlb_initialize(tlb);

  /* all memory devices allow fast reading.  all memory devices except
     ROMs allow fast writing: */
  tlb->tme_bus_tlb_emulator_off_read = memory->tme_posix_memory_contents;
  if (memory->tme_posix_memory_type != TME_POSIX_MEMORY_ROM) {
    tlb->tme_bus_tlb_emulator_off_write = memory->tme_posix_memory_contents;
  }
  tlb->tme_bus_tlb_rwlock = &memory->tme_posix_memory_rwlock;

  /* our bus cycle handler: */
  tlb->tme_bus_tlb_cycle_private = memory;
  tlb->tme_bus_tlb_cycle = _tme_posix_memory_bus_cycle;

  /* if this device is cacheable: */
  if (__tme_predict_true(memory->tme_posix_memory_tlb_tokens != NULL)) {

    /* this TLB entry is for cacheable memory: */
    tlb->tme_bus_tlb_cacheable = &memory->tme_posix_memory_cacheable;

    /* if this TLB entry is for writing: */
    if (cycles & TME_BUS_CYCLE_WRITE) {

      /* lock our mutex: */
      tme_mutex_lock(&memory->tme_posix_memory_mutex);

      /* get the backing TLB entry: */
      tlb_token = tlb->tme_bus_tlb_token;

      /* hash the TLB entry into our writable TLB entry set: */
      _tlb_token
	= (((tme_hash_data_to_ulong(tlb_token)
	     / sizeof(struct tme_token))
	    % TME_MEMORY_POSIX_TLBS_SIZE)
	   + memory->tme_posix_memory_tlb_tokens);

      /* if there is a different TLB entry already at this position in
	 the writable TLB entry set: */
      tlb_token_other = *_tlb_token;
      if (__tme_predict_true(tlb_token_other != NULL)) {
	if (tlb_token_other != tlb_token) {

	  /* invalidate this other TLB entry: */
	  tme_token_invalidate(tlb_token_other);
	}
      }

      /* save the TLB entry into the writable TLB entry set: */
      *_tlb_token = tlb->tme_bus_tlb_token;

      /* loop over the valids bitmasks: */
      for (valids = memory->tme_posix_memory_valids;
	   valids != NULL;
	   valids = valids->tme_posix_memory_valids_next) {

	/* clear the bit for this address' page in this valids
	   bitmask: */
	page_index = (address >> valids->tme_posix_memory_valids_page_size_log2);
	*(valids->tme_posix_memory_valids_bitmask
	  + (page_index / 8))
	  &= ~(1 << (page_index % 8));
      }

      /* this TLB entry allows reading and writing: */
      tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;

      /* this TLB entry only covers the current TLB size: */
      tlb_size = memory->tme_posix_memory_tlb_size;
      address &= 0 - (unsigned long) tlb_size;
      tlb->tme_bus_tlb_addr_first = address;
      tlb->tme_bus_tlb_addr_last = TME_MIN(address | (tlb_size - 1), memory_address_last);

      /* unlock our mutex: */
      tme_mutex_unlock(&memory->tme_posix_memory_mutex);

      return (TME_OK);
    }

    /* this TLB entry only allows reading: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ;
    tlb->tme_bus_tlb_emulator_off_write = TME_EMULATOR_OFF_UNDEF;
  }

  /* otherwise, this device is not cacheable: */
  else {

    /* all memory devices allow reading and writing: */
    tlb->tme_bus_tlb_cycles_ok = TME_BUS_CYCLE_READ | TME_BUS_CYCLE_WRITE;
  }

  /* this TLB entry can cover the whole device: */
  tlb->tme_bus_tlb_addr_first = 0;
  tlb->tme_bus_tlb_addr_last = memory_address_last;

  return (TME_OK);
}

/* this function allocates a new valids bitmask: */
static tme_shared tme_uint8_t *
_tme_posix_memory_valids_new(void *_memory,
			     tme_uint32_t page_size_log2)
{
  struct tme_posix_memory *memory;
  tme_uint32_t page_size;
  unsigned long page_count;
  struct tme_posix_memory_valids *valids;

  /* recover our data structure: */
  memory = (struct tme_posix_memory *) _memory;

  /* lock our mutex: */
  tme_mutex_lock(&memory->tme_posix_memory_mutex);

  /* get the page size for this valids bitmask: */
  assert (page_size_log2 < (sizeof(page_size) * 8 - 1));
  page_size = 1;
  page_size <<= page_size_log2;

  /* update the current writable TLB size: */
  memory->tme_posix_memory_tlb_size
    = TME_MIN(memory->tme_posix_memory_tlb_size, page_size);

  /* get the page count for this valids bitmask: */
  page_count
    = ((memory->tme_posix_memory_cacheable.tme_bus_cacheable_size
	+ (page_size - 1))
       >> page_size_log2);

  /* allocate and initialize a new valids bitmask: */
  valids
    = ((struct tme_posix_memory_valids *)
       tme_malloc(sizeof(struct tme_posix_memory_valids)
		  + ((page_count + 7) / 8)));
  valids->tme_posix_memory_valids_page_size_log2 = page_size_log2;
  memset((tme_uint8_t *) valids->tme_posix_memory_valids_bitmask + 0,
	 0xff,
	 ((page_count + 7) / 8));

  /* add this new valids bitmask to the list: */
  valids->tme_posix_memory_valids_next = memory->tme_posix_memory_valids;
  memory->tme_posix_memory_valids = valids;

  /* invalidate all outstanding TLB entries: */
  _tme_posix_memory_tlbs_invalidate(memory);

  /* unlock our mutex: */
  tme_mutex_unlock(&memory->tme_posix_memory_mutex);

  /* return the bitmask: */
  return (valids->tme_posix_memory_valids_bitmask);
}

/* this function sets a bit in the valids bitmask: */
static void
_tme_posix_memory_valids_set(void *_memory,
			     tme_shared tme_uint8_t *valids_bitmask,
			     unsigned long page_index)
{
  struct tme_posix_memory *memory;

  /* recover our data structure: */
  memory = (struct tme_posix_memory *) _memory;

  /* lock our mutex: */
  tme_mutex_lock(&memory->tme_posix_memory_mutex);

  /* set the bit for this page in the valids bitmask: */
  *(valids_bitmask
    + (page_index / 8))
    |= TME_BIT(page_index % 8);

  /* invalidate all outstanding TLB entries: */
  _tme_posix_memory_tlbs_invalidate(memory);

  /* unlock our mutex: */
  tme_mutex_unlock(&memory->tme_posix_memory_mutex);
}

/* the new memory function: */
TME_ELEMENT_SUB_NEW_DECL(tme_host_posix,memory) {
  unsigned int memory_type;
  unsigned long memory_size;
  const char *filename;
  int fd;
  struct stat statbuf;
  struct tme_posix_memory *memory;
  struct tme_bus_cacheable *cacheable;
  ssize_t bytes_read;
  int arg_i;
  int usage;

  /* assume we have no backing file: */
  filename = NULL;
  memory_type = -1;
  memory_size = 0;
  arg_i = 1;
  usage = FALSE;

  /* we are regular RAM if our arguments are:

     ram SIZE

  */
  if (TME_ARG_IS(args[arg_i + 0], "ram")
      && (memory_size = tme_bus_addr_parse(args[arg_i + 1], 0)) > 0) {
    memory_type = TME_POSIX_MEMORY_RAM;
    arg_i += 2;
  }

  /* we are ROM if our arguments are:

     rom FILE
     
  */
  else if (TME_ARG_IS(args[arg_i + 0], "rom")
	   && (filename = args[arg_i + 1]) != NULL) {
    memory_type = TME_POSIX_MEMORY_ROM;
    arg_i += 2;
  }

  /* we are persistent storage if our arguments are:

     persistent FILE

  */
  else if (TME_ARG_IS(args[arg_i + 0], "persistent")
	   && (filename = args[arg_i + 1]) != NULL) {
    memory_type = TME_POSIX_MEMORY_PERSISTENT;
    arg_i += 2;
  }
	   
  else {
    usage = TRUE;
  }

  if (args[arg_i + 0] != NULL) {
    tme_output_append_error(_output,
			    "%s %s", 
			    args[arg_i],
			    _("unexpected"));
    usage = TRUE;
  }

  if (usage) {
    tme_output_append_error(_output,
			    "%s %s { rom %s | ram %s | persistent %s }",
			    _("usage:"),
			    args[0],
			    _("ROM-FILE"),
			    _("SIZE"),
			    _("PERSISTENT-FILE"));
    return (-1);
  }

  /* start the memory structure: */
  memory = tme_new0(struct tme_posix_memory, 1);
  memory->tme_posix_memory_type = memory_type;

  /* if we have a backing file: */
  fd = -1;
  if (filename != NULL) {

    /* open the file for reading: */
    fd = open(filename, (memory_type == TME_POSIX_MEMORY_ROM
			 ? O_RDONLY
			 : O_RDWR));
    if (fd < 0) {
      tme_output_append_error(_output,
			      "%s",
			      filename);
      tme_free(memory);
      return (errno);
    }

    /* stat the file: */
    if (fstat(fd, &statbuf) < 0) {
      tme_output_append_error(_output,
			      "%s",
			      filename);
      close(fd);
      tme_free(memory);
      return (errno);
    }
    memory_size = statbuf.st_size;
    if (memory_size == 0) {
      tme_output_append_error(_output,
			      "%s",
			      filename);
      close(fd);
      tme_free(memory);
      return (EINVAL);
    }

#ifdef HAVE_MMAP    
    /* try to mmap the file: */
    memory->tme_posix_memory_contents = 
      mmap(NULL, 
	   statbuf.st_size, 
	   PROT_READ
	   | (memory_type != TME_POSIX_MEMORY_ROM
	      ? PROT_WRITE
	      : 0),
	   MAP_SHARED,
	   fd,
	   0);
    if (memory->tme_posix_memory_contents != MAP_FAILED) {
      memory->tme_posix_memory_mapped = TRUE;
    }
#endif /* HAVE_MMAP */
  }

  /* if we have to, allocate memory space: */
  if (!memory->tme_posix_memory_mapped) {
    memory->tme_posix_memory_contents = tme_new0(tme_uint8_t, memory_size);

    /* if we have to, read in the backing file: */
    if (fd >= 0) {
      bytes_read = read(fd, memory->tme_posix_memory_contents, memory_size);
      if (bytes_read < 0
	  || memory_size != (unsigned long) bytes_read) {
	/* XXX diagnostic: */
	close(fd);
	tme_free(memory->tme_posix_memory_contents);
	tme_free(memory);
	return (-1);
      }

      /* if this is a ROM, we can close the file now: */
      if (memory_type == TME_POSIX_MEMORY_ROM) {
	close(fd);
	fd = -1;
      }
    }
  }

  /* remember any backing fd: */
  memory->tme_posix_memory_fd = fd;

  /* initialize our rwlock: */
  tme_rwlock_init(&memory->tme_posix_memory_rwlock);

  /* initialize our mutex: */
  tme_mutex_init(&memory->tme_posix_memory_mutex);

  /* assume that this memory won't be cacheable: */
  memory->tme_posix_memory_tlb_tokens = NULL;

  /* if we are regular RAM or ROM over a threshold size: */
  if ((memory_type == TME_POSIX_MEMORY_RAM
       && memory_size >= TME_MEMORY_POSIX_CACHEABLE_SIZE_RAM)
      || (memory_type == TME_POSIX_MEMORY_ROM
	  && memory_size >= TME_MEMORY_POSIX_CACHEABLE_SIZE_ROM)) {

    /* allocate the writable TLB entry set: */
    memory->tme_posix_memory_tlb_tokens
      = tme_new0(struct tme_token *,
		 TME_MEMORY_POSIX_TLBS_SIZE);

    /* initialize the valids list: */
    memory->tme_posix_memory_valids = NULL;

    /* initialize the current writable TLB size: */
    memory->tme_posix_memory_tlb_size = ((tme_uint32_t) 1) << 31;

    /* initialize the bus cacheable structure: */
    cacheable = &memory->tme_posix_memory_cacheable;
    cacheable->tme_bus_cacheable_contents = memory->tme_posix_memory_contents;
    cacheable->tme_bus_cacheable_size = memory_size;
    cacheable->tme_bus_cacheable_private = memory;
    cacheable->tme_bus_cacheable_valids_new = _tme_posix_memory_valids_new;
    cacheable->tme_bus_cacheable_valids_set = _tme_posix_memory_valids_set;
  }

  /* initialize our simple bus device descriptor: */
  memory->tme_posix_memory_device.tme_bus_device_tlb_fill = _tme_posix_memory_tlb_fill;
  memory->tme_posix_memory_device.tme_bus_device_address_last = (memory_size - 1);

  /* fill the element: */
  element->tme_element_private = memory;
  element->tme_element_connections_new = tme_bus_device_connections_new;

  return (0);
}
