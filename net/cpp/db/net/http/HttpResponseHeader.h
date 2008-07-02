/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpResponseHeader_H
#define db_net_http_HttpResponseHeader_H

#include "db/net/http/HttpHeader.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpResponseHeader is the header for an HTTP Response Message. It
 * contains a Status-Line, and, optionally, a collection of HTTP header fields.
 * 
 * Here is an example of a HTTP/1.1 response header, each line is
 * terminated with a CRLF (there is a blank line at the end):
 * 
 * HTTP/1.1 200 OK
 * Server: Some Server
 * Date: Sat, 21 Jan 2006 19:15:46 GMT
 * Content-Encoding: gzip
 * Content-Length: 400
 * Content-Type: text/html
 * Connection: close
 * 
 * 
 * @author Dave Longley
 */
class HttpResponseHeader : public HttpHeader
{
protected:
   /**
    * The status code (i.e. "200", "404") for the response.
    */
   unsigned int mStatusCode;
   
   /**
    * The status message (i.e. "OK", "Not Found") for the response.
    */
   char* mStatusMessage;
   
public:
   /**
    * Creates a new HttpResponseHeader.
    */
   HttpResponseHeader();
   
   /**
    * Destructs this HttpResponseHeader.
    */
   virtual ~HttpResponseHeader();
   
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
    */
   virtual void getStartLine(std::string& line);
   
   /**
    * Returns true if this header has a special start line, false if not.
    * 
    * @return true if this header has a special start line, false if not.
    */
   virtual bool hasStartLine();
   
   /**
    * Sets the HTTP status for the response.
    * 
    * @param code the status code (i.e. "200", "404").
    * @param message the status message (i.e. "OK", "Not Found").
    */
   virtual void setStatus(unsigned int code, const char* message);
   
   /**
    * Gets the HTTP status code for the response.
    * 
    * @return the status code for the response (i.e. "200", "404").
    */
   virtual unsigned int getStatusCode();
   
   /**
    * Gets the HTTP status message for the response.
    * 
    * @return the status message for the response (i.e. "OK", "Not Found").
    */
   virtual const char* getStatusMessage();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
