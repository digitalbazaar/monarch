/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_rt_DynamicObjectIterators_H
#define db_rt_DynamicObjectIterators_H

#include "monarch/rt/DynamicObjectIterator.h"

namespace db
{
namespace rt
{

/**
 * A DynamicObjectIteratorSingle is an implementation for an Iterator for a
 * single DynamicObject.
 *
 * @author Dave Longley
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class DynamicObjectIteratorSingle : public DynamicObjectIteratorImpl
{
public:
   /**
    * Creates a new DynamicObjectIteratorSingle for the given DynamicObject.
    *
    * @param dyno the DynamicObject to iterate over.
    */
   DynamicObjectIteratorSingle(DynamicObject& dyno);

   /**
    * Destructs this DynamicObjectIteratorSingle.
    */
   virtual ~DynamicObjectIteratorSingle();

   /**
    * Returns true once.
    *
    * @return true first call, false after.
    */
   virtual bool hasNext();

   /**
    * Gets the object.
    *
    * @return the object.
    */
   virtual DynamicObject& next();
};

/**
 * A DynamicObjectIteratorArray is the implementation for an Iterator for an
 * Array DynamicObject.
 *
 * @author Dave Longley
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class DynamicObjectIteratorArray : public DynamicObjectIteratorImpl
{
protected:
   /**
    * The object array.
    */
   DynamicObjectImpl::ObjectArray* mArray;

   /**
    * An iterator for the array in the DynamicObject.
    */
   DynamicObjectImpl::ObjectArray::iterator mArrayIterator;

public:
   /**
    * Creates a new DynamicObjectIteratorArray for the given DynamicObject.
    *
    * @param dyno the DynamicObject to iterate over.  Must be an Array.
    */
   DynamicObjectIteratorArray(DynamicObject& dyno);

   /**
    * Destructs this DynamicObjectIteratorArray.
    */
   virtual ~DynamicObjectIteratorArray();

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
};

/**
 * A DynamicObjectIteratorMap is the implementation for an Iterator for a
 * Map DynamicObject.
 *
 * @author Dave Longley
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class DynamicObjectIteratorMap : public DynamicObjectIteratorImpl
{
protected:
   /**
    * The name of the current object.
    */
   const char* mName;

   /**
    * The object map.
    */
   DynamicObjectImpl::ObjectMap* mMap;

   /**
    * An iterator for the object in the DynamicObject.
    */
   DynamicObjectImpl::ObjectMap::iterator mMapIterator;

public:
   /**
    * Creates a new DynamicObjectIteratorMap for the given DynamicObject.
    *
    * @param dyno the DynamicObject to iterate over.  Must be a Map.
    */
   DynamicObjectIteratorMap(DynamicObject& dyno);

   /**
    * Destructs this DynamicObjectIteratorMap.
    */
   virtual ~DynamicObjectIteratorMap();

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
};

} // end namespace rt
} // end namespace db
#endif
