/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
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
         
         // create a signed jar verifier
         String[] aliases = new String[]{"test", "test2"};
         SignedJarVerifier verifier = new SignedJarVerifier(
            "data/test-keystore", "password", aliases, false);
         
         // create a SignedJarClassLoader
         SignedJarClassLoader loader =
            new SignedJarClassLoader(jarName, verifier);
         
         System.out.println("jars verified by " + loader.getClass());
         
         /*
         // create a file for the test jar
         java.io.File file = new java.io.File(jarName);
         
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
