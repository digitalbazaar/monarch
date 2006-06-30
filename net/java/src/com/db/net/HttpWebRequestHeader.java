/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.HashMap;

import com.db.logging.Logger;
import com.db.util.UrlCoder;

/**
 * An http (HyperText Transfer Protocol) web request header. An instance
 * of this class holds a http request line along with a collection of
 * headers in key-value pair format for the http web request.
 * 
 * @author Dave Longley
 */
public class HttpWebRequestHeader extends HttpHeader
{
   /**
    * The request method for this http web request header.
    */
   protected String mMethod;
   
   /**
    * The request path for this http request header.
    */
   protected String mPath;
   
   /**
    * Creates a new http web request header.
    */
   public HttpWebRequestHeader()
   {
      // set defaults
      setVersion("HTTP/1.1");
      setMethod("GET");
      setPath("/");
      setAccept("text/xml,text/html,image/gif,image/jpeg,*/*");
   }
   
   /**
    * Parses this http web request header from a string.
    * 
    * @param header the request header in string format.
    * @return true if the header was successfully parsed, false if not.
    */
   public boolean parse(String header)
   {
      boolean rval = false;
      
      // reset members
      setMethod("");
      setPath("");
      setVersion("");
      clearHeaders();
      
      try
      {
         // get the first line of the header
         int index = header.indexOf(CRLF);
         if(index != -1)
         {
            // parse request line
            String requestLine = header.substring(0, index);
            String[] split = requestLine.split(" ");
            if(split.length > 2)
            {
               setMethod(split[0]);
               setPath(split[1]);
               setVersion(split[2]);
               
               getLogger().debug(getClass(),
                  "http web request method=" + getMethod() +
                  ", path=" + getPath() + ", version=" + getVersion());

               // parse headers
               parseHeaders(header, index, header.length());
               
               rval = true;
            }
            else
            {
               getLogger().debug(getClass(),
                  "http web request line does not have 3 " +
                  "components (method, path, version)!");
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      if(!rval || !isValid())
      {
         getLogger().debug(getClass(),
            "http web request header is invalid!," + "header=\n" + header);
      }
      
      return rval;
   }
   
   /**
    * Sets the method for this http web request header.
    * 
    * @param method the method for this http web request header
    *               (i.e. "GET", "POST").
    */
   public void setMethod(String method)
   {
      mMethod = method;
   }
   
   /**
    * Gets the method for this http web request header.
    * 
    * @return the method for this http web request header
    *         (i.e. "GET", "POST").
    */
   public String getMethod()
   {
      return mMethod;
   }
   
   /**
    * Sets this header to use a GET method.
    */
   public void useGetMethod()
   {
      setMethod("GET");
   }
   
   /**
    * Sets this header to use a POST method.
    */
   public void usePostMethod()
   {
      setMethod("POST");
   }
   
   /**
    * Sets the full path for this http web request header.
    * 
    * @param path the full path for this http web request header.
    */
   public void setPath(String path)
   {
      mPath = path;
   }
   
   /**
    * Gets the full path for this http web request header.
    * 
    * @return the full path for this http web request header.
    */
   public String getPath()
   {
      return mPath;
   }
   
   /**
    * Gets the path from the request path. This path will not any
    * variables.
    * 
    * @return the collection of ordered path components.
    */
   public String getPathWithoutVariables()
   {
      String path = getPath();
      
      // do not include variables
      int index = path.indexOf("?");
      if(index != -1)
      {
         path = path.substring(0, index);
      }      
      
      return path;
   }
   
   /**
    * Gets the relative path from the request path. This path will not
    * include "http://" or "https://", etc, or any variables. Often, this path
    * will be exactly the same as calling getPath() as many http clients do not
    * send the absolute path.
    * 
    * @return the collection of ordered path components.
    */
   public String getRelativePathWithoutVariables()
   {
      String path = getRelativePath();
      
      // do not include variables
      int index = path.indexOf("?");
      if(index != -1)
      {
         path = path.substring(0, index);
      }      
      
      return path;
   }
   
   /**
    * Gets the relative path from the request path. This path will not
    * include "http://" or "https://", etc. Often, this path will be
    * exactly the same as calling getPath() as many http clients do not
    * send the absolute path.
    * 
    * @return the collection of ordered path components.
    */
   public String getRelativePath()
   {
      String path = getPath();
      
      try
      {
         if(path != null)
         {
            // strip schema off, if any
            int index = path.indexOf("://");
            if(index != -1)
            {
               if(path.length() > (index + 1))
               {
                  path.substring(index + 1);
                  
                  // strip host name off
                  index = path.indexOf("/");
                  if(index != -1)
                  {
                     path = "/" + path.substring(index);
                  }
               }
               else
               {
                  path = "/";
               }
            }
         }
         else
         {
            path = "/";
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not get relative path from http web request header!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return path;
   }
   
   /**
    * Gets all the components of the request path and stores them ordered from
    * most base path to least.
    * 
    * @return the collection of ordered path components.
    */
   public String[] getPathComponents()
   {
      return getPathComponents(getRelativePathWithoutVariables());
   }
   
   /**
    * Gets all the components of the passed path and stores them ordered from
    * most base path to least. Path variables are not included.
    * 
    * @param path the path to get the components of.
    * @return the collection of ordered path components.
    */
   public static String[] getPathComponents(String path)
   {
      String[] components = new String[0];
      
      // remove first slash so that blank component isn't included
      if(path.startsWith("/") && path.length() > 1)
      {
         path = path.substring(1);
      }
      
      // do not include variables
      int index = path.indexOf("?");
      if(index != -1)
      {
         path = path.substring(0, index);
      }
      
      // get path components
      components = path.split("/");
      
      return components;
   }
   
   /**
    * Gets variables from the request path, if any. Returns the variables in
    * a hash map.
    * 
    * @return the hashmap of variables.
    */
   public HashMap getPathVariables()
   {
      HashMap vars = new HashMap();
      
      String path = getPath();
      
      try
      {
         if(path != null)
         {
            // find the start of the variables
            int index = path.indexOf("?");
            if(index != -1 && path.length() > (index + 1))
            {
               // leave only the variables
               path = path.substring(index + 1);
               
               // now split the path into name/value pairs
               String[] pairs = path.split("&");
               for(int i = 0; i < pairs.length; i++)
               {
                  // split the name and value
                  String[] split = pairs[i].split("=");

                  // get the name
                  String name = split[0];
                  String value = "";
                  
                  // get the value
                  if(split.length >= 2)
                  {
                     value = UrlCoder.decode(split[1]);
                  }
                  
                  // add the variable to the map
                  vars.put(name, value);
               }
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not parse variables in url!,an exception occurred," +
            "exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return vars;
   }
   
   /**
    * Sets the user agent header for this http web request header.
    * 
    * @param userAgent the user agent header for this http web request header.
    */
   public void setUserAgent(String userAgent)
   {
      addHeader("User-Agent", userAgent);
   }
   
   /**
    * Gets the user agent header for this http web request header.
    * 
    * @return the user agent header for this http web request header.
    */
   public String getUserAgent()
   {
      return getHeader("User-Agent");
   }
   
   /**
    * Sets the host header for this http web request header.
    * 
    * @param host the host header for this http web request header.
    */
   public void setHost(String host)
   {
      addHeader("Host", host);
   }
   
   /**
    * Gets the host header for this http web request header.
    * 
    * @return the host header for this http web request header.
    */
   public String getHost()
   {
      return getHeader("Host");
   }
   
   /**
    * Sets the content types to accept for this http web request header.
    * 
    * @param accept the content types to accept for this
    *               http web request header.
    */
   public void setAccept(String accept)
   {
      addHeader("Accept", accept);
   }
   
   /**
    * Gets the content types to accept for this http web request header.
    * 
    * @return gets the content types to accept for this http web request header.
    */
   public String getAccept()
   {
      return getHeader("Accept");
   }
   
   /**
    * Sets the content encodings to accept for this http web request header.
    * 
    * @param acceptEncoding the content encodings to accept for this
    *                       http web request header.
    */
   public void setAcceptEncoding(String acceptEncoding)
   {
      addHeader("Accept-Encoding", acceptEncoding);
   }
   
   /**
    * Gets the content encodings to accept for this http web request header.
    * 
    * @return gets the content encodings to accept for this http web request
    *         header.
    */
   public String getAcceptEncoding()
   {
      return getHeader("Accept-Encoding");
   }   
   
   /**
    * Returns true if this header is valid, false if it is not.
    * 
    * @return true if this header is valid, false if it is not.
    */
   public boolean isValid()
   {
      boolean rval = false;
      
      if(super.isValid())
      {
         // request valid so far
         rval = true;
         
         // if the request is http 1.1, make sure the host header is included
         if(getVersion().equals("HTTP/1.1"))
         {
            if(getHost() == null || getHost().equals(""))
            {
               getLogger().error(getClass(),
                  "HTTP/1.1 request must include \"host\" header");
            
               // request is invalid
               rval = false;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Converts this http web request header object to string format.
    * 
    * @returns this http web request header object in string format.
    */   
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      // add date header
      addDateHeader();

      // http request line format: METHOD /path/ HTTP/1.0
      sb.append(getMethod());
      sb.append(' ');
      sb.append(getPath());
      sb.append(' ');
      sb.append(getVersion());
      sb.append(CRLF);
      
      // add headers
      sb.append(super.toString());

      return sb.toString();      
   }
}
