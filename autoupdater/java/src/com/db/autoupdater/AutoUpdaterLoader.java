/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An interface for an AutoUpdaterLoader. A class that implements this
 * interface can load an AutoUpdater. 
 * 
 * @author Dave Longley
 */
public interface AutoUpdaterLoader
{
   /**
    * Loads an AutoUpdater.
    * 
    * @return the loaded AutoUpdater.
    */
   public AutoUpdater loadAutoUpdater();
   
   /**
    * Runs an AutoUpdater that runs the AutoUpdateable application specified
    * in the configuration file with the given filename.
    * 
    * @param configFilename the name of the configuration file that specifies
    *                       the AutoUpdateable application to load.
    * @param args the arguments to start the application with.
    */
   public void runAutoUpdater(String configFilename, String[] args);
   
   /**
    * Passes arguments to a running AutoUpdateable application.
    * 
    * @param args the arguments to pass to a running AutoUpdateable application.
    */
   public void passArguments(String[] args);
}
