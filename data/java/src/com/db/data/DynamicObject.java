/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A DynamicObject is a reference counted object with a collection of
 * unordered name/value member pairs. Members can be dynamically added to
 * a DynamicObject.
 * 
 * @author Dave Longley
 */
public class DynamicObject implements Iterable<DynamicObject>
{
   /**
    * The possible types for a DynamicObject.
    */
   @SuppressWarnings("all")
   public enum Type
   {
      String, Boolean, Number, Map, Array
   };
   
   /**
    * The type for this object.
    */
   protected Type mType;
   
   /**
    * The value for this object.
    */
   protected Object mValue;
   
   /**
    * Creates a new DynamicObject.
    */
   public DynamicObject()
   {
      set("");
   }
   
   /**
    * Creates a new DynamicObject.
    * 
    * @param value the string value for this DynamicObject.
    */
   public DynamicObject(String value)
   {
      set(value);
   }
   
   /**
    * Creates a new DynamicObject.
    * 
    * @param value the boolean value for this DynamicObject.
    */
   public DynamicObject(boolean value)
   {
      set(value);
   }
   
   /**
    * Creates a new DynamicObject.
    * 
    * @param value the number value for this DynamicObject.
    */
   public DynamicObject(Number value)
   {
      set(value);
   }
   
   /**
    * Sets this object's value to a string.
    * 
    * @param value the value for this object.
    */
   public void set(String value)
   {
      mType = Type.String;
      mValue = value;
   }
   
   /**
    * Sets this object's value to a boolean.
    * 
    * @param value the value for this object.
    */
   public void set(boolean value)
   {
      mType = Type.Boolean;
      mValue = value;
   }
   
   /**
    * Sets this object's value to a double.
    * 
    * @param value the value for this object.
    */
   public void set(Double value)
   {
      mType = Type.Number;
      mValue = copyObject(value);
   }
   
   /**
    * Sets this object's value to a number.
    * 
    * @param value the value for this object.
    */
   public void set(Number value)
   {
      mType = Type.Number;
      mValue = copyObject(value);
   }
   
   /**
    * Sets a member of this object to the passed string.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   public void set(String name, String value)
   {
      set(name, new DynamicObject(value));
   }
   
   /**
    * Sets a member of this object to the passed boolean.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   public void set(String name, boolean value)
   {
      set(name, new DynamicObject(value));
   }
   
   /**
    * Sets a member of this object to the passed double.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   public void set(String name, Double value)
   {
      set(name, new DynamicObject(value));
   }
   
   /**
    * Sets a member of this object to the passed number.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   public void set(String name, Number value)
   {
      set(name, new DynamicObject(value));
   }
   
   /**
    * Sets a member of this object to the passed DynamicObject.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   @SuppressWarnings("unchecked")
   public void set(String name, DynamicObject value)
   {
      // change to map type if necessary
      setType(Type.Map);
      HashMap<String, DynamicObject> map =
         (HashMap<String, DynamicObject>)mValue;
      map.put(name, value);
   }
   
   /**
    * Sets an element of this object to the passed string.
    * 
    * @param index the index of the element.
    * @param value the value of the element.
    */
   public void set(int index, String value)
   {
      set(index, new DynamicObject(value));
   }
   
   /**
    * Sets an element of this object to the passed boolean.
    * 
    * @param index the index of the element.
    * @param value the value of the element.
    */
   public void set(int index, boolean value)
   {
      set(index, new DynamicObject(value));
   }
   
   /**
    * Sets an element of this object to the passed number.
    * 
    * @param index the index of the element.
    * @param value the value of the element.
    */
   public void set(int index, Number value)
   {
      set(index, new DynamicObject(value));
   }
   
   /**
    * Sets an element of this object to the passed DynamicObject.
    * 
    * @param index the index of the element.
    * @param value the value of the element.
    */
   @SuppressWarnings("unchecked")
   public void set(int index, DynamicObject value)
   {
      // change to array type if necessary
      setType(Type.Array);
      
      ArrayList<DynamicObject> array =
         (ArrayList<DynamicObject>)mValue;
      
      if(index < 0)
      {
         index = array.size() + index;
      }
      
      // fill the object array as necessary
      if(index >= array.size())
      {
         int i = index - array.size() + 1;
         for(; i > 0; i--)
         {
            array.add(new DynamicObject());
         }
      }
      
      array.set(index, value);
   }
   
