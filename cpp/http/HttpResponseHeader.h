/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpResponseHeader_H
#define monarch_http_HttpResponseHeader_H

#include "monarch/http/HttpHeader.h"

namespace monarch
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
   int mStatusCode;

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
    * Sets the HTTP status for the response. If message is NULL the code is
    * used to look up a default message with getDefaultStatusMessage().
    *
    * @param code the status code (i.e. "200", "404").
    * @param message the status message (i.e. "OK", "Not Found") or NULL to
    *        look up the default for the given code.
    */
   virtual void setStatus(int code, const char* message = NULL);

   /**
    * Clears the HTTP status for the response.
    */
   virtual void clearStatus();

   /**
    * Gets the HTTP status code for the response.
    *
    * @return the status code for the response (i.e. "200", "404").
    */
   virtual int getStatusCode();

   /**
    * Gets the HTTP status message for the response.
    *
    * @return the status message for the response (i.e. "OK", "Not Found").
    */
   virtual const char* getStatusMessage();

   /**
    * Writes the contents of this response header into the passed one.
    *
    * @param header the header to write to.
    */
   virtual void writeTo(HttpResponseHeader* header);

   /**
    * Gets the type of header this is.
    *
    * @return the type of header this is.
    */
   virtual Type getType();

   /**
    * Return the default HTTP status message for known codes or NULL if not
    * known.
    *
    * @param code the HTTP status code.
    *
    * @return a status message or NULL.
    */
   static const char* getDefaultStatusMessage(int code);
};

} // end namespace http
} // end namespace monarch
#endif
