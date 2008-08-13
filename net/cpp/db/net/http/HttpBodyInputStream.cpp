/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpBodyInputStream.h"

#include "db/net/http/HttpChunkedTransferInputStream.h"
#include "db/rt/DynamicObject.h"
#include "db/util/Math.h"

using namespace std;
using namespace db::io;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

HttpBodyInputStream::HttpBodyInputStream(
   HttpConnection* connection, HttpHeader* header, HttpTrailer* trailer) :
FilterInputStream(connection->getInputStream(), false)
{
   // store connection, trailer
   mConnection = connection;
   mTrailer = trailer;
   
   // no bytes received yet
   mBytesReceived = 0;
   mContentLength = 0;
   mContentLengthKnown = false;
   
   // wrap input stream if using chunked transfer encoding
   mChunkedTransfer = false;
   string transferEncoding;
   if(header->getField("Transfer-Encoding", transferEncoding))
   {
      if(strncasecmp(transferEncoding.c_str(), "chunked", 7) == 0)
      {
         mChunkedTransfer = true;
         mInputStream = new HttpChunkedTransferInputStream(
            (ConnectionInputStream*)mInputStream, trailer);
         mCleanupInputStream = true;
      }
   }
   
   if(!mChunkedTransfer)
   {
      // determine how much content needs to be received
      mContentLengthKnown =
         header->getField("Content-Length", mContentLength);
      
      // see if content length was specified as a negative amount
      if(mContentLengthKnown && mContentLength < 0)
      {
         // treat as if content length isn't known
         mContentLengthKnown = false;
      }
      else if(!mContentLengthKnown)
      {
         // assume no content if content-length header is missing
         mContentLengthKnown = true;
      }
   }
}

HttpBodyInputStream::~HttpBodyInputStream()
{
}

int HttpBodyInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // do chunked or unknown length transfer
   if(mChunkedTransfer || !mContentLengthKnown)
   {
      // read in from connection
      if((rval = mInputStream->read(b, length)) > 0)
      {
         // update bytes received
         mBytesReceived += rval;
         
         // update http connection content bytes read (reset as necessary)
         if(mConnection->getContentBytesRead() > Math::HALF_MAX_LONG_VALUE)
         {
            mConnection->setContentBytesRead(0);
         }
         
         mConnection->setContentBytesRead(
            mConnection->getContentBytesRead() + rval);
      }
      
      // if finished, update trailer with content length
      if(rval == 0 && mTrailer != NULL)
      {
         mTrailer->update(mBytesReceived);
      }
   }
   // do known length transfer
   else
   {
      // read in from connection, decrement stored content length as read
      int cl = (int)(mContentLength & 0x7fffffff);
      length = (cl < length ? cl : length);
      if(mContentLength > 0 && (rval = mInputStream->read(b, length)) > 0)
      {
         // update content length, bytes received
         mContentLength -= rval;
         mBytesReceived += rval;
         
         // update http connection content bytes read (reset as necessary)
         if(mConnection->getContentBytesRead() > Math::HALF_MAX_LONG_VALUE)
         {
            mConnection->setContentBytesRead(0);
         }
         
         mConnection->setContentBytesRead(
            mConnection->getContentBytesRead() + rval);
      }
      
      // see if content is remaining
      if(rval == 0 && mContentLength > 0)
      {
         rval = 1;
         Thread* t = Thread::currentThread();
         if(t->isInterrupted())
         {
            // we will probably want this to be more robust in the
            // future so this kind of exception can be recovered from
            ExceptionRef e = new IOException(
               "Receiving HTTP content body interrupted!");
            Exception::setLast(e, false);
         }
         else
         {
            ExceptionRef e = new IOException(
               "Could not receive all HTTP content bytes!");
            Exception::setLast(e, false);
         }
      }
      else if(mTrailer != NULL)
      {
         // update trailer with bytes received
         mTrailer->update(mBytesReceived);
      }
   }
   
   return rval;
}

void HttpBodyInputStream::close()
{
   // do not close underlying stream
}
