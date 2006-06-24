/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.util.Stack;

/**
 * A WizardPageNavigator is a class that is used to navigate a pool 
 * of wizard pages.
 * 
 * @author Dave Longley
 */
public class WizardPageNavigator
{
   /**
    * The pool of all available wizard pages to navigate over.
    */
   protected WizardPagePool mPagePool;
   
   /**
    * The wizard page selector used to select wizard pages.
    */
   protected WizardPageSelector mPageSelector;

   /**
    * This stack holds the history of steps in this wizard as wizard pages.
    * 
    * The top-most page on the stack is the current page.
    * 
    * The second top-most page is the previous page.
    */
   protected Stack mStepStack;
   
   /**
    * The current wizard page.
    */
   protected WizardPage mCurrentPage;
   
   /**
    * The final (last displayed) wizard page.
    */
   protected WizardPage mFinalPage;
   
   /**
    * Creates a new WizardPageNavigator with an empty pool of wizard pages
    * and no wizard page selector.
    */
   public WizardPageNavigator()
   {
      this(new WizardPagePool(), null);
   }

   /**
    * Creates a new WizardPageNavigator with the specified pool of
    * wizard pages and the specified wizard page selector.
    * 
    * @param pagePool the pool of wizard pages to navigate.
    * @param pageSelector the wizard page selector for selecting pages.
    */
   public WizardPageNavigator(
      WizardPagePool pagePool, WizardPageSelector pageSelector)
   {
      // store page pool
      mPagePool = pagePool;
      
      // set page selector
      setPageSelector(pageSelector);
      
      // create step stack
      mStepStack = new Stack();
      
      // no current page yet
      setCurrentPage(null);
      
      // no final page yet
      setFinalPage(null);
   }
   
   /**
    * Sets the current page.
    * 
    * @param page the new current page.
    */
   protected void setCurrentPage(WizardPage page) 
   {
      mCurrentPage = page;
   }
   
   /**
    * Adds a wizard page to the step stack.
    * 
    * @param page the page to add to the step stack.
    */
   protected void addPageStep(WizardPage page)
   {
      mStepStack.push(page);
   }
   
   /**
    * Removes a wizard page from the step stack and returns it.
    * 
    * @return the page from the top of the step stack or null if none exists.
    */
   protected WizardPage removePageStep()
   {
      WizardPage rval = null;
      
      if(!mStepStack.isEmpty())
      {
         rval = (WizardPage)mStepStack.pop();
      }
      
      return rval;
   }
   
   /**
    * Gets the current wizard page step from the step stack. 
    * 
    * @return the current wizard page step from the step stack or null
    *         if none exists.
    */
   protected WizardPage getPageStep()
   {
      WizardPage rval = null;
      
      if(!mStepStack.isEmpty())
      {
         rval = (WizardPage)mStepStack.peek();
      }
      
      return rval;
   }
   
   /**
    * Gets the wizard page pool.
    *
    * @return the pool of available wizard pages.
    */
   public WizardPagePool getPagePool()
   {
      return mPagePool;
   }
   
   /**
    * Sets the wizard page selector to use to select wizard pages.
    * 
    * @param wps the wizard page selector to use.
    */
   public void setPageSelector(WizardPageSelector wps)
   {
      mPageSelector = wps;
   }
   
   /**
    * Gets the wizard page selector that is used to select wizard pages.
    * 
    * @return the wizard page selector used to selected wizard pages.
    */
   public WizardPageSelector getPageSelector()
   {
      return mPageSelector;
   }
   
   /**
    * Navigates to the previous page and returns it.
    * 
    * @return the previous page (now current) -- or null if none exists.
    */
   public WizardPage previousPage()
   {
      // remove the current wizard page step
      removePageStep();
      
      // set the current page to the current step
      setCurrentPage(getPageStep());
      
      // return current page
      return getCurrentPage();
   }
   
   /**
    * Navigates to the next page and returns it.
    * 
    * @return the next page (now current) -- or null if none exists.
    */
   public WizardPage nextPage()
   {
      // set the current page
      setCurrentPage(getPageSelector().getNextWizardPage(this));
      
      // update the step stack
      if(getCurrentPage() != null)
      {
         // add a step
         addPageStep(getCurrentPage());
      }
      
      // return the current page
      return getCurrentPage();
   }

   /**
    * Gets the current wizard page.
    * 
    * @return the current wizard page.
    */
   public WizardPage getCurrentPage()
   {
      return mCurrentPage;
   }
   
   /**
    * Sets the final page in the wizard.
    * 
    * @param page the page to use as the final page.
    */
   public void setFinalPage(WizardPage page)
   {
      mFinalPage = page;
   }
   
   /**
    * Gets the final wizard page.
    * 
    * @return the final wizard page.
    */
   public WizardPage getFinalPage()
   {
      return mFinalPage;
   }
   
   /**
    * Returns true if the current page is the first page, false if not.
    * 
    * @return true if the current page is the first page, false if not.
    */
   public boolean onFirstPage()
   {
      return getStepCount() <= 1;
   }
   
   /**
    * Returns true if the current page is the final page, false if not.
    * 
    * @return true if the current page is the final page, false if not.
    */
   public boolean onFinalPage()
   {
      return getCurrentPage() == getFinalPage();
   }
   
   /**
    * Gets the step count. This is the number of wizard pages
    * that have been navigated to (including the current one).
    * 
    * @return the step count.
    */
   public int getStepCount()
   {
      return mStepStack.size();
   }
}
