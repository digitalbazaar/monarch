/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import javax.swing.event.ChangeListener;

/**
 * Any class that reports changes via a change listener can implement
 * this class.
 * 
 * @author Dave Longler.
 */
public interface ChangeReporter
{
   /**
    * Adds a change listener.
    * 
    * @param cl the change listener to add.
    */
   public void addChangeListener(ChangeListener cl);
   
   /**
    * Removes a change listener.
    * 
    * @param cl the change listener to remove.
    */
   public void removeChangeListener(ChangeListener cl);
}
