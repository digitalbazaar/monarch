/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/DynamicObjectIterator.h"

#include "db/rt/Exception.h"

#include <cstdlib>

using namespace db::rt;

DynamicObjectIteratorImpl::DynamicObjectIteratorImpl(DynamicObject dyno)
{
   mObject = dyno;
   mIndex = -1;
   mName = NULL;

   switch(mObject->getType())
   {
      case Map:
         mMapIterator = mObject->mMap->begin();
         break;
      case Array:
         mArrayIterator = mObject->mArray->begin();
         break;
      default:
         break;
   }
}

DynamicObjectIteratorImpl::~DynamicObjectIteratorImpl()
{
}

bool DynamicObjectIteratorImpl::hasNext()
{
   bool rval;
   
   switch(mObject->getType())
   {
      case Map:
         rval = (mMapIterator != mObject->mMap->end());
         break;
      case Array:
         rval = (mArrayIterator != mObject->mArray->end());
         break;
      default:
         rval = (mIndex != 0);
         break;
   }
   
   return rval;
}

DynamicObject& DynamicObjectIteratorImpl::next()
{
   DynamicObject* rval;
   
   switch(mObject->getType())
   {
      case Map:
         rval = &mMapIterator->second;
         mIndex++;
         mName = mMapIterator->first;
         mMapIterator++;
         break;
      case Array:
         rval = &(*mArrayIterator);
         mIndex++;
         mArrayIterator++;
         break;
      default:
         // always return this object
         rval = &mObject;
         mIndex = 0;
         break;
   }
   
   return *rval;
}

void DynamicObjectIteratorImpl::remove()
{
   switch(mObject->getType())
   {
      case Map:
      {
         // copy iterator and reverse to previous position for deletion
         DynamicObjectImpl::ObjectMap::iterator last = mMapIterator;
         last--;
         mObject->removeMember(last);
         mIndex--;
         mName = NULL;
         break;
      }
      case Array:
         mArrayIterator = mObject->mArray->erase(--mArrayIterator);
         mIndex--;
         break;
      default:
         break;
   }
}

const char* DynamicObjectIteratorImpl::getName()
{
   return mName;
}

int DynamicObjectIteratorImpl::getIndex()
{
   return mIndex;
}

DynamicObjectIterator::DynamicObjectIterator(DynamicObjectIteratorImpl* impl) :
   Collectable<DynamicObjectIteratorImpl>(impl)
{
}

DynamicObjectIterator::~DynamicObjectIterator()
{
}
