/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.Map.Entry;

/**
 * A DynamicObjectIterator is used to iterate over a DynamicObject.
 * 
 * @author Dave Longley
 */
public class DynamicObjectIterator implements Iterator<DynamicObject>
{
   /**
    * The DynamicObject being iterated over.
    */
   protected DynamicObject mObject;
   
   /**
    * The index of the current object.
    */
   protected int mIndex;
   
   /**
    * An iterator to the current object in the DynamicObject of type Map.
    */
   protected Iterator< Entry<String, DynamicObject> > mMapIterator;
   
   /**
    * Stores the current entry in a Map iteration.
    */
   protected Entry<String, DynamicObject> mCurrentEntry;
   
   /**
    * An iterator to the current object in the DynamicObject of type Array.
    */
   protected Iterator<DynamicObject> mArrayIterator;
   
   /**
    * Set to true for non-map/non-array objects once the object has
    * been returned.
    */
   protected boolean mFinished;
   
   /**
    * Creates a new DynamicObjectIterator to iterate over the passed
    * DynamicObject.
    * 
    * @param dyno the DynamicObject to iterate over.
    */
   @SuppressWarnings("unchecked")
   public DynamicObjectIterator(DynamicObject dyno)
   {
      mObject = dyno;
      mIndex = -1;
      
      if(dyno.getType().equals(DynamicObject.Type.Map))
      {
         HashMap<String, DynamicObject> map =
            (HashMap<String, DynamicObject>)dyno.mValue;
         mMapIterator = map.entrySet().iterator();
      }
      else if(dyno.getType().equals(DynamicObject.Type.Array))
      {
         mArrayIterator = ((ArrayList<DynamicObject>)dyno.mValue).iterator();
      }
      else
      {
         mFinished = false;
      }
   }
   
   /**
    * Returns true if there are more elements to iterate over.
    *
    * @return true if there are more elements, false if not.
    */
   public boolean hasNext()
   {
      boolean rval;
      
      switch(mObject.getType())
      {
         case Map:
            rval = mMapIterator.hasNext();
            break;
         case Array:
            rval = mArrayIterator.hasNext();
            break;
         default:
            rval = !mFinished;
            break;
      }
      
      return rval;
   }
   
   /**
    * Returns the next element in the iteration.
    *
    * @return the next element in the iteration.
    * 
    * @exception NoSuchElementException iteration has no more elements.
    */
   public DynamicObject next()
   {
      DynamicObject rval = null;
      mIndex++;
      
      switch(mObject.getType())
      {
         case Map:
            mCurrentEntry = mMapIterator.next();
            rval = mCurrentEntry.getValue();
            break;
         case Array:
            rval = mArrayIterator.next();
            break;
         default:
            // return this object, finish
            rval = mObject;
            mFinished = true;
            break;
      }
      
      return rval;
   }
   
   /**
    * Removes the current element from the underlying collection.
    * 
    * @exception UnsupportedOperationException if remove() is not supported.
    */
   public void remove()
   {
      switch(mObject.getType())
      {
         case Map:
            mMapIterator.remove();
            break;
         case Array:
            mArrayIterator.remove();
            mIndex--;
            break;
         default:
            // not supported
            throw new UnsupportedOperationException(
               "DynamicObjectIterator::remove() not supported!");
      }
   }
   
   /**
    * Gets the name of the last DynamicObject returned by next().
    * 
    * @return the name of the last DynamicObject returned by next().
    */
   public String getName()
   {
      return mCurrentEntry.getKey();
   }
   
   /**
    * Gets the index of the last DynamicObject returned by next().
    * 
    * @return the index of the last DynamicObject returned by next().
    */
   public int getIndex()
   {
      return mIndex;
   }
}
