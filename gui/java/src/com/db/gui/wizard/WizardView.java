/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.CardLayout;
import java.awt.Component;
import java.awt.Insets;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextPane;

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
      
      // create error dialog
      mErrorDialog = new WizardErrorDialog();
      
      // create quit dialog
      mQuitDialog = new WizardQuitDialog();
      
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
      
      // set up wizard pages panel
      mWizardPagesPanel = new JPanel();
      mWizardPagesLayout = new CardLayout();
      mWizardPagesPanel.setLayout(mWizardPagesLayout);
      
      // create buttons for wizard navigation
      mCancelButton = new JButton("Cancel");
      mCancelButton.setActionCommand("cancelWizard");
      mCancelButton.addActionListener(this);
      
      // create glue to push buttons right
      Component glue = Box.createGlue();

      // create previous button
      mPrevButton = new JButton("<< Previous");
      mPrevButton.setActionCommand("previousStep");
      mPrevButton.addActionListener(this);
      
      // disabled by default (should be on first wizard page)
      mPrevButton.setEnabled(false);

      // create next button
      mNextButton = new JButton("Next >>");
      mNextButton.setActionCommand("nextStep");
      mNextButton.addActionListener(this);

      // place wizard pages panel
      ll.placeNext(mWizardPagesPanel, 1.0, 1.0, true, true);
      add(mWizardPagesPanel);

      // place cancel button
      ll.placeNext(mCancelButton, 0.0, 0.0, new Insets(2, 2, 2, 2));
      add(mCancelButton);

      // place glue
      ll.placeNext(glue, 1.0, 0.0, true, false);
      add(glue);
      
      // place previous button
      ll.placeNext(mPrevButton, 0.0, 0.0, new Insets(2, 2, 2, 2));
      add(mPrevButton);
      
      // place next button
      ll.placeNext(mNextButton, 0.0, 0.0, new Insets(2, 2, 2, 2));
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
         mNextButton.setText("Next >>");
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
         WizardView.centerWindow(mErrorDialog);
         mErrorDialog.setAlwaysOnTop(true);
         mErrorDialog.setModal(true);
         mErrorDialog.setVisible(true);
      }
   }
   
   /**
    * Hides the error dialog.
    */
   public void hideErrorDialog()
   {
      mErrorDialog.setAlwaysOnTop(false);
      mErrorDialog.setModal(false);
      mErrorDialog.setVisible(false);
   }
   
   /**
    * Display the quit confirmation dialog.
    */
   public void displayQuitDialog()
   {
      WizardView.centerWindow(mQuitDialog);
      mQuitDialog.setAlwaysOnTop(true);
      mQuitDialog.setModal(true);
      mQuitDialog.setVisible(true);
   }
   
   /**
    * Hides the quit dialog.
    */
   public void hideQuitDialog()
   {
      mQuitDialog.setAlwaysOnTop(false);
      mQuitDialog.setModal(false);
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
         // cancels wizard, runs wizard specific cancelling routine
         getWizard().cancelWizard();
      }
      else if(e.getActionCommand().equals("previousStep"))
      {
         // displays the previous page
         getWizard().displayPreviousPage();
      }
      else if(e.getActionCommand().equals("nextStep"))
      {
         // check current page for errors
         if(getWizard().checkCurrentPageForErrors())
         {
            // display the next page
            getWizard().displayNextPage();
         }
         else
         {
            // display the wizard page errors
            displayErrorDialog();
         }
      }
      else if(e.getActionCommand().equals("finishWizard"))
      {
         // check current page for errors
         if(getWizard().checkCurrentPageForErrors())
         {
            // finish wizard, run wizard specific finishing routine
            getWizard().finishWizard();
         }
      }
      else if(e.getActionCommand().equals("errorDialogOk"))
      {
         // hide error dialog
         hideErrorDialog();
      }
      else if(e.getActionCommand().equals("quitDialogQuit"))
      {
         // hide quit dialog
         hideQuitDialog();
         
         // cancel wizard
         getWizard().cancelWizard();
      }
      else if(e.getActionCommand().equals("quitDialogContinue"))
      {
         // hide quit dialog
         hideQuitDialog();
      }
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
   
   /**
    * Centers the passed window.
    * 
    * @param window the window to center.
    */
   public static void centerWindow(Window window)
   {
      int x = (window.getGraphicsConfiguration().getBounds().width -
               window.getWidth()) / 2;
      int y = (window.getGraphicsConfiguration().getBounds().height -
               window.getHeight()) / 2;
        
      window.setLocation(x, y);
   }   
}
