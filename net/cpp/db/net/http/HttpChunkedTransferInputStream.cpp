/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpChunkedTransferInputStream.h"
#include "db/util/Convert.h"
#include "db/util/Math.h"

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

int HttpChunkedTransferInputStream::read(char* b, unsigned int length)
{
   int rval = -1;
   
   Exception* exception = NULL;
   
   // get underlying connection input stream
   ConnectionInputStream* is = (ConnectionInputStream*)mInputStream;
   
   if(mChunkBytesLeft == 0 && !mLastChunk)
   {
      // read chunk-size
      string chunkSize;
      if(is->readCrlf(chunkSize))
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
         exception = new IOException("Could not read HTTP chunk size!");
      }
   }
   
   // read some chunk bytes into the passed data buffer
   int numBytes = 0;
   if(mChunkBytesLeft > 0 && exception == NULL && numBytes != -1 &&
      !mThread->isInterrupted())
   {
      unsigned int readSize = (length < mChunkBytesLeft) ?
         length : mChunkBytesLeft;
      numBytes = is->read(b, readSize);
      if(numBytes != -1)
      {
         // decrement bytes left
         mChunkBytesLeft -= numBytes;
         
         // increment bytes read
         rval = (rval == -1) ? numBytes : rval + numBytes;
      }
      else
      {
         exception = new IOException("Could not read HTTP chunk!");
      }
   }
   
   // if this is the last chunk, then read in the
   // chunk trailer and last CRLF
   if(exception == NULL && mLastChunk)
   {
      // build trailer headers
      string trailerHeaders;
      string line;
      while(is->readCrlf(line) && line.length() > 0)
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
   else if(exception == NULL && mChunkBytesLeft == 0)
   {
      // read chunk-data CRLF
      string throwout;
      is->readCrlf(throwout);
   }
   else if(exception == NULL && numBytes == -1)
   {
      // if the length is greater than zero then the whole chunk wasn't read
      exception = new IOException("Could not read entire HTTP chunk!");
   }
   
   if(exception != NULL)
   {
      Exception::setLast(exception);
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
