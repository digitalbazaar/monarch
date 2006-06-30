/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.FileInputStream;
import java.io.InputStream;
import java.net.Socket;
import java.security.KeyStore;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This client is used to connect to a web server that supports
 * http (HyperText Transfer protocol).
 * 
 * @author Dave Longley
 */
public class HttpWebClient
{
   /**
    * The host to connect to.
    */
   protected String mHost;
   
   /**
    * The port to connect to.
    */
   protected int mPort;
   
   /**
    * The ssl key managers, if any. 
    */
   protected KeyManager[] mKeyManagers;
   
   /**
    * The ssl trust managers, if any.
    */
   protected TrustManager[] mTrustManagers;
   
   /**
    * The path that follows the host and port name, if any.
    * 
    * |      host     |port|path|
    * http://localhost:8174/bss/
    */
   protected String mPath;
   
   /**
    * The end point address for this http client.
    */
   protected String mEndpointAddress;
   
   /**
    * The schema for the endpoint address, i.e. http, https.
    */
   protected String mSchema;
   
   /**
    * Creates a new http web client with the given endpoint address.
    * 
    * @param endpointAddress the endpoint address to connect to.
    */
   public HttpWebClient(String endpointAddress)
   {
      mEndpointAddress = "";
      mKeyManagers = null;
      mTrustManagers = new TrustManager[]{new TrustAllSSLManager()};
      setEndpointAddress(endpointAddress);
   }
   
   /**
    * Creates a new http web client with the given host and port.
    * 
    * @param host the host to connect to.
    * @param port the port to connect on.
    */
   public HttpWebClient(String host, int port)
   {
      this(host + ":" + port);
   }   
   
