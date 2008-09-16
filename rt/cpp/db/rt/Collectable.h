/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Collectable_H
#define db_rt_Collectable_H

#include <cstddef>

// FIXME: remove ifdef once atomic functions are implemented in mingw
#ifdef WIN32
#include <windows.h>
#endif

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
       * 
       * FIXME: The compiler attribute is provided to ensure the count is
       * 32-bit aligned so that it doesn't break fragile windows atomic
       * code. This can be removed once mingw32 is updated with atomic
       * add/fetch and sub/fetch functions.
       */
#ifdef WIN32
      volatile unsigned int count __attribute__ ((aligned (4)));
#else
      volatile unsigned int count;
#endif
   };
   
   /**
    * A reference to a HeapObject. When the HeapObject is NULL, this
    * reference is NULL.
    */
   Reference* mReference;
   
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
   release();
}

template<typename HeapObject>
void Collectable<HeapObject>::acquire(Reference* ref)
{
   if(ref != NULL)
   {
      // do atomic increment and fetch
      // FIXME: remove ifdef once __sync_add_and_fetch is implemented in mingw
#ifdef WIN32
      InterlockedIncrement((long int*)&ref->count);
#else
      __sync_add_and_fetch(&ref->count, 1);
#endif
   }
   
   mReference = ref;
}

template<typename HeapObject>
void Collectable<HeapObject>::release()
{
   // old reference only needs to be released if it is not NULL
   if(mReference != NULL)
   {
      // do atomic fetch and decrement, test return value
      // FIXME: remove ifdef once __sync_sub_and_fetch is implemented in mingw
#ifdef WIN32
      if(InterlockedDecrement((long int*)&mReference->count) == 0)
#else
      if(__sync_sub_and_fetch(&mReference->count, 1) == 0)
#endif
      {
         // this Collectable is responsible for deleting the reference
         // if it was the last one
         delete mReference->ptr;
         delete mReference;
      }
      
      mReference = NULL;
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
bool Collectable<HeapObject>::operator==(const Collectable& rhs) const
{
   return
      (this == &rhs) ||
      (this->mReference == rhs.mReference) ||
      (this->mReference != NULL && rhs.mReference != NULL &&
       this->mReference->ptr == rhs.mReference->ptr);
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
   release();
}

template<typename HeapObject>
bool Collectable<HeapObject>::isNull() const
{
   return mReference == NULL;
}

} // end namespace rt
} // end namespace db
#endif
