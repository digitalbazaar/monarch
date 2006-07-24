/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.TimeZone;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An http (HyperText Transfer Protocol) header. An instance
 * of this class holds a http request or http response line along
 * with a collection of custom headers in key-value pair format.
 * 
 * @author Dave Longley
 */
public abstract class HttpHeader
{
   /**
    * The version for this http header.
    */
   protected String mVersion;   

   /**
    * A map of the custom headers to their values.
    */
   protected HashMap mHeaders;
   
   /**
    * Stores the boundary for multipart http messages.
    */
   protected String mBoundary;
   
   /**
    * Standard line break, carriage return + line feed.
    */
   public static final String CRLF = "\r\n";   
   
   /**
    * Creates a new http header object.
    */
   public HttpHeader()
   {
      // set defaults
      setVersion("HTTP/1.1");
      
      // create headers map
      mHeaders = new HashMap();
      
      // no multipart boundary by default
      setBoundary(null);
   }
   
   /**
    * BiCapitalizes a header. 
    * 
    * @param header the header to biCapitalize.
    * 
    * @return the biCapitalized header.
    */
   protected String biCapitalizeHeader(String header)
   {
      StringBuffer sb = new StringBuffer(header.length());
      
      // bicapitalize the header
      StringTokenizer st = new StringTokenizer(header, "-", true);
      while(st.hasMoreTokens())
      {
         String token = st.nextToken();
         if(token.length() > 1)
         {
            char c = token.charAt(0);
            c = Character.toUpperCase(c);
            sb.append(c);
            sb.append(token.substring(1, token.length()));
         }
         else
         {
            token.toUpperCase();
            sb.append(token);
         }
      }
      
      return sb.toString();
   }
   
   /**
    * Sets the boundary for multipart http messages.
    * 
    * @param boundary the boundary for multipart http messages.
    */
   protected void setBoundary(String boundary)
   {
      mBoundary = boundary;
   }
   
   /**
    * Joins an array of strings together using the passed delimiter starting
    * at the given begin index and stopping at the given end index.
    * 
    * @param array the string array to join.
    * @param beginIndex the index to start joining at.
    * @param endIndex the index to stop joining at (the value at this
    *                 index is not included). 
    * @param delimiter the delimiter to join the array with.
    * 
    * @return the joined string.
    */
   protected String joinArray(String[] array, int beginIndex, int endIndex,
                              String delimiter)
   {
      StringBuilder sb = new StringBuilder();
      
      for(int i = beginIndex; i < endIndex; i++)
      {
         sb.append(array[i]);
         
         if(i < (endIndex - 1))
         {
            sb.append(delimiter);
         }
      }
      
      return sb.toString();
   }
   
   /**
    * Parses the headers out of the passed string and stores them in
    * this object.
    * 
    * @param headers the string of headers to parse.
    * @param start the index to start parsing at.
    * @param stop the index to stop parsing at.
    */
   public void parseHeaders(String headers, int start, int stop)
   {
      if(start == 0 && headers.length() == stop)
      {
         parseHeaders(headers);
      }
      else
      {
         parseHeaders(headers.substring(start, stop));
      }
   }