   /**
    * Parses an endpoint address into host, port, and path.
    */
   protected void parseEndpointAddress() 
   {
      getLogger().debug(getClass(),
         "parsing endpoint address for http web client...");
      
      String endpt = getEndpointAddress();
      String[] schema = endpt.split("://");
      
      getLogger().debug(getClass(), "endpt=" + endpt);
      
      try
      {
         // for storing the uri, not including the schema
         String uri;
         
         // schema first (http/https/etc)
         if(schema.length > 1)
         {
            mSchema = schema[0];
            uri = schema[1];
         }
         else
         {
            mSchema = "http";
            uri = schema[0];
         }
         
         getLogger().detail(getClass(), "schema=" + mSchema);

         // next split uri on colons
         String[] colons = uri.split(":");
         
         // hostname first (localhost:port)
         setHost(colons[0]);
         
         getLogger().detail(getClass(), "host=" + colons[0]);
         
         // split on slashes
         String[] slashes = colons[1].split("/");
         if(slashes.length >= 0)
         {
            // port first
            int port = Integer.parseInt(slashes[0]);
            setPort(port);
            
            getLogger().detail(getClass(), "port=" + getPort());
               
            // combine remaining slashes to get web service path
            String path = "/";
            for(int i = 1; i < slashes.length; i++)
            {
               path += slashes[i] + "/";
            }
            
            setWebServicePath(path);
            
            getLogger().detail(getClass(), 
               "web service path=" + getWebServicePath());
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), LoggerManager.getStackTrace(t));
      }
   }
   
   /**
    * Gets a web connection to the web service.
    * 
    * @return the web connection to the web service or null if failure.
    */
   protected WebConnection getWebConnection()
   {
      HttpWebConnection hwc = null;
      
      try
      {
         if(mSchema.equals("https"))
         {
            // create ssl context, factory
            SSLContext sslcontext = SSLContext.getInstance("TLS");
            sslcontext.init(mKeyManagers, mTrustManagers, null);
            SSLSocketFactory factory = sslcontext.getSocketFactory();

            // create ssl socket
            SSLSocket s = (SSLSocket)factory.createSocket(getHost(), getPort());
            
            String[] suites = factory.getSupportedCipherSuites();
            s.setEnabledCipherSuites(suites);
            
            // create web connection
            hwc = new HttpWebConnection(s);
         }
         else
         {
            // use regular socket
            Socket s = new Socket(getHost(), getPort());

            // create web connection
            hwc = new HttpWebConnection(s);
         }
         
         getLogger().debug(getClass(),
            "connected to: " + getHost() + ":" + getPort());
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), 
            "could not establish an http web connection.");
         getLogger().debug(getClass(), LoggerManager.getStackTrace(t));
         
         if(hwc != null)
         {
            hwc.disconnect();
         }
         
         hwc = null;
      }
      
      return hwc;
   }
   
   /**
    * Attempts to connect to the web service. If a connection cannot be
    * established, the connection will be retried multiple times.
    * 
    * @return the web connection to the web service or null if failure.
    */
   public WebConnection connect()
   {
      WebConnection wc = null;
      
      getLogger().debug(getClass(), 
         "trying to establish an http web connection...");
      
      // try 10 times = 10 seconds of retry
      int tries = 10;
      for(int i = 0; i < tries; i++)
      {
         wc = getWebConnection();

         // if web connection acquired, break
         if(wc != null)
         {
            break;
         }
         
         try
         {
            // sleep for a bit
            Thread.sleep(1000);
         }
         catch(Throwable t)
         {
         }
      }
      
      if(wc != null)
      {
         getLogger().debug(getClass(),
            "http web connection established," +
            "ip=" + wc.getHost() + ":" + wc.getRemotePort());
      }
      else
      {
         getLogger().error(getClass(), 
            "could not establish an http web connection!");
      }
      
      return wc;
   }
   
   /**
    * Sends an http web request from the client to the server.
    * 
    * @param request the http web request to send to the server.
    * @param is the input stream to read the request body from.
    * @return true if the request was successfully sent, false if not.
    */
   public boolean sendRequest(HttpWebRequest request, InputStream is)
   {
      boolean rval = false;
      
      if(request.sendHeader())
      {
         rval = request.sendBody(is);
      }
      
      return rval;
   }
   
   /**
    * Sends an http web request from the client to the server.
    * 
    * @param request the http web request to send to the server.
    * @param body the body to send along with the request.
    * @return true if the request was successfully sent, false if not.
    */
   public boolean sendRequest(HttpWebRequest request, String body)
   {
      boolean rval = false;
      
      if(request.sendHeader())
      {
         rval = request.sendBody(body);
      }
      
      return rval;
   }
   
   /**
    * Sends an http web request from the client to the server.
    * 
    * @param request the http web request to send to the server.
    * @param body the body to send along with the request.
    * @return true if the request was successfully sent, false if not.
    */
   public boolean sendRequest(HttpWebRequest request, byte[] body)
   {
      boolean rval = false;
      
      if(request.sendHeader())
      {
         rval = request.sendBody(body);
      }
      
      return rval;
   }   
   
   /**
    * Sends an http web request from the client to the server.
    * 
    * @param request the http web request to send to the server.
    * @return true if the request was successfully sent, false if not.
    */
   public boolean sendRequest(HttpWebRequest request)
   {
      boolean rval = false;
      
      rval = request.sendHeader();
      
      return rval;
   }

   /**
    * Recieves the response header from the server.
    * 
    * @param response the http response to read with.
    * @return true if the response header could be read, false if not.
    */
   public boolean receiveResponseHeader(HttpWebResponse response)
   {
      boolean rval = false;
      
      // keep reading while HTTP 1xx
      while(response.receiveHeader() &&
            response.getHeader().getStatusCode().startsWith("1"))
      {
         // read and discard body if status code is 1xx
         response.receiveBody();
      }
      
      // set whether or not the header was read
      rval = !response.getHeader().getStatusCode().startsWith("1");
      
      return rval;
   }

   /**
    * Sets the host to connect to.
    * 
    * @param host the host to connect to.
    */
   public void setHost(String host)
   {
      mHost = host;
   }
   
   /**
    * Gets the host to connect to.
    * 
    * @return the host to connect to.
    */
   public String getHost()
   {
      return mHost;
   }
   
   /**
    * Sets the port to connect on.
    * 
    * @param port the port to connect on.
    */
   public void setPort(int port)
   {
      mPort = port;
   }
   
   /**
    * Gets the port to connect on.
    * 
    * @return the port to connect on.
    */
   public int getPort()
   {
      return mPort;
   }
   
   /**
    * Sets an ssl certificate (from a keystore) to trust. Uses
    * the default algorithm for the certificate (SunX509).
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setTrustedSSLKeystore(String keystore, String password)
   {
      return setTrustedSSLKeystore(keystore, password, "SunX509");
   }

   /**
    * Sets an ssl certificate (from a keystore) to trust.
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @param algorithm the algorithm for the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setTrustedSSLKeystore(String keystore, String password,
                                        String algorithm)
   {
      boolean rval = false;
      
      try
      {
         // load keystore
         KeyStore ks = KeyStore.getInstance("JKS");
         ks.load(new FileInputStream(keystore), password.toCharArray());
         
         // get key managers
         KeyManagerFactory kmf = KeyManagerFactory.getInstance(algorithm);
         kmf.init(ks, password.toCharArray());
         mKeyManagers = kmf.getKeyManagers();
         
         // get trust managers
         String alg = TrustManagerFactory.getDefaultAlgorithm();
         TrustManagerFactory tmf = TrustManagerFactory.getInstance(alg);
         tmf.init(ks);
         mTrustManagers = tmf.getTrustManagers();

         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(), 
            "Could not load keystore!, keystore=" + keystore);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }   
   
   /**
    * Sets the path to the web service.
    * 
    * @param path the path to the web service.
    */
   public void setWebServicePath(String path)
   {
      mPath = path;
   }
   
   /**
    * Gets the path to the web service.
    * 
    * @return the path to the web service.
    */
   public String getWebServicePath()
   {
      return mPath;
   }
   
   /**
    * Sets the endpoint address.
    * 
    * @param endpointAddress the endpoint address for this client.
    */
   public void setEndpointAddress(String endpointAddress)
   {
      if(!mEndpointAddress.equals(endpointAddress))
      {
         mEndpointAddress = endpointAddress;
         parseEndpointAddress();
      }      
   }   

   /**
    * Gets the endpoint address.
    * 
    * @return the endpoint address for this client.
    */
   public String getEndpointAddress()
   {
      return mEndpointAddress;
   }   
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
