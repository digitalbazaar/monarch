/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.Crc16;

/**
 * A unit test for the Crc16.
 * 
 * @author Dave Longley
 */
public class UTCrc16
{
   /**
    * Runs the unit test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up loggers
         LoggerManager.setFile("dbutil", "ut-crc16.log", false);
         LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
         
         String s = new String("hello");
         byte[] b = s.getBytes();
         
         Crc16 crc16 = new Crc16();
         crc16.update(b);
         short c = (short)crc16.getValue();
         System.out.println("crc=" + (c & 0xffff));
         System.out.println("crc hex=" + Integer.toHexString(c & 0xffff));
         
         // CRC should be 38c5 in hex
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
