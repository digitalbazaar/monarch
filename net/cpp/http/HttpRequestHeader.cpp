/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpRequestHeader.h"

using namespace std;
using namespace db::net::http;

HttpRequestHeader::HttpRequestHeader()
{
}

HttpRequestHeader::~HttpRequestHeader()
{
}

string HttpRequestHeader::getStartLine()
{
   return getMethod() + " " + getPath() + " HTTP/" + getVersion();
}

void HttpRequestHeader::setMethod(const string& method)
{
   mMethod = method;
}

const string& HttpRequestHeader::getMethod()
{
   return mMethod;
}

void HttpRequestHeader::setVersion(const string& version)
{
   mVersion = version;
}

const string& HttpRequestHeader::getVersion()
{
   return mVersion;
}

void HttpRequestHeader::setPath(const string& path)
{
   mPath = path;
}

const string& HttpRequestHeader::getPath()
{
   return mPath;
}
