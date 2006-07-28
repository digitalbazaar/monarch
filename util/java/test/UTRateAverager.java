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
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }      
   }
}
