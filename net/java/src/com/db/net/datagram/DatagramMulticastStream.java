/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

/**
 * A DatagramMulticastStream is a DatagramStream that sends simultaneously
 * sends datagrams to all members in its multicast group.
 * 
 * @author Dave Longley
 */
public class DatagramMulticastStream extends DatagramStream
{
   /**
    * The address for the multicast group.
    */
   protected InetAddress mGroupAddress;
   
   /**
    * The port for the multicast group.
    */
   protected int mGroupPort;
   
   /**
    * Creates a new DatagramMulticastStream and joins the multicast group
    * with the specified host.
    * 
    * @param socket the MulticastSocket for this stream.
    * @param groupHost the host for the multicast group.
    * @param groupPort the port for the multicast group.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public DatagramMulticastStream(
      MulticastSocket socket, String groupHost, int groupPort)
   throws IOException
   {
      this(socket, groupHost, groupPort, true);
   }
   
   /**
    * Creates a new DatagramMulticastStream.
    * 
    * @param socket the MulticastSocket for this stream.
    * @param groupHost the host for the multicast group.
    * @param groupPort the port for the multicast group.
    * @param joinGroup true to join the multicast group, false not to.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public DatagramMulticastStream(
      MulticastSocket socket, String groupHost, int groupPort,
      boolean joinGroup)
   throws IOException
   {
      super(socket);
      
      // store the group address
      mGroupAddress = InetAddress.getByName(groupHost);
      
      // store the group port
      mGroupPort = groupPort;

      if(joinGroup)
      {
         // join the multicast group at the given address
         socket.joinGroup(mGroupAddress);
      }
   }
   
   /**
    * Writes a datagram to this stream. This method will return false if this
    * stream has no destination to send datagrams to.
    * 
    * @param datagram the datagram to send.
    * 
    * @return true if successfully sent, false if not.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public boolean sendDatagram(DatagramPacket datagram)
   throws IOException
   {
      boolean rval = false;
      
      if(!isClosed())
      {
         // set the address and port for the datagram
         datagram.setAddress(getGroupAddress());
         datagram.setPort(getGroupPort());
         
         // send the datagram
         getSocket().send(datagram);
      }
      
      return rval;
   }
   
   /**
    * Gets the multicast socket for this stream.
    * 
    * @return the multicast socket for this stream.
    */
   public MulticastSocket getMulticastSocket()
   {
      return (MulticastSocket)getSocket();
   }
   
   /**
    * Gets the internet address for the multicast group for this stream.
    * 
    * @return the internet address for the multicast group for this stream.
    */
   public InetAddress getGroupAddress()
   {
      return mGroupAddress;
   }
   
   /**
    * Gets the port for the multicast group for this stream.
    * 
    * @return the port for the multicast group for this stream.
    */
   public int getGroupPort()
   {
      return mGroupPort;
   }   
   
   /**
    * Sets the time to live for the datagrams sent to the multicast group.
    * 
    * @param ttl the time to live for the datagrams sent to the multicast group.
    */
   public void setTimeToLive(int ttl)
   {
      if(!isClosed())
      {
         try
         {
            getMulticastSocket().setTimeToLive(ttl);
         }
         catch(IOException ignore)
         {
         }
      }
   }
   
   /**
    * Sets the time to live for the datagrams sent to the multicast group.
    * 
    * @return the time to live for the datagrams sent to the multicast group.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public int getTimeToLive() throws IOException
   {
      return getMulticastSocket().getTimeToLive();
   }
   
   /**
    * Closes this stream. This stream will no longer be capable of sending
    * or receiving datagrams.
    * 
    * The stream will leave the multicast group.
    */
   public void close()
   {
      close(true);
   }
   
   /**
    * Closes this stream. This stream will no longer be capable of sending
    * or receiving datagrams.
    * 
    * @param leaveGroup true to leave the multicast group, false not to.
    */
   public void close(boolean leaveGroup)
   {
      if(!isClosed())
      {
         if(leaveGroup)
         {
            try
            {
               // leave the group
               getMulticastSocket().leaveGroup(
                  getMulticastSocket().getInetAddress());
            }
            catch(Throwable ignore)
            {
            }
         }
         
         // close the datagram socket
         getSocket().close();
      }      
   }
}
