/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.CardLayout;
import java.awt.Component;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextPane;

import com.db.event.EventObject;
import com.db.gui.LayeredLayout;

/**
 * A generic view for a Wizard.
 * 
 * @author Mike Johnson
 * @author Manu Sporny
 * @author Dave Longley
 */
public class WizardView extends JPanel implements ActionListener
{
   /**
    * The wizard this view represents.
    */
   protected Wizard mWizard;
   
   /**
    * The wizard description text displayed at the top of the wizard panel.
    */
   protected JTextPane mDescriptionTextPane;
   
   /**
    * The wizard panel which contains the wizard pages.
    */
   protected JPanel mWizardPagesPanel;
   
   /**
    * The wizard pages card layout.
    */
   protected CardLayout mWizardPagesLayout;
   
   /**
    * The wizard pages that this view can display.
    */
   protected Vector<WizardPage> mWizardPages;
   
   /**
    * The cancel button.
    */
   protected JButton mCancelButton;
   
   /**
    * The previous step button.
    */
   protected JButton mPrevButton;
   
   /**
    * The next step button.
    */
   protected JButton mNextButton;
   
   /**
    * The dialog used for displaying errors.
    */
   protected WizardErrorDialog mErrorDialog;

   /**
    * The quit confirmation dialog box.
    */
   protected WizardQuitDialog mQuitDialog;
   
   /**
    * Creates a wizard view.
    * 
    * @param wizard the wizard to create the view for.
    */
   public WizardView(Wizard wizard)
   {
      // store the wizard
      mWizard = wizard;
      
      // listen for page validation errors to display error dialog
      mWizard.getWizardPageValidationFailedEventDelegate().addListener(
         this, "displayErrorDialog");
      
      // create wizard pages list
      mWizardPages = new Vector<WizardPage>();
      
      // create error dialog
      mErrorDialog = new WizardErrorDialog(null);
      mErrorDialog.setModal(true);
      mErrorDialog.addActionListener(this);
      
      // create quit dialog
      mQuitDialog = new WizardQuitDialog(null);
      mQuitDialog.setModal(true);
      mQuitDialog.addActionListener(this);
      
      // setup the main panel
      setupPanel();
   }
   
   /**
    * Sets up the panel contents.
    */
   protected void setupPanel()
   {
      int[][] plot = {{0,0}, // wizard pages panel
                      {0,1}, {1,1}, {2,1}, {3,1}}; // glue & buttons
      LayeredLayout ll = new LayeredLayout(plot);
      setLayout(ll);
      setOpaque(false);
      
      // set up wizard pages panel
      mWizardPagesPanel = new JPanel();
      mWizardPagesPanel.setOpaque(false);
      mWizardPagesLayout = new CardLayout();
      mWizardPagesPanel.setLayout(mWizardPagesLayout);
      
      // create buttons for wizard navigation
      mCancelButton = new JButton("Cancel");
      mCancelButton.setActionCommand("cancelWizard");
      mCancelButton.addActionListener(this);
      
      // create glue to push buttons right
      Component glue = Box.createGlue();

      // create previous button
      mPrevButton = new JButton("< Previous");
      mPrevButton.setActionCommand("previousStep");
      mPrevButton.addActionListener(this);
      
      // disabled by default (should be on first wizard page)
      mPrevButton.setEnabled(false);

      // create next button
      mNextButton = new JButton("Next >");
      mNextButton.setActionCommand("nextStep");
      mNextButton.addActionListener(this);
      
      // place wizard pages panel
      ll.placeNext(mWizardPagesPanel, 1.0, 1.0, true, true,
         new Insets(5, 5, 0, 5));
      add(mWizardPagesPanel);

      // place cancel button
      ll.placeNext(mCancelButton, 0.0, 0.0, new Insets(5, 5, 5, 5));
      add(mCancelButton);

      // place glue
      ll.placeNext(glue, 1.0, 0.0, true, false);
      add(glue);
      
      // place previous button
      ll.placeNext(mPrevButton, 0.0, 0.0, new Insets(5, 5, 5, 5));
      add(mPrevButton);
      
      // place next button
      ll.placeNext(mNextButton, 0.0, 0.0, new Insets(5, 0, 5, 5));
      add(mNextButton);
   }
   
   /**
    * Sets the description that is displayed at the top of this wizard.
    * 
    * @param description the description text to set. The text may be marked
    *                    up with any HTML notation compliant with HTML 3.2.
    */
   public void setDescription(String description)
   {
      mDescriptionTextPane.setText(description);
   }
   
   /**
    * Adds a wizard page to this view.
    * 
    * @param page the wizard page to add.
    */
   public void addPage(WizardPage page)
   {
      // add the wizard page to the layout
      mWizardPagesLayout.addLayoutComponent(page.getView(), page.getName());
      
      // add the wizard page to the panel
      mWizardPagesPanel.add(page.getView(), page.getName());
      
      // add the wizard page to the list of pages
      mWizardPages.add(page);
      
      // listen to the page for validation changes
      page.getValidationPassedEventDelegate().addListener(
         this, "wizardPageValidationPassed");
      page.getValidationFailedEventDelegate().addListener(
         this, "wizardPageValidationFailed");
      page.getValidationPendingEventDelegate().addListener(
         this, "wizardPageValidationPending");
   }
   
