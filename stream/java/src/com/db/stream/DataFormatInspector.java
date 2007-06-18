/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

/**
 * A DataFormatInspector inspects data by checking to see if its format is
 * recognized or not.
 * 
 * @author Dave Longley
 */
public interface DataFormatInspector extends DataInspector
{
   /**
    * Returns whether or not this inspector is "data-satisfied." The inspector
    * is data-satisfied when it has inspected enough data to determine whether
    * or not the format of the data is recognized.
    * 
    * @return true if this inspector has inspected enough data to determine
    *         if the format is or is not recognized, false if not.
    */
   public boolean isDataSatisfied();
   
   /**
    * Returns whether or not this inspector has recognized the format of
    * the data it has been inspecting.
    * 
    * @return true if the data format is recognized, false if not.
    */
   public boolean isFormatRecognized();
   
   /**
    * Sets whether or not this inspector should keep inspecting data after
    * being data-satisfied, false if not.
    * 
    * @param inspect true to keep inspecting, false not to.
    */
   public void setKeepInspecting(boolean inspect);
   
   /**
    * Gets whether or not this inspector should keep inspecting data after
    * being data-satisfied, false if not.
    * 
    * @return true to keep inspecting, false not to.
    */
   public boolean keepInspecting();
   
   /**
    * Gets the total number of bytes inspected so far.
    * 
    * @return the number of bytes inspected so far.
    */
   public long getBytesInspected();
   
   /**
    * Gets a string identifier for the format that was detected.
    * 
    * @return a string identifier for the format that was detected.
    */
   public String getFormat();
   
   /**
    * Gets a custom readable report on the data inspection.
    * 
    * @return a custom readable report on the data inspection.
    */
   public String getInspectionReport();
}
