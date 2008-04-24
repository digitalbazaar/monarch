/*
 * Copyright (c) 2006-2008 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.util.StringTools;
import com.db.util.UniqueSet;

/**
 * A unit test for StringTools.
 * 
 * @author Dave Longley
 */
public class UTStringTools
{
   /**
    * Tests hexadecimal conversion.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public static void runConvertTest() throws IOException
   {
      System.out.println("\nTESTING StringTools.bytesToHex()/hexToBytes()");
      
      // convert to hex
      String data = "abcdefghiABCDEFGZXYW0123987{;}*%6,./.12`~";
      String original = data;
      
      String hex = StringTools.bytesToHex(data.getBytes("ASCII"));
      
      assert(hex.equals("616263646566676869414243444546475a585957303132333938377b3b7d2a25362c2e2f2e3132607e"));
      assert(hex.length() == 82);
      
      ByteArrayOutputStream baos = new ByteArrayOutputStream(hex.length() / 2);
      StringTools.hexToBytes(hex, baos);
      String ascii = baos.toString("ASCII");
      
      assert(ascii.equals(data));
      assert(ascii.equals(original));
      
      byte[] bytes;
      
      hex = "0";
      baos.reset();
      StringTools.hexToBytes(hex, baos);
      bytes = baos.toByteArray();
      assert(bytes.length == 1);
      assert(bytes[0] == 0x0);
      
      hex = "d";
      baos.reset();
      StringTools.hexToBytes(hex, baos);
      bytes = baos.toByteArray();
      assert(bytes.length == 1);
      assert(bytes[0] == 0xd);
      
      hex = "230f";
      baos.reset();
      StringTools.hexToBytes(hex, baos);
      bytes = baos.toByteArray();
      assert(bytes.length == 2);
      assert(bytes[0] == 0x23);
      assert(bytes[1] == 0x0f);
   }
   
   /**
    * Runs the StringTools test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbutil", "ut-stringtools.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
         
         String testStrings[] = new String[]
         {
            "1234,56723,1234,231321,31223123",
            "56723,1234,1234,123213,31223123",
            "1234,1234,1234,231312,321325,564512,1234",
            "1234,123456,1234567,1234",
            "123,1234,123456",
            "123,122,1234",
            "1234",
            "1241314,123441123421242411234"
         };
         
         System.out.println("\nTESTING StringTools.removeContentOnce()");
         
         for(int i = 0; i < testStrings.length; i++)
         {
            System.out.print("\"" + testStrings[i] + "\" -> ");
            System.out.println(
               "\"" +
               StringTools.removeContentOnce(testStrings[i], ",", "1234") +
               "\"");
         }
         
         System.out.println("\nTESTING StringTools.removeContent()");
         
         for(int i = 0; i < testStrings.length; i++)
         {
            System.out.println(
               "\"" + testStrings[i] + "\" -> \"" + 
               StringTools.removeContent(testStrings[i], ",", "1234") + "\"");
         }
         
         UniqueSet<String> set = new UniqueSet<String>();
         set.add("1");
         set.add("2");
         set.add("3");
         set.add("4");
         
         System.out.println(StringTools.glue(set, ","));
         
         runConvertTest();
         
         System.out.println("PASS");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
