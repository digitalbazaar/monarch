/* 
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Runtime.CompilerServices;

namespace DB.Common
{
   /// <summary>
   /// An associate array list is an array list whose items are associated
   /// with other objects. The items in this list can be sorted without
   /// losing the individual items' associations.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class AssociativeArrayList : ArrayList, IComparer
   {
      /// <summary>
      /// A compare delegate that follows the IComparer method signature.
      /// </summary>
      public delegate int CompareDelegate(object o1, object o2);
      
      /// <summary>
      /// The compare delegate to call to sort this collection.
      /// </summary>
      protected CompareDelegate mCompareDelegate;
      
      /// <summary>
      /// The hashtable that maintains the associations for the items in
      /// this list and whatever object they are associated with.
      /// </summary>
      protected Hashtable mAssociationTable;
      
      /// <summary>
      /// Creates a new associative array list.
      /// </summary>
      public AssociativeArrayList()
      {
         // create the association table
         mAssociationTable = new Hashtable();
      }
      
      /// <summary>
      /// Compares two objects o1 and o2 by using some sort function.
      /// </summary>
      /// 
      /// <param name="o1">the first object to compare.</param>
      /// <param name="o2">the second object to compare.</param>
      /// <returns>if the first object is less than the second one, a value
      /// less than 0 is returned. If the first object is greater than the
      /// second one, a value greater than 0 is returned. If the objects are
      /// equal, then 0 is returned.</returns>
      public int Compare(object o1, object o2)
      {
         int rval = 0;
         
         if(mCompareDelegate != null)
         {
            rval = mCompareDelegate(o1, o2);
         }
         
         return rval;
      }
      
      /// <summary>
      /// Adds an item to this list along with an association for the item.
      /// If the item passed is null, no association will be set.
      /// </summary>
      /// 
      /// <param name="item">the item to add.</param>
      /// <param name="association">the object to associate with the
      /// item.</param>
      /// <returns>the index at which the item was added.</returns>
      public virtual int Add(object item, object association)
      {
         // add the item to the list
         int index = base.Add(item);
         
         if(item != null)
         {
            // add the association to the association table
            mAssociationTable[item] = association;
         }
         
         return index;
      }
      
      /// <summary>
      /// Removes the given object from this collection.
      /// 
      /// Overridden to remove an item's association from the association
      /// table when it is removed from this list.
      /// </summary>
      /// 
      /// <param name="obj">the object to remove.</param>
      public override void Remove(object obj)
      {
         if(obj != null)
         {
            // remove the object from the association table
            mAssociationTable.Remove(obj);
         }

         // remove the object from the list
         base.Remove(obj);
      }
      
      /// <summary>
      /// Removes an item at the given index.
      /// 
      /// Overridden to remove an item's association from the association
      /// table when it is removed from this list.
      /// </summary>
      /// 
      /// <param name="obj">the object to remove.</param>
      public override void RemoveAt(int index)
      {
         if(index >= 0 && index < Count)
         {
            // get the object
            object obj = this[index];
            
            // remove the object if it isn't null
            if(obj != null)
            {
               mAssociationTable.Remove(this[index]);
            }
         }
         
         // remove the object from the list
         base.RemoveAt(index);
      }
      
      /// <summary>
      /// Clears this collection.
      /// 
      /// Overridden to clear the association table.
      /// </summary>
      public override void Clear()
      {
         // clear the table and the list
         mAssociationTable.Clear();
         base.Clear();
      }
      
      /// <summary>
      /// Sorts this array according to some specified compare delegate.
      /// </summary>
      /// 
      /// <param name="d">the compate delegate to use.</param>
      [MethodImpl(MethodImplOptions.Synchronized)]
      public virtual void Sort(CompareDelegate d)
      {
         mCompareDelegate = d;
         Sort(this);
      }
      
      /// <summary>
      /// Gets the associated object for the given item.
      /// </summary>
      /// 
      /// <param name="item">the item to get the associated item
      /// for.</param>
      /// 
      /// <returns>the associated object for the given item.</returns>
      public object GetAssociatedObject(object item)
      {
         object rval = null;
         
         if(item != null)
         {
            rval = mAssociationTable[item];
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets or sets the association for an item in this list. If
      /// the index passed is not an object in this list, then the
      /// item will be appened to this list.
      /// 
      /// If setting an item and the item passed is null, no association
      /// will be set for that item.
      /// </summary>
      public virtual object this[object item]
      {
         get
         {
            object rval = null;
            
            // see if the passed object is an index or an item
            if(item is int)
            {
               rval = base[(int)item];
            }
            else if(item is Int32)
            {
               rval = base[Convert.ToInt32(item)];
            }
            else
            {
               // return the association for this item
               rval = GetAssociatedObject(item);
            }
            
            return rval;
         }
         
         set
         {
            // see if the passed object is an index or an item
            if(item is int)
            {
               base[(int)item] = value;
            }
            else if(item is Int32)
            {
               base[Convert.ToInt32(item)] = value;
            }
            else
            {
               // see if the item is in the list
               if(item != null && Contains(item))
               {
                  mAssociationTable[item] = value;
               }
               else
               {
                  // add the item and its association
                  Add(item, value);
               }
            }
         }
      }
   }
}
