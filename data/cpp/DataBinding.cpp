/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DataBinding.h"

#include <stddef.h>

using namespace db::data;

DataBinding::DataBinding(void* obj)
{
   mObject = obj;
   mDataNamespace = NULL;
   mDataName = NULL;
}

DataBinding::~DataBinding()
{
}

DataBinding* DataBinding::startData(
   const char* charEncoding, const char* ns, const char* name) 
{
   // FIXME:
   return NULL;
}

void DataBinding::appendData(
   const char* charEncoding, char* data, unsigned int length)
{
   // FIXME:
}

void DataBinding::endData(DataBinding* db)
{
   // FIXME:
}

void DataBinding::setData(
   const char* charEncoding, const char* ns, const char* name,
   const char* data, unsigned int length)
{
   // FIXME:
}

void* DataBinding::getObject()
{
   return mObject;
}
