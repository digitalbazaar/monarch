/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.*;
import com.db.crypto.*;

import java.security.PrivateKey;
import java.security.PublicKey;

/**
 * A unit test for the KeyManager.
 * 
 * @author Dave Longley
 */
public class UTKeyManager
{
   /**
    * The data to sign/verify.
    */
   public static final String mData = "chicken";
   
   /**
    * Runs the test.
    * 
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcrypto", "ut-keymanager.log", false);
      LoggerManager.setFileVerbosity("dbcrypto", Logger.DETAIL_VERBOSITY);
      LoggerManager.setConsoleVerbosity("dbcrypto", Logger.ERROR_VERBOSITY);
      
      System.out.println("\nTesting KeyManager...");
      System.out.println("------------------------\n");

      System.out.println("Generating keys...");

      KeyManager km = new KeyManager();
      
      // use either "RSA" or "DSA"
      String algorithm = "DSA"; 
      
      // generate keys
      boolean generated = km.generateKeyPair(algorithm);

      System.out.println(algorithm + " Keys generated: " + generated);
      
      if(!generated)
      {
         System.exit(1);
      }

      System.out.println();
      System.out.println("Key pair:");
      System.out.println();
      System.out.println("Private Key (ASN.1 DER[PKCS#8]->Base64 = PEM):");
      System.out.println(km.getPEMPrivateKey());
      System.out.println();
      System.out.println("Public Key (ASN.1 DER[X.509]->Base64 = PEM):");
      System.out.println(km.getPEMPublicKey());
      System.out.println();

      String prkey = km.getPrivateKeyString();
      String pukey = km.getPublicKeyString();
      
      // test sign with cryptor
      System.out.println("Signing with strings using cryptor: ");
      byte[] data = Cryptor.sign("text", prkey);
      Base64Coder coder = new Base64Coder();
      System.out.println("Signature:'" + coder.encode(data) + "'");
      System.out.println("Verified: " + Cryptor.verify(data, "text", pukey));

      // store the keys
      System.out.println("Storing keys in DER-Base64 format...");

      km.storePrivateKey("private.key", "password");
      km.storePublicKey("public.key");

      System.out.println("Keys stored on disk.");
      
      // load keys from disk
      System.out.println("Loading keys from disk...");
      
      km.loadPrivateKeyFromFile("private.key", "password");
      km.loadPublicKeyFromFile("public.key");

      System.out.println("Keys loaded from disk:");
      System.out.println("Private Key (ASN.1 DER[PKCS#8]->Base64 = PEM):");
      System.out.println(km.getPEMPrivateKey());
      System.out.println();
      System.out.println("Public Key (ASN.1 DER[X.509]->Base64 = PEM):");
      System.out.println(km.getPEMPublicKey());
      System.out.println();
      
      // store the keys in PEM format
      System.out.println("Storing keys in PEM format...");

      km.storePEMPrivateKey("private.pem", "password");
      km.storePEMPublicKey("public.pem");
      
      // load keys from disk
      System.out.println("Loading PEM keys from disk...");
      
      km.loadPEMPrivateKeyFromFile("private.pem", "password");
      km.loadPEMPublicKeyFromFile("public.pem");

      System.out.println("Keys loaded from disk:");
      System.out.println("Private Key (ASN.1 DER[PKCS#8]->Base64 = PEM):");
      System.out.println(km.getPEMPrivateKey());
      System.out.println();
      System.out.println("Public Key (ASN.1 DER[X.509]->Base64 = PEM):");
      System.out.println(km.getPEMPublicKey());
      System.out.println();      

      // get the private key
      PrivateKey privateKey = km.getPrivateKey();

      // get the public key in base64 encoded string format
      String publicKey = km.getPublicKeyString();

      System.out.println("Data to sign: " + mData);
      
      System.out.println("Signing with private key... ");

      String sig = "";

      // sign the data with the private key
      // base64 encode the signature
      Base64Coder encoder = new Base64Coder();
      sig = encoder.encode(Cryptor.sign(mData, privateKey));

      System.out.println("Verifying data with public key...");

      try
      {
         // base64 decode the signature
         Base64Coder decoder = new Base64Coder();
         byte[] bytes = decoder.decode(sig);

         // decode the public key
         PublicKey key = KeyManager.decodePublicKey(publicKey);
         
         // verify the digital signature
         if(Cryptor.verify(bytes, mData, key))
         {
            System.out.println("SUCCESS: The data has been verified!");
         }
         else
         {
            System.out.println("FAILURE: The data was not verified!");
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
