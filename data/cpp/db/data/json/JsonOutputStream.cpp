/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonOutputStream.h"

using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::util;

JsonOutputStream::JsonOutputStream(DynamicObject dyno) :
   mInputStream(NULL, 0)
{
   // start JsonReader
   mReader.start(dyno);
}

JsonOutputStream::~JsonOutputStream()
{
}

bool JsonOutputStream::write(const char* b, int length)
{
   // set input stream byte array
   mInputStream.setByteArray(b, length);
   
   // use reader
   return mReader.read(&mInputStream) == NULL;
}

void JsonOutputStream::close()
{
   // finish reader
   mReader.finish();
}
