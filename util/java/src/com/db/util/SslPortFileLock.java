/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.File;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLServerSocketFactory;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

/**
 * An SslPortFileLock is PortFileLock that uses SSL communication.
 * 
 * @author Dave Longley
 */
public class SslPortFileLock extends PortFileLock
{
   /**
    * Creates a new SslPortFileLock that uses any available ephemeral port for
    * receiving passed information.
    * 
    * @param filename the name of the file to use for locking.
    */
   public SslPortFileLock(String filename)
   {
      this(filename, 0);
   }
   
   /**
    * Creates a new SslPortFileLock that uses any available ephemeral port for
    * receiving passed information.
    * 
    * @param file the file to use for locking.
    */
   public SslPortFileLock(File file)
   {
      this(file, 0);
   }
   
   /**
    * Creates a new SslPortFileLock that uses the specified port for receiving
    * passed information.
    * 
    * @param filename the name of the file to use for locking.
    * @param port the port to use for receiving passed information.
    */
   public SslPortFileLock(String filename, int port)
   {
      this(new File(filename), port);
   }
   
   /**
    * Creates a new SslPortFileLock that uses the specified port for receiving
    * passed information.
    * 
    * @param file the file to use for locking.
    * @param port the port to use for receiving passed information.
    */
   public SslPortFileLock(File file, int port)
   {
      super(file, port);
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
      // use a SSL server socket
      SSLServerSocket rval = null;
      
      try
      {
         // use a default factory
         SSLServerSocketFactory factory =
            (SSLServerSocketFactory)SSLServerSocketFactory.getDefault();

         // create ssl socket
         SSLServerSocket socket =
            (SSLServerSocket)factory.createServerSocket();
         
         // create socket address to bind to
         InetSocketAddress address = new InetSocketAddress(
             "127.0.0.1", port);
         
         // try to bind
         socket.bind(address);
         
         // client authorization not necessary
         socket.setNeedClientAuth(false);
         
         // use all supported cipher suites
         String[] suites = factory.getSupportedCipherSuites();
         socket.setEnabledCipherSuites(suites);
         
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
    * @param port the port to bind to.
    * 
    * @return the socket to use to pass information or null if the socket
    *         could not bind to the port.
    */
   protected Socket bindSocket(int port)
   {
      // use a SSL socket
      Socket rval = null;
      
      try
      {
         // use a default factory
         SSLSocketFactory factory =
            (SSLSocketFactory)SSLSocketFactory.getDefault();

         // create ssl socket
         SSLSocket socket = (SSLSocket)factory.createSocket();
         
         // create socket address to bind to
         InetSocketAddress address = new InetSocketAddress(
             "127.0.0.1", port);
         
         // try to bind
         socket.bind(address);
         
         // use all supported cipher suites
         String[] suites = factory.getSupportedCipherSuites();
         socket.setEnabledCipherSuites(suites);
         
         // set return value
         rval = socket;
      }
      catch(Throwable ignore)
      {
         // ignore failure, failure to be handled by the user of this class
      }
      
      return rval;
   }
}
