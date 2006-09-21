/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.File;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.util.FileLock;

/**
 * A unit test for the FileLock class.
 * 
 * @author Dave Longley
 */
public class UTFileLock
{
   /**
    * Runs the FileLock test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbutil", "ut-filelock.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
         
         String filename = "lockfile.lock";
         
         // get a file lock
         FileLock fileLock = new FileLock(filename);
         
         // lock the file
         if(fileLock.tryLock())
         {
            System.out.println("lock acquired");
            fileLock.unlock();
            System.out.println("file unlocked.");
         }
         else
         {
            System.out.println("lock not acquired!");
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
