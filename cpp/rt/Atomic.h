/*
 * Copyright (c) 2009-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_Atomic_H
#define monarch_rt_Atomic_H

#include "monarch/util/Macros.h"
#include <cstdlib>
#include <inttypes.h>

#ifdef WIN32
#include <windows.h>
// FIXME: get the CPU address size and use it instead of assuming 32
#define ALIGN_BYTES 4
#define ALIGN_BITS  32
typedef int8_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_int8_t;
typedef int16_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_int16_t;
typedef int32_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_int32_t;
typedef uint8_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_uint8_t;
typedef uint16_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_uint16_t;
typedef uint32_t __attribute__ ((aligned(ALIGN_BYTES))) aligned_uint32_t;
#else
typedef int8_t aligned_int8_t;
typedef int16_t aligned_int16_t;
typedef int32_t aligned_int32_t;
typedef uint8_t aligned_uint8_t;
typedef uint16_t aligned_uint16_t;
typedef uint32_t aligned_uint32_t;
#endif

namespace monarch
{
namespace rt
{

/**
 * The Atomic class provides methods for doing atomic operations that are
 * supported by the system's CPU.
 *
 * @author Dave Longley
 */
class Atomic
{
public:
   /**
    * Allocates memory that is aligned such that it can be safely used in
    * special atomic operations. Certain operating systems (MS Windows)
    * require that memory that is used in operations such as Compare-And-Swap
    * must be aligned on boundaries that match the address size for the CPU.
    *
    * @param size the size of the memory to allocate.
    *
    * @return a pointer to the allocated memory.
    */
   static void* mallocAligned(size_t size);

   /**
    * Frees some previously allocated aligned-memory.
    *
    * @param ptr the pointer to the aligned-memory to free.
    */
   static void freeAligned(void* ptr);

   /**
    * Performs an atomic store into the given destination.
    *
    * @param dst the storage destination.
    * @param value the value to store.
    */
   template<typename T>
   static inline void store(volatile T* dst, T value);

   /**
    * Performs an atomic load from the given address.
    *
    * @param ptr the address with the value to load.
    *
    * @return the value.
    */
   template<typename T>
   static inline T load(volatile T* ptr);

   /**
    * Performs an atomic Increment-And-Fetch. Increments the value at
    * the given destination.
    *
    * @param dst the destination with the value to increment.
    *
    * @return the new value.
    */
   template<typename T>
   static inline T incrementAndFetch(volatile T* dst);

   /**
    * Performs an atomic Decrement-And-Fetch. Decrements the value at
    * the given destination.
    *
    * @param dst the destination with the value to decrement.
    *
    * @return the new value.
    */
   template<typename T>
   static inline T decrementAndFetch(volatile T* dst);

   /**
    * Performs an atomic Add-And-Fetch. Adds the value to to the destination.
    *
    * @param dst the destination of the addtion.
    * @param value the value to add to the destination.
    *
    * @return the new value.
    */
   template<typename T>
   static inline T addAndFetch(volatile T* dst, T value);

   /**
    * Performs an atomic Subtract-And-Fetch. Adds the value to to the
    * destination.
    *
    * @param dst the destination of the subtraction.
    * @param value the value to subtract from the destination.
    *
    * @return the new value.
    */
   template<typename T>
   static inline T subtractAndFetch(volatile T* dst, T value);

   /**
    * Performs an atomic Compare-And-Swap (CAS). The given new value will only
    * be written to the destination if it contains the given old value. If
    * the old value matches, the write will occur and the function returns
    * true. If the old value does not match, nothing will happen and the
    * function will return false.
    *
    * @param dst the destination to write the new value to.
    * @param oldVal the old value.
    * @param newVal the new value.
    *
    * @return true if successful, false if not.
    */
   template<typename T>
   static inline bool compareAndSwap(volatile T* dst, T oldVal, T newVal);
   template<typename T>
   static inline bool compareAndSwap(volatile T** dst, T* oldVal, T* newVal);
};

template<typename T>
void Atomic::store(volatile T* dst, T value)
{
   while(true)
   {
      T oldValue = *dst;
      if(Atomic::compareAndSwap(dst, oldValue, value))
      {
         break;
      }
   }
}

template<typename T>
T Atomic::load(volatile T* ptr)
{
   T rval;
   while (true)
   {
      rval = *ptr;
      if(Atomic::compareAndSwap(ptr, rval, rval))
      {
         break;
      }
   }
   return rval;
}

template<typename T>
T Atomic::incrementAndFetch(volatile T* dst)
{
#ifdef WIN32
   // NOTE: newer API has 16 bit functions
   MO_STATIC_ASSERT(sizeof(T) == 4 || sizeof(T) == 8);
   if(sizeof(T) == 4)
   {
      return InterlockedIncrement((LONG*)&dst);
   }
   else if(sizeof(T) == 8)
   {
      return InterlockedIncrement64((LONGLONG*)&dst);
   }
#else
   return __sync_add_and_fetch(dst, 1);
#endif
}

template<typename T>
T Atomic::decrementAndFetch(volatile T* dst)
{
#ifdef WIN32
   // NOTE: newer API has 16 bit functions
   MO_STATIC_ASSERT(sizeof(T) == 4 || sizeof(T) == 8);
   if(sizeof(T) == 4)
   {
      return InterlockedDecrement((LONG*)&dst);
   }
   else if(sizeof(T) == 8)
   {
      return InterlockedDecrement64((LONGLONG*)&dst);
   }
#else
   return __sync_sub_and_fetch(dst, 1);
#endif
}

template<typename T>
T Atomic::addAndFetch(volatile T* dst, T value)
{
#ifdef WIN32
   MO_STATIC_ASSERT(sizeof(T) == 4 || sizeof(T) == 8);
   if(sizeof(T) == 4)
   {
      return InterlockedAdd((LONG*)&dst, (LONG)value);
   }
   else if(sizeof(T) == 8)
   {
      return InterlockedAdd64((LONGLONG*)&dst, (LONG)value);
   }
#else
   return __sync_add_and_fetch(dst, value);
#endif
}

template<typename T>
T Atomic::subtractAndFetch(volatile T* dst, T value)
{
#ifdef WIN32
   MO_STATIC_ASSERT(sizeof(T) == 4 || sizeof(T) == 8);
   if(sizeof(T) == 4)
   {
      return InterlockedAdd((LONG*)&dst, -(LONG)value);
   }
   else if(sizeof(T) == 8)
   {
      return InterlockedDecrement64((LONGLONG*)&dst, -(LONG)value);
   }
#else
   return __sync_sub_and_fetch(dst, value);
#endif
}

template<typename T>
bool Atomic::compareAndSwap(volatile T* dst, T oldVal, T newVal)
{
#ifdef WIN32
   // NOTE: newer API has 16 and 64 bit functions
   MO_STATIC_ASSERT(sizeof(T) == 4);
   return (InterlockedCompareExchange(
      (LONG*)dst, (LONG)newVal, (LONG)oldVal) == (LONG)oldVal);
#else
   return __sync_bool_compare_and_swap(dst, oldVal, newVal);
#endif
}

template<typename T>
bool Atomic::compareAndSwap(volatile T** dst, T* oldVal, T* newVal)
{
#ifdef WIN32
   return (InterlockedCompareExchangePointer(*dst, newVal, oldVal) == oldVal);
#else
   return __sync_bool_compare_and_swap(dst, oldVal, newVal);
#endif
}

} // end namespace rt
} // end namespace monarch
#endif
