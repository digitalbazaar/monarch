/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpRequestHeader_H
#define HttpRequestHeader_H

#include "HttpHeader.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpRequestHeader is the header for an HTTP Request Message. It contains
 * a Request-Line, and, optionally, a collection of HTTP header fields.
 * 
 * Here is an example of a HTTP/1.1 request header, each line is terminated
 * with a CRLF (there is a blank line at the end):
 * 
 * GET /some/path/ HTTP/1.1
 * Host: www.someurl.com
 * Connection: close
 * Accept-Encoding: gzip
 * Accept: image/gif, text/html
 * User-Agent: Mozilla 4.0
 * 
 * 
 * @author Dave Longley
 */
class HttpRequestHeader : public HttpHeader
{
protected:
   /**
    * The method (i.e. "GET", "POST") for the request.
    */
   std::string mMethod;
   
   /**
    * The path for the request.
    */
   std::string mPath;
   
   /**
    * The version (major.minor) for the request.
    */
   std::string mVersion;
   
public:
   /**
    * Creates a new HttpRequestHeader.
    */
   HttpRequestHeader();
   
   /**
    * Destructs this HttpRequestHeader.
    */
   virtual ~HttpRequestHeader();
   
   /**
    * Gets the start line for this HttpHeader.
    * 
    * @return the startLine for this HttpHeader.
    */
   virtual std::string getStartLine();
   
   /**
    * Sets the HTTP method for the request.
    * 
    * @param method the method for the request (i.e. "GET", "POST").
    */
   virtual void setMethod(const std::string& method);
   
   /**
    * Gets the HTTP method for the request.
    * 
    * @return the method for the request (i.e. "GET", "POST").
    */
   virtual const std::string& getMethod();
   
   /**
    * Sets the HTTP version for the request (major.minor).
    * 
    * @param version the version for the request (i.e. "1.0", "1.1").
    */
   virtual void setVersion(const std::string& version);
   
   /**
    * Gets the HTTP version for the request (major.minor).
    * 
    * @return the version for the request (i.e. "1.0", "1.1").
    */
   virtual const std::string& getVersion();
   
   /**
    * Sets the path for the request.
    * 
    * @param path the path for the request.
    */
   virtual void setPath(const std::string& path);
   
   /**
    * Gets the path for the request.
    * 
    * @return the path for the request.
    */
   virtual const std::string& getPath();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
