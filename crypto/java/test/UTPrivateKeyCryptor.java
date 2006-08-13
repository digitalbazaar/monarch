/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.crypto.*;

/**
 * A unit test for the PrivateKeyCryptor.
 * 
 * @author Dave Longley
 */
public class UTPrivateKeyCryptor
{
   /**
    * Runs the test.
    * 
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      System.out.println("\nTesting KeyManager...");
      System.out.println("------------------------\n");
      
      try
      {
         // try out PrivateKeyCryptor
         PrivateKeyCryptor pkc =
            new PrivateKeyCryptor("test.pkey", "password");
         
         if(pkc.generateKeys())
         {
            System.out.println("PrivateKeyCryptor generated keys...");
            
            System.out.println("PUBLIC KEY:");
            System.out.println(pkc.getPEMPublicKey());
            System.out.println();
            String s1 = pkc.getPrivateKeyString(); 
            System.out.println("PRIVATE KEY:");
            System.out.println(pkc.getPEMPrivateKey());
            
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
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
