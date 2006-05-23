/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.FileInputStream;
import java.net.ServerSocket;
import java.security.KeyStore;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLServerSocketFactory;
import javax.net.ssl.TrustManager;

import com.db.logging.Logger;

/**
 * This class handles SSL web connections. 
 * 
 * @author Dave Longley
 */
public class SSLWebConnectionHandler extends AbstractWebConnectionHandler
{
   /**
    * The SSL key managers, if any. 
    */
   protected KeyManager[] mKeyManagers;
   
   /**
    * The SSL trust managers, if any.
    */
   protected TrustManager[] mTrustManagers;   
   
   /**
    * Creates a new SSLWebConnectionHandler.
    * 
    * @param wcs the web connection servicer used by this web connection handler
    *            to service web connections.
    */
   public SSLWebConnectionHandler(WebConnectionServicer wcs)
   {
      super(wcs);

      // set up trust managers
      mTrustManagers = new TrustManager[]{new TrustAllSSLManager()};
   }
   
   /**
    * Creates a new SSL server socket for listening on a port. 
    * 
    * @param port the port the server socket will listen on.
    * @return the new server socket.
    */
   protected ServerSocket createServerSocket(int port)
   {
      ServerSocket serverSocket = null; 
      
      try
      {
         // get an SSL server socket factory
         SSLServerSocketFactory factory = null;
         
         if(mKeyManagers != null)
         {
            // if key managers exist, use them to create ssl context, factory
            SSLContext sslcontext = SSLContext.getInstance("TLS");
            sslcontext.init(mKeyManagers, mTrustManagers, null);
            factory = sslcontext.getServerSocketFactory();
         }
         else
         {
            // no key managers, so use default factory
            factory =
               (SSLServerSocketFactory)SSLServerSocketFactory.getDefault();
         }
         
         // create ssl socket
         SSLServerSocket SSLServerSocket =
            (SSLServerSocket)factory.createServerSocket(port);
         
         // client authorization not necessary
         SSLServerSocket.setNeedClientAuth(false);
         
         // get all supported cipher suites and enable them
         String[] suites = factory.getSupportedCipherSuites();
         SSLServerSocket.setEnabledCipherSuites(suites);

         // set server socket
         serverSocket = SSLServerSocket;
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return serverSocket;
   }
   
   /**
    * Gets the amount of data (in bytes) necessary to detect the SSL protocol.
    * 
    * @return the amount of data (in bytes) needed to detect the protocol.
    */
   protected int getProtocolNumDetectBytes()
   {
      // minimum bytes required to determine client-hello & version
      return 5;
   }
   
   /**
    * Indicates whether or not connections handled by this web
    * connection handler should be considered secure.
    * 
    * Web connections handled by an SSL web connection handler are
    * considered secure. 
    * 
    * @return true if the connections handled by this web connection
    *         handler should be considered secure, false if they should
    *         not be considered secure. 
    */
   public boolean webConnectionsSecure()
   {
      // ssl connections considered secure
      return true;
   }
   
   /**
    * Indicates whether or not the passed web connection's protocol is
    * supported by this web connection handler. This method may
    * read from the web connection as long as it unreads whatever
    * data was read. This method is useful for determining whether or not
    * a web connection handler should accept a proxy web connection. 
    *
    * @param webConnection the web connection to inspect.
    * @return true if the web connection's protocol is supported, false
    *         if it is not.
    */
   public boolean isWebConnectionProtocolSupported(WebConnection webConnection)
   {
      boolean rval = false;
      
      // for reading from the web connection to determine the protocol
      boolean read = false;
      byte[] buffer = new byte[getProtocolNumDetectBytes()];
      int length = 0;
      
      try
      {
         getLogger().debug("Attempting to detect SSL protocol...");

         // read detect bytes from the web connection
         length = webConnection.read(buffer, 0, buffer.length);
         read = true;
         
         if(length >= 3)
         {
            getLogger().debug("TLS v1.0 record type of handshake(22)?: " +
                              buffer[0]);
            
            // check SSL record type
            // check for handshake (value of 22 or 0x16)
            if(buffer[0] == 0x16)
            {
               // offset + 1 is major version
               // offset + 2 is minor version
               // offset + 3,4 is a short that is the length of
               // data in the record excluding the header (max of 16384)
               getLogger().debug("TLS v1.0 record version major?: " +
                                 buffer[1]);
               getLogger().debug("TLS v1.0 record version minor?: " +
                                 buffer[2]);
               
               // SSL 3.0 / TLS 1.0 both have major version 3.0
               if(buffer[1] == 0x03)
               {
                  getLogger().debug("TLS v1.0 connection detected");
                  rval = true;
               }
            }
         }
         
         // not TLS 1.0, may be SSL 2.0/3.0
         if(!rval && length >= getProtocolNumDetectBytes())
         {
            getLogger().debug("not TLS v1.0, is it SSL v2.0/v3.0?");
            
            // EXPECT SSL 2.0/3.0:
            // * means optional, escape and padding only exist if
            // the most significant bit is set for record-length
            //
            // FORMAT:
            // HEADER {record-length, *is-escape-record?, *padding}
            // MAC-DATA[MAC_SIZE]
            // ACTUAL-DATA[N]
            // PADDING-DATA[PADDING]
            
            // for storing when the actual data starts
            int actualDataOffset = 0;
            
            // for storing record length
            int recLen = 0;
            
            // see if the most significant bit is set in the first byte
            if((buffer[0] & 0x80) == 0x80)
            {
               // total header length is 2 bytes
               actualDataOffset = 2;
                  
               // get the record length
               recLen = ((buffer[0] & 0x7F) << 8) | buffer[1];
            }
            else
            {
               // total header length is 3 bytes
               actualDataOffset = 3;
                
               // get the record length
               recLen = ((buffer[0] & 0x3F) << 8) | buffer[1];
            }
            
            getLogger().debug("SSL v2.0/v3.0 record size?: " + recLen);
            
            // get the client-hello
            int clientHello = buffer[actualDataOffset];
            getLogger().debug("SSL v2.0/v3.0 record client-hello(1)?: " +
                              clientHello);
            
            // get version
            int version = buffer[actualDataOffset + 1];
            if(version == 0)
            {
               version = buffer[actualDataOffset + 2];
            }
            
            getLogger().debug("SSL v2.0/v3.0 record version?: " + version);
            
            // if we have a client hello, then we have SSL v2.0 
            if(clientHello == 1)
            {
               getLogger().debug("SSL v" + version + ".0 " +
                                 "connection detected");
               rval = true;
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      // if data was read from the connection, then unread it
      if(read)
      {
         try
         {
            webConnection.unread(buffer, 0, length);
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      return rval;
   }
   
   /**
    * Sets an ssl certificate (from a keystore) for this plugin. Uses
    * the default algorithm for the certificate (SunX509).
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setSSLKeystore(String keystore, String password)
   {
      return setSSLKeystore(keystore, password, "SunX509");
   }
   
   /**
    * Sets an ssl certificate (from a keystore) for this plugin.
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @param algorithm the algorithm for the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setSSLKeystore(String keystore, String password,
                                 String algorithm)
   {
      boolean rval = false;
      
      try
      {
         // load key store
         KeyStore ks = KeyStore.getInstance("JKS");
         ks.load(new FileInputStream(keystore), password.toCharArray());
         
         // get key managers
         KeyManagerFactory kmf = KeyManagerFactory.getInstance(algorithm);
         kmf.init(ks, password.toCharArray());
         mKeyManagers = kmf.getKeyManagers();
         
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error("Could not load keystore!, keystore=" + keystore);
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      return rval;
   }
}
