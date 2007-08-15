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
   
   // FIXME: need a namespace/prefix table/interface
   
   // iterate through all data names
   list<DataName*> dataNames = db->getDataNames();
   list<void*> children;
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
         for(list<void*>::iterator child = children.begin();
             child != children.end(); child++)
         {
            // write out start element
            
            // increase indentation
            
            // set child, write out data
            binding->setObject(*child);
            write(binding, os);
            
            // decrease indentation
            
            // write out end element
         }
      }
      else
      {
         // get data
         char* data;
         dm->getData(db->getObject(), &data);
         
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
