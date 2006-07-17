/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * An http (HyperText Transfer Protocol) web response.
 * 
 * Here is an example of a HTTP/1.1 web response header, each line is
 * terminated with a CRLF:
 * 
 * HTTP/1.1 200 OK
 * Server: Some Server
 * Date: Sat, 21 Jan 2006 19:15:46 GMT
 * Content-Encoding: gzip
 * Content-Length: 400
 * Content-Type: text/html
 * Connection: close
 * 
 * 
 * @author Dave Longley
 */
public class HttpWebResponse extends WebResponse
{
   /**
    * The header for this response.
    */
   protected HttpWebResponseHeader mHeader;

   /**
    * Creates a new http web response.
    * 
    * @param hwc the http web connection used to send or receive this response.
    */
   public HttpWebResponse(HttpWebConnection hwc)
   {
      super(hwc);
      
      // create header
      mHeader = new HttpWebResponseHeader();
   }
   
   /**
    * Sends the response header of this http web response.
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
    * Sends an http body part header inside of this response.
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
    * Sends the body of this http web response as a string.
    * 
    * @param body the body of this http web response as a string.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(String body)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(body, getHeader());
      
      return rval;
   }
   
   /**
    * Sends the body of this http web response as a byte array.
    * 
    * @param body the body of this http web response as a byte array.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] body)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBody(body, getHeader());
      
      return rval;
   }   

   /**
    * Sends the body of this http web response as a byte array.
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
      rval = hwc.sendBody(buffer, offset, length, getHeader());
      
      return rval;
   }
   
   /**
    * Sends the body of this http web response. The body is read from the
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
      rval = hwc.sendBody(is, getHeader());
      
      return rval;
   }
   
   /**
    * Sends an http body part body for this http web response as a string.
    * 
    * @param body the body as a byte array.
    * @param header the http body part header associated with the body.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      String body, HttpBodyPartHeader header, boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(body, getHeader(), header, lastBodyPart);
      
      return rval;      
   }
   
   /**
    * Sends an http body part body for this http web response as a byte array.
    * 
    * @param body the body as a string.
    * @param header the http body part header associated with the body.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      byte[] body, HttpBodyPartHeader header, boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(body, getHeader(), header, lastBodyPart);
      
      return rval;      
   }   

   /**
    * Sends an http body part body for this http web response as a byte array.
    * 
    * @param buffer the buffer containing the http body part body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the http body part body in bytes.
    * @param header the http body part header associated with the body.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      byte[] buffer, int offset, int length,
      HttpBodyPartHeader header, boolean lastBodyPart)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(buffer, offset, length, getHeader(),
         header, lastBodyPart);
      
      return rval;      
   }   
   
   /**
    * Sends an http body part body for this http web response. The body is
    * read from the passed input stream.
    *
    * @param is the input stream to read the body from.
    * @param header the http body part header associated with the body.
    * @param lastBodyPart true if this body is for the last body part,
    *                     false if not.
    *                     
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      InputStream is, HttpBodyPartHeader header, boolean lastBodyPart)   
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.sendBodyPartBody(is, getHeader(), header, lastBodyPart);
      
      return rval;      
   }
   
   /**
    * Sends a 100 Continue http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendContinueResponse()
   {
      boolean rval = false;
      
      getHeader().setStatusCode("100 Continue");
      getHeader().setContentLength(0);
      getHeader().setContentEncoding(null);
      getHeader().setConnection("keep-alive");
      
      rval = sendHeader();
      
      return rval;
   }
      
   /**
    * Sends a 202 Accepted (request accepted, but still processing)
    * http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendAcceptedResponse()
   {
      boolean rval = false;
      
      getHeader().setStatusCode("202 Accepted");
      getHeader().setContentLength(0);
      getHeader().setContentEncoding(null);
      getHeader().setConnection("keep-alive");
      
      rval = sendHeader();
      
      return rval;
   }
   
   /**
    * Sends a 400 Bad Request http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendBadRequestResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 400 Bad Request</h2></body></html>";
      
      getHeader().setStatusCode("400 Bad Request");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }   
   
   /**
    * Sends a 403 Forbidden http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendForbiddenResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 403 Forbidden</h2></body></html>";
      
      getHeader().setStatusCode("403 Forbidden");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }   

   /**
    * Sends a 404 Not Found http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendNotFoundResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 404 Not Found</h2></body></html>";
      
      getHeader().setStatusCode("404 Not Found");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }   

   /**
    * Sends a 405 Method Not Allowed http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendMethodNotAllowedResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 405 Method Not Allowed</h2></body></html>";
      
      getHeader().setStatusCode("405 Method Not Allowed");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }   

   /**
    * Sends a 500 Server Error http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendServerErrorResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 500 Server Error</h2></body></html>";
      
      getHeader().setStatusCode("500 Server Error");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }   
   
   /**
    * Sends a 505 Version Not Supported http web response.
    * 
    * @return true if the http web response was successfully sent,
    *         false if not.
    */
   public boolean sendVersionNotSupportedResponse()
   {
      boolean rval = false;
      
      String page =
         "<html><body><h2>HTTP 505 Version Not Supported</h2></body></html>";
      
      getHeader().setStatusCode("505 Version Not Supported");
      getHeader().setContentType("text/html");
      getHeader().setContentEncoding(null);
      getHeader().setContentLength(page.length());
      getHeader().setConnection("close");
      
      if(sendHeader())
      {
         rval = sendBody(page);
      }
      
      return rval;
   }
   
