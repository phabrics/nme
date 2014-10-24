/* $Id: memory.h,v 1.5 2010/06/05 19:35:38 fredette Exp $ */

/* tme/memory.h - header file for memory functions: */

/*
 * Copyright (c) 2005 Matt Fredette
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

#ifndef _TME_MEMORY_H
#define _TME_MEMORY_H

#include <tme/common.h>
_TME_RCSID("$Id: memory.h,v 1.5 2010/06/05 19:35:38 fredette Exp $");

/* includes: */
#include <tme/threads.h>

/* macros: */

/* pointers to memory that may be shared between threads must be
   qualified with tme_shared, which is normally defined to be
   volatile, even when threads are cooperative.

   if threads are cooperative and TME_NO_AUDIT_ATOMICS is defined,
   tme_shared is defined to be empty: */
#define tme_shared _tme_volatile
#if TME_THREADS_COOPERATIVE
#ifdef TME_NO_AUDIT_ATOMICS
#undef tme_shared
#define tme_shared /**/
#endif /* TME_NO_AUDIT_ATOMICS */
#endif /* TME_THREADS_COOPERATIVE */

/* many of the memory macros do pointer casts, and pointer casts can
   silently discard qualifiers in the original type.  to detect this
   at compile time, we pass a pointer being cast through a function
   that takes an argument of type void * and only the qualifiers being
   preserved in the cast: */
static _tme_inline void *
_tme_audit_pointer(void *pointer)
{
  return (pointer);
}
static _tme_inline tme_shared void *
_tme_audit_pointer_shared(tme_shared void *pointer)
{
  return (pointer);
}
static _tme_inline _tme_const void *
_tme_audit_pointer_const(_tme_const void *pointer)
{
  return (pointer);
}
#define _tme_cast_pointer(type_cast, type_expected, e)	\
  ((type_cast) (e) + (0 && _tme_audit_pointer(_tme_audit_type(e, type_expected))))
#define _tme_cast_pointer_shared(type_cast, type_expected, e)	\
  ((tme_shared type_cast) (e) + (0 && _tme_audit_pointer_shared(_tme_audit_type(e, type_expected))))
#define _tme_cast_pointer_const(type_cast, type_expected, e)	\
  ((_tme_const type_cast) (e) + (0 && _tme_audit_pointer_const(_tme_audit_type(e, type_expected))))

/* NB: by default, we use tme_uint8_t for the atomic flag type and
   just pass the NULL rwlock pointer to tme_memory_atomic_read8() and
   tme_memory_atomic_write8().  when threads are not cooperative and
   the host CPU needs synchronization for atomic reads and writes of
   tme_uint8_t, the host CPU-specific memory header file must override
   this default: */
#define tme_memory_atomic_flag_t tme_shared tme_uint8_t
#define tme_memory_atomic_read_flag(flag)				\
  tme_memory_atomic_read8(flag, (tme_rwlock_t *) 0, sizeof(tme_memory_atomic_flag_t))
#define tme_memory_atomic_write_flag(flag, x)				\
  tme_memory_atomic_write8(flag, x, (tme_rwlock_t *) 0, sizeof(tme_memory_atomic_flag_t))
#define tme_memory_atomic_init_flag(flag, x)				\
  tme_memory_atomic_write_flag(flag, x)

/* the default 8-bit memory access macros: */
#define tme_memory_read8(mem, align_min)				\
  ((*_tme_cast_pointer_const(tme_uint8_t *, tme_uint8_t *, mem)) + (0 && (align_min)))
#define tme_memory_write8(mem, x, align_min)				\
  do {									\
    (*_tme_cast_pointer(tme_uint8_t *, tme_uint8_t *, mem))		\
      = (x) + (0 && (align_min));					\
  } while (/* CONSTCOND */ 0)
#define tme_memory_atomic_read8(mem, lock, align_min)			\
  ((*_tme_audit_type(mem, tme_uint8_t *)) + (0 && (lock) && (align_min)))
#define tme_memory_atomic_write8(mem, x, lock, align_min)		\
  do {									\
    (*_tme_audit_type(mem, tme_uint8_t *))				\
       = (x) + (0 && (lock) && (align_min));				\
  } while (/* CONSTCOND */ 0)
#define tme_memory_bus_read8(mem, lock, align_min, bus_boundary)	\
  tme_memory_atomic_read8(mem, lock, (align_min) + (0 && (bus_boundary)))
#define tme_memory_bus_write8(mem, x, lock, align_min, bus_boundary)	\
  tme_memory_atomic_write8(mem, x, lock, (align_min) + (0 && (bus_boundary)))

/* include the automatically generated header: */
#ifdef _TME_IMPL
#include <libtme/memory-auto.h>
#else
#include <tme/memory-auto.h>
#endif

#if !TME_THREADS_COOPERATIVE

