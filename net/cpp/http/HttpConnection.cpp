/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace std;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;

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

db::io::IOException* HttpConnection::sendHeader(HttpHeader* header)
{
   // FIXME:
   return NULL;
}

db::io::IOException* HttpConnection::receiveHeader(HttpHeader* header)
{
   // FIXME:
   return NULL;
}

db::io::IOException* HttpConnection::sendBody(
   HttpHeader* header, InputStream* is)
{
   // FIXME:
   return NULL;
   
//   // update http connection content bytes written (reset as necessary)
//   if(getContentBytesWritten() > Math::HALF_MAX_LONG_VALUE)
//   {
//      setContentBytesWritten(0);
//   }
//   
//   setContentBytesWritten(getContentBytesWritten() + numBytes);
}

db::io::IOException* HttpConnection::receiveBody(
   HttpHeader* header, OutputStream* os)
{
   // FIXME:
   return NULL;
   
//   // update http connection content bytes read (reset as necessary)
//   if(getContentBytesRead() > Math::HALF_MAX_LONG_VALUE)
//   {
//      setContentBytesRead(0);
//   }
//   
//   setContentBytesRead(getContentBytesRead() + numBytes);
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
