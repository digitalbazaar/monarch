/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DataBinding.h"

using namespace std;
using namespace db::data;

bool DataName::equals(DataName* dn)
{
   bool rval = false;
   
   if(ns == NULL && dn->ns == NULL)
   {
      // compare names
      rval = (strcmp(name, dn->name) == 0);
   }
   else if(ns != NULL && dn->ns != NULL)
   {
      // compare namespaces
      if(strcmp(ns, dn->ns) == 0)
      {
         // compare names
         rval = (strcmp(name, dn->name) == 0);
      }
   }
   
   return rval;
}

DataBinding::DataBinding(void* obj)
{
   mObject = obj;
   mRootDataName = NULL;
}

DataBinding::~DataBinding()
{
   // clean up all data mappings
   for(DataMappingMap::iterator i = mDataMappings.begin();
       i != mDataMappings.end(); i++)
   {
      // free data name
      freeDataName(i->first);
   }
   
   // clean up all data bindings
   for(DataBindingMap::iterator i = mDataBindings.begin();
       i != mDataBindings.end(); i++)
   {
      // free data name
      freeDataName(i->first);
   }
   
   // free root data name
   freeDataName(mRootDataName);
   
   // free current data name stack
   for(DataNameList::iterator i = mDataNameStack.begin();
       i != mDataNameStack.end(); i++)
   {
      freeDataName(*i);
   }
}

bool DataBinding::DataNameComparator::operator()(
   DataName* dn1, DataName* dn2) const
{
   bool rval = false;
   
   if(dn1 != NULL && dn2 != NULL)
   {
      if(dn1->ns == NULL && dn2->ns != NULL)
      {
         rval = true;
      }
      else if(dn1->ns != NULL && dn2->ns != NULL)
      {
         // compare namespaces
         int i = strcmp(dn1->ns, dn2->ns);
         if(i > 0)
         {
            rval = true;
         }
         else if(i == 0)
         {
            // compare names
            rval = strcmp(dn1->name, dn2->name) < 0;
         }
      }
      else if(dn1->ns == NULL && dn2->ns == NULL)
      {
         // compare names
         rval = strcmp(dn1->name, dn2->name) < 0;
      }
   }
   
   return rval;
}

DataName* DataBinding::createDataName(
   const char* ns, const char* name, bool major, bool verbose)
{
   DataName* dn = new DataName();
   dn->major = major;
   dn->verbose = verbose;
   
   if(ns != NULL)
   {
      dn->ns = strdup(ns);
   }
   else
   {
      dn->ns = NULL;
   }
   
   dn->name = strdup(name);
   
   return dn;
}

void DataBinding::freeDataName(DataName* dn)
{
   if(dn != NULL)
   {
      // delete name space if exists
      if(dn->ns != NULL)
      {
         delete [] dn->ns;
      }
      
      // delete name
      delete [] dn->name;
      
      // delete data name
      delete dn;
   }
}

void* DataBinding::getCreateAddObject(DataName* dn)
{
   // return mObject by default
   return mObject;
}

void* DataBinding::getSetGetObject(DataName* dn)
{
   // return mObject by default
   return mObject;
}

void DataBinding::addDataMapping(
   const char* ns, const char* name, bool major, bool verbose, DataMapping* dm)
{
   // create data name
   DataName* dn = createDataName(ns, name, major, verbose);
   
   // set data mapping
   mDataMappings[dn] = dm;
   
   // add data name to order
   mDataNameOrder.push_back(dn);
}

void DataBinding::removeDataMapping(const char* ns, const char* name)
{
   // create data name
   DataName* dn = createDataName(ns, name, true, true);
   
   // remove data mapping
   mDataMappings.erase(dn);
   for(DataNameList::iterator i = mDataNameOrder.begin();
       i != mDataNameOrder.end(); i++)
   {
      if((*i)->equals(dn))
      {
         mDataNameOrder.erase(i);
         break;
      }
   }
   
   // clean up data name
   freeDataName(dn);
}

void DataBinding::addDataBinding(
   const char* ns, const char* name, DataBinding* db)
{
   // create data name
   DataName* dn = createDataName(ns, name, true, true);
   
   // set data binding
   mDataBindings[dn] = db;
   
   // add data name to order
   mDataNameOrder.push_back(dn);
}

