/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

/**
 * A DatagramStream is a stream for datagrams. Datagrams can either be sent
 * or received on this stream. Datagrams are not guaranteed to be received
 * in the same order that they are sent in, as is the nature of datagrams.
 * 
 * This class is intentionally named DatagramStream and not DatagramConnection
 * because datagrams are connectionless. No connections are established to send
 * or receive datagrams, they are simply sent out to some destination and a
 * listened for from some destination. Hence this class refers to a "stream"
 * of datagrams, not a stateful connection overwhich datagrams are sent and
 * received. 
 * 
 * @author Dave Longley
 */
public class DatagramStream
{
   /**
    * The datagram socket for this connection. This socket is used to send
    * and receive datagram packets.
    */
   protected DatagramSocket mSocket;
   
   /**
    * Creates a new DatagramStream.
    * 
    * @param socket the DatagramSocket for this stream.
    */
   public DatagramStream(DatagramSocket socket)
   {
      // store socket
      mSocket = socket;
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
      
      if(!isClosed() && getSocket().isBound())
      {
         // set the address and port for the datagram
         datagram.setAddress(getSocket().getInetAddress());
         datagram.setPort(getSocket().getPort());
         
         // send the datagram
         getSocket().send(datagram);
      }
      
      return rval;
   }
   
   /**
    * Reads a datagram from this stream. This method will return false
    * if this stream has no origin from which to receive datagrams.
    * 
    * @param datagram the datagram to receive.
    * 
    * @return true if successfully sent, false if not.
    * 
    * @exception IOException thrown if an IO exception occurs.
    */
   public boolean receiveDatagram(DatagramPacket datagram)
   throws IOException
   {
      boolean rval = false;
      
      if(!isClosed())
      {
         // receive the datagram
         getSocket().receive(datagram);
      }
      
      return rval;
   }

   /**
    * Gets the datagram socket for this stream.
    * 
    * @return the datagram socket for this stream.
    */
   public DatagramSocket getSocket()
   {
      return mSocket;
   }
   
   /**
    * Sets the timeout (in milliseconds) for receiving a datagram from this
    * stream.
    * 
    * @param timeout the timeout (in milliseconds) for receiving a datagram
    *                from this stream.
    */
   public void setReadTimeout(int timeout)
   {
      if(!isClosed())
      {
         try
         {
            getSocket().setSoTimeout(Math.max(0, timeout));
         }
         catch(IOException ignore)
         {
         }
      }
   }
   
   /**
    * Gets the timeout (in milliseconds) for receiving a datagram from this
    * stream.
    * 
    * @return the timeout (in milliseconds) for receiving a datagram from this
    *         stream.
    * 
    * @exception IOException thrown if an IOException occurs.
    */
   public int getReadTimeout() throws IOException
   {
      return getSocket().getSoTimeout();
   }
   
   /**
    * Closes this stream. This stream will no longer be capable of sending
    * or receiving datagrams.
    */
   public void close()
   {
      if(!isClosed())
      {
         // disconnect and close the datagram socket
         getSocket().disconnect();
         getSocket().close();
      }
   }
   
   /**
    * Returns true if this stream is closed, false if not.
    *
    * @return true if this stream is closed, false if not.
    */
   public boolean isClosed()
   {
      boolean rval = false;
      
      if(mSocket != null && mSocket.isClosed())
      {
         rval = true;
      }
      
      return rval;
   }
}
