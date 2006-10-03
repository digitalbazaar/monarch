/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;

import com.db.logging.Logger;

/**
 * A PortFileLock is an object that is used to create a lock file that is
 * used to lock the use of some resource. It also opens a port that allows 
 * information to be received when another process tries to acquire a lock
 * but this object has already has the lock.
 * 
 * The port number for this PortFileLock will be written (and is the only
 * data written) to the file used for locking. It will be written as
 * text, not binary.
 * 
 * A common use for this object is to prevent multiple instances of the
 * same application from being run, and, when a second instance fails to
 * run, to allow the command line arguments for that second instance to
 * be passed to the first instance that is already running.
 * 
 * @author Dave Longley
 */
public class PortFileLock extends FileLock
{
   /**
    * The port to use to receive passed information.
    */
   protected int mPort;
   
   /**
    * The connection handler for this PortFileLock. This is the object
    * that handles any incoming connections to the server socket that is
    * listening to the port for this PortFileLock. It can be null.
    */
   protected PortFileLockConnectionHandler mConnectionHandler;
   
   /**
    * Creates a new PortFileLock that uses any available ephemeral port for
    * receiving passed information.
    * 
    * @param filename the name of the file to use for locking.
    */
   public PortFileLock(String filename)
   {
      this(filename, 0);
   }
   
   /**
    * Creates a new PortFileLock that uses any available ephemeral port for
    * receiving passed information.
    * 
    * @param file the file to use for locking.
    */
   public PortFileLock(File file)
   {
      this(file, 0);
   }
   
   /**
    * Creates a new PortFileLock that uses the specified port for receiving
    * passed information.
    * 
    * @param filename the name of the file to use for locking.
    * @param port the port to use for receiving passed information.
    */
   public PortFileLock(String filename, int port)
   {
      this(new File(filename), port);
   }
   
   /**
    * Creates a new PortFileLock that uses the specified port for receiving
    * passed information.
    * 
    * @param file the file to use for locking.
    * @param port the port to use for receiving passed information.
    */
   public PortFileLock(File file, int port)
   {
      super(file);
      
      // set the port
      mPort = port;
      
      // set no connection handler
      setConnectionHandler(null);
   }
   
   /**
    * Gets the socket to use to listen for information.
    * 
    * @param port the port to open or 0 to open any free ephemeral port. 
    * 
    * @return the socket to use to listen for information or null if the
    *         socket could not be bound to the specified port.
    */
   protected ServerSocket bindServerSocket(int port)
   {
      // use a generic server socket
      ServerSocket rval = null;
      
      try
      {
         // create the server socket
         ServerSocket socket = new ServerSocket();
         
         // create socket address to bind to
         InetSocketAddress address = new InetSocketAddress(
             "127.0.0.1", port);
         
         // try to bind
         socket.bind(address);
         
         // set return value
         rval = socket;
      }
      catch(Throwable ignore)
      {
         // ignore failure, failure to be handled by the user of this class
      }
      
      return rval;
   }
   
