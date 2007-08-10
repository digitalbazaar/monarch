/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlReader.h"

using namespace db::data;
using namespace db::data::xml;
using namespace db::io;

XmlReader::XmlReader()
{
   // create parser
   mParser = XML_ParserCreate(NULL);
}

XmlReader::~XmlReader()
{
   // free parser
   XML_ParserFree(mParser);
}

bool XmlReader::read(DataBinding* db, InputStream* is)
{
   bool rval = false;
   
   // FIXME: implement me
   
   return rval;
}
