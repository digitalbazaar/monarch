/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import javax.crypto.SecretKey;

import com.db.crypto.Cryptor;
import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.stream.AesStreamCryptor;
import com.db.stream.DESStreamCryptor;
import com.db.stream.StreamCryptor;
import com.db.stream.TripleDesStreamCryptor;
import com.db.util.Base64Coder;

/**
 * A unit test for the StreamCryptor class (and extending classes).
 * 
 * @author Dave Longley
 */
public class UTStreamCryptor
{
   /**
    * Runs the StreamCryptor test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbstream", "ut-streamcryptor.log", false);
         LoggerManager.setFile("dbcrypto", "ut-streamcryptor.log");
         LoggerManager.setFileVerbosity("dbstream", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbstream", Logger.ERROR_VERBOSITY);
         
         String src = "test/data/test.mp3";
         
         String desEncrypted = "test/data/des-encrypted.tmp";
         String desDecrypted = "test/data/des-decrypted.mp3";
         
         String tripleDesEncrypted = "test/data/des3-encrypted.tmp";
         String tripleDesDecrypted = "test/data/des3-decrypted.mp3";

         String aesEncrypted = "test/data/aes-encrypted.tmp";
         String aesDecrypted = "test/data/aes-decrypted.mp3";
         
         StreamCryptor sc;
         long time;
         
         Base64Coder base64 = new Base64Coder();
         
         SecretKey desKey = Cryptor.generateKey("DES");
         String desKeyStr = base64.encode(desKey.getEncoded());
         
         SecretKey tripleDesKey = Cryptor.generateKey("DESede");
         String tripleDesKeyStr = base64.encode(tripleDesKey.getEncoded());
         
         SecretKey aesKey = Cryptor.generateKey("AES");
         String aesKeyStr = base64.encode(aesKey.getEncoded());
         
         // DES TEST:
         
         // create a DES stream cryptor
         sc = new DESStreamCryptor(64, desKeyStr);
         
         // encrypt file
         time = System.currentTimeMillis();
         sc.encrypt(src, desEncrypted);
         System.out.println(
            "DES encrypt complete,time=" +
            (System.currentTimeMillis() - time));
         
         // decrypt file
         time = System.currentTimeMillis();
         sc.decrypt(desEncrypted, desDecrypted);
         System.out.println(
            "DES decrypt complete,time=" +
            (System.currentTimeMillis() - time));
         
         // TRIPLE DES TEST:
         
         // create a TripleDES stream cryptor
         sc = new TripleDesStreamCryptor(64, tripleDesKeyStr);
         
         // encrypt file
         time = System.currentTimeMillis();
         sc.encrypt(src, tripleDesEncrypted);
         System.out.println(
            "TripleDES encrypt complete,time=" +
            (System.currentTimeMillis() - time));
         
         // decrypt file
         time = System.currentTimeMillis();
         sc.decrypt(tripleDesEncrypted, tripleDesDecrypted);
         System.out.println(
            "TripleDES decrypt complete,time=" +
            (System.currentTimeMillis() - time));
         
         // AES TEST:
         
         // create a AES stream cryptor
         sc = new AesStreamCryptor(64, aesKeyStr);
         
         // encrypt file
         time = System.currentTimeMillis();
         sc.encrypt(src, aesEncrypted);
         System.out.println(
            "AES encrypt complete,time=" +
            (System.currentTimeMillis() - time));
         
         // decrypt file
         time = System.currentTimeMillis();
         sc.decrypt(aesEncrypted, aesDecrypted);
         System.out.println(
            "AES decrypt complete,time=" +
            (System.currentTimeMillis() - time));
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
