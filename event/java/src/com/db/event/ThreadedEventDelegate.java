/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.event;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.LoggerManager;
import com.db.util.JobThreadPool;
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
    * A thread that monitors the event queue for new events and dispatches them. 
    */
   protected Thread mEventDispatchThread;
   
   /**
    * A thread pool for threads that process events.
    */
   protected JobThreadPool mProcessEventsThreadPool;
   
   /**
    * A map of listener to EventProcessor.
    */
   protected HashMap<Object, EventProcessor> mListenerToEventProcessor;
   
   /**
    * Constructs a new threaded event delegate.
    */
   public ThreadedEventDelegate()
   {
      // no event dispatch thread yet
      mEventDispatchThread = null;
      
      // create the process events thread pool
      mProcessEventsThreadPool = new JobThreadPool(1);
      
      // set 5 minute time limit on threads
      mProcessEventsThreadPool.setJobThreadExpireTime(300000);
      
      // create listener to event processor map
      mListenerToEventProcessor = new HashMap<Object, EventProcessor>();
   }
   
   /**
    * Stops the event dispatch thread.
    * 
    * @exception Throwable thrown if an exception occurs.
    */
   @Override
   protected void finalize() throws Throwable
   {
      // stop the event dispatch thread
      if(mEventDispatchThread != null)
      {
         mEventDispatchThread.interrupt();
         mEventDispatchThread = null;
      }
      
      // terminate all process events threads
      mProcessEventsThreadPool.terminateAllThreads(1000);
      
      super.finalize();
   }
   
   /**
    * Dispatches any available events for processing. Tells EventProcessors
    * to process their events.
    */
   public void dispatchEvents()
   {
      try
      {
         // keep dispatching until interrupted
         while(!Thread.currentThread().isInterrupted())
         {
            // lock while iterating over event processors
            synchronized(this)
            {
               // go through each event processor
               for(EventProcessor ep: mListenerToEventProcessor.values())
               {
                  // lock on the event processor
                  synchronized(ep)
                  {
                     // if the event processor is not processing events and has
                     // events to process, then process events on a process
                     // events thread from the thread pool
                     if(!ep.isProcessing() && ep.hasEvents())
                     {
                        // create process events job
                        MethodInvoker job = new MethodInvoker(
                           ep, "processEvents");
                        
                        // indicate that the event processor is now in use
                        ep.setProcessing(true);
                        
                        // run the job via the process events thread pool
                        mProcessEventsThreadPool.runJob(job);
                     }
                  }
               }
            }
            
            // sleep for just a moment
            Thread.sleep(1);
         }
      }
      catch(InterruptedException e)
      {
         // ensure interrupted state remains flipped
         Thread.currentThread().interrupt();
      }
   }
   
   /**
    * Adds a listener to this event delegate if it has not been added yet.
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
         // create an EventProcessor
         EventProcessor ep = new EventProcessor(listener, methodName);
         
         // add the listener to EventProcessor mapping
         mListenerToEventProcessor.put(listener, ep);
         
         // set the number of thread available in the process events thread pool
         mProcessEventsThreadPool.setPoolSize(mListenerToEventProcessor.size());
         
         // if the event dispatch thread hasn't started, start it
         if(mEventDispatchThread == null)
         {
            MethodInvoker mi = new MethodInvoker(this, "dispatchEvents");
            mEventDispatchThread = mi;
            mi.backgroundExecute();
         }
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
         // remove listener to EventProcessor mapping
         mListenerToEventProcessor.remove(listener);
         
         // if there are no more listeners, stop the event dispatch thread
         if(mListenerToEventProcessor.size() == 0)
         {
            mEventDispatchThread.interrupt();
            mEventDispatchThread = null;
         }
         
         // set the number of thread available in the process events thread pool
         mProcessEventsThreadPool.setPoolSize(
            Math.max(1, mListenerToEventProcessor.size()));
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
      
      if(mListenerToEventProcessor.get(listener) != null)
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
      // add the event to all of the event processors
      for(EventProcessor ep: mListenerToEventProcessor.values()) 
      {
         ep.addEvent(event);
      }
   }
   
   /**
    * An EventProcessor is an object that is used to process events. Each
    * EventProcessor has an associated listener object that has method(s)
    * that are called to process events. Each listener object has one or
    * more methods with a given method name that can be invoked and passed
    * a single event object.
    * 
    * A listener object may overload its method name for different event
    * classes allowing it to use a different method implementation for each
    * event type. A map is maintained that stores the pre-determined method
    * that should be called for each type of event.
    * 
    * An event queue is used to store all of the events that haven't yet been
    * processed. Whenever it is time to process an event, the event is removed
    * from the queue and processed via the appropriate method on the listener.
    * 
    * Events are always processed in the same order that they are added.
    * 
    * The EventProcessor has a flag that can be flipped externally to indicate
    * that event processing has started. This is done externally so that the
    * event processing can occur, whilst maintaining event order, in a separate
    * process. Once processing has completed, the flag is switched back by
    * the EventProcessor. 
    * 
    * @author Dave Longley
    */
   public class EventProcessor
   {
      /**
       * The listener.
       */
      protected Object mListener;
      
      /**
       * The name of the listener method used to process events.
       */
      protected String mMethodName;

      /**
       * A mapping of event class to method. This map stores pre-determined
       * methods on the listener that should be called based on event type.
       */
      protected HashMap<Class, Method> mEventClassToMethod;
      
      /**
       * The queue that stores the events to be processed by the listener.
       */
      protected List<Object> mEventQueue;
      
      /**
       * Set to true if this EventProcessor is currently processing events,
       * false if not.
       */
      protected boolean mIsProcessing;
      
      /**
       * Creates a new EventProcessor with the given listener and method name.
       * 
       * @param listener the listener for this event processor.
       * @param methodName the name of the listener method(s) to use to
       *                   process events.
       */
      public EventProcessor(Object listener, String methodName)
      {
         // store listener and method name
         mListener = listener;
         mMethodName = methodName;
         
         // create event class to method map
         mEventClassToMethod = new HashMap<Class, Method>();
         
         // create event queue
         mEventQueue = new LinkedList<Object>();
         
         // not processing
         mIsProcessing = false;
      }
      
      /**
       * Processes all events in the passed queue.
       * 
       * @param queue the event queue with the events to process.
       */
      protected void processEvents(List<Object> queue)
      {
         // process all of the events in the passed queue
         for(Object event: queue)
         {
            // proceed if thread is not interrupted
            if(!Thread.currentThread().isInterrupted())
            {
               // get the method for the event class
               Class eventClass = event.getClass();
               Method method = mEventClassToMethod.get(eventClass);
               if(method == null)
               {
                  // find the method
                  method = MethodInvoker.findMethod(
                     mListener, mMethodName, event);
                  if(method != null)
                  {
                     // add a new class to method map entry
                     mEventClassToMethod.put(eventClass, method);
                  }
               }
               
               if(method != null)
               {
                  // process event, synchronize on the listener
                  MethodInvoker mi = new MethodInvoker(
                     mListener, method, event);
                  mi.execute(mListener);
               }
               else
               {
                  // no appropriate method found, log error
                  LoggerManager.getLogger("dbevent").error(getClass(),
                     "could not find method '" +
                     MethodInvoker.getSignature(mMethodName, event) +
                     "' in class '" + mListener.getClass().getName() + "'");
               }
            }
            else
            {
               // break, thread interrupted
               break;
            }
         }
      }
      
      /**
       * Adds an event to the event queue for processing.
       * 
       * @param event the event to add to the event queue for processing.
       */
      public void addEvent(Object event)
      {
         // synchronize on the event queue
         synchronized(mEventQueue)
         {
            mEventQueue.add(event);
         }
      }
      
      /**
       * Processes all events in the event queue.
       */
      public void processEvents()
      {
         // create a temporary event queue
         List<Object> queue = new LinkedList<Object>();
         
         // synchronize on the event queue
         synchronized(mEventQueue)
         {
            // move all events into the temporary queue for processing
            queue.addAll(mEventQueue);
            
            // clear the event queue
            mEventQueue.clear();
         }
         
         // process the events in the temporary queue
         processEvents(queue);
         
         // no longer processing events
         mIsProcessing = false;
      }
      
      /**
       * Sets whether or not this EventProcessor is processing events.
       * 
       * @param processing true if this EventProcessor is processing events,
       *                   false if not.
       */
      public synchronized void setProcessing(boolean processing)
      {
         mIsProcessing = processing;
      }
      
      /**
       * Returns true if this EventProcessor is currently processing events,
       * false if not.
       *
       * @return true if this EventProcessor is currently processing events,
       *         false if not.
       */
      public synchronized boolean isProcessing()
      {
         return mIsProcessing;
      }
      
      /**
       * Returns true if this EventProcessor has events to process.
       *
       * @return true if this EventProcessor has events to process, false
       *         if not. 
       */
      public synchronized boolean hasEvents()
      {
         return !mEventQueue.isEmpty();
      }
   }
}
