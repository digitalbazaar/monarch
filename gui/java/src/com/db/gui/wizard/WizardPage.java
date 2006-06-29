/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.util.Vector;

import com.db.event.EventDelegate;
import com.db.event.EventObject;

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
    * This event delegate is used for validation failed events.
    */
   protected EventDelegate mValidationPassedEventDelegate;
   
   /**
    * This event delegate is used for validation failed events.
    */
   protected EventDelegate mValidationFailedEventDelegate;
   
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
      
      // create errors container
      mErrors = new Vector();
      
      // create the validation passed event delegate
      mValidationPassedEventDelegate = new EventDelegate();

      // create the validation failed event delegate
      mValidationFailedEventDelegate = new EventDelegate();
      
      // the view for this page will be created when getView() is first called
   }
   
   /**
    * Fires a validation passed event.
    */
   protected void fireValidationPassed()
   {
      // create event
      EventObject event = new EventObject("validationPassed");
      event.setData("page", this);
      event.setDataKeyMessage("page", 
         "The WizardPage that passed validation.");
      event.setData("task", getWizardTask());
      event.setDataKeyMessage("task", "The WizardTask.");
      
      // fire event
      getValidationPassedEventDelegate().fireEvent(event);
   }

   /**
    * Fires a validation failed event.
    */
   protected void fireValidationFailed()
   {
      // create event
      EventObject event = new EventObject("validationFailed");
      event.setData("page", this);
      event.setDataKeyMessage("page", 
         "The WizardPage that failed validation.");
      event.setData("task", getWizardTask());
      event.setDataKeyMessage("task", "The WizardTask.");
      
      // fire event
      getValidationFailedEventDelegate().fireEvent(event);
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
    * This method can be used to activate whatever is necessary to update a
    * WizardPage. It can used to clear data, look up data before display, etc.
    * 
    * The wizard's task is passed to this method for convenience.
    * 
    * @param task the WizardTask for this page.
    */
   public void activate(WizardTask task)
   {
      // run page validation
      validate(task);
   }
      
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
    * Returns whether or not this page has errors.
    * 
    * @return whether or not this page has errors.
    */
   public boolean hasErrors()
   {
      return !getErrors().isEmpty();
   }
   
   /**
    * Gets the view for this page. If the view does not yet
    * exist, it will be created.
    * 
    * @return the view for this page.
    */
   public WizardPageView getView()
   {
      if(mView == null)
      {
         mView = createView();
      }
      
      return mView;
   }
   
   /**
    * Gets the validation passed event delegate.
    * 
    * @return the validation passed event delegate.
    */
   public EventDelegate getValidationPassedEventDelegate()
   {
      return mValidationPassedEventDelegate;
   }   

   /**
    * Gets the validation failed event delegate.
    * 
    * @return the validation failed event delegate.
    */
   public EventDelegate getValidationFailedEventDelegate()
   {
      return mValidationFailedEventDelegate;
   }   
}
