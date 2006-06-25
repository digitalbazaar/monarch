/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Point;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;

/**
 * A DynamicValidationWizardPageView is a WizardPageView that can
 * display dynamic validation errors.
 * 
 * @author Dave Longley
 */
public abstract class DynamicValidationWizardPageView extends WizardPageView
{
   /**
    * The validation display label.
    */
   protected JLabel mValidationDisplayLabel;
   
   /**
    * Creates a new DynamicValidationWizardPageView.
    * 
    * @param page the wizard page this view is for.
    */
   public DynamicValidationWizardPageView(WizardPage page)
   {
      super(page);
      
      // setup the view
      setupView();
   }
   
   /**
    * Creates this view -- including the user input panel (as specified
    * by an extending class) and the dynamic validation error display.
    */
   protected void setupView()
   {
      // setup layout
      setSize(500, 500);
      setLayout(new PositionLayout(this));
      
      // create user input panel
      JPanel userInputPanel = createUserInputPanel();
      
      // create validation display panel
      JPanel validationDisplay = createValidationDisplayPanel();
      
      // get insets
      Insets insets = getInsets();
      
      // validation display constraints
      PositionConstraints validationDisplayConstraints =
         new PositionConstraints();
      validationDisplayConstraints.location = new Point(
         5, getHeight() - 5 - validationDisplay.getPreferredSize().height -
         insets.bottom);
      validationDisplayConstraints.size = new Dimension(
         getWidth() - 10 - insets.left - insets.right,
         validationDisplay.getPreferredSize().height);
      validationDisplayConstraints.anchor =
         PositionConstraints.ANCHOR_BOTTOM |
         PositionConstraints.ANCHOR_LEFT | PositionConstraints.ANCHOR_RIGHT;
      
      // user input constraints
      PositionConstraints userInputConstraints = new PositionConstraints();
      userInputConstraints.location = new Point(0, 0);
      userInputConstraints.size = new Dimension(
         getWidth() - insets.left - insets.right,
         validationDisplayConstraints.getTop());
      userInputConstraints.anchor = PositionConstraints.ANCHOR_ALL;
      
      // add components
      add(userInputPanel, userInputConstraints);
      add(validationDisplay, validationDisplayConstraints);
   }
   
   /**
    * Creates the user input panel.
    * 
    * @return the user input panel.
    */
   protected abstract JPanel createUserInputPanel();

   /**
    * Creates the validation display panel.
    * 
    * @return the validation display panel.
    */
   protected JPanel createValidationDisplayPanel()
   {
      JPanel panel = new JPanel();
      
      // make panel transparent
      panel.setOpaque(false);
      
      // create border for panel
      panel.setBorder(BorderFactory.createEtchedBorder());

      // create label for displaying the first validation error
      mValidationDisplayLabel = new JLabel("Click 'Next' to continue.");
      
      // add label
      panel.add(mValidationDisplayLabel);
      
      return panel;
   }
   
   /**
    * Updates the validation display by validating the current page
    * and displaying the first error -- if any -- or that no errors
    * were detected.
    * 
    * This method should be called when a page is activated.
    * 
    * This method should also be called whenever the user enters
    * information so that the validation display can be updated.
    */
   public void updateValidationDisplay()
   {
      // validate page
      if(getPage().validate(getPage().getWizardTask()))
      {
         // validation passed
         
         // set text color to black
         mValidationDisplayLabel.setForeground(Color.black);
         
         // set text
         mValidationDisplayLabel.setText("Click 'Next' to continue.");
      }
      else
      {
         // validation failed
         
         // set text color to red
         mValidationDisplayLabel.setForeground(Color.red);
         
         // set text to first error if one exists
         if(getPage().getErrors().size() > 0)
         {
            // get first page error
            String error = (String)getPage().getErrors().get(0);
            mValidationDisplayLabel.setText(error);
         }
         else
         {
            // display that validation was not passed
            mValidationDisplayLabel.setText("Page not yet valid.");
         }
      }
   }
}
