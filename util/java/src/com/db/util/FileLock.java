/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.FileChannel;

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
    * The channel file lock (java.nio.channels.FileLock) if this FileLock
    * as a lock, null otherwise.
    */
   protected java.nio.channels.FileLock mChannelLock;
   
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
      
      // no channel lock yet
      mChannelLock = null;
   }
   
   /**
    * Gets the channel lock.
    * 
    * @return the channel lock.
    */
   protected java.nio.channels.FileLock getChannelLock()
   {
      return mChannelLock;
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
    * Tries to acquire a lock for this FileLock.
    * 
    * @return true if this FileLock is now locked, false if a lock could not
    *         be acquired.
    */
   public synchronized boolean tryLock()
   {
      // ensure lock has not already been acquired
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
               
               // get a file channel
               FileChannel channel =
                  new RandomAccessFile(getFile(), "rw").getChannel();
               
               // try to get a lock on the channel
               mChannelLock = channel.tryLock();
            }
         }
         catch(IOException e)
         {
            // see if the file was created
            if(created)
            {
               // log the bug, there was an error
               getLogger().debug(getClass(), Logger.getStackTrace(e));
               
               // delete the file
               getFile().delete();
            }
            else
            {
               // no need to log the exception, a lock was only tried
            }
         }
      }
      
      return hasLock();
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
               
               // get a file channel
               FileChannel channel =
                  new RandomAccessFile(getFile(), "rw").getChannel();
               
               // lock on the channel
               mChannelLock = channel.lock();
            }
         }
         catch(AsynchronousCloseException e)
         {
            // another thread closed the channel, so try the lock again
            getLogger().debug(getClass(), Logger.getStackTrace(e));
            lock();
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
         try
         {
            // release the channel lock
            getChannelLock().release();
         }
         catch(IOException e)
         {
            // ignore this exception, only occurs if the channel is no
            // longer open which means that no lock exists anyway
            getLogger().debug(getClass(), Logger.getStackTrace(e));
         }
         
         mChannelLock = null;
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
      
      // has a lock if the channel lock is not null, the file exists, and
      // the channel is open
      if(getChannelLock() != null)
      {
         if(getFile().exists() && getChannelLock().channel().isOpen())
         {
            // channel is open and locked
            rval = true;
         }
         else
         {
            // throw out the lock, it is invalid because the channel isn't open
            mChannelLock = null;
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
