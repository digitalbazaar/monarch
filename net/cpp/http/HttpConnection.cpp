/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpChunkedTransferInputStream.h"
#include "HttpChunkedTransferOutputStream.h"
#include "Math.h"
#include "Thread.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

HttpConnection::HttpConnection(Connection* c, bool cleanup) :
   WebConnection(c, cleanup)
{
   // no content bytes read yet
   mContentBytesRead = 0;
   
   // no content bytes written yet
   mContentBytesWritten = 0;
}

HttpConnection::~HttpConnection()
{
}

WebRequest* HttpConnection::createRequest()
{
   // create HttpRequest
   return new HttpRequest(this);
}

IOException* HttpConnection::sendHeader(HttpHeader* header)
{
   IOException* rval = NULL;
   
   string out;
   header->toString(out);
   if(!getOutputStream()->write(out.c_str(), out.length()))
   {
      rval = (IOException*)Exception::getLast();
   }
   
   return rval;
}

IOException* HttpConnection::receiveHeader(HttpHeader* header)
{
   IOException* rval = NULL;
   
   // read until eof, error, or blank line w/CRLF
   string headerStr;
   string line;
   bool read;
   ConnectionInputStream* is = getInputStream();
   Exception::setLast(NULL);
   while(is->readCrlf(line) && line != "")
   {
      headerStr.append(line);
      headerStr.append(HttpHeader::CRLF);
   }
   
   if(Exception::getLast() != NULL)
   {
      rval = (IOException*)Exception::getLast();
   }
   else
   {
      // parse header
      if(!header->parse(headerStr))
      {
         rval = new IOException(
            "Could not receive HTTP header!", "db.net.http.BadRequest");
         Exception::setLast(rval);
      }
   }
   
   return rval;
}

IOException* HttpConnection::sendBody(HttpHeader* header, InputStream* is)
{
   IOException* rval = NULL;
   
   OutputStream* os = getOutputStream();
   
   // wrap output stream if using chunked transfer encoding
   HttpChunkedTransferOutputStream* chunkout = NULL;
   string transferEncoding;
   if(header->getHeader("Transfer-Encoding", transferEncoding))
   {
      if(transferEncoding.find("chunked") != string::npos)
      {
         os = chunkout = new HttpChunkedTransferOutputStream(
            getOutputStream(), header);
      }
   }
   
   // determine how much content needs to be read
   long long contentLength = 0;
   bool lengthUnspecified = true;
   if(header->getHeader("Content-Length", contentLength) && contentLength > 0)
   {
      lengthUnspecified = false;
   }
   
   // vars for read/write
   bool writeError = false;
   unsigned int length = 2048;
   char b[length];
   int numBytes = 0;
   
   // do chunked or unspecified length transfer
   if(chunkout != NULL || lengthUnspecified)
   {
      // read in content, write out to connection
      while(!writeError && (numBytes = is->read(b, length)) != -1)
      {
         // write out to connection
         if(os->write(b, numBytes))
         {
            // update http connection content bytes written (reset as necessary)
            if(getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
            {
               setContentBytesWritten(0);
            }
            
            setContentBytesWritten(getContentBytesWritten() + numBytes);
         }
         else
         {
            writeError = true;
         }
      }
      
      if(chunkout != NULL)
      {
         if(!writeError)
         {
            // close chunkout
            chunkout->close();
         }
         
         // clean up chunkout
         delete chunkout;
      }
   }
   else
   {
      // do specified length transfer:
      
      // read in content, write out to connection
      unsigned long long contentRemaining = contentLength;
      unsigned int readSize = Math::minimum(contentRemaining, length);
      while(!writeError && contentRemaining > 0 &&
            (numBytes = is->read(b, readSize)) != -1)
      {
         // write out to connection
         if(os->write(b, numBytes))
         {
            contentRemaining -= numBytes;
            readSize = Math::minimum(contentRemaining, length);
            
            // update http connection content bytes written (reset as necessary)
            if(getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
            {
               setContentBytesWritten(0);
            }
            
            setContentBytesWritten(getContentBytesWritten() + numBytes);
         }
         else
         {
            writeError = true;
         }
      }
      
      // check to see if an error occurred: if a write error occurred or
      // if content is remaining
      if(writeError)
      {
         rval = (IOException*)Exception::getLast();
      }
      else if(contentRemaining > 0)
      {
         Thread* t = Thread::currentThread();
         if(t->isInterrupted())
         {
            // we will probably want this to be more robust in the
            // future so this kind of exception can be recovered from
            rval = new IOException(
               "Sending HTTP content body interrupted!");
         }
         else
         {
            rval = new IOException(
               "Could not read HTTP content bytes to send!");
         }
         
         Exception::setLast(rval);
      }
   }
   
   return rval;
}

IOException* HttpConnection::receiveBody(HttpHeader* header, OutputStream* os)
{
   IOException* rval = NULL;
   
   InputStream* is = getInputStream();
   
   // wrap input stream if using chunked transfer encoding
   HttpChunkedTransferInputStream* chunkin = NULL;
   string transferEncoding;
   if(header->getHeader("Transfer-Encoding", transferEncoding))
   {
      if(transferEncoding.find("chunked") != string::npos)
      {
         is = chunkin = new HttpChunkedTransferInputStream(
            getInputStream(), header);
      }
   }
   
   // determine how much content needs to be received
   long long contentLength = 0;
   bool lengthUnspecified = true;
   if(header->getHeader("Content-Length", contentLength) && contentLength > 0)
   {
      lengthUnspecified = false;
   }
   
   // vars for read/write
   bool writeError = false;
   unsigned int length = 2048;
   char b[length];
   int numBytes = 0;
   
   // do chunked or unspecified length transfer
   if(chunkin != NULL || lengthUnspecified)
   {
      // read in from connection, write out content
      while(!writeError && (numBytes = is->read(b, length)) != -1)
      {
         // update http connection content bytes read (reset as necessary)
         if(getContentBytesRead() > Math::HALF_MAX_LONG_VALUE)
         {
            setContentBytesRead(0);
         }
         
         setContentBytesRead(getContentBytesRead() + numBytes);
         
         // write out content
         writeError = !os->write(b, numBytes);
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
      unsigned int readSize = Math::minimum(contentRemaining, length);
      while(!writeError && contentRemaining > 0 &&
            (numBytes = is->read(b, readSize)) != -1)
      {
         contentRemaining -= numBytes;
         readSize = Math::minimum(contentRemaining, length);
         
         // update http connection content bytes read (reset as necessary)
         if(getContentBytesRead() > Math::HALF_MAX_LONG_VALUE)
         {
            setContentBytesRead(0);
         }
         
         setContentBytesRead(getContentBytesRead() + numBytes);
         
         // write out content
         writeError = !os->write(b, numBytes);
      }
      
      // check to see if an error occurred: if a write error occurred or
      // if content is remaining
      if(writeError)
      {
         rval = (IOException*)Exception::getLast();
      }
      else if(contentRemaining > 0)
      {
         Thread* t = Thread::currentThread();
         if(t->isInterrupted())
         {
            // we will probably want this to be more robust in the
            // future so this kind of exception can be recovered from
            rval = new IOException(
               "Receiving HTTP content body interrupted!");
         }
         else
         {
            rval = new IOException(
               "Could not receive all HTTP content bytes!");
         }
         
         Exception::setLast(rval);
      }
   }
   
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
