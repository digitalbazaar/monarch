/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpTransferChunkedOutputStream.h"
#include "Convert.h"

#include <string>

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpTransferChunkedOutputStream::HttpTransferChunkedOutputStream(
   ConnectionOutputStream* os, HttpHeader* header) :
FilterOutputStream(os, false)
{
   // store http header
   mHeader = header;
}

HttpTransferChunkedOutputStream::~HttpTransferChunkedOutputStream()
{
}

void HttpTransferChunkedOutputStream::write(const char* b, unsigned int length)
throw(IOException)
{
   if(length > 0)
   {
      // get the chunk-size
      string chunkSize = Convert::intToHex(length);
      
      // write chunk-size
      mOutputStream->write(chunkSize.c_str(), chunkSize.length());
      
      // write CRLF
      mOutputStream->write(HttpHeader::CRLF, 2);
      
      // write chunk data
      mOutputStream->write(b, length);
      
      // FIXME: increment content bytes written
      // update http connection content bytes written
      //hwc.setContentBytesWritten(hwc.getContentBytesWritten() + length);
      
      // write CRLF
      mOutputStream->write(HttpHeader::CRLF, 2);
   }
}

void HttpTransferChunkedOutputStream::close() throw(IOException)
{
   // write chunk-size of "0"
   mOutputStream->write('0');
   
   // write CRLF
   mOutputStream->write(HttpHeader::CRLF, 2);
   
   // send content-length header
   string value;
   mHeader->getHeader("Content-Length", value);
   mOutputStream->write("Content-Length: ", 16);
   mOutputStream->write(value.c_str(), value.length());
   mOutputStream->write(HttpHeader::CRLF, 2);
   
   // write out last CRLF
   mOutputStream->write(HttpHeader::CRLF, 2);
   
   // do not close underlying stream
}
