/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlReader.h"
#include "IOException.h"

using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

XmlReader::XmlReader()
{
   // create parser
   mParser = XML_ParserCreateNS(NULL, '|');
   
   // set handlers
   XML_SetElementHandler(mParser, &startElement, &endElement);
}

XmlReader::~XmlReader()
{
   // free parser
   XML_ParserFree(mParser);
}
#include <iostream>
using namespace std;
void XmlReader::startElement(
   void* xr, const XML_Char* name, const XML_Char** attrs)
{
   cout << "start element=" << name << endl;
   // FIXME:
}

void XmlReader::endElement(void* reader, const XML_Char* name)
{
   cout << "end element=" << name << endl;
   // FIXME:
}

bool XmlReader::read(DataBinding* db, InputStream* is)
{
   bool rval = false;
   
   char* b = (char*)XML_GetBuffer(mParser, 2048);
   if(b != NULL)
   {
      int numBytes;
      while((numBytes = is->read(b, 2048)) != -1)
      {
         XML_ParseBuffer(mParser, numBytes, false);
      }
      
      // parse last
      XML_ParseBuffer(mParser, 0, true);
   }
   else
   {
      // set memory exception
      Exception::setLast(new IOException("Insufficient memory to parse xml!"));
   }
   
   return rval;
}
