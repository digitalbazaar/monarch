/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.jar;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;

/**
 * A SignedJarClassLoader is a class loader that will only load classes
 * from signed jars.
 * 
 * @author Dave Longley
 */
public class SignedJarClassLoader extends URLClassLoader
{
   /**
    * The signed jar verifier used to verify jar files.
    */
   protected SignedJarVerifier mVerifier;
   
   /**
    * Creates a new SignedJarClassLoader with the system class loader
    * as its parent class loader.
    * 
    * @param jarFilename the name of the jar file to load.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jar.
    * @exception SecurityException thrown if any of the jar file fails
    *                              signature verification.
    */
   public SignedJarClassLoader(String jarFilename, SignedJarVerifier verifier)
   throws IOException, SecurityException
   {
      this(new URL[]{new File(jarFilename).toURL()},
        ClassLoader.getSystemClassLoader(), verifier);
   }
   
   /**
    * Creates a new SignedJarClassLoader with the specified parent class loader.
    * 
    * @param jarFilename the name of the jar file to load.
    * @param parent the parent class loader to use.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jar.
    * @exception SecurityException thrown if any of the jar file fails
    *                              signature verification.
    */
   public SignedJarClassLoader(
      String jarFilename, ClassLoader parent, SignedJarVerifier verifier)
   throws IOException, SecurityException
   {
      this(new URL[]{new File(jarFilename).toURL()}, parent, verifier);
   }
   
   /**
    * Creates a new SignedJarClassLoader with the system class loader
    * as its parent class loader.
    * 
    * @param jarFile the file with the jar.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jar.
    * @exception SecurityException thrown if any of the jar file fails
    *                              signature verification.
    */
   public SignedJarClassLoader(File jarFile, SignedJarVerifier verifier)
   throws IOException, SecurityException
   {
      this(new URL[]{jarFile.toURL()},
         ClassLoader.getSystemClassLoader(), verifier);
   }
   
   /**
    * Creates a new SignedJarClassLoader with the specified parent class loader.
    * 
    * @param jarFile the file with the jar.
    * @param parent the parent class loader to use.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jar.
    * @exception SecurityException thrown if any of the jar file fails
    *                              signature verification.
    */
   public SignedJarClassLoader(
      File jarFile, ClassLoader parent, SignedJarVerifier verifier)
   throws IOException, SecurityException
   {
      this(new URL[]{jarFile.toURL()}, parent, verifier);
   }   
   
   /**
    * Creates a new SignedJarClassLoader with the system class loader
    * as its parent class loader.
    * 
    * @param urls the urls that point at signed jar files to load.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IllegalArgumentException thrown if any of the passed URLs do
    *                                     not point at a jar file.
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jars.
    * @exception SecurityException thrown if any of the jar files fail
    *                              signature verification.
    */
   public SignedJarClassLoader(URL[] urls, SignedJarVerifier verifier)
   throws IllegalArgumentException, IOException, SecurityException
   {
      this(urls, ClassLoader.getSystemClassLoader(), verifier);
   }
   
   /**
    * Creates a new SignedJarClassLoader with the specified parent class loader.
    *
    * @param urls the urls that point at signed jar files to load.
    * @param parent the parent class loader to use.
    * @param verifier the signed jar verifier to use.
    * 
    * @exception IllegalArgumentException thrown if any of the passed URLs do
    *                                     not point at a jar file.
    * @exception IOException thrown if there is an IO error while reading
    *                        or verifying the jars.
    * @exception SecurityException thrown if any of the jar files fail
    *                              signature verification.
    */
   public SignedJarClassLoader(
      URL[] urls, ClassLoader parent, SignedJarVerifier verifier)
   throws IllegalArgumentException, IOException, SecurityException
   {
      super(urls, parent);
      
      // verify that the urls point to jar files
      if(!checkJarUrls(urls))
      {
         throw new IllegalArgumentException(
            "URLs must point to jars! " +
            "Only 'jar' and 'file' protocols are acceptable.");
      }
      
      // set the signed jar verifier
      setVerifier(verifier);
      
      // verify the jars that the urls point to
      verifyJarsAtURLs(urls);
   }
   
   /**
    * Checks to see if a set of urls all point to jar files.
    * 
    * @param urls the urls to check.
    * 
    * @return true if the urls all point to jar files, false if not.
    */
   protected boolean checkJarUrls(URL[] urls)
   {
      boolean rval = true;
      
      for(int i = 0; i < urls.length && rval; i++)
      {
         // we only need an extra check if the url doesn't use the jar protocol
         if(!urls[i].getProtocol().equals("jar"))
         {
            // FUTURE CODE: we may want to change this in the future to acquire
            // the content at the URL to see if it is jar content
            if(urls[i].getProtocol().equals("file"))
            {
               // ensure that the path ends in "jar"
               rval = urls[i].getPath().endsWith("jar");
            }
            else if(!urls[i].getPath().endsWith("jar"))
            {
               // currently only "jar" and "file" protocols are acceptable
               rval = false;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Verifies the signatures of all of the jars at the given URLs.
    * 
    * @param urls the urls to check.
    * 
    * @exception IOException thrown if there is a IO error while reading
    *                        from the urls.
    * @exception SecurityException thrown if a jar's signature cannot be
    *                              verified.
    */
   protected void verifyJarsAtURLs(URL[] urls)
   throws IOException, SecurityException
   {
      for(int i = 0; i < urls.length; i++)
      {
         // open a url connection
         URLConnection connection = urls[i].openConnection();
         
         // get an input stream for reading the jar
         InputStream is = connection.getInputStream();
         
         // verify the jar data in input stream
         if(!getVerifier().verifyJar(is))
         {
            // jar not signed, throw security exception
            throw new SecurityException("Jar not signed!");
         }
      }
   }
   
   /**
    * Sets the signed jar verifier.
    * 
    * @param verifier the signed jar verifier to use.
    */
   protected void setVerifier(SignedJarVerifier verifier)
   {
      mVerifier = verifier;
   }

   /**
    * Gets the signed jar verifier. A default verifier will be created one
    * does not already exist.
    * 
    * @return the signed jar verifier.
    */
   protected SignedJarVerifier getVerifier()
   {
      if(mVerifier == null)
      {
         mVerifier = new SignedJarVerifier();
      }
      
      return mVerifier;
   }
}
