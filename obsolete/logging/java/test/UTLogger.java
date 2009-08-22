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
         LoggerManager.setFile("logger2", "ut-logger.log", true);
         LoggerManager.setFile("logger3", "ut-logger.log", true);
         LoggerManager.setConsoleVerbosity("logger1", Logger.ERROR_VERBOSITY);
         LoggerManager.setConsoleVerbosity("logger3", Logger.ERROR_VERBOSITY);
         //LoggerManager.setFileVerbosity("logger1", Logger.ERROR_VERBOSITY);
         LoggerManager.getLogger("logger1").setMaxFileSize(126);
         LoggerManager.getLogger("logger2").setMaxFileSize(126);
         LoggerManager.getLogger("logger3").setMaxFileSize(126);
         
         for(int i = 0; i < 200; i++)
         {
            LoggerManager.getLogger("logger1").error(
               UTLogger.class, "TEST LOGGER1.");
            LoggerManager.getLogger("logger2").error(
               UTLogger.class, "TEST LOGGER2.");
            LoggerManager.getLogger("logger3").error(
               UTLogger.class, "TEST LOGGER3.");
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
