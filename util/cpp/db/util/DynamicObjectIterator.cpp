/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectIterator.h"
#include "db/rt/Exception.h"

using namespace db::rt;
using namespace db::util;

DynamicObjectIteratorImpl::DynamicObjectIteratorImpl(DynamicObject dyno)
{
   mObject = dyno;
   
   if(mObject->getType() == DynamicObjectImpl::Map)
   {
      mMapNext = mMapCurrent = mObject->mMap->begin();
   }
   else if(mObject->getType() == DynamicObjectImpl::Array)
   {
      mArrayNext = mArrayCurrent = mObject->mArray->begin();
   }
}

DynamicObjectIteratorImpl::~DynamicObjectIteratorImpl()
{
}

DynamicObject& DynamicObjectIteratorImpl::next()
{
   DynamicObject* rval = NULL;
   
   if(mObject->getType() == DynamicObjectImpl::Map)
   {
      mMapCurrent = mMapNext;
      mMapNext++;
      rval = &mMapCurrent->second;
   }
   else if(mObject->getType() == DynamicObjectImpl::Array)
   {
      mArrayCurrent = mArrayNext;
      mArrayNext++;
      rval = &(*mArrayCurrent);
   }
   
   return *rval;
}

bool DynamicObjectIteratorImpl::hasNext()
{
   bool rval = false;
   
   if(mObject->getType() == DynamicObjectImpl::Map)
   {
      rval = mMapNext != mObject->mMap->end();
   }
   else if(mObject->getType() == DynamicObjectImpl::Array)
   {
      rval = mArrayNext != mObject->mArray->end();
   }
   
   return rval;
}

void DynamicObjectIteratorImpl::remove()
{
   // not supported
   Exception::setLast(
      new Exception("DynamicObjectIterator::remove() not supported!"));
}

DynamicObjectIterator::DynamicObjectIterator(DynamicObjectIteratorImpl* impl) :
   Collectable<DynamicObjectIteratorImpl>(impl)
{
}

DynamicObjectIterator::~DynamicObjectIterator()
{
}
