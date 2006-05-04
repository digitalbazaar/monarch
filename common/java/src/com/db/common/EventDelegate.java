/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import java.util.Iterator;
import java.util.Vector;

/**
 * This class is used to delegate events to listeners.
 * 
 * @author Dave Longley
 */
public class EventDelegate
{
   /**
    * A list of all of the listeners of this delegate.
    */
   protected Vector mListeners;
   
   /**
    * The name of the listener method to call to process an event.
    */
   protected String mListenerMethod;

   /**
    * Constructs a new event delegate.
    * 
    * @param listenerMethod the name of the listener method to call to
    *                       process an event.
    */
   public EventDelegate(String listenerMethod)
   {
      // create listener list
      mListeners = new Vector();
      
      // set listener method
      mListenerMethod = listenerMethod;
   }
   
   /**
    * Adds a listener.
    *
    * @param listener the listener to add.
    */
   public synchronized void addListener(Object listener)
   {
      mListeners.add(listener);
   }
   
   /**
    * Removes a listener.
    * 
    * @param listener the listener to remove.
    */
   public synchronized void removeListener(Object listener)
   {
      mListeners.remove(listener);
   }
   
   /**
    * Fires an event to all listeners.
    * 
    * @param event the event to fire.
    */
   public synchronized void fireEvent(Object event)
   {
      Iterator i = mListeners.iterator();
      while(i.hasNext())
      {
         Object listener = i.next();
         
         // store event as a parameter to the listener method 
         Object[] params = new Object[]{event};
         
         // fire message, synchronize on the listener
         MethodInvoker mi =
            new MethodInvoker(listener, mListenerMethod, params);
         mi.execute(listener);
      }
   }
}