   /**
    * Removes a wizard page from this view.
    * 
    * @param page the wizard page to remove.
    */
   public void removePage(WizardPage page)
   {
      // remove the wizard page from the layout
      mWizardPagesLayout.removeLayoutComponent(page.getView());
      
      // remove the wizard page from the panel
      mWizardPagesPanel.remove(page.getView());
      
      // remove the wizard page from the list of pages
      mWizardPages.remove(page);
   }
   
   /**
    * Enables or disables displaying the final page of the wizard.
    * 
    * @param firstPage true if the wizard is on the first page, false if not.
    */
   public void displayingFirstPage(boolean firstPage)
   {
      // enable or disable the "previous" button
      mPrevButton.setEnabled(!firstPage);
   }
   
   /**
    * Enables or disables displaying the final page of the wizard.
    * 
    * @param finalPage true if wizard is on final page, false if not.
    */
   public void displayingFinalPage(boolean finalPage)
   {
      if(finalPage)
      {
         mNextButton.setText("Finish");
         mNextButton.setActionCommand("finishWizard");
      }
      else
      {
         mNextButton.setText("Next >");
         mNextButton.setActionCommand("nextStep");
      }
   }
   
   /**
    * Displays the given page.
    * 
    * @param page the page to display.
    */
   public void displayPage(WizardPage page)
   {
      // see if the page is not currently supported
      if(!mWizardPages.contains(page))
      {
         // add the page if it is not supported
         addPage(page);
      }
      
      // show the page in the wizard pages panel
      mWizardPagesLayout.show(mWizardPagesPanel, page.getName());
   }
   
   /**
    * Display an error dialog detailing the problems with the validation
    * for a wizard page.
    */
   public void displayErrorDialog()
   {
      if(getWizard().getCurrentPage() != null)
      {
         // get the current page validation errors
         mErrorDialog.setWarnings(getWizard().getCurrentPage().getErrors());
         
         // move to top and display
         mErrorDialog.setLocationRelativeTo(this);
         mErrorDialog.setVisible(true);
      }
   }
   
   /**
    * Hides the error dialog.
    */
   public void hideErrorDialog()
   {
      mErrorDialog.setVisible(false);
   }
   
   /**
    * Display the quit confirmation dialog.
    */
   public void displayQuitDialog()
   {
      mQuitDialog.setLocationRelativeTo(this);
      mQuitDialog.setVisible(true);
   }
   
   /**
    * Hides the quit dialog.
    */
   public void hideQuitDialog()
   {
      mQuitDialog.setVisible(false);
   }
   
   /**
    * Invoked when an action is performed.
    * 
    * @param e the event related to the action that was performed.
    */
   public void actionPerformed(ActionEvent e)
   {
      if(e.getActionCommand().equals("cancelWizard"))
      {
         // show quit dialog
         displayQuitDialog();
      }
      else if(e.getActionCommand().equals("previousStep"))
      {
         // display the previous page
         getWizard().displayPreviousPage();
      }
      else if(e.getActionCommand().equals("nextStep"))
      {
         // display the next page
         getWizard().displayNextPage();
      }
      else if(e.getActionCommand().equals("finishWizard"))
      {
         // finish wizard
         getWizard().finish();
      }
      else if(e.getActionCommand().equals("errorDialogOk"))
      {
         // hide error dialog
         hideErrorDialog();
      }
      else if(e.getActionCommand().equals("quitDialogContinue"))
      {
         // hide quit dialog
         hideQuitDialog();
      }
      else if(e.getActionCommand().equals("quitDialogQuit"))
      {
         // hide quit dialog
         hideQuitDialog();
         
         // cancel wizard
         getWizard().cancel();
      }
   }
   
   /**
    * Enables the next button when a wizard page passes validation if
    * the page's view supports dynamic validation (it is a
    * DynamicValidationWizardPageView).
    * 
    * @param event the validation passed event.
    */
   public void wizardPageValidationPassed(EventObject event)
   {
      WizardPage page = (WizardPage)event.getData("page");
      if(page.getView() instanceof DynamicValidationWizardPageView)
      {
         mNextButton.setEnabled(true);
      }
   }

   /**
    * Disables the next button when a wizard page fails validation if
    * the page's view supports dynamic validation (it is a
    * DynamicValidationWizardPageView).
    * 
    * @param event the validation failed event.
    */
   public void wizardPageValidationFailed(EventObject event)
   {
      WizardPage page = (WizardPage)event.getData("page");
      if(page.getView() instanceof DynamicValidationWizardPageView)
      {
         mNextButton.setEnabled(false);
      }
   }
   
   /**
    * Disables the next button when a wizard page's validation is
    * pending if the page's view supports dynamic validation (it is a
    * DynamicValidationWizardPageView).
    * 
    * @param event the validation pending event.
    */
   public void wizardPageValidationPending(EventObject event)
   {
      WizardPage page = (WizardPage)event.getData("page");
      if(page.getView() instanceof DynamicValidationWizardPageView)
      {
         mNextButton.setEnabled(false);
      }
   }   
   
   /**
    * Displays the error dialog when a wizard page fails validation.
    * 
    * @param event the validation failed event.
    */
   public void displayErrorDialog(EventObject event)
   {
      // display the wizard page errors
      displayErrorDialog();
   }
   
   /**
    * Gets the wizard this view is for.
    * 
    * @return the wizard this view is for.
    */
   public Wizard getWizard()
   {
      return mWizard;
   }
}
