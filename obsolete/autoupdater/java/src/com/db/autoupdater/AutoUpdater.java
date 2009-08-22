/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An interface that should be implemented by a class that automatically
 * updates an AutoUpdateable application.
 * 
 * Another class AbstractAutoUpdater in the com.db.autoupdater.basic
 * package provides a basic implementation for an AutoUpdater.
 * 
 * An AutoUpdater works in conjunction with and is loaded by an
 * AutoUpdaterLoader. 
 * 
 * @author Dave Longley
 */
public interface AutoUpdater
{
   /**
    * Runs the AutoUpdateable application. 
    * 
    * @param args the arguments to start the application with.
    *                       
    * @return true if the AutoUpdateable application should be restarted,
    *         false if not.
    */
   public boolean runAutoUpdateable(String[] args);
   
   /**
    * Passes arguments to a running AutoUpdateable application.
    * 
    * @param args the arguments to pass to a running AutoUpdateable application.
    */
   public void passArguments(String[] args);
   
   /**
    * Gets whether or not this AutoUpdater requires a reload.
    * 
    * This method should return true whenever this AutoUpdater must
    * be reloaded because its core libraries (i.e. classes/jars
    * necessary to load this AutoUpdater) have changed via an update but the
    * AutoUpdater can be reloaded.
    * 
    * @return true if this AutoUpdater requires a reload, false if not.
    */
   public boolean requiresReload();
   
   /**
    * Gets whether or not this AutoUpdater and the program that loaded it
    * should be completely shutdown.
    * 
    * @return true if a complete shutdown of the AutoUpdate process is required.  
    */
   public boolean requiresShutdown();
}
