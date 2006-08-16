/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import com.db.net.wsdl.Wsdl;
import com.db.net.wsdl.WsdlBinding;
import com.db.net.wsdl.WsdlBindingFactory;
import com.db.net.wsdl.WsdlPortType;

/**
 * A WsdlSoapBindingFactory creates WsdlSoapBindings.
 * 
 * @author Dave Longley
 */
public class WsdlSoapBindingFactory implements WsdlBindingFactory
{
   /**
    * Creates a new WsdlSoapBindingFactory.
    */
   public WsdlSoapBindingFactory()
   {
   }
   
   /**
    * Creates a new WsdlSoapBinding.
    * 
    * @param wsdl the wsdl the binding is associated with.
    * @param portType the port type for the binding.
    *
    * @return a new WsdlSoapBinding.
    */
   public WsdlBinding createWsdlBinding(Wsdl wsdl, WsdlPortType portType)
   {
      WsdlBinding rval = null;
      
      rval = new WsdlSoapBinding(
         wsdl, portType.getName() + "SoapBinding", portType);
      
      return rval;
   }
}
