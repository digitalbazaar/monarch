/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

/**
 * A RpcSoapMessage is a class that contains a RpcSoapEnvelope that was or is
 * to be transported using SOAP (Simple Object Access Protocol).
 * 
 * This class represents either a SOAP request or a SOAP response.
 *  
 * @author Dave Longley
 */
public class RpcSoapMessage extends SoapMessage
{
   /**
    * The soap action to include with this message, if any.
    */
   protected String mSoapAction;   
   
   /**
    * Creates a new RpcSoapMessage with an empty RpcSoapEnvelope and no
    * set remote IP address.
    */
   public RpcSoapMessage()
   {
      // use empty envelope
      this(new RpcSoapEnvelope());
   }
   
   /**
    * Creates a new RpcSoapMessage with the specified RpcSoapEnvelope and no
    * set remote IP address.
    * 
    * @param envelope the RpcSoapEnvelope for this message.
    */
   public RpcSoapMessage(RpcSoapEnvelope envelope)
   {
      // set default blank remote IP address
      this(envelope, "0.0.0.0");
   }
   
   /**
    * Creates a new RpcSoapMessage with the specified RpcSoapEnvelope and remote
    * IP address.
    * 
    * @param envelope the RpcSoapEnvelope for this message.
    * @param remoteIP the remote IP address.
    */
   public RpcSoapMessage(RpcSoapEnvelope envelope, String remoteIP)
   {
      super(envelope, remoteIP);
      
      // default the soap action to a blank string
      setSoapAction("");
   }
   
   /**
    * Gets the RpcSoapEnvelope for this RpcSoapMessage.
    * 
    * @return the RpcSoapEnvelope for this RpcSoapMessage.
    */
   public RpcSoapEnvelope getRpcSoapEnvelope()
   {
      return (RpcSoapEnvelope)mSoapEnvelope;
   }
   
   /**
    * Sets the soap action associated with this soap message.
    * 
    * @param action the soap action associated with this soap message.
    */
   public void setSoapAction(String action)
   {
      if(action == null)
      {
         action = "";
      }
      
      mSoapAction = action;
   }

   /**
    * Gets the soap action associated with this soap message.
    * 
    * @return the soap action associated with this soap message.
    */
   public String getSoapAction()
   {
      return mSoapAction;
   }   
}
