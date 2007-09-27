/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/MutatorInputStream.h"

using namespace db::io;
using namespace db::data;

MutatorInputStream::MutatorInputStream(
   InputStream* is, DataMutationAlgorithm* algorithm, bool cleanup) :
   FilterInputStream(is, cleanup),
   mReadBuffer(2048),
   mMutatedData(4096),
   mMutator(&mReadBuffer, &mMutatedData)
{
   // set mutation algorithm
   mMutator.setAlgorithm(algorithm);
}

MutatorInputStream::~MutatorInputStream()
{
}

int MutatorInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // mutate data
   if(mMutator.mutate(mInputStream))
   {
      // get data from the mutator
      rval = mMutator.get(b, length);
   }
   
   return rval;
}

long MutatorInputStream::skip(long count)
{
   return mMutator.skipMutatedBytes(mInputStream, count);
}
