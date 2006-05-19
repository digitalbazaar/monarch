/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.net.URL;
import java.net.URLClassLoader;

import com.db.common.ConfigOptions;
import com.db.common.EventDelegate;
import com.db.common.EventObject;
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
    * An event delegate for check for update started events.
    */
   protected EventDelegate mCheckForUpdateStartedEventDelegate;
   
   /**
    * An event delegate for update script found events.
    */
   protected EventDelegate mUpdateScriptFoundEventDelegate;
   
   /**
    * An event delegate for update script processed events.
    */
   protected EventDelegate mUpdateScriptProcessedEventDelegate;
   
   /**
    * An event delegate for update script not found events.
    */
   protected EventDelegate mUpdateScriptNotFoundEventDelegate;
   
   /**
    * Creates a new AutoUpdater.
    */
   public AutoUpdater()
   {
      // no reload required by default
      setRequiresReload(false);
      
      // create check for update started event delegate
      mCheckForUpdateStartedEventDelegate = new EventDelegate();

      // create update script found event delegate
      mUpdateScriptFoundEventDelegate = new EventDelegate();
      
      // create update script found event delegate
      mUpdateScriptProcessedEventDelegate = new EventDelegate();
      
      // create update script not found event delegate
      mUpdateScriptNotFoundEventDelegate = new EventDelegate();
   }
   
   /**
    * Fires a check for update started event.
    * 
    * @param event the event to fire.
    */
   protected void fireCheckForUpdateStartedEvent(EventObject event)
   {
      mCheckForUpdateStartedEventDelegate.fireEvent(event);
   }

   /**
    * Fires an update script found event.
    * 
    * @param event the event to fire.
    */
   protected void fireUpdateScriptFoundEvent(EventObject event)
   {
      mUpdateScriptFoundEventDelegate.fireEvent(event);
   }
   
   /**
    * Fires an update script processed event.
    * 
    * @param event the event to fire.
    */
   protected void fireUpdateScriptProcessedEvent(EventObject event)
   {
      mUpdateScriptProcessedEventDelegate.fireEvent(event);
   }
   
   /**
    * Fires an update script not found event.
    * 
    * @param event the event to fire.
    */
   protected void fireUpdateScriptNotFoundEvent(EventObject event)
   {
      mUpdateScriptNotFoundEventDelegate.fireEvent(event);
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
      
      // fire a check for update started event
      EventObject event = new EventObject("checkForUpdateStarted");
      fireCheckForUpdateStartedEvent(event);
      
      // get the update script source
      UpdateScriptSource source = getUpdateScriptSource();
      
      // check for an update script
      if(source.hasUpdateScript(application))
      {
         // get the update script
         UpdateScript script = source.getUpdateScript(application);
         
         // validate the script
         if(script.validate())
         {
            // fire event indicating that an update script has been found
            event = new EventObject("updateScriptFound");
            event.setData("updateScript", script);
            event.setData("processUpdate", false);
            fireUpdateScriptFoundEvent(event);
            
            // see if the update should be processed
            if(event.getDataBooleanValue("processUpdate"))
            {
               // process update script
               rval = true;
               processUpdateScript(application, script);
            }
         }
         else
         {
            // fire event indicating that an update script has not been found
            event = new EventObject("updateScriptNotFound");
            fireUpdateScriptNotFoundEvent(event);
         }
      }
      else
      {
         // fire event indicating that an update script has not been found
         event = new EventObject("updateScriptNotFound");
         fireUpdateScriptNotFoundEvent(event);
      }
      
      return rval;
   }
   
   /**
    * Shutsdown any running application and processes an update script.
    * 
    * @param application the application that is running.
    * @param script the update script to process.
    * 
    * @return true if an update was successfully processed without failure
    *         or cancellation, false if not.
    */
   protected boolean processUpdateScript(
      AutoUpdateable application, UpdateScript script)
   {
      boolean rval = false;
      
      // shutdown the application
      application.shutdown();
      
      // process the script
      if(!script.process())
      {
         // script processing was successful
         rval = true;
      }
      else
      {
         // script processing was cancelled or there was an error
         
         // attempt to revert script
         script.revert();
      }
      
      // set whether or not this AutoUpdater requires a reload
      setRequiresReload(script.autoUpdaterRequiresReload());
      
      // fire event indicating an update script was processed
      EventObject event = new EventObject("updateScriptProcessed");
      event.setData("updateScript", script);
      fireUpdateScriptProcessedEvent(event);
      
      return rval;
   }
   
   /**
    * This method is provided for convenience. It can be overloaded to
    * pause the current thread for some period of time. Another way to
    * pause between update checks is to handle the checkForUpdateStarted
    * event by pausing.
    * 
    * Causes the update checker thread to pause for some period of time
    * before checking for an update. The default period of time is
    * 30 seconds.
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
         while(!requiresReload())
         {
            // pause this thread
            pauseUpdateCheckerThread();
         
            // check for an update for the application
            if(!requiresReload() && !Thread.interrupted())
            {
               checkForUpdate(application);
            }
         }
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
         // get the jars necessary to load the AutoUpdateable interface
         String classPath = config.getString("autoupdateable-classpath");
         String[] split = classPath.split(",");
         
         URL[] urls = new URL[split.length];
         for(int i = 0; i < urls.length; i++)
         {
            urls[i] = new URL(split[i]);
         }

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
            updateChecker.interrupt();
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
    * Gets the check for update started event delegate.
    * 
    * @return the check for update started event delegate.
    */
   public EventDelegate getCheckForUpdateStartedEventDelegate()
   {
      return mCheckForUpdateStartedEventDelegate;
   }
   
   /**
    * Gets the update script found event delegate.
    * 
    * @return the update script found event delegate.
    */
   public EventDelegate getUpdateScriptFoundEventDelegate()
   {
      return mUpdateScriptFoundEventDelegate;
   }
   
   /**
    * Gets the update script processed event delegate.
    * 
    * @return the update script processed event delegate.
    */
   public EventDelegate getUpdateScriptProcessedEventDelegate()
   {
      return mUpdateScriptProcessedEventDelegate;
   }
   
   /**
    * Gets the update script not found event delegate.
    * 
    * @return the update script not found event delegate.
    */
   public EventDelegate getUpdateScriptNotFoundEventDelegate()
   {
      return mUpdateScriptNotFoundEventDelegate;
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
    * Gets the logger for this AutoUpdater.
    * 
    * @return the logger for this AutoUpdater.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
