/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.*;
import com.db.util.*;

/**
 * A unit test for testing a RateAverager.
 * 
 * @author Dave Longley
 */
public class UTRateAverager
{
   /**
    * A universally accessible RateAverager for testing.
    */
   public static RateAverager smRateAverager;
   
   /**
    * Prints out the current rate for the rate averager.
    */
   public static void printRate()
   {
      // print out the current rate
      System.out.println("rate=" + smRateAverager.getCurrentRate());
   }
   
   /**
    * Prints out the total rate for the rate averager.
    */
   public static void printTotalRate()   
   {
      // print out the total rate
      System.out.println("total rate=" + smRateAverager.getTotalRate());
   }

   /**
    * Runs the rate averager test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbutil", "ut-rateaverager.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         
         // create the rate averager
         smRateAverager = new RateAverager(100, 1);
         
         // start rate averager
         smRateAverager.start();
         
         // print rate
         printRate();
         
         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();
         
         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();

         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();
         
         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();

         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();

         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();
         
         // increase item count
         Thread.sleep(4);
         smRateAverager.increaseItemCount(4);
         
         // print rate
         printRate();

         // increase item count
         Thread.sleep(1);
         smRateAverager.increaseItemCount(1);
         
         // print rate
         printRate();
         
         // stop rate averager
         smRateAverager.stop();
         
         // print total rate
         printTotalRate();
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
