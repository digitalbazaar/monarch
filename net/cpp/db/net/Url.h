/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Url_H
#define db_net_Url_H

#include "db/rt/Object.h"
#include "db/net/MalformedUrlException.h"
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
 * url = <scheme>://<authority><path>?<query>
 * authority = <userinfo>@<host>:<port>
 * userinfo = <user>:<password> 
 *
 * See also: RFC 3986
 * 
 * @author Dave Longley
 */
class Url
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
    * The userinfo of this url, if one exists.
    */
   std::string mUserInfo;
   
   /**
    * The user of this url, if one exists.
    */
   std::string mUser;
   
   /**
    * The password of this url, if one exists.
    */
   std::string mPassword;
   
   /**
    * The path of this url, if one exists.
    */
   std::string mPath;
   
   /**
    * The query of this url, if one exists.
    */
   std::string mQuery;
   
   /**
    * The host for this url, if one exists.
    */
   std::string mHost;
   
   /**
    * The port for this url, if one exists.
    */
   unsigned int mPort;
   
public:
   /**
    * Creates a new Url from the passed string. An exception may be raised
    * if the url is malformed.
    *
    * @param url the string to create this Url from.
    */
   Url(const std::string& url);
   
   /**
    * Creates a new Url from the passed string. An exception may be raised
    * if the url is malformed.
    *
    * @param url the string to create this Url from.
    */
   Url(const char* url);
   
   /**
    * Destructs this Url.
    */
   virtual ~Url();
   
   /**
    * Sets this Url equal to another one.
    * 
    * @param rhs the Url to set this one equal to.
    */
   virtual Url& operator=(const Url& rhs);
   
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
    * Gets the userinfo this url, if it exists.
    * 
    * @return the userinfo of this url or a blank string.
    */
   virtual const std::string& getUserInfo();
   
   /**
    * Gets the user of this url, if one exists.
    * 
    * @return the user of this url or a blank string.
    */
   virtual const std::string& getUser();
   
   /**
    * Gets the password of this url, if one exists.
    * 
    * @return the password of this url or a blank string.
    */
   virtual const std::string& getPassword();
   
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
    * Convenience method to get the host of this url (not including
    * the port).
    * 
    * @return the host of this url.
    */
   virtual const std::string& getHost();
   
   /**
    * Convenience method to get the port of this url.
    * 
    * @return the port of this url.
    */
   virtual unsigned int getPort();
   
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
   virtual std::string& toString(std::string& str) const;
   
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
