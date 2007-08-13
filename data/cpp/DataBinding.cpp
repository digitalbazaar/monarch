/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DataBinding.h"

#include <stddef.h>

using namespace db::data;

DataBinding::DataBinding(void* obj)
{
   mObject = obj;
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
         charEncoding << ", " << name << ")" << endl;
   }
   else
   {
      cout << "startData(" <<
         charEncoding << ", NULL, " << name << ")" << endl;
   }
   
   // FIXME:
   //return NULL;
   return this;
}

void DataBinding::appendData(
   const char* charEncoding, char* data, unsigned int length)
{
   // FIXME:
}

void DataBinding::endData(
   const char* charEncoding, const char* ns, const char* name, DataBinding* db)
{
   if(db != NULL && ns != NULL)
   {
      cout << "endData(" << charEncoding << ", " << ns << ", " << name << ", " << db << ")" << endl;
   }
   else if(ns != NULL)
   {
      cout << "endData(" << charEncoding << ", " << ns << ", " << name << ", NULL)" << endl;
   }
   else
   {
      cout << "endData(" << charEncoding << ", NULL, " << name << ", NULL)" << endl;
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
