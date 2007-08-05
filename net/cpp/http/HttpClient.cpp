/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpSocket.h"
#include "Url.h"

using namespace db::net;
using namespace db::net::http;

HttpClient::HttpClient()
{
}

HttpClient::~HttpClient()
{
}

HttpConnection* HttpClient::connect(
   InternetAddress* address, unsigned int timeout)
{
   HttpConnection* rval = NULL;
   
   // connect with given timeout
   TcpSocket* s = new TcpSocket();
   if(s->connect(address, timeout))
   {
      rval = new HttpConnection(new Connection(s, true), true);
   }
   else
   {
      // close and clean up socket
      s->close();
      delete s;
   }
   
   return rval;
}
