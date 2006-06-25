/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

/**
 * A WizardBuilder is used to build a Wizard. This interface should
 * be implemented to build custom Wizards.
 * 
 * @author Dave Longley
 */
public interface WizardBuilder
{
   /**
    * Creates the task for a Wizard.
    * 
    * @return the task for a Wizard.
    */
   public WizardTask createTask();

   /**
    * Creates the page pool for a Wizard.
    * 
    * @param task the wizard task for the pages.
    * 
    * @return the page pool for a Wizard.
    */
   public WizardPagePool createPagePool(WizardTask task);
   
   /**
    * Creates the page selector for a Wizard.
    * 
    * @param pagePool the page pool for the wizard.
    * 
    * @return the page selector for a Wizard.
    */
   public WizardPageSelector createPageSelector(WizardPagePool pagePool);
   
   /**
    * Creates a new Wizard.
    * 
    * @return a new Wizard.
    */
   public Wizard createWizard();
}
