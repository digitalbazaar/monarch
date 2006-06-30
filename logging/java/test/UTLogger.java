/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.*;

/**
 * This test tests the logger and logger manager.
 *
 * @author Dave Longley
 */
public class UTLogger
{
   /**
    * Runs the logger test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         System.out.println("\nTesting Logger...");
         System.out.println("------------------\n");
         
         // set up logger
         LoggerManager.setFile("logger1", "ut-logger.log", false);
         LoggerManager.setConsoleVerbosity("logger1", Logger.ERROR_VERBOSITY);
         //LoggerManager.setFileVerbosity("logger1", Logger.ERROR_VERBOSITY);

         LoggerManager.getLogger("logger1").error(
            UTLogger.class, "TEST LOGGER.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
