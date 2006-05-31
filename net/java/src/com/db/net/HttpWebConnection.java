/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Date;
import java.util.HashMap;

import com.db.logging.Logger;
import com.db.stream.BoundaryInputStream;

/**
 * This class represents an http web connection to a web server.
 * 
 * @author Dave Longley
 */
public class HttpWebConnection extends WebConnectionWrapper
{
   /**
    * A mapping of http content encoders for this web connection. 
    */
   protected HashMap mHttpContentEncoders;
   
   /**
    * A mapping of http content decoders for this web connection. 
    */
   protected HashMap mHttpContentDecoders;
   
   /**
    * The last read boundary. This is the last http multipart boundary
    * that was read.
    */
   protected String mLastReadBoundary;
   
   /**
    * Creates a new http web connection.
    * 
    * @param workerSocket the worker socket for this web connection.
    */
   public HttpWebConnection(Socket workerSocket)
   {
      this(new GenericWebConnection(workerSocket));
   }   
   
   /**
    * Creates a new http web connection.
    * 
    * @param wc the web connection to wrap as an http web connection.
    */
   public HttpWebConnection(WebConnection wc)
   {
      super(wc);
      
      // create maps for content encoders/decoders
      mHttpContentEncoders = new HashMap();
      mHttpContentDecoders = new HashMap();
      
      // no last read boundary
      mLastReadBoundary = null;
   }
   
