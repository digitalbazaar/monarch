/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.net.URL;
import java.net.URLClassLoader;

import com.db.common.ConfigOptions;
import com.db.common.MethodInvoker;
import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

/**
 * An application automatic updater. Retrieves an update script from a
 * UpdateScriptSource and processes it to update an application.
 * 
 * @author Dave Longley
 */
public abstract class AutoUpdater
{
   /**
    * Set to true when this AutoUpdater requires a reload, false otherwise.
    */
   protected boolean mRequiresReload;
   
   /**
    * Creates a new AutoUpdater.
    */
   public AutoUpdater()
   {
   }
   
   /**
    * Sets whether or not this AutoUpdater requires a reload.
    * 
    * @param reload true if this AutoUpdater requires a reload, false if not.
    */
   protected void setRequiresReload(boolean reload)
   {
      mRequiresReload = reload;
   }
   
   /**
    * Checks for an update for the given application.
    * 
    * @param application the application to check for updates of.
    * 
    * @return true if an update was processed, false if not.
    */
   protected boolean checkForUpdate(AutoUpdateable application)
   {
      boolean rval = false;
      
      // get the update script source
      UpdateScriptSource source = getUpdateScriptSource();
      
      // check for an update script
      if(source.hasUpdateScript(application))
      {
         // get the update script
         UpdateScript script = source.getUpdateScript(application);
         
         // validate the script
         if(validateUpdateScript(script))
         {
            // shutdown the application
            application.shutdown();
            
            // process the script
            script.process();
            
            // set whether or not this AutoUpdater requires a reload
            setRequiresReload(script.autoUpdaterRequiresReload());
            
            // an update was processed
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Causes the update checker thread to pause for some period of time
    * before checking for an update.
    * 
    * Throws an InterruptedException if the thread is interrupted while
    * sleeping. 
    * 
    * @exception InterruptedException
    */
   public void pauseUpdateCheckerThread() throws InterruptedException
   {
      // check every 30 seconds
      Thread.sleep(30000);
   }
   
   /**
    * Continuously checks for an update for the given application.
    * 
    * @param application the application to check for updates of.
    */
   public void continuouslyCheckForUpdate(AutoUpdateable application)
   {
      try
      {
         // pause this thread
         pauseUpdateCheckerThread();
         
         // check for an update for the application
         checkForUpdate(application);
      }
      catch(InterruptedException e)
      {
         // interrupt thread
         Thread.currentThread().interrupt();
      }
   }
   
   /**
    * Loads an AutoUpdateable application from the passed configuration.
    * 
    * @param config the configuration to load an AutoUpdateable application
    *               from.
    *               
    * @return the AutoUpdateable application or null if one could not be loaded.
    */
   public AutoUpdateable loadAutoUpdateable(ConfigOptions config)
   {
      AutoUpdateable rval = null;
      
      try
      {
         // get the jar necessary to load the AutoUpdateable interface
         URL[] urls = new URL[1];
         urls[0] = new URL(config.getString("autoupdateable-jar"));

         // create a class loader for the AutoUpdateable
         ClassLoader classLoader = new URLClassLoader(urls);
         
         // load the AutoUpdateable
         Class c = classLoader.loadClass(
            config.getString("autoupdateable-class"));
         rval = (AutoUpdateable)c.newInstance();
      }
      catch(Throwable t)
      {
         getLogger().error(Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Runs an application while monitoring for updates in a background process.
    * 
    * This method returns true if the application has finished executing and
    * should be run again once updates have been installed, and false if
    * the application has finished executing and should not be run again, even
    * after updates have been installed.
    * 
    * @param application the auto-updateable application to execute.
    */
   public void run(AutoUpdateable application)
   {
      // check for an update, start the application if there isn't one
      if(!checkForUpdate(application))
      {
         // start the update checker thread
         Object[] params = new Object[]{application};
         MethodInvoker updateChecker =
            new MethodInvoker(this, "continuouslyCheckForUpdate", params);
         updateChecker.backgroundExecute();
         
         // execute application
         application.execute();
      
         try
         {
            // join the update checker thread
            updateChecker.join();
         }
         catch(InterruptedException e)
         {
            // interrupt threads
            updateChecker.interrupt();
            Thread.currentThread().interrupt();
         }
      }
   }
   
   /**
    * Gets whether or not this AutoUpdater requires a reload.
    * 
    * @return true if this AutoUpdater requires a reload, false if not.
    */
   public boolean requiresReload()
   {
      return mRequiresReload;
   }
   
   /**
    * Gets the UpdateScriptSource for this AutoUpdater. 
    * 
    * @return the UpdateScriptSource for this AutoUpdater.
    */
   public abstract UpdateScriptSource getUpdateScriptSource();
   
   /**
    * Validates the passed update script by performing whatever checks are
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
   public abstract boolean validateUpdateScript(UpdateScript script);
   
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
