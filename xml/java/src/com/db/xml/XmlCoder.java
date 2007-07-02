/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

/**
 * This class provides the ability to encode and decode using
 * XML encoding.
 * 
 * @author Dave Longley
 */
public class XmlCoder
{
   /**
    * The single instance of the XMLCoder.
    */
   private static XmlCoder smInstance = new XmlCoder();
   
   /**
    * Creates the XmlCoder singleton instance.
    */
   protected XmlCoder()
   {
   }

   /**
    * Gets the XmlCoder singleton instance.
    * 
    * @return the XmlCoder singleton instance.
    */
   public static XmlCoder getInstance()
   {
      return smInstance;
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
         try
         {
            string = new String(string.getBytes(), "UTF-8");
         }
         catch(Exception ignore)
         {
            // utf-8 is supported
         }
         
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
         
         try
         {
            string = new String(string.getBytes("UTF-8"));
         }
         catch(Exception ignore)
         {
            // utf-8 is supported
         }
      }
      
      return string;
   }
}
