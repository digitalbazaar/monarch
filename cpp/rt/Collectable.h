/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_Collectable_H
#define monarch_rt_Collectable_H

#include "monarch/rt/Atomic.h"

namespace monarch
{
namespace rt
{

/**
 * A Collectable is a reference counter for heap-allocated objects. When the
 * number of references to a particular heap-allocated object reaches zero,
 * that object will be garbage-collected.
 *
 * A Collectable makes no attempt to solve any circular-reference problems. If
 * multiple Collectables contain references to each other it is possible that
 * their HeapObjects will never be garbage-collected.
 *
 * Collectables are *not* thread-safe by design (for speed). We use lock-free
 * algorithms here to handle reference counting, and given that a DCAS
 * (Double Compare And Swap) algorithm is not available in GCC, we cannot
 * easily make use of that operation to atomically create references and
 * increment their counts.
 *
 * Therefore, in the given scenario:
 *
 * Thread A has the only reference R1 to object X.
 * Thread B has a reference R2 to some object other than X or to NULL.
 *
 * If Thread B tries to set R2 to R1's value whilst Thread A is setting R1 to
 * NULL, then it is possible that:
 *
 * Thread B sees that R1 points to X.
 * Thread A preempts and sets R1 to NULL, thereby collecting X.
 * Thread B preempts and tries to increment X's reference count.
 * Violation.
 *
 * Note: It should be safe to enter a similar situation where both R1 and R2
 * reference the same object, however, if this can be avoided it should be.
 *
 * In order to avoid situations like this, the programmer is expected to always
 * maintain enough references to HeapObjects that are being modified in more
 * than one thread. Also, if a HeapObject needs to be modified concurrently,
 * then a new Collectable must be created for each thread that needs to
 * modify it. If the same Collectable is modified concurrently, the results
 * are undefined.
 *
 * @author Dave Longley
 */
template<typename HeapObject>
class Collectable
{
protected:
   /**
    * The definition for a reference a HeapObject.
    */
   struct Reference
   {
      /**
       * A pointer to a HeapObject.
       */
      HeapObject* ptr;

      /**
       * A reference count for HeapObject.
       */
      volatile aligned_int32_t count;

      /**
       * True if ownership over the HeapObject's memory has been
       * relinquished and, therefore, it should not be deleted.
       */
      volatile bool relinquished;
   };

   /**
    * A reference to a HeapObject. When the HeapObject is NULL, this
    * reference is NULL.
    */
   volatile Reference* mReference;

public:
   /**
    * Creates a new Collectable that points to the given HeapObject.
    *
    * @param ptr the HeapObject to point at.
    */
   Collectable(HeapObject* ptr = NULL);

   /**
    * Creates a new Collectable by copying an existing one.
    *
    * @param copy the Collectable to copy.
    */
   Collectable(const Collectable& copy);

   /**
    * Destructs this Collectable.
    */
   virtual ~Collectable();

   /**
    * Sets this Collectable equal to another one.
    *
    * @param rhs the Collectable to set this one equal to.
    *
    * @return a reference to this Collectable.
    */
   virtual Collectable& operator=(const Collectable& rhs);

   /**
    * Compares this Collectable against another one for equality. If the
    * passed Collectable has a reference to the same HeapObject as this
    * one, then they are equal.
    *
    * @param rhs the Collectable to compare this one against.
    *
    * @return true if this Collectable is equal the another one, false if not.
    */
   virtual bool operator==(const Collectable& rhs) const;

   /**
    * Compares this Collectable against another one for inequality.
    *
    * @param rhs the Collectable to compare this one against.
    *
    * @return true if this Collectable is not equal the another one,
    *         false if not.
    */
   virtual bool operator!=(const Collectable& rhs) const;

   /**
    * Returns a reference to this Collectable's HeapObject.
    *
    * @return a reference to this Collectable's HeapObject.
    */
   virtual HeapObject& operator*() const;

   /**
    * Returns a pointer to this Collectable's HeapObject.
    *
    * @return a pointer to this Collectable's HeapObject.
    */
   virtual HeapObject* operator->() const;

   /**
    * Sets this Collectable's HeapObject to NULL.
    */
   virtual void setNull();

   /**
    * Returns true if this Collectable's HeapObject is NULL, false if not.
    *
    * @return true if this Collectable's HeapObject is NULL, false if not.
    */
   virtual bool isNull() const;

