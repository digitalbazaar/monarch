/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpChunkedTransferInputStream.h"
#include "db/util/Convert.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

HttpChunkedTransferInputStream::HttpChunkedTransferInputStream(
   ConnectionInputStream* is, HttpTrailer* trailer) :
PeekInputStream(is, false)
{
   // store trailer
   mTrailer = trailer;
   
   // no current chunk yet
   mChunkBytesLeft = 0;
   
   // not last chunk yet
   mLastChunk = false;
   
   // store the thread reading from this stream
   mThread = Thread::currentThread();
}

HttpChunkedTransferInputStream::~HttpChunkedTransferInputStream()
{
}

int HttpChunkedTransferInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // get underlying connection input stream
   ConnectionInputStream* is = (ConnectionInputStream*)mInputStream;
   
   if(mChunkBytesLeft == 0 && !mLastChunk)
   {
      // read chunk-size
      string chunkSize;
      if(is->readCrlf(chunkSize) == 1)
      {
         // ignore chunk-extension
         int sizeLength = chunkSize.length();
         char* size = strchr(chunkSize.c_str(), ' ');
         if(size != NULL)
         {
            sizeLength = size - chunkSize.c_str();
         }
         
         // get size of chunk data
         mChunkBytesLeft = Convert::hexToInt(chunkSize.c_str(), sizeLength);
         
         // this is the last chunk if length is 0
         mLastChunk = (mChunkBytesLeft == 0);
      }
      else
      {
         // the chunk size could not be read!
         ExceptionRef e = new IOException("Could not read HTTP chunk size!");
         Exception::setLast(e);
         rval = -1;
      }
   }
   
   // read some chunk bytes into the passed data buffer
   int numBytes = 1;
   if(mChunkBytesLeft > 0 && rval != -1 && numBytes > 0 &&
      !mThread->isInterrupted())
   {
      int readSize = (length < mChunkBytesLeft) ? length : mChunkBytesLeft;
      numBytes = is->read(b, readSize);
      if(numBytes > 0)
      {
         // decrement bytes left
         mChunkBytesLeft -= numBytes;
         
         // increment bytes read
         rval += numBytes;
      }
      else
      {
         ExceptionRef e = new IOException("Could not read HTTP chunk!");
         Exception::setLast(e);
         rval = -1;
      }
   }
   
   if(rval != -1)
   {
      // if this is the last chunk, then read in the
      // chunk trailer and last CRLF
      if(mLastChunk)
      {
         // build trailer headers
         string trailerHeaders;
         string line;
         while(is->readCrlf(line) > 0 && line.length() > 0)
         {
            trailerHeaders.append(line);
            trailerHeaders.append(HttpHeader::CRLF);
         }
         
         // parse trailer headers, if appropriate
         if(mTrailer != NULL)
         {
            mTrailer->parse(trailerHeaders);
         }
      }
      else if(mChunkBytesLeft == 0)
      {
         // read chunk-data CRLF
         string throwout;
         is->readCrlf(throwout);
      }
      else if(numBytes == 0)
      {
         // if the chunk bytes left is greater than zero and end of stream
         // was read, then whole chunk wasn't read
         ExceptionRef e = new IOException("Could not read entire HTTP chunk!");
         Exception::setLast(e);
         rval = -1;
      }
   }
   
   return rval;
}

void HttpChunkedTransferInputStream::close()
{
   // reset
   mChunkBytesLeft = 0;
   mLastChunk = false;
   
   // do not close underlying stream
}
