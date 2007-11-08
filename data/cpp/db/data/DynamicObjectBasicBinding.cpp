/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectBasicBinding.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectBasicBinding::DynamicObjectBasicBinding(DynamicObject* dyno) :
   DataBinding(dyno),
   mStringMapping(
      &DynamicObjectBasicBinding::setString,
      &DynamicObjectBasicBinding::getString),
   mBooleanMapping(
      &DynamicObjectBasicBinding::setBoolean,
      &DynamicObjectBasicBinding::getBoolean),
   mNumberMapping(
      &DynamicObjectBasicBinding::setNumber,
      &DynamicObjectBasicBinding::getNumber)
{
}

DynamicObjectBasicBinding::~DynamicObjectBasicBinding()
{
}

void* DynamicObjectBasicBinding::getSetGetObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void DynamicObjectBasicBinding::serializationStarted()
{
   // FIXME: instead of NULL namespace, use "db.com"? 
   const char* ns = NULL;
   
   // remove all data mappings
   removeDataMapping(ns, "string");
   removeDataMapping(ns, "boolean");
   removeDataMapping(ns, "number");
   
   // add data mapping according to object type
   DynamicObject* dyno = (DynamicObject*)getObject();
   switch((*dyno)->getType())
   {
      case String:
         setDataName(ns, "string");
         addDataMapping(ns, "string", true, true, &mStringMapping);
         break;
      case Boolean:
         setDataName(ns, "boolean");
         addDataMapping(ns, "boolean", true, true, &mBooleanMapping);
         break;
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         setDataName(ns, "number");
         addDataMapping(ns, "number", true, true, &mNumberMapping);
         break;
      case Map:
         break;
      case Array:
         break;
   }
}

void DynamicObjectBasicBinding::deserializationStarted()
{
   // FIXME: instead of NULL namespace, use "db.com"? 
   const char* ns = NULL;
   
   // remove all data mappings
   removeDataMapping(ns, "string");
   removeDataMapping(ns, "boolean");
   removeDataMapping(ns, "number");
   
   // add all data mappings
   addDataMapping(ns, "string", true, true, &mStringMapping);
   addDataMapping(ns, "boolean", true, true, &mBooleanMapping);
   addDataMapping(ns, "number", true, true, &mNumberMapping);
}

void DynamicObjectBasicBinding::setString(const char* str)
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   (*dyno) = (const char*)str;
}

const char* DynamicObjectBasicBinding::getString()
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->getString();
}

void DynamicObjectBasicBinding::setBoolean(const char* b)
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   (*dyno) = (strcmp(b, "true") == 0) ? true : false;
}

const char* DynamicObjectBasicBinding::getBoolean()
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->toString(mTempString).c_str();
}

void DynamicObjectBasicBinding::setNumber(const char* num)
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   if(strchr(num, 'e') != NULL || strchr(num, 'E') != NULL ||
      strchr(num, '.') != NULL)
   {
      // read number in as a double
      (*dyno) = (double)strtod(num, NULL);
   }
   else if(strchr(num, '-') != NULL)
   {
      // read number in as signed 64-bit integer
      (*dyno) = (long long)strtoll(num, NULL, 10);
   }
   else
   {
      // read number in as unsigned 64-bit integer
      (*dyno) = (unsigned long long)strtoull(num, NULL, 10);
   }
}

const char* DynamicObjectBasicBinding::getNumber()
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->toString(mTempString).c_str();
}
