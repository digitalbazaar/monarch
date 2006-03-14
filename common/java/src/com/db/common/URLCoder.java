/*
 * Copyright (c) 2003 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.LoggerManager;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.net.URLDecoder;

/**
 * This class provides the ability to encode and decode using
 * URL encoding.
 * 
 * @author Dave Longley
 */
public class URLCoder
{
   private static URLCoder mInstance = new URLCoder();
   
   /**
    * Default constructor
    */
   protected URLCoder() {}

   public static URLCoder getInstance()
   {
      return mInstance;
   }

   public static String encode(String string)
   {
      String encoded = "";
      
      if(string != null && !string.equals(""))
      {
         try
         {
            // encode the string
            encoded = URLEncoder.encode(string, "UTF-8");
         }
         catch(UnsupportedEncodingException uee)
         {
            LoggerManager.error("dbcommon", "UTF-8 encoding not supported.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(uee));
         }
      }

      return encoded;
   }
   
   public static String decode(String string)
   {
      String decoded = "";
      
      if(string != null && !string.equals(""))
      {
         try
         {
            // decode the string
            decoded = URLDecoder.decode(string, "UTF-8");
         }
         catch(UnsupportedEncodingException uee)
         {
            LoggerManager.error("dbcommon", "UTF-8 encoding not supported.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(uee));
         }
      }

      return decoded;
   }
}
