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

bool HttpChunkedTransferOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   if(length > 0)
   {
      // update data sent
      mDataSent += length;
      
      // get the chunk-size
      string chunkSize = Convert::intToHex(length);
      
      // write chunk-size
      // write CRLF
      // write chunk data
      // write CRLF
      rval =
         mOutputStream->write(chunkSize.c_str(), chunkSize.length()) &&
         mOutputStream->write(HttpHeader::CRLF, 2) &&
         mOutputStream->write(b, length) &&
         mOutputStream->write(HttpHeader::CRLF, 2);
   }
   
   return rval;
}

void HttpChunkedTransferOutputStream::close()
{
   // write chunk-size of "0" and CRLF
   char c = '0';
   bool write =
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
