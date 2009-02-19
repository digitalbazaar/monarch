/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/MutatorOutputStream.h"

#include "db/rt/Exception.h"

using namespace db::io;
using namespace db::rt;

MutatorOutputStream::MutatorOutputStream(
   OutputStream* os, bool cleanupStream,
   MutationAlgorithm* algorithm, bool cleanupAlgorithm,
   ByteBuffer* src, ByteBuffer* dst) :
   FilterOutputStream(os, cleanupStream)
{
   // store mutation algorithm
   mAlgorithm = algorithm;
   mCleanupAlgorithm = cleanupAlgorithm;
   mResult = MutationAlgorithm::NeedsData;
   mFinished = false;
   
   // set source buffer
   if(src == NULL)
   {
      mSource = new ByteBuffer(2048);
      mCleanupSource = true;
   }
   else
   {
      mSource = src;
      mCleanupSource = false;
   }
   
   // set destination buffer
   if(dst == NULL)
   {
      mDestination = new ByteBuffer(4096);
      mCleanupDestination = true;
   }
   else
   {
      mDestination = src;
      mCleanupDestination = false;
   }
}

MutatorOutputStream::~MutatorOutputStream()
{
   if(mCleanupSource)
   {
      delete mSource;
   }
   
   if(mCleanupDestination)
   {
      delete mDestination;
   }
   
   if(mCleanupAlgorithm && mAlgorithm != NULL)
   {
      delete mAlgorithm;
   }
}

bool MutatorOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   // determine buffer to obtain source data from
   ByteBuffer* src;
   if(!mSource->isEmpty())
   {
      // append passed data to the source buffer
      mSource->put(b, length, true);
      src = mSource;
   }
   else
   {
      // wrap the passed data to mutate it without caching it
      mInputWrapper.setBytes((char*)b, 0, length, false);
      src = &mInputWrapper;
   }
   
   // signal algorithm to complete if length is zero
   bool finish = (length == 0);
   
   // keep mutating while algorithm does not need data and is not complete
   bool write = true;
   while(rval && write && mResult < MutationAlgorithm::CompleteAppend)
   {
      // try to mutate data
      mResult = mAlgorithm->mutateData(src, mDestination, finish);
      switch(mResult)
      {
         case MutationAlgorithm::NeedsData:
            if(finish)
            {
               // no more data available for algorithm
               mResult = MutationAlgorithm::Error;
               ExceptionRef e = new Exception(
                  "Insufficient data for mutation algorithm.",
                  "db.io.MutationException");
               Exception::setLast(e, false);
               rval = false;
            }
            else
            {
               // more data needed in order to write
               write = false;
            }
            break;
         case MutationAlgorithm::Error:
            // exception
            write = false;
            break;
         default:
            if(!mDestination->isEmpty())
            {
               // write destination data out
               rval = (mDestination->get(mOutputStream) > 0);
            }
            break;
      }
   }
   
   if(mResult == MutationAlgorithm::CompleteAppend)
   {
      if(!src->isEmpty())
      {
         // write source data to destination
         rval = (src->get(mOutputStream) > 0);
      }
   }
   else if(mResult == MutationAlgorithm::CompleteTruncate)
   {
      // clear any source bytes
      mSource->clear();
      mInputWrapper.clear();
   }
   else
   {
      // copy excess bytes into source buffer
      if(src == &mInputWrapper && !mInputWrapper.isEmpty())
      {
         mSource->put(&mInputWrapper, mInputWrapper.length(), true);
      }
   }
   
   return rval;
}

bool MutatorOutputStream::finish()
{
   bool rval = true;
   
   if(!mFinished)
   {
      // ensure mutation is finished
      rval = write(NULL, 0);
      
      // now finished
      mFinished = true;
   }
   
   return rval;
}

void MutatorOutputStream::close()
{
   // ensure finished
   finish();
   
   // close underlying stream
   mOutputStream->close();
}

void MutatorOutputStream::setAlgorithm(MutationAlgorithm* ma, bool cleanup)
{
   if(mCleanupAlgorithm && mAlgorithm != NULL)
   {
      delete mAlgorithm;
   }
   
   mAlgorithm = ma;
   mCleanupAlgorithm = cleanup;
   mResult = MutationAlgorithm::NeedsData;
   mFinished = false;
   mSource->clear();
   mInputWrapper.clear();
   mDestination->clear();
}

MutationAlgorithm* MutatorOutputStream::getAlgorithm()
{
   return mAlgorithm;
}
