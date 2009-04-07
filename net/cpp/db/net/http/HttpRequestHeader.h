/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpRequestHeader_H
#define db_net_http_HttpRequestHeader_H

#include "db/net/http/HttpHeader.h"

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
 * GET /some/path/?query HTTP/1.1
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
   char* mMethod;
   
   /**
    * The path for the request.
    */
   char* mPath;
   
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
    * Parses the start line for this HttpHeader from the passed string.
    * 
    * @param str the string to parse from.
    * @param length the length of the start line (no null character included).
    * 
    * @return true if the start line could be parsed, false if not.
    */
   virtual bool parseStartLine(const char* str, unsigned int length);
   
   /**
    * Gets the start line for this HttpHeader.
    * 
    * @param line the startLine to populate.
    * 
    * @return true if the start line could be parsed, false if not.
    */
   virtual void getStartLine(std::string& line);
   
   /**
    * Returns true if this header has a special start line, false if not.
    * 
    * @return true if this header has a special start line, false if not.
    */
   virtual bool hasStartLine();
   
   /**
    * Sets the HTTP method for the request.
    * 
    * @param method the method for the request (i.e. "GET", "POST").
    */
   virtual void setMethod(const char* method);
   
   /**
    * Gets the HTTP method for the request.
    * 
    * @return the method for the request (i.e. "GET", "POST").
    */
   virtual const char* getMethod();
   
   /**
    * Sets the path for the request.
    * 
    * @param path the path for the request.
    */
   virtual void setPath(const char* path);
   
   /**
    * Gets the path for the request.
    * 
    * @return the path for the request.
    */
   virtual const char* getPath();
   
   /**
    * Writes the contents of this request header into the passed one.
    * 
    * @param header the header to write to.
    */
   virtual void writeTo(HttpRequestHeader* header);
   
   /**
    * Gets the type of header this is.
    * 
    * @return the type of header this is.
    */
   virtual Type getType();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
