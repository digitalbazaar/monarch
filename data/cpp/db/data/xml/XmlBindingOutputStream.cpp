/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/XmlBindingOutputStream.h"

using namespace db::data;
using namespace db::data::xml;
using namespace db::io;

XmlBindingOutputStream::XmlBindingOutputStream(DataBinding* db) :
   mInputStream(NULL, 0)
{
   // start XmlReader
   mReader.start(db);
}

XmlBindingOutputStream::~XmlBindingOutputStream()
{
}

bool XmlBindingOutputStream::write(const char* b, int length)
{
   // set input stream byte array
   mInputStream.setByteArray(b, length);
   
   // use reader
   return mReader.read(&mInputStream) == NULL;
}

void XmlBindingOutputStream::close()
{
   // finish reader
   mReader.finish();
}
