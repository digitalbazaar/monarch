/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Connection.h"
#include "db/util/Math.h"

#include <cstring>

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

int ConnectionInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // throttle the read as appropriate
   BandwidthThrottler* bt = mConnection->getBandwidthThrottler(true);
   if(bt != NULL)
   {
      bt->requestBytes(length, length);
   }
   
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
   
   return rval;
}

int ConnectionInputStream::readFully(char* b, int length)
{
   int rval = 0;
   
   // keep reading until eos, error, or length reached
   int remaining = length;
   int offset = 0;
   int numBytes = 0;
   while(remaining > 0 && (numBytes = read(b + offset, remaining)) > 0)
   {
      remaining -= numBytes;
      offset += numBytes;
      rval += numBytes;
   }
   
   if(numBytes == -1)
   {
      rval = -1;
   }
   
   return rval;
}

int ConnectionInputStream::readLine(string& line)
{
   int rval = 0;
   
   line.erase();
   
   // read one character at a time
   char c;
   int numBytes;
   while((numBytes = read(&c, 1)) > 0 && c != '\n')
   {
      // see if the character is a carriage return
      if(c == '\r')
      {
         // see if the next character is an eol -- and we've found a CRLF
         if(peek(&c, 1) > 0 && c == '\n')
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
         line.push_back(c);
         
         // a character was appended, so not end of stream
         rval = 1;
      }
   }
   
   if(numBytes == -1)
   {
      rval = -1;
   }
   
   return rval;
}

int ConnectionInputStream::readCrlf(string& line)
{
   int rval = 0;
   
   line.erase();
   
   // peek ahead
   char b[1024];
   int numBytes;
   bool block = false;
   int readSize = 1023;
   while(rval != 1 && (numBytes = peek(b, readSize, block)) != -1 &&
         (numBytes > 0 || !block))
   {
      // read maximum amount
      readSize = 1023;
      
      // maximum line length of 1 MB
      if(line.length() > (1024 << 10))
      {
         rval = -1;
         ExceptionRef e = new Exception(
            "Could not read CRLF, line too long.", "db.net.CRLFLineTooLong");
         Exception::setLast(e, false);
         break;
      }
      
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
         b[numBytes] = 0;
         
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
            i[0] = 0;
            
            // append peeked bytes up until found CR to string
            line.append(b);
            
            // read and discard up until the CR
            read(b, i - b);
            
            // if there's room to check for an LF, do it
            if((i - b) < (numBytes - 1))
            {
               // see if the next character is a LF
               if(i[1] == '\n')
               {
                  // CRLF found before end of stream
                  rval = 1;
                  
                  // read and discard CRLF (2 characters)
                  read(b, 2);
               }
               else
               {
                  // append CR to line, discard character
                  line.push_back('\r');
                  read(b, 1);
               }
            }
            else
            {
               // read 1 more byte to find the LF
               readSize = 1;
            }
         }
      }
   }
   
   if(numBytes == -1)
   {
      rval = -1;
   }
   
   return rval;
}

inline int ConnectionInputStream::peek(char* b, int length, bool block)
{
   // peek using socket input stream
   return mConnection->getSocket()->getInputStream()->peek(b, length, block);
}

inline void ConnectionInputStream::close()
{
   // close socket input stream
   mConnection->getSocket()->getInputStream()->close();
}

inline unsigned long long ConnectionInputStream::getBytesRead()
{
   return mBytesRead;
}
