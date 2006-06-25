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
    * Gets the first wizard page in a given wizard page pool.
    * 
    * @param pagePool the wizard page pool to get the first page in.
    * 
    * @return the first wizard page in the given pool.
    */
   public WizardPage getFirstPage(WizardPagePool pagePool);
   
   /**
    * Gets the next wizard page in a given wizard page pool, given the
    * current page.
    * 
    * @param pagePool the wizard page pool to get the next page in.
    * @param current the current wizard page or null if there no current page.
    *
    * @return the next wizard page.
    */
   public WizardPage getNextPage(WizardPagePool pagePool, WizardPage current);
   
   /**
    * Gets the final wizard page in a given wizard page pool.
    * 
    * @param pagePool the wizard page pool to get the final page in.
    * 
    * @return the final wizard page in the given pool.
    */
   public WizardPage getFinalPage(WizardPagePool pagePool);
}
