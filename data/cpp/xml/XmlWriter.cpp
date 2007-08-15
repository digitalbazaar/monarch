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
   
   // iterate through all data names
   list<DataName*> dataNames = db->getDataNames();
   for(list<DataName*>::iterator i = dataNames.begin();
       i != dataNames.end(); i++)
   {
//      // get data mapping
//      DataMapping* dm = db->getDataMapping(*i);
//      if(dm->isCreateMapping())
//      {
//         // write out data binding
//         write(db->getDataBinding(*(++i)), os);
//      }
//      else
//      {
//         // FIXME:
//         // get object
//         void* obj = NULL;
//         
//         // get data
//         char* data;
//         dm->getData(obj, data);
//         
//         // FIXME:
//         // is it an element or an attribute?
//      }
   }
   
   return rval;
}
