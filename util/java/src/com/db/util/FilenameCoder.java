/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A FilenameCoder is used to encode filenames for particular operating
 * systems. It translates filename strings that contain characters that
 * cannot be used in filenames on certain operating systems. 
 * 
 * @author Dave Longley
 */
public class FilenameCoder
{
   /**
    * Creates a new FilenameCoder.
    */
   public FilenameCoder()
   {
   }
   
   /**
    * Encodes the passed RELATIVE filename for use on the current operating
    * system. This method will translate the passed filename so that it can be
    * written to the filesystem for the current operating system.
    * 
    * Do not run this method on a full path name -- it will remove the
    * path separator characters. 
    *
    * @param filename the filename to encode.
    * 
    * @return the encoded filename.
    */
   public String encode(String filename)
   {
      String rval = filename;
      
      // get the operating system name
      String osName = System.getProperty("os.name");
      
      if(osName.startsWith("Mac OS")) 
      {
         // operating system is Mac OS
         rval = filename.replaceAll(
            "(/)|(:)|(\\*)|(\\?)|(\\<)|(\\>)|(\\|)", "_");
      }
      else if(osName.startsWith("Windows"))
      {
         // operating system is Windows
         rval = filename.replaceAll(
            "(\\\\)|(/)|(:)|(\\*)|(\\?)|(\\<)|(\\>)|(\\|)", "_");
      }
      else 
      { 
         // assume operating system is Unix or Linux
         rval = filename.replaceAll(
            "(/)|(:)|(\\*)|(\\?)|(\\<)|(\\>)|(\\|)", "_");
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this FilenameCoder.
    * 
    * @return the logger for this FilenameCoder.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbutil");
   }
}
