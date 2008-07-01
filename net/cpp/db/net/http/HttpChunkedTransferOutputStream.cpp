/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpChunkedTransferOutputStream.h"
#include "db/util/Convert.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpChunkedTransferOutputStream::HttpChunkedTransferOutputStream(
   ConnectionOutputStream* os, HttpTrailer* trailer) :
BufferedOutputStream(NULL, os, false),
mOutputBuffer(1024)
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

bool HttpChunkedTransferOutputStream::flush()
{
   bool rval = true;
   
   // write buffered data, if any
   if(mBuffer->length() > 0)
   {
      // update data sent
      mDataSent += mBuffer->length();
      
      // get the chunk-size
      string chunkSize = Convert::intToHex(mBuffer->length());
      
      // write chunk-size
      // write CRLF
      // write chunk data
      // write CRLF
      // flush
      rval =
         mOutputStream->write(chunkSize.c_str(), chunkSize.length()) &&
         mOutputStream->write(HttpHeader::CRLF, 2) &&
         mOutputStream->write(mBuffer->data(), mBuffer->length()) &&
         mOutputStream->write(HttpHeader::CRLF, 2) &&
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
      char c = '0';
      rval = rval &&
         mOutputStream->write(&c, 1) &&
         mOutputStream->write(HttpHeader::CRLF, 2);
      
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
