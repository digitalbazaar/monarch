/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/DataMutator.h"

using namespace db::io;

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

int DataMutator::mutateData(ByteBuffer* src, ByteBuffer* dest, bool finish)   
{
   int rval = 0;
   
   if(mAlgorithm == this)
   {
      if(!src->isEmpty())
      {
         // get all data from source and put it in destination
         src->get(dest, src->length(), true);
         rval = 1;
      }
   }
   else
   {
      // use set algorithm
      rval = mAlgorithm->mutateData(src, dest, finish);
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
   int rval = 0;
   
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
         read = mAlgorithm->mutateData(
            mSource, mDestination, mAlgorithmFinished) > 0;
      }
   }
   
   if(hasData() && rval != -1)
   {
      // mutation succeeded, data available
      rval = mDestination->length();
   }
   else if(rval > 0)
   {
      // mutation succeeded, but no more data
      rval = 0;
   }
   
   return rval;
}

int DataMutator::mutate(const char* b, int length)
{
   int rval = 0;
   
   if(!mAlgorithmFinished)
   {
      // set algorithm to finished if length is zero
      mAlgorithmFinished = (length == 0);
      
      // determine if there is any cached source data
      if(!mSource->isEmpty())
      {
         // append the data to the source buffer
         mSource->put(b, length, true);
         
         // try to mutate data
         rval = mAlgorithm->mutateData(
            mSource, mDestination, mAlgorithmFinished);
      }
      else
      {
         // try to mutate passed data without caching it
         mInputWrapper.setBytes((char*)b, 0, length, false);
         rval = mAlgorithm->mutateData(
            &mInputWrapper, mDestination, mAlgorithmFinished);
         
         // copy excess bytes into source buffer
         if(!mInputWrapper.isEmpty())
         {
            mSource->put(&mInputWrapper, mInputWrapper.length(), true);
         }
      }
      
      if(hasData() && rval > 0)
      {
         // mutation succeeded, data available
         rval = mDestination->length();
      }
      else if(rval > 0)
      {
         // mutation succeeded, but no more data
         rval = 0;
      }
   }
   
   return rval;
}

long long DataMutator::skipMutatedBytes(InputStream* is, long long count)
{
   long long rval = 0;
   
   // mutate and skip data
   long long remaining = count;
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

void DataMutator::setSource(ByteBuffer* src)
{
   mSource = src;
}

ByteBuffer* DataMutator::getSource()
{
   return mSource;
}

void DataMutator::setDestination(ByteBuffer* dest)
{
   mDestination = dest;
}

ByteBuffer* DataMutator::getDestination()
{
   return mDestination;
}
