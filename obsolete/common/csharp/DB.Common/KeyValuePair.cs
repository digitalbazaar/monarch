/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.IO;
using System.Text;
using System.Xml;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A simple class for storing key-value pairs.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class KeyValuePair : IComparer
   {
      /// <summary>
      /// The key.
      /// </summary>
      protected object mKey;
      
      /// <summary>
      /// The value.
      protected object mValue;
      
      /// <summary>
      /// Creates a new KeyValuePair with the given key and value.
      /// </summary>
      ///
      /// <param name="key">the key.</param>
      /// <param name="val">the value.</param>
      public KeyValuePair(object key, object val)
      {
         if(key != null)
         {
            mKey = key;
         }
         else
         {
            mKey = "null";
         }
         
         if(val != null)
         {
            mValue = val;
         }
         else
         {
            mValue = "null";
         }
      }
      
      /// <summary>
      /// Gets the key of this key-value pair.
      /// </summary>
      public virtual object Key
      {
         get
         {
            return mKey;
         }
      }
      
      /// <summary>
      /// Gets the value of this key-value pair.
      /// </summary>
      public virtual object Value
      {
         get
         {
            return mValue;
         }
      }
      
      /// <summary>
      /// Compares two key-value pairs. Compares on the value of the key-value
      /// pair. If the first argument has a lesser value than the second,
      /// a negative integer is returned. If the first argument is equal to
      /// the second, 0 is returned. If the first argument is greater than
      /// the second, than a positive integer is returned.
      /// </summary>
      /// 
      /// <param name="o1">the first object to be compared.</param>
      /// <param name="o2">the second object to be compared.</param>
      /// <returns>a negative integer, zero, or a positive integer as the
      /// first argument is less than, equal to, or greater than the
      /// second.</returns> 
      public virtual int Compare(object o1, object o2)
      {
         int rval = 0;
         
         KeyValuePair kvp1 = (KeyValuePair)o1;
         KeyValuePair kvp2 = (KeyValuePair)o2;
         
         if(kvp1.Value != null && kvp1.Value.Equals(kvp2.Value))
         {
            rval = 0;
         }
         else if(kvp1.Value != null && kvp2.Value != null)
         {
            string value1 = kvp1.Value.ToString();
            string value2 = kvp2.Value.ToString();
         
            rval = value1.CompareTo(value2);
         
            if(rval != 0)
            {
               rval = (rval < 0) ? -1 : 1;
            }
         }
         
         return rval;
      }
      
      /// <summary>
      /// A key-value pair is equal to another key-value pair if
      /// its key and value are equal.
      /// </summary>
      /// 
      /// <param name="kvp">another key-value pair to compare to.</param>
      public virtual bool Equals(KeyValuePair kvp)
      {
         bool rval = false;
         
         if(kvp != this)
         {
            /*if(kvp != null)
            {
               if(kvp.Key.Equals(Key) &&
                  kvp.Value.Equals(Value))
               {
                  rval = true;
               }
            }*/
         }
         else
         {
            rval = true;
         }
         
         return rval;
      }
   }
}
