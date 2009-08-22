/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_CookieJar_H
#define db_http_CookieJar_H

#include "db/http/HttpHeader.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"

namespace db
{
namespace http
{

/**
 * A Cookie is a small piece of state information sent by a server to a client
 * that will be resent by the client in its next communication with the server.
 *
 * Cookie
 * {
 *    "name" : string,
 *    "value" : string,
 *    "comment" : string,
 *    "maxAge" : int32,
 *    "secure" : boolean,
 *    "httpOnly" : boolean,
 *    "path" : string,
 *    "domain" : string,
 *    "version" : int32
 * }
 *
 * @member name the name of the cookie.
 * @member value the value of the cookie.
 * @member comment an optional comment string.
 * @member maxAge the maximum age of the cookie in seconds.
 * @member secure true if the cookie must be sent over a secure protocol.
 * @member httpOnly true to restrict access to the cookie from javascript.
 * @member path the path to the cookie.
 * @member domain the optional domain the cookie belongs to
 *                (must start with a dot).
 * @member version the optional version of the cookie.
 */
typedef db::rt::DynamicObject Cookie;
typedef db::rt::DynamicObjectIterator CookieIterator;

/**
 * A CookieJar stores cookies and is capable of being serialized to or from
 * an HttpHeader.
 *
 * @author Dave Longley
 */
class CookieJar
{
public:
   /**
    * The possible origin of cookies. Either cookies were sent by a server
    * or they were sent by a client.
    */
   enum CookieOrigin
   {
      Server, Client
   };

protected:
   /**
    * The current cookies, mapped by name to Cookie.
    */
   db::rt::DynamicObject mCookies;

public:
   /**
    * Creates a new CookieJar.
    */
   CookieJar();

   /**
    * Destructs this CookieJar.
    */
   virtual ~CookieJar();

   /**
    * Reads cookies into this jar from the passed HttpHeader. Any duplicate
    * cookies set in this jar will be overwritten.
    *
    * The cookies will be read from either the Set-Cookie header or the
    * Cookie header based on the value of "origin." If the origin is
    * Server, the cookies will be read from "Set-Cookie", if it is Client,
    * it will be read from "Cookie".
    *
    * @param header the header to read the cookies from.
    * @param origin the CookieOrigin.
    */
   virtual void readCookies(HttpHeader* header, CookieOrigin origin);

   /**
    * Writes the cookies in this jar to the passed HttpHeader.
    *
    * The cookies will be written to either the Set-Cookie header or the
    * Cookie header based on the value of "origin." If the origin is
    * Server, the cookies will be written to "Set-Cookie", if it is Client,
    * they will be written to "Cookie".
    *
    * The cookie header's value will be appended to unless overwrite is
    * specified as true, in which case any existing cookies will be overwritten.
    *
    * @param header the header to write the cookies to.
    * @param origin the CookieOrigin.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeCookies(
      HttpHeader* header, CookieOrigin origin, bool overwrite = false);

   /**
    * Sets a cookie in this jar. It must be written out to an HttpHeader
    * to send it somewhere.
    *
    * @param cookie the cookie to set in this jar; it will *not* be cloned.
    */
   virtual void setCookie(Cookie& cookie);

   /**
    * Sets a cookie in this jar. It must be written out to an HttpHeader
    * to send it somewhere.
    *
    * @param name the name of the cookie.
    * @param value the value of this cookie.
    * @param maxAge the maximum age of the cookie in seconds, 0 to expire and
    *               -1 for infinite.
    * @param secure true if the cookie should only be sent using a secure
    *               protocol, i.e. SSL, false if it doesn't matter.
    * @param httpOnly true to restrict access to the cookie from javascript.
    * @param path the path to the cookie, defaults to "/".
    * @param domain the domain the cookie belongs to (must start with a dot),
    *               defaults to none.
    * @param version the cookie version, defaults to 0 to send no version.
    */
   virtual void setCookie(
      const char* name, const char* value, int maxAge, bool secure,
      bool httpOnly = true, const char* path = "/",
      const char* domain = NULL, int version = 0);

   /**
    * Gets a cookie from this jar.
    *
    * @param name the name of the cookie to get.
    *
    * @return the cookie (set to null if no such cookie).
    */
   virtual Cookie getCookie(const char* name);

   /**
    * Marks an existing cookie to expire. This is different from removing
    * a cookie from this jar; this method will set the specified cookie's
    * value to NULL and maximum age to 0.
    *
    * @param name the name of the cookie to delete.
    * @param secure true if the cookie is secure, false if not.
    */
   virtual void deleteCookie(const char* name, bool secure);

   /**
    * Removes a cookie entirely from this jar. This is different from expiring
    * a cookie which can be done via deleteCookie().
    *
    * @param name the name of the cookie to remove.
    *
    * @return true if a cookie was remove, false if not.
    */
   virtual bool removeCookie(const char* name);

   /**
    * Clears all cookies from this jar. This will actually only remove the
    * cookies from this jar, it not set them to expire.
    */
   virtual void clearCookies();
};

} // end namespace http
} // end namespace db
#endif
