/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectArrayBinding.h"
#include "db/data/DynamicObjectMapBinding.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectElementBinding::DynamicObjectElementBinding(
   DynamicObject* dyno, DynamicObjectArrayBinding* ab) :
   DataBinding(dyno),
   mIndexMapping(
      &DynamicObjectElementBinding::setIndex,
      &DynamicObjectElementBinding::getIndex),
   mStringMapping(
      &DynamicObjectElementBinding::createDynamicObject,
      &DynamicObjectElementBinding::addDynamicObject),
   mBooleanMapping(
      &DynamicObjectElementBinding::createDynamicObject,
      &DynamicObjectElementBinding::addDynamicObject),
   mNumberMapping(
      &DynamicObjectElementBinding::createDynamicObject,
      &DynamicObjectElementBinding::addDynamicObject),
   mMapMapping(
      &DynamicObjectElementBinding::createDynamicObject,
      &DynamicObjectElementBinding::addDynamicObject),
   mArrayMapping(
      &DynamicObjectElementBinding::createDynamicObject,
      &DynamicObjectElementBinding::addDynamicObject)
{
   // store parent array binding
   mParentBinding = ab;
   
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // set root data name
   setDataName(ns, "element");
   
   // add index mapping
   addDataMapping(ns, "index", false, true, &mIndexMapping);
   
   // no map or array bindings yet
   mMapBinding = NULL;
   mArrayBinding = NULL;
}

DynamicObjectElementBinding::~DynamicObjectElementBinding()
{
   if(mMapBinding != NULL)
   {
      delete mMapBinding;
   }
   
   if(mArrayBinding != NULL)
   {
      delete mArrayBinding;
   }
}

void* DynamicObjectElementBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void* DynamicObjectElementBinding::getSetGetObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void DynamicObjectElementBinding::serializationStarted()
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
   
   // add data mapping according to object type (increment index)
   DynamicObject& d = *(DynamicObject*)getObject();
   DynamicObject* dyno = &d[mParentBinding->getNextIndex()];
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
         // lazily create map binding
         if(mMapBinding == NULL)
         {
            mMapBinding = new DynamicObjectMapBinding(dyno);
         }
         addDataMapping(ns, "object", true, true, &mMapMapping);
         addDataBinding(ns, "object", mMapBinding);
         break;
      case Array:
         // lazily create array binding
         if(mArrayBinding == NULL)
         {
            mArrayBinding = new DynamicObjectArrayBinding(dyno);
         }
         addDataMapping(ns, "array", true, true, &mArrayMapping);
         addDataBinding(ns, "array", mArrayBinding);
         break;
   }
}

void DynamicObjectElementBinding::deserializationStarted()
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
   
   // lazily create map binding
   if(mMapBinding == NULL)
   {
      mMapBinding = new DynamicObjectMapBinding();
   }
   
   // lazily create array binding
   if(mArrayBinding == NULL)
   {
      mArrayBinding = new DynamicObjectArrayBinding();
   }
   
   // add all data mappings and bindings
   addDataMapping(ns, "string", true, false, &mStringMapping);
   addDataBinding(ns, "string", &mBasicBinding);
   addDataMapping(ns, "boolean", true, false, &mBooleanMapping);
   addDataBinding(ns, "boolean", &mBasicBinding);
   addDataMapping(ns, "number", true, false, &mNumberMapping);
   addDataBinding(ns, "number", &mBasicBinding);
   addDataMapping(ns, "object", true, true, &mMapMapping);
   addDataBinding(ns, "object", mMapBinding);
   addDataMapping(ns, "array", true, true, &mArrayMapping);
   addDataBinding(ns, "array", mArrayBinding);
}

void DynamicObjectElementBinding::setIndex(int index)
{
   mParentBinding->setIndex(index);
}

int DynamicObjectElementBinding::getIndex()
{
   // return current index
   return mParentBinding->getIndex();
}

DynamicObject* DynamicObjectElementBinding::createDynamicObject()
{
   // create heap-allocated reference
   return new DynamicObject();
}

void DynamicObjectElementBinding::addDynamicObject(DynamicObject* dyno)
{
   // set object at current index and delete heap-allocated reference
   DynamicObject* d = (DynamicObject*)getObject();
   (*d)[mParentBinding->getIndex()] = *dyno;
   delete dyno;
}

void DynamicObjectElementBinding::getChildren(
   DataName* dn, list<void*>& children)
{
   // get dynamic object
   DynamicObject& d = *(DynamicObject*)getObject();
   DynamicObject* dyno = &d[mParentBinding->getIndex()];
   
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

DynamicObjectArrayBinding::DynamicObjectArrayBinding(DynamicObject* dyno) :
   DataBinding(dyno),
   mElementMapping(
      &DynamicObjectArrayBinding::createElement,
      &DynamicObjectArrayBinding::addElement),
   mElementBinding(dyno, this)
{
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // set root data name
   setDataName(ns, "array");
   
   // add data mappings and bindings
   addDataMapping(ns, "element", true, true, &mElementMapping);
   addDataBinding(ns, "element", &mElementBinding);
}

DynamicObjectArrayBinding::~DynamicObjectArrayBinding()
{
}

void* DynamicObjectArrayBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void DynamicObjectArrayBinding::serializationStarted()
{
   // reset current index
   mCurrentIndex = -1;
}

void DynamicObjectArrayBinding::deserializationStarted()
{
   // reset current index
   mCurrentIndex = -1;
}

void* DynamicObjectArrayBinding::createElement()
{
   // return underlying object, element's object is the same object
   return getObject();
}

void DynamicObjectArrayBinding::addElement(void* e)
{
   // nothing to do here, element's object is the same as the underlying one
}

void DynamicObjectArrayBinding::setIndex(int index)
{
   mCurrentIndex = index;
}

int DynamicObjectArrayBinding::getIndex()
{
   // return current index
   return mCurrentIndex;
}

int DynamicObjectArrayBinding::getNextIndex()
{
   // increment and return current index
   return ++mCurrentIndex;
}

void DynamicObjectArrayBinding::getChildren(DataName* dn, list<void*>& children)
{
   // for each element, return a pointer to the underlying object
   DynamicObject* dyno = (DynamicObject*)getObject();
   int length = (*dyno)->length();
   for(int i = 0; i < length; i++)
   {
      children.push_back(getObject());
   }
}
