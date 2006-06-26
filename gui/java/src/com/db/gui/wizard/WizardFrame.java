/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui.wizard;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JDialog;
import javax.swing.JFrame;

import com.db.event.EventObject;

/**
 * A wizard frame for displaying a wizard.
 * 
 * @author Dave Longley
 * @author Mike Johnson
 */
public class WizardFrame extends JDialog
implements ActionListener, WindowListener
{
   /**
    * A reference to the Wizard this frame displays.
    */
   protected Wizard mWizard;
   
   /**
    * A wizard quit dialog.
    */
   protected WizardQuitDialog mQuitDialog;
   
   /**
    * Creates a new WizardFrame.
    * 
    * @param wizard the wizard to display in the frame.
    */
   public WizardFrame(Wizard wizard)
   {
      // store wizard
      mWizard = wizard;
      
      // listen to the wizard for started event
      mWizard.getWizardStartedEventDelegate().addListener(
         this, "wizardStarted");
      
      // listen to the wizard for finished event
      mWizard.getWizardFinishedEventDelegate().addListener(
         this, "wizardFinished");
      
      // listen to the wizard for cancelled event
      mWizard.getWizardCancelledEventDelegate().addListener(
         this, "wizardCancelled");
      
      // create the wizard quit dialog
      mQuitDialog = new WizardQuitDialog();
      mQuitDialog.setModal(true);
      mQuitDialog.addActionListener(this);
      
      // listen to self for window closing events
      addWindowListener(this);
      
      // add Wizard's view to content pane
      getContentPane().add(mWizard.getView());

      // setup the frame
      setupFrame();
      
      // make unresizable by default
      setResizable(false);
   }
   
   /**
    * Sets up the wizard frame.
    * 
    * Should be overridden to customize the frame creation.
    */
   protected void setupFrame()
   {
      // dispose on close 
      setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

      // pack frame
      pack();
      
      // set size
      setSize(640, 480);
   }
   
   /**
    * Display the quit confirmation dialog.
    */
   protected void displayQuitDialog()
   {
      mQuitDialog.setLocationRelativeTo(this);
      mQuitDialog.setVisible(true);
   }
   
   /**
    * Hides the quit dialog.
    */
   protected void hideQuitDialog()
   {
      mQuitDialog.setVisible(false);
   }   
   
   /**
    * Called when the wizard has started.
    * 
    * @param event the wizard started event.
    */
   public void wizardStarted(EventObject event)
   {
      // show this frame
      setVisible(true);
   }
   
   /**
    * Called when the wizard has finished.
    * 
    * @param event the wizard finished event.
    */
   public void wizardFinished(EventObject event)
   {
      // hide
      setVisible(false);
      
      // dispose
      dispose();
   }
   
   /**
    * Called when the wizard has been cancelled.
    * 
    * @param event the wizard cancelled event.
    */
   public void wizardCancelled(EventObject event)
   {
      // hide
      setVisible(false);
      
      // dispose
      dispose();
   }
   
   /**
    * Gets the wizard held in this frame.
    * 
    * @return the wizard held in this frame.
    */
   public Wizard getWizard()
   {
      return mWizard;
   }
   
   /**
    * Called when the wizard quit dialog performs an action. 
    * 
    * @param e the action event.
    */
   public void actionPerformed(ActionEvent e)
   {
      if(e.getActionCommand().equals("quitDialogContinue"))
      {
         // hide quit dialog
         hideQuitDialog();
      }
      else if(e.getActionCommand().equals("quitDialogQuit"))
      {
         // hide quit dialog
         hideQuitDialog();
         
         // cancel wizard if running
         if(mWizard.isRunning())
         {
            mWizard.cancel();
         }
         else
         {
            // hide
            setVisible(false);
            
            // dispose
            dispose();
         }
      }
   }
   
   /**
    * Called whenever the window has been opened.
    * 
    * @param e the window event data.
    */
   public void windowOpened(WindowEvent e) 
   {
   }

   /**
    * Invoked when the user attempts to close the window
    * from the window's system menu.
    */
   public void windowClosing(WindowEvent e)
   {
      // see if the wizard is running
      if(mWizard.isRunning())
      {
         // show quit dialog
         displayQuitDialog();
      }
      else
      {
         // hide
         setVisible(false);
         
         // dispose
         dispose();
      }
   }

   /**
    * Called whenever this window has been closed via the window manager.
    * 
    * @param e the window event data.
    */
   public void windowClosed(WindowEvent e) 
   {
   }

   /**
    * Called whenever the window has been iconified via the window manager.
    * 
    * @param e the window event data.
    */
   public void windowIconified(WindowEvent e)  
   {      
   }
   
   /**
    * Called whenever the window has been deiconified via the window manager.
    * 
    * @param e the window event data.
    */
   public void windowDeiconified(WindowEvent e) 
   {
   }

   /**
    * Called whenever this window has been selected via the window manager.
    * 
    * @param e the window event data.
    */
   public void windowActivated(WindowEvent e)
   {
   }

   /**
    * Called whenever this window has been deactivated via the window manager.
    * 
    * @param e the window event data.
    */
   public void windowDeactivated(WindowEvent e) 
   {
   }
}
