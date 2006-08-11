/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.autoupdater.*;
import com.db.autoupdater.basic.*;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

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
      CustomAutoUpdaterLoader loader = new CustomAutoUpdaterLoader();
      
      // run AutoUpdater
      runAutoUpdater(loader, args);
   }
   
   /**
    * Runs the test AutoUpdater with the AutoUpdateable application specified
    * in the configuration file with the given filename.
    * 
    * @param loader the auto-updater loader.
    * @param args the arguments to start the application with.
    */
   public static void runAutoUpdater(
      AutoUpdaterLoader loader, String[] args)
   {
      System.out.println("Starting AutoUpdater...");

      AutoUpdater updater = null;
      boolean run = true;
      while(run)
      {
         // load the auto-updater as necessary
         if(updater == null)
         {
            updater = loader.loadAutoUpdater();
         }
      
         if(updater != null)
         {
            // run the auto-updateable application
            run = updater.runAutoUpdateable(args);
            
            // if the updater needs a new loader, don't run again
            if(updater.requiresNewLoader())
            {
               run = false;
            }

            // clean up the auto-updater if it needs reloading
            if(updater.requiresReload())
            {
               updater = null;
               System.gc();
            }
         }
         else
         {
            System.out.println("Could not load CustomAutoUpdater!");
            break;
         }
      }
      
      System.out.println("AutoUpdater Finished.");
   }   
   
   /**
    * A simple CustomAutoUpdaterLoader.
    * 
    * @author Dave Longley
    */
   public static class CustomAutoUpdaterLoader implements AutoUpdaterLoader
   {
      /**
       * Loads a CustomAutoUpdater.
       * 
       * @return the loaded CustomAutoUpdater.
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
   public static class CustomAutoUpdater extends AbstractAutoUpdater
   {
      /**
       * Gets the configuration filename for the auto-updateable.
       * 
       * @return the configuration filename for the auto-updateable.
       */
      protected String getConfigFilename()   
      {
         return "customautoupdateable.cfg";
      }      
      
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
       * Validates this update script by performing whatever checks are
       * necessary.
       * 
       * @return true if this script has been validated and is ready to be
       *         processed, false if not.
       */
      public boolean validate()   
      {
         boolean rval = false;
         
         System.out.println("Validating Update Script...");
         
         rval = true;
         
         System.out.println("Update Script validated.");
         
         return rval;
      }
      
      /**
       * Processes this update script. Any call to this method should cause
       * cancelled() to return false until cancel() is called.
       * 
       * @return true if the script was processed, false if it was cancelled or
       *         encountered an error.
       */
      public boolean process()
      {
         boolean rval = false;
         
         System.out.println("Processing Update Script...");
         
         rval = true;
         
         System.out.println("Update Script Processed.");
         
         return rval;
      }
      
      /**
       * Cancels processing this update script. Any call to this method should
       * cause cancelled() to return true.
       */
      public void cancel()      
      {
         System.out.println("Cannot cancel.");
      }
      
      /**
       * Returns true if this script was cancelled, false if it was not. This
       * method should return false unless process() has been called followed
       * by a call to cancel(). Any subsequent call to process() should cause
       * this method to return true until cancel() is called.
       * 
       * @return true if this script has been cancelled since the last call
       *         to process().
       */
      public boolean cancelled()
      {
         // cannot cancel
         return false;
      }      
      
      /**
       * Reverts changes made by this script, if possible.
       * 
       * @return true if the revert was successful, false if not and the
       *         installation is now in an indetermine state.
       */
      public boolean revert()   
      {
         boolean rval = false;
         
         System.out.println("Reverting Update Script...");
         rval = true;
         
         return rval;
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
