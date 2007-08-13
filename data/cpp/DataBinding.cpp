/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DataBinding.h"

using namespace std;
using namespace db::data;

DataBinding::DataBinding()
{
   mCurrentDataName = NULL;
}

DataBinding::~DataBinding()
{
}

bool DataBinding::DataNameComparator::operator()(
   DataName* dn1, DataName* dn2) const
{
   bool rval = false;
   
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
   
   return rval;
}

DataBinding::DataName* DataBinding::createDataName(
   const char* ns, const char* name)
{
   DataName* dn = new DataName();
   
   if(ns != NULL)
   {
      dn->ns = new char[strlen(ns) + 1];
      strcpy(dn->ns, ns);
   }
   else
   {
      dn->ns = NULL;
   }
   
   dn->name = new char[strlen(name) + 1];
   strcpy(dn->name, name);
   
   return dn;
}

void DataBinding::freeDataName(DataName* dn)
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

void DataBinding::addDataMapping(
   const char* ns, const char* name, DataMapping* dm)
{
   // create DataName
   DataName* dn = createDataName(ns, name);
   
   // find existing data mapping
   map<DataName*, DataMapping*, DataNameComparator>::iterator i =
      mDataMappings.find(dn);
   if(i != mDataMappings.end())
   {
      // free new data name, use old data name
      freeDataName(dn);
      dn = i->first;
   }
   
   // set data mapping
   mDataMappings[dn] = dm;
}

void DataBinding::addDataBinding(
   const char* ns, const char* name, DataBinding* db)
{
   // create DataName
   DataName* dn = createDataName(ns, name);
   
   // find existing data binding
   map<DataName*, DataBinding*, DataNameComparator>::iterator i =
      mDataBindings.find(dn);
   if(i != mDataBindings.end())
   {
      // delete new data name, use old data name
      delete dn;
      dn = i->first;
   }
   
   // set data binding
   mDataBindings[dn] = db;
}

#include <iostream>
using namespace std;
DataBinding* DataBinding::startData(
   const char* charEncoding, const char* ns, const char* name) 
{
   DataBinding* rval = NULL;
   mCurrentDataName = NULL;
   
   // FIXME: remove printouts
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
   
   // create DataName for look up
   DataName dn;
   dn.ns = (char*)ns;
   dn.name = (char*)name;
   
   // find data binding
   map<DataName*, DataBinding*, DataNameComparator>::iterator i =
      mDataBindings.find(&dn);
   if(i != mDataBindings.end())
   {
      // store current data name and return binding
      mCurrentDataName = i->first;
      rval = i->second;
      
      // clear old data
      char data[1];
      memset(data, 0, 1);
      rval->setData(charEncoding, ns, name, data, 0);
   }
   
   return rval;
}

void DataBinding::appendData(
   const char* charEncoding, char* data, unsigned int length)
{
   if(mCurrentDataName != NULL)
   {
      // find data mapping
      map<DataName*, DataMapping*, DataNameComparator>::iterator i =
         mDataMappings.find(mCurrentDataName);
      if(i != mDataMappings.end())
      {
         // get data mapping
         DataMapping* dm = i->second;
         
         // get old data
         const char* oldData = dm->getData();
         unsigned int oldLength = strlen(oldData);
         
         // set new data
         char d[oldLength + length + 1];
         strncpy(d, oldData, oldLength);
         strncpy(d + oldLength, data, length);
         memset(d + oldLength + length, 0, 1);
         dm->setData(d);
         
         // FIXME: remove printout
         cout << "appending data='" << d << "'" << endl;
      }
   }
}

void DataBinding::endData(
   const char* charEncoding, const char* ns, const char* name, DataBinding* db)
{
   // FIXME: remove printouts
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
   
   // FIXME: add object in data binding
   
   // FIXME:
}

void DataBinding::setData(
   const char* charEncoding, const char* ns, const char* name,
   const char* data, unsigned int length)
{
   // create DataName for look up
   DataName dn;
   dn.ns = (char*)ns;
   dn.name = (char*)name;
   
   // find data mapping
   map<DataName*, DataMapping*, DataNameComparator>::iterator i =
      mDataMappings.find(&dn);
   if(i != mDataMappings.end())
   {
      // get mapping
      DataMapping* dm = i->second;
      
      // set new data
      char d[length + 1];
      strncpy(d, data, length);
      memset(d + length, 0, 1);
      dm->setData(d);
      
      // FIXME: remove printout
      cout << "setting data='" << d << "'" << endl;
   }
}
