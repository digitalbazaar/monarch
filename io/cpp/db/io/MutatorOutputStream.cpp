/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/MutatorOutputStream.h"

using namespace db::io;

MutatorOutputStream::MutatorOutputStream(
   OutputStream* os, DataMutationAlgorithm* algorithm, bool cleanup) :
   FilterOutputStream(os, cleanup),
   mUnmutatedData(2048),
   mMutatedData(4096),
   mMutator(&mUnmutatedData, &mMutatedData)
{
   // set mutation algorithm
   mMutator.setAlgorithm(algorithm);
}

MutatorOutputStream::~MutatorOutputStream()
{
}

bool MutatorOutputStream::write(const char* b, int length)
{
   bool rval = false;
   
   // mutate data
   if(mMutator.mutate(b, length) != -1)
   {
      // write mutated data directly to underlying output stream
      mOutputStream->write(mMutatedData.data(), mMutatedData.length());
      mMutatedData.clear();
      rval = true;
   }
   
   return rval;
}

void MutatorOutputStream::close()
{
   // ensure mutation is finished
   write(NULL, 0);
   
   // close underlying stream
   mOutputStream->close();
}