   /**
    * Gets the socket to use to pass information.
    * 
    * @param port the port to connect to.
    * 
    * @return the socket to use to pass information or null if the socket
    *         could not connect to the port.
    */
   protected Socket connectSocket(int port)
   {
      // use a generic socket
      Socket rval = null;
      
      try
      {
         // create the socket
         Socket socket = new Socket();
         
         // create socket address to bind to
         InetSocketAddress address = new InetSocketAddress(
             "127.0.0.1", port);
         
         // try to connect
         socket.connect(address);
         
         // set return value
         rval = socket;
      }
      catch(Throwable t)
      {
         // there was some error
         getLogger().error(getClass(),
            "An exception occurred while trying to connect to " +
            "pass information!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Accepts connections to the information port and passes them off to
    * the connection handler so long as this object has a lock.
    * 
    * @param socket the server socket to accept connections with.
    */
   public void acceptConnections(ServerSocket socket)
   {
      try
      {
         // set the timeout for the socket to something low so we can check
         // to make sure we still have a lock
         socket.setSoTimeout(10);
         
         // keep accepting connections until the lock is released
         while(hasLock())
         {
            try
            {
               // accept a connection
               Socket worker = socket.accept();
               
               // only try to pass the worker off if it isn't null
               if(worker != null)
               {
                  synchronized(this)
                  {
                     // pass the worker off to the connection handler, if any
                     if(getConnectionHandler() != null)
                     {
                        // execute handler in a background process
                        MethodInvoker mi = new MethodInvoker(
                           getConnectionHandler(),
                           "handlePortFileLockConnection", worker);
                        mi.backgroundExecute();
                     }
                     else
                     {
                        // no handler, so disconnect the connection
                        worker.close();
                     }
                  }
               }
            }
            catch(SocketTimeoutException ignore)
            {
               // ignore socket timeout exceptions
            }
         }
      }
      catch(IOException e)
      {
         // there was some IO error
         getLogger().error(getClass(),
            "An exception occurred while trying to accept connections!");
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      // release the lock
      unlock();
      
      try
      {
         // close the socket
         socket.close();
      }
      catch(IOException ignore)
      {
         // ignore, just close the server socket
      }
   }
   
   /**
    * Locks this FileLock, if it is not already locked.
    * 
    * @return true if this FileLock successfully locked (or was already
    *         locked), false if it was not because it was already locked by
    *         another process or because the file could not be created.
    */
   @Override
   public synchronized boolean lock()
   {
      // ensure lock has not already been acquired
      if(!hasLock())
      {
         // acquire the file lock
         if(super.lock())
         {
            try
            {
               // get a server socket
               ServerSocket socket = bindServerSocket(getPort());
               if(socket != null)
               {
                  // write the port number to the lock file
                  FileOutputStream fos = new FileOutputStream(getFile(), true);
                  fos.write(new String("" + socket.getLocalPort()).getBytes());
                  fos.close();
                  
                  // start accepting connections in a background process
                  MethodInvoker mi = new MethodInvoker(
                     this, "acceptConnections", socket);
                  mi.backgroundExecute();
               }
               else
               {
                  // socket could not be opened, so unlock
                  unlock();
               }
            }
            catch(Throwable t)
            {
               // there was some IO error
               getLogger().error(getClass(),
                  "An exception occurred while trying to open " +
                  "a server socket!");
               getLogger().debug(getClass(), Logger.getStackTrace(t));
               
               // unlock
               unlock();
            }
         }
      }
      
      return hasLock();      
   }
   
   /**
    * Gets a socket to write data to whomever has the lock this PortFileLock
    * cannot acquire. This method is used, for instance, to send command line
    * parameters to a running instance of an application that maintains the
    * lock this PortFileLock cannot acquire.
    * 
    * The caller of this method is required to close the socket when finished
    * using it.
    * 
    * @return the socket to communicate with, or null if none is available.
    */
   public Socket getCommSocket()
   {
      Socket rval = null;
      
      try
      {
         // open the locked file for reading
         FileInputStream fis = new FileInputStream(getFile());
         
         // largest port number is 65535 (5 bytes) written as text
         byte[] buffer = new byte[5];
         int numBytes = fis.read(buffer);
         fis.close();
         
         if(numBytes > 0)
         {
            // get the port number
            int port = Integer.parseInt(new String(buffer, 0, numBytes));
            
            // connect a socket to the port
            Socket socket = connectSocket(port);
            
            // return the socket
            rval = socket;
         }
      }
      catch(Throwable t)
      {
         // there was some IO error
         getLogger().error(getClass(),
            "An exception occurred while trying to get a comm socket!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Sets the connection handler for this PortFileLock. This is the object
    * that handles any incoming connections to the server socket that is
    * listening to the port for this PortFileLock. 
    * 
    * @param handler the PortFileLockConnectionHandler for this PortFileLock.
    */
   public synchronized void setConnectionHandler(
      PortFileLockConnectionHandler handler)
   {
      mConnectionHandler = handler;
   }
   
   /**
    * Gets the connection handler for this PortFileLock. This is the object
    * that handles any incoming connections to the server socket that is
    * listening to the port for this PortFileLock. 
    * 
    * @return the PortFileLockConnectionHandler for this PortFileLock (can be
    *         null).
    */
   public synchronized PortFileLockConnectionHandler getConnectionHandler()
   {
      return mConnectionHandler;
   }
   
   /**
    * Gets the port to use to pass information or 0 if no port has been
    * assigned yet.
    * 
    * @return the port to use to pass information or 0 if no port has been
    *         assigned yet.
    */
   public int getPort()
   {
      return mPort;
   }
}
