/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/deflate/Deflater.h"

using namespace db::compress::deflate;
using namespace db::io;

Deflater::Deflater()
{
}

Deflater::~Deflater()
{
}

int Deflater::mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish)
{
   int rval = 0;
   
   // FIXME:
   
   return rval;
}
