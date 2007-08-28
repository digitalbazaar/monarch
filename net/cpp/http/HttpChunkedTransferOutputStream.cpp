/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpChunkedTransferOutputStream.h"
#include "Convert.h"
#include "Math.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpChunkedTransferOutputStream::HttpChunkedTransferOutputStream(
   ConnectionOutputStream* os, HttpHeader* trailers) :
FilterOutputStream(os, false)
{
   // store http trailers header
   mTrailers = trailers;
   
   // no data sent yet
   mDataSent = 0;
}

HttpChunkedTransferOutputStream::~HttpChunkedTransferOutputStream()
{
}

bool HttpChunkedTransferOutputStream::write(const char* b, unsigned int length)
{
   bool rval = true;
   
   if(length > 0)
   {
      // update data sent
      mDataSent += length;
      
      // get the chunk-size
      string chunkSize = Convert::intToHex(length);
      
      // write chunk-size
      rval &= mOutputStream->write(chunkSize.c_str(), chunkSize.length());
      
      // write CRLF
      rval &= mOutputStream->write(HttpHeader::CRLF, 2);
      
      // write chunk data
      rval &= mOutputStream->write(b, length);
      
      // write CRLF
      rval &= mOutputStream->write(HttpHeader::CRLF, 2);
   }
   
   return rval;
}

void HttpChunkedTransferOutputStream::close()
{
   // write chunk-size of "0"
   char c = '0';
   mOutputStream->write(&c, 1);
   
   // write CRLF
   mOutputStream->write(HttpHeader::CRLF, 2);
   
   if(mTrailers != NULL)
   {
      // send content-length header to ensure it is accurate
      mTrailers->setField("Content-Length", mDataSent);
      
      // write out header
      string str;
      mTrailers->toString(str);
      mOutputStream->write(str.c_str(), str.length());
   }
   else
   {
      // write out last CRLF
      mOutputStream->write(HttpHeader::CRLF, 2);
   }
   
   // reset data sent
   mDataSent = 0;
   
   // do not close underlying stream
}
