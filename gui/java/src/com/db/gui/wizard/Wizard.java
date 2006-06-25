/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import com.db.event.EventDelegate;
import com.db.event.EventObject;

/**
 * A Wizard is a device used to accomplish some task via a collection
 * of wizard pages. 
 * 
 * @author Dave Longley
 */
public abstract class Wizard implements WizardPageSelector
{
   /**
    * The task for this wizard.
    */
   protected WizardTask mTask;
   
   /**
    * The wizard page pool for this wizard.
    */
   protected WizardPagePool mPagePool;
   
   /**
    * The wizard page navigator for this wizard.
    */
   protected WizardPageNavigator mPageNavigator;
   
   /**
    * The view for this wizard.
    */
   protected WizardView mView;
   
   /**
    * Set to true when a wizard is running. A wizard is running once its
    * start() method is called. A wizard can only be finished or cancelled
    * when it is running. A wizard can only be started when it is not
    * running. Once a wizard is finished or cancelled, it is no
    * longer running.
    */
   protected boolean mRunning;
   
   /**
    * This event delegate is used to for wizard started events.
    */
   protected EventDelegate mWizardStartedEventDelegate;

   /**
    * This event delegate is used to for wizard finished events.
    */
   protected EventDelegate mWizardFinishedEventDelegate;
   
   /**
    * This event delegate is used to for wizard cancelled events.
    */
   protected EventDelegate mWizardCancelledEventDelegate;
   
   /**
    * This event delegate is used to for wizard page validation failed events.
    */
   protected EventDelegate mWizardPageValidationFailedEventDelegate;
   
   /**
    * Creates a new Wizard that uses itself as its WizardPageSelector.
    * 
    * The getNextWizardPage(WizardPageNavigator wpn) method should be
    * overridden to provide next page navigation.
    */
   public Wizard()
   {
      this(null);
      
      // set self as wizard page selector
      getPageNavigator().setPageSelector(this);
   }

   /**
    * Creates a new Wizard.
    * 
    * @param pageSelector the wizard page selector used to select the
    *                     next page to display from the pool of pages.
    */
   public Wizard(WizardPageSelector pageSelector)
   {
      // create the wizard page pool
      mPagePool = new WizardPagePool();
      
      // create the wizard page navigator
      mPageNavigator = new WizardPageNavigator(mPagePool, pageSelector);
      
      // create the wizard started event delegate
      mWizardStartedEventDelegate = new EventDelegate();

      // create the wizard finished event delegate
      mWizardFinishedEventDelegate = new EventDelegate();
      
      // create the wizard cancelled event delegate
      mWizardCancelledEventDelegate = new EventDelegate();
      
      // create the wizard page validation failed event delegate
      mWizardPageValidationFailedEventDelegate = new EventDelegate();

      // create the wizard task
      mTask = createTask();
      
      // create the wizard view
      mView = createView();
      
      // create and add the pages to this wizard
      createPages();
   }
   
   /**
    * Sets whether or not this wizard is running.
    * 
    * @param running true to set this wizard's state to running, false to
    *                set it to not running.
    */
   protected void setRunning(boolean running)
   {
      mRunning = running;      
   }
   
   /**
    * Gets the page navigator.
    * 
    * @return the page navigator.
    */
   protected WizardPageNavigator getPageNavigator()
   {
      return mPageNavigator;
   }
   
   /**
    * Adds a page to this wizard. The last page added to this wizard via
    * this method will be set as the final page.
    * 
    * @param page the page to add to this wizard.
    */
   protected void addPage(WizardPage page)
   {
      // add page to the page pool
      mPagePool.addPage(page);
      
      // add the page to the view
      getView().addPage(page);
      
      // make added page the final page by default
      setFinalPage(page);
      
      // set first page if none is set yet
      if(getPageNavigator().getFirstPage() == null)
      {
         getPageNavigator().setFirstPage(page);
      }
   }
   
   /**
    * Removes a page from this wizard.
    * 
    * @param page the page to remove from this wizard.
    */
   protected void removePage(WizardPage page)
   {
      // remove page from the page pool
      mPagePool.removePage(page);
      
      // if the current page is the passed page, go to the previous page
      if(getPageNavigator().getCurrentPage() == page)
      {
         getPageNavigator().previousPage();
      }
   }
   
