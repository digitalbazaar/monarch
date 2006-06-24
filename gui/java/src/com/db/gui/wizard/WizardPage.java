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
    * Called whenever the page is activated by the wizard.
    */
   public void activatePage()
   {
      // default does nothing
   }
      
   /**
    * Checks all of the data that a wizard page contains for errors. This
    * method is called before proceeding to the next step in a wizard. 
    * 
    * @return true if the the page is valid, false otherwise.
    */
   public abstract boolean checkErrors();
   
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
