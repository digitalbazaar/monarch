/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import com.db.common.logging.Logger;

/**
 * An http (HyperText Transfer Protocol) web response header. An instance
 * of this class holds a http response line along with a collection of
 * headers in key-value pair format for the http web response.
 * 
 * @author Dave Longley
 */
public class HttpWebResponseHeader extends HttpHeader
{
   /**
    * The response status code for this http web response header.
    */
   protected String mStatusCode;
   
   /**
    * Creates a new http web response header.
    */
   public HttpWebResponseHeader()
   {
      // set defaults
      setVersion("HTTP/1.1");
      setStatusCode("404 Not Found");
   }
   
   /**
    * Parses this http web response header from a string.
    * 
    * @param header the response header in string format.
    * @return true if the header was successfully parsed, false if not.
    */   
   public boolean parse(String header)
   {
      boolean rval = false;
      
      // reset members
      setVersion("");
      setStatusCode("");
      clearHeaders();
      
      try
      {
         // get the first line of the header
         int index = header.indexOf(HttpHeader.CRLF);
         if(index != -1)
         {
            String responseLine = header.substring(0, index);
            String[] split = responseLine.split(" ");
            if(split.length > 2)
            {
               setVersion(split[0]);
               setStatusCode(joinArray(split, 1, split.length, " "));
               
               getLogger().debug("http web response version=" + getVersion() +
                                 ", status code=" + getStatusCode());

               // parse headers
               parseHeaders(header, index, header.length());
               
               rval = true;
            }
            else
            {
               getLogger().debug("http web response line does not have 2 " +
                                 "components (version, status code)!");
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      if(!rval || !isValid())
      {
         getLogger().debug("http web response header is invalid!," +
                           "header=\n" + header);
      }
      
      return rval;      
   }
   
   /**
    * Sets the status code for this http web response header.
    * 
    * @param statusCode the status code for this http web response header (i.e.
    *                   "404 Not Found", "200 OK").
    */
   public void setStatusCode(String statusCode)
   {
      mStatusCode = statusCode;
   }
   
   /**
    * Gets the status code for this http web response header.
    * 
    * @return the status code for this http web response header (i.e.
    *         "404 Not Found", "200 OK").
    */
   public String getStatusCode()
   {
      return mStatusCode;
   }
   
   /**
    * Sets this header to use a 200 OK response.
    */
   public void useOKStatusCode()
   {
      setStatusCode("200 OK");
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
         // response valid so far
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Converts this http web response header object to string format.
    * 
    * @returns this http web response header object in string format.
    */   
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      // add date header
      addDateHeader();

      // http response line format: HTTP/1.0 404 Not Found
      sb.append(getVersion());
      sb.append(' ');
      sb.append(getStatusCode());
      sb.append(CRLF);
      
      // add headers
      sb.append(super.toString());
      
      return sb.toString();      
   }
}
