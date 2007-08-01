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
   mThread = NULL;
}

ConnectionInputStream::~ConnectionInputStream()
{
}

int ConnectionInputStream::read(char* b, unsigned int length)
{
   int rval = -1;
   
   if(mThread == NULL)
   {
      // set current thread
      mThread = Thread::currentThread();
   }
   
   // throttle the read as appropriate
   BandwidthThrottler* bt = mConnection->getBandwidthThrottler(true);
   if(bt != NULL)
   {
      if(!mThread->isInterrupted())
      {
         bt->requestBytes(length, length);
      }
   }
   
   if(!mThread->isInterrupted())
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
   
   line.erase();
   
   // read one character at a time
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
         line.append(1, c);
         
         // a character was appended, so not end of stream
         rval = true;
      }
   }
   
   return rval;
}

bool ConnectionInputStream::readCrlf(string& line)
{
   bool rval = false;
   
   line.erase();
   
   // peek ahead
   char b[1024];
   unsigned int numBytes;
   bool block = false;
   while(!rval && (numBytes = peek(b, 1023, block)) != -1)
   {
      if(numBytes <= 1)
      {
         // not enough peek bytes available, so activate blocking
         block = true;
      }
      else
      {
         // peek bytes available, so deactivate blocking
         block = false;
         
         // ensure peek buffer ends in NULL byte
         memset(b + numBytes, 0, 1);
         
         // look for a CR
         char* i = strchr(b, '\r');
         if(i == NULL)
         {
            // CR not found, append all peeked bytes to string
            line.append(b, numBytes);
            
            // read and discard
            read(b, numBytes);
         }
         else
         {
            // null CR for copying
            memset(i, 0, 1);
            
            // append peeked bytes up until found CR to string
            line.append(b);
            
            // read and discard up until the CR
            read(b, i - b);
            
            // if there's room to check for an LF, do it
            if((i - b) < numBytes)
            {
               // see if the next character is a LF
               if(i[1] == '\n')
               {
                  // CRLF found before end of stream
                  rval = true;
                  
                  // read and discard CRLF (2 characters)
                  read(b, 2);
               }
               else
               {
                  // append CR to line, discard character
                  line.append(1, '\r');
                  read(b, 1);
               }
            }
         }
      }
   }
   
   return rval;
   
//   // read one character at a time until a CRLF is found
//   bool found = false;
//   char c, p;
//   while(!found && read(&c, 1))
//   {
//      // see if the character is a carriage return
//      if(c == '\r')
//      {
//         // see if the next character is an eol -- and we've found a CRLF
//         if(peek(&p, 1) != -1 && p == '\n')
//         {
//            // read the character in and discard it
//            read(&p, 1);
//            
//            // CRLF found
//            found = true;
//         }
//      }
//      
//      if(!found)
//      {
//         // append the character
//         line.append(1, c);
//         
//         // a character was appended, so not end of stream
//         rval = true;
//      }
//   }
   
   return rval;
}

int ConnectionInputStream::peek(char* b, unsigned int length, bool block)
{
   // peek using socket input stream
   return mConnection->getSocket()->getInputStream()->peek(b, length, block);
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