   /**
    * Parses the headers out of the passed string and stores them in
    * this object.
    * 
    * @param headers the string of headers to parse.
    */
   public void parseHeaders(String headers)
   {
      try
      {
         StringTokenizer st = new StringTokenizer(headers, CRLF);
         while(st.hasMoreTokens())
         {
            String token = st.nextToken();
            
            int index = token.indexOf(": ");
            if(index != -1)
            {
               // get header and value
               String header = token.substring(0, index); 
               String value = "";
               if(token.length() > index + 2)
               {
                  value = token.substring(index + 2, token.length());
               }
               
               addHeader(header, value);
               getLogger().debug(getClass(),
                  "http header='" + header + "',value='" + value + "'");
               
               // check for boundary
               if(header.equalsIgnoreCase("Content-Type"))
               {
                  index = value.toLowerCase().indexOf("boundary=");
                  if(index != -1)
                  {
                     String boundary = "";
                     
                     // boundary= is 9 characters 
                     if(value.length() > index + 9)
                     {
                        boundary = value.substring(index + 9, value.length());
                     }
                     
                     setBoundary("--" + boundary);
                     getLogger().debug(getClass(),
                        "http boundary='" + boundary + "'");
                  }
               }
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
   }
   
   /**
    * Parses this http header.
    * 
    * @param header the request header in string format.
    * 
    * @return true if the header was successfully parsed, false if not.
    */
   public abstract boolean parse(String header);
   
   /**
    * Adds a header.
    * 
    * @param header the name for the header.
    * @param value the value for the header.
    */
   public void addHeader(String header, String value)
   {
      if(value == null)
      {
         removeHeader(header);
      }
      else
      {
         mHeaders.put(biCapitalizeHeader(header), value);
      }
   }
   
   /**
    * Adds all of the headers in the given map.
    * 
    * @param headers the headers map.
    */
   public void addHeaders(Map headers)
   {
      for(Iterator i = headers.keySet().iterator(); i.hasNext();) 
      {
         String header = (String)i.next();
         String value = (String)headers.get(header);
         addHeader(header, value);
      }
   }
   
   /**
    * Removes a header.
    * 
    * @param header the name for the header.
    */
   public void removeHeader(String header)
   {
      mHeaders.remove(biCapitalizeHeader(header));
   }
   
   /**
    * Returns true if this http header has the passed header, false if not.
    * 
    * @param header the header to look for.
    * 
    * @return true if this http header has the passed header, false if not.
    */
   public boolean hasHeader(String header)
   {
      return getHeader(header) != null;
   }
   
   /**
    * Gets the value of a header.
    * 
    * @param header the name of the header.
    * @return the value of the header.
    */
   public String getHeader(String header)
   {
      return (String)mHeaders.get(biCapitalizeHeader(header));
   }
   
   /**
    * Sets all of the headers to the ones in the given map.
    * 
    * @param headers the headers map.
    */
   public void setHeaders(Map headers)
   {
      // clear the existing headers
      clearHeaders();
      
      // add headers in the passed map
      addHeaders(headers);
   }
   
   /**
    * Gets all of the headers in a map. Any changes to this map will
    * be reflected in this object.
    * 
    * @return the headers map.
    */
   public HashMap getHeaders()
   {
      return mHeaders;
   }
   
   /**
    * Clears all of the current headers from this object.
    */
   public void clearHeaders()
   {
      mHeaders.clear();
      setBoundary(null);
   }
   
   /**
    * Sets the version for this http header.
    * 
    * @param version the version for this http header (i.e. "HTTP/1.0").
    */
   public void setVersion(String version)
   {
      mVersion = version;
   }
   
   /**
    * Gets the version for this http header.
    * 
    * @return the version for this http header (i.e. "HTTP/1.0").
    */
   public String getVersion()
   {
      return mVersion;
   }   
   
   /**
    * Sets the content-type.
    * 
    * @param contentType the content-type.
    */
   public void setContentType(String contentType)
   {
      addHeader("Content-Type", contentType);
   }
    
   /**
    * Gets the content-type.
    * 
    * @return the content-type.
    */
   public String getContentType()
   {
      return getHeader("Content-Type");
   }
   
   /**
    * Sets the content-encoding value.
    * 
    * @param contentEncoding the content-encoding value.
    */
   public void setContentEncoding(String contentEncoding)
   {
      addHeader("Content-Encoding", contentEncoding);
   }
    
   /**
    * Gets the content-encoding value.
    * 
    * @return the content-encoding header value.
    */
   public String getContentEncoding()
   {
      return getHeader("Content-Encoding");
   }
   
   /**
    * Sets the transfer-encoding value.
    * 
    * @param transferEncoding the transfer-encoding header value.
    */
   public void setTransferEncoding(String transferEncoding)
   {
      addHeader("Transfer-Encoding", transferEncoding);
   }

   /**
    * Gets the transfer-encoding value.
    * 
    * @return the transfer-encoding header value.
    */
   public String getTransferEncoding()
   {
      return getHeader("Transfer-Encoding");
   }   
   
   /**
    * Sets the content-length (in bytes).
    * 
    * @param length the content-length, pass a negative number to set no
    *        content length.
    */
   public void setContentLength(long length)
   {
      if(length < 0)
      {
         // length is negative, remove content-length header
         removeHeader("Content-Length");
      }
      else
      {
         addHeader("Content-Length", "" + length);
      }
   }
   
   /**
    * Gets the content-length (in bytes).
    * 
    * @return the content-length, -1 is returned if no content-length is set.
    */
   public long getContentLength()
   {
      long rval = -1;
      
      try
      {
         String len = getHeader("Content-Length");
         if(len != null)
         {
            rval = Long.parseLong(getHeader("Content-Length"));
         }
      }
      catch(Throwable t)
      {
      }
      
      return rval;
   }
   
   /**
    * Sets the content disposition header value. This is useful for setting 
    * the file name that the body of an http message should be saved as
    * if it is not html or plain text. For example:
    * 
    * setContentDisposition("attachment; filename=myFile.dat");
    * 
    * The above would provide a hint to a web browser or server on what the
    * file should be named.
    *
    * @param contentDisposition the content disposition.   
    */
   public void setContentDisposition(String contentDisposition)
   {
      addHeader("Content-Disposition", contentDisposition);
   }

   /**
    * Gets the content disposition header value. 
    * 
    * @return the content disposition header value. 
    */
   public String getContentDisposition()
   {
      return getHeader("Content-Disposition");
   }
   
   /**
    * Gets a the value for content-disposition key.
    * 
    * Same as "parseContentDispositionValue()".
    * 
    * @param key the key for the value to retrieve.
    * 
    * @return the value for the key (null if not found).
    */
   public String getContentDispositionValue(String key)
   {
      return parseContentDispositionValue(key);
   }

   /**
    * Parses out the value from the content-disposition header value.
    * 
    * Same as "getContentDispositionValue()".
    * 
    * @param key the key for the value to retrieve.
    * 
    * @return the value for the key (null if not found).
    */
   public String parseContentDispositionValue(String key)
   {
      String value = null;
      
      String contentDisposition = getContentDisposition();
      if(contentDisposition != null)
      {
         int startIndex = contentDisposition.indexOf(key + "=");
         int index = startIndex + key.length() + 1;
         if(startIndex != -1 && contentDisposition.length() > index)
         {
            // check for semicolon
            int endIndex = contentDisposition.indexOf(";", index);
            if(endIndex != -1)
            {
               // go to semicolon
               value = contentDisposition.substring(index, endIndex);
            }
            else
            {
               // go to end of content disposition
               value = contentDisposition.substring(index);
            }
            
            // strip starting and ending quotes, if any
            if(value != null)
            {
               if(value.startsWith("\""))
               {
                  if(value.length() > 1)
                  {
                     value = value.substring(1);
                  }
               }
               
               if(value.endsWith("\""))
               {
                  if(value.length() > 1)
                  {
                     value = value.substring(0, value.length() - 1);
                  }
               }
            }
         }
      }
      
      return value;
   }  
   
   /**
    * Sets the connection header.
    * 
    * @param connection the connection value.
    */
   public void setConnection(String connection)
   {
      addHeader("Connection", connection);
   }
    
   /**
    * Gets the connection header value.
    * 
    * @return the connection header value.
    */
   public String getConnection()
   {
      return getHeader("Connection");
   }
   
   /**
    * Gets whether or not the header indicates that a web connection
    * should be kept alive.
    * 
    * @return true if a web connection should be kept alive, false if not.
    */
   public boolean isKeepAlive()
   {
      boolean rval = false;
      
      String connection = getConnection();
      if(connection == null)
      {
         if(getVersion().equals("HTTP/1.1"))
         {
            rval = true;
         }
      }
      else if(connection.equalsIgnoreCase("Keep-Alive"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Adds a date header to this http header using the current date.
    */
   public void addDateHeader()
   {
      // add date
      SimpleDateFormat df = new SimpleDateFormat("EEE, d MMM yyyy HH:mm:ss");
      TimeZone tz = TimeZone.getTimeZone("GMT");
      df.setTimeZone(tz);
      
      addHeader("Date", df.format(new Date()) + " GMT");
   }
   
   /**
    * Gets the boundary for multipart http messages.
    * 
    * @return the boundary for multipart http messages.
    */
   public String getBoundary()
   {
      return mBoundary;
   }
   
   /**
    * Gets the end boundary for multipart http messages.
    * 
    * @return the end boundary for multipart messages.
    */
   public String getEndBoundary()
   {
      String endBoundary = null;
      
      if(getBoundary() != null)
      {
         endBoundary = getBoundary() + "--";
      }
      
      return endBoundary;
   }
   
   /**
    * Returns true if this header is for a multipart http message, false
    * if not.
    * 
    * @return true if this header is for a multipart http message,
    *         false if not. 
    */
   public boolean isMultipart()
   {
      return (getBoundary() != null);
   }
   
   /**
    * Sets the date header for this http header object to the current date.
    */
   public void setDateHeader()
   {
      // add date
      SimpleDateFormat df = new SimpleDateFormat("EEE, d MMM yyyy HH:mm:ss");
      TimeZone tz = TimeZone.getTimeZone("GMT");
      df.setTimeZone(tz);
      
      addHeader("Date", df.format(new Date()) + " GMT");
   }
   
   /**
    * Returns true if this header is valid, false if it is not.
    * 
    * @return true if this header is valid, false if it is not.
    */
   public boolean isValid()
   {
      boolean rval = false;
      
      // see if the version is valid
      if(getVersion().startsWith("HTTP"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Converts this http header object to string format.
    * 
    * @returns this http header object in string format.
    */
   public String toString()
   {
      StringBuffer headers = new StringBuffer();
      
      // iterate through all headers
      Iterator i = getHeaders().keySet().iterator();
      while(i.hasNext())
      {
         String header = (String)i.next();
         headers.append(header);
         headers.append(": ");
         headers.append(getHeader(header));
         headers.append(CRLF);
      }
      
      // add line break
      headers.append(CRLF);
      
      return headers.toString();
   }
   
   /**
    * Gets this header in bytes.
    * 
    * @return this header in bytes.
    */
   public byte[] getBytes()
   {
      return toString().getBytes();
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
