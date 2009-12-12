/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/xml/DomWriter.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/io/BufferedOutputStream.h"

using namespace std;
using namespace monarch::data;
using namespace monarch::data::xml;
using namespace monarch::io;
using namespace monarch::rt;

DomWriter::DomWriter()
{
   // Initialize to compact representation
   setCompact(true);
   setIndentation(0, 3);
}

DomWriter::~DomWriter()
{
}

bool DomWriter::write(Element& e, OutputStream* os, int level)
{
   DynamicObject nsPrefixMap;
   nsPrefixMap->setType(Map);
   return writeWithNamespaceSupport(e, os, level, nsPrefixMap);
}

bool DomWriter::writeWithNamespaceSupport(
   Element& e, OutputStream* os, int level, DynamicObject& nsPrefixMap)
{
   bool rval = true;

   if(level < 0)
   {
      level = mIndentLevel;
   }

   // add any entries to the current namespace prefix map
   if(e->hasMember("attributes") && e["attributes"]->getType() == Map)
   {
      AttributeIterator attrs = e["attributes"].getIterator();
      while(attrs->hasNext())
      {
         Attribute& attr = attrs->next();

         // check for an xml namespace prefix definition
         const char* attrName = attrs->getName();
         if(strncmp(attrName, "xmlns:", 6) == 0)
         {
            const char* uri = attr["value"]->getString();
            nsPrefixMap[uri] = attrName + 6;
         }
      }
   }

   // create element name, which may require using the namespace prefix map
   string elementName;
   if(e->hasMember("namespace"))
   {
      const char* ns = e["namespace"]->getString();
      if(nsPrefixMap->hasMember(ns))
      {
         // prepend prefix for given namespace
         elementName.append(nsPrefixMap[ns]->getString());
         elementName.push_back(':');
      }
   }
   elementName.append(e["name"]->getString());

   // open start element
   rval =
      ((mCompact || level == 0) ? true : os->write("\n", 1)) &&
      writeIndentation(os, level) &&
      os->write("<", 1) &&
      os->write(elementName.c_str(), elementName.length());

   // write attributes
   e["attributes"]->setType(Map);
   AttributeIterator attrs = e["attributes"].getIterator();
   while(rval && attrs->hasNext())
   {
      Attribute& attr = attrs->next();

      // create attribute name, which may require using the namespace prefix map
      string attrName;
      if(attr->hasMember("namespace"))
      {
         const char* ns = attr["namespace"]->getString();
         if(nsPrefixMap->hasMember(ns))
         {
            // prepend prefix for given namespace
            attrName.append(nsPrefixMap[ns]->getString());
            attrName.push_back(':');
         }
      }
      attrName.append(attr["name"]->getString());

      rval =
         os->write(" ", 1) &&
         os->write(attrName.c_str(), attrName.length()) &&
         os->write("=\"", 2) &&
         os->write(attr["value"]->getString(), attr["value"]->length()) &&
         os->write("\"", 1);
   }

   // close start element, if element is empty, use compact element
   e["data"]->setType(String);
   e["children"]->setType(Map);
   int dataLength = e["data"]->length();
   bool empty = (dataLength == 0 && e["children"]->length() == 0);
   rval = rval && (empty ? os->write("/>", 2) : os->write(">", 1));

   // write element data and children
   if(rval && !empty)
   {
      // write element children
      if(rval && e["children"]->length() > 0)
      {
         // serialize each child
         DynamicObjectIterator lists = e["children"].getIterator();
         while(rval && lists->hasNext())
         {
            DynamicObject& list = lists->next();
            list->setType(Array);
            ElementIterator children = list.getIterator();
            while(rval && children->hasNext())
            {
               // serialize child
               Element& child = children->next();
               rval = writeWithNamespaceSupport(
                  child, os, level + 1, nsPrefixMap);
            }
         }
      }

      // write element data
      if(dataLength > 0)
      {
         // xml-encode data
         string encoded = encode(e["data"]->getString());
         rval = os->write(encoded.c_str(), encoded.length());
      }

      // write end element
      // (only write indentation when data is blank and children are present)
      if(dataLength == 0 && !empty)
      {
         rval = rval &&
            (mCompact ? true : os->write("\n", 1)) &&
            writeIndentation(os, level);
      }

      rval = rval &&
         os->write("</", 2) &&
         os->write(elementName.c_str(), elementName.length()) &&
         os->write(">", 1);
   }

   return rval;
}

bool DomWriter::write(DynamicObject& root, OutputStream* os)
{
   return XmlWriter::write(root, os);
}
