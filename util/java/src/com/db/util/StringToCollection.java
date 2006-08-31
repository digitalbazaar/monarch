/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;

/**
 * This class provides static methods for converting strings to collections.
 * 
 * @author Dave Longley
 */
public class StringToCollection
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
}
