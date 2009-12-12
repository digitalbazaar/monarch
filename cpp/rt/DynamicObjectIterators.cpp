/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/DynamicObjectIterators.h"

using namespace monarch::rt;

/* Single */

DynamicObjectIteratorSingle::DynamicObjectIteratorSingle(DynamicObject& dyno) :
   DynamicObjectIteratorImpl(dyno)
{
}

DynamicObjectIteratorSingle::~DynamicObjectIteratorSingle()
{
}

bool DynamicObjectIteratorSingle::hasNext()
{
   return (mIndex != 0);
}

DynamicObject& DynamicObjectIteratorSingle::next()
{
   mIndex = 0;
   return mObject;
}

/* Array */

DynamicObjectIteratorArray::DynamicObjectIteratorArray(DynamicObject& dyno) :
   DynamicObjectIteratorImpl(dyno),
   mArray(dyno->mArray),
   mArrayIterator(mArray->begin())
{
}

DynamicObjectIteratorArray::~DynamicObjectIteratorArray()
{
}

bool DynamicObjectIteratorArray::hasNext()
{
   return (mArrayIterator != mArray->end());
}

DynamicObject& DynamicObjectIteratorArray::next()
{
   DynamicObject& rval = *mArrayIterator;
   mIndex++;
   mArrayIterator++;
   return rval;
}

void DynamicObjectIteratorArray::remove()
{
   mArrayIterator = mArray->erase(--mArrayIterator);
   mIndex--;
}

/* Map */

DynamicObjectIteratorMap::DynamicObjectIteratorMap(DynamicObject& dyno) :
   DynamicObjectIteratorImpl(dyno),
   mName(NULL),
   mMap(dyno->mMap),
   mMapIterator(mMap->begin())
{
}

DynamicObjectIteratorMap::~DynamicObjectIteratorMap()
{
}

bool DynamicObjectIteratorMap::hasNext()
{
   return (mMapIterator != mMap->end());
}

DynamicObject& DynamicObjectIteratorMap::next()
{
   DynamicObject& rval = mMapIterator->second;
   mIndex++;
   mName = mMapIterator->first;
   mMapIterator++;
   return rval;
}

void DynamicObjectIteratorMap::remove()
{
   // copy iterator and reverse to previous position for deletion
   DynamicObjectImpl::ObjectMap::iterator last = mMapIterator;
   last--;
   mObject->removeMember(last);
   mIndex--;
   mName = NULL;
}

const char* DynamicObjectIteratorMap::getName()
{
   return mName;
}
