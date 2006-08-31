/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;
import java.util.Collection;
import java.util.Iterator;

/**
 * This class provides static methods for converting collections to strings.
 * 
 * @author Dave Longley
 */
public class CollectionToString
{
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
}
