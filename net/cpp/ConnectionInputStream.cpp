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

ConnectionInputStream::ConnectionInputStream(Connection* c)
{
   mConnection = c;
   mBytesRead = 0;
}

ConnectionInputStream::~ConnectionInputStream()
{
}

bool ConnectionInputStream::read(char& b) throw(IOException)
{
   bool rval = false;
   
   if(read(&b, 1) != -1)
   {
      rval = true;
   }
   
   return rval;
}

int ConnectionInputStream::read(char* b, unsigned int length) throw(IOException)
{
   // get the current thread
   Thread* thread = Thread::currentThread();
   
   // check for an interruption
   if(thread != NULL && thread->isInterrupted())
   {
      throw InterruptedException("Connection read interrupted!");
   }
   
   // throttle the read as appropriate
   BandwidthThrottler* bt = mConnection->getBandwidthThrottler(true);
   if(bt != NULL)
   {
      length = bt->requestBytes(length);
   }
   
   // read from the socket input stream
   int numBytes = mConnection->getSocket()->getInputStream()->read(b, length);
   if(numBytes > 0)
   {
      // update bytes read (reset as necessary)
      if(mBytesRead > Math::HALF_MAX_LONG_VALUE)
      {
         mBytesRead = 0;
      }
      
      mBytesRead += numBytes;
   }
   
   return numBytes;
}

bool ConnectionInputStream::readLine(string& line) throw(IOException)
{
   bool rval = false;
   
   // read one character at a time
   char c;
   while(read(c) && c != '\n')
   {
      // see if the character is a carriage return
      if(c == '\r')
      {
         // see if the next character is an eol -- and we've found a CRLF
         if(peek(&c, 1) != -1 && c == '\n')
         {
            // read the character in and discard it
            read(c);
         }
         
         // set character to an eol since a carriage return is treated the same
         c = '\n';
      }
      else
      {
         // append the character
         line.append(c, 1);
         
         // a character was appended, so not end of stream
         rval = true;
      }
   }
   
   return rval;
}

bool ConnectionInputStream::readCrlf(string& line) throw(IOException)
{
   bool rval = false;
   
   // read one character at a time until a CRLF is found
   bool found = false;
   char c, p;
   while(!found && read(c))
   {
      // see if the character is a carriage return
      if(c == '\r')
      {
         // see if the next character is an eol -- and we've found a CRLF
         if(peek(&p, 1) != -1 && p == '\n')
         {
            // read the character in and discard it
            read(p);
            
            // CRLF found
            found = true;
         }
      }
      
      if(!found)
      {
         // append the character
         line.append(c, 1);
         
         // a character was appended, so not end of stream
         rval = true;
      }
   }
   
   return rval;
}

int ConnectionInputStream::peek(char* b, unsigned int length) throw(IOException)
{
   // peek using socket input stream
   return mConnection->getSocket()->getInputStream()->peek(b, length);
}

void ConnectionInputStream::close() throw(IOException)
{
   // close socket input stream
   mConnection->getSocket()->getInputStream()->close();
}

const unsigned long long& ConnectionInputStream::getBytesRead()
{
   return mBytesRead;
}
