/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Url_H
#define Url_H

#include "Object.h"
#include "MalformedUrlException.h"
#include <string>

namespace db
{
namespace net
{

/**
 * A Url represents a Uniform Resource Locator (URL).
 * 
 * A URL is written as follows:
 * 
 * <scheme>:<scheme-specific-part>
 * 
 * A common, but not required, syntax for urls is as follows:
 * 
 * <scheme>://<authority><path>?<query>
 * 
 * @author Dave Longley
 */
class Url : public virtual db::rt::Object
{
protected:
   /**
    * The scheme for this url.
    */
   std::string mScheme;
   
   /**
    * The scheme-specific-part for this url.
    */
   std::string mSchemeSpecificPart;
   
   /**
    * The authority part of this url, if one exists.
    */
   std::string mAuthority;
   
   /**
    * The path of this url, if one exists.
    */
   std::string mPath;
   
   /**
    * The query of this url, if one exists.
    */
   std::string mQuery;
   
public:
   /**
    * Creates a new Url from the passed string. An exception may be raised
    * if the url is malformed.
    *
    * @param url the string to create this Url from.
    */
   Url(const std::string& url);
   
   /**
    * Destructs this Url.
    */
   virtual ~Url();
   
   /**
    * Sets this Url to the passed string.
    * 
    * @param url the string to create this Url from.
    * 
    * @return a MalformedUrlException if the URL is malformed, NULL if not.
    */
   virtual MalformedUrlException* setUrl(const std::string& url);
   
   /**
    * Gets the scheme for this url.
    * 
    * @return the scheme for this url.
    */
   virtual const std::string& getScheme();
   
   /**
    * Gets the scheme-specific-part for this url.
    *
    * @return the scheme-specific-part for this url.
    */
   virtual const std::string& getSchemeSpecificPart();
   
   /**
    * Gets the authority of this url, if one exists.
    * 
    * @return the authority of this url or a blank string.
    */
   virtual const std::string& getAuthority();
   
   /**
    * Gets the path of this url, if one exists.
    * 
    * @return the path of this url or a blank string.
    */
   virtual const std::string& getPath();
   
   /**
    * Gets the query of this url, if one exists.
    * 
    * @return the query of this url or a blank string.
    */
   virtual const std::string& getQuery();
   
   /**
    * Gets the default port for the scheme (protocol) of this url.
    * 
    * @return the default port for the scheme (protocol) of this url.
    */
   virtual unsigned int getDefaultPort();
   
   /**
    * Writes this url to the passed string.
    * 
    * @param str the string to store this url in.
    * 
    * @return the string representation for this url.
    */
   virtual std::string& toString(std::string& str);
   
   /**
    * URL-encodes the passed string.
    * 
    * @param str the string to URL-encode.
    * @param length the length of the string.
    * 
    * @return the URL-encoded string.
    */
   static std::string encode(const char* str, unsigned int length);
   
   /**
    * URL-decodes the passed string.
    * 
    * @param str the string to URL-decode.
    * @param length the length of the string.
    * 
    * @return the URL-decoded string.
    */
   static std::string decode(const char* str, unsigned int length);
};

} // end namespace net
} // end namespace db
#endif
