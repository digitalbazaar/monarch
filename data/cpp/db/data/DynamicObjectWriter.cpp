/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DynamicObjectWriter.h"

using namespace std;
using namespace db::data;
using namespace db::util;

DynamicObjectWriter::DynamicObjectWriter()
{
}

DynamicObjectWriter::~DynamicObjectWriter()
{
}

DynamicObject DynamicObjectWriter::write(DataBinding* db)
{
   DynamicObject rval;
   
   // notify binding of serialization process
   db->serializationStarted();
   
   // a list for storing child objects
   list<void*> children;
   
   // iterate through all data names
   list<DataName*> dataNames = db->getDataNames();
   for(list<DataName*>::iterator i = dataNames.begin();
       i != dataNames.end(); i++)
   {
      // get data mapping
      DataMapping* dm = db->getDataMapping(*i);
      
      // see if the mapping is for child objects
      if(dm->isChildMapping())
      {
         // get list of child objects
         children.clear();
         db->getChildren(*i, children);
         
         // get data binding
         DataBinding* binding = db->getDataBinding(*(++i));
         
         // use binding for each child
         if(children.size() > 1)
         {
            // build array
            int count = 0;
            for(list<void*>::iterator child = children.begin();
                child != children.end(); child++, count++)
            {
               // set child, write out to DynamicObject
               binding->setObject(*child);
               rval[(*i)->name][count] = write(binding);
            }
         }
         else
         {
            // build single object
            binding->setObject(*children.begin());
            rval[(*i)->name] = write(binding);
         }
      }
      else
      {
         // set object members based on data type
         char* data;
         switch(dm->getDataType())
         {
            case DataMapping::String:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = data;
               break;
            case DataMapping::Boolean:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = *((bool*)data);
               break;
            case DataMapping::Int32:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = *((int*)data);
               break;
            case DataMapping::UInt32:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = *((unsigned int*)data);
               break;
            case DataMapping::Int64:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = *((long long*)data);
               break;
            case DataMapping::UInt64:
               dm->getRawData(db->getObject(), &data);
               rval[(*i)->name] = *((unsigned long long*)data);
               break;
         }
         
         // free allocated data
         free(data);
      }
   }
   
   return rval;
}
