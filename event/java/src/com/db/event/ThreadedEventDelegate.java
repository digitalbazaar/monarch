/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.event;

import java.lang.reflect.Method;
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
    * Constructs a new threaded event delegate.
    */
   public ThreadedEventDelegate()
   {
      // create listener to event queue map
      mListenerToEventQueue = new HashMap();
      
      // create listener to event thread map
      mListenerToEventThread = new HashMap();
   }
   
   /**
    * Processes events for a listener.
    * 
    * @param listener the listener to run the thread for.
    * @param methodName the name of the method to call on the listener. 
    * @param queue the event queue with the listener's events.
    */
   public void processEvents(Object listener, String methodName, Vector queue)
   {
      // create aa event class to method map
      HashMap classToMethod = new HashMap();
      
      while(!Thread.currentThread().isInterrupted())
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
         
         // proceed is thread is not interrupted
         if(!Thread.currentThread().isInterrupted())
         {
            // process events if they exist
            if(events != null)
            {
               Iterator i = events.iterator();
               while(i.hasNext())
               {
                  // get the next event
                  Object event = i.next();
                  
                  // store next event as a parameter to the listener method 
                  Object[] params = new Object[]{event};

                  // get the method for the event class
                  Class eventClass = event.getClass();
                  Method method = (Method)classToMethod.get(eventClass);
                  if(method == null)
                  {
                     // find the method
                     method = MethodInvoker.findMethod(
                        listener, methodName, params);
                     
                     if(method != null)
                     {
                        // add a new class to method map entry
                        classToMethod.put(eventClass, method);
                     }
                  }
                  
                  if(method != null)
                  {
                     // fire message, synchronize on the listener
                     MethodInvoker mi =
                        new MethodInvoker(listener, method, params);
                     mi.execute(listener);
                  }
                  else
                  {
                     // log error
                     LoggerManager.getLogger("dbevent").error(getClass(),
                        "could not find method '" +
                        MethodInvoker.getSignature(methodName, params) +
                        "' in class '" + listener.getClass().getName() + "'");
                  }
               }
            
               // throw out temporary event queue
               events = null;
            }
            
            try
            {
               // sleep
               Thread.sleep(1);
            }
            catch(InterruptedException e)
            {
               // interrupt thread
               Thread.currentThread().interrupt();
            }
         }
      }
   }

   /**
    * Adds a listener.
    *
    * @param listener the listener to add.
    * @param methodName the name of the listener method to call to handle
    *                   an event.
    */
   public synchronized void addListener(Object listener, String methodName)
   {
      if(listener == null)
      {
         throw new IllegalArgumentException(
            "Cannot add a 'null' listener.");
      }

      if(methodName == null)
      {
         throw new IllegalArgumentException(
            "Cannot add a listener with a 'null' method name.");
      }
      
      if(!hasListener(listener))
      {
         // add event queue for listener
         Vector queue = new Vector();
         mListenerToEventQueue.put(listener, queue);
         
         // start event thread for listener
         Object[] params = new Object[]{listener, methodName, queue};
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
      if(hasListener(listener))
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
    * Returns true if the passed listener is already listening for
    * events fired by this delegate, false if not.
    *
    * @param listener the listener to check for.
    * 
    * @return true if the listener is already listening to this delegate,
    *         false if not. 
    */
   public synchronized boolean hasListener(Object listener)
   {
      boolean rval = false;
      
      if(mListenerToEventThread.get(listener) != null)
      {
         rval = true;
      }
      
      return rval;
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
