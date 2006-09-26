/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;

/**
 * This class provides static methods for manipulating strings in useful ways.
 * 
 * @author Dave Longley
 */
public class StringTools
{
   /**
    * Splits the passed string according to the given regular expression
    * and puts the split parts into the passed collection.
    * 
    * @param str the string to split.
    * @param regex the regular expression to use.
    * @param collection the collection to put the split parts into.
    */
   public static void split(
      String str, String regex, Collection<String> collection)
   {
      String[] split = str.split(regex);
      for(int i = 0; i < split.length; i++)
      {
         collection.add(split[i]);
      }
   }
   
   /**
    * Glues the objects in the specified collection together using the
    * specified separator. Each object's string representation is obtained
    * by calling String.valueOf(object).
    * 
    * @param collection the collection to glue together.
    * @param separator the separator to use.
    * 
    * @return the string of glued objects from the collection.
    */
   public static String glue(
      Collection<? extends Object> collection, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(Object obj: collection)
      {
         if(sb.length() > 0)
         {
            sb.append(separator);
         }
         
         sb.append(String.valueOf(obj));
      }
      
      return sb.toString();
   }
   
   /**
    * Glues the objects in the specified array together using the
    * specified separator. Each object's string representation is obtained
    * by calling String.valueOf(object).
    * 
    * @param array the array to glue together.
    * @param separator the separator to use.
    * 
    * @return the string of glued objects from the collection.
    */
   public static String glue(Object[] array, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(int i = 0; i < array.length; i++)
      {
         if(i > 0)
         {
            sb.append(separator);
         }
         
         sb.append(String.valueOf(array[i]));
      }
      
      return sb.toString();      
   }
   
   /**
    * Creates a string of repeated content where each content string is
    * separated by a separation string. An example of the use of this method
    * is for generating a string of question marks ('?') for a large
    * parameterized SQL query. 
    * 
    * @param content the content string to use.
    * @param count the number of content strings to glue together. 
    * @param separator the separator string to use.
    *  
    * @return the string of glued content.
    */
   public static String glue(String content, int count, String separator)   
   {
      // determine the string buffer size
      int size = (content.length() + (count - 1) * separator.length()) * count;
      StringBuffer sb = new StringBuffer(size);
      
      for(int i = 0; i < count; i++)
      {
         if(i > 0)
         {
            sb.append(separator);
         }
         
         sb.append(content);
      }
      
      return sb.toString();      
   }
   
   /**
    * Removes a particular string of content from a glued together string of
    * contents that are separated by some separator string. For instance,
    * this method can be used to remove the string "1234" from any of the given
    * strings:
    * 
    * STRING BEFORE REMOVE -> STRING AFTER REMOVE
    * "1234,56723,231321,31223123" -> "56723,231321,31223123"
    * "56723,1234,123213,31223123" -> "56723,123213,31223123"
    * "231312,321325,564512,1234" -> "231312,321325,564512"
    * "1234,123456,1234567" -> "123456,1234567"
    * "123,1234,123456" -> "123456,1234567"
    * "1234" -> ""
    * "1241314,412124241" -> "1241314,412124241"
    * "123,122,1234" -> "123,122"
    * 
    * i.e. String str = removeContent("123,122,1234", ",", "1234"); 
    * 
    * The string "1234" and only the string "1234" (along with any necessary
    * separators) will be removed from any of the above strings.
    * 
    * @param gluedString the glued string of contents.
    * @param separator the separator string.
    * @param remove the string to remove.
    * 
    * @return the new string minus one instance of the removed content.
    */
   public static String removeContentOnce(
      String gluedString, String separator, String remove)
   {
      StringBuffer sb = new StringBuffer(gluedString);
      
      // see if there are any separators in the glued string
      int index = sb.indexOf(separator);
      if(index != -1)
      {
         // there are separators, so first look for (remove + separator)
         // at the beginning
         index = sb.indexOf(remove + separator);
         if(index != -1 && index != 0)
         {
            index = -1;
         }

         // if not found at the beginning...
         if(index == -1)
         {
            // next look for (separator + remove + separator) in the middle
            index = sb.indexOf(separator + remove + separator);
         }
         
         // if not found in the middle...
         if(index == -1)
         {
            // next look for (separator + remove) at the end
            index = sb.indexOf(separator + remove,
               sb.length() - separator.length() - remove.length());
         }
         
         // see if an occurrence was found
         if(index != -1)
         {
            // remove the occurrence
            sb.replace(
               index, index + separator.length() + remove.length(), "");
         }
      }
      else
      {
         // see if the glued string equals the remove string
         if(gluedString.equals(remove))
         {
            // clear the string buffer
            sb.setLength(0);
         }
      }
      
      return sb.toString();
   }
   
   /**
    * Performs the same function as removeContentOnce() multiple times until
    * the remove content is no longer present in the passed glued together
    * string of contents.
    * 
    * @param gluedString the glued string of contents.
    * @param separator the separator string.
    * @param remove the string to remove.
    * 
    * @return the new string that does not contain the removed content.
    */
   public static String removeContent(
      String gluedString, String separator, String remove)
   {
      StringBuffer sb = new StringBuffer(gluedString);
      
      // see if there are any separators in the glued string
      int index = sb.indexOf(separator);
      while(index != -1)
      {
         // there are separators, so first look for (remove + separator)
         // at the beginning
         index = sb.indexOf(remove + separator);
         if(index != -1 && index != 0)
         {
            index = -1;
         }

         // if not found at the beginning...
         if(index == -1)
         {
            // next look for (separator + remove + separator) in the middle
            index = sb.indexOf(separator + remove + separator);
         }
         
         // if not found in the middle...
         if(index == -1)
         {
            // next look for (separator + remove) at the end
            index = sb.indexOf(separator + remove,
               sb.length() - separator.length() - remove.length());
         }
         
         // see if an occurrence was found
         if(index != -1)
         {
            // remove the occurrence
            sb.replace(
               index, index + separator.length() + remove.length(), "");
         }
      }
      
      // see if the glued string equals the remove string
      if(gluedString.equals(remove))
      {
         // clear the string buffer
         sb.setLength(0);
      }
      
      return sb.toString();      
   }   
}
