/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteArrayOutputStream.h"

using namespace db::io;
using namespace db::rt;

ByteArrayOutputStream::ByteArrayOutputStream(ByteBuffer* b)
{
   mBuffer = b;
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
}

bool ByteArrayOutputStream::write(const char* b, unsigned int length)
{
   // put bytes in byte buffer
   mBuffer->put(b, length, true);
   return true;
}

ByteBuffer* ByteArrayOutputStream::getByteArray()
{
   return mBuffer;
}
