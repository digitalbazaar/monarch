/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * This class is used to delegate events to listeners on separate threads.
 * Each listener has its own thread that it receives and processes events on.
 * 
 * @author Dave Longley
 */
public class ThreadedEventDelegate
{
   /**
    * A map of listener to its event queue.
    */
   protected HashMap mListenerToEventQueue;
   
   /**
    * A map of listener to its event thread.
    */
   protected HashMap mListenerToEventThread;
   
   /**
    * The name of the listener method to call to process an event.
    */
   protected String mListenerMethod;

   /**
    * Constructs a new threaded event delegate.
    * 
    * @param listenerMethod the name of the listener method to call to
    *                       process an event.
    */
   public ThreadedEventDelegate(String listenerMethod)
   {
      // create listener to event queue map
      mListenerToEventQueue = new HashMap();
      
      // create listener to event thread map
      mListenerToEventThread = new HashMap();
      
      // set listener method
      mListenerMethod = listenerMethod;
   }
   
   /**
    * Processes events for a listener.
    * 
    * @param listener the listener to run the thread for.
    */
   public void processEvents(Object listener)
   {
      while(!Thread.interrupted())
      {
         // get the event queue for the listener
         Vector queue = (Vector)mListenerToEventQueue.get(listener);
         if(queue != null)
         {
            // pull all of the events out of the queue and store
            // them in a temporary event queue
            Vector events = null;
            
            // lock on the queue, get its events, and clear it
            synchronized(queue)
            {
               if(queue.size() > 0)
               {
                  events = new Vector();
                  events.addAll(queue);
                  queue.clear();
               }
            }
            
            // process events if they exist
            if(events != null)
            {
               Iterator i = events.iterator();
               while(i.hasNext())
               {
                  // store next event as a parameter to the listener method 
                  Object[] params = new Object[]{i.next()};
                  
                  // fire message, synchronize on the listener
                  MethodInvoker mi =
                     new MethodInvoker(listener, mListenerMethod, params);
                  mi.execute(listener);
               }
            
               // throw out temporary event queue
               events = null;
            }
         }
         
         // sleep
         try
         {
            Thread.sleep(1);
         }
         catch(Throwable ignore)
         {
            // interrupt thread
            Thread.currentThread().interrupt();
         }
      }
   }

   /**
    * Adds a listener.
    *
    * @param listener the listener to add.
    */
   public synchronized void addListener(Object listener)
   {
      if(mListenerToEventThread.get(listener) == null)
      {
         // add event queue for listener
         mListenerToEventQueue.put(listener, new Vector());
            
         // start event thread for listener
         Object[] params = new Object[]{listener};
         MethodInvoker mi = new MethodInvoker(this, "processEvents", params);
         mListenerToEventThread.put(listener, mi);
         mi.backgroundExecute();
      }
   }
   
   /**
    * Removes a listener.
    * 
    * @param listener the listener to remove.
    */
   public synchronized void removeListener(Object listener)
   {
      if(mListenerToEventThread.get(listener) != null)
      {
         // interrupt listener event thread
         Thread thread = (Thread)mListenerToEventThread.get(listener);
         thread.interrupt();
            
         // remove listener from maps
         mListenerToEventQueue.remove(listener);
         mListenerToEventThread.remove(listener);
      }
   }
   
   /**
    * Fires an event to all listeners.
    * 
    * @param event the event to fire.
    */
   public synchronized void fireEvent(Object event)
   {
      Iterator i = mListenerToEventThread.keySet().iterator();
      while(i.hasNext())
      {
         Object listener = i.next();
         
         // get the event queue for the listener
         Vector queue = (Vector)mListenerToEventQueue.get(listener);
         
         // lock on the queue and push an event onto it
         synchronized(queue)
         {
            queue.add(0, event);
         }
      }
   }
}
