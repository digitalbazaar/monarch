/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Connection.h"

#include "db/rt/DynamicObject.h"
#include "db/util/Math.h"

using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

ConnectionOutputStream::ConnectionOutputStream(Connection* c)
{
   mConnection = c;
   mBytesWritten = 0;
   mUseBuffer = false;
}

ConnectionOutputStream::~ConnectionOutputStream()
{
}

bool ConnectionOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   if(!mUseBuffer)
   {
      // wrap bytes and immediately flush
      mBuffer.setBytes((char*)b, 0, length, false);
      rval = flush();
   }
   else
   {
      int written = 0;
      while(rval && written < length)
      {
         // put bytes into buffer
         written += mBuffer.put(b + written, length - written, false);
         
         // flush buffer if full
         if(mBuffer.isFull())
         {
            rval = flush();
         }
      }
   }
   
   return rval;
}

bool ConnectionOutputStream::flush()
{
   bool rval = true;
   
   int numBytes;
   BandwidthThrottler* bt = mConnection->getBandwidthThrottler(false);
   
   // flush previously unflushed data (due to non-blocking send)
   while(rval && mUnflushed.length() > 0)
   {
      numBytes = mUnflushed.length();
      if(bt != NULL)
      {
         bt->requestBytes(mBuffer.length(), numBytes);
      }
      
      // send data through the socket output stream
      if((rval = mConnection->getSocket()->getOutputStream()->write(
         mUnflushed.data(), numBytes)))
      {
         // clear written bytes from buffer
         mUnflushed.clear(numBytes);
         
         // update bytes written (reset as necessary)
         if(mBytesWritten > Math::HALF_MAX_LONG_VALUE)
         {
            mBytesWritten = 0;
         }
         
         mBytesWritten += numBytes;
      }
      else
      {
         // see if send would block
         ExceptionRef e = Exception::getLast();
         if(e->getDetails()->hasMember("wouldBlock"))
         {
            // clear number of bytes sent
            mUnflushed.clear(e->getDetails()["sent"]->getInt32());
         }
      }
   }
   
   // flush buffered output
   while(rval && mBuffer.length() > 0)
   {
      // throttle the write as appropriate
      numBytes = mBuffer.length();
      if(bt != NULL)
      {
         bt->requestBytes(mBuffer.length(), numBytes);
      }
      
      // send data through the socket output stream
      if((rval = mConnection->getSocket()->getOutputStream()->write(
         mBuffer.data(), numBytes)))
      {
         // clear written bytes from buffer
         mBuffer.clear(numBytes);
         
         // update bytes written (reset as necessary)
         if(mBytesWritten > Math::HALF_MAX_LONG_VALUE)
         {
            mBytesWritten = 0;
         }
         
         mBytesWritten += numBytes;
      }
      else
      {
         // see if send would block
         ExceptionRef e = Exception::getLast();
         if(e->getDetails()->hasMember("wouldBlock"))
         {
            // clear number of bytes sent
            mBuffer.clear(e->getDetails()["sent"]->getInt32());
         }
      }
   }
   
   // put any unflushed bytes into unflushed buffer
   mUnflushed.put(&mBuffer, mBuffer.length(), true);
   
   // clear buffer
   mBuffer.clear();
   
   return rval;
}

void ConnectionOutputStream::close()
{
   // make sure to flush ;)
   flush();
   
   // close socket output stream
   mConnection->getSocket()->getOutputStream()->close();
}

inline uint64_t ConnectionOutputStream::getBytesWritten()
{
   return mBytesWritten;
}

void ConnectionOutputStream::resizeBuffer(int size)
{
   // flush existing buffer
   if(mUseBuffer)
   {
      flush();
   }
   
   if(size > 0)
   {
      if(mUseBuffer)
      {
         // resize existing buffer
         mBuffer.resize(size);
      }
      else
      {
         // allocate new bytes for buffer
         mBuffer.setBytes(NULL, 0, 0, false);
         mBuffer.resize(size);
         mUseBuffer = true;
      }
   }
   else
   {
      if(mUseBuffer)
      {
         // clean up old buffer space and stop using it
         mBuffer.resize(0);
         mUseBuffer = false;
      }
   }
}
