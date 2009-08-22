/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.File;
import java.io.IOException;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A FileLock is an object that is used to create a lock file that is used
 * to lock the use of some resource.
 * 
 * A common use for this object is to prevent multiple instances of the
 * same application from being run. 
 * 
 * The file this FileLock uses will be automatically deleted (if it was
 * created by this object) when this object goes out of scope or the JVM
 * that created it exists.
 * 
 * @author Dave Longley
 */
public class FileLock
{
   /**
    * The file for this file lock.
    */
   protected File mFile;
   
   /**
    * True if this FileLock has a lock, false if not.
    */
   protected boolean mHasLock;
   
   /**
    * Creates a new FileLock that uses the file with the given filename for
    * locking.
    * 
    * @param filename the name of the file to use with this lock.
    */
   public FileLock(String filename)
   {
      this(new File(filename));
   }

   /**
    * Creates a new FileLock that uses the specified File for locking.
    * 
    * @param file the file to use for locking.
    */
   public FileLock(File file)
   {
      // store the file
      mFile = file;
      
      // no lock yet
      mHasLock = false;
   }
   
   /**
    * When disposing, this object deletes the file that this FileLock uses if
    * it has the lock, i.e. calls unlock().
    */
   public void dispose()
   {
      // unlock
      unlock();
   }
   
   /**
    * Locks this FileLock, if it is not already locked. This method will
    * block until the lock can be acquired.
    * 
    * @return true if this FileLock successfully locked (or was already
    *         locked), false if it was not because it was already locked by
    *         another process or because the file could not be created.
    */
   public synchronized boolean lock()
   {
      if(!hasLock())
      {
         // set to true if the file was created
         boolean created = false;
         
         try
         {
            // create the file, ensure the file doesn't already exist
            created = getFile().createNewFile();
            if(created)
            {
               // delete the file on exit
               getFile().deleteOnExit();
               
               // lock acquired
               mHasLock = true;
            }
         }
         catch(IOException e)
         {
            getLogger().debug(getClass(), Logger.getStackTrace(e));
            
            // if the file was created, delete it
            if(created)
            {
               getFile().delete();
            }
         }         
      }
      
      return hasLock();
   }
   
   /**
    * Unlocks this FileLock, if it has a lock.
    */
   public synchronized void unlock()
   {
      if(hasLock())
      {
         // delete the file
         getFile().delete();
         
         // release the lock
         mHasLock = false;
      }
   }

   /**
    * Checks to see if this FileLock is locked. 
    * 
    * @return true if this FileLock has a lock, false if it does not.
    */
   public synchronized boolean hasLock()
   {
      boolean rval = false;
      
      // has a lock mHasLock is true and the file exists
      if(mHasLock)
      {
         if(getFile().exists())
         {
            rval = true;
         }
         else
         {
            // throw out the lock, it is invalid because the file doesn't exist
            mHasLock = false;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the file that this FileLock uses for locking.
    * 
    * @return the file that this FileLock uses for locking.
    */
   public File getFile()
   {
      return mFile;
   }
   
   /**
    * Gets the logger for this FileLock.
    * 
    * @return the logger for this FileLock.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
