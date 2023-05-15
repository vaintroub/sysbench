
/*
* This is Windows port of bits and pieces of Concurrency Kit that is used by sysbench.#
* (memory intrinsics). ring queue was taken literally from ck_ring.h
*/

/*
 * Copyright 2009-2015 Samy Al Bahra.
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include <windows.h>
#include <intrin.h>
#include "pthread.h" /* spinlock*/
#include <stdbool.h> /* bool */
#include <stdint.h> /* uint32_t */

#define CK_MD_CACHELINE 64
#define CK_CC_CACHELINE __declspec(align(CK_MD_CACHELINE))
#define CK_CC_UNLIKELY
#define CK_CC_LIKELY
#define CK_CC_INLINE inline
#define CK_CC_FORCE_INLINE __forceinline
#define CK_CC_RESTRICT __restrict



#define ck_spinlock_t pthread_spinlock_t
#define ck_spinlock_init(x) pthread_spin_init(x, PTHREAD_PROCESS_PRIVATE)
#define ck_spinlock_lock pthread_spin_lock
#define ck_spinlock_unlock pthread_spin_unlock

static void ck_pr_barrier()
{
  _ReadWriteBarrier();
}
static void ck_pr_fence_load()
{
  MemoryBarrier();
}
static void ck_pr_dec_int(int* target)
{
  _InterlockedDecrement(target);
}

static void ck_pr_dec_uint(unsigned int* target)
{
  _InterlockedDecrement(target);
}

static void ck_pr_inc_int(int* target)
{
  _InterlockedIncrement(target);
}
static void ck_pr_store_int(int* target, int value)
{
  *target = value;
  _WriteBarrier();
}
static void ck_pr_store_uint(unsigned int* target, unsigned int value)
{
  *target = value;
  _WriteBarrier();
}
static uint32_t ck_pr_faa_uint(unsigned int* target, unsigned int delta)
{
	return InterlockedExchangeAdd(target, delta);
}
static uint32_t ck_pr_faa_32(uint32_t * target, uint32_t delta)
{
  return InterlockedExchangeAdd(target, delta);
}
static uint64_t ck_pr_faa_64(uint64_t* target, uint64_t delta)
{
	return InterlockedExchangeAdd64(target, delta);
}
static uint64_t ck_pr_load_64(uint64_t* target)
{
#ifdef _WIN64
  _ReadBarrier();
  return *target;
#else
  return InterlockedXor64(target, 0);
#endif
}
static void ck_pr_store_64(uint64_t* target, uint64_t value)
{
  *target = value;
}
static void ck_pr_fence_store()
{
  MemoryBarrier();
}
static uint64_t ck_pr_fas_64(uint64_t* target, uint64_t value)
{
  return _InterlockedExchange64(target, value);
}
static uint64_t ck_pr_inc_64(uint64_t* target)
{
  return _InterlockedIncrement64(target);
}

static uint8_t ck_pr_load_8(uint8_t* target)
{
  return InterlockedXor8(target,0);
}
static void ck_pr_store_8(uint8_t* target, uint8_t value)
{
  InterlockedExchange8(target,value);
}
static int ck_pr_load_int(int* target)
{
  _ReadBarrier();
  return *target;
}
static unsigned int ck_pr_load_uint(unsigned int* target)
{
  _ReadBarrier();
  return *target;
}
static void ck_pr_fence_store_atomic()
{
  MemoryBarrier();
}
static inline bool
ck_pr_cas_uint_value(unsigned int* target, unsigned int old_value, unsigned int new_value, unsigned int* original_value)
{
  *original_value = InterlockedCompareExchange(target, new_value, old_value);
  return original_value == old_value;
}
/*
 * Concurrent ring buffer.
 */

struct ck_ring {
  unsigned int c_head;
  char pad[CK_MD_CACHELINE - sizeof(unsigned int)];
  unsigned int p_tail;
  unsigned int p_head;
  char _pad[CK_MD_CACHELINE - sizeof(unsigned int) * 2];
  unsigned int size;
  unsigned int mask;
};
typedef struct ck_ring ck_ring_t;

struct ck_ring_buffer {
  void* value;
};
typedef struct ck_ring_buffer ck_ring_buffer_t;

CK_CC_INLINE static unsigned int
ck_ring_size(const struct ck_ring* ring)
{
  unsigned int c, p;

  c = ck_pr_load_uint(&ring->c_head);
  p = ck_pr_load_uint(&ring->p_tail);
  return (p - c) & ring->mask;
}
CK_CC_INLINE static unsigned int
ck_ring_capacity(const struct ck_ring* ring)
{
  return ring->size;
}

CK_CC_INLINE static void
ck_ring_init(struct ck_ring* ring, unsigned int size)
{

  ring->size = size;
  ring->mask = size - 1;
  ring->p_tail = 0;
  ring->p_head = 0;
  ring->c_head = 0;
  return;
}

/*
 * The _ck_ring_* namespace is internal only and must not used externally.
 */
CK_CC_FORCE_INLINE static bool
_ck_ring_enqueue_sp(struct ck_ring* ring,
  void* CK_CC_RESTRICT buffer,
  const void* CK_CC_RESTRICT entry,
  unsigned int ts,
  unsigned int* size)
{
  const unsigned int mask = ring->mask;
  unsigned int consumer, producer, delta;

  consumer = ck_pr_load_uint(&ring->c_head);
  producer = ring->p_tail;
  delta = producer + 1;
  if (size != NULL)
    *size = (producer - consumer) & mask;

  if (CK_CC_UNLIKELY((delta & mask) == (consumer & mask)))
    return false;

  buffer = (char*)buffer + ts * (producer & mask);
  memcpy(buffer, entry, ts);

  /*
   * Make sure to update slot value before indicating
   * that the slot is available for consumption.
   */
  ck_pr_fence_store();
  ck_pr_store_uint(&ring->p_tail, delta);
  return true;
}
CK_CC_INLINE static bool
ck_ring_enqueue_spmc(struct ck_ring* ring,
  struct ck_ring_buffer* buffer,
  const void* entry)
{

  return _ck_ring_enqueue_sp(ring, buffer, &entry,
    sizeof(entry), NULL);
}

CK_CC_FORCE_INLINE static bool
_ck_ring_dequeue_mc(struct ck_ring* ring,
  const void* buffer,
  void* data,
  unsigned int ts)
{
  const unsigned int mask = ring->mask;
  unsigned int consumer, producer;

  consumer = ck_pr_load_uint(&ring->c_head);

  do {
    const char* target;

    /*
     * Producer counter must represent state relative to
     * our latest consumer snapshot.
     */
    ck_pr_fence_load();
    producer = ck_pr_load_uint(&ring->p_tail);

    if (CK_CC_UNLIKELY(consumer == producer))
      return false;

    ck_pr_fence_load();

    target = (const char*)buffer + ts * (consumer & mask);
    memcpy(data, target, ts);

    /* Serialize load with respect to head update. */
    ck_pr_fence_store_atomic();
  } while (ck_pr_cas_uint_value(&ring->c_head,
    consumer,
    consumer + 1,
    &consumer) == false);

  return true;
}

CK_CC_INLINE static bool
ck_ring_dequeue_spmc(struct ck_ring* ring,
  const struct ck_ring_buffer* buffer,
  void* data)
{
  return _ck_ring_dequeue_mc(ring, buffer, (void**)data, sizeof(void*));
}