   /**
    * Sets the final page in this wizard. This method will not add the
    * passed page to the pool of pages, it will simply mark it as the
    * final page.
    * 
    * @param page the final page in this wizard.
    */
   protected void setFinalPage(WizardPage page)
   {
      getPageNavigator().setFinalPage(page);
   }
   
   /**
    * Creates the task for this wizard.
    * 
    * @return the task for this wizard.
    */
   protected abstract WizardTask createTask();
   
   /**
    * Creates the view for this wizard.
    * 
    * @return the view for this wizard.
    */
   protected abstract WizardView createView();
   
   /**
    * Creates and adds the wizard pages to this wizard.
    */
   protected abstract void createPages();
   
   /**
    * Updates the WizardTask with the passed page.
    * 
    * @param page the page to update the WizardTask with.
    */
   protected void updateWizardTask(WizardPage page)
   {
      if(page != null)
      {
         // update the wizard task
         page.updateWizardTask(getTask());
      }
   }
   
   /**
    * Displays the passed page in the view.
    * 
    * @param page the page to display.
    */
   protected void displayPage(WizardPage page)
   {
      if(page != null)
      {
         // notify view of whether or not the first page is being displayed
         getView().displayingFirstPage(onFirstPage());
         
         // notify view of whether or not the final page is being displayed
         getView().displayingFinalPage(onFinalPage());
         
         // activate the page
         page.activatePage(getTask());
         
         // display the page in the view
         getView().displayPage(page);
      }
   }
   
   /**
    * Fires a wizard started event.
    */
   public void fireWizardStarted()
   {
      // create event
      EventObject event = new EventObject("wizardStarted");
      event.setData("wizard", getTask());
      event.setDataKeyMessage("wizard", "The Wizard that started.");
      event.setData("task", getTask());
      event.setDataKeyMessage("task", "The WizardTask to be performed.");
      
      // fire event
      getWizardStartedEventDelegate().fireEvent(event);
   }
   
   /**
    * Fires a wizard finished event.
    */
   public void fireWizardFinished()
   {
      // create event
      EventObject event = new EventObject("wizardFinished");
      event.setData("wizard", getTask());
      event.setDataKeyMessage("wizard", "The Wizard that finished.");
      event.setData("task", getTask());
      event.setDataKeyMessage("task", "The WizardTask that was performed.");
      
      // fire event
      getWizardFinishedEventDelegate().fireEvent(event);
   }
   
   /**
    * Fires a wizard cancelled event.
    */
   public void fireWizardCancelled()
   {
      // create event
      EventObject event = new EventObject("wizardCancelled");
      event.setData("wizard", getTask());
      event.setDataKeyMessage("wizard", "The Wizard that was cancelled.");
      event.setData("task", getTask());
      event.setDataKeyMessage("task", "The WizardTask that was cancelled.");
      
      // fire event
      getWizardCancelledEventDelegate().fireEvent(event);
   }
   
   /**
    * Fires a wizard page validation failed event.
    * 
    * @param page the current page.
    */
   public void fireWizardPageValidationFailed(WizardPage page)
   {
      // create event
      EventObject event = new EventObject("wizardPageValidationFailed");
      event.setData("wizard", getTask());
      event.setDataKeyMessage("wizard",
         "The Wizard whose page failed validation.");
      event.setData("task", getTask());
      event.setDataKeyMessage("task", "The current WizardTask.");
      event.setData("page", page);
      event.setDataKeyMessage("page", 
         "The WizardPage that failed validation.");
      
      // fire event
      getWizardPageValidationFailedEventDelegate().fireEvent(event);
   }
   
   /**
    * This method is provided for overriding convenience. If a class
    * that extends a Wizard wishes to handle its own wizard page
    * selection, it can override this method and call the default
    * constructor (Wizard()) when instantiating to use this class as
    * a WizardPageSelector.
    * 
    * Gets the next wizard page for the given wizard page navigator.
    * 
    * @param wpn the wizard page navigator to get the next wizard page for.
    *
    * @return the next wizard page.
    */
   public WizardPage getNextWizardPage(WizardPageNavigator wpn)
   {
      return null;
   }
   
   /**
    * Checks to see if the current page is valid.
    * 
    * @return true if the current page is valid, false if not.
    */
   public boolean validateCurrentPage()
   {
      boolean rval = false;
      
      WizardPage page = getPageNavigator().getCurrentPage();
      if(page != null)
      {
         // try to validate page
         if(page.validate(getTask()))
         {
            // validation successful
            rval = true;
         }
         else
         {
            // fire validation failed event
            fireWizardPageValidationFailed(page);
         }
      }
      
      return rval;
   }
   
