/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import java.util.HashMap;

/**
 * A PortMapping on an Internet Gateway Device.
 * 
 * @author Dave Longley
 */
public class PortMapping
{
   /**
    * A BoxingHashMap that stores the data for this PortMapping.
    */
   protected HashMap<Object, Object> mData;
   
   /**
    * Creates a new blank PortMapping.
    */
   public PortMapping()
   {
      // create data map
      mData = new HashMap<Object, Object>();
      
      // set defaults
      setRemoteHost("");
      setExternalPort(-1);
      setProtocol("");
      setInternalPort(-1);
      setInternalClient("");
      setEnabled(false);
      setDescription("");
      setLeaseDuration(0);
   }
   
   /**
    * Creates a new PortMapping from a set of return values from an action.
    * 
    * @param retvals a map of return values from an action.
    */
   public PortMapping(HashMap retvals)
   {
      // set defaults
      this();
      
      // copy values
      setRemoteHost((String)retvals.get("NewRemoteHost"));
      setExternalPort((Short)retvals.get("NewExternalPort"));
      setProtocol((String)retvals.get("NewProtocol"));
      setInternalPort((Short)retvals.get("NewInternalPort"));
      setInternalClient((String)retvals.get("NewInternalClient"));
      setEnabled((Boolean)retvals.get("NewEnabled"));
      setDescription((String)retvals.get("NewPortMappingDescription"));
      setLeaseDuration((Integer)retvals.get("NewLeaseDuration"));
   }
   
   /**
    * Sets the remote host for this port mapping.
    * 
    * This is an IP address of the form "x.x.x.x" or a blank string.
    * 
    * @param host the remote host for this port mapping.
    */
   public void setRemoteHost(String host)
   {
      mData.put("RemoteHost", host);
   }
   
   /**
    * Gets the remote host for this port mapping.
    * 
    * This is an IP address of the form "x.x.x.x" or a blank string.
    * 
    * @return the remote host for this port mapping.
    */
   public String getRemoteHost()
   {
      return (String)mData.get("RemoteHost");
   }
   
   /**
    * Sets the external port for this port mapping.
    * 
    * @param port the external port for this port mapping.
    */
   public void setExternalPort(int port)
   {
      mData.put("ExternalPort", port);
   }   
   
   /**
    * Gets the external port for this port mapping.
    * 
    * @return the external port for this port mapping.
    */
   public int getExternalPort()
   {
      return (Integer)mData.get("ExternalPort");
   }
   
   /**
    * Sets the protocol for this port mapping.
    * 
    * This is either "TCP" or "UDP".
    * 
    * @param protocol the protocol for this port mapping.
    */
   public void setProtocol(String protocol)
   {
      mData.put("PortMappingProtocol", protocol);
   }
   
   /**
    * Gets the protocol for this port mapping.
    * 
    * This is either "TCP" or "UDP".
    * 
    * @return the protocol for this port mapping.
    */
   public String getProtocol()
   {
      return (String)mData.get("PortMappingProtocol");
   }
   
   /**
    * Sets the internal port for this port mapping.
    * 
    * @param port the internal port for this port mapping.
    */
   public void setInternalPort(int port)
   {
      mData.put("InternalPort", port);
   }
   
   /**
    * Gets the internal port for this port mapping.
    * 
    * @return the internal port for this port mapping.
    */
   public int getInternalPort()
   {
      return (Integer)mData.get("InternalPort");
   }
   
   /**
    * Sets the internal client for this port mapping.
    * 
    * This is an IP address of the form "x.x.x.x" or a blank string.
    * 
    * @param address the internal client address for this port mapping.
    */
   public void setInternalClient(String address)
   {
      mData.put("InternalClient", address);
   }
   
   /**
    * Gets the internal client for this port mapping.
    * 
    * This is an IP address of the form "x.x.x.x" or a blank string.
    * 
    * @return the internal client for this port mapping.
    */
   public String getInternalClient()
   {
      return (String)mData.get("InternalClient");
   }
   
   /**
    * Sets whether or not this port mapping is enabled.
    * 
    * @param enable true to enable this port mapping, false to disable it.
    */
   public void setEnabled(boolean enable)
   {
      mData.put("PortMappingEnabled", enable);
   }
   
   /**
    * Gets whether or not this port mapping is enabled.
    * 
    * @return true if this port mapping is enabled, false if not.
    */
   public boolean isEnabled()
   {
      return (Boolean)mData.get("PortMappingEnabled");
   }
   
   /**
    * Sets a description for this port mapping.
    * 
    * @param description the description for this port mapping.
    */
   public void setDescription(String description)
   {
      mData.put("PortMappingDescription", description);
   }
   
   /**
    * Gets a description for this port mapping.
    * 
    * @return the description for this port mapping.
    */
   public String getDescription()
   {
      return (String)mData.get("PortMappingDescription");
   }
   
   /**
    * Sets the lease duration for this port mapping. Measured in seconds.
    * 
    * @param seconds the lease duration for this port mapping (in seconds).
    */
   public void setLeaseDuration(int seconds)
   {
      mData.put("PortMappingLeaseDuration", seconds);
   }
   
   /**
    * The lease duration for this port mapping. Measured in seconds.
    * 
    * @return the lease duration for this port mapping.
    */
   public int getLeaseDuration()
   {
      return (Integer)mData.get("PortMappingLeaseDuration");
   }
   
   /**
    * Gets a string representation of this PortMapping.
    * 
    * @return a string representation of this PortMapping.
    */
   @Override
   public String toString()
   {
      StringBuffer sb = new StringBuffer();

      sb.append("[PortMapping]");
      
      sb.append("\nRemoteHost=" + getRemoteHost());
      sb.append("\nExternalPort=" + getExternalPort());
      sb.append("\nProtocol=" + getProtocol());
      sb.append("\nInternalPort=" + getInternalPort());
      sb.append("\nInternalClient=" + getInternalClient());
      sb.append("\nPortMappingEnabled=" + isEnabled());
      sb.append("\nPortMappingDescription=" + getDescription());
      sb.append("\nPortMappingLeaseDuration=" + getLeaseDuration());
      
      sb.append("\n");
      
      return sb.toString();      
   }
}
