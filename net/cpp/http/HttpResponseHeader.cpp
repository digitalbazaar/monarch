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

void HttpResponseHeader::getStartLine(string& line)
{
   char code[10];
   sprintf(code, "%d", getStatusCode()); 
   line.append("HTTP/" + getVersion() + " " + code + " " + getStatusMessage());
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
