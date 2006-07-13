/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.filechooser.FileSystemView;

/**
 * A ValidatingFileChooser is a JFileChooser that can have files that
 * are chosen validated before accepting them.
 * 
 * @author Dave Longley
 */
public class ValidatingFileChooser extends JFileChooser
{
   /**
    * The file choice validator to use.
    */
   protected FileChoiceValidator mFileChoiceValidator;
   
   /**
    * Constructs a <code>ValidatingFileChooser</code> pointing to the user's
    * default directory. This default depends on the operating system.
    * It is typically the "My Documents" folder on Windows, and the
    * user's home directory on Unix.
    */
   public ValidatingFileChooser()
   {
   }
   
   /**
    * Constructs a <code>ValidatingFileChooser</code> using the given path.
    * Passing in a <code>null</code>
    * string causes the file chooser to point to the user's default directory.
    * This default depends on the operating system. It is
    * typically the "My Documents" folder on Windows, and the user's
    * home directory on Unix.
    *
    * @param currentDirectoryPath  a <code>String</code> giving the path
    *          to a file or directory
    */
   public ValidatingFileChooser(String currentDirectoryPath)
   {
      super(currentDirectoryPath);
   }

   /**
    * Constructs a <code>ValidatingFileChooser</code> using the
    * given <code>File</code> as the path. Passing in a <code>null</code> file
    * causes the file chooser to point to the user's default directory.
    * This default depends on the operating system. It is
    * typically the "My Documents" folder on Windows, and the user's
    * home directory on Unix.
    *
    * @param currentDirectory a <code>File</code> object specifying
    *                         the path to a file or directory.
    */
   public ValidatingFileChooser(File currentDirectory)
   {
      super(currentDirectory);
   }

   /**
    * Constructs a <code>ValidatingFileChooser</code> using the given
    * <code>FileSystemView</code>.
    * 
    * @param fsv the file system view to use.
    */
   public ValidatingFileChooser(FileSystemView fsv)
   {
      super(fsv);
   }


   /**
    * Constructs a <code>ValidatingFileChooser</code> using the given
    * current directory and <code>FileSystemView</code>.
    * 
    * @param currentDirectory the directory to use.
    * @param fsv the file system view to use.
    */
   public ValidatingFileChooser(File currentDirectory, FileSystemView fsv)
   {
      super(currentDirectory, fsv);
   }

   /**
    * Constructs a <code>ValidatingFileChooser</code> using the given
    * current directory path and <code>FileSystemView</code>.
    * 
    * @param currentDirectoryPath the current directory path to use.
    * @param fsv the file system view to use.
    */
   public ValidatingFileChooser(String currentDirectoryPath, FileSystemView fsv)
   {
      super(currentDirectoryPath, fsv);
   }
   
   /**
    * Called by the UI when the user hits the Approve button
    * (labeled "Open" or "Save", by default). This can also be
    * called by the programmer.
    * 
    * If the selection is validated, then this method will cause an action
    * event to fire with the command string equal to
    * <code>APPROVE_SELECTION</code>.
    *
    * @see #APPROVE_SELECTION
    */
   public void approveSelection()
   {
      if(getFileChoiceValidator() != null)
      {
         if(getFileChoiceValidator().validateFileChoice(this))
         {
            // approve selection
            super.approveSelection();
         }
      }
      else
      {
         // approve selection
         super.approveSelection();
      }
   }   
   
   /**
    * Sets the file choice validator to use.
    * 
    * @param fcv the file choice validator to use when a user tries to accept
    *            a file.
    */
   public void setFileChoiceValidator(FileChoiceValidator fcv)
   {
      mFileChoiceValidator = fcv;
   }
   
   /**
    * Gets the file choice validator to use.
    * 
    * @return the file choice validator to use when a user tries to accept
    *         a file.
    */
   public FileChoiceValidator getFileChoiceValidator()
   {
      return mFileChoiceValidator;
   }
}
