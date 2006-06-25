/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

/**
 * A WizardTask represents the task a wizard is performing. Individual
 * Wizards will each have their own task with specific methods. That task
 * should simply implement this interface.
 * 
 * @author Dave Longley
 */
public interface WizardTask
{
   /**
    * Starts this wizard task.
    */
   public void start();
   
   /**
    * Finishes this wizard task.
    */
   public void finish();
   
   /**
    * Cancels this wizard task.
    */
   public void cancel();
}
