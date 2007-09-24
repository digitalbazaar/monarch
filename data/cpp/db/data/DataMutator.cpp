/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/DataMutator.h"

using namespace db::io;
using namespace db::data;

DataMutator::DataMutator(ByteBuffer* src, ByteBuffer* dest)
{
   // store buffers
   mSource = src;
   mDestination = dest;
   
   // no algorithm yet, use default
   mAlgorithm = this;
   mAlgorithmFinished = false;
}

DataMutator::~DataMutator()
{
}

bool DataMutator::mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish)   
{
   bool rval = false;
   
   if(!mSource->isEmpty())
   {
      // get all data from source and put it in destination
      mSource->get(mDestination, mSource->length(), true);
      rval = true;
   }
   
   return rval;
}

void DataMutator::setAlgorithm(DataMutationAlgorithm* algorithm)
{
   mAlgorithm = algorithm;
   mAlgorithmFinished = false;
}

int DataMutator::mutate(InputStream* is)
{
   int rval = 1;
   
   // mutate while no data is available and not finished
   bool read = mSource->isEmpty();
   while(rval != -1 && !hasData() && !mAlgorithmFinished)
   {
      // read as necessary
      if(read)
      {
         rval = mSource->put(is);
         mAlgorithmFinished = (rval == 0);
      }
      
      if(rval != -1)
      {
         // try to mutate data
         read = !mAlgorithm->mutateData(
            mSource, mDestination, mAlgorithmFinished);
      }
   }
   
   if(hasData() && rval != -1)
   {
      rval = 1;
   }
   else if(rval > 0)
   {
      rval = 0;
   }
   
   return rval;
}

long DataMutator::skipMutatedBytes(InputStream* is, long count)
{
   long rval = 0;
   
   // mutate and skip data
   long remaining = count;
   while(remaining > 0 && mutate(is))
   {
      // clear bytes
      remaining -= mDestination->clear((int)remaining);
   }
   
   if(remaining < count)
   {
      // some bytes were skipped
      rval = count - remaining;
   }
   
   return rval;
}

int DataMutator::get(char* b, int length)
{
   return mDestination->get(b, length);
}

bool DataMutator::hasData()
{
   return !mDestination->isEmpty();
}
