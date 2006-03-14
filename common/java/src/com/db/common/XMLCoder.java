/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

/**
 * This class provides the ability to encode and decode using
 * XML encoding.
 * 
 * @author Dave Longley
 */
public class XMLCoder
{
   private static XMLCoder mInstance = new XMLCoder();
   
   /**
    * Default constructor
    */
   protected XMLCoder() {}

   public static XMLCoder getInstance()
   {
      return mInstance;
   }

   public static String encode(String string)
   {
      if(string == null)
      {
         string = "";
      }
      else if(!string.equals(""))
      {
         string = string.replaceAll("&", "&amp;");
         string = string.replaceAll("\"", "&quot;");
         string = string.replaceAll("'", "&apos;");
         string = string.replaceAll("<", "&lt;");
         string = string.replaceAll(">", "&gt;");
      }
      
      return string;
   }
   
   public static String decode(String string)
   {
      if(string == null)
      {
         string = "";
      }
      else if(!string.equals(""))
      {
         string = string.replaceAll("&gt;", ">");
         string = string.replaceAll("&lt;", "<");
         string = string.replaceAll("&apos;", "'");
         string = string.replaceAll("&quot;", "\"");
         string = string.replaceAll("&amp;", "&");
      }      

      return string;
   }
}
