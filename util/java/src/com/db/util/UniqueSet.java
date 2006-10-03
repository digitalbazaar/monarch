/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Collection;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

/**
 * A collection of unique objects. Whether or not an object
 * is distinct is determined only by its equals() method.
 * 
 * @param <T> the type for this unique set.
 * 
 * @author Dave Longley
 */
public class UniqueSet<T> implements Set<T>
{
   /**
    * The underlying collection that holds the objects.
    */
   protected Collection<T> mCollection;
   
   /**
    * Creates a new unique set.
    */
   public UniqueSet()
   {
      mCollection = new Vector<T>();
   }
   
   /**
    * Creates a unique set from a collection with unique elements.
    * 
    * The caller of this method must guarantee the collection is unique.
    * 
    * @param c the collection to use.
    */
   public UniqueSet(Collection<? extends T> c)
   {
      mCollection = new Vector<T>();
      for(Iterator<? extends T> i = c.iterator(); i.hasNext();)
      {
         add(i.next());
      }
   }
   
   /**
    * Creates a unique set from an array with unique elements.
    * 
    * The caller of this method must guarantee the array elements are unique.
    * 
    * @param array the array to use.
    */
   public UniqueSet(T[] array)
   {
      mCollection = new Vector<T>();

      for(int i = 0; i < array.length; i++)
      {
         add(array[i]);
      }
   }
   
   /**
    * Intersects this set with another collection.
    * 
    * @param intersect the collection to intersect with.
    */
   public void intersect(Collection<?> intersect)
   {
      mCollection.retainAll(intersect);      
   }
   
   /**
    * Intersects this set with another one.
    * 
    * @param intersect the set to intersect with.
    */
   public void intersect(UniqueSet<?> intersect)
   {
      mCollection.retainAll(intersect);      
   }
   
   /**
    * Intersects this set with an array.
    * 
    * @param intersect the array to intersect with.
    */
   public void intersect(T[] intersect)
   {
      mCollection.retainAll(new UniqueSet<T>(intersect));
   }
   
   /**
    * Intersects this set with another collection.
    * 
    * @param c the collection intersect this set with.
    * 
    * @return <tt>true</tt> if this collection changed as a result of the
    *         call.
    */
   public boolean retainAll(Collection<?> c)
   {
      boolean rval = false;
      
      if(!(c instanceof UniqueSet))
      {
         rval = mCollection.retainAll(new UniqueSet<Object>(c));
      }
      else
      {
         rval = mCollection.retainAll(c);
      }
      
      return rval;
   }
   
   /**
    * Returns true if this set contains all of the elements in the
    * passed collection.
    *
    * @param c the collection to check.
    * @return true if this set contains all of the elements in the passed
    *         collection, false if not.
    */
   public boolean containsAll(Collection<?> c)
   {
      return mCollection.containsAll(c);
   }
   
   /**
    * Converts this set into an array.
    * 
    * @return an array with the elements from this set. 
    */
   public Object[] toArray()
   {
      return mCollection.toArray();
   }
   
   /**
    * Converts this set into an array.
    * 
    * @param array an array to copy the elements into.
    * 
    * @return an array with the elements from this set. 
    */
   public <X> X[] toArray(X[] array)
   {
      return mCollection.toArray(array);
   }
   
   /**
    * Returns an iterator over the elements in this set.  The elements
    * are returned in no particular order.
    *
    * @return an Iterator over the elements in this set.
    */
   public Iterator<T> iterator()
   {
      return mCollection.iterator();
   }

   /**
    * Returns the number of elements in this set (its cardinality).
    *
    * @return the number of elements in this set (its cardinality).
    */
   public int size()
   {
      return mCollection.size();
   }
   
   /**
    * Returns true if this set is empty, false if not.
    *
    * @return true if this set is empty, false if not.
    */
   public boolean isEmpty()
   {
      return mCollection.isEmpty();
   }

   /**
    * Returns true if this set contains the specified object.
    *
    * @param o object whose presence in this set is to be tested.
    * 
    * @return true if this set contains the specified object.
    */
   public boolean contains(Object o)
   {
      return mCollection.contains(o);
   }
   
   /**
    * Adds the specified object to this set if it is not already
    * present.
    *
    * @param o object to be added to this set.
    * 
    * @return true if the set did not already contain the specified
    *         object and it was added, false if not.
    */
   public boolean add(T o)
   {
      boolean rval = false;
      
      if(!mCollection.contains(o))
      {
         rval = mCollection.add(o);
      }
      
      return rval;
   }
   
   /**
    * Adds all objects from the specified collection to this set that are
    * not already in this set.
    *
    * @param c the collection of objects to be added to this set.
    * @return true if at least one of the objects in the collection was added,
    *         false if not.
    */
   public boolean addAll(Collection<? extends T> c)
   {
      boolean rval = true;
      
      for(T obj: c)
      {
         rval &= add(obj);
      }
      
      return rval;
   }
   
   /**
    * Adds all objects from the specified unique set to this set.
    *
    * @param us the unique set of objects to be added to this set.
    * 
    * @return true if at least one of the objects in the set was added,
    *         false if not.
    */
   public boolean addAll(UniqueSet<? extends T> us)
   {
      return mCollection.addAll(us);
   }
   
   /**
    * Gets the first element from this unique set.
    * 
    * @return the first element, or null if there is no first element.
    */
   public Object getFirstElement()
   {
      Object rval = null;
      
      for(Iterator i = iterator(); i.hasNext() && rval == null;)
      {
         rval = i.next();
      }
      
      return rval;
   }
   
   /**
    * Removes the specified object from this set if it is present.
    *
    * @param o object to be removed from this set, if present.
    * 
    * @return true if the set contained the specified object,
    *         false if not.
    */
   public boolean remove(Object o)
   {
      return mCollection.remove(o);
   }

   /**
    * Removes from this set all of its elements that are contained in
    * the specified collection (optional operation).
    *
    * @param c elements to be removed from this set.
    * @return <tt>true</tt> if this set changed as a result of the call.
    */
   public boolean removeAll(Collection<?> c)
   {
      return mCollection.removeAll(c);
   }

   /**
    * Removes all of the elements from this set.
    */
   public void clear()
   {
      mCollection.clear();
   }
   
   /**
    * Compares the specified object with this set for equality.  Returns
    * <tt>true</tt> if the given object is also a set, the two sets have
    * the same size, and every member of the given set is contained in
    * this set.
    *
    * @param o the object to be compared for equality with this set.
    * 
    * @return <tt>true</tt> if the specified object is equal to this set.
    */
   @Override
   public boolean equals(Object o)
   {
      boolean rval = false;
      
      if(o != this)
      {
         if(o instanceof UniqueSet)
         {
            UniqueSet set = (UniqueSet)o;
            if(size() == set.size())
            {
               rval = containsAll(set);
            }
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns the hash code value for this set.
    *
    * @return the hash code value for this set.
    */
   @Override
   public int hashCode()
   {
      return super.hashCode();
   }
}