/* include the host CPU-specific memory header file.

   this header file must define TME_MEMORY_ALIGNMENT_ATOMIC(type),
   which evaluates to zero if an object of the given type can never be
   accessed atomically no matter what the alignment, otherwise it
   evaluates to the minimum alignment needed to guarantee an atomic
   access.

   this header file must define TME_MEMORY_BUS_BOUNDARY, which is the
   host CPU's bus boundary.  for most CPUs, this will be the size of
   the largest type for which TME_MEMORY_ALIGNMENT_ATOMIC(type)
   returns nonzero.  if this evaluates to zero, all bus accesses will
   be emulated as atomic accesses.

   this header file may define TME_MEMORY_ALIGNMENT_ACCEPT(type),
   which evaluates to the smallest alignment for the given type that
   is acceptable for partial accesses.  this controls when it is
   cheaper to stop testing an object's alignment and just dereference
   its parts.

   this header file should also provide as many of the atomic
   operations as possible, as macros.  any operations not provided as
   macros are performed by the default macro implementations or the
   function implementations in memory-auto.c.  it is also possible for
   a CPU-specific macro to still call the default function
   implementation, if, for example, the host CPU can't do an operation
   atomically at a certain requested alignment.

   if TME_MEMORY_ALIGNMENT_ATOMIC(TME_MEMORY_TYPE_COMMON) evaluates to
   zero, all atomic operations will be handled by the default function
   implementations in memory-auto.c, and those functions will do the
   atomic operations under software lock.  TME_MEMORY_TYPE_COMMON is
   the dominant type used by all compiled emulated elements.

   if TME_MEMORY_ALIGNMENT_ATOMIC(TME_MEMORY_TYPE_COMMON) evaluates to
   nonzero, all atomic operations that can be done atomically by the
   host CPU, are done atomically (either by host CPU-specific macros
   or the default macro implementations), and all other atomic
   operations are handled by the default function implementations,
   which, instead of doing the atomic operations under software lock,
   do them with all other threads suspended.

   for best performance, the host CPU-specific memory header file
   should define TME_MEMORY_ALIGNMENT_ATOMIC to cover as many types as
   possible, provide the various tme_memory_atomic_cx* operations,
   and, if the host CPU can do any misaligned atomic reads and writes,
   the the tme_memory_atomic_read* and tme_memory_atomic_write*
   operations.

   regardless of TME_MEMORY_ALIGNMENT_ATOMIC(), if reads and writes of
   properly aligned pointers are not guaranteed to be atomic, this
   header file must also define tme_memory_atomic_pointer_read() and
   tme_memory_atomic_pointer_write() as macros.  (pointers are handled
   specially because the size of a pointer may not be a power of
   two.): */

#include <tme-memory.h>

#else  /* TME_THREADS_COOPERATIVE */

/* we don't know if this CPU can do atomic reads and writes at all.
   it doesn't matter, since threads are cooperative: */
#define TME_MEMORY_ALIGNMENT_ATOMIC(type)	(0)
#define TME_MEMORY_BUS_BOUNDARY			(sizeof(tme_uint8_t))

/* the memory barrier function: */
#define TME_MEMORY_BARRIER_READ_BEFORE_READ	(0)
#define TME_MEMORY_BARRIER_WRITE_BEFORE_WRITE	(0)
#define TME_MEMORY_BARRIER_READ_BEFORE_WRITE	(0)
#define TME_MEMORY_BARRIER_WRITE_BEFORE_READ	(0)
#define tme_memory_barrier(address, size, barrier) do { } while (/* CONSTCOND */ 0 && ((address) + 1) && (size) && (barrier))

#endif /* TME_THREADS_COOPERATIVE */

/* if an acceptable-alignment macro is not provided, we assume that it
   is cheaper to always access an object that may be only
   half-size-aligned as two half-size-aligned halves, rather than test
   the object's address for size-alignment and branch: */
#ifndef TME_MEMORY_ALIGNMENT_ACCEPT
#define TME_MEMORY_ALIGNMENT_ACCEPT(type)	(sizeof(type) / 2)
#endif /* !defined(TME_MEMORY_ALIGNMENT_ACCEPT) */

/* if the atomic pointer read and write operations are not provided,
   there is either no multiprocessing, or normal reads and writes of
   properly aligned pointers are guaranteed to be atomic: */
#ifndef tme_memory_atomic_pointer_read
#define tme_memory_atomic_pointer_read(type, lvalue, lock)	\
  (_tme_audit_type(lvalue, type) + (0 && _tme_audit_type(lock, tme_rwlock_t *)))
#endif /* !tme_memory_atomic_pointer_read */
#ifndef tme_memory_atomic_pointer_write
#define tme_memory_atomic_pointer_write(type, lvalue, x, lock) \
  ((*_tme_audit_type(&(lvalue), type *)) = (x) + (0 && _tme_audit_type(lock, tme_rwlock_t *)))
#endif /* !tme_memory_atomic_pointer_write */

#endif /* !_TME_MEMORY_H */
