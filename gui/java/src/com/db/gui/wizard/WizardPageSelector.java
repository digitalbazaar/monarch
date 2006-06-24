/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

/**
 * A WizardPageSelector is a class that is used to select wizard
 * pages from a pool of pages.
 * 
 * @author Dave Longley
 */
public interface WizardPageSelector
{
   /**
    * Gets the next wizard page for the given wizard page navigator.
    * 
    * @param wpn the wizard page navigator to get the next wizard page for.
    *
    * @return the next wizard page.
    */
   public WizardPage getNextWizardPage(WizardPageNavigator wpn);
}
