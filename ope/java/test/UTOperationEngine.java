/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.LoggerManager;
import com.db.logging.Logger;

/**
 * A unit test for OperationEngine.
 * 
 * @author Dave Longley
 */
public class UTOperationEngine
{
   /**
    * Runs the OperationEngine test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbope", "ut-operationengine.log", false);
         LoggerManager.setFileVerbosity("dbope", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbope", Logger.ERROR_VERBOSITY);
         
         // FIXME: implement me
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
