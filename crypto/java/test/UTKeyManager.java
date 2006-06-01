/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.util.*;
import com.db.crypto.*;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;
import java.security.SignatureException;

//import sun.misc.BASE64Encoder;
//import sun.misc.BASE64Decoder;

public class UTKeyManager
{
   public static final String mData = "chicken";
   
   public static void main(String[] args)
   {
      System.out.println("\nTesting KeyManager...");
      System.out.println("------------------------\n");

      System.out.println("Generating keys...");

      KeyManager km = new KeyManager();
      boolean generated = km.generateKeyPair();

      System.out.println("Keys generated: " + generated);
      
      /*String publicKey =
         "MIIBuDCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZp\n" + 
         "RV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoDgYUAAoGBAOtjMoYgSVC251Xe2XV6l2zfC2QnztEJH9qv/aZ4zNM4E334UAFlKuxj6KxXBsYalgWxmIN4106RD8+9YI8GBoAUhX7P05NeFsnKP6x3j7SgeF2ngL0n0+0ffUVT3rFxnfpy3x6tgMrpSNG3/o/gdl8o8UPklPV3MOAW0lbHR+3T";
      */
      /*String publicKey =
         "MIIBuDCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZp" + 
         "RV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoDgYUAAoGBAOtjMoYgSVC251Xe2XV6l2zfC2QnztEJH9qv/aZ4zNM4E334UAFlKuxj6KxXBsYalgWxmIN4106RD8+9YI8GBoAUhX7P05NeFsnKP6x3j7SgeF2ngL0n0+0ffUVT3rFxnfpy3x6tgMrpSNG3/o/gdl8o8UPklPV3MOAW0lbHR+3T";
      
      if(km.loadPrivateKey("username.profile", "password"))
      {
         System.out.println("loaded key");
         
         PrivateKey pk = km.getPrivateKey();
         String privateKey = km.getPrivateKeyString();
         
         byte[] signed1 = Cryptor.sign("chicken", pk);
         byte[] signed2 = Cryptor.sign("chicken", privateKey);
         
         System.out.println("SIGNATURE 1: " + Cryptor.verify(signed1, "chicken", publicKey));
         System.out.println("SIGNATURE 2: " + Cryptor.verify(signed2, "chicken", publicKey));
         
      }*/
      
      
      
      
      if(!generated)
         System.exit(1);

      System.out.println();
      System.out.println("Key pair:");
      System.out.println();
      System.out.println("Private Key:");
      String prkey = km.getPrivateKeyString();
      System.out.println("\"" + prkey + "\"");
      System.out.println();
      System.out.println("Public Key:");
      String pukey = km.getPublicKeyString();
      System.out.println("\"" + pukey + "\"");
      System.out.println();
      
      // test sign with cryptor
      System.out.println("Signing with strings using cryptor: ");
      byte[] data = Cryptor.sign("text", prkey);
      System.out.println("Signature: " + data);
      System.out.println("Verified: " + Cryptor.verify(data, "text", pukey));
      

      // store the keys
      System.out.println("Storing keys...");

      km.storePrivateKey("private.key", "password");
      km.storePublicKey("public.key");

      System.out.println("Keys stored on disk...");

      // load keys from disk
      System.out.println("Loading keys from disk...");
      
      km.loadPrivateKey("private.key", "password");
      km.loadPublicKey("public.key");

      System.out.println("Keys loaded from disk...");

      // get the private key
      PrivateKey privateKey = km.getPrivateKey();

      // get the public key in base64 encoded string format
      String publicKey = km.getPublicKeyString();

      System.out.println("Data to sign: " + mData);
      
      System.out.println("Signing with private key... ");

      String sig = "";

      try
      {
         // sign the data with the private key
         Signature signature =
            Signature.getInstance(privateKey.getAlgorithm());
         
         signature.initSign(privateKey);
         signature.update(mData.getBytes());

         // base64 encode the signature
         //BASE64Encoder encoder = new BASE64Encoder();
         //sig = encoder.encode(signature.sign());
         Base64Coder encoder = new Base64Coder();
         sig = encoder.encode(signature.sign());
      }
      catch(InvalidKeyException ike)
      {
         System.out.println("FAILURE: could not sign data");
         ike.printStackTrace();
         System.exit(1);
      }
      catch(NoSuchAlgorithmException nsae)
      {
         System.out.println("FAILURE: could not sign data");
         nsae.printStackTrace();
         System.exit(1);
      }
      catch(SignatureException se)
      {
         System.out.println("FAILURE: could not sign data");
         se.printStackTrace();
         System.exit(1);
      }

      System.out.println("Verifying data with public key...");

      try
      {
         // base64 decode the signature
         //BASE64Decoder decoder = new BASE64Decoder();
         //byte[] bytes = decoder.decodeBuffer(sig);
         Base64Coder decoder = new Base64Coder();
         byte[] bytes = decoder.decode(sig);

         // decode the public key
         PublicKey key = KeyManager.decodePublicKey(publicKey);
         
         // verify the digital signature
         Signature signature = Signature.getInstance(key.getAlgorithm());
         signature.initVerify(key);
         signature.update(mData.getBytes());
         if(signature.verify(bytes))
            System.out.println("SUCCESS: The data has been verified!");
         else
            System.out.println("FAILURE: The data was not verified!");
         
         // try out PrivateKeyCryptor
         PrivateKeyCryptor pkc = new PrivateKeyCryptor("test.pkey", "password");
         
         if(pkc.generateKeys())
         {
            System.out.println("PKeyCryptor generated keys...");
            
            System.out.println("PUBLIC KEY: " + pkc.getPublicKeyString());
            System.out.println();
            String s1 = pkc.getPrivateKeyString(); 
            System.out.println("PRIVATE KEY: " + pkc.getPrivateKeyString());
            
            System.out.println();
            System.out.println("Clearing key and password...");
            pkc.clear();
            
            System.out.println("EXPECTING ERRORS...");
            System.out.println("SHOULD be null: " + pkc.getPrivateKeyString());
            
            System.out.println("Setting password...");
            pkc.setPassword("password");
            
            String s2 = pkc.getPrivateKeyString();
            System.out.println("SHOULD WORK: " + s2);
            System.out.println();
            
            if(s1.equals(s2))
            {
               System.out.println("SUCCESS: private keys match!");
            }
            else
            {
               System.out.println("FAILURE: private keys do not match!");
            }
         }
         else
         {
            System.out.println("FAILURE: Could not generate keys!");
         }
      }
      catch(InvalidKeyException ike)
      {
         System.out.println("FAILURE: could not verify data");
         ike.printStackTrace();
         System.exit(1);
      }
      catch(NoSuchAlgorithmException nsae)
      {
         System.out.println("FAILURE: could not verify data");
         nsae.printStackTrace();
         System.exit(1);
      }
      catch(SignatureException se)
      {
         System.out.println("FAILURE: could not verify data");
         se.printStackTrace();
         System.exit(1);
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
