/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpHeader_H
#define db_net_http_HttpHeader_H

#include "db/io/OutputStream.h"
#include "db/rt/Collectable.h"
#include "db/rt/WindowsSupport.h"
#include "db/util/Date.h"
#include "db/util/StringTools.h"

#include <cstring>
#include <map>
#include <string>

namespace db
{
namespace net
{
namespace http
{

#ifdef WIN32
#   ifdef BUILD_DB_NET_DLL
#      define DLL_CLASS __WIN32_DLL_EXPORT
#   else
#      define DLL_CLASS __WIN32_DLL_IMPORT
#   endif
#else
#   define DLL_CLASS
#endif

/**
 * An HttpHeader is the header for an HTTP Message. It contains a version
 * and, optionally, a collection of HTTP header fields.
 * 
 * Both http requests and responses use an HttpHeader. A request uses a
 * request-line for its start-line and a response uses a status-line for its
 * start-line. Both share a version.
 * 
 * A multipart http message will use a MIME boundary as the start-line for
 * its HttpHeader.
 * 
 * @author Dave Longley
 */
class DLL_CLASS HttpHeader
{
protected:
   /**
    * The version (HTTP/major.minor) for the header.
    */
   char* mVersion;
   
   /**
    * The map containing the header fields using case-insensitive comparator to
    * compare field names.
    */
   typedef std::multimap<
      const char*, std::string, db::util::StringCaseComparator>
      FieldMap;
   FieldMap mFields;
   
   /**
    * Stores the size, in bytes, of the http header fields. This is used for
    * optimization.
    */
   size_t mFieldsSize;
   
public:
   /**
    * The standard line break for HTTP, carriage return + line feed.
    */
   static const char* CRLF;
   
   /**
    * The standard HTTP-date format, i.e. Tue, 15 Nov 1994 12:45:26 GMT.
    */
   static const char* sDateFormat;
   
   /**
    * Creates a new HttpHeader.
    */
   HttpHeader();
   
   /**
    * Destructs this HttpHeader.
    */
   virtual ~HttpHeader();
   
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
    * Sets the HTTP version for the header (HTTP/major.minor).
    * 
    * @param version the version for the header (i.e. "HTTP/1.0", "HTTP/1.1").
    */
   virtual void setVersion(const char* version);
   
   /**
    * Gets the HTTP version for the header (HTTP/major.minor).
    * 
    * @return the version for the header (i.e. "HTTP/1.0", "HTTP/1.1").
    */
   virtual const char* getVersion();
   
   /**
    * Sets a header field, replacing any existing ones.
    * 
    * @param name the name of the header field to set.
    * @param value the value for the header field.
    */
   virtual void setField(const char* name, long long value);
   
   /**
    * Sets a header field, replacing any existing ones.
    * 
    * @param name the name of the header field to set.
    * @param value the value for the header field.
    */
   virtual void setField(const char* name, const std::string& value);
   
   /**
    * Adds another field without replacing one of the same name.
    * 
    * @param name the name of the header field to add.
    * @param value the value for the header field.
    */
   virtual void addField(const char* name, const std::string& value);
   
   /**
    * Removes a header field. This will remove all fields with the given name.
    * 
    * @param name the name of the header field to remove.
    */
   virtual void removeField(const char* name);
   
   /**
    * Clears all header fields.
    */
   virtual void clearFields();
   
   /**
    * Gets the number of header fields with the given name.
    * 
    * @param name the name of the header field.
    * 
    * @return the number of header field values with the passed field name.
    */
   virtual int getFieldCount(const char* name);
   
   /**
    * Gets a header field value.
    * 
    * @param name the name of the header field to get the value of.
    * @param value the value to populate.
    * @param index the index of the field, for fields that have
    *              multiple entries.
    * 
    * @return true if the header field exists, false if not.
    */
   virtual bool getField(const char* name, long long& value, int index = 0);
   
   /**
    * Gets a header field value.
    * 
    * @param name the name of the header field to get the value of.
    * @param value the value to populate.
    * @param index the index of the field, for fields that have
    *              multiple entries.
    * 
    * @return true if the header field exists, false if not.
    */
   virtual bool getField(const char* name, std::string& value, int index = 0);
   
   /**
    * Returns true if this header has the passed field, false if not.
    * 
    * @param name the name of the header field to check for.
    * 
    * @return true if this header has the passed field, false if not.
    */
   virtual bool hasField(const char* name);
   
   /**
    * Parses this header from the passed string.
    * 
    * @param str the string to parse from.
    * 
    * @return true if the header could be parsed, false if not.
    */
   virtual bool parse(const std::string& str);
   
   /**
    * Writes this header to a string.
    * 
    * @return the string.
    */
   virtual std::string toString();
   
   /**
    * Writes this header to an OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(db::io::OutputStream* os);
   
   /**
    * Sets the GMT date for this header. A value of NULL will set the date
    * to the current GMT.
    * 
    * @param date the date to use (NULL to use the current date).
    */
   virtual void setDate(db::util::Date* date = NULL);
   
   /** 
    * Gets the GMT date for this header.
    * 
    * @param date the Date to populate.
    * 
    * @return the GMT date for this header, false if no date header exists.
    */
   virtual bool getDate(db::util::Date& date);
   
   /**
    * Returns true if this header has a Content-Length field that is non-zero
    * or a Transfer-Encoding field.
    * 
    * @return true if this header specifies content, false if not.
    */
   virtual bool hasContent();
   
   /**
    * Writes the contents of this header into the passed one.
    * 
    * @param header the header to write to.
    */
   virtual void writeTo(HttpHeader* header);
   
   /**
    * BiCapitalizes a header field so that its name is normalized as an
    * HTTP header field name. HTTP header field names do not *have* to be
    * BiCapitalized, it's just the standard practice -- and it makes header
    * field names more readable.
    * 
    * @param name the name of the header field to BiCapitalize.
    */
   static void biCapitalize(char* name);
};

#undef DLL_CLASS

// typedef for a counted reference to an HttpHeader
typedef db::rt::Collectable<HttpHeader> HttpHeaderRef;

} // end namespace http
} // end namespace net
} // end namespace db
#endif
