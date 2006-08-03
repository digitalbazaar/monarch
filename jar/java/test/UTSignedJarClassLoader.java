/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.File;
import java.security.CodeSigner;

import com.db.jar.*;

/**
 * A unit test for testing a SignedJarClassLoader.
 * 
 * @author Dave Longley
 */
public class UTSignedJarClassLoader
{
   /**
    * Runs the SignedJarClassLoader.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // the name of the jar to verify
         String jarName = "data/test.jar";
         
         // create code signers
         //CodeSigner signer1 = new CodeSigner();
         
         // create a signed jar verifier
         SignedJarVerifier verifier = new SignedJarVerifier();
         
         // create a SignedJarClassLoader
         SignedJarClassLoader loader =
            new SignedJarClassLoader(jarName, verifier);
         
         System.out.println("jars verified.");
         
         /*
         // create a file for the test jar
         File file = new File(jarName);
         
         // verify a jar
         boolean signed = verifier.verifyJar(file);
         
         // print out whether or not the jar was signed
         System.out.println("Jar signed: " + signed);*/
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
