/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.client.igd;

import com.db.util.BoxingHashMap;

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
   protected BoxingHashMap mData;
   
   /**
    * Creates a new blank PortMapping.
    */
   public PortMapping()
   {
      // create data map
      mData = new BoxingHashMap();
      
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
   public PortMapping(BoxingHashMap retvals)
   {
      // set defaults
      this();
      
      // copy values
      setRemoteHost(retvals.getString("NewRemoteHost"));
      setExternalPort(retvals.getShort("NewExternalPort"));
      setProtocol(retvals.getString("NewProtocol"));
      setInternalPort(retvals.getShort("NewInternalPort"));
      setInternalClient(retvals.getString("NewInternalClient"));
      setEnabled(retvals.getBoolean("NewEnabled"));
      setDescription(retvals.getString("NewPortMappingDescription"));
      setLeaseDuration(retvals.getInt("NewLeaseDuration"));
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
      return mData.getString("RemoteHost");
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
      return mData.getInt("ExternalPort");
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
      return mData.getString("PortMappingProtocol");
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
      return mData.getInt("InternalPort");
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
      return mData.getString("InternalClient");
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
      return mData.getBoolean("PortMappingEnabled");
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
      return mData.getString("PortMappingDescription");
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
      return mData.getInt("PortMappingLeaseDuration");
   }
   
   /**
    * Gets a string representation of this PortMapping.
    * 
    * @return a string representation of this PortMapping.
    */
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
