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
      unsigned int count;
      
      /**
       * A lock for deleting the HeapObject to ensure it is thread safe.
       */
      ExclusiveLock deleteLock;
   };
   
   /**
    * A reference to a HeapObject.
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
   if(ptr != NULL)
   {
      // create reference to HeapObject
      mReference = new Reference;
      mReference->ptr = ptr;
      mReference->count = 1;
   }
   else
   {
      // no reference
      mReference = NULL;
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
   // set reference and increase count
   mReference = ref;
   if(mReference != NULL)
   {
      mReference->count++;
   }
}

template<typename HeapObject>
void Collectable<HeapObject>::release()
{
   // decrement reference count
   if(mReference != NULL)
   {
      mReference->deleteLock.lock();
      if(--mReference->count == 0)
      {
         mReference->deleteLock.unlock();
         
         // delete HeapObject and reference
         delete mReference->ptr;
         delete mReference;
         mReference = NULL;
      }
      else
      {
         mReference->deleteLock.unlock();
      }
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
   bool rval = false;
   
   if(this == &rhs)
   {
      rval = true;
   }
   else if(this->mReference == rhs.mReference)
   {
      rval = true;
   }
   
   return rval;
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
   // release old reference and acquire NULL one
   release();
   acquire(NULL);
}

template<typename HeapObject>
bool Collectable<HeapObject>::isNull()
{
   return mReference == NULL;
}

} // end namespace rt
} // end namespace db
#endif
