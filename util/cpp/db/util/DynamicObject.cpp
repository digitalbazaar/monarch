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

void DynamicObject::operator=(const std::string& value)
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

DynamicObject& DynamicObject::operator[](const std::string& name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](unsigned int index)
{
   return (*mReference->ptr)[index];
}

DynamicObjectIterator DynamicObject::getIterator()
{
   return DynamicObjectIterator(new DynamicObjectIteratorImpl(*this));
}
