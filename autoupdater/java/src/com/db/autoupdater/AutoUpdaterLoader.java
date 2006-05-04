/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import com.db.common.ConfigFile;
import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

/**
 * This class is used to load an AutoUpdater.
 * 
 * @author Dave Longley
 */
public abstract class AutoUpdaterLoader
{
   /**
    * Creates a new AutoUpdaterLoader.
    */
   public AutoUpdaterLoader()
   {
   }
   
   /**
    * Loads an AutoUpdater.
    * 
    * @return the loaded AutoUpdater.
    */
   public abstract AutoUpdater loadAutoUpdater();
   
   /**
    * Starts an AutoUpdater that runs the AutoUpdateable application specified
    * in the configuration file with the given filename.
    * 
    * @param configFilename the name of the configuration file that specifies
    *                       the AutoUpdateable application to load.
    * @param args the arguments to start the application with.
    */
   public void startAutoUpdater(String configFilename, String[] args)
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
                  // run the application
                  updater.run(application, args);
                  
                  // run the application again if the AutoUpdater doesn't
                  // require a reload and the application should restart
                  run = !updater.requiresReload() &&
                         application.shouldRestart(); 
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
    * Gets the logger for this AutoUpdater.
    * 
    * @return the logger for this AutoUpdater.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }   
}
