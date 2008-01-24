/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/MutatorInputStream.h"

using namespace db::io;
using namespace db::rt;

MutatorInputStream::MutatorInputStream(
   InputStream* is, MutationAlgorithm* algorithm, bool cleanup) :
   FilterInputStream(is, cleanup),
   mSource(2048),
   mDestination(4096)
{
   // store mutation algorithm
   mAlgorithm = algorithm;
   mResult = MutationAlgorithm::NeedsData;
   mSourceEmpty = false;
}

MutatorInputStream::~MutatorInputStream()
{
}

int MutatorInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // mutate while no data is available and algorithm not complete
   while(rval == 0 && mResult < MutationAlgorithm::CompleteAppend)
   {
      // try to mutate data
      mResult = mAlgorithm->mutateData(&mSource, &mDestination, mSourceEmpty);
      switch(mResult)
      {
         case MutationAlgorithm::NeedsData:
            if(mSource.isFull() || mSourceEmpty)
            {
               // no more data available for algorithm
               Exception* e = new Exception(
                  "Insufficient data for mutation algorithm!",
                  "db.io.MutationException");
               Exception::setLast(e);
               rval = -1;
            }
            else
            {
               // read more data from underlying stream
               mSourceEmpty = (mSource.put(mInputStream) == 0);
            }
            break;
         case MutationAlgorithm::Error:
            rval = -1;
            break;
         default:
            // set rval to available data
            rval = mDestination.length();
            break;
      }
   }
   
   // if the algorithm has completed, handle any excess source data
   if(mResult >= MutationAlgorithm::CompleteAppend)
   {
      if(mResult == MutationAlgorithm::CompleteAppend)
      {
         // empty source into destination
         mSource.get(&mDestination, mSource.length(), true);
         
         // get bytes from destination
         rval = mDestination.get(b, length);
         
         if(rval == 0 && !mSourceEmpty)
         {
            // read bytes directly into passed buffer
            rval = mInputStream->read(b, length);
         }
      }
      else if(mDestination.isEmpty() && !mSourceEmpty)
      {
         // clear remaining bytes in underlying input stream
         mSource.clear();
         while(mSource.put(mInputStream) > 0)
         {
            mSource.clear();
         }
      }
   }
   else if(mResult != MutationAlgorithm::Error)
   {
      // get data from destination buffer
      rval = mDestination.get(b, length);
   }
   
   return rval;
}