   /**
    * Displays the first page in this wizard.
    */
   public void displayFirstPage()
   {
      // navigate to the first page
      WizardPage page = getPageNavigator().firstPage();
      
      // display the page
      displayPage(page);
   }

   /**
    * Displays the next page in this wizard if the current page passes
    * validation.
    */
   public void displayNextPage()
   {
      // validate current page
      if(validateCurrentPage())
      {
         // update the wizard task with the current page
         updateWizardTask(getPageNavigator().getCurrentPage());
      
         // navigate to the next page
         WizardPage page = getPageNavigator().nextPage();
      
         // display the page
         displayPage(page);
      }
   }
   
   /**
    * Displays the previous page in this wizard.
    */
   public void displayPreviousPage()
   {
      // navigate to the next page
      WizardPage page = getPageNavigator().previousPage();
      
      // display the page
      displayPage(page);
   }
   
   /**
    * Gets the current page displayed by this wizard.
    *
    * @return the current page displayed by this wizard.
    */
   public WizardPage getCurrentPage()
   {
      return getPageNavigator().getCurrentPage();
   }
   
   /**
    * Returns true if the current page is the first page, false if not.
    * 
    * @return true if the current page is the first page, false if not.
    */
   public boolean onFirstPage()
   {
      return getPageNavigator().onFirstPage();      
   }

   /**
    * Returns true if the current page is the final page, false if not.
    * 
    * @return true if the current page is the final page, false if not.
    */
   public boolean onFinalPage()
   {
      return getPageNavigator().onFinalPage();      
   }
   
   /**
    * Starts this wizard.
    */
   public void startWizard()
   {
      // only start wizard if it is not already running
      if(!isRunning())
      {
         // wizard is now running
         setRunning(true);
         
         // start the task
         getTask().start();
      
         // display the first page
         displayFirstPage();
         
         // fire started event
         fireWizardStarted();
      }
   }
   
   /**
    * Finishes this wizard.
    */
   public void finishWizard()
   {
      // only finish wizard if this wizard is running
      if(isRunning())
      {
         // validate current page
         if(validateCurrentPage())
         {
            // update the wizard task with the current page
            updateWizardTask(getPageNavigator().getCurrentPage());
         
            // finish the task
            getTask().finish();
            
            // wizard is no longer running
            setRunning(false);
         
            // fire finished event
            fireWizardFinished();
         }
      }
   }
   
   /**
    * Cancels this wizard.
    */
   public void cancelWizard()
   {
      // only cancel wizard if this wizard is running
      if(isRunning())
      {
         // cancel the task
         getTask().cancel();
         
         // wizard is no longer running
         setRunning(false);
         
         // fire cancelled event
         fireWizardCancelled();
      }
   }
   
   /**
    * Gets whether or not this wizard is running. A wizard is running once it
    * has been started and it has not been finished or cancelled. 
    * 
    * @return true if this wizard is running, false if not.
    */
   public boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Gets the task for this wizard.
    * 
    * @return the task for this wizard.
    */
   public WizardTask getTask()
   {
      return mTask;
   }
   
   /**
    * Gets the view for this wizard.
    * 
    * @return the view for this wizard.
    */
   public WizardView getView()
   {
      return mView;
   }
   
   /**
    * Gets the wizard started event delegate.
    * 
    * @return the wizard started event delegate.
    */
   public EventDelegate getWizardStartedEventDelegate()
   {
      return mWizardStartedEventDelegate;
   }

   /**
    * Gets the wizard finished event delegate.
    * 
    * @return the wizard finished event delegate.
    */
   public EventDelegate getWizardFinishedEventDelegate()
   {
      return mWizardFinishedEventDelegate;
   }
   
   /**
    * Gets the wizard cancelled event delegate.
    * 
    * @return the wizard cancelled event delegate.
    */
   public EventDelegate getWizardCancelledEventDelegate()
   {
      return mWizardCancelledEventDelegate;
   }
   
   /**
    * Gets the wizard page validation failed event delegate.
    * 
    * @return the wizard page validation failed event delegate.
    */
   public EventDelegate getWizardPageValidationFailedEventDelegate()
   {
      return mWizardPageValidationFailedEventDelegate;
   }
}
