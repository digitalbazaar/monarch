/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/xml/DomReader.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::xml;
using namespace monarch::io;
using namespace monarch::rt;

DomReader::DomReader() :
   mRootStarted(false),
   mNamespacePrefixMap(NULL)
{
}

DomReader::~DomReader()
{
}

bool DomReader::start(Element& root)
{
   // create namespace prefix map
   mNamespacePrefixMap = DynamicObject();
   mNamespacePrefixMap->setType(Map);

   // start
   XmlReader::start(root);
   return true;
}

bool DomReader::finish()
{
   bool rval = XmlReader::finish();

   // free namespace prefix map
   mNamespacePrefixMap.setNull();

   // no longer started
   mRootStarted = false;

   return rval;
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

      // save namespace declarations as xmlns:prefix attributes
      {
         DynamicObjectIterator i = mNamespacePrefixMap.getIterator();
         while(i->hasNext())
         {
            DynamicObject& prefix = i->next();
            // create attribute: "xmlns:<prefix>"="<namespace-uri>"
            // but only include colon if prefix length > 0
            int len = prefix->length() + 7;
            char attrName[len];
            snprintf(attrName, len, "xmlns%s%s",
               prefix->length() > 0 ? ":" : "", prefix->getString());
            Attribute& attr = (*e)["attributes"][attrName];
            attr["name"] = attrName;
            attr["value"] = i->getName();
         }
         mNamespacePrefixMap->clear();
      }

      if(ns != NULL)
      {
         // free namespace string
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
            // free namespace string
            free(ns);
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

void DomReader::startNamespaceDeclaration(
   const XML_Char* prefix, const XML_Char* uri)
{
   // storage declaration info
   mNamespacePrefixMap[uri] = (prefix == NULL) ? "" : prefix;
}
