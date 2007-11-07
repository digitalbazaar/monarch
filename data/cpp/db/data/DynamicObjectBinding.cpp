/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectBinding.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectBinding::DynamicObjectBinding(DynamicObject& dyno) :
   DataBinding(&dyno),
   mNameMapping(
      &DynamicObjectBinding::setName, &DynamicObjectBinding::getName),
   mCreateMapping(
      &DynamicObjectBinding::createDynamicObject,
      &DynamicObjectBinding::addDynamicObject),
   mStringMapping(
      &DynamicObjectBinding::setString, &DynamicObjectBinding::getString),
   mNumberMapping(
      &DynamicObjectBinding::setNumber, &DynamicObjectBinding::getNumber),
   mBooleanMapping(
      &DynamicObjectBinding::setBoolean, &DynamicObjectBinding::getBoolean)
{
   // FIXME: instead of NULL namespace, use "db.com" 
   const char* ns = NULL;
   
   // FIXME: this class needs a lot of work
   
   // set root data name
   // FIXME: root data name won't be "DynamicObject" i don't think
   // FIXME: we might need to overload setObject() for this binding to
   // set its root data name according to the object passed, is this
   // problematic when reading from xml?
   setDataName(ns, "DynamicObject");
   
   // FIXME: probably need 2 bindings, one for setting data in objects
   // and one for holding data in objects
   
   // add mappings
   addDataMapping(ns, "Name", true, true, &mNameMapping);
   addDataMapping(ns, "Object", true, true, &mCreateMapping);
   addDataMapping(ns, "String", true, true, &mCreateMapping);
   addDataMapping(ns, "String", true, true, &mStringMapping);
   addDataMapping(ns, "Number", true, true, &mCreateMapping);
   addDataMapping(ns, "Number", true, true, &mNumberMapping);
   addDataMapping(ns, "Boolean", true, true, &mCreateMapping);
   addDataMapping(ns, "Boolean", true, true, &mBooleanMapping);
   
   // add bindings
   addDataBinding(ns, "Object", this);
   addDataBinding(ns, "String", this);
   addDataBinding(ns, "Number", this);
   addDataBinding(ns, "Boolean", this);
}

DynamicObjectBinding::~DynamicObjectBinding()
{
}

void* DynamicObjectBinding::getCreateAddObject(DataName* dn)
{
   // use this as the object for creating/adding fields
   return this;
}

void* DynamicObjectBinding::getSetGetObject(DataName* dn)
{
   // use this as the object for getting/setting data
   return this;
}

DynamicObject* DynamicObjectBinding::createDynamicObject()
{
   // return heap-allocated dyno
   return new DynamicObject();
}

void DynamicObjectBinding::addDynamicObject(DynamicObject* dyno)
{
   // set dyno and clean up heap-allocated field
   DynamicObject* thisDyno = (DynamicObject*)getObject();
   
   // get the last name
   string name = mNameStack.front();
   mNameStack.pop_front();
   
   if(strcmp(getCurrentDataName()->name, "String"))
   {
      (*thisDyno)[name.c_str()] = (*dyno)->getString();
   }
   else if(strcmp(getCurrentDataName()->name, "Number"))
   {
      // FIXME:
      //(*thisDyno)[name.c_str()] = (*dyno)->getString();
   }
   else if(strcmp(getCurrentDataName()->name, "Boolean"))
   {
      // FIXME:
   }
   else if(strcmp(getCurrentDataName()->name, "Object"))
   {
      // FIXME:
   }
   
   // clean up heap-allocated dyno
   delete dyno;
}

void DynamicObjectBinding::setName(const char* name)
{
   // FIXME:
}

const char* DynamicObjectBinding::getName()
{
   // FIXME:
}

void DynamicObjectBinding::setString(const char* str)
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   (*dyno) = str;
}

const char* DynamicObjectBinding::getString()
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->getString();
}

void DynamicObjectBinding::setNumber(const char* num)
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

const char* DynamicObjectBinding::getNumber()
{
   // FIXME: this will convert the dyno to a string
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->getString();
}

void DynamicObjectBinding::setBoolean(const char* b)
{
   DynamicObject* dyno = (DynamicObject*)getObject();
   (*dyno) = (strcmp(b, "true") == 0) ? true : false;
}

const char* DynamicObjectBinding::getBoolean()
{
   // FIXME: this will convert the dyno to a string
   DynamicObject* dyno = (DynamicObject*)getObject();
   return (*dyno)->getString();
}

void DynamicObjectBinding::getChildren(DataName* dn, list<void*>& children)
{
   // FIXME:
//   // get fields from Form
//   list<FormField*> fields;
//   Form* form = (Form*)getObject();
//   form->getFields(fields);
//   
//   for(list<FormField*>::iterator i = fields.begin();
//       i != fields.end(); i++)
//   {
//      children.push_back(*i);
//   }
}
