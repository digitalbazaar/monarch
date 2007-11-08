/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectMapBinding.h"
#include "db/data/DynamicObjectArrayBinding.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectMemberBinding::DynamicObjectMemberBinding(
   DynamicObject* dyno, DynamicObjectMapBinding* mb) :
   DataBinding(dyno),
   mNameMapping(
      &DynamicObjectMemberBinding::setName,
      &DynamicObjectMemberBinding::getName),
   mStringMapping(
      &DynamicObjectMemberBinding::createDynamicObject,
      &DynamicObjectMemberBinding::addDynamicObject),
   mBooleanMapping(
      &DynamicObjectMemberBinding::createDynamicObject,
      &DynamicObjectMemberBinding::addDynamicObject),
   mNumberMapping(
      &DynamicObjectMemberBinding::createDynamicObject,
      &DynamicObjectMemberBinding::addDynamicObject),
   mMapMapping(
      &DynamicObjectMemberBinding::createDynamicObject,
      &DynamicObjectMemberBinding::addDynamicObject),
   mArrayMapping(
      &DynamicObjectMemberBinding::createDynamicObject,
      &DynamicObjectMemberBinding::addDynamicObject)
{
   // store parent map binding
   mParentBinding = mb;
   
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // set root data name
   setDataName(ns, "member");
   
   // add name mapping
   addDataMapping(ns, "name", false, true, &mNameMapping);
   
   // no map or array bindings yet
   mMapBinding = NULL;
   mArrayBinding = NULL;
}

DynamicObjectMemberBinding::~DynamicObjectMemberBinding()
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

void* DynamicObjectMemberBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void* DynamicObjectMemberBinding::getSetGetObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void DynamicObjectMemberBinding::serializationStarted()
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
   DynamicObject* dyno = &d[mParentBinding->getNextName()];
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

void DynamicObjectMemberBinding::deserializationStarted()
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

void DynamicObjectMemberBinding::setName(const char* name)
{
   mParentBinding->setName(name);
}

const char* DynamicObjectMemberBinding::getName()
{
   // return current name
   return mParentBinding->getName();
}

DynamicObject* DynamicObjectMemberBinding::createDynamicObject()
{
   // create heap-allocated reference
   return new DynamicObject();
}

void DynamicObjectMemberBinding::addDynamicObject(DynamicObject* dyno)
{
   // set object at current name and delete heap-allocated reference
   DynamicObject* d = (DynamicObject*)getObject();
   (*d)[mParentBinding->getName()] = *dyno;
   delete dyno;
}

void DynamicObjectMemberBinding::getChildren(
   DataName* dn, list<void*>& children)
{
   // get dynamic object
   DynamicObject& d = *(DynamicObject*)getObject();
   DynamicObject* dyno = &d[mParentBinding->getName()];
   
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

DynamicObjectMapBinding::DynamicObjectMapBinding(DynamicObject* dyno) :
   DataBinding(dyno),
   mMemberMapping(
      &DynamicObjectMapBinding::createMember,
      &DynamicObjectMapBinding::addMember),
   mMemberBinding(dyno, this)
{
   // FIXME: instead of NULL namespace, use "db.com"?
   const char* ns = NULL;
   
   // set root data name
   setDataName(ns, "object");
   
   // add data mappings and bindings
   addDataMapping(ns, "member", true, true, &mMemberMapping);
   addDataBinding(ns, "member", &mMemberBinding);
}

DynamicObjectMapBinding::~DynamicObjectMapBinding()
{
   if(mCurrentName != NULL)
   {
      delete [] mCurrentName;
   }
}

void* DynamicObjectMapBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

void DynamicObjectMapBinding::serializationStarted()
{
   // reset current name and iterator
   mCurrentName = NULL;
   DynamicObject* dyno = (DynamicObject*)getObject();
   mIterator = dyno->getIterator();
}

void DynamicObjectMapBinding::deserializationStarted()
{
   // reset current name and iterator
   mCurrentName = NULL;
   DynamicObject* dyno = (DynamicObject*)getObject();
   mIterator = dyno->getIterator();
}

void* DynamicObjectMapBinding::createMember()
{
   // return underlying object, member's object is the same object
   return getObject();
}

void DynamicObjectMapBinding::addMember(void* m)
{
   // nothing to do here, member's object is the same as the underlying one
}

void DynamicObjectMapBinding::setName(const char* name)
{
   if(mCurrentName != NULL)
   {
      delete [] mCurrentName;
   }
   
   mCurrentName = strdup(name);
}

const char* DynamicObjectMapBinding::getName()
{
   // return current name
   return mCurrentName;
}

const char* DynamicObjectMapBinding::getNextName()
{
   // increment and return current index
   mIterator->next();
   if(mCurrentName != NULL)
   {
      delete [] mCurrentName;
   }
   mCurrentName = strdup(mIterator->getName());
   return mCurrentName;
}

void DynamicObjectMapBinding::getChildren(DataName* dn, list<void*>& children)
{
   // for each element, return a pointer to the underlying object
   DynamicObject* dyno = (DynamicObject*)getObject();
   int length = (*dyno)->length();
   for(int i = 0; i < length; i++)
   {
      children.push_back(getObject());
   }
}
