/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_DynamicObjectIterator_H
#define db_util_DynamicObjectIterator_H

#include "db/util/Iterator.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace util
{

/**
 * A DynamicObjectIteratorImpl is the implementation for an Iterator for a
 * DynamicObject. Below is the class definition for the DynamicObjectIterator
 * type.
 * 
 * @author Dave Longley
 */
class DynamicObjectIteratorImpl : public Iterator<DynamicObject>
{
protected:
   /**
    * The DynamicObject being iterated over.
    */
   DynamicObject mObject;
   
   /**
    * The index of the current object.
    */
   int mIndex;
   
   /**
    * An iterator to the current object in the DynamicObject.
    */
   DynamicObjectImpl::ObjectMap::iterator mMapCurrent;
   DynamicObjectImpl::ObjectArray::iterator mArrayCurrent;
   
   /**
    * An iterator to the next object in the DynamicObject.
    */
   DynamicObjectImpl::ObjectMap::iterator mMapNext;
   DynamicObjectImpl::ObjectArray::iterator mArrayNext;
   
   /**
    * Set to true for non-map/non-array objects once the object has
    * been returned.
    */
   bool mFinished;
   
public:
   /**
    * Creates a new DynamicObjectIteratorImpl for the given DynamicObject.
    * 
    * @param dyno the DynamicObject to iterate over.
    */
   DynamicObjectIteratorImpl(DynamicObject dyno);
   
   /**
    * Destructs this DynamicObjectIteratorImpl.
    */
   virtual ~DynamicObjectIteratorImpl();
   
   /**
    * Gets the next object and advances the DynamicObjectIterator.
    * 
    * @return the next object.
    */
   virtual DynamicObject& next();
   
   /**
    * Returns true if this DynamicObjectIterator has more objects.
    * 
    * @return true if this DynamicObjectIterator has more objects, false if not.
    */
   virtual bool hasNext();
   
   /**
    * Removes the current object and advances the DynamicObjectIterator.
    */
   virtual void remove();
   
   /**
    * Gets the name of the last DynamicObject returned by next().
    * 
    * @return the name of the last DynamicObject returned by next().
    */
   const char* getName();
   
   /**
    * Gets the index of the last DynamicObject returned by next().
    * 
    * @return the index of the last DynamicObject returned by next().
    */
   int getIndex();
};

/**
 * A DynamicObjectIterator is a reference counter for a
 * DynamicObjectIteratorImpl.
 * 
 * This class is only declared because a simple typedef could not be used
 * due to cyclical header referencing (where forward declaration could not
 * solve the problem).
 * 
 * @author Dave Longley
 */
class DynamicObjectIterator :
public db::rt::Collectable<DynamicObjectIteratorImpl>
{
public:
   /**
    * Constructs a DynamicObjectIterator that points at the given
    * heap-allocated DynamicObjectIteratorImpl.
    * 
    * @param impl the DynamicObjectIteratorImpl to point at.
    */
   DynamicObjectIterator(DynamicObjectIteratorImpl* impl = NULL);
   
   /**
    * Destructs this DynamicObjectIterator.
    */
   virtual ~DynamicObjectIterator();
};

} // end namespace util
} // end namespace db
#endif
