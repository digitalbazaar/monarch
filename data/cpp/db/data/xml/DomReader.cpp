/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/DomReader.h"

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

DomReader::DomReader()
{
   mRootStarted = false;
}

DomReader::~DomReader()
{
}

void DomReader::startElement(const XML_Char* name, const XML_Char** attrs)
{
   if(mException.isNull() && !mDynoStack.empty())
   {
      // parse element's local name
      parseLocalName(&name);
      
      Element* e;
      if(mRootStarted)
      {
         // create child element and push stack
         e = &(*mDynoStack.front())["children"][name]->append();
         mDynoStack.push_front(e);
      }
      else
      {
         // update root element
         e = &(*mDynoStack.front());
         mRootStarted = true;
      }
      
      // initialize element
      (*e)["name"] = name;
      (*e)["data"] = "";
      (*e)["attributes"]->setType(Map);
      (*e)["children"]->setType(Map);
      
      // parse element attributes
      for(int i = 0; attrs[i] != NULL; i += 2)
      {
         // parse attribute's local name
         parseLocalName(&attrs[i]);
         (*e)["attributes"][attrs[i]] = attrs[i + 1];
      }
   }
}

void DomReader::endElement(const XML_Char* name)
{
   if(mException.isNull() && !mDynoStack.empty())
   {
      // pop stack
      mDynoStack.pop_front();
   }
}

void DomReader::appendData(const XML_Char* data, int length)
{
   if(mException.isNull() && !mDynoStack.empty())
   {
      // append data to element
      Element& e = *mDynoStack.front();
      const char* d = e["data"]->getString();
      int len = strlen(d);
      char temp[len + length + 1];
      memcpy(temp, d, len);
      memcpy(temp + len, data, length);
      temp[len + length] = 0;
      e["data"] = temp;
   }
}

void DomReader::start(DynamicObject& root)
{
   XmlReader::start(root);
}

bool DomReader::finish()
{
   bool rval = XmlReader::finish();
   
   // no longer started
   mRootStarted = false;
   
   return rval;
}
