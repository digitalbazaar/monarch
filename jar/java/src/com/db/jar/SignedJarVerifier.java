/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.jar;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.CodeSigner;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;

import sun.security.util.SignatureFileVerifier;

/**
 * A SignedJarVerifier is used to verify signed jar files.
 * 
 * @author Dave Longley
 */
public class SignedJarVerifier
{
   /**
    * The valid certificates. This is the list of certificates that can
    * be used to verify jars.
    */
   protected Certificate[] mCertificates;
   
   /**
    * A hashmap of valid certificates. Each entry in the map is an alias
    * that points to a certificate chain.
    */
   protected HashMap<String, Certificate[]> mAliasToCertificateChain;
   
   /**
    * Stores the minimum number of certificates that must be validated for
    * any jar entry.
    */
   protected int mMinCertificateCount;
   
   /**
    * Set to true if all of the certificates in the list of certificates
    * must have been used to verify a jar in order for it to pass
    * verification.
    */
   protected boolean mRequireAllCertificatesToVerify;
   
   /**
    * Creates a new default SignedJarVerifier. This verifier will only
    * ensure that jars that are verified were signed by "some certificate" --
    * if you wish to restrict that "some certificate" you must use a
    * constructor that specifies an array of certificates or a keystore.
    */
   public SignedJarVerifier()
   {
      this(null, false);
   }
   
   /**
    * Creates a new SignedJarVerifier. A jar will only pass verification if
    * either <code>all</code> is set to false and the jar has been verified
    * by at least one certificate owned by an alias in the list of aliases,
    * or if <code>all</code> has been set to true and all of the aliases
    * must have a certificate that verified the jar.
    * 
    * @param keystoreFilename the filename of the keystore to use.
    * @param keystorePassword the keystore password.
    * @param aliases the list of aliases to use from the keystore.
    * @param all true to require that every alias has a certificate that
    *            verified the jar, false to require that at least one
    *            alias has a certificate that has verified the jar.
    *            
    * @exception CertificateException if any certificates can't be loaded from
    *                                 the keystore.
    * @exception IOException if there is an IO error while reading the keystore.
    * @exception KeyStoreException if there is an error with the keystore.
    * @exception NoSuchAlgorithmException if the algorithm for the keystore
    *                                     can't be found.
    */
   public SignedJarVerifier(
      String keystoreFilename, String keystorePassword, String[] aliases,
      boolean all)
   throws CertificateException, IOException, KeyStoreException,
          NoSuchAlgorithmException
   {
      // array of certificates not used
      mCertificates = null;
      
      // load certificates into map
      loadCertificates(keystoreFilename, keystorePassword, aliases);
      mRequireAllCertificatesToVerify = all;
      
      // set the minimum number of certificates required to verify a jar entry
      if(mRequireAllCertificatesToVerify)
      {
         mMinCertificateCount = aliases.length;
      }
      else
      {
         mMinCertificateCount = 1;
      }      
   }
   
   /**
    * Creates a new SignedJarVerifier. A jar will only pass verification if
    * either <code>all</code> is set to false and the jar has been verified
    * by at least one certificate owned by an alias in the list of aliases,
    * or if <code>all</code> has been set to true and all of the aliases
    * must have a certificate that verified the jar.
    * 
    * @param keystore the keystore to get the code signers from.
    * @param aliases the list of aliases to use from the keystore.
    * @param all true to require that every alias has a certificate that
    *            verified the jar, false to require that at least one
    *            alias has a certificate that has verified the jar.
    *            
    * @exception KeyStoreException if there is an error with the keystore.
    */
   public SignedJarVerifier(KeyStore keystore, String[] aliases, boolean all)
   throws KeyStoreException
   {
      // array of certificates not used
      mCertificates = null;

      // load certificates into map
      loadCertificates(keystore, aliases);
      mRequireAllCertificatesToVerify = all;
      
      // set the minimum number of certificates required to verify a jar entry
      if(mRequireAllCertificatesToVerify)
      {
         mMinCertificateCount = aliases.length;
      }
      else
      {
         mMinCertificateCount = 1;
      }
   }
   
   /**
    * Creates a new SignedJarVerifier. A jar will only pass verification if
    * either <code>all</code> is set to false and the jar has been verified
    * by at least one certificate in the passed list, or if <code>all</code>
    * has been set to true and all of the certificates have verified the jar.
    * 
    * @param certificates the accepted certificates.
    * @param all true to require that all of the certificates have verified
    *            a jar for it to be considered verified, false to require that
    *            at least certificate to have verified a jar for it to be
    *            considered verified.
    */
   public SignedJarVerifier(Certificate[] certificates, boolean all)
   {
      // map of certificates not used
      mAliasToCertificateChain = null;
      
      // store certificates
      mCertificates = certificates;
      mRequireAllCertificatesToVerify = all;
      
      // set the minimum number of certificates required to verify a jar entry
      if(mRequireAllCertificatesToVerify)
      {
         mMinCertificateCount = mCertificates.length;
      }
      else
      {
         mMinCertificateCount = 1;
      }
   }   
   
