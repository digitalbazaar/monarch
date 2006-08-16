/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.http;

import com.db.logging.Logger;

/**
 * An http (HyperText Transfer Protocol) body part header. An instance
 * of this class holds a boundary line along with a collection of
 * headers in key-value pair format for the http body part.
 * 
 * An example http message that uses multiparts is below (each line in
 * the headers is terminated with a CRLF):
 * 
 *****************************************************************************
 * 
 * MIME-Version: 1.0
 * Content-Type: Multipart/Related; boundary=MIME_boundary; type=text/xml;
 *         start="theEnvelope"
 * Content-Description: This is the optional message description.
 * 
 * --MIME_boundary
 * Content-Type: text/xml; charset=UTF-8
 * Content-Transfer-Encoding: 8bit
 * Content-ID: theEnvelope
 * 
 * <?xml version='1.0' ?>
 * <SOAP-ENV:Envelope>
 * <SOAP-ENV:Body>
 * <content id="theImage"/>
 * </SOAP-ENV:Body>
 * </SOAP-ENV:Envelope>
 *
 * --MIME_boundary
 * Content-Type: image/jpeg
 * Content-Transfer-Encoding: binary
 * Content-ID: theImage
 * 
 * ...binary JPEG image...
 * --MIME_boundary--
 * 
 *****************************************************************************
 *
 * The body parts of the above message are the text that falls between each
 * boundary. 
 * 
 * @author Dave Longley
 */
public class HttpBodyPartHeader extends HttpHeader
{
   /**
    * The boundary line for this header.
    */
   protected String mBoundaryLine;
   
   /**
    * Creates a new http body request header.
    * 
    * @param boundary the boundary line for this header.
    */
   public HttpBodyPartHeader(String boundary)
   {
      mBoundaryLine = boundary;
   }
   
   /**
    * Parses this http body part header from a string.
    * 
    * @param header the body part header in string format.
    * @return true if the header was successfully parsed, false if not.
    */
   public boolean parse(String header)
   {
      boolean rval = false;
      
      // reset members
      clearHeaders();
      
      try
      {
         // get the first line of the header
         int index = header.indexOf(CRLF);
         if(index != -1)
         {
            // skip the line if it is the boundary line
            String line = header.substring(0, index);
            if(line.equals(mBoundaryLine))
            {
               // skip the line, parse the headers
               parseHeaders(header, index, header.length());
            }
            else
            {
               // boundary line already passed, parse headers
               parseHeaders(header, 0, header.length());
            }
            
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      if(!rval || !isValid())
      {
         getLogger().debug(getClass(),
            "http body part header is invalid!,header=\n" + header);
      }
      
      return rval;
   }
   
   /**
    * Sets the Content-ID header.
    * 
    * @param contentId the Content-ID value.
    */
   public void setContentId(String contentId)
   {
      addHeader("Content-ID", contentId);
   }
    
   /**
    * Gets the Content-ID header value.
    * 
    * @return the Content-ID header value.
    */
   public String getContentId()
   {
      return getHeader("Content-ID");
   }   
   
   /**
    * Returns true if this header is valid, false if it is not.
    * 
    * @return true if this header is valid, false if it is not.
    */
   public boolean isValid()
   {
      // body part headers assumed valid
      return true;
   }
}
