/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_DynamicObject_H
#define db_util_DynamicObject_H

#include "db/rt/Collectable.h"
#include "db/util/DynamicObjectImpl.h"

namespace db
{
namespace util
{

// forward declare DynamicObjectIterator
class DynamicObjectIterator;

/**
 * A DynamicObject is a reference counted object with a collection of
 * unordered name/value member pairs. Members can be dynamically added to
 * a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObject : public db::rt::Collectable<DynamicObjectImpl>
{
public:
   /**
    * Creates a new DynamicObject with a new, empty DynamicObjectImpl.
    */
   DynamicObject();
   
   /**
    * Creates a DynamicObject that will reference count and then destroy
    * the passed DynamicObjectImpl.
    * 
    * @param impl a pointer to a DynamicObjectImpl.
    */
   DynamicObject(DynamicObjectImpl* impl);
   
   /**
    * Creates a new DynamicObject by copying another one.
    * 
    * @param copy the DynamicObject to copy.
    */
   DynamicObject(const DynamicObject& rhs);
   
   /**
    * Destructs this DynamicObject.
    */
   virtual ~DynamicObject();
   
   /**
    * Compares this DynamicObject to another one for equality. If the
    * DynamicObjects are the same type, and have the same value or contain
    * all the same values (for Maps/Arrays), then they are equal.
    * 
    * @param rhs the DynamicObject to compare to this one.
    * 
    * @return true if the passed DynamicObject is equal to this one, false
    *         if not.
    */ 
   virtual bool operator==(const DynamicObject& rhs);
   
   /**
    * Compares this DynamicObject to another one for non-equality. If the
    * DynamicObjects are the same type, and have the same value or contain
    * all the same values (for Maps/Arrays), then they are equal -- otherwise
    * they are not.
    * 
    * @param rhs the DynamicObject to compare to this one.
    * 
    * @return true if the passed DynamicObject is not equal to this one, false
    *         if not.
    */ 
   virtual bool operator!=(const DynamicObject& rhs);
   
   /**
    * Sets this object's value to a string.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(const char* value);
   
   /**
    * Sets this object's value to a boolean.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(bool value);
   
   /**
    * Sets this object's value to a 32-bit integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(int value);
   
   /**
    * Sets this object's value to a 32-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned int value);
   
   /**
    * Sets this object's value to a 64-bit integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(long long value);
   
   /**
    * Sets this object's value to a 64-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned long long value);
   
   /**
    * Sets this object's value to a double.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(double value);
   
   /**
    * Gets a DynamicObject from a DynamicObjectImpl based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](const std::string& name);
   
   /**
    * Gets a DynamicObject from a DynamicObjectImpl based on its index. A
    * negative index will index in reverse, with -1 referring to the last
    * element.
    * 
    * @param index the index of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](int index);
   
   /**
    * Gets a reference-counted DynamicObjectIterator for iterating over
    * the members of this object or its array elements.
    * 
    * @return a DynamicObjectIterator.
    */
   virtual DynamicObjectIterator getIterator();
   
   /**
    * Clones this DynamicObject and returns it.
    * 
    * @return a clone of this DynamicObject.
    */
   virtual DynamicObject clone();
   
   /**
    * Determines if this DynamicObject is a subset of another. If this
    * DynamicObject does not reference the exact same DynamicObject as the
    * passed one, then a check will be made to determine if this DynamicObject
    * this is a subset of the passed one.
    * 
    * This DynamicObject is only a subset of the passed one if both are
    * Maps and the passed DynamicObject contains at least all of the members
    * and values of this DynamicObject.
    * 
    * @param rhs the DynamicObject to check as a superset.
    * 
    * @return true if this DynamicObject is a subset of the passed one.
    */
   virtual bool isSubset(const DynamicObject& rhs);
};

} // end namespace util
} // end namespace db
#endif
