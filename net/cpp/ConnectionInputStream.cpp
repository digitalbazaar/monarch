/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Connection.h"
#include "Math.h"

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

int ConnectionInputStream::read(char* b, unsigned int length)
{
   int rval = -1;
   
   Thread* t = Thread::currentThread();
   
   // throttle the read as appropriate
   BandwidthThrottler* bt = mConnection->getBandwidthThrottler(true);
   if(bt != NULL)
   {
      if(!t->isInterrupted())
      {
         bt->requestBytes(length, length);
      }
   }
   
   if(!t->isInterrupted())
   {
      // read from the socket input stream
      rval = mConnection->getSocket()->getInputStream()->read(b, length);
      if(rval > 0)
      {
         // update bytes read (reset as necessary)
         if(mBytesRead > Math::HALF_MAX_LONG_VALUE)
         {
            mBytesRead = 0;
         }
         
         mBytesRead += rval;
      }
   }
   
   return rval;
}

bool ConnectionInputStream::readLine(string& line)
{
   bool rval = false;
   
   // read one character at a time
   line.erase();
   char c;
   while(read(&c, 1) && c != '\n')
   {
      // see if the character is a carriage return
      if(c == '\r')
      {
         // see if the next character is an eol -- and we've found a CRLF
         if(peek(&c, 1) != -1 && c == '\n')
         {
            // read the character in and discard it
            read(&c, 1);
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

bool ConnectionInputStream::readCrlf(string& line)
{
   bool rval = false;
   
   // read one character at a time until a CRLF is found
   line.erase();
   bool found = false;
   char c, p;
   while(!found && read(&c, 1))
   {
      // see if the character is a carriage return
      if(c == '\r')
      {
         // see if the next character is an eol -- and we've found a CRLF
         if(peek(&p, 1) != -1 && p == '\n')
         {
            // read the character in and discard it
            read(&p, 1);
            
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

int ConnectionInputStream::peek(char* b, unsigned int length)
{
   // peek using socket input stream
   return mConnection->getSocket()->getInputStream()->peek(b, length);
}

void ConnectionInputStream::close()
{
   // close socket input stream
   mConnection->getSocket()->getInputStream()->close();
}

unsigned long long ConnectionInputStream::getBytesRead()
{
   return mBytesRead;
}
