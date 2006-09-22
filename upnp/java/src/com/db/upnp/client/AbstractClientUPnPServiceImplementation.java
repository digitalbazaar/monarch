/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client;

import java.util.Iterator;

import com.db.net.soap.RpcSoapEnvelope;
import com.db.net.soap.SoapOperation;
import com.db.net.soap.SoapOperationParameter;
import com.db.upnp.service.UPnPErrorException;
import com.db.upnp.service.UPnPService;
import com.db.upnp.service.UPnPServiceAction;
import com.db.upnp.service.UPnPServiceActionArgument;
import com.db.upnp.service.UPnPServiceStateVariable;
import com.db.util.BoxingHashMap;

/**
 * An AbstractClientUPnPServiceImplementation provides the basic implementation
 * for a ClientUPnPServiceImplementation.
 * 
 * @author Dave Longley
 */
public class AbstractClientUPnPServiceImplementation
implements ClientUPnPServiceImplementation
{
   /**
    * The UPnPService this client implementation is for.
    */
   protected UPnPService mService;
   
   /**
    * The UPnPServiceClient that is used to communicate with the UPnPService.
    */
   protected UPnPServiceClient mServiceClient;
   
   /**
    * Creates a new AbstractClientUPnPServiceImplementation for the specified
    * UPnPService.
    * 
    * @param service the UPnPService this client implementation is for.
    */
   public AbstractClientUPnPServiceImplementation(UPnPService service)
   {
      // store service
      mService = service;
      
      // no service client set yet
      setServiceClient(null);
   }
   
   /**
    * Performs an action using the service.
    * 
    * @param actionName the name of the action.
    * @param params the parameters for the action.
    * 
    * @return a BoxingHashMap with the return values from the action (may
    *         be empty).
    * 
    * @exception UPnPErrorException thrown if a UPnPError occurs.
    */
   public BoxingHashMap performAction(String actionName, Object[] params)
   throws UPnPErrorException
   {
      BoxingHashMap rval = new BoxingHashMap();
      
      // find the action to perform in service's list of actions
      UPnPServiceAction action =
         getService().getDescription().getActionList().getAction(actionName);
      if(action != null)
      {
         // create a soap operation
         SoapOperation operation = new SoapOperation(
            action.getName(), getService().getServiceType(), null);
         
         // add soap parameter for each action argument with "in" direction
         int count = 0;
         for(Iterator i = action.getArgumentList().iterator();
             i.hasNext(); count++) 
         {
            UPnPServiceActionArgument argument =
               (UPnPServiceActionArgument)i.next();
            
            if(argument.getDirection().equals("in"))
            {
               // add soap parameter, use passed parameter value
               operation.addParameter(new SoapOperationParameter(
                  argument.getName(), String.valueOf(params[count]), null));
            }
         }
         
         // create a rpc soap envelope
         RpcSoapEnvelope envelope = new RpcSoapEnvelope();
         
         // set encoding style
         envelope.setEncodingStyle(RpcSoapEnvelope.SOAP_ENCODING_URI);
         
         // set the operation
         envelope.setSoapOperation(operation);
         
         // send the envelope
         getServiceClient().sendSoapEnvelope(envelope, mService);
         
         // see if the envelope has a soap operation
         if(envelope.containsSoapOperation())
         {
            // pull out the envelope's soap operation
            operation = envelope.getSoapOperation();
            
            // get return value for each action argument with "out" direction
            for(Iterator i = action.getArgumentList().iterator(); i.hasNext();) 
            {
               UPnPServiceActionArgument argument =
                  (UPnPServiceActionArgument)i.next();
               
               if(argument.getDirection().equals("out"))
               {
                  // get the state variable for the argument
                  UPnPServiceStateVariable variable =
                     getService().getDescription().getStateTable().
                     getStateVariable(argument.getStateVariable());

                  // get the soap parameter that represents the result
                  SoapOperationParameter parameter =
                     operation.getParameter(argument.getName(), null);
                  
                  if(variable != null && parameter != null)
                  {
                     // convert the return value to the appropriate type
                     Object retval = UPnPServiceStateVariable.convertType(
                        parameter.getValue(), variable.getDataType());
                     
                     // add an entry to the map
                     rval.put(parameter.getName(), retval);
                  }
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the UPnPService this client implementation is for.
    * 
    * @return the UPnPService this client implementation is for.
    */
   public UPnPService getService()
   {
      return mService;
   }
   
   /**
    * Sets the UPnPServiceClient to use to communicate with the service.
    * 
    * @param client the UPnPServiceClient to use to communicate with the
    *               service.
    */
   public void setServiceClient(UPnPServiceClient client)
   {
      mServiceClient = client;
   }
   
   /**
    * Gets the UPnPServiceClient to use to communicate with the service.
    * 
    * @return the UPnPServiceClient to use to communicate with the service.
    */
   public UPnPServiceClient getServiceClient()
   {
      return mServiceClient;
   }
}
