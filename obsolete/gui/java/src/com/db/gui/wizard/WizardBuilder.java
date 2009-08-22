/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

/**
 * A WizardBuilder is used to build a Wizard. This class should
 * be extended to build custom Wizards.
 * 
 * @author Dave Longley
 */
public abstract class WizardBuilder
{
   /**
    * Creates the task for a Wizard.
    * 
    * @return the task for a Wizard.
    */
   public abstract WizardTask createTask();

   /**
    * Creates the page pool for a Wizard.
    * 
    * @param task the wizard task for the pages.
    * 
    * @return the page pool for a Wizard.
    */
   public abstract WizardPagePool createPagePool(WizardTask task);
   
   /**
    * Creates the page selector for a Wizard.
    * 
    * @param pagePool the page pool for the wizard.
    * 
    * @return the page selector for a Wizard.
    */
   public abstract WizardPageSelector createPageSelector(
      WizardPagePool pagePool);
   
   /**
    * Creates a new Wizard.
    * 
    * @return a new Wizard.
    */
   public Wizard createWizard()      
   {
      // create a new task
      WizardTask task = createTask();
      
      // create a new page pool for the task
      WizardPagePool pagePool = createPagePool(task);
      
      // create a page selector
      WizardPageSelector pageSelector = createPageSelector(pagePool);
      
      // create wizard
      Wizard wizard = new Wizard(task, pagePool, pageSelector);
      
      // return wizard
      return wizard;
   }
}
