/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpChunkedTransferInputStream.h"
#include "Convert.h"
#include "Math.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

HttpChunkedTransferInputStream::HttpChunkedTransferInputStream(
   ConnectionInputStream* is, HttpHeader* header) :
PeekInputStream(is, false)
{
   // store header
   mHeader = header;
   
   // no current chunk yet
   mChunkBytesLeft = 0;
   
   // not last chunk yet
   mLastChunk = false;
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
         string::size_type index = chunkSize.find_first_of(' ');
         if(index != string::npos)
         {
            chunkSize = chunkSize.substr(0, index);
         }
         
         // get size of chunk data
         mChunkBytesLeft = Convert::hexToInt(
            chunkSize.c_str(), chunkSize.length());
         
         // this is the last chunk if length is 0
         mLastChunk = (length == 0);
      }
      else
      {
         // the chunk size could not be read!
         exception = new IOException("Could not read HTTP chunk size!");
      }
   }
   
   // read the chunk into the passed data buffer
   Thread* t = Thread::currentThread();
   int numBytes = 0;
   while(exception == NULL && !t->isInterrupted() &&
         mChunkBytesLeft > 0 && numBytes != -1)
   {
      numBytes = is->read(b, length);
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
      while(is->readCrlf(line) && line != "")
      {
         trailerHeaders.append(line);
         trailerHeaders.append(HttpHeader::CRLF);
      }
      
      // parse trailer headers
      mHeader->parse(trailerHeaders);
   }
   else if(exception == NULL && mChunkBytesLeft == 0)
   {
      // read chunk-data CRLF
      string throwout;
      is->readCrlf(throwout);
   }
   else if(exception == NULL)
   {
      // if the length is greater than zero then the whole chunk wasn't read
      exception = new IOException("Could not read entire HTTP chunk!");
   }
   
   if(exception != NULL)
   {
      Thread::setException(exception);
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
