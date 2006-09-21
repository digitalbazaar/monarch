/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.net.Socket;

/**
 * A PortFileLockConnectionHandler handles a connection to a PortFileLock.
 * 
 * It is responsible for reading from the passed socket and disconnecting it
 * when finished. This handler receives its own thread.
 * 
 * @author Dave Longley
 */
public interface PortFileLockConnectionHandler
{
   /**
    * Handles a connection to a PortFileLock.
    * 
    * @param socket the connected worker socket.
    */
   public void handlePortFileLockConnection(Socket socket);
}
