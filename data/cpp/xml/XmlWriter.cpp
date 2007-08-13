/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "xml/XmlWriter.h"

using namespace db::data;
using namespace db::data::xml;
using namespace db::io;

XmlWriter::XmlWriter()
{
}

XmlWriter::~XmlWriter()
{
}

//void XmlWriter::parseNamespace(const char** name, char** ns)
//{
//   // parse namespace, if one exists
//   *ns = NULL;
//   const char* sep = strchr(*name, '|');
//   if(sep != NULL)
//   {
//      *ns = new char[sep - *name];
//      strncpy(*ns, *name, sep - *name);
//      memset(ns + (sep - *name), 0, 1);
//      *name = sep + 1;
//   }
//}

bool XmlWriter::write(DataBinding* db, OutputStream* os)
{
   bool rval = false;
   
   // FIXME: implement me
   
   return rval;
}
