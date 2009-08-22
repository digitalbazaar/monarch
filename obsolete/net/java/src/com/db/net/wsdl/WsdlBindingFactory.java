/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.wsdl;

/**
 * A WsdlBindingFactory creates WsdlBindings of a specific type.
 * 
 * @author Dave Longley
 */
public interface WsdlBindingFactory
{
   /**
    * Creates a new WsdlBinding of a specific type.
    * 
    * @param wsdl the wsdl the binding is associated with.
    * @param portType the port type for the binding.
    *
    * @return a new specific type of WsdlBinding.
    */
   public abstract WsdlBinding createWsdlBinding(
      Wsdl wsdl, WsdlPortType portType);
}
