/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/XmlBindingInputStream.h"

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

XmlBindingInputStream::XmlBindingInputStream(DataBinding* db, int bufferSize) :
   mReadBuffer(bufferSize),
   mReadBufferStream(&mReadBuffer, false),
   mIgnoreStream(&mReadBufferStream, false)
{
   // create read state for root data binding
   ReadState rs;
   populateReadState(rs, db);
   mStateStack.push_front(rs);
   mElementDataPending = false;
}

XmlBindingInputStream::~XmlBindingInputStream()
{
   // clean up all read state children lists
   for(list<ReadState>::iterator i = mStateStack.begin();
       i != mStateStack.end(); i++)
   {
      freeReadState(*i);
   }
}

void XmlBindingInputStream::populateReadState(ReadState& rs, DataBinding* db)
{
   // set data binding, not started read yet
   rs.db = db;
   rs.started = false;
   
   // notify binding of serialization start
   db->serializationStarted();
      
   // get initial data name
   rs.dn = db->getDataNames().begin();
   
   // no children yet
   rs.children = NULL;
}

void XmlBindingInputStream::freeReadState(ReadState& rs)
{
   if(rs.children != NULL)
   {
      delete rs.children;
      rs.children = NULL;
   }
}

void XmlBindingInputStream::writeElementData(ReadState& rs)
{
   // write out remaining data to read buffer
   if(mXmlWriter.writeElementData(
      rs.db->getDataMapping(*rs.dn), rs.db->getSetGetObject(*rs.dn),
      &mIgnoreStream))
   {
      // write out end element if dealing with non-root
      if(!rs.db->getDataName()->equals(*rs.dn))
      {
         mXmlWriter.writeEndElement(&mIgnoreStream);
      }
      
      // element data no longer pending
      mElementDataPending = false;
      
      // increment data name
      rs.dn++;
   }
   else
   {
      // set ignore count, get remaining unwritten bytes
      IOException* e = dynamic_cast<IOException*>(Exception::getLast());
      if(e != NULL)
      {
         mIgnoreStream.setIgnoreCount(e->getUsedBytes());
         mElementDataPending = true;
         
         // clear exception
         Exception::clearLast();
      }
   }
}

int XmlBindingInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // clear last exception
   Exception* last = Exception::getLast();
   Exception::clearLast(false);
   
   // read from buffer as much as possible
   if(!mReadBuffer.isEmpty())
   {
      rval = mReadBuffer.get(b, length);
   }
   else if(!mStateStack.empty())
   {
      // get current read state
      ReadState* rs = &mStateStack.front();
      
      // start read if not started
      if(!rs->started)
      {
         // get root data name
         DataName* root = rs->db->getDataName();
         if(root == NULL)
         {
            Exception::setLast(
               new IOException("No root element for DataBinding!"));
            rval = -1;
         }
         else
         {
            // write start element
            mXmlWriter.writeStartElement(root, &mIgnoreStream);
            rs->started = true;
         }
      }
      else if(mElementDataPending)
      {
         // write out element data
         writeElementData(*rs);
      }
      else
      {
         // get current data name
         if(rs->dn == rs->db->getDataNames().end())
         {
            // free read state and pop off stack
            freeReadState(*rs);
            mStateStack.pop_front();
            
            // no more data names, write end element
            mXmlWriter.writeEndElement(&mIgnoreStream);
            
            if(!mStateStack.empty())
            {
               // if element was a child, increment child
               rs = &mStateStack.front();
               if(rs->children != NULL)
               {
                  rs->child++;
               }
            }
         }
         else
         {
            // check data mapping for data name
            DataMapping* dm = rs->db->getDataMapping(*rs->dn);
            if(!dm->isChildMapping())
            {
               if(rs->db->getDataName()->equals(*rs->dn))
               {
                  // write out element data for root element
                  writeElementData(*rs);
               }
               else
               {
                  // only write element if it is verbose or has data
                  if((*rs->dn)->verbose ||
                     dm->hasData(rs->db->getSetGetObject(*rs->dn)))
                  {
                     if((*rs->dn)->major)
                     {
                        // write start element, indicate data is pending
                        mXmlWriter.writeStartElement(*rs->dn, &mIgnoreStream);
                        mElementDataPending = true;
                     }
                     else
                     {
                        // write attribute
                        mXmlWriter.writeAttribute(
                           *rs->dn, dm, rs->db->getSetGetObject(*rs->dn),
                           &mIgnoreStream);
                        
                        // increment data name
                        rs->dn++;
                     }
                  }
                  else
                  {
                     // skip data name, it has no data and isn't verbose
                     rs->dn++;
                  }
               }
            }
            else
            {
               // get children if they do not already exist
               if(rs->children == NULL)
               {
                  rs->children = new list<void*>();
                  rs->db->getChildren(*rs->dn, *rs->children);
                  rs->child = rs->children->begin();
                  
                  // increment data name
                  rs->dn++;
               }
               
               // get current child
               if(rs->child != rs->children->end())
               {
                  // get data binding for child
                  DataBinding* db = rs->db->getDataBinding(*rs->dn);
                  
                  // set data binding's object to child
                  db->setObject(*rs->child);
                  
                  // create read state and push onto stack
                  ReadState rs2;
                  populateReadState(rs2, db);
                  mStateStack.push_front(rs2);
                  
                  // recurse
                  rval = read(b, length);
               }
               else
               {
                  // increment data name
                  rs->dn++;
                  
                  // clean up children
                  delete rs->children;
                  rs->children = NULL;
               }
            }
         }
      }
      
      if(rval == 0 && !Exception::hasLast())
      {
         if(!mReadBuffer.isEmpty())
         {
            // read from read buffer
            rval = mReadBuffer.get(b, length);
         }
         else if(!mStateStack.empty())
         {
            // if no data has been written to the read buffer, recurse to
            // write out more xml
            rval = read(b, length);
         }
      }
   }
   
   // restore old exception
   if(!Exception::hasLast())
   {
      Exception::setLast(last);
   }
   else if(last != NULL)
   {
      // clean up old exception
      delete last;
   }
   
   return rval;
}

void XmlBindingInputStream::setIndentation(int level, int spaces)
{
   mXmlWriter.setIndentation(level, spaces);
}
