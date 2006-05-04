/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.autoupdater.AutoUpdateable;

/**
 * A simple custom AutoUpdateable application.
 * 
 * @author Dave Longley
 */
public class CustomAutoUpdateable implements AutoUpdateable
{
   /**
    * Stores whether or not this application should be restarted.
    */
   protected boolean mShouldRestart;
   
   /**
    * The execution thread.
    */
   protected Thread mExecutionThread;

   /**
    * Creates a new CustomAutoUpdateable.
    */
   public CustomAutoUpdateable()
   {
      mShouldRestart = true;
      mExecutionThread = null;
   }
   
   /**
    * Executes this auto-updateable application with the given parameters.
    * 
    * This method should load all of the necessary jar files to execute
    * this application.
    * 
    * @param args the parameters to execute this auto-updateable application
    *             with.
    */
   public void execute(String[] args)
   {
      try
      {
         mExecutionThread = Thread.currentThread();
         
         // sleep for 60 seconds
         Thread.sleep(60000);
         
         // execution completed, should not restart
         mShouldRestart = false;
      }
      catch(InterruptedException e)
      {
         // execution interrupted, should restart
         mShouldRestart = true;
      }
   }
   
   /**
    * Shuts down this auto-updateable application.
    */
   public void shutdown()
   {
      // interrupt execution thread
      if(mExecutionThread != null)
      {
         mExecutionThread.interrupt();
      }
   }
   
   /**
    * Returns true if this application should be restarted, false if not.
    * 
    * @return true if this application should be restarted, false if not.
    */
   public boolean shouldRestart()
   {
      return mShouldRestart;
   }
}
