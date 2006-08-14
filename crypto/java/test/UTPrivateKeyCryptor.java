/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.crypto.*;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

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
      // set up logger
      LoggerManager.setFile("dbcrypto", "ut-privatekeycryptor.log", false);
      LoggerManager.setFileVerbosity("dbcrypto", Logger.DETAIL_VERBOSITY);
      LoggerManager.setConsoleVerbosity("dbcrypto", Logger.ERROR_VERBOSITY);
      
      System.out.println("\nTesting KeyManager...");
      System.out.println("------------------------\n");
      
      try
      {
         // try out PrivateKeyCryptor
         PrivateKeyCryptor pkc = new PrivateKeyCryptor("test.pem", "password");
         
         if(pkc.generateKeys())
         {
            System.out.println("PrivateKeyCryptor generated keys...");
            
            System.out.println("\nPUBLIC KEY:");
            System.out.println(pkc.getPEMPublicKey());
            System.out.println();
            System.out.println("\nPRIVATE KEY:");
            System.out.println(pkc.getPEMPrivateKey());
            System.out.println();

            // store private key string
            String s1 = pkc.getPrivateKeyString();
            
            System.out.println("Clearing key and password...");
            
            // clear private key cryptor
            pkc.clear();
            
            System.out.println(
               "Expecting an ERROR and private key to be null...");
            System.out.println("\nPRIVATE KEY:");
            System.out.println(pkc.getPrivateKeyString());
            
            System.out.println("Setting password...");
            pkc.setPassword("password");
            System.out.println("PASSWORD HAS BEEN SET");
            
            // get private key string
            String s2 = pkc.getPrivateKeyString();
            
            System.out.println("\nExpecting private key to be loaded again...");
            System.out.println("\nPRIVATE KEY:");
            System.out.println(s2);
            System.out.println();
            
            if(s1.equals(s2))
            {
               System.out.println("\nSUCCESS: private keys match!");
            }
            else
            {
               System.out.println("\nFAILURE: private keys do not match!");
            }
         }
         else
         {
            System.out.println("\nFAILURE: Could not generate keys!");
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
