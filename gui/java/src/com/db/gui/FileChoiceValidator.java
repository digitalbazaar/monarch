/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

/**
 * A FileChoiceValidator is used in conjunction with a ValidatingFileChooser
 * to allow some kind of validation method to be called before a file that
 * is selected by a user is approved.
 * 
 * @author Dave Longley
 */
public interface FileChoiceValidator
{
   /**
    * Called when a user tries to choose a file with an associated
    * ValidatingFileChooser. This method should determine whether or not
    * the user's choice is valid and take any appropriate action for
    * either case.
    * 
    * The ValidatingFileChooser will fire an action event "APPROVE_SELECTION"
    * and close (become hidden) if the choice is validated.
    * 
    * @param vfc the ValidatingFileChooser with a selected file to validate.
    * 
    * @return true if the file choice is valid and the ValidatingFileChooser
    *         should close, false if the file choice is not valid. 
    */
   public boolean validateFileChoice(ValidatingFileChooser vfc);
}
