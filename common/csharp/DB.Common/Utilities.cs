/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// This class provides basic utility methods.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class Utilities
   {
      /// <summary>
      /// Compares two lists for equality. The two lists are equal
      /// if they have the same number of elements in them and if
      /// they contain the same elements.
      /// 
      /// The order of the elements is not used in the comparison.
      /// </summary>
      /// 
      /// <param name="list1">the first list to compare.</param>
      /// <param name="list2">the second list to compare.</param>
      public static bool Equals(IList list1, IList list2)
      {
         bool rval = false;
         
         if(list1 != list2)
         {
            // ensure neither list is null
            if(list1 != null && list2 != null)
            {
               // ensure list have the same number of entries 
               if(list1.Count == list2.Count)
               {
                  // compare elements
                  rval = true;
                  foreach(object element in list1)
                  {
                     if(!list2.Contains(element))
                     {
                        rval = false;
                        break;
                     }
                  }
               }
            }
         }
         
         return rval;
      }
      
      /// <summary>
      /// Compares two hashtables for equality. The two hashtables are
      /// equal if all of their keys map to the same values (the values
      /// will be compared using the Equals() method, and if the number
      /// of entries is the same for both tables.
      /// </summary>
      /// 
      /// <param name="ht1">the first hashtable to compare.</param>
      /// <param name="ht2">the second hashtable to compare.</param>
      public static bool Equals(Hashtable ht1, Hashtable ht2)
      {
         bool rval = false;
         
         if(ht1 != ht2)
         {
            // ensure neither hashtable is null
            if(ht1 != null && ht2 != null)
            {
               // ensure hashtables have the same number of entries 
               if(ht1.Count == ht2.Count)
               {
                  // compare all key->value mappings
                  rval = true;
                  foreach(object key in ht1.Keys)
                  {
                     object val1 = ht1[key];
                     object val2 = ht2[key];
                     
                     if((val1 != null && val2 == null) ||
                        (val1 == null && val2 != null))
                     {
                        rval = false;
                        break;
                     }
                     else if(val1 != null)
                     {
                        if(!val1.Equals(val2))
                        {
                           rval = false;
                           break;
                        }
                     }
                  }
               }
            }
         }
         else
         {
            rval = true;
         }
         
         return rval;
      }
   }
}