   /**
    * Gets a DynamicObject based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the DynamicObject.
    */
   @SuppressWarnings("unchecked")
   public DynamicObject get(String name)
   {
      DynamicObject rval = null;
      
      // change to map type if necessary
      setType(Type.Map);
      
      HashMap<String, DynamicObject> map =
         (HashMap<String, DynamicObject>)mValue;
      rval = map.get(name);
      if(rval == null)
      {
         // create new map entry
         rval = new DynamicObject();
         map.put(name, rval);
      }
      
      return rval;
   }
   
   /**
    * Gets a DynamicObject based on its index. A negative index will
    * index in reverse, with -1 referring to the last element.
    * 
    * @param index the index of the member.
    * 
    * @return the DynamicObject.
    */
   @SuppressWarnings("unchecked")
   public DynamicObject get(int index)
   {
      // change to array type if necessary
      setType(Type.Array);
      
      ArrayList<DynamicObject> array =
         (ArrayList<DynamicObject>)mValue;
      
      if(index < 0)
      {
         index = array.size() + index;
      }
      
      // fill the object array as necessary
      if(index >= array.size())
      {
         int i = index - array.size() + 1;
         for(; i > 0; i--)
         {
            array.add(new DynamicObject());
         }
      }
      
      return array.get(index);
   }
   
   /**
    * Appends a DynamicObject to this one and returns a reference to it for
    * it to be set to a value.
    * 
    * @return the DynamicObject to be set.
    */
   public DynamicObject append()
   {
      return get(length());
   }
   
   /**
    * Appends the passed DynamicObject to this one and returns a reference to
    * it for it to be set to a value.
    * 
    * @param dyno the DynamicObject to append.
    * 
    * @return the DynamicObject to be set.
    */
   public DynamicObject append(DynamicObject dyno)
   {
      set(length(), dyno);
      return dyno;
   }
   
   /**
    * Sets this object's type. Its existing value will be converted if
    * possible.
    * 
    * @param type the type to set this object to.
    */
   public void setType(Type type)
   {
      if(mType != type)
      {
         switch(type)
         {
            case String:
               set(getString());
               break;
            case Boolean:
               set(getBoolean());
               break;
            case Number:
               set(getNumber());
               break;
            case Map:
               {
                  mType = Type.Map;
                  mValue = new HashMap<String, DynamicObject>();
               }
               break;
            case Array:
               {
                  mType = Type.Array;
                  mValue = new ArrayList<DynamicObject>();
               }
               break;
         }
      }
   }
   
   /**
    * Gets this object's type.
    * 
    * @return the type of this object.
    */
   public Type getType()
   {
      return mType;
   }
   
   /**
    * Gets this object's value as a string. If the type of this object
    * is not a string, then the returned pointer may be invalidated by
    * the next call to getString().
    * 
    * @return the value of this object.
    */
   public String getString()
   {
      return String.valueOf(mValue);
   }
   
   /**
    * Gets this object's value as a boolean.
    * 
    * @return the value of this object.
    */
   public boolean getBoolean()
   {
      boolean rval = false;
      
      switch(mType)
      {
         case Boolean:
            rval = ((Boolean)mValue).booleanValue();
            break;
         case String:
            rval = mValue.equals("true");
            break;
         case Number:
            rval = ((Number)mValue).intValue() == 1;
            break;
         default:
            break;
      }
      
      return rval;
   }
   
