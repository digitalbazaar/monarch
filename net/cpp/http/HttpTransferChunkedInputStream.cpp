/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpTransferChunkedInputStream.h"
#include "HttpHeader.h"
#include "Convert.h"
#include "Math.h"

#include <string>

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::util;

HttpTransferChunkedInputStream::HttpTransferChunkedInputStream(
   ConnectionInputStream* is) : PeekInputStream(is, false)
{
   // no current chunk yet
   mChunkBytesLeft = 0;
   
   // not last chunk yet
   mLastChunk = false;
}

HttpTransferChunkedInputStream::~HttpTransferChunkedInputStream()
{
}

int HttpTransferChunkedInputStream::read(char* b, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
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
         throw IOException("Could not read HTTP chunk size!");
      }
   }
   
   // read the chunk into the passed data buffer
   int numBytes = 0;
   while(mChunkBytesLeft > 0 && numBytes != -1)
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
         throw IOException("Could not read HTTP chunk!");
      }
   }
   
   // FIXME: increment content bytes read
   // hwc.setContentBytesRead(hwc.getContentBytesRead() + numBytes);
   
   // if this is the last chunk, then read in the
   // chunk trailer and last CRLF
   if(mLastChunk)
   {
      // build trailer headers
      string trailerHeaders;
      string line;
      while(is->readCrlf(line) && line != "")
      {
         trailerHeaders.append(line + HttpHeader::CRLF);
         line.erase();
      }
      
      // FIXME:
      // parse trailer headers
      //header->parseHeaders(trailerHeaders->toString());
      
      // FIXME:
      // remove "chunked" from transfer-encoding header
      /*
      String transferEncoding = header.getTransferEncoding();
      transferEncoding.replaceAll("chunked", "");
      if(transferEncoding.equals(""))
      {
         header.setTransferEncoding(null);
      }
      else
      {
         header.setTransferEncoding(transferEncoding);
      }
      */
   }
   else if(mChunkBytesLeft == 0)
   {
      // read chunk-data CRLF
      string throwout;
      is->readCrlf(throwout);
   }
   else
   {
      // if the length is greater than zero then the
      // whole chunk wasn't read
      throw IOException("Could not read entire HTTP chunk!");
   }
   
   return rval;
}

void HttpTransferChunkedInputStream::close() throw(IOException)
{
   // does nothing, do not close underlying stream
}
