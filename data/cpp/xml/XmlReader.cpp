/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlReader.h"
#include "expat.h"

using namespace db::data;
using namespace db::data::xml;
using namespace db::io;

XmlReader::XmlReader()
{
   // FIXME: just test code, remove this
   XML_Parser p = XML_ParserCreate(NULL);
   XML_ParserFree(p);
}

XmlReader::~XmlReader()
{
}

bool XmlReader::read(DataBinding* db, InputStream* is)
{
   bool rval = false;
   
   // FIXME: implement me
   
   return rval;
}