   /**
    * Gets the certificates from a keystore into a map with entries of
    * alias -> certificateChain.
    * 
    * @param keystoreFilename the filename of the keystore.
    * @param keystorePassword the keystore password.
    * @param aliases the aliases for the keystore.
    * 
    * @exception CertificateException if any certificates can't be loaded from
    *                                 the keystore.
    * @exception IOException if there is an IO error while reading the keystore.
    * @exception KeyStoreException if there is an error with the keystore.
    * @exception NoSuchAlgorithmException if the algorithm for the keystore
    *                                     can't be found.
    */
   protected void loadCertificates(
      String keystoreFilename, String keystorePassword, String[] aliases)
   throws CertificateException, IOException, KeyStoreException,
          NoSuchAlgorithmException
   {
      // load keystore
      FileInputStream fis = new FileInputStream(keystoreFilename);
      KeyStore keystore = KeyStore.getInstance("JKS");
      keystore.load(fis, keystorePassword.toCharArray());
      
      // load the certificates from the keystore
      loadCertificates(keystore, aliases);
   }

   /**
    * Gets the certificates from a keystore into a map with entries of
    * alias -> certificateChain.
    * 
    * @param keystore the keystore.
    * @param aliases the aliases for the keystore.
    * 
    * @exception KeyStoreException if there is an error with the keystore.
    */
   protected void loadCertificates(KeyStore keystore, String[] aliases)
   throws KeyStoreException
   {
      // create the certificates map
      mAliasToCertificateChain = new HashMap<String, Certificate[]>();
      
      // iterate through the aliases
      for(int i = 0; i < aliases.length; i++)
      {
         // load certificates from keystore for the alias
         Certificate[] chain = keystore.getCertificateChain(aliases[i]);
         
         if(chain != null)
         {
            // add entry to the map
            mAliasToCertificateChain.put(aliases[i], chain);
         }
         else
         {
            // add a trusted certificate if one exists
            Certificate cert = keystore.getCertificate(aliases[i]);
            if(cert != null)
            {
               chain = new Certificate[1];
               chain[0] = cert;

               // add entry to the map
               mAliasToCertificateChain.put(aliases[i], chain);
            }
         }
      }
   }
   
   /**
    * If <code>all</code> is true then this method checks to see if the
    * first array of certificates contains all of the certificates in
    * the second array. If <code>all</code> is false, then this method
    * checks to see if at least one certificate from the secon array is
    * in the first array.
    * 
    * @param certs1 the first array of certificates.
    * @param certs2 the second array of certificates.
    * @param all true to check to see if the two arrays of certificates
    *            are equal, false to see if the first array contains
    *            at least one certificate from the second array.
    * 
    * @return true if enough certificates from the second array were found
    *         in the first array.
    */
   protected boolean findArrayCertificates(
      Certificate[] certs1, Certificate[] certs2, boolean all)
   {
      boolean rval = false;
      
      if(all)
      {
         rval = Arrays.equals(certs1, certs2);
      }
      else
      {
         // ensure at least one of the certificates in the second array
         // is in the first array -- keep checking the first array
         // until we run out of certificates or one is found from the
         // second array
         for(int i = 0; i < certs1.length && !rval; i++)
         {
            for(int j = 0; j < certs2.length && !rval; j++)
            {
               if(certs1[i].equals(certs2[j]))
               {
                  // certificate found
                  rval = true;
               }
            }
         }
      }

      return rval;
   }
   
