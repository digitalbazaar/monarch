/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;
import java.util.Iterator;

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
   public static void split(String str, String regex, Collection collection)
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
   public static String glue(Collection collection, String separator)
   {
      StringBuffer sb = new StringBuffer();
      
      for(Iterator i = collection.iterator(); i.hasNext();)
      {
         if(sb.length() > 0)
         {
            sb.append(separator);
         }
         
         sb.append(String.valueOf(i.next()));
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
}