   /**
    * Relinquishes memory ownership over this Collectable's HeapObject. This
    * Collectable and all others that were referencing the same HeapObject will
    * no longer be responsible for deleting the HeapObject.
    *
    * More Collectables may be created from this one (ie via the equals
    * operator) and point at the HeapObject, however, when the reference count
    * for the HeapObject eventually reaches zero, its memory will not be
    * collected by any Collectable. Memory reclamation must be done manually,
    * or a brand new Collectable must be manually constructed to wrap the
    * HeapObject again.
    *
    * @return the relinquished HeapObject.
    */
   virtual HeapObject* relinquish();

protected:
   /**
    * Acquires the passed Reference.
    *
    * @param ref the Reference to acquire.
    */
   virtual void acquire(volatile Reference* ref);

   /**
    * Releases the passed Reference.
    *
    * @param ref the Reference to release.
    */
   virtual void release(volatile Reference* ref);
};

template<typename HeapObject>
Collectable<HeapObject>::Collectable(HeapObject* ptr)
{
   if(ptr == NULL)
   {
      mReference = NULL;
   }
   else
   {
      // create a reference to the HeapObject
      mReference = new Reference;
      mReference->ptr = ptr;
      mReference->count = 1;
      mReference->relinquished = false;
   }
}

template<typename HeapObject>
Collectable<HeapObject>::Collectable(const Collectable& copy)
{
   // acquire copy's reference
   acquire(copy.mReference);
}

template<typename HeapObject>
Collectable<HeapObject>::~Collectable()
{
   // release reference
   setNull();
}

template<typename HeapObject>
Collectable<HeapObject>& Collectable<HeapObject>::operator=(
   const Collectable& rhs)
{
   if(this != &rhs)
   {
      // acquire new reference, release old reference
      volatile Reference* ref = mReference;
      acquire(rhs.mReference);
      release(ref);
   }

   return *this;
}

template<typename HeapObject>
bool Collectable<HeapObject>::operator==(const Collectable& rhs) const
{
   volatile Reference* lRef = mReference;
   volatile Reference* rRef = rhs.mReference;
   return
      (this == &rhs) ||
      (lRef == rRef) ||
      (lRef != NULL && rRef != NULL && lRef->ptr == rRef->ptr);
}

template<typename HeapObject>
bool Collectable<HeapObject>::operator!=(const Collectable& rhs) const
{
   return !(*this == rhs);
}

template<typename HeapObject>
HeapObject& Collectable<HeapObject>::operator*() const
{
   return *mReference->ptr;
}

template<typename HeapObject>
HeapObject* Collectable<HeapObject>::operator->() const
{
   return mReference->ptr;
}

template<typename HeapObject>
void Collectable<HeapObject>::setNull()
{
   // release old reference
   volatile Reference* ref = mReference;
   mReference = NULL;
   release(ref);
}

template<typename HeapObject>
bool Collectable<HeapObject>::isNull() const
{
   return mReference == NULL;
}

template<typename HeapObject>
HeapObject* Collectable<HeapObject>::relinquish()
{
   HeapObject* rval = NULL;

   volatile Reference* ref = mReference;
   if(ref != NULL)
   {
      // set relinquished flag
      ref->relinquished = true;
      rval = ref->ptr;
   }

   return rval;
}

template<typename HeapObject>
void Collectable<HeapObject>::acquire(volatile Reference* ref)
{
   if(ref != NULL)
   {
      // do atomic increment and fetch
      Atomic::incrementAndFetch(&ref->count);
   }

   mReference = ref;
}

template<typename HeapObject>
void Collectable<HeapObject>::release(volatile Reference* ref)
{
   // old reference only needs to be released if it is not NULL
   if(ref != NULL)
   {
      // do atomic fetch and decrement, test return value
      if(Atomic::decrementAndFetch(&ref->count) == 0)
      {
         // this Collectable is responsible for deleting the HeapObject if
         // it was the last one and memory ownership was not relinquished
         if(!ref->relinquished)
         {
            delete ref->ptr;
         }

         // this Collectable is responsible for deleting the reference
         // if it was the last one
         delete ref;
      }
   }
}

} // end namespace rt
} // end namespace monarch
#endif
