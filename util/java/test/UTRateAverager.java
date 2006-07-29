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
         smRateAverager = new RateAverager(2000);
         
         // start rate averager
         smRateAverager.start();
         
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(760, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(811, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(811, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(760, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(811, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(760, 15);
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         
         // windows start moving here, rates should stay the same
         printRate();
         smRateAverager.addRate(819, 16);
         printRate();
         smRateAverager.addRate(768, 15);
         printRate();
         smRateAverager.addRate(820, 16);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(768, 15);
         smRateAverager.addRate(819, 16);
         smRateAverager.addRate(819, 16);
         
         // print rate
         printRate();
         
         /*
         // print rate
         printRate();
         
         // add rate
         smRateAverager.addRate(50, 250);

         // print rate
         printRate();
         
         // add rate
         smRateAverager.addRate(49, 250);

         // print rate
         printRate();
         
         // add rate
         smRateAverager.addRate(50, 250);

         // print rate
         printRate();
         
         // add rate
         smRateAverager.addRate(50, 250);

         // print rate
         printRate();

         // add rate
         smRateAverager.addRate(51, 250);

         // print rate
         printRate();

         // add rate
         smRateAverager.addRate(50, 250);

         // print rate
         printRate();*/

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
