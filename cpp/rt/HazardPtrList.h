/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_HazardPtrList_H
#define db_rt_HazardPtrList_H

#include "db/rt/Atomic.h"

namespace db
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
   void* ptr;
   HazardPtr* next;
};

/**
 * The Atomic class provides methods for doing atomic operations that are
 * supported by the system's CPU.
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
    * Gets the first hazard pointer in this list. This is most commonly
    * used to iterate over the list to check for existing hazard pointers.
    *
    * @return the first hazard pointer in this list.
    */
   const HazardPtr* first();
};

} // end namespace rt
} // end namespace db
#endif
