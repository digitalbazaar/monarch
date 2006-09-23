/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

/**
 * A DatagramHandler is an object that listens for and accepts datagrams on
 * some port.
 * 
 * @author Dave Longley
 */
public interface DatagramHandler
{
   /**
    * Begins accepting datagrams on the given port.
    * 
    * @param port the port to start accepting datagrams on.
    */
   public void startAcceptingDatagrams(int port);
   
   /**
    * Stops accepting all datagrams. 
    */
   public void stopAcceptingDatagrams();
   
   /**
    * Returns true if this datagram handler is accepting datagrams, false if
    * it is not.
    * 
    * @return true if this datagram handler is accepting datagrams, false
    *         if not.
    */
   public boolean isAcceptingDatagrams();
   
   /**
    * Stops this handler's datagram servicer.
    */
   public void stopDatagramServicer();
   
   /**
    * Gets the port that this datagram handler is accepting datagrams on.
    * 
    * @return the port that this datagram handler is accepting datagrams on,
    *         or -1 if it is not accepting datagrams.
    */
   public int getPort();
}
