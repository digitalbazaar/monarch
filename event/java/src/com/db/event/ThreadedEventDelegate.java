/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.event;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.LoggerManager;
import com.db.util.MethodInvoker;

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
    * A map of listener to the method to call to handle an event.
    */
   protected HashMap mListenerToMethod;

   /**
    * Constructs a new threaded event delegate.
    */
   public ThreadedEventDelegate()
   {
      // create listener to event queue map
      mListenerToEventQueue = new HashMap();
      
      // create listener to event thread map
      mListenerToEventThread = new HashMap();
      
      // create listener to method map
      mListenerToMethod = new HashMap();
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
         // lock and get the event queue and method for the listener
         Vector queue = null;
         String method = null;
         synchronized(this)
         {
            // get the event queue for the listener
            queue = (Vector)mListenerToEventQueue.get(listener);
            
            // get the listener method
            method = (String)mListenerToMethod.get(listener);
         }
         
         if(queue != null && method != null)
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
                     new MethodInvoker(listener, method, params);
                  mi.execute(listener);
               }
            
               // throw out temporary event queue
               events = null;
            }
         }
         else if(method == null)
         {
            try
            {
               throw new NullPointerException(
                  "Cannot call 'null' method on listener " +
                  listener.getClass().getName() + " in " +
                  getClass().getName());
            }
            catch(Throwable t)
            {
               LoggerManager.error("dbevent", t.getMessage());
               LoggerManager.debug("dbevent",
                  LoggerManager.getStackTrace(t));
            }
         }
         
         try
         {
            // sleep
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
    * @param method the name of the listener method to call to handle an event.
    */
   public synchronized void addListener(Object listener, String method)
   {
      if(mListenerToEventThread.get(listener) == null)
      {
         // add event queue for listener
         mListenerToEventQueue.put(listener, new Vector());
         
         // add method for listener
         mListenerToMethod.put(listener, method);
            
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
         mListenerToMethod.remove(listener);
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
            queue.add(event);
         }
      }
   }
}
