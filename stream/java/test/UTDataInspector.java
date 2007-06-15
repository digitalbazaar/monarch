/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.FileInputStream;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.stream.DataInspector;
import com.db.stream.InspectorInputStream;

/**
 * A unit test for the DataInspector interface and InspectorInputStream class.
 * 
 * @author Dave Longley
 */
public class UTDataInspector
{
   /**
    * A test inspector that inspects all of the data passed to it.
    * 
    * @author Dave Longley
    */
   public static class WholeInspector implements DataInspector
   {
      /**
       * The number of inspected bytes.
       */
      public int inspected = 0;
      
      /**
       * Inspects the data in the passed buffer for some implementation
       * specific attributes. This method returns the number of bytes that
       * were successfully inspected such that the passed buffer can safely
       * clear that number of bytes. 
       * 
       * An inspector should treat subsequent calls to this method as if the
       * data in the passed buffer is consecutive in nature (i.e. read
       * from a stream).
       * 
       * @param b the buffer with data to inspect.
       * @param offset the offset at which to begin inspecting bytes.
       * @param length the maximum number of bytes to inspect.
       * 
       * @return the number of bytes that were inspected in the passed buffer.
       */
      public int inspectData(byte[] b, int offset, int length)
      {
         inspected += length;
         
         System.out.println(
            "WholeInspector: inspected " + length + " byte(s).");
//         LoggerManager.getLogger("dbstream").debug(
//            "WholeInspector: inspected " + length + " byte(s).");
         
         // return inspected bytes
         return length;
      }
   }
   
   /**
    * A test inspector that can only inspect half of the data given
    * to it.
    * 
    * @author Dave Longley
    */
   public static class HalfInspector implements DataInspector
   {
      /**
       * The number of inspected bytes.
       */
      public int inspected = 0;
      
      /**
       * Inspects the data in the passed buffer for some implementation
       * specific attributes. This method returns the number of bytes that
       * were successfully inspected such that the passed buffer can safely
       * clear that number of bytes. 
       * 
       * An inspector should treat subsequent calls to this method as if the
       * data in the passed buffer is consecutive in nature (i.e. read
       * from a stream).
       * 
       * @param b the buffer with data to inspect.
       * @param offset the offset at which to begin inspecting bytes.
       * @param length the maximum number of bytes to inspect.
       * 
       * @return the number of bytes that were inspected in the passed buffer.
       */
      public int inspectData(byte[] b, int offset, int length)
      {
         length = Math.max(1, length / 2);
         
         inspected += length;
         
         System.out.println(
            "HalfInspector: inspected " + length + " byte(s).");
//         LoggerManager.getLogger("dbstream").debug(
//            "HalfInspector: inspected " + length + " byte(s).");
         
         // return inspected bytes
         return length;
      }
   }
   
   /**
    * A test inspector that can only inspect a quarter of the data given
    * to it.
    * 
    * @author Dave Longley
    */
   public static class QuarterInspector implements DataInspector
   {
      /**
       * The number of inspected bytes.
       */
      public int inspected = 0;
      
      /**
       * Inspects the data in the passed buffer for some implementation
       * specific attributes. This method returns the number of bytes that
       * were successfully inspected such that the passed buffer can safely
       * clear that number of bytes. 
       * 
       * An inspector should treat subsequent calls to this method as if the
       * data in the passed buffer is consecutive in nature (i.e. read
       * from a stream).
       * 
       * @param b the buffer with data to inspect.
       * @param offset the offset at which to begin inspecting bytes.
       * @param length the maximum number of bytes to inspect.
       * 
       * @return the number of bytes that were inspected in the passed buffer.
       */
      public int inspectData(byte[] b, int offset, int length)
      {
         length = Math.max(1, length / 4);
         
         inspected += length;
         
         System.out.println(
            "QuarterInspector: inspected " + length + " byte(s).");
//         LoggerManager.getLogger("dbstream").debug(
//            "QuarterInspector: inspected " + length + " byte(s).");
         
         // return inspected bytes
         return length;
      }
   }
   
   /**
    * Runs a simple inspector test.
    */
   public static void runInspectorTest()
   {
      System.out.println("Running inspector test...");
      
      try
      {
         // set up logger
         LoggerManager.setFile("dbstream", "ut-datainspector.log", false);
         LoggerManager.setFileVerbosity("dbstream", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbstream", Logger.ERROR_VERBOSITY);
         
         String src = "test/data/test.mp3";
         
         // open file
         FileInputStream fis = new FileInputStream(src);
         
         // create inspector input stream
         InspectorInputStream iis = new InspectorInputStream(fis);
         
         // add inspectors
         iis.addInspector("WholeInspector", new WholeInspector());
         iis.addInspector("HalfInspector", new HalfInspector());
         //iis.addInspector("QuarterInspector", new QuarterInspector());
         
         // create buffer for reading
         byte[] b = new byte[2048];
         
         // read file
         long time = System.currentTimeMillis();
         while(iis.read(b) != -1);
         time = System.currentTimeMillis() - time;
         
         System.out.println("Inspect time=" + time);
         System.out.println("WholeInspector total=" + 
            ((WholeInspector)iis.getInspector("WholeInspector")).inspected);
//         LoggerManager.getLogger("dbstream").debug("WholeInspector total=" + 
//            ((WholeInspector)iis.getInspector("WholeInspector")).inspected);
         System.out.println("HalfInspector total=" + 
            ((HalfInspector)iis.getInspector("HalfInspector")).inspected);
//         LoggerManager.getLogger("dbstream").debug("HalfInspector total=" + 
//            ((HalfInspector)iis.getInspector("HalfInspector")).inspected);
//         System.out.println("QuarterInspector total=" + 
//            ((QuarterInspector)iis.getInspector("QuarterInspector")).inspected);
//         LoggerManager.getLogger("dbstream").debug("QuarterInspector total=" + 
//            ((QuarterInspector)iis.getInspector("QuarterInspector")).inspected);
         
         // close stream
         iis.close();
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
      
      System.out.println("Inspector test complete.");
   }
   
   /**
    * Runs the DataInspector test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      // run inspector test
      runInspectorTest();
   }
}
