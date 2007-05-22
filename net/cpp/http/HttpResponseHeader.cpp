/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpResponseHeader.h"

using namespace std;
using namespace db::net::http;

HttpResponseHeader::HttpResponseHeader()
{
}

HttpResponseHeader::~HttpResponseHeader()
{
}

string HttpResponseHeader::getStartLine()
{
   char code[10];
   sprintf(code, "%d", getStatusCode()); 
   return "HTTP/" + getVersion() + " " + code + " " + getStatusMessage();
}

void HttpResponseHeader::setVersion(const string& version)
{
   mVersion = version;
}

const string& HttpResponseHeader::getVersion()
{
   return mVersion;
}

void HttpResponseHeader::setStatus(unsigned int code, const string& message)
{
   mStatusCode = code;
   mStatusMessage = message;
}

unsigned int HttpResponseHeader::getStatusCode()
{
   return mStatusCode;
}

const string& HttpResponseHeader::getStatusMessage()
{
   return mStatusMessage;
}
