/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

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
    * @return the return value from the called method.
    */
   public Object callSoapMethod(SoapMessage sm);
   
   /**
    * Returns true if the passed soap action is valid for this service.
    *
    * @param action the soap action to check.
    * @return true if the passed soap action is valid, false if not.
    */
   public boolean isSoapActionValid(String action);
   
   /**
    * Gets the URI to the soap service.
    * 
    * @return the URI to the soap service.
    */
   public String getURI();   

   /**
    * Gets the WSDL as a string.
    * 
    * @return the WSDL as a string.
    */
   public String getWsdl();
   
   /**
    * Gets the path to the WSDL.
    * 
    * @return the WSDL path.
    */
   public String getWsdlPath();
}
