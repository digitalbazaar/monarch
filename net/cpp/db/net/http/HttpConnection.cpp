/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpConnection.h"
#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/net/http/HttpBodyOutputStream.h"
#include "db/net/http/HttpChunkedTransferInputStream.h"
#include "db/net/http/HttpChunkedTransferOutputStream.h"
#include "db/rt/Thread.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;

const unsigned long long MAX_ULONG_VALUE = 0xffffffffffffffffLL;
const unsigned long HALF_MAX_LONG_VALUE =
   (unsigned long)(MAX_ULONG_VALUE / 2);

HttpConnection::HttpConnection(Connection* c, bool cleanup) :
   WebConnection(c, cleanup)
{
   // no content bytes read yet
   mContentBytesRead = 0;
   
   // no content bytes written yet
   mContentBytesWritten = 0;
   
   // resize output buffer comfortably
   getOutputStream()->resizeBuffer(1024);
}

HttpConnection::~HttpConnection()
{
}

WebRequest* HttpConnection::createRequest()
{
   // create HttpRequest
   return new HttpRequest(this);
}

inline bool HttpConnection::sendHeader(HttpHeader* header)
{
   return header->write(getOutputStream()) && getOutputStream()->flush();
}

bool HttpConnection::receiveHeader(HttpHeader* header)
{
   bool rval = true;
   
   // read until eof, error, or blank line w/CRLF
   string headerStr;
   string line;
   ConnectionInputStream* is = getInputStream();
   int read;
   while((read = is->readCrlf(line)) > 0 && line.length() > 0)
   {
      headerStr.append(line);
      headerStr.append(HttpHeader::CRLF);
   }
   
   if(read == -1)
   {
      // read failed
      rval = false;
   }
   else
   {
      // parse header
      if(!header->parse(headerStr))
      {
         ExceptionRef e = new IOException(
            "Could not receive HTTP header!", "db.net.http.BadRequest");
         Exception::setLast(e, false);
      }
   }
   
   return rval;
}

bool HttpConnection::sendBody(
   HttpHeader* header, InputStream* is, HttpTrailer* trailer)
{
   bool rval = true;
   
   // create HttpBodyOutputStream
   HttpBodyOutputStream os(this, header, trailer);
   
   // determine how much content needs to be read
   long long contentLength = 0;
   bool lengthUnspecified = true;
   if(header->getField("Content-Length", contentLength) && contentLength >= 0)
   {
      lengthUnspecified = false;
   }
   
   // vars for read/write
   unsigned int length = 2048;
   char b[length];
   int numBytes = 0;
   
   // do unspecified length transfer
   if(lengthUnspecified)
   {
      // read in content, write out to connection
      while(rval && (numBytes = is->read(b, length)) > 0)
      {
         // write out to connection
         rval = os.write(b, numBytes);
      }
   }
   else
   {
      // do specified length transfer:
      
      // read in content, write out to connection
      unsigned long long contentRemaining = contentLength;
      unsigned int readSize = (contentRemaining < length) ?
         contentRemaining : length;
      while(rval && contentRemaining > 0 &&
            (numBytes = is->read(b, readSize)) > 0)
      {
         // write out to connection
         if(rval = os.write(b, numBytes))
         {
            contentRemaining -= numBytes;
            readSize = (contentRemaining < length) ? contentRemaining : length;
         }
      }
      
      // check to see if content is remaining
      if(rval)
      {
         if(contentRemaining > 0)
         {
            rval = false;
            Thread* t = Thread::currentThread();
            if(t->isInterrupted())
            {
               // FIXME:
               // we will probably want this to be more robust in the
               // future so this kind of exception can be recovered from
               ExceptionRef e = new IOException(
                  "Sending HTTP content body interrupted!");
               Exception::setLast(e, false);
            }
            else
            {
               ExceptionRef e = new IOException(
                  "Could not read HTTP content bytes to send!");
               Exception::setLast(e, false);
            }
         }
      }
   }
   
   // close body stream (will not close underlying stream)
   os.close();
   
   // check read error
   rval = (numBytes != -1);
   
   return rval;
}

