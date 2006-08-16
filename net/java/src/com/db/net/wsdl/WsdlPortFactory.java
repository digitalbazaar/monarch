/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

/**
 * A WsdlPortFactory creates WsdlPorts of a specific type.
 * 
 * @author Dave Longley
 */
public interface WsdlPortFactory
{
   /**
    * Creates a new WsdlPort of a specific type.
    * 
    * @param wsdl the wsdl the port is associated with.
    * @param binding the binding for this port.
    *
    * @return a new specific type of WsdlPort.
    */
   public abstract WsdlPort createWsdlPort(
      Wsdl wsdl, WsdlBinding binding);
}
