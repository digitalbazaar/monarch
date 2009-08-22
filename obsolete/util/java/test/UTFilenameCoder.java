/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.File;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.util.FilenameCoder;

/**
 * A unit test for the FilenameCoder.
 * 
 * @author Dave Longley
 */
public class UTFilenameCoder
{
   /**
    * Runs the FilenameCoder test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbutil", "ut-filenamecoder.log", false);
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
         
         String filename = "Lynn Whatever - 6:10 A.M..mp3";
         
         // create a FilenameCoder
         FilenameCoder fc = new FilenameCoder();
         
         // encode the filename
         filename = fc.encode(filename);
         System.out.println(filename);

         File file = new File(filename);
         if(file.createNewFile())
         {
            File newFile = new File(filename);
            System.out.println("exists1=" + file.exists());
            System.out.println("exists2=" + newFile.exists());
            newFile.delete();
            file.delete();
            System.out.println("exists1=" + file.exists());
            System.out.println("exists2=" + newFile.exists());
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
