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

inline bool ConnectionOutputStream::write(const char* b, unsigned int length)
{
   bool rval = true;
   
   // set the data offset
   unsigned int offset = 0;
   unsigned numBytes = length;
   while(rval && length > 0 && !Thread::interrupted(false))
   {
      // throttle the write as appropriate
      BandwidthThrottler* bt = mConnection->getBandwidthThrottler(false);
      if(bt != NULL)
      {
         bt->requestBytes(length, numBytes);
      }
      
      if(!Thread::interrupted(false))
      {
         // send data through the socket output stream
         if(rval = mConnection->getSocket()->getOutputStream()->write(
            b + offset, numBytes))
         {
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
   }
   
   return rval && !Thread::interrupted(false);
}

void ConnectionOutputStream::close()
{
   // close socket output stream
   mConnection->getSocket()->getOutputStream()->close();
}

unsigned long long ConnectionOutputStream::getBytesWritten()
{
   return mBytesWritten;
}
