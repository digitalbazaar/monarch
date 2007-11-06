/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectReader.h"
#include "db/util/DynamicObjectIterator.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectReader::DynamicObjectReader()
{
}

DynamicObjectReader::~DynamicObjectReader()
{
}

void DynamicObjectReader::read(DynamicObject dyno, DataBinding* db)
{
   // go through data bindings and get data from dynamic object
   // according to data names
   
   // notify binding of deserialization process
   db->deserializationStarted();
   
   // iterate through all data names
   list<DataName*> dataNames = db->getDataNames();
   for(list<DataName*>::iterator i = dataNames.begin();
       i != dataNames.end(); i++)
   {
      // get data mapping and basic name
      DataMapping* dm = db->getDataMapping(*i);
      const char* name = (*i)->name;
      
      // see if the mapping is for child objects
      if(dm->isChildMapping())
      {
         // get child data binding
         DataBinding* binding = db->getDataBinding(*(++i));
         
         // ensure member exists
         if(dyno->hasMember(name))
         {
            if(dyno[name]->getType() == Array)
            {
               // create and add children for each element
               DynamicObjectIterator doi = dyno[name].getIterator();
               while(doi->hasNext())
               {
                  void* child = dm->createChild(db->getCreateAddObject(*i));
                  binding->setObject(child);
                  read(doi->next(), binding);
                  dm->addChild(db->getCreateAddObject(*i), child);
               }
            }
            else
            {
               // create child for member
               void* child = dm->createChild(db->getCreateAddObject(*i));
               binding->setObject(child);
               read(dyno[name], binding);
               dm->addChild(db->getCreateAddObject(*i), child);
            }
         }
      }
      else
      {
         // set regular object data based on data type
         bool b;
         int int32;
         unsigned uint32;
         long long int64;
         unsigned long long uint64;
         switch(dm->getDataType())
         {
            case DataMapping::String:
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)dyno[name]->getString(), dyno[name]->length());
               break;
            case DataMapping::Boolean:
               b = dyno[name]->getBoolean();
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)&b, dyno[name]->length());
               break;
            case DataMapping::Int32:
               int32 = dyno[name]->getInt32();
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)&int32, dyno[name]->length());
               break;
            case DataMapping::UInt32:
               uint32 = dyno[name]->getUInt32();
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)&uint32, dyno[name]->length());
               break;
            case DataMapping::Int64:
               int64 = dyno[name]->getInt64();
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)&int64, dyno[name]->length());
               break;
            case DataMapping::UInt64:
               uint64 = dyno[name]->getUInt64();
               dm->setRawData(
                  db->getSetGetObject(*i),
                  (char*)&uint64, dyno[name]->length());
               break;
         }
      }
   }
}   
