/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.autoupdater.*;
import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

/**
 * This class is used to test the Digital Bazaar AutoUpdater framework. 
 * 
 * @author Dave Longley
 */
public class UTAutoUpdater
{
   /**
    * Runs the test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcommon", "ut-autoupdater.log", false);
      LoggerManager.setFile("dbautoupdater", "ut-autoupdater.log", true);
      LoggerManager.setConsoleVerbosity(
         "dbcommon", Logger.ERROR_VERBOSITY);
      LoggerManager.setConsoleVerbosity(
         "dbautoupdater", Logger.ERROR_VERBOSITY);
      
      // create AutoUpdaterLoader
      CustomAutoUpdaterLoader loader =
         new CustomAutoUpdaterLoader();
      
      System.out.println("Starting AutoUpdater...");
      
      // run AutoUpdater
      loader.runAutoUpdater("customautoupdateable.cfg", args);
      
      System.out.println("AutoUpdater Finished.");
   }
   
   /**
    * A simple CustomAutoUpdaterLoader.
    * 
    * @author Dave Longley
    */
   public static class CustomAutoUpdaterLoader extends AutoUpdaterLoader
   {
      /**
       * Loads an AutoUpdater.
       * 
       * @return the loaded AutoUpdater.
       */
      public AutoUpdater loadAutoUpdater()
      {
         AutoUpdater rval = null;
         
         System.out.println("Loading CustomAutoUpdater...");
         
         rval = new CustomAutoUpdater();
         
         return rval;
      }
   }
   
   /**
    * A simple CustomAutoUpdater.
    *
    * @author Dave Longley
    */
   public static class CustomAutoUpdater extends AutoUpdater
   {
      /**
       * Gets the UpdateScriptSource for this AutoUpdater. 
       * 
       * @return the UpdateScriptSource for this AutoUpdater.
       */
      public UpdateScriptSource getUpdateScriptSource()
      {
         UpdateScriptSource rval = null;
         
         System.out.println("Getting Update Script Source...");
         
         rval = new CustomUpdateScriptSource();
         
         return rval;
      }
      
      /**
       * Validates the update script by performing whatever checks are
       * necessary.
       * 
       * This method could potentially fire an event that triggers a GUI to
       * ask the user whether or not the update script should be processed. 
       *
       * @param script the update script to validate.
       * 
       * @return true if the script has been validated and is ready to be
       *         processed, false if not.
       */
      public boolean validateUpdateScript(UpdateScript script)
      {
         boolean rval = false;
         
         System.out.println("Validating Update Script...");
         
         rval = true;
         
         System.out.println("Update Script validated.");
         
         return rval;
      }
   }
   
   /**
    * A simple CustomUpdateScriptSource.
    * 
    * @author Dave Longley
    */
   public static class CustomUpdateScriptSource implements UpdateScriptSource
   {
      /**
       * Checks this source for an update script.
       * 
       * @param application the auto-updateable application the update script
       *                    is for.
       *                    
       * @return true if this source has an update script.
       */
      public boolean hasUpdateScript(AutoUpdateable application)
      {
         boolean rval = false;
         
         System.out.println("Checking for update...");
         
         rval = true;
         
         return rval;
      }
      
      /**
       * Gets the update script for an auto-updateable application.
       *
       * @param application the auto-updateable application the update script
       *                    is for.
       *                    
       * @return the update script.
       */
      public UpdateScript getUpdateScript(AutoUpdateable application)      
      {
         UpdateScript rval = null;
         
         System.out.println("Getting Update Script...");
         
         rval = new CustomUpdateScript();
         
         return rval;
      }
   }
   
   /**
    * A simple CustomUpdateScript.
    * 
    * @author Dave Longley
    */
   public static class CustomUpdateScript implements UpdateScript
   {
      /**
       * Processes this update script.
       */
      public void process()
      {
         System.out.println("Processing Update Script...");
         
         System.out.println("Update Script Processed.");
      }
      
      /**
       * Returns true if the AutoUpdater that processed this script requires
       * a reload, false if not.
       * 
       * @return true if the AutoUpdater that processed this script requires
       *         a reload, false if not.
       */
      public boolean autoUpdaterRequiresReload()
      {
         boolean rval = false;
         
         rval = true;
         
         return rval;
      }
   }
}
