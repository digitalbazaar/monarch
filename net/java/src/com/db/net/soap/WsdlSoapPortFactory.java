/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlBinding;
import com.db.net.wsdl.WsdlPort;
import com.db.net.wsdl.WsdlPortFactory;

/**
 * A WsdlSoapPortFactory creates WsdlSoapPorts.
 * 
 * @author Dave Longley
 */
public class WsdlSoapPortFactory implements WsdlPortFactory
{
   /**
    * Creates a new WsdlSoapPortFactory.
    */
   public WsdlSoapPortFactory()
   {
   }
   
   /**
    * Creates a new WsdlSoapPort.
    * 
    * @param wsdl the wsdl the port is associated with.
    * @param binding the binding for this port.
    *
    * @return a new WsdlSoapPort.
    */
   public WsdlPort createWsdlPort(Wsdl wsdl, WsdlBinding binding)
   {
      WsdlPort rval = null;
      
      rval = new WsdlSoapPort(
         binding, binding.getPortType().getName() + "Port");
      
      return rval;
   }
}
