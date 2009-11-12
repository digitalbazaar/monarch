/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/ConnectionInputStream.h"

#include "db/net/Connection.h"
#include "db/util/Math.h"

#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

#define MAX_READ_SIZE   1023

ConnectionInputStream::ConnectionInputStream(Connection* c) :
   mConnection(c),
   mBytesRead(0),
   mPeekBuffer(0),
   mPeeking(false)
{
}

ConnectionInputStream::~ConnectionInputStream()
{
}

inline int ConnectionInputStream::read(char* b, int length)
{
   int rval = 0;

   if(mPeeking || mPeekBuffer.isEmpty())
   {
      // throttle the read as appropriate
      BandwidthThrottler* bt = mConnection->getBandwidthThrottler(true);
      if(bt != NULL)
      {
         bt->requestBytes(length, length);
      }

      // read from the socket input stream
      InputStream* is = mConnection->getSocket()->getInputStream();
      if(is != NULL)
      {
         rval = is->read(b, length);
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not read from connection. Socket closed.",
            "db.net.Socket.Closed");
         Exception::set(e);
         rval = -1;
      }
   }
   else
   {
      // read from peek buffer
      rval = mPeekBuffer.get(b, length);
   }

   if(rval > 0 && !mPeeking)
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

   // reset line and create buffer to parse for CRLFs
   line.erase();
   char b[MAX_READ_SIZE + 1];

   // keep peeking ahead until there's an error or a line is completed either
   // by CRLF or EOF
   int numBytes;
   bool block = false;
   int offset = 0;
   int readSize = MAX_READ_SIZE;
   bool eof = false;
   while(rval == 0 && !eof &&
         (numBytes = peek(b + offset, readSize, block)) != -1)
   {
      if(numBytes == 0)
      {
         if(!block)
         {
            // not enough peek bytes available, but we didn't block,
            // so activate blocking to try and get more bytes
            block = true;
         }
         else
         {
            // we were blocking but still didn't get any peek bytes, so
            // we've hit the end of the stream
            eof = true;
            if(offset == 1)
            {
               // include CR if we had one from a previous pass
               line.push_back('\r');
            }
         }
      }
      else
      {
         // NULL-terminate our buffer so we can use strchr() to find the
         // next CR, and if were preserving a CR from the last pass by
         // using offset=1, then add that offset to the number of bytes
         // available in our buffer
         numBytes += offset;
         b[numBytes] = 0;

         // now that peeked bytes are available, deactivate blocking, and
         // reset the readSize and the offset
         block = false;
         readSize = MAX_READ_SIZE;
         offset = 0;

         // look for a CR (which will either find a novel CR, or a CR from
         // a previous pass where the CR was at the end of the buffer and
         // was moved to the front of it)
         char* i = strchr(b, '\r');
         if(i == NULL)
         {
            // CR not found, append all peeked bytes to the line and
            // then read and discard them
            line.append(b, numBytes);
            read(b, numBytes);
         }
         else
         {
            // determine the length of the current partial line and append
            // all peeked bytes up to but not including the CR to the line
            int partial = i - b;
            line.append(b, partial);

            // see if there are more bytes in the buffer after the CR
            bool hasMore = (partial < (numBytes - 1));
            if(hasMore)
            {
               // check for an LF that immediately follows the CR
               if(i[1] == '\n')
               {
                  // CRLF found before end of stream, so a valid CRLF line has
                  // been found
                  rval = 1;

                  // read and discard peeked bytes and CRLF (+2 chars)
                  read(b, partial + 2);
               }
               else
               {
                  // there is no following LF, so append the CR to the line,
                  // as we haven't found a CRLF line only a partial line that
                  // happens to have a CR in it
                  line.push_back('\r');

                  // read and discard peeked bytes and CR (+1 char)
                  read(b, partial + 1);
               }
            }
            else
            {
               // there is not enough peeked data to see if there is a
               // LF following the CR we found, so keep the existing CR
               // and read 1 more byte to check for it in the next pass
               offset = readSize = 1;

               // read and discard peeked bytes and CR (+1 char), then set CR
               // at the beginning of the buffer so it can be found in the next
               // pass (it won't be overwritten because offset=1)
               read(b, partial + 1);
               b[0] = '\r';
            }
         }
      }

      // maximum line length of 1 MB
      if(rval == 0 && line.length() > (1024 << 10))
      {
         rval = -1;
         ExceptionRef e = new Exception(
            "Could not read CRLF, line too long.", "db.net.CRLFLineTooLong");
         Exception::set(e);
         break;
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
   int rval = 0;

   // see if more data needs to be read
   if(block && length > mPeekBuffer.length())
   {
      // allocate enough space in the peek buffer
      mPeekBuffer.allocateSpace(length, true);

      // read into the peek buffer from this stream
      mPeeking = true;
      rval = mPeekBuffer.put(this);
      mPeeking = false;
   }

   // check for peeked bytes
   if(!mPeekBuffer.isEmpty() && rval != -1)
   {
      // read from the peek buffer
      rval = mPeekBuffer.get(b, length);

      // reset peek buffer so that data will be read again
      mPeekBuffer.reset(rval);
   }

   return rval;
}

inline void ConnectionInputStream::close()
{
   // close socket input stream
   InputStream* is = mConnection->getSocket()->getInputStream();
   if(is != NULL)
   {
      is->close();
   }
}

inline uint64_t ConnectionInputStream::getBytesRead()
{
   return mBytesRead;
}
