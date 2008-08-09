/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_DynamicObjectIterator_H
#define db_rt_DynamicObjectIterator_H

#include "db/rt/Iterator.h"
#include "db/rt/DynamicObject.h"

namespace db
{
namespace rt
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
    * The name of the current object.
    */
   const char* mName;
   
   /**
    * An iterator for the object in the DynamicObject.
    */
   DynamicObjectImpl::ObjectMap::iterator mMapIterator;
   DynamicObjectImpl::ObjectArray::iterator mArrayIterator;
   
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
    * Checks if this DynamicObjectIterator has more objects. For simple types
    * (not Map or Array) this will only return true once.
    * 
    * @return true if this DynamicObjectIterator has more objects, false if not.
    */
   virtual bool hasNext();
   
   /**
    * Gets the next object and advances the DynamicObjectIterator.
    * 
    * @return the next object.
    */
   virtual DynamicObject& next();
   
   /**
    * Removes the current object. Only valid for Maps and Arrays. Invalidates
    * the results of getName() and getIndex() until the next next();
    * 
    * WARNING: Arrays are implemented using vectors and use of remove() will
    *          result in a memory shift of all remaining data.
    */
   virtual void remove();
   
   /**
    * Gets the name of the last DynamicObject returned by next() for Maps.
    * For other types or between remove() and the next call to next() the
    * return value is NULL. Value not valid after a call to remove().
    * 
    * @return the name of the last DynamicObject returned by next() for Maps
    *         otherwise NULL.
    */
   virtual const char* getName();
   
   /**
    * Gets the index of the last DynamicObject returned by next(). Before
    * next() is called and after a remove() on the first object getIndex()
    * will return -1. Value not valid after a call to remove().
    * 
    * @return the index of the last DynamicObject returned by next().
    */
   virtual int getIndex();
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
public Collectable<DynamicObjectIteratorImpl>
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

} // end namespace rt
} // end namespace db
#endif
