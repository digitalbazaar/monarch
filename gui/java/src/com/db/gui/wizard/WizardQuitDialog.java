/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.Insets;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.db.gui.LayeredLayout;

/**
 * Inner class dialog for confirming quitting the wizard.
 * 
 * @author Mike Johnson
 * @author Dave Longley
 */
public class WizardQuitDialog extends JDialog
{
   /**
    * The quit button means the user wants to quit the wizard.
    */
   protected JButton mQuitButton;
   
   /**
    * The continue button means the user wants to continue using the wizard.
    */
   protected JButton mContinueButton;
   
   /**
    * Quit wizard dialog constructor.
    */
   public WizardQuitDialog()
   {
      setTitle("Quit wizard without saving?");
      setupPanel();
   }
   
   /**
    * Sets up the dialog.
    */
   public void setupPanel()
   {
      int[][] plot = {{0,0}, // label 1
                      {0,1}, // label 2
                      {0,2}, {1,2}}; // quit, cancel buttons
      LayeredLayout ll = new LayeredLayout(plot);
      JPanel view = new JPanel(ll);
      
      // add message label 1
      JLabel mMessageLabel1 = new JLabel("Are you sure you want to quit the " +
            "wizard now?");
      ll.placeNext(mMessageLabel1, 1.0, 0.0, true, false, 
            new Insets(5, 5, 5, 5));
      view.add(mMessageLabel1);
      
      // add message label 2
      JLabel mMessageLabel2 = new JLabel("Your progress in this wizard will " +
            "not be saved.");
      ll.placeNext(mMessageLabel2, 1.0, 0.0, true, false, 
            new Insets(5, 5, 5, 5));
      view.add(mMessageLabel2);
      
      // add buttons
      mQuitButton = new JButton("Quit!");
      mQuitButton.setActionCommand("quitDialogQuit");
      ll.placeNext(mQuitButton, 1.0, 0.0, LayeredLayout.LEFT,
            new Insets(2, 2, 2, 2));
      view.add(mQuitButton);
      
      mContinueButton = new JButton("Continue!");
      mContinueButton.setActionCommand("quitDialogContinue");
      ll.placeNext(mContinueButton, 1.0, 0.0, LayeredLayout.RIGHT,
            new Insets(2, 2, 2, 2));
      view.add(mContinueButton);
      
      setContentPane(view);
      pack();
      setModal(true);
   }
   
   /**
    * Adds an action listener.
    * 
    * @param al the new action listener.
    */
   public void addActionListener(ActionListener al)
   {
      mQuitButton.addActionListener(al);
      mContinueButton.addActionListener(al);
   }
}   
