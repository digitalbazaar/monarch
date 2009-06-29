/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
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
      // parse element's local name and namespace
      char* ns;
      parseNamespace(&name, &ns);
      
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
      (*e)["namespace"] = (ns == NULL ? "" : ns);
      (*e)["data"] = "";
      (*e)["attributes"]->setType(Map);
      (*e)["children"]->setType(Map);
      
      if(ns != NULL)
      {
         free(ns);
      }
      
      // parse element attributes
      for(int i = 0; attrs[i] != NULL; i += 2)
      {
         // parse attribute's local name and namespace
         parseNamespace(&attrs[i], &ns);
         Attribute& attr = (*e)["attributes"][attrs[i]];
         attr["name"] = attrs[i];
         attr["namespace"] = (ns == NULL ? "" : ns);
         attr["value"] = attrs[i + 1];
         
         if(ns != NULL)
         {
            free(ns);
         }
         
         // add to element's namespace prefix map if is xmlns attribute
         const char* attrName = attr["name"]->getString();
         if(strncmp(attrName, "xmlns:", 6) == 0)
         {
            const char* uri = attr["value"]->getString();
            (*e)["namespacePrefixMap"]->setType(Map);
            (*e)["namespacePrefixMap"][uri] = attrName + 6;
         }
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

bool DomReader::finish()
{
   bool rval = XmlReader::finish();
   
   // no longer started
   mRootStarted = false;
   
   return rval;
}
