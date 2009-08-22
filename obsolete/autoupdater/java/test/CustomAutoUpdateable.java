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
      mShouldRestart = false;
      mExecutionThread = null;
   }
   
   /**
    * Executes this auto-updateable application with the given parameters.
    * 
    * This method should load all of the necessary jar files to execute
    * this application.
    */
   public void execute()
   {
      try
      {
         mExecutionThread = Thread.currentThread();
         
         // sleep for 60 seconds
         Thread.sleep(60000);
      }
      catch(InterruptedException ignore)
      {
      }
   }
   
   /**
    * Returns true while this auto-updateable application is running,
    * false while it is not running.
    * 
    * @return true while this auto-updateable application is running,
    *         false while it is not running.
    */
   public boolean isRunning()
   {
      boolean rval = false;
      
      if(mExecutionThread != null)
      {
         rval = mExecutionThread.isAlive();
      }
      
      return rval;
   }
   
   /**
    * Processes arguments for this auto-updateable application.
    * 
    * @param args the arguments to process.
    */
   public void processArguments(String[] args)
   {
      System.out.println("Processing arguments...");
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
      
      // should restart
      mShouldRestart = true;
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
   
   /**
    * Gets the current version of this application.
    *  
    * @return the current version of this application.
    */
   public String getVersion()
   {
      return "1.0";
   }
}
