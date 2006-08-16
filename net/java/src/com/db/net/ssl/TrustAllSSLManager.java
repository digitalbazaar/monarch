/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.ssl;

import java.security.cert.X509Certificate;
import javax.net.ssl.X509TrustManager;

/**
 * An ssl trust manager that trusts all certificates. Used only to take
 * advantage of encryption/decryption features of ssl, not endpoint
 * identity verification.
 * 
 * @author Dave Longley
 */
public class TrustAllSSLManager implements X509TrustManager
{
   /**
    * Creates a new TrustAllSSLManager.
    */
   public TrustAllSSLManager()
   {
   }
   
   /**
    * Gets a list of the accepted issuers. Returns null.
    * 
    * @return a list of the accepted issuers.
    */
   public X509Certificate[] getAcceptedIssuers()
   {
      return null;
   }
   
   /**
    * Checks to see if the client is trusted.
    * 
    * @param certs the certificates to check.
    * @param authType the type of authorization. 
    */
   public void checkClientTrusted(X509Certificate[] certs, String authType)
   {
   }
   
   /**
    * Checks to see if the server is trusted.
    * 
    * @param certs the certificates to check.
    * @param authType the type of authorization. 
    */
   public void checkServerTrusted(X509Certificate[] certs, String authType)
   {
   }
}