   /**
    * Receives the response header of this http web response.
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
    * Receives the body of this http web response and returns it as
    * a string.
    * 
    * @return the body of this http web response as a string, or null if
    *         the body could not be received.
    */
   public String receiveBodyString()
   {
      String rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyString(getHeader());
      
      return rval;
   }
   
   /**
    * Receives the body of this http web response and returns it as
    * a byte array.
    * 
    * @return the body of this http web response as a byte array, or null if
    *         the body could not be received.
    */
   public byte[] receiveBody()
   {
      byte[] rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBody(getHeader());
      
      return rval;
   }   
      
   /**
    * Receives the body of this http web response and writes it to
    * the passed output stream.
    * 
    * @param os the output stream to write the response body to.
    * 
    * @return true if the body was successfully received, false if not.
    */
   public boolean receiveBody(OutputStream os)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBody(os, getHeader());
      
      return rval;
   }
   
   /**
    * Receives an http body part body as a string.
    *
    * @param header the http body part header associated with the body.
    * 
    * @return the body of the body part as a string, or null if the body
    *         could not be received. 
    */
   public String receiveBodyPartBodyString(HttpBodyPartHeader header)
   {
      String rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBodyString(getHeader(), header);
      
      return rval;
   }
   
   /**
    * Receives an http body part body as a byte array.
    *
    * @param header the http body part header associated with the body.
    * 
    * @return the body of the body part as a byte array, or null if the body
    *         could not be received. 
    */
   public byte[] receiveBodyPartBody(HttpBodyPartHeader header)
   {
      byte[] rval = null;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBody(getHeader(), header);
      
      return rval;
   }
   
   /**
    * Receives an http body part body and writes it to the passed
    * output stream.
    *
    * @param os the output stream to write the body to.
    * @param header the http body part header associated with the body.
    * 
    * @return true if the body was successfully received, false if not.
    */
   public boolean receiveBodyPartBody(
      OutputStream os, HttpBodyPartHeader header)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.receiveBodyPartBody(os, getHeader(), header);
      
      return rval;
   }
   
   /**
    * Skips an http body part body.
    *
    * @param header the http body part header associated with the body.
    * 
    * @return true if the http body part body was successfully skipped, false
    *         if not.
    */
   public boolean skipBodyPartBody(HttpBodyPartHeader header)
   {
      boolean rval = false;
      
      HttpWebConnection hwc = (HttpWebConnection)getWebConnection();
      rval = hwc.skipBodyPartBody(getHeader(), header);
      
      return rval;
   }
   
   /**
    * Returns true if this web response is valid, false if it is not.
    * 
    * @return true if this web response is valid, false if it is not.
    */
   public boolean isValid()
   {
      boolean rval = false;
      
      // see if the header is valid
      rval = getHeader().isValid();
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of 100 Continue,
    * false if not.
    * 
    * @return true if this http web response has a status code of 100 Continue,
    *         false if not.
    */
   public boolean isStatusCodeContinue()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("100"))
      {
         rval = true;
      }
      
      return rval;
   }

   /**
    * Returns true if this http web response has a status code of 200 (OK),
    * false if not.
    * 
    * @return true if this http web response has a status code of 200 OK,
    *         false if not.
    */
   public boolean isStatusCodeOK()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("200"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of 202 Accepted,
    * false if not.
    * 
    * @return true if this http web response has a status code of 202 Accepted,
    *         false if not.
    */
   public boolean isStatusCodeAccepted()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("202"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of
    * 400 Bad Request, false if not.
    * 
    * @return true if this http web response has a status code of
    *         400 Bad Request, false if not.
    */
   public boolean isStatusCodeBadRequest()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("400"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of 403 Forbidden,
    * false if not.
    * 
    * @return true if this http web response has a status code of 403 Forbidden,
    *         false if not.
    */
   public boolean isStatusCodeForbidden()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("403"))
      {
         rval = true;
      }
      
      return rval;
   }   

   /**
    * Returns true if this http web response has a status code of 404 Not Found,
    * false if not.
    * 
    * @return true if this http web response has a status code of 404 Not Found,
    *         false if not.
    */
   public boolean isStatusCodeNotFound()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("404"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of
    * 405 Method Not Allowed, false if not.
    * 
    * @return true if this http web response has a status code of
    *         405 Method Not Allowed, false if not.
    */
   public boolean isStatusCodeMethodNotAllowed()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("405"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of
    * 500 Server Error, false if not.
    * 
    * @return true if this http web response has a status code of
    *         500 Server Error, false if not.
    */
   public boolean isStatusCodeServerError()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("500"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response has a status code of
    * 505 Version Not Supported, false if not.
    * 
    * @return true if this http web response has a status code of
    *         505 Version Not Supported, false if not.
    */
   public boolean isStatusCodeVersionNotSupported()
   {
      boolean rval = false;
      
      if(getHeader().getStatusCode() != null &&
         getHeader().getStatusCode().startsWith("505"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this http web response is multipart, false if not.
    * 
    * @return true if this response is multipart, false if not.
    */
   public boolean isMultipart()
   {
      boolean rval = false;
      
      // see if header is marked as multipart
      rval = getHeader().isMultipart();
      
      return rval;
   }
   
   /**
    * Returns true if this http web response is multipart and it
    * has more parts to read, false if not.
    * 
    * @return true if this http web response is multipart and it has more
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
    * Gets the header for this http web response.
    * 
    * @return the header for this http web response.
    */
   public HttpWebResponseHeader getHeader()
   {
      return mHeader;
   }
   
   /**
    * Gets the http web connection for this response.
    * 
    * @return the http web connection for this response.
    */
   public HttpWebConnection getHttpWebConnection()
   {
      return (HttpWebConnection)getWebConnection();
   }
}
