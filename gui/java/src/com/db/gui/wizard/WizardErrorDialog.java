/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.Insets;
import java.awt.event.ActionListener;
import java.util.Iterator;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import com.db.gui.LayeredLayout;

/**
 * The wizard error dialog is used to display errors in the wizard
 * process. It is most often used to display wizard page validation errors
 * to the user.
 * 
 * @author Manu Sporny
 * @author Mike Johnson
 * @author Dave Longley
 */
public class WizardErrorDialog extends JDialog
{
   /**
    * The error label is used to warn the user that there has been an
    * error.
    */
   protected JLabel mErrorLabel;
   
   /**
    * The text area is used to display all of the errors.
    */
   protected JTextArea mTextArea;
   
   /**
    * The OK button is used to dismiss the dialog when the user 
    * has finished reading it.
    */
   protected JButton mOkButton;
   
   /**
    * Sets up the support request dialog.
    */
   public WizardErrorDialog()
   {
      setTitle("Wizard Page Errors");
      setupPanel();
   }

   /**
    * Sets up the panel.
    */
   protected void setupPanel()
   {
      int[][] plot = {{0,0},{0,1},{0,2}};
      LayeredLayout ll = new LayeredLayout(plot);
      JPanel view = new JPanel(ll);
      
      // add request label
      mErrorLabel = new JLabel("The information you entered resulted " +
                               "in the following errors:");
      ll.placeNext(mErrorLabel, 1.0, 0.0, true, false,
                   new Insets(5, 5, 5, 5));
      view.add(mErrorLabel);
      
      // add text area
      mTextArea = new JTextArea();
      mTextArea.setRows(5);
      mTextArea.setEditable(false);
      mTextArea.setLineWrap(true);
      mTextArea.setWrapStyleWord(true);
      
      JScrollPane sp = new JScrollPane(mTextArea);
      ll.placeNext(sp, 1.0, 1.0, true, true, new Insets(0, 5, 5, 5));
      view.add(sp);
      
      // add the OK button
      mOkButton = new JButton("Ok");
      mOkButton.setActionCommand("errorDialogOk");
      ll.placeNext(mOkButton, 1.0, 0.0, LayeredLayout.RIGHT,
                   new Insets(0, 0, 5, 0));
      view.add(mOkButton);
      
      setContentPane(view);
      pack();
      setLocationRelativeTo(null);
   }
   
   /**
    * Set the warning string displayed in the dialog.
    * 
    * @param error a warning string displayed in the dialog.
    */
   public void setWarning(String error)
   {
      mTextArea.setText("");
      
      mTextArea.append(error +"\n\n");
   }

   /**
    * Set the warning strings displayed in the dialog.
    * 
    * @param errors a vector of warning strings that are to be
    *                       displayed in the dialog.
    */
   public void setWarnings(Vector errors)
   {
      mTextArea.setText("");
      
      // Go through all of the warning strings and display them onscreen.
      Iterator i = errors.iterator();         
      while(i.hasNext())
      {
         String warning = (String)i.next();
         mTextArea.append(warning +"\n\n");
      }
      
      // move caret to top of error list
      mTextArea.setCaretPosition(0);
   }
   
   /**
    * Adds an action listener.
    * 
    * @param al the new action listener.
    */
   public void addActionListener(ActionListener al)
   {
      mOkButton.addActionListener(al);
   }
}
