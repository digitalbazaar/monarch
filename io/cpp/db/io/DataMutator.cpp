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
   mAlgorithmExitCode = 0;
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
      
      if(finish)
      {
         // algorithm finished
         mAlgorithmExitCode = rval = 2;
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
   mAlgorithmExitCode = 0;
}

int DataMutator::mutate(InputStream* is)
{
   int rval = 0;
   
   // mutate while no data is available and not finished
   bool finish = false;
   bool read = false;
   while(rval != -1 && !hasData() && mAlgorithmExitCode == 0)
   {
      // read as necessary
      if(read)
      {
         rval = mSource->put(is);
         finish = (rval == 0);
      }
      
      if(rval != -1)
      {
         // try to mutate data
         rval = mAlgorithm->mutateData(mSource, mDestination, finish);
         
         if(rval > 1)
         {
            // algorithm completed
            mAlgorithmExitCode = rval;
         }
         else
         {
            // read if mutation algorithm needs more data
            read = (rval == 0);
         }
      }
   }
   
   // handle remaining source data if algorithm has completed
   if(rval != -1 && mAlgorithmExitCode > 0)
   {
      if(mAlgorithmExitCode == 2)
      {
         // copy any source data to destination buffer
         mSource->get(mDestination, mSource->length(), true);
         
         // fill destination buffer directly
         rval = mDestination->put(is);
      }
      else if(mAlgorithmExitCode == 3)
      {
         // truncate remaining data
         mSource->clear();
         while((rval = mSource->put(is)) > 0)
         {
            mSource->clear();
         }
      }
   }
   
   if(rval > 0)
   {
      rval = mDestination->length();
   }
   
   return rval;
}

int DataMutator::mutate(const char* b, int length)
{
   int rval = 0;
   
   if(mAlgorithmExitCode == 0)
   {
      // try to finish algorithm if length is zero
      bool finish = (length == 0);
      
      // determine if there is any cached source data
      if(!mSource->isEmpty())
      {
         // append the data to the source buffer
         mSource->put(b, length, true);
         
         // try to mutate data
         rval = mAlgorithm->mutateData(mSource, mDestination, finish);
      }
      else
      {
         // try to mutate passed data without caching it
         mInputWrapper.setBytes((char*)b, 0, length, false);
         rval = mAlgorithm->mutateData(&mInputWrapper, mDestination, finish);
         
         // copy excess bytes into source buffer
         if(!mInputWrapper.isEmpty())
         {
            mSource->put(&mInputWrapper, mInputWrapper.length(), true);
         }
      }
   }
   
   if(rval != -1)
   {
      if(mAlgorithmExitCode == 2)
      {
         // copy data to destination
         mSource->get(mDestination, mSource->length(), true);
         mDestination->put(b, length, true);
      }
      else if(mAlgorithmExitCode == 3)
      {
         // truncate data
         mSource->clear();
         mDestination->clear();
      }
   }
   
   if(rval > 0)
   {
      rval = mDestination->length();
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
      remaining -= mDestination->clear((int)(remaining & 0x7fffffff));
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
