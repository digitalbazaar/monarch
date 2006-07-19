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
   protected HashMap mContentEncoders;
   
   /**
    * A mapping of http content decoders for this web connection. 
    */
   protected HashMap mContentDecoders;
   
   /**
    * A mapping of http transfer encoders for this web connection. 
    */
   protected HashMap mTransferEncoders;
   
   /**
    * A mapping of http transfer decoders for this web connection. 
    */
   protected HashMap mTransferDecoders;
   
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
      mContentEncoders = new HashMap();
      mContentDecoders = new HashMap();
      
      // create maps for transfer encoders/decoders
      mTransferEncoders = new HashMap();
      mTransferDecoders = new HashMap();
      
      // no last read boundary
      mLastReadBoundary = null;
      
      /*
      // add chunked transfer encoder/decoder by default
      ChunkedHttpTransferCoder chunkedCoder = new ChunkedHttpTransferCoder();
      setTransferEncoder(
         chunkedCoder.getSupportedTransferEncoding(), chunkedCoder);
      setTransferDecoder(
         chunkedCoder.getSupportedTransferEncoding(), chunkedCoder);
      */
      
      // Gzip content encoder is NOT added by default because if HTTP/1.0
      // is used then gzipping must occur before setting the header so as to
      // set the appropriate content length. Gzip content encoder can still
      // be added manually on a per-implementation basis.
      
      // add gzip content decoder by default
      GZipHttpContentCoder gzipCoder = new GZipHttpContentCoder();
      setContentDecoder(gzipCoder.getSupportedContentEncoding(), gzipCoder);
   }
   
   /**
    * Sets the http content encoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding to use the content
    *                        encoder for.
    * @param hce the http content encoder.
    */
   public void setContentEncoder(
      String contentEncoding, HttpContentEncoder hce)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // set encoder
      mContentEncoders.put(contentEncoding, hce);
   }
   
   /**
    * Gets the http content encoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding to get the content
    *                        encoder for.
    *                        
    * @return the http content encoder or null if there was no encoder
    *         associated with the given content-encoding.
    */
   public HttpContentEncoder getContentEncoder(String contentEncoding)
   {
      HttpContentEncoder rval = null;
      
      if(contentEncoding != null)
      {
         // force encoding to lower case
         contentEncoding = contentEncoding.toLowerCase();
         
         // split content-encoding by comma
         String[] types = contentEncoding.split(",");
         for(int i = 0; i < types.length && rval == null; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get content encoder
            rval = (HttpContentEncoder)mContentEncoders.get(type);
         }
      }
      
      return rval;
   }
   
   /**
    * Removes the http content encoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding for the encoder to remove.
    * 
    * @return the removed content encoder.
    */
   public HttpContentEncoder removeContentEncoder(
      String contentEncoding)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // get old encoder
      HttpContentEncoder rval = getContentEncoder(contentEncoding);
      
      // remove content encoder
      mContentEncoders.remove(contentEncoding);
      
      return rval;
   }
   
   /**
    * Sets the http content decoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding to use the content
    *                        decoder for.
    * @param hcd the http content decoder.
    */
   public void setContentDecoder(
      String contentEncoding, HttpContentDecoder hcd)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // set decoder
      mContentDecoders.put(contentEncoding, hcd);
   }
   
   /**
    * Gets the http content decoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding to get the content
    *                        decoder for.
    *                        
    * @return the http content decoder or null if there was no decoder
    *         associated with the given content-encoding.
    */
   public HttpContentDecoder getContentDecoder(String contentEncoding)
   {
      HttpContentDecoder rval = null;
      
      if(contentEncoding != null)
      {
         // force encoding to lower case
         contentEncoding = contentEncoding.toLowerCase();
         
         // split content-encoding by comma
         String[] types = contentEncoding.split(",");
         for(int i = 0; i < types.length && rval == null; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get content decoder
            rval = (HttpContentDecoder)mContentDecoders.get(type);
         }      
      }
      
      return rval;
   }
   
   /**
    * Removes the http content decoder for the specified content-encoding.
    * 
    * @param contentEncoding the content-encoding for the decoder to remove.
    * 
    * @return the removed content decoder.
    */
   public HttpContentDecoder removeContentDecoder(String contentEncoding)
   {
      // force encoding to lower case
      contentEncoding = contentEncoding.toLowerCase();
      
      // get old decoder
      HttpContentDecoder rval = getContentDecoder(contentEncoding);
      
      // remove content decoder
      mContentDecoders.remove(contentEncoding);
      
      return rval;
   }
   
   /**
    * Sets the http transfer encoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding to use the transfer
    *                         encoder for.
    * @param hte the http transfer encoder.
    */
   public void setTransferEncoder(
      String transferEncoding, HttpTransferEncoder hte)
   {
      // force encoding to lower case
      transferEncoding = transferEncoding.toLowerCase();
      
      // set encoder
      mTransferEncoders.put(transferEncoding, hte);
   }
   
   /**
    * Gets the http transfer encoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding to get the transfer
    *                         encoder for.
    *                        
    * @return the http transfer encoder or null if there was no encoder
    *         associated with the given transfer-encoding.
    */
   public HttpTransferEncoder getTransferEncoder(String transferEncoding)
   {
      HttpTransferEncoder rval = null;
      
      if(transferEncoding != null)
      {
         // force encoding to lower case
         transferEncoding = transferEncoding.toLowerCase();
         
         // split transfer-encoding by semi-colon
         String[] types = transferEncoding.split(";");
         for(int i = 0; i < types.length && rval == null; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get transfer encoder
            rval = (HttpTransferEncoder)mTransferEncoders.get(type);
         }
      }
      
      return rval;
   }
   
   /**
    * Removes the http transfer encoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding for the encoder to remove.
    * 
    * @return the removed transfer encoder.
    */
   public HttpTransferEncoder removeTransferEncoder(String transferEncoding)
   {
      // force encoding to lower case
      transferEncoding = transferEncoding.toLowerCase();
      
      // get old encoder
      HttpTransferEncoder rval = getTransferEncoder(transferEncoding);
      
      // remove transfer encoder
      mTransferEncoders.remove(transferEncoding);
      
      return rval;
   }
   
   /**
    * Sets the http transfer decoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding to use the transfer
    *                         decoder for.
    * @param htd the http transfer decoder.
    */
   public void setTransferDecoder(
      String transferEncoding, HttpTransferDecoder htd)
   {
      // force encoding to lower case
      transferEncoding = transferEncoding.toLowerCase();
      
      // set decoder
      mTransferDecoders.put(transferEncoding, htd);
   }
   
   /**
    * Gets the http transfer decoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding to get the transfer
    *                         decoder for.
    *                        
    * @return the http transfer decoder or null if there was no decoder
    *         associated with the given transfer-encoding.
    */
   public HttpTransferDecoder getTransferDecoder(String transferEncoding)
   {
      HttpTransferDecoder rval = null;
      
      if(transferEncoding != null)
      {
         // force encoding to lower case
         transferEncoding = transferEncoding.toLowerCase();
         
         // split transfer-encoding by semi-colon
         String[] types = transferEncoding.split(";");
         for(int i = 0; i < types.length && rval == null; i++)
         {
            // trim any whitespace off the type
            String type = types[i].trim();
            
            // get transfer decoder
            rval = (HttpTransferDecoder)mTransferDecoders.get(type);
         }      
      }
      
      return rval;
   }
   
   /**
    * Removes the http transfer decoder for the specified transfer-encoding.
    * 
    * @param transferEncoding the transfer-encoding for the decoder to remove.
    * 
    * @return the removed transfer decoder.
    */
   public HttpTransferDecoder removeTransferDecoder(String transferEncoding)
   {
      // force encoding to lower case
      transferEncoding = transferEncoding.toLowerCase();
      
      // get old decoder
      HttpTransferDecoder rval = getTransferDecoder(transferEncoding);
      
      // remove transfer decoder
      mTransferDecoders.remove(transferEncoding);
      
      return rval;
   }   
   
   /**
    * Sends an http header over the underlying web connection.
    * 
    * @param header the http header to send.
    * 
    * @return true if the header was successfully sent, false if not.
    */
   public boolean sendHeader(HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         // get header string
         String str = header.toString();
         getLogger().debug(getClass(), "sending http header:\r\n" + str);
         
         // get header in bytes
         byte[] buffer = str.getBytes();
         
         // write to web connection
         getWebConnection().write(buffer, 0, buffer.length);
         rval = true;
         
         getLogger().debug(getClass(), "http header sent.");
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not send http header!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part header over the underlying web connection.
    * 
    * @param header the http body part header to send.
    * @param boundary the boundary to use.
    * 
    * @return true if the header was successfully sent, false if not.
    */
   public boolean sendBodyPartHeader(HttpBodyPartHeader header, String boundary)
   {
      boolean rval = false;
      
      try
      {
         // get the header string
         String str = header.toString();
         getLogger().debug(getClass(),
            "sending http body part header:\r\n" + str);
         
         // prepend the boundary
         str = boundary + HttpHeader.CRLF + str;
         
         // get header in bytes
         byte[] buffer = str.getBytes();
         
         // write to web connection
         getWebConnection().write(buffer, 0, buffer.length);
         rval = true;
         
         getLogger().debug(getClass(), "http body part header sent.");
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not send http body part header!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }      
      
      return rval;
   }
   
   /**
    * Sends an http body as a string.
    * 
    * @param body the http body as a string.
    * @param header the http header associated with the body.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(String body, HttpHeader header)
   {
      boolean rval = false;
      
      rval = sendBody(body.getBytes(), header);
      
      return rval;
   }
   
   /**
    * Sends an http body as a byte array.
    * 
    * @param body the http body as a byte array.
    * @param header the http header associated with the body.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(byte[] body, HttpHeader header)   
   {
      return sendBody(body, 0, body.length, header);
   }

   /**
    * Sends an http body as a byte array.
    * 
    * @param buffer the buffer containing the http body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the body in bytes.
    * @param header the http header associated with the body.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(
      byte[] buffer, int offset, int length, HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         // get a byte array input stream
         ByteArrayInputStream bais =
            new ByteArrayInputStream(buffer, offset, length);
         
         // send the body
         rval = sendBody(bais, header);

         // close input stream
         bais.close();
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), 
            "could not send http body!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
  
   /**
    * Sends an http body. The body is read from the passed input stream.
    * 
    * @param is the input stream to read the http body from.
    * @param header the http header associated with the body.
    * 
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBody(InputStream is, HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug(getClass(), "sending http body...");
         
         // get the content-encoding
         String contentEncoding = header.getContentEncoding();
         
         // get content encoder
         HttpContentEncoder hce = getContentEncoder(contentEncoding);
         
         // update input stream if a content encoder is to be used
         if(hce != null)
         {
            // get encoded stream
            is = hce.getHttpContentEncodedStream(contentEncoding, is);
         }
         
         // get the transfer-encoding
         String transferEncoding = header.getTransferEncoding();
         
         // get transfer encoder
         HttpTransferEncoder hte = getTransferEncoder(transferEncoding);
         
         // start time
         long st = new Date().getTime();
         long totalWritten = 0;

         // use transfer encoder if one is available
         if(hte != null)
         {
            totalWritten = hte.writeHttpBody(header, is, this);
         }
         else
         {
            // read from the input stream until end of stream
            int numBytes = -1;
            byte[] buffer = new byte[16384];
            while((numBytes = is.read(buffer)) != -1)
            {
               // write to the web connection
               getWebConnection().write(buffer, 0, numBytes);
               totalWritten += numBytes;
            }
         }
         
         // writing complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug(getClass(),
            "http body (" + totalWritten + " bytes) sent in " +
            timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "Could not send http body!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part body. The body is read from the passed input
    * stream.
    * 
    * @param is the input stream to read the body from.
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    *                    
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      InputStream is, HttpHeader parentHeader,
      HttpBodyPartHeader bodyPartHeader, boolean endBoundary)
   {
      boolean rval = false;
      
      // send the body
      if(sendBody(is, bodyPartHeader))
      {
         // send the boundary
         try
         {
            // get the boundary
            String boundary = parentHeader.getBoundary();
            if(endBoundary)
            {
              boundary = parentHeader.getEndBoundary(); 
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
            
            getLogger().debug(getClass(),
               "http body part body (" + buffer.length +
               " bytes) sent in " + timespan + " ms.");         
         }
         catch(Throwable t)
         {
            getLogger().debug(getClass(),
               "Could not send http body part body boundary, " +
               "an exception occurred!,\ntrace= " + Logger.getStackTrace(t));
         }
      }
      
      return rval;
   }
   
   /**
    * Sends an http body part body as a string.
    * 
    * @param body the body as a string.
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    *                    
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      String body, HttpHeader parentHeader,
      HttpBodyPartHeader bodyPartHeader, boolean endBoundary)
   {
      boolean rval = false;
      
      rval = sendBodyPartBody(body.getBytes(), parentHeader,
         bodyPartHeader, endBoundary);
      
      return rval;
   }
   
   /**
    * Sends an http body part body as a byte array.
    * 
    * @param body the body as a byte array.
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    *                    
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      byte[] body, HttpHeader parentHeader,
      HttpBodyPartHeader bodyPartHeader, boolean endBoundary)
   {
      return sendBodyPartBody(body, 0, body.length,
         parentHeader, bodyPartHeader, endBoundary);
   }

   /**
    * Sends an http body part body as a byte array.
    * 
    * @param buffer the buffer containing the http body part body.
    * @param offset the offset the body begins at in the buffer.
    * @param length the length of the http body part body in bytes.
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * @param endBoundary true if the end boundary should be used after this
    *                    part, false if not.
    *                    
    * @return true if the body was successfully sent, false if not.
    */
   public boolean sendBodyPartBody(
      byte[] buffer, int offset, int length, HttpHeader parentHeader,
      HttpBodyPartHeader bodyPartHeader, boolean endBoundary)
   {
      boolean rval = false;
      
      try
      {
         // get a byte array input stream
         ByteArrayInputStream bais =
            new ByteArrayInputStream(buffer, offset, length);
         
         // send the body part body
         rval = sendBodyPartBody(bais, parentHeader,
            bodyPartHeader, endBoundary);

         // close input stream
         bais.close();
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not send http body part body!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http header.
    * 
    * @param header the http header to write to.
    * 
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
         getLogger().debug(getClass(),
            "received http header:\r\n" + str + "\r\n");
         
         if(!str.equals(""))
         {
            // parse header
            rval = header.parse(str);
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not receive http header!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and returns it as a string.
    * 
    * @param header the http header associated with the body.
    *                      
    * @return the http body as a string, or null if a body could not
    *         be received.
    */
   public String receiveBodyString(HttpHeader header)
   {
      String rval = null;
      
      byte[] body = receiveBody(header);
      if(body != null)
      {
         rval = new String(body);
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and returns it as a byte array.
    * 
    * @param header the http header associated with the body.
    * 
    * @return the http body as a byte array, or null if a body could not
    *         be received.
    */
   public byte[] receiveBody(HttpHeader header)
   {
      byte[] rval = null;
      
      try
      {
         // get a byte array output stream
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // receive the body and write to the byte array output stream
         if(receiveBody(baos, header))
         {
            rval = baos.toByteArray();
         }

         // close output stream
         baos.close();
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not receive http body!, " +
            "an exception occurred,trace= " + Logger.getStackTrace(t));         
      }
      
      return rval;
   }
   
   /**
    * Receives an http body and writes it to the passed output stream.
    *
    * @param os the output stream to write the http body to.
    * @param header the http header associated with the body.
    * 
    * @return true if the http body was successfully received, false if not.
    */
   public boolean receiveBody(OutputStream os, HttpHeader header)
   {
      boolean rval = false;
      
      try
      {
         getLogger().debug(getClass(), "receiving http body...");
         
         // get the content-encoding
         String contentEncoding = header.getContentEncoding();
         
         // get content decoder
         HttpContentDecoder hcd = getContentDecoder(contentEncoding);
         
         // update output stream if a content decoder is to be used
         if(hcd != null)
         {
            // get decoded stream
            os = hcd.getHttpContentDecodedStream(contentEncoding, os);
         }
         
         // get the transfer-encoding
         String transferEncoding = header.getTransferEncoding();
         
         // get transfer decoder
         HttpTransferDecoder htd = getTransferDecoder(transferEncoding);
         
         // start time
         long st = new Date().getTime();
         long totalRead = 0;

         // use transfer decoder if one is available
         if(htd != null)
         {
            totalRead = htd.readHttpBody(header, this, os);
         }
         else
         {
            // get the content length
            long contentLength = header.getContentLength();
            
            // read from the web connection until content-length reached
            int numBytes = -1;
            byte[] buffer = new byte[16384];
            while((contentLength < 0 || totalRead < contentLength) &&
                  (numBytes = getWebConnection().read(
                     buffer, 0, buffer.length)) != -1)
            {
               // write to the output stream
               os.write(buffer, 0, numBytes);
               totalRead += numBytes;
            }
         }
         
         // reading complete
         rval = true;
         
         // end time
         long et = new Date().getTime();
         
         // calculate transfer time
         long timespan = et - st;
         
         getLogger().debug(getClass(),
            "http body (" + totalRead + " bytes) received in " +
            timespan + " ms.");         
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(),
            "could not receive http body!, " +
            "an exception occurred!,\ntrace= " + Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and returns it as a string.
    * 
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * 
    * @return the http body part body as a string, or null if a body part
    *         body could not be received.
    */
   public String receiveBodyPartBodyString(
      HttpHeader parentHeader, HttpBodyPartHeader bodyPartHeader)
   {
      String rval = null;
      
      byte[] body = receiveBodyPartBody(parentHeader, bodyPartHeader);
      if(body != null)
      {
         rval = new String(body);
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and returns it as a byte array.
    *
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * 
    * @return the http body part body as a byte array, or null if a body part
    *         body could not be received.
    */
   public byte[] receiveBodyPartBody(
      HttpHeader parentHeader, HttpBodyPartHeader bodyPartHeader)
   {
      byte[] rval = null;
      
      try
      {
         // get a byte array output stream
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // receive the body part body and write to the output stream
         if(receiveBodyPartBody(baos, parentHeader, bodyPartHeader))
         {
            rval = baos.toByteArray();
         }

         // close output stream
         baos.close();
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), 
            "could not receive http body part body!, " +
            "an exception occurred,\ntrace= " + Logger.getStackTrace(t));         
      }
      
      return rval;
   }
   
   /**
    * Receives an http body part body and writes it to the passed output stream.
    *
    * @param os the output stream to write the http body part body to.
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * 
    * @return true if the http body part body was successfully received, false
    *         if not.
    */
   public boolean receiveBodyPartBody(
      OutputStream os, HttpHeader parentHeader,
      HttpBodyPartHeader bodyPartHeader)
   {
      boolean rval = false;
      
      try
      {
         if(os != null)
         {
            getLogger().debug(getClass(), "receiving http body part body...");         
         }
         else
         {
            getLogger().debug(getClass(), "skipping http body part body...");
         }
         
         // get the content-encoding
         String contentEncoding = bodyPartHeader.getContentEncoding();
         
         // get content decoder
         HttpContentDecoder hcd = getContentDecoder(contentEncoding);
         
         // update output stream if a content decoder is to be used
         if(hcd != null && os != null)
         {
            // get decoded stream
            os = hcd.getHttpContentDecodedStream(contentEncoding, os);
         }
         
         // get the transfer-encoding
         String transferEncoding = bodyPartHeader.getTransferEncoding();
         
         // get transfer decoder
         HttpTransferDecoder htd = getTransferDecoder(transferEncoding);
         
         // start time
         long st = new Date().getTime();
         long totalRead = 0;

         // use transfer decoder if one is available
         if(htd != null)
         {
            totalRead = htd.readHttpBody(bodyPartHeader, this, os);
         }
         
         // get input stream for reading from the web connection
         InputStream is = getWebConnection().getReadStream();
         
         // get the boundary
         String boundary = parentHeader.getBoundary();

         // create a boundary input stream to read from the web connection
         BoundaryInputStream bis = new BoundaryInputStream(is);
         bis.addBoundary(HttpHeader.CRLF + boundary + HttpHeader.CRLF);
         bis.addBoundary(HttpHeader.CRLF + boundary + "--" + HttpHeader.CRLF);
         
         // read from the web connection until the boundary is reached
         int numBytes = -1;
         byte[] buffer = new byte[16384];
         while((numBytes = bis.read(buffer, 0, buffer.length)) != -1)
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
         
         if(os != null)
         {
            getLogger().debug(getClass(),
               "http body part body (" + totalRead +
               " bytes) received in " + timespan + " ms.");         
         }
         else
         {
            getLogger().debug(getClass(),
               "http body part body (" + totalRead + " bytes) " +
               "skipped in " + timespan + " ms.");         
         }
      }
      catch(Throwable t)
      {
         if(os != null)
         {
            getLogger().debug(getClass(),
               "could not receive http body part body!, " +
               "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
         }
         else
         {
            getLogger().debug(getClass(),
               "could not skip http body part body!, " +
               "an exception occurred,\ntrace= " + Logger.getStackTrace(t));
         }
      }
      
      return rval;
   }
   
   /**
    * Skips an http body part body.
    *
    * @param parentHeader the parent http header associated with the body.
    * @param bodyPartHeader the http body part header associated with the body.
    * 
    * @return true if the http body part body was successfully skipped, false
    *         if not.
    */
   public boolean skipBodyPartBody(
      HttpHeader parentHeader, HttpBodyPartHeader bodyPartHeader)
   {
      return receiveBodyPartBody(null, parentHeader, bodyPartHeader);
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