   /**
    * Sets the http content encoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding to use the content
    *                        encoder for.
    * @param hce the http content encoder.
    */
   public void setHttpContentEncoder(String contentEncoding,
                                     HttpContentEncoder hce)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      mHttpContentEncoders.put(contentEncoding, hce);
   }
   
   /**
    * Gets the http content encoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding to get the content
    *                        encoder for.
    * @return the http content encoder or null if there was no encoder
    *         associated with the given content encoding.
    */
   public HttpContentEncoder getHttpContentEncoder(String contentEncoding)
   {
      HttpContentEncoder rval = null;
      
      if(contentEncoding != null)
      {
         // force encoding to lower case
         contentEncoding = contentEncoding.toLowerCase();
         
         // split content-encoding by comma
         String[] types = contentEncoding.split(",");
         for(int i = 0; i < types.length; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get content encoder
            rval = (HttpContentEncoder)mHttpContentEncoders.get(type);
            if(rval != null)
            {
               // content encoder found
               break;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Removes the http content encoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding for the encoder to remove.
    * @return the removed content encoder.
    */
   public HttpContentEncoder removeHttpContentEncoder(String contentEncoding)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // get old encoder
      HttpContentEncoder rval = getHttpContentEncoder(contentEncoding);
      
      // remove content encoder
      mHttpContentEncoders.remove(contentEncoding);
      
      return rval;
   }
   
   /**
    * Sets the http content decoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding to use the content
    *                        decoder for.
    * @param hcd the http content decoder.
    */
   public void setHttpContentDecoder(String contentEncoding,
                                     HttpContentDecoder hcd)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      mHttpContentDecoders.put(contentEncoding, hcd);
   }
   
   /**
    * Gets the http content decoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding to get the content
    *                        decoder for.
    * @return the http content decoder or null if there was no decoder
    *         associated with the given content-encoding.
    */
   public HttpContentDecoder getHttpContentDecoder(String contentEncoding)
   {
      HttpContentDecoder rval = null;
      
      if(contentEncoding != null)
      {
         // force encoding to lower case
         contentEncoding = contentEncoding.toLowerCase();
         
         // split content-encoding by comma
         String[] types = contentEncoding.split(",");
         for(int i = 0; i < types.length; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get content decoder
            rval = (HttpContentDecoder)mHttpContentDecoders.get(type);
            if(rval != null)
            {
               // content decoder found
               break;
            }
         }      
      }
      
      return rval;
   }
   
   /**
    * Removes the http content decoder for the specified content encoding.
    * 
    * @param contentEncoding the content encoding for the decoder to remove.
    * @return the removed content decoder.
    */
   public HttpContentDecoder removeHttpContentDecoder(String contentEncoding)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // get old decoder
      HttpContentDecoder rval = getHttpContentDecoder(contentEncoding);
      
      // remove content decoder
      mHttpContentDecoders.remove(contentEncoding);
      
      return rval;
   }
   
   /**
    * Sends an http header over the underlying web connection.
    * 
    * @param header the http header to send.
    * @return true if the header was successfully sent, false if not.
    */
   public boolean sendHeader(HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         // get header string
         String str = header.toString();
         getLogger().debug("sending http header:\r\n" + str);
         
         // get header in bytes
         byte[] buffer = str.getBytes();
         
         // write to web connection
         getWebConnection().write(buffer, 0, buffer.length);
         rval = true;
         
         getLogger().debug("http header sent.");
      }
      catch(Throwable t)
      {
         getLogger().debug("could not send http header!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part header over the underlying web connection.
    * 
    * @param header the http body part header to send.
    * @param boundary the boundary to use.
    * @return true if the header was successfully sent, false if not.
    */
   public boolean sendBodyPartHeader(HttpBodyPartHeader header, String boundary)
   {
      boolean rval = false;
      
      try
      {
         // get the header string
         String str = header.toString();
         getLogger().debug("sending http body part header:\r\n" + str);
         
         // prepend the boundary
         str = boundary + HttpHeader.CRLF + str;
         
         // get header in bytes
         byte[] buffer = str.getBytes();
         
         // write to web connection
         getWebConnection().write(buffer, 0, buffer.length);
         rval = true;
         
         getLogger().debug("http body part header sent.");
      }
      catch(Throwable t)
      {
         getLogger().debug("could not send http body part header!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));
      }      
      
      return rval;
   }
   
   /**
    * Sends an http body as a string.
    * 
    * @param body the http body as a string.
    * @param contentEncoding the content-encoding for the body.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(String body, String contentEncoding)
   {
      boolean rval = false;
      
      rval = sendBody(body.getBytes(), contentEncoding);
      
      return rval;
   }
   
   /**
    * Sends an http body as a byte array.
    * 
    * @param body the http body as a byte array.
    * @param contentEncoding the content-encoding for the body.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] body, String contentEncoding)   
   {
      return sendBody(body, 0, body.length, contentEncoding);
   }

   /**
    * Sends an http body as a byte array.
    * 
    * @param buffer the buffer containing the http body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the body in bytes.
    * @param contentEncoding the content-encoding for the body.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] buffer, int offset, int length,
                           String contentEncoding)
   {
      boolean rval = false;
      
      try
      {
         // get a byte array input stream
         ByteArrayInputStream bais =
            new ByteArrayInputStream(buffer, offset, length);
         
         // send the body
         rval = sendBody(bais, contentEncoding);

         // close input stream
         bais.close();
      }
      catch(Throwable t)
      {
         getLogger().debug("could not send http body!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
  
   /**
    * Sends an http body. The body is read from the passed input stream.
    * 
    * @param is the input stream to read the http body from.
    * @param contentEncoding the content-encoding for the body. 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(InputStream is, String contentEncoding)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug("sending http body...");
         
         // get content encoder
         HttpContentEncoder hce = getHttpContentEncoder(contentEncoding);
         
         // update input stream if a content encoder is to be used
         if(hce != null)
         {
            // get encoded stream
            is = hce.getHttpContentEncodedStream(contentEncoding, is);
         }
         
         // start time
         long st = new Date().getTime();
         
         // read from the input stream until end of stream
         long totalWritten = 0;
         int numBytes = -1;
         byte[] buffer = new byte[16384];
         while((numBytes = is.read(buffer)) != -1)
         {
            // write to the web connection
            getWebConnection().write(buffer, 0, numBytes);
            totalWritten += numBytes;
         }
         
         // writing complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug("http body (" + totalWritten + " bytes) sent in " +
                           timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug("Could not send http body, " +
                           "an exception occurred!");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part body. The body is read from the passed input
    * stream.
    * 
    * @param is the input stream to read the body from.
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(InputStream is, String contentEncoding,
                                   String boundary, boolean endBoundary)
   {
      boolean rval = false;
      
      // send the body
      if(sendBody(is, contentEncoding))
      {
         // send the boundary
         try
         {
            // see if we are sending the end boundary or not
            if(endBoundary)
            {
               boundary += "--";
            }

            // start time
            long st = new Date().getTime();
   
            // write the boundary
            byte[] buffer = boundary.getBytes();
            getWebConnection().write(buffer, 0, buffer.length);
            
            // writing complete
            rval = true;
            
            // end time
            long et = new Date().getTime();
            
            // calculate transfer time
            long timespan = et - st;
            
            getLogger().debug("http body part body (" + buffer.length +
                              " bytes) sent in " + timespan + " ms.");         
         }
         catch(Throwable t)
         {
            getLogger().debug("Could not send http body part body boundary, " +
                              "an exception occurred!");
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part body as a string.
    * 
    * @param body the body as a string.
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(String body, String contentEncoding,
                                   String boundary, boolean endBoundary)
   {
      boolean rval = false;
      
      rval = sendBodyPartBody(body.getBytes(), contentEncoding,
                              boundary, endBoundary);
      
      return rval;
   }
   
   /**
    * Sends an http body part body as a byte array.
    * 
    * @param body the body as a byte array.
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(byte[] body, String contentEncoding,
                                   String boundary, boolean endBoundary)
   {
      return sendBodyPartBody(body, 0, body.length, contentEncoding,
                              boundary, endBoundary);
   }

   /**
    * Sends an http body part body as a byte array.
    * 
    * @param buffer the buffer containing the http body part body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the http body part body in bytes.
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(byte[] buffer, int offset, int length,
                                   String contentEncoding,
                                   String boundary, boolean endBoundary)
   {
      boolean rval = false;
      
      try
      {
         // get a byte array input stream
         ByteArrayInputStream bais =
            new ByteArrayInputStream(buffer, offset, length);
         
         // send the body part body
         rval = sendBodyPartBody(bais, contentEncoding, boundary, endBoundary);

         // close input stream
         bais.close();
      }
      catch(Throwable t)
      {
         getLogger().debug("could not send http body part body!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http header.
    * 
    * @param header the http header to write to.
    * @return true if the header was successfully received, false if not.
    */
   public boolean receiveHeader(HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         // read one line at a time from the web connection
         StringBuffer sb = new StringBuffer();
         String line = null;
         while((line = getWebConnection().readLine()) != null &&
               !line.equals(""))
         {
            sb.append(line + HttpHeader.CRLF);
         }
         
         String str = sb.toString();
         getLogger().debug("received http header:\r\n" + str + "\r\n");
         
         if(!str.equals(""))
         {
            // parse header
            rval = header.parse(str);
         }
      }
      catch(Throwable t)
      {
         getLogger().debug("could not receive http header!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and returns it as a string.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @return the http body as a string, or null if a body could not
    *         be received.
    */
   public String receiveBodyString(String contentEncoding)
   {
      return receiveBodyString(contentEncoding, -1);
   }
   
   /**
    * Receives an http body and returns it as a string.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @param contentLength the content-length (a negative content-length
    *                      may be passed if no content-length has been
    *                      specified).
    * @return the http body as a string, or null if a body could not
    *         be received.
    */
   public String receiveBodyString(String contentEncoding, long contentLength)
   {
      String rval = null;
      
      byte[] body = receiveBody(contentEncoding, contentLength);
      if(body != null)
      {
         rval = new String(body);
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and returns it as a byte array.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @return the http body as a byte array, or null if a body could not
    *         be received.
    */
   public byte[] receiveBody(String contentEncoding)
   {
      return receiveBody(contentEncoding, -1);
   }
   
   /**
    * Receives an http body and returns it as a byte array.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @param contentLength the content-length (a negative content-length
    *                      may be passed if no content-length has been
    *                      specified).
    * @return the http body as a byte array, or null if a body could not
    *         be received.
    */
   public byte[] receiveBody(String contentEncoding, long contentLength)
   {
      byte[] rval = null;
      
      try
      {
         // get a byte array output stream
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // receive the body and write to the byte array output stream
         if(receiveBody(baos, contentEncoding, contentLength))
         {
            rval = baos.toByteArray();
         }

         // close output stream
         baos.close();
      }
      catch(Throwable t)
      {
         getLogger().debug("could not receive http body!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));         
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and writes it to the passed output stream.
    *
    * @param os the output stream to write the http body to.
    * @param contentEncoding the content-encoding for the body.
    * @return true if the http body was successfully received, false if not.
    */
   public boolean receiveBody(OutputStream os, String contentEncoding)
   {
      return receiveBody(os, contentEncoding, -1);
   }

   /**
    * Receives an http body and writes it to the passed output stream.
    *
    * @param os the output stream to write the http body to.
    * @param contentEncoding the content-encoding for the body.
    * @param contentLength the content-length (a negative content-length
    *                      may be passed if no content-length has been
    *                      specified).
    * @return true if the http body was successfully received, false if not.
    */
   public boolean receiveBody(OutputStream os, String contentEncoding,
                              long contentLength)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug("receiving http body...");
         
         // get input stream for reading from the web connection
         InputStream is = getWebConnection().getReadStream();
         
         // get the content decoder
         HttpContentDecoder hcd = getHttpContentDecoder(contentEncoding);

         // update output stream if a content decoder is to be used
         if(hcd != null)
         {
            // get decoded stream
            is = hcd.getHttpContentDecodedStream(contentEncoding, is);
         }
         
         // start time
         long st = new Date().getTime();
         
         // read from the web connection until content-length reached
         long totalRead = 0;
         int numBytes = -1;
         byte[] buffer = new byte[16384];
         while((contentLength < 0 || totalRead < contentLength) &&
               (numBytes = is.read(buffer, 0, buffer.length)) != -1)
         {
            // write to the output stream
            os.write(buffer, 0, numBytes);
            totalRead += numBytes;
         }
         
         // writing complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug("http body (" + totalRead + " bytes) received in " +
                           timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug("Could not receive http body, " +
                           "an exception occurred!");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and returns it as a string.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @return the http body part body as a string, or null if a body part
    *         body could not be received.
    */
   public String receiveBodyPartBodyString(String contentEncoding,
                                           String boundary)
   {
      String rval = null;
      
      byte[] body = receiveBodyPartBody(contentEncoding, boundary);
      if(body != null)
      {
         rval = new String(body);
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and returns it as a byte array.
    * 
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @return the http body part body as a byte array, or null if a body part
    *         body could not be received.
    */
   public byte[] receiveBodyPartBody(String contentEncoding, String boundary)
   {
      byte[] rval = null;
      
      try
      {
         // get a byte array output stream
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // receive the body part body and write to the output stream
         if(receiveBodyPartBody(baos, contentEncoding, boundary))
         {
            rval = baos.toByteArray();
         }

         // close output stream
         baos.close();
      }
      catch(Throwable t)
      {
         getLogger().debug("could not receive http body part body!, " +
                           "an exception occurred.");
         getLogger().debug(Logger.getStackTrace(t));         
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and writes it to the passed output stream.
    *
    * @param os the output stream to write the http body part body to.
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @return true if the http body part body was successfully received, false
    *         if not.
    */
   public boolean receiveBodyPartBody(OutputStream os, String contentEncoding,
                                      String boundary)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug("receiving http body part body...");         
         
         // start time
         long st = new Date().getTime();
         
         // get input stream for reading from the web connection
         InputStream is = getWebConnection().getReadStream();

         // create a boundary input stream to read from the web connection
         BoundaryInputStream bis = new BoundaryInputStream(is);
         bis.addBoundary(HttpHeader.CRLF + boundary + HttpHeader.CRLF);
         bis.addBoundary(HttpHeader.CRLF + boundary + "--" + HttpHeader.CRLF);
         
         // get the content decoder
         HttpContentDecoder hcd = getHttpContentDecoder(contentEncoding);

         // update output stream if a content decoder is to be used
         if(hcd != null)
         {
            // get decoded stream
            is = hcd.getHttpContentDecodedStream(contentEncoding, bis);
         }
         
         // read from the web connection until boundary reached
         long totalRead = 0;
         int numBytes = -1;
         byte[] buffer = new byte[16384];
         while((numBytes = is.read(buffer, 0, buffer.length)) != -1)
         {
            // write to the output stream
            os.write(buffer, 0, numBytes);
            totalRead += numBytes;
         }
         
         // save last read boundary
         mLastReadBoundary = bis.getReachedBoundary();
         
         // writing complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug("http body part body (" + totalRead +
                           " bytes) received in " + timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug("Could not receive http body part body, " +
                           "an exception occurred!");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Skips an http body part body.
    *
    * @param contentEncoding the content-encoding for the body.
    * @param boundary the boundary to use.
    * @return true if the http body part body was successfully skipped, false
    *         if not.
    */
   public boolean skipBodyPartBody(String contentEncoding, String boundary)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug("skipping http body part body...");         
         
         // start time
         long st = new Date().getTime();
         
         // get input stream for reading from the web connection
         InputStream is = getWebConnection().getReadStream();

         // create a boundary input stream to read from the web connection
         BoundaryInputStream bis = new BoundaryInputStream(is);
         bis.addBoundary(HttpHeader.CRLF + boundary + HttpHeader.CRLF);
         bis.addBoundary(HttpHeader.CRLF + boundary + "--" + HttpHeader.CRLF);
         
         // get the content decoder
         HttpContentDecoder hcd = getHttpContentDecoder(contentEncoding);

         // update output stream if a content decoder is to be used
         if(hcd != null)
         {
            // get decoded stream
            is = hcd.getHttpContentDecodedStream(contentEncoding, bis);
         }
         
         // read from the web connection until boundary reached
         long totalRead = 0;
         int numBytes = -1;
         byte[] buffer = new byte[16384];
         while((numBytes = is.read(buffer, 0, buffer.length)) != -1)
         {
            // do not write out
            totalRead += numBytes;
         }
         
         // save last read boundary
         mLastReadBoundary = bis.getReachedBoundary();
         
         // skipping complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug("http body part body (" + totalRead + " bytes) " +
                           "skipped in " + timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug("Could not skip http body part body, " +
                           "an exception occurred!");
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Gets the last read boundary.
    * 
    * @return the last read boundary or null if one was no reached.
    */
   public String getLastReadBoundary()
   {
      return mLastReadBoundary;
   }
}
