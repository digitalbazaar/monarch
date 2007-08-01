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
   
   // start line ends with CRLF
   string::size_type lineEnd = str.find(CRLF);
   if(lineEnd != string::npos)
   {
      // get start line
      string line = str.substr(0, lineEnd);
      
      // tokenize on spaces
      StringTokenizer st(line, ' ');
      if(st.getTokenCount() == 3)
      {
         setMethod(st.getToken(0));
         setPath(st.getToken(1));
         setVersion(st.getToken(2));
         
         rval = true;
      }
      else
      {
         setMethod("");
         setPath("");
         setVersion("");
      }
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
