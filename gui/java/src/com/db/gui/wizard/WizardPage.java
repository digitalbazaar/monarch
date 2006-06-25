/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.util.Vector;

/**
 * A WizardPage is a single page of a Wizard that is used to
 * accomplish some WizardTask. Each wizard page can validate the data it
 * contains and report any data validation issues. Each wizard page
 * can update its WizardTask.
 * 
 * @author Mike Johnson
 * @author Manu Sporny
 * @author Dave Longley
 */
public abstract class WizardPage
{
   /**
    * The WizardTask this page is designed to work on. 
    */
   protected WizardTask mWizardTask;
   
   /**
    * The name of this wizard page.
    */
   protected String mName;

   /**
    * The errors in this wizard page.
    */
   protected Vector mErrors;
   
   /**
    * The view for this page.
    */
   protected WizardPageView mView;
   
   /**
    * Constructs a new WizardPage with the specified page name.
    *
    * @param name the wizard page name (used to uniquely identify this page).
    * @param task the wizard task this page will work on.
    */
   public WizardPage(String name, WizardTask task)
   {
      // set name
      mName = name;
      
      // store task
      mWizardTask = task;
      
      // init errors
      mErrors = new Vector();
      
      // create the view for this page
      mView = createView();
   }
   
   /**
    * Adds an error to this page.
    * 
    * @param error the error to add.
    */
   protected void addError(String error)
   {
      mErrors.add(error);
   }
   
   /**
    * Clears the errors on this page.
    */
   protected void clearErrors()
   {
      mErrors.clear();
   }
   
   /**
    * Creates the view for this page.
    * 
    * @return the view for this page.
    */
   protected abstract WizardPageView createView();
   
   /**
    * This method is called before displaying this page in a wizard.
    * 
    * This method can be used to activate whatever is necessary on a
    * WizardPage. It can used to clear data, look up data before display, etc.
    * 
    * The wizard's task is passed to this method for convenience.
    * 
    * @param task the WizardTask for this page.
    */
   public abstract void activatePage(WizardTask task);
      
   /**
    * Checks all of the data that a wizard page contains for errors.
    * 
    * This method is called before writing the data to the WizardTask.
    * 
    * The wizard's task is passed to this method for convenience.
    *  
    * @param task the WizardTask for this page.
    * 
    * @return true if the page is valid, false otherwise.
    */
   public abstract boolean validate(WizardTask task);
   
   /**
    * Updates the WizardTask with the data on this page.
    * 
    * This method is called after validate() and before proceeding to the
    * next step in a wizard.
    * 
    * The wizard's task is passed to this method for convenience.
    *
    * @param task the WizardTask to update. 
    */
   public abstract void updateWizardTask(WizardTask task);
   
   /**
    * Gets the name of this page.
    *
    * @return the name of this page.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the WizardTask for this page.
    * 
    * @return task the wizard task for this page.
    */
   public WizardTask getWizardTask()
   {
      return mWizardTask;
   }
   
   /**
    * Retrieves the errors if any exist for this wizard page.
    * 
    * @return an empty vector of Strings if there are no errors, or a 
    *         vector of error strings.
    */
   public Vector getErrors()
   {
      return mErrors;
   }
   
   /**
    * Gets the view for this page.
    * 
    * @return the view for this page.
    */
   public WizardPageView getView()
   {
      return mView;
   }
}
