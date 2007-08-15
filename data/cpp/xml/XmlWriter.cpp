/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlWriter.h"

#include <list>

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;

XmlWriter::XmlWriter()
{
}

XmlWriter::~XmlWriter()
{
}

bool XmlWriter::write(DataBinding* db, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: going to need to know what data name is in use
   // for the current data binding so that it won't write its own
   // element tags out for self-data mappings or accidentally close
   // an element tag for text before child elements
   // 
   // going to need data mapping methods for retrieving single objects
   // and collections of objects (lists) (alternatively something funky
   // with the count of objects could be done but its probably too messy
   // for the objects)
   
   // iterate through all data names
   list<DataName*> dataNames = db->getDataNames();
   for(list<DataName*>::iterator i = dataNames.begin();
       i != dataNames.end(); i++)
   {
      // get data mapping
      DataMapping* dm = db->getDataMapping(*i);
      if(dm->isCreateMapping())
      {
         // write out start element
         
         // increase indentation
         
         // write out data binding
         write(db->getDataBinding(*(++i)), os);
         
         // decrease indentation
         
         // write out end element
      }
      else
      {
         // FIXME:
         // get object
         void* obj = NULL;
         
         // get data
         char* data;
         dm->getData(obj, &data);
         
         if((*i)->major)
         {
            // write out start element
            
            // write out data
            
            // write out end element
         }
         else
         {
            // write out attribute name and quote
            
            // write out data
            
            // close attribute quote
         }
      }
   }
   
   return rval;
}
