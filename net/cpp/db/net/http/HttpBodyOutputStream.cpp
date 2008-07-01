/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpBodyOutputStream.h"
#include "db/net/http/HttpChunkedTransferOutputStream.h"
#include "db/util/Math.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::util;

HttpBodyOutputStream::HttpBodyOutputStream(
   HttpConnection* connection, HttpHeader* header, HttpTrailer* trailer) :
FilterOutputStream(connection->getOutputStream(), false)
{
   // store connection
   mConnection = connection;
   
   // wrap output stream if using chunked transfer encoding
   string transferEncoding;
   if(header->getField("Transfer-Encoding", transferEncoding))
   {
      if(strncasecmp(transferEncoding.c_str(), "chunked", 7) == 0)
      {
         mOutputStream = new HttpChunkedTransferOutputStream(
            (ConnectionOutputStream*)mOutputStream, trailer);
         mCleanupOutputStream = true;
      }
   }
   
   // not finished yet
   mFinished = false;
}

HttpBodyOutputStream::~HttpBodyOutputStream()
{
}

bool HttpBodyOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   if(length > 0)
   {
      // write out to underlying stream
      if((rval = mOutputStream->write(b, length)))
      {
         // update http connection content bytes written (reset as necessary)
         if(mConnection->getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
         {
            mConnection->setContentBytesWritten(0);
         }
         
         mConnection->setContentBytesWritten(
            mConnection->getContentBytesWritten() + length);
      }
   }
   
   return rval;
}

bool HttpBodyOutputStream::finish()
{
   bool rval = true;
   
   if(!mFinished)
   {
      // flush underlying stream
      unsigned long long old = mConnection->getBytesWritten();
      if((rval = mOutputStream->flush()))
      {
         // update http connection content bytes written (reset as necessary)
         if(mConnection->getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
         {
            mConnection->setContentBytesWritten(0);
         }
         
         mConnection->setContentBytesWritten(
            mConnection->getContentBytesWritten() +
            (mConnection->getBytesWritten() - old));
      }
      
      if(mCleanupOutputStream)
      {
         // close underlying stream, it was created internally for
         // transfer-encoding (i.e. "chunked")
         mOutputStream->close();
      }
      
      // now finished
      mFinished = true;
   }
   
   return rval;
}

void HttpBodyOutputStream::close()
{
   // underlying stream closed by finish()
   finish();
}
