/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Connection.h"
#include "Math.h"
#include "Thread.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

ConnectionOutputStream::ConnectionOutputStream(Connection* c)
{
   mConnection = c;
   mBytesWritten = 0;
}

ConnectionOutputStream::~ConnectionOutputStream()
{
}

void ConnectionOutputStream::write(const char& b) throw(IOException)
{
   write(&b, 1);
}

inline void ConnectionOutputStream::write(const char* b, unsigned int length)
throw(IOException)
{
   // get the current thread
   Thread* thread = Thread::currentThread();
   
   // set the data offset
   unsigned int offset = 0;
   
   while(length > 0)
   {
      // check for interruption
      if(thread != NULL && thread->isInterrupted())
      {
         throw InterruptedException("Connection write interrupted!");
      }
      
      // throttle the write if appropriate
      int numBytes = length;
      BandwidthThrottler* bt = mConnection->getBandwidthThrottler(false);
      if(bt != NULL)
      {
         numBytes = bt->requestBytes(length);
      }
      
      // send data through the socket output stream
      mConnection->getSocket()->getOutputStream()->write(b + offset, numBytes);
      
      // increment offset and decrement length
      offset += numBytes;
      length -= numBytes;
      
      // update bytes written (reset as necessary)
      if(mBytesWritten > Math::HALF_MAX_LONG_VALUE)
      {
         mBytesWritten = 0;
      }
      
      mBytesWritten += numBytes;
   }
}

void ConnectionOutputStream::close() throw(IOException)
{
   // close socket output stream
   mConnection->getSocket()->getOutputStream()->close();
}

unsigned long long ConnectionOutputStream::getBytesWritten()
{
   return mBytesWritten;
}
