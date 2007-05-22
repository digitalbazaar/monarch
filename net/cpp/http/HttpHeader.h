/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpHeader_H
#define HttpHeader_H

#include "Object.h"
#include <map>
#include <string>

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpHeader is the header for an HTTP Message. It contains a start-line,
 * and, optionally, a collection of HTTP header fields.
 * 
 * Both http requests and responses use an HttpHeader. A request uses a
 * request-line for its start-line and a response uses a status-line for its
 * start-line.
 * 
 * A multipart http message will use a MIME boundary as the start-line for
 * its HttpHeader.
 * 
 * @author Dave Longley
 */
class HttpHeader : public virtual db::rt::Object
{
protected:
   /**
    * The map containing the headers.
    */
   std::map<std::string, std::string> mHeaders;
   
public:
   /**
    * The standard line break for HTTP, carriage return + line feed.
    */
   static const char CRLF[];
   
   /**
    * Creates a new HttpHeader.
    */
   HttpHeader();
   
   /**
    * Destructs this HttpHeader.
    */
   virtual ~HttpHeader();
   
   /**
    * Gets the start line for this HttpHeader.
    * 
    * @return the startLine for this HttpHeader.
    */
   virtual std::string getStartLine() = 0;
   
   /**
    * Sets a header value.
    * 
    * @param header the header to set.
    * @param value the value for the header.
    */
   virtual void setHeader(const std::string& header, const std::string& value);
   
   /**
    * Adds a value to an existing header. If the header does not exist,
    * it will be created.
    * 
    * @param header the header to update.
    * @param value the value to add to the header.
    */
   virtual void addHeader(const std::string& header, const std::string& value);
   
   /**
    * Removes a header.
    * 
    * @param header the header to remove.
    */
   virtual void removeHeader(const std::string& header);
   
   /**
    * Gets a header value.
    * 
    * @param header the header to get the value of.
    * @param value the value to populate.
    * 
    * @return true if the header exists, false if not.
    */
   virtual bool getHeader(const std::string& header, std::string& value);
   
   /**
    * BiCapitalizes the passed header so that it is normalized as an
    * HTTP header. HTTP headers do not *have* to be BiCapitalized, its
    * just the standard practice -- and it makes headers more readable.
    * 
    * @param header the header to BiCapitalize.
    */
   static void biCapitalize(std::string& header);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
