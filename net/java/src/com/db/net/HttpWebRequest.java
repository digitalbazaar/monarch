/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * An http (HyperText Transfer Protocol) web request.
 * 
 * Here is an example of a HTTP/1.1 web request header, each line is terminated
 * with a CRLF:
 * 
 * GET /some/path/ HTTP/1.1
 * Host: www.someurl.com
 * Connection: close
 * Accept-Encoding: gzip
 * Accept: image/gif, text/html
 * User-Agent: Mozilla 4.0
 * 
 * 
 * @author Dave Longley
 */
public class HttpWebRequest extends WebRequest
{
   /**
    * The header for this request.
    */
   protected HttpWebRequestHeader mHeader;
   
   /**
    * Creates a new http web request.
    * 
    * @param hwc the http web connection used to send or receive this request.
    */   
   public HttpWebRequest(HttpWebConnection hwc)
   {
      super(hwc);
      
      // create header
      mHeader = new HttpWebRequestHeader();
   }
   
   /**
    * Sends the request header of this http web request.
    * 
    * @return true if the header was successfully sent, false if not.
    */
   public boolean sendHeader()
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendHeader(getHeader());
      
      return rval;
   }
   
   /**
    * Sends an http body part header inside of this request.
    * 
    * @param header the http body part header to send.
    * 
    * @return true if an http body part header could be sent, false if not. 
    */
   public boolean sendBodyPartHeader(HttpBodyPartHeader header)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartHeader(header, getHeader().getBoundary());
      
      return rval;
   }
   
   /**
    * Sends the body of this http web request as a string.
    * 
    * @param body the body of this http web request as a string.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(String body)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(body, getHeader().getContentEncoding());
      
      return rval;
   }
   
   /**
    * Sends the body of this http web request as a byte array.
    * 
    * @param body the body of this http web request as a byte array.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] body)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(body, getHeader().getContentEncoding());
      
      return rval;
   }

   /**
    * Sends the body of this http web request as a byte array.
    * 
    * @param buffer the buffer containing the http body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the body in bytes.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] buffer, int offset, int length)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(buffer, offset, length,
                          getHeader().getContentEncoding());
      
      return rval;
   }   
   
   /**
    * Sends the body of this http web request. The body is read from the
    * passed input stream.
    *
    * @param is the input stream to read the body from.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(InputStream is)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(is, getHeader().getContentEncoding());
      
      return rval;
   }
   
   /**
    * Sends an http body part body for this http web request as a string.
    * 
    * @param body the body as a string.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(String body, boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(body, getHeader().getContentEncoding(),
                                  getHeader().getBoundary(), lastBodyPart);
      
      return rval;      
   }
   
   /**
    * Sends an http body part body for this http web request as a byte array.
    * 
    * @param body the body as a byte array.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(byte[] body, boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(body, getHeader().getContentEncoding(),
                                  getHeader().getBoundary(), lastBodyPart);
      
      return rval;        
   }

   /**
    * Sends an http body part body for this http web request as a byte array.
    * 
    * @param buffer the buffer containing the http body part body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the http body part body in bytes.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(byte[] buffer, int offset, int length,
                                   boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(buffer, offset, length,
                                  getHeader().getContentEncoding(),
                                  getHeader().getBoundary(), lastBodyPart);
      
      return rval;      
   }
   
   /**
    * Sends an http body part body for this http web request. The body is
    * read from the passed input stream.
    *
    * @param is the input stream to read the body from.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(InputStream is, boolean lastBodyPart)   
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(is, getHeader().getContentEncoding(),
                                  getHeader().getBoundary(), lastBodyPart);
      
      return rval;      
   }
   
   /**
    * Receives the request header of this http web request.
    * 
    * @return true if the header was successfully received, false if not.
    */
   public boolean receiveHeader()
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveHeader(getHeader());
      
      return rval;
   }
   
   /**
    * Receives an http body part header.
    * 
    * @return the received http body part header or null if one could not
    *         be received.
    */
   public HttpBodyPartHeader receiveBodyPartHeader()
   {
      HttpBodyPartHeader header =
         new HttpBodyPartHeader(getHeader().getBoundary());
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      if(!hwc.receiveHeader(header))
      {
         header = null;
      }
      
      return header;
   }
   
   /**
    * Receives the body of this http web request and returns it as
    * a string.
    * 
    * @return the body of this http web request as a string, or null if
    *         the body could not be received.
    */
   public String receiveBodyString()
   {
      return receiveBodyString(getHeader().getContentEncoding());
   }
   
   /**
    * Receives the body of this http web request and returns it as
    * a string.
    * 
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return the body of this http web request as a string, or null if
    *         the body could not be received.
    */
   public String receiveBodyString(String contentEncoding)
   {
      String rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyString(contentEncoding,
                                   getHeader().getContentLength());
      
      return rval;
   }
   
   /**
    * Receives the body of this http web request and returns it as
    * a byte array.
    * 
    * @return the body of this http web request as a byte array, or null if
    *         the body could not be received.
    */
   public byte[] receiveBody()
   {
      return receiveBody(getHeader().getContentEncoding());
   }
   
   /**
    * Receives the body of this http web request and returns it as
    * a byte array.
    * 
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return the body of this http web request as a byte array, or null if
    *         the body could not be received.
    */
   public byte[] receiveBody(String contentEncoding)
   {
      byte[] rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBody(contentEncoding, getHeader().getContentLength());
      
      return rval;
   }
   
   /**
    * Receives the body of this http web request and writes it to the
    * passed output stream.
    *
    * @param os the output stream to write the request body to.
    * 
    * @return true if the body was successfully received, false if not.
    */
   public boolean receiveBody(OutputStream os)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBody(os, getHeader().getContentEncoding(),
                             getHeader().getContentLength());
      
      return rval;
   }
   
   /**
    * Receives an http body part body as a string.
    *
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return the body of the body part as a string, or null if the body
    *         could not be received. 
    */
   public String receiveBodyPartBodyString(String contentEncoding)
   {
      String rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBodyString(contentEncoding,
                                           getHeader().getBoundary());
      
      return rval;
   }
   
   /**
    * Receives an http body part body as a byte array.
    *
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return the body of the body part as a byte array, or null if the body
    *         could not be received. 
    */
   public byte[] receiveBodyPartBody(String contentEncoding)
   {
      byte[] rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBody(contentEncoding,
                                     getHeader().getBoundary());
      
      return rval;
   }
   
   /**
    * Receives an http body part body and writes it to the passed
    * output stream.
    *
    * @param os the output stream to write the body to.
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return true if the body was successfully received, false if not.
    */
   public boolean receiveBodyPartBody(OutputStream os, String contentEncoding)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBody(os, contentEncoding,
                                     getHeader().getBoundary());
      
      return rval;
   }
   
   /**
    * Skips an http body part body.
    *
    * @param contentEncoding the content-encoding for the body.
    * 
    * @return true if the http body part body was successfully skipped, false
    *         if not.
    */
   public boolean skipBodyPartBody(String contentEncoding)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.skipBodyPartBody(contentEncoding, getHeader().getBoundary());
      
      return rval;
   }
   
   /**
    * Creates an http web response for this request.
    * 
    * @return an http web response for this request.
    */
   public HttpWebResponse createHttpWebResponse()
   {
      HttpWebResponse response =
         new HttpWebResponse((HttpWebConnection)getWebConnection());
      response.getHeader().setConnection(getHeader().getConnection());
      
      return response;
   }
   
   /**
    * Returns true if this web request is valid, false if it is not.
    * 
    * @return true if this web request is valid, false if it is not.
    */
   public boolean isValid()
   {
      boolean rval = false;
      
      // see if the header is valid
      rval = getHeader().isValid();
      
      return rval;
   }
   
   /**
    * Returns true if this http web request is multipart, false if not.
    * 
    * @return true if this request is multipart, false if not.
    */
   public boolean isMultipart()
   {
      boolean rval = false;
      
      // see if header is marked as multipart
      rval = getHeader().isMultipart();
      
      return rval;
   }
   
   /**
    * Returns true if this http web request is multipart and it
    * has more parts to read, false if not.
    * 
    * @return true if this http web request is multipart and it has more
    *         parts to read, false otherwise.
    */
   public boolean hasMoreBodyParts()
   {
      boolean rval = false;
      
      if(isMultipart() && getHttpWebConnection().isConnected())
      {
         String lastBoundary = getHttpWebConnection().getLastReadBoundary();
         if(lastBoundary != null &&
            lastBoundary.equals(getHeader().getEndBoundary()))
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the header for this http web request.
    * 
    * @return the header for this http web request.
    */
   public HttpWebRequestHeader getHeader()
   {
      return mHeader;
   }
   
   /**
    * Gets the http web connection for this request.
    * 
    * @return the http web connection for this request.
    */
   public HttpWebConnection getHttpWebConnection()
   {
      return (HttpWebConnection)getWebConnection();
   }
}
