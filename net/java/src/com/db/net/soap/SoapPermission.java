/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

/**
 * A SoapPermission identifies some action or property that a SecureSoapService
 * can allow or deny access to. The name of a SoapPermission uniquely
 * identifies it. The names are broken into groups using a period ('.').
 * 
 * Listed below are the names of permissions that have already been defined
 * at this level:
 * 
 * envelope.log - Allows or denies logging soap envelopes.
 * 
 * @author Dave Longley
 */
public class SoapPermission
{
   /**
    * The name for this soap permission.
    */
   protected String mName;
   
   /**
    * Creates a new SoapPermission with the specified name.
    * 
    * @param name the name of this SoapPermission.
    * 
    * @exception IllegalArgumentException thrown if the passed name is null or
    *                                     a blank string.
    */
   public SoapPermission(String name)
   {
      if(name == null || name.length() == 0)
      {
         throw new IllegalArgumentException(
            "SoapPermission name must not be null or have a length of 0.");
      }
      
      // store name
      mName = name;
   }
   
   /**
    * Returns true if the passed object is a SoapPermission and its
    * name is equal to the name of this SoapPermission.
    * 
    * @parma obj the object to check against for equality.
    * 
    * @return true if the passed object is a SoapPermission and its name
    *         is the same as this SoapPermission's name, false otherwise.
    */
   public boolean equals(Object obj)
   {
      boolean rval = false;
      
      if(this != obj)
      {
         if(obj instanceof SoapPermission && obj != null)
         {
            SoapPermission permission = (SoapPermission)obj;
            if(getName().equals(permission.getName()))
            {
               rval = true;
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
    * Gets the name of this permission.
    *
    * @return the name of this permission.
    */
   public String getName()
   {
      return mName;
   }
}
