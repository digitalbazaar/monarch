/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Collectable_H
#define db_rt_Collectable_H

#include "db/rt/ExclusiveLock.h"

namespace db
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
 * Collectables are *not* thread-safe by design (for speed). If two threads
 * need to modify a HeapObject at the same time, then a new Collectable
 * should be created and passed to one of the two threads.
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
      volatile unsigned int count;
   };
   
   /**
    * A reference to a HeapObject. When the HeapObject is NULL, this
    * reference is private to this Collectable and should not be deleted.
    */
   Reference* mReference;
   
   /**
    * A private NULL reference.
    */
   Reference mNullReference;
   
   /**
    * Acquires the passed Reference.
    * 
    * @param ref the Reference to acquire.
    */
   virtual void acquire(Reference* ref);
   
   /**
    * Releases the current Reference.
    */
   virtual void release();
   
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
   virtual bool operator==(const Collectable& rhs);
   
   /**
    * Compares this Collectable against another one for inequality.
    * 
    * @param rhs the Collectable to compare this one against.
    * 
    * @return true if this Collectable is not equal the another one, false if not.
    */
   virtual bool operator!=(const Collectable& rhs);
   
   /**
    * Returns a reference to this Collectable's HeapObject.
    * 
    * @return a reference to this Collectable's HeapObject.
    */
   virtual HeapObject& operator*();
   
   /**
    * Returns a pointer to this Collectable's HeapObject.
    * 
    * @return a pointer to this Collectable's HeapObject.
    */
   virtual HeapObject* operator->();
   
   /**
    * Sets this Collectable's HeapObject to NULL.
    */
   virtual void setNull();
   
   /**
    * Returns true if this Collectable's HeapObject is NULL, false if not.
    * 
    * @return true if this Collectable's HeapObject is NULL, false if not.
    */
   virtual bool isNull();
};

template<typename HeapObject>
Collectable<HeapObject>::Collectable(HeapObject* ptr)
{
   // initialize private NULL reference
   mNullReference.ptr = NULL;
   
   if(ptr == NULL)
   {
      // use private NULL reference
      mReference = &mNullReference;
   }
   else
   {
      // create a reference to the HeapObject
      mReference = new Reference;
      mReference->ptr = ptr;
      mReference->count = 1;
   }
}

template<typename HeapObject>
Collectable<HeapObject>::Collectable(const Collectable& copy)
{
   // initialize private NULL reference
   mNullReference.ptr = NULL;
   
   // acquire copy's reference
   acquire(copy.mReference);
}

template<typename HeapObject>
Collectable<HeapObject>::~Collectable()
{
   // release reference
   release();
}

template<typename HeapObject>
void Collectable<HeapObject>::acquire(Reference* ref)
{
   if(ref->ptr != NULL)
   {
      // do atomic increment and fetch, set new reference
      __sync_add_and_fetch(&ref->count, 1);
      mReference = ref;
   }
   else
   {
      // use private NULL reference
      mReference = &mNullReference;
   }
}

template<typename HeapObject>
void Collectable<HeapObject>::release()
{
   // old reference only needs to be released if it is
   // shared (it is not NULL)
   if(mReference != &mNullReference)
   {
      // do atomic fetch and decrement
      unsigned int count = __sync_sub_and_fetch(&mReference->count, 1);
      
      // this Collectable is responsible for deleting the reference
      // if it was the last one
      if(count == 0)
      {
         delete mReference->ptr;
         delete mReference;
      }
      
      // use private NULL reference
      mReference = &mNullReference;
   }
}

template<typename HeapObject>
Collectable<HeapObject>& Collectable<HeapObject>::operator=(
   const Collectable& rhs)
{
   if(this != &rhs)
   {
      // release old reference and acquire new one
      release();
      acquire(rhs.mReference);
   }
   
   return *this;
}

template<typename HeapObject>
bool Collectable<HeapObject>::operator==(const Collectable& rhs)
{
   return
      (this == &rhs) ||
      (this->mReference->ptr == rhs.mReference->ptr);
}

template<typename HeapObject>
bool Collectable<HeapObject>::operator!=(const Collectable& rhs)
{
   return !(*this == rhs);
}

template<typename HeapObject>
HeapObject& Collectable<HeapObject>::operator*()
{
   return *mReference->ptr;
}

template<typename HeapObject>
HeapObject* Collectable<HeapObject>::operator->()
{
   return mReference->ptr;
}

template<typename HeapObject>
void Collectable<HeapObject>::setNull()
{
   // release old reference
   release();
}

template<typename HeapObject>
bool Collectable<HeapObject>::isNull()
{
   return mReference == &mNullReference;
}

} // end namespace rt
} // end namespace db
#endif
