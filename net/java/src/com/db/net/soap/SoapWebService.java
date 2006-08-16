/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.net.wsdl.Wsdl;

/**
 * Handles incoming SOAP requests.
 * 
 * @author Dave Longley
 */
public interface SoapWebService
{
   /**
    * Calls the appropriate soap method.
    * 
    * @param sm the soap message.
    * 
    * @return the return value from the called method.
    */
   public Object callSoapMethod(SoapMessage sm);
   
   /**
    * Returns true if the passed soap action is valid for this service.
    *
    * @param action the soap action to check.
    * 
    * @return true if the passed soap action is valid, false if not.
    */
   public boolean isSoapActionValid(String action);
   
   /**
    * Creates a soap message for use with this service.
    * 
    * @return a soap message for use with this service.
    */
   public SoapMessage createSoapMessage();
   
   /**
    * Gets the URI to the soap service.
    * 
    * @return the URI to the soap service.
    */
   public String getURI();   

   /**
    * Gets the WSDL.
    * 
    * @return the WSDL.
    */
   public Wsdl getWsdl();
   
   /**
    * Gets the WSDL port type.
    * 
    * @return the WSDL port type.
    */
   public String getPortType();
   
   /**
    * Gets the path to the WSDL.
    * 
    * @return the WSDL path.
    */
   public String getWsdlPath();
}
