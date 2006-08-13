/*
 * Copyright (c) 2003-2006 Digital Bazar, Inc.  All rights reserved.
 */
import java.io.File;

import com.db.crypto.*;

/**
 * Runs a unit test on Cryptor.
 * 
 * @author Dave Longley
 */
public class UTCryptor
{
   /**
    * Runs the unit test.
    * 
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         File file = new File("test.mp3");
         String md5 = Cryptor.getMD5ChecksumString(file);
         String sha1 = Cryptor.getSHA1ChecksumString(file);
         
         System.out.println("size: " + file.length());
         System.out.println("md5: " + md5);
         System.out.println("sha1: " + sha1);

         System.out.println("encrypted:" +
                            Cryptor.encrypt(
                               "oqwjer2938rnosdakjf982y39ir3ld", "changeit"));
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
      
      
      /*System.out.println("\nTesting Cryptor...");
      System.out.println("------------------\n");
      
      System.out.println("Testing passwords that encrypt passwords!");

      // create the password
      String password = "chicken";

      System.out.println("The password to encrypt is: " + password);
      System.out.println("The password to encrypt the password with is: " +
                         "\"ThateChicken\"");
      
      System.out.println("Encrypting...");

      password = Cryptor.encrypt(password, "THateChicken");

      System.out.println("The encrypted password is: " + password);

      System.out.println("Attempting to decrypt with wrong password: " +
                         "\"lol!!1!\"");
      System.out.println("Decrypting...");

      String garbage = Cryptor.decrypt(password, "lol!!1!");

      if(garbage == null)
      {
         System.out.println("The password is not correct! But we expected " +
                            "that, so its good.");
      }
      else
      {
         System.out.println("This is not good, the wrong password decrypted " +
                            "the password!");
         System.out.println("FAILURE");
         System.exit(1);
      }

      System.out.println("Attempting to decrypt with correct password: " +
                         "\"THateChicken\"");
      System.out.println("Decrypting...");

      password = Cryptor.decrypt(password, "THateChicken");

      if(password == null)
      {
         System.out.println("It didn't work!");
         System.out.println("FAILURE");
         System.exit(1);
      }

      System.out.println("The decrypted password is: " + password);

      if(password.equals("chicken"))
         System.out.println("SUCCESS");
      else
         System.out.println("FAILURE");*/
   }
}
