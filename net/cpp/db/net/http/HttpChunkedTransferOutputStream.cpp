/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/http/HttpChunkedTransferOutputStream.h"

#include "db/util/Convert.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpChunkedTransferOutputStream::HttpChunkedTransferOutputStream(
   ConnectionOutputStream* os, HttpTrailer* trailer, int chunkSize) :
BufferedOutputStream(NULL, os, false),
mChunkSize(chunkSize),
// output buffer must be large enough for:
// chunk-size + ending CRLF (2 bytes)
mOutputBuffer(chunkSize + 2)
{
   // set buffer
   setBuffer(&mOutputBuffer);
   
   // store http trailer
   mTrailer = trailer;
   
   // no data sent yet
   mDataSent = 0;
   
   // not finished yet
   mFinished = false;
}

HttpChunkedTransferOutputStream::~HttpChunkedTransferOutputStream()
{
}

bool HttpChunkedTransferOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   int max;
   int numBytes;
   while(rval && length > 0)
   {
      // put bytes into buffer, but only up to chunk size
      max = mChunkSize - mBuffer->length();
      numBytes = mBuffer->put(b, (max > length) ? length : max, false);
      
      // update data left to be written
      b += numBytes;
      length -= numBytes;
      
      // flush buffer if chunk size reached
      if(mBuffer->length() == mChunkSize)
      {
         rval = flush();
      }
   }
   
   return rval;
}

bool HttpChunkedTransferOutputStream::flush()
{
   bool rval = true;
   
   // write buffered data, if any
   if(mBuffer->length() > 0)
   {
      // update data sent
      mDataSent += mBuffer->length();
      
      // get the chunk-size and add CRLF
      string chunkSize = Convert::intToHex(mBuffer->length());
      chunkSize.append(HttpHeader::CRLF, 2);
      
      // append CRLF to end of chunk data
      mBuffer->put(HttpHeader::CRLF, 2, false);
      
      // write chunk-size + CRLF
      // write chunk data + CRLF
      // flush
      rval =
         mOutputStream->write(chunkSize.c_str(), chunkSize.length()) &&
         mOutputStream->write(mBuffer->data(), mBuffer->length()) &&
         mOutputStream->flush();
      
      // clear buffer
      mBuffer->clear();
   }
   else
   {
      // flush underlying stream
      rval = mOutputStream->flush();
   }
   
   return rval;
}

bool HttpChunkedTransferOutputStream::finish()
{
   bool rval = true;
   
   if(!mFinished)
   {
      // make sure to flush ;)
      rval = flush();
      
      // write chunk-size of "0" and CRLF
      char out[3] = { '0', '\r', '\n' };
      rval = rval && mOutputStream->write(out, 3);
      
      if(mTrailer != NULL)
      {
         // update the trailer
         mTrailer->update(mDataSent);
         
         // write out trailer
         rval = rval && mTrailer->write(mOutputStream);
      }
      else
      {
         // write out last CRLF
         rval = rval && mOutputStream->write(HttpHeader::CRLF, 2);
      }
      
      // reset data sent
      mDataSent = 0;
      
      // flush, but do not close underlying stream
      mOutputStream->flush();
      
      // now finished
      mFinished = true;
   }
   
   return rval;
}

void HttpChunkedTransferOutputStream::close()
{
   // ensure finished, then close
   finish();
   BufferedOutputStream::close();
}
