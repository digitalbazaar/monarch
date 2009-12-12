/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_HazardPtrList_H
#define monarch_rt_HazardPtrList_H

#include "monarch/rt/Atomic.h"

namespace monarch
{
namespace rt
{

/**
 * A hazard pointer is used to protect memory from being freed while it
 * is still in use.
 */
struct HazardPtr
{
   bool active;
   void* value;
   HazardPtr* next;
};

/**
 * The Atomic class provides methods for doing atomic operations that are
 * supported by the system's CPU.
 *
 * The list is intended to be used to protect memory in the following manner:
 *
 * Create a shared HazardPtrList X.
 *
 * Thread A: Acquires a hazard pointer H via acquire() from X.
 * Thread A: Sets H to value of a pointer P.
 * Thread B: Might do something to change the value of P, storing the old value
 *           of P so the memory located there can be freed.
 * Thread A: Ensures that the value of H is still the same as the value of P.
 *           If not, set H to the value of P again and repeat. Only once the
 *           values are the same should the memory be considered protected by
 *           this list (provided that Thread B checks this list before freeing
 *           any related memory).
 * Thread B: Before freeing the old value of P, make sure that that value is
 *           not in X via isProtected(). If it is, it is not permitted to
 *           be freed. If it is not, then it is safe to free.
 *
 * Other programming/garbage collection paradigms may be employed on top of
 * this HazardPtrList's protection such as reference counts. Atomically
 * incrementing a reference count after protecting the memory with this
 * HazardPtrList will allow garbage collector(s) to check a simple reference
 * count for 0 before having to scan the entire HazardPtrList. This may save
 * cycles. Do not forget, however, that once a reference count is found to
 * be 0, this list still has to be scanned before freeing the memory because
 * another thread may be in the middle of incrementing that reference count
 * back up to 1.
 *
 * Another advantage of this approach is that it helps minimize the number of
 * hazard pointers required to do certain operations. If a hazard pointer is
 * only needed to protect memory while its reference count is being incremented,
 * then each thread should only ever need to acquire 1 hazard pointer.
 *
 * @author Dave Longley
 */
class HazardPtrList
{
public:
   /**
    * The head of this list.
    */
#ifdef WIN32
   /* MS Windows requires any variable written to in an atomic operation
      to be aligned to the address size of the CPU. */
   volatile HazardPtr* mHead __attribute__ ((aligned(ALIGN_BYTES)));
#else
   volatile HazardPtr* mHead;
#endif

public:
   /**
    * Creates a new, empty HazardPtrList.
    */
   HazardPtrList();

   /**
    * Destructs a HazardPtrList.
    */
   ~HazardPtrList();

   /**
    * Acquires a hazard pointer for use. Only the "ptr" member of the hazard
    * pointer can be altered. Any other alteration is prohibited.
    *
    * @return the hazard pointer to use.
    */
   HazardPtr* acquire();

   /**
    * Releases a hazard pointer back to this list.
    *
    * @param ptr the pointer to release.
    *
    * @returns the hazard pointer back to this list.
    */
   void release(HazardPtr* ptr);

   /**
    * Checks the hazard pointer list to see if any HazardPtrs are set to
    * the given address. This method will determine if the memory at the
    * given address is marked as being in use by a HazardPtr in this list
    * and should be protected from being freed.
    *
    * @param addr the address to look for.
    *
    * @return true if at least one HazardPtr was set to the given address,
    *         false if not.
    */
   bool isProtected(void* addr);
};

} // end namespace rt
} // end namespace monarch
#endif
