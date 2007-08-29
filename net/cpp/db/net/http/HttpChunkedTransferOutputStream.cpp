/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpChunkedTransferOutputStream.h"
#include "db/util/Convert.h"
#include "db/util/Math.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpChunkedTransferOutputStream::HttpChunkedTransferOutputStream(
   ConnectionOutputStream* os, HttpTrailer* trailer) :
FilterOutputStream(os, false)
{
   // store http trailer
   mTrailer = trailer;
   
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
   
   if(mTrailer != NULL)
   {
      // update the trailer
      mTrailer->update(mDataSent);
      
      // write out trailer
      string str;
      mTrailer->toString(str);
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