OutputStream* HttpConnection::getBodyOutputStream(
   HttpHeader* header, HttpTrailer* trailer)
{
   return new HttpBodyOutputStream(this, header, trailer);
}

bool HttpConnection::receiveBody(
   HttpHeader* header, OutputStream* os, HttpTrailer* trailer)
{
   bool rval = true;
   
   InputStream* is = getInputStream();
   
   // wrap input stream if using chunked transfer encoding
   HttpChunkedTransferInputStream* chunkin = NULL;
   string transferEncoding;
   if(header->getField("Transfer-Encoding", transferEncoding))
   {
      if(strncasecmp(transferEncoding.c_str(), "chunked", 7) == 0)
      {
         is = chunkin = new HttpChunkedTransferInputStream(
            getInputStream(), trailer);
      }
   }
   
   // determine how much content needs to be received
   long long contentLength = 0;
   bool lengthUnspecified = true;
   if(header->getField("Content-Length", contentLength) && contentLength >= 0)
   {
      lengthUnspecified = false;
   }
   
   // vars for read/write
   unsigned int length = 2048;
   char b[length];
   int numBytes = 0;
   
   // do chunked or unspecified length transfer
   if(chunkin != NULL || lengthUnspecified)
   {
      unsigned long long start = getContentBytesRead();
      
      // read in from connection, write out content
      while(rval && (numBytes = is->read(b, length)) > 0)
      {
         // update http connection content bytes read (reset as necessary)
         if(getContentBytesRead() > HALF_MAX_LONG_VALUE)
         {
            setContentBytesRead(0);
         }
         
         setContentBytesRead(getContentBytesRead() + numBytes);
         
         // write out content
         rval = os->write(b, numBytes);
      }
      
      // update trailer with content length
      if(trailer != NULL)
      {
         if(start > getContentBytesRead())
         {
            // assume single overflow
            trailer->update(start + getContentBytesRead());
         }
         else
         {
            // assume no overflow
            trailer->update(getContentBytesRead() - start);
         }
      }
      
      if(chunkin != NULL)
      {
         // clean up chunkin
         chunkin->close();
         delete chunkin;
      }
   }
   else
   {
      // do specified length transfer:
      
      // read in from connection, write out content
      unsigned long long contentRemaining = contentLength;
      unsigned int readSize = (contentRemaining < length) ?
         contentRemaining : length;
      while(rval && contentRemaining > 0 &&
            (numBytes = is->read(b, readSize)) > 0)
      {
         contentRemaining -= numBytes;
         readSize = (contentRemaining < length) ? contentRemaining : length;
         
         // update http connection content bytes read (reset as necessary)
         if(getContentBytesRead() > HALF_MAX_LONG_VALUE)
         {
            setContentBytesRead(0);
         }
         
         setContentBytesRead(getContentBytesRead() + numBytes);
         
         // write out content
         rval = os->write(b, numBytes);
      }
      
      // see if content is remaining
      if(rval && contentRemaining > 0)
      {
         rval = false;
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
      else if(rval && contentRemaining == 0 && trailer != NULL)
      {
         // update trailer with content length
         trailer->update(contentLength);
      }
   }
   
   // check read error
   rval = (numBytes != -1);
   
   return rval;
}

void HttpConnection::setContentBytesRead(unsigned long long count)
{
   mContentBytesRead = count;
}

unsigned long long HttpConnection::getContentBytesRead()
{
   return mContentBytesRead;
}

void HttpConnection::setContentBytesWritten(unsigned long long count)
{
   mContentBytesWritten = count;
}

unsigned long long HttpConnection::getContentBytesWritten()
{
   return mContentBytesWritten;
}