   /**
    * Checks to see if the map of certificates for this verifier are
    * in the passed array of entry certificates.
    * 
    * @param entryCertificates the entry certificates.
    * 
    * @return true if the certificates were found in the entry certificates,
    *         false if not.
    */
   protected boolean findMappedCertificates(Certificate[] entryCertificates)
   {
      boolean rval = false;
      
      // go through the alias map and check the certificates --
      // keep checking until a certificate is deemed missing or
      // the minimum certificate count is reached
      boolean certificateMissing = false;
      int foundCount = 0;
      for(Iterator i = mAliasToCertificateChain.values().iterator();
          i.hasNext() && foundCount < mMinCertificateCount &&
          !certificateMissing;)
      {
         Certificate[] chain = (Certificate[])i.next();
         
         if(findArrayCertificates(entryCertificates, chain, false))
         {
            foundCount++;
         }
         else if(mRequireAllCertificatesToVerify)
         {
            // certificate missing if at least one certificate was not
            // found for this alias and all are required
            certificateMissing = true;
         }
      }
      
      // see if enough certificates were found
      if(!certificateMissing && foundCount >= mMinCertificateCount)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Checks to see if the certificates for this verifier are found in
    * the passed entry certificates.
    * 
    * @param entryCertificates the entry certificates.
    * 
    * @return true if the certificates were found in the entry certificates,
    *         false if not.
    */
   protected boolean findCertificates(Certificate[] entryCertificates)
   {
      boolean rval = false;
      
      // ensure that there are enough entry certificates
      if(entryCertificates.length >= mMinCertificateCount)
      {
         // determine if we're using the array of certificates or the 
         // alias to certificate chain map
         if(mCertificates != null)
         {
            // using the array of certificates
            rval = findArrayCertificates(
               entryCertificates, mCertificates, 
               mRequireAllCertificatesToVerify);
         }
         else if(mAliasToCertificateChain != null)
         {
            // using the map of alias -> certificate chains
            rval = findMappedCertificates(entryCertificates);
         }
         else
         {
            // any certificate will do
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Verifies the signature of an input stream with jar data.
    * 
    * @param is the input stream with jar data.
    * 
    * @exception IOException thrown if an IO error occurs.
    * @exception SecurityException thrown if the jar's signature cannot be
    *                              verified.
    *                              
    * @return true if the jar was signed, or false if it was not.
    */
   public boolean verifyJar(InputStream is)
   throws IOException, SecurityException
   {
      boolean rval = false;
      
      // create a jar input stream
      JarInputStream jis = new JarInputStream(is, true);
      
      // set to true if it is detected that any of the jar entries are
      // missing a signature
      boolean signatureMissing = false;
      
      // a buffer for reading in jar entries
      byte[] buffer = new byte[2048];
      
      // iterate through all jar entries -- according to JarInputStream
      // documentation, the signature will be verified for each entry --
      // however, we must verify that each entry has a digest in the
      // manifest or else no verification will take place and no exceptions
      // will be thrown (the entry will be treated as unsigned and valid)
      JarEntry jarEntry = null;
      while((jarEntry = jis.getNextJarEntry()) != null && !signatureMissing)
      {
         if(!jarEntry.isDirectory())
         {
            // get the name of the entry in upper case
            String name = jarEntry.getName().toUpperCase(Locale.ENGLISH);
            
            // see if the jar entry is not a signature block or file
            if(!SignatureFileVerifier.isBlockOrSF(name))
            {
               // read the entry all the way through so that the
               // certificates can be checked -- the end of the
               // stream will be at the end of the entry
               while(jis.read(buffer) != -1);
               
               // get the code signers for the jar entry
               CodeSigner[] codeSigners = jarEntry.getCodeSigners();
               if(codeSigners != null)
               {
                  // get the list of certificates for the code signers
                  ArrayList<Certificate> list = new ArrayList<Certificate>();
                  for(int n = 0; n < codeSigners.length; n++)
                  {
                     // iterate through the certificates and add them to
                     // the certificates list
                     for(Iterator<? extends Certificate> i =
                         codeSigners[n].getSignerCertPath().
                         getCertificates().iterator(); i.hasNext();)
                     {
                        list.add(i.next());
                     }
                  }
                  
                  // build an array of entry certificates
                  Certificate[] entryCertificates =
                     new Certificate[list.size()];
                  for(int i = 0; i < list.size(); i++)
                  {
                     entryCertificates[i] = list.get(i);
                  }
               
                  // see if the certificates for the entry match those
                  // provided to this verifier -- if we can't find
                  // the appropriate matches, then we're missing a signature
                  signatureMissing = !findCertificates(entryCertificates);
               }
               else
               {
                  // code signers are missing!
                  signatureMissing = true;
               }
            }
         }
      }
      
      // if no signatures were missing then the jar was signed
      rval = !signatureMissing;
      
      return rval;
   }
   
   /**
    * Verifies the signature of a jar file.
    * 
    * @param file the file with jar data.
    * 
    * @exception IOException thrown if an IO error occurs.
    * @exception SecurityException thrown if the jar's signature cannot be
    *                              verified.
    *                              
    * @return true if the jar was signed, or false if it was not.
    */
   public boolean verifyJar(File file) throws IOException, SecurityException
   {
      boolean rval = false;
      
      FileInputStream fis = new FileInputStream(file);
      rval = verifyJar(fis);
      fis.close();
      
      return rval;
   }
}
