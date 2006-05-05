/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An UpdateScript is a script that tells an AutoUpdater how to update
 * an application.
 * 
 * @author Dave Longley
 */
public interface UpdateScript
{
   /**
    * Validates this update script by performing whatever checks are
    * necessary.
    * 
    * @return true if this script has been validated and is ready to be
    *         processed, false if not.
    */
   public boolean validate();   

   /**
    * Processes this update script.
    */
   public void process();
   
   /**
    * Returns true if the AutoUpdater that processed this script requires
    * a reload, false if not.
    * 
    * @return true if the AutoUpdater that processed this script requires
    *         a reload, false if not.
    */
   public boolean autoUpdaterRequiresReload();
}
