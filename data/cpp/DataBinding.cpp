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
#include <iostream>
using namespace std;
DataBinding* DataBinding::startData(
   const char* charEncoding, const char* ns, const char* name) 
{
   if(ns != NULL)
   {
      cout << "startData(" <<
         charEncoding << ", " << ns << ", " << name << ")" << endl;
   }
   else
   {
      cout << "startData(" <<
         charEncoding << ", NULL, " << name << ")" << endl;
   }
   
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
   if(db != NULL)
   {
      cout << "endData(" << db << ")" << endl;
   }
   else
   {
      cout << "endData(NULL)" << endl;
   }
   
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
