/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/XmlWriter.h"

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

XmlWriter::XmlWriter()
{
   mIndentLevel = 0;
   mIndentSpaces = 0;
}

XmlWriter::~XmlWriter()
{
}

void XmlWriter::reset()
{
   // clear element stack
   mElementStack.clear();
}

bool XmlWriter::writeIndentation(OutputStream* os, bool endElement)
{
   bool rval = true;
   
   // write out indentation, if any
   int indent = mIndentLevel + (mElementStack.size() - 1) * mIndentSpaces;
   
   // ensure to indent end elements on indent level
   if(indent > 0 ||
      (endElement && indent == mIndentLevel && mIndentSpaces > 0))
   {
      char temp[indent + 1];
      memset(temp, '\n', 1);
      memset(temp + 1, ' ', indent);
      rval = os->write(temp, indent + 1);
   }
   
   return rval;
}

bool XmlWriter::writeStartElement(DataName* dn, OutputStream* os)
{
   bool rval = true;
   
   if(!mElementStack.empty())
   {
      // get current element state
      ElementState& es = mElementStack.front();
      
      // close start element as appropriate
      if(es.open)
      {
         rval = os->write(">", 1);
         es.open = false;
      }
   }
   
   // create element state
   ElementState es;
   es.dn = dn;
   es.hasData = false;
   es.open = true;
   
   // add state to element stack
   mElementStack.push_front(es);
   
   // write out indentation
   rval = writeIndentation(os, false);
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   if(rval && (rval = os->write("<", 1)))
   {
      rval = os->write(es.dn->name, strlen(es.dn->name));
   }
   
   return rval;
}

bool XmlWriter::writeEndElement(OutputStream* os)
{
   bool rval = false;
   
   if(!mElementStack.empty())
   {
      // get current element state
      ElementState& es = mElementStack.front();
      
      // write closing element
      if(es.open)
      {
         rval = os->write("/>", 2);
      }
      else
      {
         rval = true;
         
         // write indentation as appropriate
         if(!es.hasData)
         {
            rval = writeIndentation(os, true);
         }
         
         // FIXME: need a namespace/prefix table/interface
         // FIXME: need to write out namespace prefix as well
         if(rval && (rval = os->write("</", 2)))
         {
            if((rval = os->write(es.dn->name, strlen(es.dn->name))))
            {
               rval = os->write(">", 1);
            }
         }
      }
      
      // pop element state off stack
      mElementStack.pop_front();
   }
   
   return rval;
}

bool XmlWriter::writeAttribute(
   DataName* dn, const char* data, int length, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   
   // write out space
   if(os->write(" ", 1))
   {
      // write out attribute name
      if(os->write(dn->name, strlen(dn->name)))
      {
         // write out equals and quote
         if(os->write("=\"", 2))
         {
            // write out data
            if(os->write(data, length))
            {
               // close attribute quote
               rval = os->write("\"", 1);
            }
         }
      }
   }
   
   return rval;
}

bool XmlWriter::writeAttribute(
   DataName* dn, DataMapping* dm, void* obj, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: need a namespace/prefix table/interface
   // FIXME: need to write out namespace prefix as well
   
   // write out space
   if(os->write(" ", 1))
   {
      // write out attribute name
      if(os->write(dn->name, strlen(dn->name)))
      {
         // write out equals and quote
         if(os->write("=\"", 2))
         {
            // write out data
            if(dm->writeData(obj, os))
            {
               // close attribute quote
               rval = os->write("\"", 1);
            }
         }
      }
   }
   
   return rval;
}

bool XmlWriter::writeElementData(const char* data, int length, OutputStream* os)
{
   bool rval = true;
   
   // get current element state
   ElementState& es = mElementStack.front();
   
   // set whether or not element has data
   es.hasData = (length > 0);
   
   // close start element as appropriate
   if(es.open && es.hasData)
   {
      rval = os->write(">", 1);
      es.open = false;
   }
   
   if(rval)
   {
      // write out data
      rval = os->write(data, length);
   }
   
   return rval;
}

bool XmlWriter::writeElementData(
   DataMapping* dm, void* obj, OutputStream* os)
{
   bool rval = true;
   
   // get current element state
   ElementState& es = mElementStack.front();
   
   // set whether or not element has data
   es.hasData = dm->hasData(obj);
   
   // close start element as appropriate
   if(es.open && es.hasData)
   {
      rval = os->write(">", 1);
      es.open = false;
   }
   
   if(rval)
   {
      // write out data
      rval = dm->writeData(obj, os);
   }
   
   return rval;
}

bool XmlWriter::write(DataBinding* db, OutputStream* os)
{
   bool rval = true;
   
   // notify binding of serialization process
   db->serializationStarted();
   
   // get root data name
   DataName* root = db->getDataName();
   if(root == NULL)
   {
      Exception::setLast(new IOException("No root element for DataBinding!"));
      rval = false;
   }
   else
   {
      // write start element
      writeStartElement(root, os);
      
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
            // only write content for root element
            if(root->equals(*i))
            {
               rval = writeElementData(dm, db->getObject(), os);
            }
            else
            {
               // write entire element or attribute
               if((*i)->major)
               {
                  // write start element, data, end element
                  rval = 
                     writeStartElement(*i, os) &&
                     writeElementData(dm, db->getObject(), os) &&
                     writeEndElement(os);
               }
               else
               {
                  // write attribute
                  rval = writeAttribute(*i, dm, db->getObject(), os);
               }
            }
         }
      }
      
      if(rval)
      {
         // write end element
         rval = writeEndElement(os);
      }
   }
   
   return rval;
}

void XmlWriter::setIndentation(int level, int spaces)
{
   mIndentLevel = level;
   mIndentSpaces = spaces;
}
