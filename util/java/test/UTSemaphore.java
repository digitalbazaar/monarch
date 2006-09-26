/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.*;
import com.db.util.*;

/**
 * This class tests the Semaphore class.
 * 
 * @author Dave Longley
 */
public class UTSemaphore
{
   /**
    * A universally accessible semaphore for testing.
    */
   public static Semaphore smSemaphore;

   /**
    * Runs the semaphore test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         System.out.println("\nTesting Semaphore...");
         System.out.println("------------------\n");
         
         // set up logger
         LoggerManager.setFile("dbutil", "ut-semaphore.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         
         // create semaphore
         smSemaphore = new Semaphore(0, true);
         
         // print semaphore permit info
         printSemaphorePermitInfo();
         
         // fire background thread to set permits
         MethodInvoker mi = new MethodInvoker(
            UTSemaphore.class, "setPermitCount");
         mi.backgroundExecute();

         System.out.println("releasing 2 permits...");
         
         // release permits
         smSemaphore.release(2);
         
         System.out.println("Permits released.");

         // print semaphore permit info
         printSemaphorePermitInfo();

         System.out.println("acquiring 2 permits...");

         // acquire permits
         smSemaphore.acquire(2);
         
         System.out.println("2 permits acquired.");
         
         // print semaphore permit info
         printSemaphorePermitInfo();

         System.out.println("Setting permit count 7 in foreground...");
         smSemaphore.setMaxPermitCount(7);
         System.out.println("Permit count 7 set in foreground.");
         
         // print semaphore permit info
         printSemaphorePermitInfo();
         
         System.out.println("Setting permit count 1 in foreground...");
         smSemaphore.setMaxPermitCount(1);
         System.out.println("Permit count 1 set in foreground.");
         
         // print semaphore permit info
         printSemaphorePermitInfo();
         
         // fire background thread to set permits
         mi = new MethodInvoker(
            UTSemaphore.class, "setPermitCount", null);
         mi.backgroundExecute();
         
         System.out.println("Acquiring 1 permit...");
         
         smSemaphore.acquire();
         
         System.out.println("1 permit acquired.");
         
         // print semaphore permit info
         printSemaphorePermitInfo();

         System.out.println("releasing 4 permits...");
         
         // release permits
         smSemaphore.release(4);
         
         System.out.println("4 permits released.");

         // print semaphore permit info
         printSemaphorePermitInfo();
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * Prints the semaphore permit information.
    */
   public static void printSemaphorePermitInfo()
   {
      System.out.println("\nSemaphore Permit Info:\n");
      
      System.out.println(
         "MAX permit count=" + smSemaphore.getMaxPermitCount());
      System.out.println(
         "available permits=" + smSemaphore.availablePermits());
      System.out.println(
         "used permits=" + smSemaphore.usedPermits());
      
      System.out.println();
   }
   
   /**
    * Sets the number of permits for the semaphore.
    */
   public static void setPermitCount()
   {
      try
      {
         Thread.sleep(1000);
      }
      catch(Throwable t)
      {
      }
      
      System.out.println("Setting permit count 3 in background...");

      smSemaphore.setMaxPermitCount(3);
      
      System.out.println("Permit count 3 set in background.");
   }
}