void DataBinding::removeDataBinding(const char* ns, const char* name)
{
   // create data name
   DataName* dn = createDataName(ns, name, true, true);
   
   // remove data binding
   mDataBindings.erase(dn);
   for(DataNameList::iterator i = mDataNameOrder.begin();
       i != mDataNameOrder.end(); i++)
   {
      if((*i)->equals(dn))
      {
         mDataNameOrder.erase(i);
         break;
      }
   }
   
   // clean up data name
   freeDataName(dn);
}

DataBinding* DataBinding::startData(
   const char* charEncoding, const char* ns, const char* name) 
{
   DataBinding* rval = NULL;
   
   // create data name
   DataName* dn = createDataName(ns, name, true, true);
   
   // get data binding
   rval = getDataBinding(dn);
   if(rval != NULL)
   {
      // get data mapping, create child if appropriate
      DataMapping* dm = getDataMapping(dn);
      if(dm != NULL && dm->isChildMapping())
      {
         // create new child object for the data binding
         rval->mObject = dm->createChild(getCreateAddObject(dn));
      }
   }
   else
   {
      // use self for binding, no child to create
      rval = this;
   }
   
   // add new data name to stack
   rval->mDataNameStack.push_front(dn);
   
   return rval;
}

void DataBinding::appendData(
   const char* charEncoding, const char* data, unsigned int length)
{
   // get current data name
   DataName* dn = getCurrentDataName();
   
   // get data mapping
   DataMapping* dm = getDataMapping(dn);
   if(dm != NULL)
   {
      // append data
      dm->appendData(getSetGetObject(dn), data, length);
   }
}

void DataBinding::endData(
   const char* charEncoding, const char* ns, const char* name, DataBinding* db)
{
   // get data binding's current data name
   DataName* dn = db->getCurrentDataName();
   DataMapping* dm = db->getDataMapping(dn);
   if(dm != NULL && !dm->isChildMapping())
   {
      // end data
      dm->endData(db->getSetGetObject(dn));
   }
   
   // get data binding's data mapping, add child if appropriate
   dm = getDataMapping(db->getDataName());
   if(dm != NULL && dm->isChildMapping())
   {
      // add child object
      dm->addChild(getCreateAddObject(db->getDataName()), db->mObject);
   }
   
   // clean up current data name on stack, if any
   if(!db->mDataNameStack.empty())
   {
      freeDataName(db->mDataNameStack.front());
      db->mDataNameStack.pop_front();
   }
}

void DataBinding::setData(
   const char* charEncoding, const char* ns, const char* name,
   const char* data, unsigned int length)
{
   // create data name for look up
   DataName dn;
   dn.ns = (char*)ns;
   dn.name = (char*)name;
   
   DataMapping* dm = getDataMapping(&dn);
   if(dm != NULL)
   {
      // set data
      dm->setData(getSetGetObject(&dn), data, length);
   }
}

void DataBinding::setObject(void* obj)
{
   mObject = obj;
}

void* DataBinding::getObject()
{
   return mObject;
}

void DataBinding::serializationStarted()
{
   // no default implementation
}

void DataBinding::deserializationStarted()
{
   // no default implementation
}

void DataBinding::setDataName(const char* ns, const char* name)
{
   freeDataName(mRootDataName);
   mRootDataName = createDataName(ns, name, true, true);
}

DataName* DataBinding::getDataName()
{
   return mRootDataName;
}

DataName* DataBinding::getCurrentDataName()
{
   return (mDataNameStack.empty()) ? NULL : mDataNameStack.front();
}

void DataBinding::getChildren(DataName* dn, list<void*>& children)
{
   // base class has no children to populate
}

DataMapping* DataBinding::getDataMapping(DataName* dn)
{
   DataMapping* rval = NULL;
   
   // find data mapping
   DataMappingMap::iterator i = mDataMappings.find(dn);
   if(i != mDataMappings.end())
   {
      rval = i->second;
   }
   
   return rval;
}

DataBinding* DataBinding::getDataBinding(DataName* dn)
{
   DataBinding* rval = NULL;
   
   // find data binding
   DataBindingMap::iterator i = mDataBindings.find(dn);
   if(i != mDataBindings.end())
   {
      rval = i->second;
   }
   
   return rval;
}

list<DataName*>& DataBinding::getDataNames()
{
   return mDataNameOrder;
}
