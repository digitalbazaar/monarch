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
}

HttpChunkedTransferOutputStream::~HttpChunkedTransferOutputStream()
{
}

bool HttpChunkedTransferOutputStream::flush()
{
   bool rval;
   
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

void HttpChunkedTransferOutputStream::close()
{
   // make sure to flush ;)
   bool write = flush();
   
   // write chunk-size of "0" and CRLF
   char c = '0';
   write = write &&
      mOutputStream->write(&c, 1) &&
      mOutputStream->write(HttpHeader::CRLF, 2);
   
   if(mTrailer != NULL)
   {
      // update the trailer
      mTrailer->update(mDataSent);
      
      // write out trailer
      write = write && mTrailer->write(mOutputStream);
   }
   else
   {
      // write out last CRLF
      write = write && mOutputStream->write(HttpHeader::CRLF, 2);
   }
   
   // reset data sent
   mDataSent = 0;
   
   // flush, but do not close underlying stream
   mOutputStream->flush();
}
