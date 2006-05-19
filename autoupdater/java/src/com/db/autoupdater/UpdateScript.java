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
    * 
    * @return true if the script was processed, false if it was cancelled or
    *         encountered an error.
    */
   public boolean process();
   
   /**
    * Cancels processing this update script.
    */
   public void cancel();
   
   /**
    * Reverts changes made by this script, if possible.
    * 
    * @return true if the revert was successful, false if not and the
    *         installation is now in an indetermine state.
    */
   public boolean revert();
   
   /**
    * Returns true if the AutoUpdater that processed this script requires
    * a reload, false if not.
    * 
    * @return true if the AutoUpdater that processed this script requires
    *         a reload, false if not.
    */
   public boolean autoUpdaterRequiresReload();
}