   /**
    * Gets this object's value as a Number.
    * 
    * @return the value of this object.
    */
   public Number getNumber()
   {
      Number rval = null;
      
      switch(mType)
      {
         case Number:
            rval = (Number)copyObject(mValue);
            break;
         case String:
            try
            {
               rval = Double.parseDouble((String)mValue);
            }
            catch(NumberFormatException e)
            {
               rval = 0;
            }
            break;
         case Boolean:
            rval = ((Boolean)mValue).booleanValue() ? 1 : 0;
            break;
         default:
            rval = 0;
            break;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this object has the specified member, false if not.
    * 
    * @param name the name of the member.
    * 
    * @return true if this object has a member with the specified name,
    *         false if not.
    */
   @SuppressWarnings("unchecked")
   public boolean hasMember(String name)
   {
      boolean rval = false;
      
      if(mType.equals(Type.Map))
      {
         rval = ((HashMap<String, DynamicObject>)mValue).containsKey(name);
      }
      
      return rval;
   }
   
   /**
    * Removes a member from this object, if it has it.
    * 
    * @param name the member to remove from this object.
    * 
    * @return the removed object, if one was removed, otherwise NULL.
    */
   @SuppressWarnings("unchecked")
   public DynamicObject removeMember(String name)
   {
      DynamicObject rval = null;
      
      if(mType.equals(Type.Map))
      {
         rval = ((HashMap<String, DynamicObject>)mValue).remove(name);
      }
      
      return rval;
   }
   
   /**
    * Clears associated data.
    * Maps and Arrays: removes all items.
    * Strings: set to "".
    * Numbers: set to 0.
    */
   @SuppressWarnings("unchecked")
   public void clear()
   {
      switch(mType)
      {
         case String:
            set("");
            break;
         case Boolean:
            set(false);
            break;
         case Number:
            set(0);
            break;
         case Map:
            ((HashMap<String, DynamicObject>)mValue).clear();
            break;
         case Array:
            ((ArrayList<DynamicObject>)mValue).clear();
            break;
      }
   }
   
   /**
    * Gets the length of this object based on its type.
    * 
    * String length = number of characters in the string.
    * Number length = 8.
    * Boolean length = 1
    * Map length = number of entries in the map.
    * Array length = number of elements in the array.
    * 
    * @return the length of this object.
    */
   @SuppressWarnings("unchecked")
   public int length()
   {
      int rval = 0;
      
      switch(mType)
      {
         case String:
            rval = ((String)mValue).length();
            break;
         case Boolean:
            rval = 1;
            break;
         case Number:
            rval = 8;
            break;
         case Map:
            rval = ((HashMap<String, DynamicObject>)mValue).size();
            break;
         case Array:
            rval = ((ArrayList<DynamicObject>)mValue).size();
            break;            
      }
      
      return rval;
   }
   
   /**
    * Compares this DynamicObject to another one for equality. If the
    * DynamicObjects are the same type, and have the same value or contain
    * all the same values (for Maps/Arrays), then they are equal.
    * 
    * @param rhs the DynamicObject to compare to this one.
    * 
    * @return true if the passed DynamicObject is equal to this one, false
    *         if not.
    */ 
   public boolean equals(DynamicObject rhs)
   {
      boolean rval = false;
      
      if(this == rhs)
      {
         rval = true;
      }
      else if(mType.equals(rhs.getType()))
      {
         rval = mValue.equals(rhs.mValue);
      }
      else if(!mType.equals(Type.Array) &&
              !mType.equals(Type.Map) &&
              !rhs.getType().equals(Type.Array) &&
              !rhs.getType().equals(Type.Map))
      {
         // for non-maps/non-arrays, compare based on string values
         rval = String.valueOf(mValue).equals(String.valueOf(rhs.mValue));
      }
      
      return rval;
   }
   
   /**
    * Gets a reference-counted DynamicObjectIterator for iterating over
    * the members of this object or its array elements.
    * 
    * @return a DynamicObjectIterator.
    */
   public Iterator<DynamicObject> iterator()
   {
      return new DynamicObjectIterator(this);
   }
   
   /**
    * Gets a reference-counted DynamicObjectIterator for iterating over
    * the members of this object or its array elements.
    * 
    * @return a DynamicObjectIterator.
    */
   public DynamicObjectIterator getIterator()
   {
      return new DynamicObjectIterator(this);
   }
   
   /**
    * Clones this DynamicObject and returns it.
    * 
    * @return a clone of this DynamicObject.
    */
   @Override
   @SuppressWarnings("unchecked")
   public DynamicObject clone()
   {
      // copy object
      return (DynamicObject)copyObject(this);
   }
   
   /**
    * Merges the passed DynamicObject into this one.
    * 
    * If the passed DynamicObject is something other than a map or an array,
    * then this DynamicObject will simply be set to the clone of the passed
    * DynamicObject. If the passed DynamicObject is a map, then all of its
    * key-value pairs will be merged into this DynamicObject, overwriting
    * any overlapping pairs. If it is an array, then its elements can either
    * overwrite the elements in this DynamicObject or be appended to it. 
    * 
    * @param rhs the DynamicObject to merge into this one.
    * @param append true to append arrays, false to overwrite them.
    */
   public void merge(DynamicObject rhs, boolean append)
   {
      switch(rhs.getType())
      {
         case String:
            set(rhs.getString());
         case Boolean:
            set(rhs.getBoolean());
         case Number:
            set(rhs.getNumber());
            break;
         case Map:
            {
               setType(Type.Map);
               DynamicObjectIterator i = rhs.getIterator();
               while(i.hasNext())
               {
                  DynamicObject next = i.next();
                  get(i.getName()).merge(next, append);
               }
            }
            break;
         case Array:
            if(append)
            {
               setType(Type.Array);
               DynamicObjectIterator i = rhs.getIterator();
               int length = length();
               for(int ii = 0; i.hasNext(); ii++)
               {
                  get(length + ii).merge(i.next(), append);
               }
            }
            else
            {
               DynamicObjectIterator i = rhs.getIterator();
               for(int ii = 0; i.hasNext(); ii++)
               {
                  get(ii).merge(i.next(), append);
               }
            }
            break;
      }
   }
   
   /**
    * Determines if this DynamicObject is a subset of another. If this
    * DynamicObject does not reference the exact same DynamicObject as the
    * passed one, then a check will be made to determine if this DynamicObject
    * this is a subset of the passed one.
    * 
    * This DynamicObject is only a subset of the passed one if both are
    * Maps and the passed DynamicObject contains at least all of the members
    * and values of this DynamicObject.
    * 
    * @param rhs the DynamicObject to check as a superset.
    * 
    * @return true if this DynamicObject is a subset of the passed one.
    */
   public boolean isSubset(DynamicObject rhs)
   {
      boolean rval = false;
      
      if(this == rhs)
      {
         rval = true;
      }
      else if(mType.equals(Type.Map) && rhs.getType().equals(Type.Map))
      {
         // ensure right map has same or greater length
         if(length() <= rhs.length())
         {
            rval = true;
            DynamicObjectIterator i = getIterator();
            while(rval && i.hasNext())
            {
               DynamicObject leftDyno = i.next();
               if(rhs.hasMember(i.getName()))
               {
                  DynamicObject rightDyno = rhs.get(i.getName());
                  if(leftDyno.getType().equals(Type.Map) &&
                     rightDyno.getType().equals(Type.Map))
                  {
                     rval = leftDyno.isSubset(rightDyno);
                  }
                  else
                  {
                     rval = (leftDyno.equals(rightDyno));
                  }
               }
               else
               {
                  rval = false;
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Deep-copies the passed object.
    * 
    * @param obj the object to deep-copy.
    * 
    * @return the deep-copied object.
    */
   static Object copyObject(Object obj)
   {
      Object rval = null;
      
      try
      {
         // write object to byte array
         ByteArrayOutputStream baos = new ByteArrayOutputStream(2048);
         ObjectOutputStream out = new ObjectOutputStream(baos);
         out.writeObject(obj);
         out.flush();
         out.close();
         
         // read object from byte array
         ObjectInputStream in = new ObjectInputStream(
            new ByteArrayInputStream(baos.toByteArray()));
         rval = in.readObject();
      }
      catch(Throwable t)
      {
         // should not happen
         LoggerManager.getLogger("dbdata").
            error(DynamicObject.class, Logger.getStackTrace(t));
      }
      
      return rval;
   }
}
