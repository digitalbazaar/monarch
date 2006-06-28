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

import com.db.event.EventObject;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;

/**
 * A ValidationDisplayWizardPageView is a WizardPageView that displays
 * dynamic validation errors.
 * 
 * @author Dave Longley
 */
public abstract class ValidationDisplayWizardPageView extends WizardPageView
implements DynamicValidationWizardPageView
{
   /**
    * The validation display label.
    */
   protected JLabel mValidationDisplayLabel;
   
   /**
    * The text to display when a page is validated.
    */
   protected String mValidatedText;
   
   /**
    * Creates a new ValidationDisplayWizardPageView.
    * 
    * @param page the wizard page this view is for.
    */
   public ValidationDisplayWizardPageView(WizardPage page)
   {
      super(page);
      
      // set default validated text
      setValidatedText("Page information validated. You may continue.");
      
      // listen for validation passed events
      page.getValidationPassedEventDelegate().addListener(
         this, "pageValidationPassed");
      
      // listen for validation failed events
      page.getValidationFailedEventDelegate().addListener(
         this, "pageValidationFailed");
      
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
         getWidth() - 5 - insets.left - insets.right,
         validationDisplay.getPreferredSize().height);
      validationDisplayConstraints.anchor =
         PositionConstraints.ANCHOR_BOTTOM |
         PositionConstraints.ANCHOR_LEFT | PositionConstraints.ANCHOR_RIGHT;
      
      // user input constraints
      PositionConstraints userInputConstraints = new PositionConstraints();
      userInputConstraints.location = new Point(5, 5);
      userInputConstraints.size = new Dimension(
         validationDisplayConstraints.size.width,
         validationDisplayConstraints.getTop() -
         userInputConstraints.location.y - 5);
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
      mValidationDisplayLabel = new JLabel(
         "WizardPage must fire validate events to update this display.");
      
      // add label
      panel.add(mValidationDisplayLabel);
      
      return panel;
   }
   
   /**
    * Updates the validation display by checking the current page
    * for errors. If errors are found, the first error will be displayed.
    * 
    * This method is called whenever a validation event is fired
    * by the WizardPage this view is for. Therefore, this method should
    * not call validate() on the page.
    */
   public void updateValidationDisplay()
   {
      // check errors
      if(getPage().getErrors().size() > 0)
      {
         // set text color to red
         mValidationDisplayLabel.setForeground(Color.red);
         
         // set text to first error if one exists
         String error = (String)getPage().getErrors().get(0);
         mValidationDisplayLabel.setText(error);
      }
      else
      {
         // set text color to black
         mValidationDisplayLabel.setForeground(Color.black);
         
         // set text
         mValidationDisplayLabel.setText(getValidatedText());
      }
   }
   
   /**
    * Sets the text to display when the page is validated.
    * 
    * @param text the text to display when the page is validated.
    */
   public void setValidatedText(String text)
   {
      mValidatedText = text;
   }
   
   /**
    * Gets the text to display when the page is validated.
    * 
    * @return the text to display when the page is validated.
    */
   public String getValidatedText()
   {
      return mValidatedText;
   }
   
   /**
    * Called when the page this view is for passes validation.
    * 
    * @param e the validation passed event.
    */
   public void pageValidationPassed(EventObject e)
   {
      // update validation display
      updateValidationDisplay();
   }
   
   /**
    * Called when the page this view is for fails validation.
    * 
    * @param e the validation failed event.
    */
   public void pageValidationFailed(EventObject e)
   {
      // update validation display
      updateValidationDisplay();
   }
}
