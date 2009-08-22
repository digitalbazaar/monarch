/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.net.DatagramPacket;

/**
 * Any class that implements this interface can service datagrams.
 * 
 * @author Dave Longley
 */
public interface DatagramServicer
{
   /**
    * Services a datagram.
    * 
    * @param datagram the datagram to be serviced.
    */
   public void serviceDatagram(DatagramPacket datagram);
}
