/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

/**
 * An interface that should be implemented by a class that automatically
 * updates an AutoUpdateable application.
 * 
 * It is intentionally empty -- this interface is only used to enable loading
 * an AutoUpdater (with a custom interface and implementation) via an
 * AutoUpdaterLoader.
 * 
 * Another class AbstractAutoUpdater provides a basic implementation for
 * an AutoUpdater. It works in conjuction with an AbstractAutoUpdaterLoader.
 * 
 * @author Dave Longley
 */
public interface AutoUpdater
{
}
