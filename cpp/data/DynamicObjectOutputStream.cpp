/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/DynamicObjectOutputStream.h"

using namespace monarch::data;
using namespace monarch::io;
using namespace monarch::rt;

DynamicObjectOutputStream::DynamicObjectOutputStream(
   DynamicObject& dyno, DynamicObjectReader* reader, bool cleanup) :
   mReader(reader),
   mCleanupReader(cleanup),
   mInputStream(NULL, 0)
{
   // start DynamicObjectReader
   mReader->start(dyno);
}

DynamicObjectOutputStream::~DynamicObjectOutputStream()
{
   if(mCleanupReader)
   {
      delete mReader;
   }
}

bool DynamicObjectOutputStream::write(const char* b, int length)
{
   // set input stream byte array
   mInputStream.setByteArray(b, length);

   // use reader
   return mReader->read(&mInputStream);
}

void DynamicObjectOutputStream::close()
{
   // finish reader
   mReader->finish();
}
