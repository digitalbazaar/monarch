/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
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
   /**
    * The single instance of the XMLCoder.
    */
   private static XMLCoder mInstance = new XMLCoder();
   
   /**
    * Creates the XMLCoder.
    */
   protected XMLCoder()
   {
   }

   /**
    * Gets the XMLCoder singleton instance.
    * 
    * @return the XMLCoder singleton instance.
    */
   public static XMLCoder getInstance()
   {
      return mInstance;
   }

   /**
    * XML encodes a string.
    * 
    * @param string the string to XML encode.
    * 
    * @return the XML encoded string. 
    */
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
   
   /**
    * XML decodes a string.
    * 
    * @param string the string to XML decode.
    * 
    * @return the XML decoded string.
    */
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
