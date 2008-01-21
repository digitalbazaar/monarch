/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/deflate/Inflater.h"

using namespace db::compress::deflate;
using namespace db::io;

Inflater::Inflater()
{
}

Inflater::~Inflater()
{
}

int Inflater::mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish)
{
   int rval = 0;
   
   // FIXME:
   
   return rval;
}
