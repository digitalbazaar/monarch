/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpRequestHeader.h"
#include "StringTokenizer.h"

#include <sstream>

using namespace std;
using namespace db::net;
using namespace db::net::http;
using namespace db::util;

HttpRequestHeader::HttpRequestHeader()
{
}

HttpRequestHeader::~HttpRequestHeader()
{
}

bool HttpRequestHeader::parseStartLine(const std::string& str)
{
   bool rval = false;
   
   // tokenize on spaces
   StringTokenizer st(str.c_str(), ' ');
   if(st.getTokenCount() == 3)
   {
      setMethod(st.nextToken());
      setPath(st.nextToken());
      setVersion(st.nextToken());
      
      rval = true;
   }
   else
   {
      setMethod("");
      setPath("");
      setVersion("");
   }
   
   return rval;
}

void HttpRequestHeader::getStartLine(string& line)
{
   line.append(getMethod() + " " + getPath() + " " + getVersion());
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
