/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/DynamicObjectIterator.h"

#include <cstdlib>

using namespace db::rt;

DynamicObjectIteratorImpl::DynamicObjectIteratorImpl(DynamicObject& dyno) :
   mObject(dyno),
   mIndex(-1)
{
}

DynamicObjectIteratorImpl::~DynamicObjectIteratorImpl()
{
}

void DynamicObjectIteratorImpl::remove()
{
}

const char* DynamicObjectIteratorImpl::getName()
{
   return NULL;
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
