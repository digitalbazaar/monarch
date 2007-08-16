/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlWriter.h"

#include <list>

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

XmlWriter::XmlWriter()
{
}

XmlWriter::~XmlWriter()
{
}

bool XmlWriter::openStartElement(DataName* dn, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   if(os->write("<", 1))
   {
      rval = os->write(dn->name, strlen(dn->name));
   }
   
   return rval;
}

bool XmlWriter::closeStartElement(bool empty, OutputStream* os)
{
   bool rval = false;
   
   if(empty)
   {
      rval = os->write("/>", 1);
      
      // FIXME: handle indentation
   }
   else
   {
      rval = os->write(">", 1);
      
      // FIXME: handle indentation
   }
   
   return rval;
}

bool XmlWriter::writeEndElement(DataName* dn, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   if(os->write("<", 1))
   {
      rval = os->write(dn->name, strlen(dn->name));
      
      // FIXME: handle indentation
   }
   
   return rval;
}

bool XmlWriter::writeElement(
   DataName* dn, const char* data, OutputStream* os)
{
   bool rval = false;
   
   // write out start element
   if(openStartElement(dn, os))
   {
      // check for empty element
      unsigned int length = strlen(data);
      if(length == 0)
      {
         // close empty start element
         rval = closeStartElement(true, os);
      }
      else
      {
         // write out data
         if(os->write(data, length))
         {
            // write out end element
            rval = writeEndElement(dn, os);
         }
      }
   }
   
   return rval;
}

bool XmlWriter::writeAttribute(
   DataName* dn, const char* data, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   
   // write out attribute name
   if(os->write(dn->name, strlen(dn->name)))
   {
      // write out equals and quote
      if(os->write("=\"", 2))
      {
         // write out data
         if(os->write(data, strlen(data)))
         {
            // close attribute quote
            rval = os->write("\"", 1);
         }
      }
   }
   
   return rval;
}

bool XmlWriter::write(DataBinding* db, OutputStream* os)
{
   bool rval = true;
   
   // get root data name
   DataName* root = db->getDataName();
   if(root == NULL)
   {
      Exception::setLast(new IOException("No root element for DataBinding!"));
      rval = false;
   }
   else
   {
      // open start element
      openStartElement(root, os);
      
      // set to false once the start element has been closed
      bool startElementOpen = true;
      
      // a list for storing child objects
      list<void*> children;
      
      // iterate through all data names
      list<DataName*> dataNames = db->getDataNames();
      for(list<DataName*>::iterator i = dataNames.begin();
          rval && i != dataNames.end(); i++)
      {
         // get data mapping
         DataMapping* dm = db->getDataMapping(*i);
         
         // see if the mapping is for child objects
         if(dm->isChildMapping())
         {
            // get list of child objects
            children.clear();
            db->getChildren(*i, children);
            
            // close start element if open and children are present (lol)
            if(startElementOpen && !children.empty())
            {
               rval = closeStartElement(false, os);
            }
            
            // get data binding
            DataBinding* binding = db->getDataBinding(*(++i));
            
            // use binding for each child
            for(list<void*>::iterator child = children.begin();
                rval && child != children.end(); child++)
            {
               // set child, write out data
               binding->setObject(*child);
               rval = write(binding, os);
            }
         }
         else
         {
            // get data
            char* data;
            dm->getData(db->getObject(), &data);
            
            if((*i)->major)
            {
               // close start element if open
               if(startElementOpen)
               {
                  closeStartElement(false, os);
               }
               
               // write element
               rval = writeElement(*i, data, os);
            }
            else
            {
               // write attribute
               rval = writeAttribute(*i, data, os);
            }
            
            // clean up data
            delete [] data;
         }
      }
      
      if(rval)
      {
         // close start element as appropriate
         if(startElementOpen)
         {
            rval = closeStartElement(true, os);
         }
         else
         {
            // write end element
            rval = writeEndElement(root, os);
         }
      }
   }
   
   return rval;
}
