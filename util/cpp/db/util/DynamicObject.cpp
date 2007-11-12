/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"

using namespace db::rt;
using namespace db::util;

DynamicObject::DynamicObject() :
   Collectable<DynamicObjectImpl>(new DynamicObjectImpl())
{
}

DynamicObject::DynamicObject(DynamicObjectImpl* impl) :
   Collectable<DynamicObjectImpl>(impl)
{
}

DynamicObject::DynamicObject(const DynamicObject& rhs) :
   Collectable<DynamicObjectImpl>(rhs)
{
}

DynamicObject::~DynamicObject()
{
}

void DynamicObject::operator=(const char* value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(bool value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(int value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(unsigned int value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(long long value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(unsigned long long value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(double value)
{
   *mReference->ptr = value;
}

DynamicObject& DynamicObject::operator[](const std::string& name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](int index)
{
   return (*mReference->ptr)[index];
}

DynamicObjectIterator DynamicObject::getIterator()
{
   return DynamicObjectIterator(new DynamicObjectIteratorImpl(*this));
}

DynamicObject DynamicObject::clone()
{
   DynamicObject rval;
   
   int index = 0;
   DynamicObjectIterator i = getIterator();
   while(i->hasNext())
   {
      DynamicObject dyno = i->next();
      switch((*this)->getType())
      {
         case String:
            rval = dyno->getString();
            break;
         case Boolean:
            rval = dyno->getBoolean();
            break;
         case Int32:
            rval = dyno->getInt32();
            break;
         case UInt32:
            rval = dyno->getUInt32();
            break;
         case Int64:
            rval = dyno->getInt64();
            break;
         case UInt64:
            rval = dyno->getUInt64();
            break;
         case Double:
            rval = dyno->getDouble();
            break;
         case Map:
            rval[i->getName()] = dyno.clone();
            break;
         case Array:
            rval[index++] = dyno.clone();
            break;
      }
   }
   
   return rval;
}
