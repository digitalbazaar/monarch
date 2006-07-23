/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import com.db.autoupdater.AutoUpdateable;

/**
 * An UpdateScriptSource is a source for an UpdateScript. It provides a
 * script that an AbstractAutoUpdater can use to update an application.
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
}
