/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.util.StringTools;

/**
 * A unit test for StringTools.
 * 
 * @author Dave Longley
 */
public class UTStringTools
{
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
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
