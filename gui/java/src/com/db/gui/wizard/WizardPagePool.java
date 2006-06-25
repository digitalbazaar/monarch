/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.util.HashMap;
import java.util.Iterator;

/**
 * A wizard page pool stores all of the available pages for a wizard.
 * 
 * @author Dave Longley
 */
public class WizardPagePool
{
   /**
    * Hashmap of WizardPage name to WizardPages.
    */
   protected HashMap mNameToWizardPage;

   /**
    * Creates a new wizard page pool.
    */
   public WizardPagePool()
   {
      // create name to wizard page map
      mNameToWizardPage = new HashMap();
   }

   /**
    * Appends a wizard page to this pool.
    * 
    * @param page the wizard page to add.
    */
   public void addPage(WizardPage page)
   {
      mNameToWizardPage.put(page.getName(), page);      
   }
   
   /**
    * Removes a wizard page from this pool.
    * 
    * @param page the wizard page to remove.
    */
   public void removePage(WizardPage page)
   {
      mNameToWizardPage.remove(page.getName());
   }
   
   /**
    * Gets the wizard page with the passed name.
    * 
    * @param name the name of the page.
    * 
    * @return the wizard page with the passed name or null if none exists.
    */
   public WizardPage getPage(String name)
   {
      return (WizardPage)mNameToWizardPage.get(name);
   }
   
   /**
    * Gets an iterator on the pages in this pool.
    * 
    * @return an iterator on the pages in this pool.
    */
   public Iterator iterator()
   {
      return mNameToWizardPage.values().iterator();
   }
}
