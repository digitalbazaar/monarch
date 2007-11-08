/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectBinding.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectBinding::DynamicObjectBinding(DynamicObject* dyno) :
   DataBinding(dyno),
   mStringMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject),
   mBooleanMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject),
   mNumberMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject),
   mMapMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject),
   mArrayMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject)
{
   // FIXME: instead of NULL namespace, use "db.com" 
   const char* ns = NULL;
   
   // set root data name
   setDataName(ns, "dynamicobject");
}

DynamicObjectBinding::~DynamicObjectBinding()
{
}

void* DynamicObjectBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for creating/adding fields
   return this;
}

void DynamicObjectBinding::serializationStarted()
{
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // remove all data mappings and bindings
   removeDataMapping(ns, "string");
   removeDataBinding(ns, "string");
   removeDataMapping(ns, "boolean");
   removeDataBinding(ns, "boolean");
   removeDataMapping(ns, "number");
   removeDataBinding(ns, "number");
   removeDataMapping(ns, "object");
   removeDataBinding(ns, "object");
   removeDataMapping(ns, "array");
   removeDataBinding(ns, "array");
   
   // add data mapping according to object type
   DynamicObject* dyno = (DynamicObject*)getObject();
   switch((*dyno)->getType())
   {
      case String:
         addDataMapping(ns, "string", true, false, &mStringMapping);
         addDataBinding(ns, "string", &mBasicBinding);
         break;
      case Boolean:
         addDataMapping(ns, "boolean", true, false, &mBooleanMapping);
         addDataBinding(ns, "boolean", &mBasicBinding);
         break;
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         addDataMapping(ns, "number", true, false, &mNumberMapping);
         addDataBinding(ns, "number", &mBasicBinding);
         break;
      case Map:
         addDataMapping(ns, "object", true, true, &mMapMapping);
         addDataBinding(ns, "object", &mMapBinding);
         break;
      case Array:
         addDataMapping(ns, "array", true, true, &mArrayMapping);
         addDataBinding(ns, "array", &mArrayBinding);
         break;
   }
}

void DynamicObjectBinding::deserializationStarted()
{
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // remove all data mappings and bindings
   removeDataMapping(ns, "string");
   removeDataBinding(ns, "string");
   removeDataMapping(ns, "boolean");
   removeDataBinding(ns, "boolean");
   removeDataMapping(ns, "number");
   removeDataBinding(ns, "number");
   removeDataMapping(ns, "object");
   removeDataBinding(ns, "object");
   removeDataMapping(ns, "array");
   removeDataBinding(ns, "array");
   
   // add all data mappings and bindings
   addDataMapping(ns, "string", true, false, &mStringMapping);
   addDataBinding(ns, "string", &mBasicBinding);
   addDataMapping(ns, "boolean", true, false, &mBooleanMapping);
   addDataBinding(ns, "boolean", &mBasicBinding);
   addDataMapping(ns, "number", true, false, &mNumberMapping);
   addDataBinding(ns, "number", &mBasicBinding);
   addDataMapping(ns, "object", true, true, &mMapMapping);
   addDataBinding(ns, "object", &mMapBinding);
   addDataMapping(ns, "array", true, true, &mArrayMapping);
   addDataBinding(ns, "array", &mArrayBinding);
}

DynamicObject* DynamicObjectBinding::createDynamicObject()
{
   // return underlying object, child's object is the same object
   return (DynamicObject*)getObject();
}

void DynamicObjectBinding::addDynamicObject(DynamicObject* dyno)
{
   // nothing to do here, child's object is the same as the underlying one
}

void DynamicObjectBinding::getChildren(DataName* dn, list<void*>& children)
{
   // get dynamic object
   DynamicObject* dyno = (DynamicObject*)getObject();
   
   // only add child if type matches data name
   const char* name;
   switch((*dyno)->getType())
   {
      case String:
         name = "string";
         break;
      case Boolean:
         name = "boolean";
         break;
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         name = "number";
         break;
      case Map:
         name = "object";
         break;
      case Array:
         name = "array";
         break;
   }
   
   if(strcmp(dn->name, name) == 0)
   {
      children.push_back(dyno);
   }
}
