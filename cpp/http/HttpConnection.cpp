/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpConnection.h"

#include "monarch/io/IOException.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/http/HttpBodyInputStream.h"
#include "monarch/http/HttpBodyOutputStream.h"
#include "monarch/http/HttpChunkedTransferInputStream.h"
#include "monarch/http/HttpChunkedTransferOutputStream.h"
#include "monarch/rt/Thread.h"

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;

HttpConnection::HttpConnection(Connection* c, bool cleanup) :
   ConnectionWrapper(c, cleanup)
{
   // no content bytes read yet
   mContentBytesRead = 0;

   // no content bytes written yet
   mContentBytesWritten = 0;
}

HttpConnection::~HttpConnection()
{
}

HttpRequest* HttpConnection::createRequest()
{
   // create HttpRequest
   return new HttpRequest(this);
}

inline bool HttpConnection::sendHeader(HttpHeader* header)
{
   // resize output buffer to hold header, send header
   ConnectionOutputStream* os = getOutputStream();
   os->resizeBuffer(1024);
   return header->write(os) && os->flush();
}

bool HttpConnection::receiveHeader(HttpHeader* header)
{
   bool rval = true;

   // read until eof, error, or blank line w/CRLF
   string headerStr;
   string line;
   ConnectionInputStream* is = getInputStream();
   // FIXME: read a few bytes first to check for valid HTTP data to prevent
   // DOS attacks? add maximum line cap param to readCrlf() on input stream?
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
      // check for empty header (due to end of stream)
      if(headerStr.length() == 0)
      {
         ExceptionRef e = new Exception(
            "No HTTP header found.",
            "monarch.http.NoHeader");
         Exception::set(e);
         rval = false;
      }
      // parse header
      else if(!header->parse(headerStr))
      {
         ExceptionRef e = new Exception(
            "Could not receive HTTP header.",
            "monarch.http.BadHeader");
         Exception::set(e);
         rval = false;
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
   int64_t contentLength = 0;
   bool lengthUnspecified = true;
   if(header->getField("Content-Length", contentLength) && contentLength >= 0)
   {
      lengthUnspecified = false;
   }

   // vars for read/write
   unsigned int length = 2048;
   mBuffer.clear();
   mBuffer.allocateSpace(length, true);
   int numBytes = 0;

   // do unspecified length transfer
   if(lengthUnspecified)
   {
      // read in content, write out to connection
      while(rval && (numBytes = mBuffer.put(is, length)) > 0)
      {
         // write out to connection
         rval = os.write(mBuffer.data(), numBytes);
         mBuffer.clear();
      }
   }
   else
   {
      // do specified length transfer:

      // read in content, write out to connection
      int64_t contentRemaining = contentLength;
      int readSize = (contentRemaining < length) ? contentRemaining : length;
      while(rval && contentRemaining > 0 &&
            (numBytes = mBuffer.put(is, readSize)) > 0)
      {
         // write out to connection
         if((rval = os.write(mBuffer.data(), numBytes)))
         {
            contentRemaining -= numBytes;
            readSize = (contentRemaining < length) ? contentRemaining : length;
         }
         mBuffer.clear();
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
                  "Sending HTTP content body interrupted.");
               Exception::set(e);
            }
            else
            {
               ExceptionRef e = new IOException(
                  "Could not read HTTP content bytes to send.");
               Exception::set(e);
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

   // create HttpBodyInputStream
   HttpBodyInputStream is(this, header, trailer);

   // vars for read/write
   unsigned int length = 2048;
   mBuffer.clear();
   mBuffer.allocateSpace(length, true);
   int numBytes = 0;

   // read in from connection, write out content
   // Note: keep reading even if content output stream fails
   while((numBytes = mBuffer.put(&is, length)) > 0)
   {
      // write out content if no error yet
      rval = rval && os->write(mBuffer.data(), numBytes);
      mBuffer.clear();
   }

   // close input stream (will not close underlying stream)
   is.close();

   // check read error
   rval = (rval && numBytes != -1);

   return rval;
}

InputStream* HttpConnection::getBodyInputStream(
   HttpHeader* header, HttpTrailer* trailer)
{
   return new HttpBodyInputStream(this, header, trailer);
}

inline void HttpConnection::setContentBytesRead(uint64_t count)
{
   mContentBytesRead = count;
}

inline uint64_t HttpConnection::getContentBytesRead()
{
   return mContentBytesRead;
}

inline void HttpConnection::setContentBytesWritten(uint64_t count)
{
   mContentBytesWritten = count;
}

inline uint64_t HttpConnection::getContentBytesWritten()
{
   return mContentBytesWritten;
}
