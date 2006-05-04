/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An UpdateScriptSource is a source for an UpdateScript. It provides a
 * script that an AutoUpdater can use to update an application.
 * 
 * @author Dave Longley
 */
public interface UpdateScriptSource
{
   /**
    * Checks this source for an update script.
    * 
    * @param application the auto-updateable application the update script is
    *                    for.
    *                    
    * @return true if this source has an update script.
    */
   public boolean hasUpdateScript(AutoUpdateable application);
   
   /**
    * Gets the update script for an auto-updateable application.
    *
    * @param application the auto-updateable application the update script
    *                    is for.
    *                    
    * @return the update script.
    */
   public UpdateScript getUpdateScript(AutoUpdateable application);
   
   /**
    * Validates the update script by performing whatever checks are necessary.
    * 
    * This method could potentially fire an event that triggers a GUI to
    * ask the user whether or not the update script should be processed. 
    *
    * @param script the update script to validate.
    * 
    * @return true if the script has been validated and is ready to be
    *         processed, false if not.
    */
   public boolean validateUpdateScript(UpdateScript script);
}
