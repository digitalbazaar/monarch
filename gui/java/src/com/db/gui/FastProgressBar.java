/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Graphics;

import javax.swing.JComponent;

import com.db.event.EventDelegate;
import com.db.event.EventObject;
import com.db.util.MethodInvoker;

/**
 * This class represents a progress bar that can adjust and redraw
 * itself quickly.
 * 
 * @author Dave Longley
 * @author Mike Johnson
 */
public class FastProgressBar extends JComponent
implements ChangeReporter, Comparable
{
   /**
    * The minimum value for this progress bar.
    */
   protected int mMinimumValue;
   
   /**
    * The maximum value for this progress bar.
    */
   protected int mMaximumValue;
   
   /**
    * The current value for this progress bar.
    */
   protected int mValue;
   
   /**
    * Whether or not this progress bar is set to indeterminate mode.
    */
   protected boolean mIndeterminate;
   
   /**
    * The current indeterminate value. This value bounces between the minimum
    * and maximum values when the progress bar is set in indeterminate mode.
    */
   protected int mIndeterminateValue;
   
   /**
    * The indeterminate time interval. This interval is used to cycle
    * progress when this progress bar's definite value is unknown.
    */
   protected long mIndeterminateInterval;
   
   /**
    * The text to display in the progress bar.
    */
   protected String mText;
   
   /**
    * The thread used to cycle progress.
    */
   protected Thread mProgressCyclerThread;
   
   /**
    * The ui class id for this component.
    */
   protected static final String mUiClassId = "com.db.gui.FastProgressBarUI";
   
   /**
    * The change delegate for this progress bar.
    */
   protected EventDelegate mChangeDelegate;
   
   /**
    * Set to true if a repaint() has been scheduled and not yet fulfilled
    * for this progress bar.
    */
   protected boolean mRepaintScheduled;
   
   /**
    * Stores the UI for this progress bar.
    * 
    * FIXME: This should be a static instance that is shared between
    * all progress bars. 
    */
   protected FastProgressBarUI mUI;
   
   /**
    * Creates a new FastProgressBar.
    */
   public FastProgressBar()
   {
      // create change delegate
      mChangeDelegate = new EventDelegate();

      // set value range and value
      setMinimumValue(0);
      setMaximumValue(100);
      setValue(0);
      
      // set text
      setText(null);
      
      // initialize indeterminate values
      setIndeterminate(false);
      setIndeterminateValue(0);
      setIndeterminateInterval(50);
      
      // initialize progress cycler thread to null
      mProgressCyclerThread = null;
      
      // update UI to install FastProgressBar UI
      mUI = null;
      updateUI();
      
      // no repaint scheduled yet
      mRepaintScheduled = false;
   }
   
   /**
    * Fires the progress bar change event for this change reporter.
    */
   protected void fireFastProgressBarChanged()
   {
      // create change event
      EventObject event = new EventObject("fastProgressBarChanged");
      event.setData("source", this);
      
      // fire event
      getChangeDelegate().fireEvent(event);
   }
   
   /**
    * Starts cycling progress in a background process.
    */
   protected void startCyclingProgress()
   {
      // execute cycle progress in a background process 
      MethodInvoker mi = new MethodInvoker(this, "cycleProgress");
      mi.backgroundExecute();
      
      // store progress cycler thread
      mProgressCyclerThread = mi;
   }
   
   /**
    * Stops cycling progress. 
    */
   protected void stopCyclingProgress()
   {
      // interrupt current progress cycling thread if one exists
      if(mProgressCyclerThread != null)
      {
         mProgressCyclerThread.interrupt();
         mProgressCyclerThread = null;
      }
   }
   
   /**
    * This method cycles the progress on this progress bar when it is
    * in indeterminate mode.
    */
   public void cycleProgress()
   {
      try
      {
         // set indeterminate value to minimum value
         setIndeterminateValue(getMinimumValue());
         
         // progress starts out as increasing
         boolean increasing = true;
         
         // cycles progress while this progress bar is set to indeterminate
         while(isIndeterminate() && !Thread.currentThread().isInterrupted())
         {
            // sleep for indeterminate interval
            Thread.sleep(getIndeterminateInterval());
            
            // if still indeterminate and not interrupted, proceed
            if(isIndeterminate() && !Thread.currentThread().isInterrupted())
            {
               // repaint
               repaint();

               // get old indeterminate value
               int value = getIndeterminateValue();
               
               // set new value
               if(increasing)
               {
                  value += 2;
               }
               else
               {
                  value -= 2;
               }
               
               // make sure value is in range
               value = Math.min(getMaximumValue(), value);
               value = Math.max(getMinimumValue(), value);
               
               setIndeterminateValue(value);

               // set increasing or decreasing progress 
               if(value == getMaximumValue())
               {
                  increasing = false;
               }
               else if(value == getMinimumValue())
               {
                  increasing = true;
               }
            }
         }
      }
      catch(InterruptedException ignore)
      {
         // ignore interrupted exception (but keep flag set)
         Thread.currentThread().interrupt();
      }
      
      // set indeterminate value back to minimum value
      setIndeterminateValue(getMinimumValue());

      // repaint once more
      repaint();
   }
   
   /**
    * Resets the UI property with a value from the current look and feel.
    */
   @Override
   public void updateUI()
   {
      if(mUI == null) 
      {
         mUI = new FastProgressBarUI(this);
      }
      setUI(mUI);
      
      // fire change event
      fireFastProgressBarChanged();
   }
   
   /**
    * Paints this component. Overridden to reset variable that tracks painting.
    * 
    * @param g the graphics to use.
    */
   @Override
   public void paint(Graphics g)
   {
      // now painting
      mRepaintScheduled = true;
      
      // paint
      super.paint(g);
      
      // no repaint scheduled
      mRepaintScheduled = false;
   }
   
   /**
    * Paints this component in the specified region immediately. Overridden
    * to reset variable that tracks painting.
    * 
    * @param x the x value of the region to be painted.
    * @param y the y value of the region to be painted.
    * @param w the width of the region to be painted.
    * @param h the height of the region to be painted.
    */
   @Override
   public void paintImmediately(int x,int y,int w, int h)   
   {
      // now painting
      mRepaintScheduled = true;
      
      // paint
      super.paintImmediately(x, y, w, h);
      
      // no repaint scheduled
      mRepaintScheduled = false;
   }
   
   /**
    * Repaints this progress bar if a repaint hasn't already been scheduled.
    */
   @Override
   public void repaint()
   {
      if(!isRepaintScheduled())
      {
         mRepaintScheduled = true;
         super.repaint();
      }
   }
   
   /**
    * Returns true if a repaint() has been scheduled for this progress bar,
    * false if not.
    * 
    * @return true if a repaint() has been scheduled for this progress bar,
    *         false if not.
    */
   public boolean isRepaintScheduled()
   {
      return mRepaintScheduled;
   }
   
   /**
    * Returns a string that specifies the name of the L&F class that
    * renders this component.
    *
    * @return the UIClassID for this component.
    */
   @Override
   public String getUIClassID()
   {
      return mUiClassId;
   }
   
   /**
    * Gets the fast progress bar ui for this fast progress bar.
    * 
    * @return the fast progress bar ui for this fast progress bar.
    */
   public FastProgressBarUI getFastProgressBarUI()
   {
      return (FastProgressBarUI)ui;
   }
   
   /**
    * Sets the minimum value for this progress bar. 
    * 
    * @param min the minimum value for this progress bar.
    */
   public void setMinimumValue(int min)
   {
      mMinimumValue = min;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the minimum value for this progress bar. 
    * 
    * @return the minimum value for this progress bar.
    */
   public int getMinimumValue()
   {
      return mMinimumValue;
   }
   
   /**
    * Sets the maximum value for this progress bar.
    * 
    * @param max the maximum value for this progress bar.
    */
   public void setMaximumValue(int max)
   {
      mMaximumValue = max;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the maximum value for this progress bar.
    * 
    * @return the maximum value for this progress bar.
    */
   public int getMaximumValue()
   {
      return mMaximumValue;
   }
   
   /**
    * Sets the current value for this progress bar.
    * 
    * @param value the current value for this progress bar. 
    */
   public void setValue(int value)
   {
      mValue = value;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the current value for this progress bar.
    * 
    * @return the current value for this progress bar. 
    */
   public int getValue()
   {
      return mValue;
   }
   
   /**
    * Gets the range of values for this progress bar.
    * 
    * @return the range of values for this progress bar.
    */
   public long getRange()
   {
      return getMaximumValue() - getMinimumValue();
   }
   
   /**
    * Gets the percentage the current value is out of the range of values. The
    * value that is returned is between 0.0 and 1.0.
    * 
    * @return the percentage of the range of values that the current value is.
    */
   public double getPercentage()
   {
      double rval = 0.0;
      
      double value = getValue();
      rval = value / getRange();
      
      return rval;
   }

   /**
    * Sets whether or not this progress bar is in indeterminate mode. 
    * 
    * @param indeterminate true to set this progress bar to indeterminate
    *                      mode, false to set this progress bar to
    *                      determinate mode.
    */
   public synchronized void setIndeterminate(boolean indeterminate)
   {
      if(mIndeterminate != indeterminate)
      {
         mIndeterminate = indeterminate;
         
         if(indeterminate)
         {
            // start cycling progress
            startCyclingProgress();
         }
         else
         {
            // stop cycling progress
            stopCyclingProgress();
         }
         
         // fire change event
         fireFastProgressBarChanged();            
      }
   }
   
   /**
    * Gets whether or not this progress bar is in indeterminate mode. 
    * 
    * @return true to set this progress bar to indeterminate mode, false
    *         false to set this progress bar to determinate mode.
    */
   public synchronized boolean isIndeterminate()
   {
      return mIndeterminate;
   }
   
   /**
    * Sets the current indeterminate value.
    * 
    * @param value the current indeterminate value.
    */
   public void setIndeterminateValue(int value)
   {
      mIndeterminateValue = value;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the current indeterminate value.
    * 
    * @return the current indeterminate value.
    */
   public int getIndeterminateValue()
   {
      return mIndeterminateValue;
   }
   
   /**
    * Sets the indeterminate interval. This interval is used to cycle
    * progress when this progress bar's definite value is unknown.
    * 
    * @param interval the indeterminate interval.
    */
   public void setIndeterminateInterval(long interval)
   {
      mIndeterminateInterval = interval;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the indeterminate interval. This interval is used to cycle
    * progress when this progress bar's definite value is unknown.
    * 
    * @return the indeterminate interval.
    */
   public long getIndeterminateInterval()
   {
      return mIndeterminateInterval;      
   }
   
   /**
    * Gets the percentage the indeterminate value is out of the range of
    * values. The value that is returned is between 0.0 and 1.0.
    * 
    * @return the percentage of the range of values that the indeterminate
    *         value is.
    */
   public double getIndeterminatePercentage()
   {
      double rval = 0.0;
      
      double value = getIndeterminateValue();
      rval = value / getRange();
      
      return rval;
   }
   
   /**
    * Sets the text to display in the progress bar. If null is passed then
    * no text will be displayed in the progress bar.
    * 
    * @param text the text to display in the progress bar.
    */
   public void setText(String text)
   {
      mText = text;
      
      // fire change event
      fireFastProgressBarChanged();            
   }
   
   /**
    * Gets the text displayed in the progress bar.
    * 
    * @return the text displayed in the progress bar or null if no text is
    *         displayed.
    */
   public String getText()
   {
      return mText;
   }
   
   /**
    * Resets the text, meter, and background colors for this progress bar.
    */
   public void resetColors()
   {
      // reset colors on UI
      getFastProgressBarUI().resetTextAndMeterColors();
      
      // reset background
      setBackground(getFastProgressBarUI().getDefaultBackgroundColor());
   }   
   
   /**
    * Uses the selected colors for this progress bar.
    */
   public void useSelectedColors()
   {
      // reset colors
      resetColors();
      
      // swap colors
      Color meterColor = getFastProgressBarUI().getMeterColor();
      Color emptyMeterText = getFastProgressBarUI().getEmptyMeterTextColor();
      Color fullMeterText = getFastProgressBarUI().getFullMeterTextColor();
      
      getFastProgressBarUI().setMeterColor(fullMeterText);
      getFastProgressBarUI().setEmptyMeterTextColor(fullMeterText);
      getFastProgressBarUI().setFullMeterTextColor(emptyMeterText);
      setBackground(meterColor);
   }
   
   /**
    * Compares this fast progress bar to another one.
    * 
    * @param obj the fast progress bar to compare this one to.
    * 
    * @return 1 if this fast progress bar has a greater value,
    *         0 if the bar values are the same, and -1 if the
    *         passed bar has a greater value than this one.
    */
   public int compareTo(Object obj)
   {
      int rval = 0;
      
      if(obj instanceof FastProgressBar)
      {
         FastProgressBar bar = (FastProgressBar)obj;
         
         if(getValue() > bar.getValue())
         {
            rval = 1;
         }
         else if(getValue() < bar.getValue())
         {
            rval = -1;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the change delegate for this change reporter. 
    *
    * @return the change delegate.
    */
   public EventDelegate getChangeDelegate()
   {
      return mChangeDelegate;
   }   
}
