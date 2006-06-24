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
public abstract class Wizard
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
    * This event delegate is used to for wizard finished events.
    */
   protected EventDelegate mWizardFinishedEventDelegate;
   
   /**
    * This event delegate is used to for wizard cancelled events.
    */
   protected EventDelegate mWizardCancelledEventDelegate;
   
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
      
      // create the wizard task
      mTask = createTask();
      
      // create the wizard view
      mView = createView();
      
      // create and add the pages to this wizard
      createPages();
      
      // create the wizard finished event delegate
      mWizardFinishedEventDelegate = new EventDelegate();
      
      // create the wizard cancelled event delegate
      mWizardCancelledEventDelegate = new EventDelegate();
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
         page.activatePage();
         
         // display the page in the view
         getView().displayPage(page);
      }
   }
   
   /**
    * Fires a wizard finished event.
    */
   public void fireWizardFinished()
   {
      // create event
      EventObject event = new EventObject("wizardCompleted");
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
      event.setData("task", getTask());
      event.setDataKeyMessage("task", "The WizardTask that was cancelled.");
      
      // fire event
      getWizardCancelledEventDelegate().fireEvent(event);
   }
   
   /**
    * Displays the next page in this wizard.
    */
   public void displayNextPage()
   {
      // navigate to the next page
      WizardPage page = getPageNavigator().nextPage();
      
      // display the page
      displayPage(page);
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
    * Checks the errors on the current page.
    * 
    * @return true if there are no errors, false if not.
    */
   public boolean checkCurrentPageForErrors() 
   {
      boolean rval = false;
      
      WizardPage page = getPageNavigator().getCurrentPage();
      if(page != null)
      {
         rval = page.checkErrors();
      }
      
      return rval;
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
    * Cancels this wizard.
    * 
    * If this method is overridden to handle wizard-specific cancellation,
    * this method should be called at the end of the overriding method.
    */
   public void cancelWizard()
   {
      // fire cancelled event
      fireWizardCancelled();
   }
   
   /**
    * Finishes this wizard.
    * 
    * If this method is overridden to handle wizard-specific cancellation,
    * this method should be called at the end of the overriding method.
    */
   public void finishWizard()
   {
      // fire finished event
      fireWizardFinished();
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
}
