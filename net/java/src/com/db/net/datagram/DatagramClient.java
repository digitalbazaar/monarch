/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MulticastSocket;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A DatagramClient is used to send datagrams and receive them in response.
 * 
 * @author Dave Longley
 */
public class DatagramClient
{
   /**
    * Creates a new DatagramClient.
    */
   public DatagramClient()
   {
   }
   
   /**
    * Gets a DatagramStream to communicate with a single datagram server.
    * 
    * @param localPort the local port to bind to.
    * @param host the remote host (IP or hostname) to send the datagrams to.
    * @param port the remote port to send the datagrams to.
    * 
    * @return the DatagramStream to send and receive datagrams with.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public DatagramStream getDatagramStream(int localPort, String host, int port)
   throws IOException
   {
      // create a datagram socket
      DatagramSocket socket = new DatagramSocket(localPort);
      
      // connect to the remote host
      socket.connect(InetAddress.getByName(host), port);
      
      // create a datagram stream
      DatagramStream stream = new DatagramStream(socket);
      
      // return the stream
      return stream;
   }
   
   /**
    * Gets a DatagramMulticastStream to communicate with a multicast group.
    * 
    * @param groupHost the multicast group host address (remote IP or hostname).
    * @param groupPort the multicast group port.
    * @param joinGroup true to join the multicast group, false not to (join
    *                  the group if the stream should receive messages sent
    *                  to the group).
    * 
    * @return the DatagramMulticastStream to send and receive datagrams with.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public DatagramMulticastStream getMulticastStream(
      String groupHost, int groupPort, boolean joinGroup)
   throws IOException
   {
      DatagramMulticastStream rval = null;
      
      if(joinGroup)
      {
         rval = getMulticastStream(groupPort, groupHost, groupPort, true);
      }
      else
      {
         rval = getMulticastStream(-1, groupHost, groupPort, false);
      }
      
      return rval;
   }
   
   /**
    * Gets a DatagramMulticastStream to communicate with a multicast group.
    * 
    * @param localPort the local port to bind to.
    * @param groupHost the multicast group host address (remote IP or hostname).
    * @param groupPort the multicast group port.
    * @param joinGroup true to join the multicast group, false not to (join
    *                  the group if the stream should receive messages sent
    *                  to the group).
    * 
    * @return the DatagramMulticastStream to send and receive datagrams with.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public DatagramMulticastStream getMulticastStream(
      int localPort, String groupHost, int groupPort, boolean joinGroup)
   throws IOException
   {
      // create a new multicast socket
      MulticastSocket socket = null;
      if(localPort != -1)
      {
         socket = new MulticastSocket(localPort);
      }
      else
      {
         socket = new MulticastSocket();
      }
      
      // create a datagram multicast stream
      DatagramMulticastStream stream =
         new DatagramMulticastStream(socket, groupHost, groupPort, joinGroup);
      
      // return the stream
      return stream;
   }
   
   /**
    * Gets the logger for this client.
    * 
    * @return the logger for this client.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
