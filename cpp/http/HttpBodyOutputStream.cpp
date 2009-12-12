/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpBodyOutputStream.h"

#include "monarch/http/HttpChunkedTransferOutputStream.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Math.h"

using namespace std;
using namespace db::io;
using namespace db::http;
using namespace db::net;
using namespace db::rt;
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
            static_cast<ConnectionOutputStream*>(mOutputStream), trailer);
         mCleanupOutputStream = true;

         // let chunked transfer output stream use its own buffering
         connection->getOutputStream()->resizeBuffer(0);
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
      rval = mOutputStream->write(b, length);
      if(!rval)
      {
         // see if send would block
         length = 0;
         ExceptionRef e = Exception::get();
         if(e->getDetails()->hasMember("wouldBlock"))
         {
            // use number of bytes sent
            length = e->getDetails()["written"]->getInt32();
         }
      }

      if(length > 0)
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
      // flush and finish underlying stream
      if((rval = mOutputStream->flush() && mOutputStream->finish()))
      {
         // update http connection content bytes written (reset as necessary)
         if(mConnection->getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
         {
            mConnection->setContentBytesWritten(0);
         }

         mConnection->setContentBytesWritten(
            mConnection->getContentBytesWritten());
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
