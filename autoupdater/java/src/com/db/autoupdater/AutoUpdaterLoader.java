/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.ConfigFile;

/**
 * This class is used to load an AutoUpdater.
 * 
 * @author Dave Longley
 */
public abstract class AutoUpdaterLoader
{
   /**
    * The currently running AutoUpdateable application.
    */
   protected AutoUpdateable mRunningAutoUpdateable;
   
   /**
    * Creates a new AutoUpdaterLoader.
    */
   public AutoUpdaterLoader()
   {
      // no auto-updateable application is presently running
      setRunningAutoUpdateable(null);
   }
   
   /**
    * Sets the currently running AutoUpdateable application.
    * 
    * @param application the auto-updateable application that is currently
    *                    running.
    */
   protected void setRunningAutoUpdateable(AutoUpdateable application)
   {
      mRunningAutoUpdateable = application;
   }
   
   /**
    * Gets the currently running AutoUpdateable application.
    * 
    * @return the currently running auto-updateable application, or null
    *         if none is running.
    */
   protected AutoUpdateable getRunningAutoUpdateable()
   {
      return mRunningAutoUpdateable;
   }
   
   /**
    * Loads an AutoUpdater.
    * 
    * @return the loaded AutoUpdater.
    */
   public abstract AutoUpdater loadAutoUpdater();
   
   /**
    * Runs an AutoUpdater that runs the AutoUpdateable application specified
    * in the configuration file with the given filename.
    * 
    * @param configFilename the name of the configuration file that specifies
    *                       the AutoUpdateable application to load.
    * @param args the arguments to start the application with.
    */
   public void runAutoUpdater(String configFilename, String[] args)
   {
      boolean run = true;
      while(run)
      {
         // load AutoUpdater
         AutoUpdater updater = loadAutoUpdater();
         if(updater != null)
         {
            // get the AutoUpdateable application's config file
            ConfigFile configFile = new ConfigFile(configFilename);
            if(configFile.read())
            {
               // load the AutoUpdateable application
               AutoUpdateable application =
                  updater.loadAutoUpdateable(configFile);
               if(application != null)
               {
                  // process the arguments
                  application.processArguments(args);
                  
                  // set application as currently running
                  setRunningAutoUpdateable(application);
                  
                  // run the application
                  updater.run(application);
                  
                  // remove application as currently running
                  setRunningAutoUpdateable(null);
                  
                  // run the application again if the AutoUpdater doesn't
                  // require a reload and the application should restart
                  run = !updater.requiresReload() &&
                         application.shouldRestart();
                  
                  // clean up AutoUpdater and AutoUpdateable application
                  updater = null;
                  application = null;
                  System.gc();
               }
               else
               {
                  run = false;
                  getLogger().error(
                     "Could not load AutoUpdateable application!");
               }
            }
            else
            {
               run = false;
               getLogger().error(
                  "Could not read AutoUpdateable configuration file!");
            }
         }
         else
         {
            run = false;
            getLogger().error("Could not load AutoUpdater!");
         }
      }
   }
   
   /**
    * Passes arguments to a running AutoUpdateable application.
    * 
    * @param args the arguments to pass to a running AutoUpdateable application.
    */
   public void passArguments(String[] args)
   {
      // get the currently running AutoUpdateable application
      AutoUpdateable application = getRunningAutoUpdateable();
      if(application != null)
      {
         // process arguments
         application.processArguments(args);
      }
   }
   
   /**
    * Gets the logger for this AutoUpdater.
    * 
    * @return the logger for this AutoUpdater.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }   
}
