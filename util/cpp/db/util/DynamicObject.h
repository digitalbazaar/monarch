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
    * Creates a DynamicObject. This constructor is explicit so that an
    * integer is not confused for a pointer to a DynamicObjectImpl.
    * 
    * @param impl a pointer to a DynamicObjectImpl.
    */
   explicit DynamicObject(DynamicObjectImpl* impl);
   
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
};

} // end namespace util
} // end namespace db
#endif
