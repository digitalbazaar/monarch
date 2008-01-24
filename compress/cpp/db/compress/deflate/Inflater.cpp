/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/deflate/Inflater.h"

using namespace db::compress::deflate;
using namespace db::io;

// Note: chunk sizes of 128K or 256K are recommended by zlib
#define CHUNK_SIZE 16384

Inflater::Inflater()
{
}

Inflater::~Inflater()
{
}

MutationAlgorithm::Result Inflater::mutateData(
   ByteBuffer* src, ByteBuffer* dst, bool finish)
{
   // FIXME:
}
