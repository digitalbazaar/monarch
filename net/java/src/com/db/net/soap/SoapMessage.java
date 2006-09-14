/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.io.InputStream;
import java.io.OutputStream;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.http.HttpBodyPartHeader;
import com.db.net.http.HttpHeader;
import com.db.net.http.HttpWebConnection;

/**
 * A SoapMessage is a class that contains a SoapEnvelope that was or is
 * to be transported using SOAP (Simple Object Access Protocol). This message
 * is also capable of storing the IP address of the remote entity that sent
 * the SoapEnvelope, if the SoapEnvelope is being received and not sent.
 * 
 * This class can also be used to send or receive attachments after the
 * SoapEnvelope. If a SOAP message was sent in a multipart HTTP message,
 * then attachments can be sent/received along with the message as separate
 * parts.
 * 
 * @author Dave Longley
 */
public class SoapMessage
{
   /**
    * The SoapEnvelope that was/is to be transmitted.
    */
   protected SoapEnvelope mSoapEnvelope;
   
   /**
    * The remote IP address of the entity that sent the SoapOperation.
    */
   protected String mRemoteIP;
   
   /**
    * The http header for this SoapMessage, if any. Used when sending
    * or receiving attachments.
    */
   protected HttpHeader mHttpHeader;
   
   /**
    * An http web connection for reading or writing attachments.
    */
   protected HttpWebConnection mAttachmentWebConnection;
   
   /**
    * True if logging of soap envelope information is permitted, false if not.
    */
   protected boolean mSoapEnvelopeLoggingPermitted;
   
   /**
    * Creates a new SoapMessage with an empty SoapEnvelope and no set
    * remote IP address.
    */
   public SoapMessage()   
   {
      this(new SoapEnvelope());
   }
   
   /**
    * Creates a new SoapMessage with the specified SoapEnvelope and no set
    * remote IP address.
    * 
    * @param envelope the SoapEnvelope for this message.
    */
   public SoapMessage(SoapEnvelope envelope)
   {
      // set default blank remote IP address
      this(envelope, "0.0.0.0");
   }
   
   /**
    * Creates a new SoapMessage with the specified SoapEnvelope and remote
    * IP address.
    * 
    * @param envelope the SoapEnvelope for this message.
    * @param remoteIP the remote IP address.
    */
   public SoapMessage(SoapEnvelope envelope, String remoteIP)
   {
      // store the soap envelope
      mSoapEnvelope = envelope;
      
      // store remote IP
      setRemoteIP(remoteIP);
      
      // soap envelope logging permitted by default
      setSoapEnvelopeLoggingPermitted(true);
   }
   
   /**
    * Gets the SoapEnvelope for this SoapMessage.
    * 
    * @return the SoapEnvelope for this SoapMessage.
    */
   public SoapEnvelope getSoapEnvelope()
   {
      return mSoapEnvelope;
   }
   
   /**
    * Sets the remote IP.
    * 
    * @param remoteIP the remote IP.
    */
   public void setRemoteIP(String remoteIP)
   {
      mRemoteIP = remoteIP;
   }
   
   /**
    * Gets the remote IP.
    * 
    * @return the remote IP.
    */
   public String getRemoteIP()
   {
      return mRemoteIP;
   }
   
   /**
    * Sets the http header for this soap message.
    * 
    * @param header the http header for this soap message.
    */
   public void setHttpHeader(HttpHeader header)
   {
      mHttpHeader = header;
   }
   
   /**
    * Gets the http header for this soap message.
    * 
    * @return the http header for this soap message.
    */
   public HttpHeader getHttpHeader()
   {
      return mHttpHeader;
   }
   
   /**
    * Sets the http web connection to read or write soap attachments with.
    * 
    * @param hwc the http web connection to read or write soap attachments with.
    */
   public void setAttachmentWebConnection(HttpWebConnection hwc)
   {
      mAttachmentWebConnection = hwc;
   }
   
   /**
    * Gets the http web connection to read or write soap attachments with.
    * 
    * @return the http web connection to read or write soap attachments with.
    */
   public HttpWebConnection getAttachmentWebConnection()
   {
      return mAttachmentWebConnection;
   }
   
   /**
    * Sends an attachment for this soap message.
    * 
    * @param header the http body part header for the attachment.
    * @param is the input stream to read the attachment from.
    * @param lastAttachment true if this is the last attachment, false if not.
    * 
    * @return true if the attachment was written, false if not.
    */
   public boolean sendAttachment(
      HttpBodyPartHeader header, InputStream is, boolean lastAttachment)
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null)
      {
         // send the header
         if(hwc.sendHeader(header))
         {
            // send the body
            rval = hwc.sendBodyPartBody(is, getHttpHeader(),
               header, lastAttachment);
         }
      }
      
      return rval;
   }   
   
   /**
    * Receives an attachment for this soap message if there is one
    * to be received.
    * 
    * @param header the http body part header for the attachment.
    * @param os the output stream to write the attachment to.
    * 
    * @return true if an attachment was received, false if not. 
    */
   public boolean receiveAttachment(HttpBodyPartHeader header, OutputStream os)
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null && hasMoreAttachments())
      {
         // receive the header
         if(hwc.receiveHeader(header))
         {
            // receive the body
            rval = hwc.receiveBodyPartBody(os, getHttpHeader(), header);
         }
      }
      
      return rval;
   }
   
   /**
    * Returns true if there are more attachments to be received, false if not.
    * 
    * @return true if there are more attachments to be received, false if not.
    */
   public boolean hasMoreAttachments()
   {
      boolean rval = false;
      
      // get attachment connection
      HttpWebConnection hwc = getAttachmentWebConnection();
      if(hwc != null)
      {
         String endBoundary = getHttpHeader().getEndBoundary();
         
         // see if there is a last read boundary or if the last read
         // boundary is not the end boundary
         if(hwc.getLastReadBoundary() == null ||
            !hwc.getLastReadBoundary().equals(endBoundary))
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Sets whether or not logging of soap envelope information is permitted.
    * 
    * @param allow true to allow logging soap envelope information,
    *              false not to.
    */
   public void setSoapEnvelopeLoggingPermitted(boolean allow)
   {
      mSoapEnvelopeLoggingPermitted = allow;
   }
   
   /**
    * Gets whether or not logging of soap envelope information is permitted.
    * 
    * @return true if logging of soap envelope information is permitted,
    *         false if not.
    */
   public boolean isSoapEnvelopeLoggingPermitted()
   {
      return mSoapEnvelopeLoggingPermitted;
   }
   
   /**
    * Gets the logger for this SoapMessage.
    * 
    * @return the logger for this SoapMessage.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
