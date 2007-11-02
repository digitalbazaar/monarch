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
    * Creates a DynamicObject.
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
    * Gets a MemberValue from a DynamicObjectImpl based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the MemberValue.
    */
   virtual DynamicObjectImpl::MemberValue& operator[](const char* name);
};

} // end namespace util
} // end namespace db
#endif
