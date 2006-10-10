/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

/**
 * An UpdateScript is a script that tells an AbstractAutoUpdater how to update
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
    * Processes this update script. Any call to this method should cause
    * cancelled() to return false until cancel() is called.
    * 
    * @return true if the script was processed, false if it was cancelled or
    *         encountered an error.
    */
   public boolean process();
   
   /**
    * Cancels processing this update script. Any call to this method should
    * cause cancelled() to return true.
    */
   public void cancel();
   
   /**
    * Returns true if this script was cancelled, false if it was not. This
    * method should return false unless process() has been called followed
    * by a call to cancel(). Any subsequent call to process() should cause
    * this method to return true until cancel() is called.
    * 
    * @return true if this script has been cancelled since the last call
    *         to process().
    */
   public boolean cancelled();
   
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
   
   /**
    * Returns true if the entire AutoUpdate process should be shutdown,
    * false if not.
    * 
    * @return true if the entire AutoUpdateProcess should be shutdown,
    *         false if not.
    */
   public boolean shutdownRequired();
}
