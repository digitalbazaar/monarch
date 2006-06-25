/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.util.Vector;

/**
 * A WizardPage is a single page of a Wizard that is used to
 * accomplish some task. Each wizard page can validate the data it
 * contains and report any data validation issues.
 * 
 * @author Mike Johnson
 * @author Manu Sporny
 * @author Dave Longley
 */
public abstract class WizardPage
{
   /**
    * The wizard this page is for.
    */
   protected Wizard mWizard;
   
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
    * @param wizard the wizard this page is for.
    * @param name the wizard page name (used to uniquely identify this page). 
    */
   public WizardPage(Wizard wizard, String name)
   {
      // set wizard
      mWizard = wizard;
      
      // set name
      mName = name;
      
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
    * Gets access to the wizard task.
    * 
    * @return the wizard task.
    */
   protected WizardTask getWizardTask()
   {
      return getWizard().getTask();
   }
   
   /**
    * This method is called before displaying this page in a wizard.
    * 
    * This method can be used to activate whatever is necessary on a
    * WizardPage. It can used to clear data, look up data before display, etc.  
    * 
    * @param task the wizard's task.
    */
   public void activatePage(WizardTask task)
   {
      // default does nothing
   }
      
   /**
    * Checks all of the data that a wizard page contains for errors.
    * 
    * This method is called before writing the data to the WizardTask.
    *  
    * @param task the WizardTask.
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
    * @param task the WizardTask to update. 
    */
   public abstract void updateWizardTask(WizardTask task);
   
   /**
    * Gets the wizard this wizard page is for.
    * 
    * @return the wizard this wizard page is for.
    */
   public Wizard getWizard()
   {
      return mWizard;
   }
   
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
