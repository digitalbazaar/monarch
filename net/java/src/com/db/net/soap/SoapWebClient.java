/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.net.WebConnection;
import com.db.net.WebConnectionClient;
import com.db.net.wsdl.Wsdl;

/**
 * A SoapWebClient is a client that communicates over WebConnections using SOAP
 * (Simple Object Access Protocol).
 * 
 * @author Dave Longley
 */
public interface SoapWebClient extends WebConnectionClient
{
   /**
    * Creates a soap request for this soap web client.
    * 
    * @param wsdl the wsdl for the soap web service.
    * @param method the name of the soap method to call.
    * @param params the parameters for the soap method.
    * 
    * @return a soap message for this soap web client.
    */
   public SoapMessage createSoapRequest(
      Wsdl wsdl, String method, Object[] params);
   
   /**
    * Calls a remote soap method over the passed web connection using
    * the passed soap request message.
    *
    * @param wc the web connection to call the soap method over.
    * @param sm the soap request message for the method.
    * 
    * @return the return value from the soap method.
    * 
    * @exception SoapFaultException thrown when a soap fault is raised.
    */
   public Object callSoapMethod(WebConnection wc, SoapMessage sm)
   throws SoapFaultException;
   
   /**
    * Calls a remote soap method. A connection is established to the
    * appropriate soap web service, used to execute the remote method, and
    * then disconnected.
    * 
    * @param method the name of the remote method.
    * @param params the parameters for the remote method.
    * 
    * @return the return value from the remote soap method or null.
    * 
    * @exception SoapFaultException thrown when a SOAP fault is raised.
    */
   public Object callSoapMethod(String method, Object[] params)
   throws SoapFaultException;
}
