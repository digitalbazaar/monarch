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
   
   if(mObject->getType() == Map)
   {
      mMapNext = mMapCurrent = mObject->mMap->begin();
   }
   else if(mObject->getType() == Array)
   {
      mArrayNext = mArrayCurrent = mObject->mArray->begin();
   }
   else
   {
      mFinished = false;
   }
}

DynamicObjectIteratorImpl::~DynamicObjectIteratorImpl()
{
}

DynamicObject& DynamicObjectIteratorImpl::next()
{
   DynamicObject* rval = NULL;
   
   if(mObject->getType() == Map)
   {
      mMapCurrent = mMapNext;
      mMapNext++;
      rval = &mMapCurrent->second;
   }
   else if(mObject->getType() == Array)
   {
      mArrayCurrent = mArrayNext;
      mArrayNext++;
      rval = &(*mArrayCurrent);
   }
   else if(!mFinished)
   {
      // return this object
      rval = &mObject;
      mFinished = true;
   }
   
   return *rval;
}

bool DynamicObjectIteratorImpl::hasNext()
{
   bool rval = false;
   
   if(mObject->getType() == Map)
   {
      rval = (mMapNext != mObject->mMap->end());
   }
   else if(mObject->getType() == Array)
   {
      rval = (mArrayNext != mObject->mArray->end());
   }
   else
   {
      rval = !mFinished;
   }
   
   return rval;
}

void DynamicObjectIteratorImpl::remove()
{
   // not supported
   Exception::setLast(
      new Exception("DynamicObjectIterator::remove() not supported!"));
}

const char* DynamicObjectIteratorImpl::getName()
{
   const char* rval;
   
   if(mObject->getType() == Map)
   {
      rval = mMapCurrent->first;
   }
   else
   {
      rval = "";
   }
   
   return rval;
}

DynamicObjectIterator::DynamicObjectIterator(DynamicObjectIteratorImpl* impl) :
   Collectable<DynamicObjectIteratorImpl>(impl)
{
}

DynamicObjectIterator::~DynamicObjectIterator()
{
}
