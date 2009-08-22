/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An AutoUpdaterLoader is an interface that should be implemented
 * by a class that loads an AutoUpdater.
 * 
 * @author Dave Longley
 */
public interface AutoUpdaterLoader
{
   /**
    * Loads an AutoUpdater.
    * 
    * This method should dynamically load the libraries necessary to
    * load an AutoUpdater and then create and return the AutoUpdater.
    * 
    * @return the loaded AutoUpdater.
    */
   public AutoUpdater loadAutoUpdater();
}
