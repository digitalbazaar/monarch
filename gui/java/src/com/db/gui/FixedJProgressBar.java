/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.EventQueue;

import com.db.util.MethodInvoker;

/**
 * This progress bar makes sure to make changes on "EventQueue." JProgressBar
 * fails to do this and requires that you set up a bunch of junk everytime
 * you want to do anything with it in a multithreaded environment.
 * 
 * @author Dave Longley
 */
public class FixedJProgressBar extends CustomTextProgressBar
{
   
   /**
    * Creates a FixedJProgressBar.
    */
   public FixedJProgressBar()
   {
   }
   
   /**
    * Sets the painted string for this progress bar.
    *
    * @param string the string.
    */
   public void setString(String string)
   {
      if(EventQueue.isDispatchThread())
      {
         super.setString(string);
      }
      else
      {
         // must call setString(false) on the event queue when
         // using BasicProgressBarUI or else it may throw an exception
         // or just not work at all
         Object[] params = new Object[]{string};
         MethodInvoker mi =
            new MethodInvoker(this, "setString", params);
         EventQueue.invokeLater(mi);
      }
   }
   
   /**
    * Sets this progress bar to indeterminate or unindeterminate.
    * 
    * @param enable true to enable indeterminate mode, false to disable it.
    */
   public void setIndeterminate(boolean enable)
   {
      if(EventQueue.isDispatchThread())
      {
         super.setIndeterminate(enable);
      }
      else
      {
         // must call setIndeterminate(false) on the event queue when
         // using BasicProgressBarUI or else it may throw an exception
         // or just not work at all
         Object[] params = new Object[]{new Boolean(enable)};
         MethodInvoker mi =
            new MethodInvoker(this, "setIndeterminate", params);
         EventQueue.invokeLater(mi);
      }
   }
}
